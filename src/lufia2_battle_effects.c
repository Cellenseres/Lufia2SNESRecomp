/* Sample battle art before sprites and colour math. */
#include "lufia2_battle_effects.h"

#include <stdlib.h>
#include <string.h>

#include "lufia2_battle_widescreen.h"
#include "lufia2_margin_assets.h"

extern uint8_t g_ram[];

static uint32_t *s_art;
static unsigned s_art_width;
static unsigned s_art_height;
static unsigned s_center_left;
static unsigned s_center_right;
static uint8_t s_background;
static bool s_loaded;
static bool s_active;
static bool s_uniform_palette;
static unsigned s_palette_colour;
/* Transparent pixels retain their palette colour. */
static uint32_t s_line_colours[2][kPpuBufWidth];

static bool LoadArt(uint8_t id, unsigned width, unsigned height) {
    SnesRecompMarginAsset asset;
    if (s_loaded && id == s_background && width == s_art_width && height == s_art_height)
        return true;
    if (!Lufia2MarginAssetFind(LUFIA2_MARGIN_SCENE_BATTLE, id, &asset) ||
        asset.canvas_width != width || asset.canvas_height != height ||
        asset.center_width > 256u || width < 256u || width > kPpuBufWidth ||
        height > 224u || ((width - 256u) & 1u))
        return false;
    uint32_t *art = calloc((size_t)width * height, sizeof *art);
    if (!art)
        return false;
    const SnesRecompMarginComposite destination = {
        .pixels = (uint8_t *)art, .pitch = (size_t)width * 4u,
        .width = (uint16_t)width, .height = (uint16_t)height,
        .brightness = 15u,
    };
    if (!snesrecomp_margin_asset_composite_argb8888(&asset, &destination)) {
        free(art);
        return false;
    }
    free(s_art);
    s_art = art;
    s_art_width = width;
    s_art_height = height;
    s_center_left = asset.center_x;
    s_center_right = asset.center_x + asset.center_width;
    s_background = id;
    s_loaded = true;
    return true;
}

bool Lufia2BattleEffectsPrepare(Ppu *ppu, bool wide,
                               unsigned width, unsigned height) {
    const char *enabled = getenv("LUFIA2_BATTLE_WIDE_EFFECTS");
    uint8_t id;
    const Lufia2BattleState battle = Lufia2BattleInspect(g_ram);
    s_active = wide && !(enabled && strcmp(enabled, "0") == 0) &&
        Lufia2BattleWidescreenMargin(&battle, ppu, &id) &&
        PPU_mode(ppu) == 1 && LoadArt(id, width, height);
    return s_active;
}

bool Lufia2BattleEffectsActive(const Ppu *ppu) {
    return s_active && ppu && PPU_mode(ppu) == 1;
}

static void ExtendSceneWindow(int left, int right,
                              int *window_left, int *window_right) {
    /* Preserve finite names; extend the scene crop. */
    if (*window_left <= 16 && *window_right >= 239) {
        *window_left = left;
        *window_right = right - 1;
    }
}

bool Lufia2BattleEffectsWindows(const Ppu *ppu, unsigned layer,
                               int left, int right, int *w1_left, int *w1_right,
                               int *w2_left, int *w2_right, uint32_t *flags) {
    if (!Lufia2BattleEffectsActive(ppu) || (layer != 0u && layer != 5u) ||
        PPU_bgTilemapAdr(ppu, 1) != 0x5c00u ||
        PPU_bgTileAdr(ppu, 1) != 0x4000u)
        return false;
    /* Hide empty name placeholders. */
    if (layer == 5u && PPU_clipMode(ppu) == 0 &&
        PPU_halfColor(ppu) && ppu->fixedColor == 0) {
        if (*w1_left == 0 && *w1_right == 0)
            *flags &= ~3u;
        if (*w2_left == 0 && *w2_right == 0)
            *flags &= ~12u;
    }
    ExtendSceneWindow(left, right, w1_left, w1_right);
    ExtendSceneWindow(left, right, w2_left, w2_right);
    return true;
}

void Lufia2BattleEffectsBeginLine(const Ppu *ppu) {
    memset(s_line_colours, 0, sizeof s_line_colours);
    s_uniform_palette = Lufia2BattleEffectsActive(ppu);
    if (s_uniform_palette) {
        /* Palettes 2 and 3 carry background flashes. */
        s_palette_colour = ppu->cgram[32];
        for (unsigned i = 33; i < 64; ++i) {
            if (ppu->cgram[i] != s_palette_colour)
                s_uniform_palette = false;
        }
    }
}

int Lufia2BattleEffectsMosaic(int x, unsigned size) {
    if (!size)
        return x;
    const int remainder = x % (int)size;
    return x - (remainder < 0 ? remainder + (int)size : remainder);
}

static int ScrollOffset(unsigned scroll) {
    scroll &= 1023u;
    return scroll < 512u ? (int)scroll : (int)scroll - 1024;
}

void Lufia2BattleEffectsMargin(Ppu *ppu, PpuPixelPrioBufs *background,
                              unsigned y, bool sub, int left, int right,
                              PpuZbufType low_priority, bool mosaic) {
    const int margin = (int)(s_art_width - 256u) / 2;
    const int shift = ScrollOffset(ppu->hScroll[0]);
    PpuZbufType priorities[2] = {low_priority, low_priority};
    const int edges[2] = {(int)s_center_left - margin - shift,
                         (int)s_center_right - margin - shift - 1};
    for (unsigned side = 0; side < 2; ++side) {
        if (edges[side] >= -ppu->extraLeftCur &&
            edges[side] < 256 + ppu->extraRightCur) {
            const unsigned pixel = background->data[edges[side] + kPpuExtraLeftRight];
            if ((pixel & 0x0f00u) == 0 && (pixel & 255u))
                priorities[side] = (PpuZbufType)(pixel & 0xf000u);
        }
    }
    /* $97:B5D3 sets H=0, V=-1. */
    int source_y = (int)y - 1 + ScrollOffset(ppu->vScroll[0] + 1u);
    if (mosaic)
        source_y = (int)ppu->mosaicModulo[y] + ScrollOffset(ppu->vScroll[0]);
    if (source_y < 0)
        source_y = 0;
    if (source_y >= (int)s_art_height)
        source_y = (int)s_art_height - 1;
    for (int x = left; x < right; ++x) {
        const int sampled_x = mosaic
            ? Lufia2BattleEffectsMosaic(x, PPU_mosaicSize(ppu)) : x;
        int source_x = sampled_x + margin + shift;
        if (source_x >= (int)s_center_left && source_x < (int)s_center_right)
            continue; /* Keep the native centre sample. */
        if (source_x < 0)
            source_x = 0;
        if (source_x >= (int)s_art_width)
            source_x = (int)s_art_width - 1;
        const unsigned at = (unsigned)(x + kPpuExtraLeftRight);
        const uint32_t colour = s_art[(size_t)source_y * s_art_width + source_x];
        background->data[at] = 0x0500u; /* No repeated native tiles in art gaps. */
        if (colour >> 24) {
            s_line_colours[sub][at] = colour;
            /* Art keeps BG1's layer and priority. */
            background->data[at] = priorities[source_x >= (int)s_center_right];
        }
    }
}

unsigned Lufia2BattleEffectsColour(const Ppu *ppu, unsigned pixel,
                                  unsigned index, bool sub) {
    const uint32_t colour = s_line_colours[sub][index];
    if (!Lufia2BattleEffectsOpaque(pixel, index, sub))
        return ppu->cgram[pixel & 255u];
    if (s_uniform_palette)
        return s_palette_colour;
    /* Convert art to SNES RGB555. */
    return ((colour >> 19) & 31u) | ((colour >> 6) & 0x03e0u) |
           ((colour << 7) & 0x7c00u);
}

bool Lufia2BattleEffectsOpaque(unsigned pixel, unsigned index, bool sub) {
    return (pixel & 0x0fffu) == 0 && (s_line_colours[sub][index] >> 24) != 0;
}

void Lufia2BattleEffectsShutdown(void) {
    free(s_art);
    s_art = NULL;
    s_loaded = s_active = false;
    Lufia2BattleEffectsBeginLine(NULL);
}
