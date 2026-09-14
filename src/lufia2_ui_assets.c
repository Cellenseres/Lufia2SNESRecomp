#include "lufia2_ui_assets.h"

#include <stdlib.h>

#include "lufia2_resource.h"

enum {
    LUFIA2_PANEL_RESOURCE_ID = 642,
    LUFIA2_PANEL_RESOURCE_SIZE = 0x4000,
    LUFIA2_PANEL_PALETTE_FILE_OFFSET = 0x0F8040,
    LUFIA2_PANEL_PALETTE_COLOURS = 16,
    LUFIA2_PANEL_TILE_BYTES = 32,
    LUFIA2_PANEL_TILE_PIXELS = 8,
    LUFIA2_PANEL_TILES_WIDE = 6,
    LUFIA2_PANEL_TILES_HIGH = 6,
    LUFIA2_PANEL_WIDTH = 48,
    LUFIA2_PANEL_HEIGHT = 48,
    LUFIA2_PANEL_MARGIN = 16,
};

/* Canonical 48x48 tiled 9-slice derived from the verified BG1 panel. */
static const uint16_t kPanelTilemap[LUFIA2_PANEL_TILES_HIGH]
                                   [LUFIA2_PANEL_TILES_WIDE] = {
    {0x0801, 0x0802, 0x0805, 0x0806, 0x4802, 0x4801},
    {0x0803, 0x0804, 0x080F, 0x080F, 0x4804, 0x4803},
    {0x0807, 0x080F, 0x080F, 0x080F, 0x080F, 0x4807},
    {0x0808, 0x080F, 0x080F, 0x080F, 0x080F, 0x4808},
    {0x8803, 0x8804, 0x080F, 0x080F, 0xC804, 0xC803},
    {0x8801, 0x8802, 0x8805, 0x8806, 0xC802, 0xC801},
};

/* Production reads this palette from the ROM; the copy detects recipe drift. */
static const uint16_t kExpectedPanelPalette[LUFIA2_PANEL_PALETTE_COLOURS] = {
    0x280A, 0x0400, 0x0024, 0x0066,
    0x08C9, 0x152C, 0x218F, 0x2DF2,
    0x3A55, 0x0C85, 0x10A6, 0x14C7,
    0x1D09, 0x254B, 0x2D8D, 0x31CF,
};

static uint16_t ReadU16Le(const uint8_t *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint8_t Expand5(uint16_t value) {
    value &= 31u;
    return (uint8_t)((value << 3) | (value >> 2));
}

static uint32_t Bgr555ToArgb(uint16_t colour) {
    const uint32_t red = Expand5(colour);
    const uint32_t green = Expand5((uint16_t)(colour >> 5));
    const uint32_t blue = Expand5((uint16_t)(colour >> 10));
    return 0xFF000000u | (red << 16) | (green << 8) | blue;
}

static void DecodeTile4bpp(const uint8_t *tile,
                           const uint16_t palette[16],
                           bool h_flip,
                           bool v_flip,
                           uint32_t out_pixels[64]) {
    for (unsigned y = 0; y < LUFIA2_PANEL_TILE_PIXELS; y++) {
        const unsigned source_y = v_flip ? 7u - y : y;
        for (unsigned x = 0; x < LUFIA2_PANEL_TILE_PIXELS; x++) {
            const unsigned source_x = h_flip ? 7u - x : x;
            const unsigned bit = 7u - source_x;
            const unsigned colour =
                ((tile[source_y * 2u] >> bit) & 1u) |
                (((tile[source_y * 2u + 1u] >> bit) & 1u) << 1) |
                (((tile[16u + source_y * 2u] >> bit) & 1u) << 2) |
                (((tile[16u + source_y * 2u + 1u] >> bit) & 1u) << 3);
            out_pixels[y * LUFIA2_PANEL_TILE_PIXELS + x] =
                Bgr555ToArgb(palette[colour]);
        }
    }
}

bool Lufia2UiAssetsExtract(const uint8_t *headerless_rom,
                           size_t rom_size,
                           Lufia2UiPanelAsset *out) {
    Lufia2ResourceView resource = {0};
    uint16_t palette[LUFIA2_PANEL_PALETTE_COLOURS];
    uint32_t *pixels = NULL;
    uint32_t tile_pixels[64];
    bool ok = false;

    if (!out)
        return false;
    *out = (Lufia2UiPanelAsset){0};
    if (!headerless_rom || rom_size != LUFIA2_US_ROM_SIZE ||
        LUFIA2_PANEL_PALETTE_FILE_OFFSET > rom_size ||
        rom_size - LUFIA2_PANEL_PALETTE_FILE_OFFSET < sizeof palette)
        return false;

    for (unsigned i = 0; i < LUFIA2_PANEL_PALETTE_COLOURS; i++) {
        palette[i] = ReadU16Le(
            headerless_rom + LUFIA2_PANEL_PALETTE_FILE_OFFSET + i * 2u);
        if (palette[i] != kExpectedPanelPalette[i])
            return false;
    }

    if (!Lufia2ResourceExtract(headerless_rom, rom_size,
                               LUFIA2_PANEL_RESOURCE_ID, &resource) ||
        resource.size != LUFIA2_PANEL_RESOURCE_SIZE)
        goto done;

    pixels = (uint32_t *)malloc(
        LUFIA2_PANEL_WIDTH * LUFIA2_PANEL_HEIGHT * sizeof(*pixels));
    if (!pixels)
        goto done;

    for (unsigned tile_y = 0; tile_y < LUFIA2_PANEL_TILES_HIGH; tile_y++) {
        for (unsigned tile_x = 0; tile_x < LUFIA2_PANEL_TILES_WIDE; tile_x++) {
            const uint16_t word = kPanelTilemap[tile_y][tile_x];
            const unsigned tile_number = word & 0x03FFu;
            const size_t tile_offset =
                (size_t)tile_number * LUFIA2_PANEL_TILE_BYTES;

            if (((word >> 10) & 7u) != 2u || (word & 0x2000u) != 0u ||
                tile_offset > resource.size ||
                resource.size - tile_offset < LUFIA2_PANEL_TILE_BYTES)
                goto done;

            DecodeTile4bpp(resource.data + tile_offset, palette,
                           (word & 0x4000u) != 0u,
                           (word & 0x8000u) != 0u,
                           tile_pixels);

            for (unsigned y = 0; y < LUFIA2_PANEL_TILE_PIXELS; y++) {
                for (unsigned x = 0; x < LUFIA2_PANEL_TILE_PIXELS; x++) {
                    const unsigned destination_x =
                        tile_x * LUFIA2_PANEL_TILE_PIXELS + x;
                    const unsigned destination_y =
                        tile_y * LUFIA2_PANEL_TILE_PIXELS + y;
                    pixels[destination_y * LUFIA2_PANEL_WIDTH + destination_x] =
                        tile_pixels[y * LUFIA2_PANEL_TILE_PIXELS + x];
                }
            }
        }
    }

    *out = (Lufia2UiPanelAsset){
        pixels,
        LUFIA2_PANEL_WIDTH,
        LUFIA2_PANEL_HEIGHT,
        LUFIA2_PANEL_MARGIN,
        LUFIA2_PANEL_MARGIN,
        LUFIA2_PANEL_MARGIN,
        LUFIA2_PANEL_MARGIN,
        true,
    };
    pixels = NULL;
    ok = true;

done:
    free(pixels);
    Lufia2ResourceDestroy(&resource);
    return ok;
}

void Lufia2UiAssetsDestroy(Lufia2UiPanelAsset *asset) {
    if (!asset)
        return;
    free(asset->pixels);
    *asset = (Lufia2UiPanelAsset){0};
}
