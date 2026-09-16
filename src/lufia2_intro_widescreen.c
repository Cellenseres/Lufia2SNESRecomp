#include "lufia2_intro_widescreen.h"

#include <stdio.h>
#include <stdlib.h>

#include "lufia2_intro_margins.h"
#include "lufia2_intro_trees.h"
#include "lufia2_runtime.h"

enum {
    WAVE_BG = 0x01,
    BACKDROP_BG = 0x02,
    TREE_BG = 0x04,
    BACKDROP_LAYER = 1,
    TREE_LAYER = 2,
    SCENE_BG_MASK = 0x07,
    INTRO_MAP = 0x02,
    NO_MAP = 0x00,
    TILEMAP_COLUMNS = 32,
    TILEMAP_ROWS = 32,
    /* One margin, at eight pixels per column. */
    BORDER_COLUMNS = 6,
    NATIVE_WIDTH = 256,
    /* The scene clips its own outer columns. */
    WINDOW_MASK = 0x03,
};

typedef struct IntroWidescreen {
    bool active;
    bool trees_active;
    bool layer_borrowed;
    bool window_borrowed;
    uint8_t window_left;
    uint8_t window_right;
    unsigned reported_static_lines;
    size_t lines;
    uint16_t line_scroll_x[LUFIA2_PPU_VISIBLE_LINES];
    uint16_t line_scroll_y[LUFIA2_PPU_VISIBLE_LINES];
    uint8_t line_mosaic[LUFIA2_PPU_VISIBLE_LINES];
} IntroWidescreen;

static IntroWidescreen s_state;

/* Which layers may leave the authentic columns. */
static uint8_t RequestedLayers(void) {
    static int s_layers = -1;
    if (s_layers < 0) {
        const char *choice = getenv("LUFIA2_INTRO_WIDE");
        const char first = choice ? *choice : '\0';
        if (first == 't' || first == 'T')
            s_layers = WAVE_BG | TREE_BG;
        else if (first == '1' || first == 'o' || first == 'O' || first == 'y')
            s_layers = WAVE_BG;
        else
            s_layers = 0;
    }
    return (uint8_t)s_layers;
}

/* Override for the painting's last line; 0 detects. */
static unsigned SkyLineOverride(void) {
    static int s_lines = -1;
    if (s_lines < 0) {
        const char *choice = getenv("LUFIA2_INTRO_SKY_LINES");
        const int value = choice ? atoi(choice) : 0;
        s_lines = value < 0 ? 0
                : (value > LUFIA2_PPU_VISIBLE_LINES
                       ? LUFIA2_PPU_VISIBLE_LINES
                       : value);
    }
    return (unsigned)s_lines;
}

/* Told apart by where its layers keep their graphics. */
/* Wrapping is invisible only where the border is one colour. */
static unsigned BorderTilePixel(
    const Ppu *ppu, unsigned chars, unsigned tile, unsigned bpp,
    unsigned x, unsigned y) {
    const unsigned words = (bpp == 2u) ? 8u : 16u;
    const unsigned base = chars + tile * words + y;
    const uint16_t low = ppu->vram[base & 0x7fffu];
    const unsigned bit = 7u - x;
    unsigned value = ((low >> bit) & 1u) | (((low >> (8u + bit)) & 1u) << 1);
    if (bpp == 4u) {
        const uint16_t high = ppu->vram[(base + 8u) & 0x7fffu];
        value |= (((high >> bit) & 1u) << 2) | (((high >> (8u + bit)) & 1u) << 3);
    }
    return value;
}

static bool BordersAreFlat(const Ppu *ppu, unsigned layer) {
    const unsigned tilemap = (unsigned)PPU_bgTilemapAdr(ppu, layer);
    const unsigned chars = (unsigned)PPU_bgTileAdr(ppu, layer);
    const unsigned bpp = (layer == 2u) ? 2u : 4u;
    const unsigned depth = (bpp == 2u) ? 4u : 16u;
    int colour = -1;

    for (unsigned row = 0; row < TILEMAP_ROWS; row++) {
        for (unsigned i = 0; i < BORDER_COLUMNS * 2u; i++) {
            const unsigned column = (i < BORDER_COLUMNS)
                ? i
                : TILEMAP_COLUMNS - 1u - (i - BORDER_COLUMNS);
            const uint16_t entry =
                ppu->vram[(tilemap + row * TILEMAP_COLUMNS + column) & 0x7fffu];
            const unsigned palette = ((entry >> 10) & 7u) * depth;
            for (unsigned y = 0; y < 8u; y++) {
                for (unsigned x = 0; x < 8u; x++) {
                    const unsigned value = BorderTilePixel(
                        ppu, chars, entry & 0x3ffu, bpp, x, y);
                    if (!value)
                        continue;
                    const int here = (int)(palette + value);
                    if (colour < 0)
                        colour = here;
                    else if (colour != here)
                        return false;
                }
            }
        }
    }
    return true;
}

/* Stands still, no map loaded. */
static bool IsSplashScreen(const Ppu *ppu) {
    if (PPU_mode(ppu) != 1 || ppu->screenEnabled[1])
        return false;
    for (unsigned bg = 0; bg < 3u; bg++) {
        if (ppu->hScroll[bg] || ppu->vScroll[bg])
            return false;
        if ((ppu->screenEnabled[0] & (1u << bg)) && !BordersAreFlat(ppu, bg))
            return false;
    }
    return true;
}

static bool IsNightScene(const Ppu *ppu) {
    return ppu && PPU_mode(ppu) == 1 && ppu->bgTileAdr == 0x6622u &&
           (unsigned)PPU_bgTilemapAdr(ppu, TREE_LAYER) == 0x1800u;
}

/* A painting down to its first moving band. */
static unsigned StaticBackdropLines(void) {
    static uint16_t h[LUFIA2_PPU_VISIBLE_LINES];
    static uint16_t v[LUFIA2_PPU_VISIBLE_LINES];
    static uint8_t mosaic[LUFIA2_PPU_VISIBLE_LINES];
    const unsigned override_lines = SkyLineOverride();

    if (override_lines)
        return override_lines;
    if (!Lufia2CapturePpuRasterEffects(
            BACKDROP_LAYER, h, v, mosaic, LUFIA2_PPU_VISIBLE_LINES)) {
        return LUFIA2_PPU_VISIBLE_LINES;
    }
    for (unsigned y = 1; y < LUFIA2_PPU_VISIBLE_LINES; y++) {
        if (h[y] != h[0] || v[y] != v[0])
            return y;
    }
    return LUFIA2_PPU_VISIBLE_LINES;
}

static void BorrowWindow(Ppu *ppu) {
    if (!s_state.window_borrowed) {
        s_state.window_left = ppu->window1left;
        s_state.window_right = ppu->window1right;
        s_state.window_borrowed = true;
    }
    ppu->window1left = 0;
    ppu->window1right = (uint8_t)(NATIVE_WIDTH - 1);
}

/* The guest never re-enables it, so this must be reversible. */
static void BorrowTreeLayer(Ppu *ppu) {
    s_state.layer_borrowed = true;
    s_state.trees_active = true;
    ppu->screenEnabled[0] = (uint8_t)(ppu->screenEnabled[0] & ~TREE_BG);
}

void Lufia2IntroWidescreenRelease(Ppu *ppu);

bool Lufia2IntroWidescreenPrepare(
    Ppu *ppu, uint8_t runtime_map, unsigned margin) {
    s_state.active = false;
    s_state.trees_active = false;

    const uint8_t wanted = RequestedLayers();
    if (!ppu || !wanted || !margin) {
        Lufia2IntroWidescreenRelease(ppu);
        return false;
    }
    if (runtime_map == NO_MAP && IsSplashScreen(ppu)) {
        Lufia2IntroWidescreenRelease(ppu);
        PpuSetExtraSpace(ppu, (uint8_t)margin);
        PpuSetWidescreenLayerMask(
            ppu, (uint8_t)(ppu->screenEnabled[0] & SCENE_BG_MASK));
        return true;
    }
    if (runtime_map != INTRO_MAP || !IsNightScene(ppu)) {
        /* Declining is also how the scene ends. */
        Lufia2IntroWidescreenRelease(ppu);
        return false;
    }

    /* From the request: borrowing the bit clears it. */
    const bool wants_trees = (wanted & TREE_BG) != 0u;
    const uint8_t ppu_layers =
        (uint8_t)(ppu->screenEnabled[0] & wanted & ~TREE_BG);
    if (!ppu_layers && !wants_trees) {
        Lufia2IntroWidescreenRelease(ppu);
        return false;
    }

    if (wants_trees)
        BorrowTreeLayer(ppu);
    BorrowWindow(ppu);

    const unsigned static_lines = StaticBackdropLines();
    PpuSetExtraSpace(ppu, (uint8_t)margin);
    PpuSetWidescreenLayerMask(ppu, (uint8_t)(ppu_layers | BACKDROP_BG));
    PpuSetWidescreenLayerClampBand(
        ppu, BACKDROP_LAYER, 0,
        (uint8_t)(static_lines > 255u ? 255u : static_lines));
    PpuSetWidescreenWindowExpansion(
        ppu, (uint8_t)(ppu->screenEnabled[0] & SCENE_BG_MASK), WINDOW_MASK);

    if (static_lines != s_state.reported_static_lines) {
        fprintf(stderr,
            "[intro] widescreen: ppu layers $%02X, host layers $%02X, "
            "painting holds lines 0-%u\n",
            (unsigned)ppu_layers, (unsigned)(wants_trees ? TREE_BG : 0u),
            static_lines ? static_lines - 1u : 0u);
        s_state.reported_static_lines = static_lines;
    }
    s_state.active = true;
    return true;
}

void Lufia2IntroWidescreenObserve(const Ppu *ppu) {
    if (!s_state.trees_active)
        return;
    s_state.lines = Lufia2CapturePpuRasterEffects(
        TREE_LAYER, s_state.line_scroll_x, s_state.line_scroll_y,
        s_state.line_mosaic, LUFIA2_PPU_VISIBLE_LINES)
        ? LUFIA2_PPU_VISIBLE_LINES
        : 0u;
    Lufia2IntroTreesObserve(
        ppu, TREE_LAYER, s_state.line_scroll_x, s_state.lines);
}

void Lufia2IntroWidescreenPaint(
    const Ppu *ppu, uint8_t *frame, size_t width, size_t height,
    unsigned margin) {
    if (!s_state.active)
        return;
    Lufia2IntroMarginsApply(
        frame, width, height, margin,
        (ppu && !PPU_forcedBlank(ppu)) ? (unsigned)PPU_brightness(ppu) : 0u);
    if (s_state.trees_active) {
        Lufia2IntroTreesPaint(
            ppu, TREE_LAYER, frame, width, height, margin,
            s_state.line_scroll_x, s_state.lines);
    }
}

void Lufia2IntroWidescreenRelease(Ppu *ppu) {
    if (!ppu || (!s_state.layer_borrowed && !s_state.window_borrowed))
        return;
    if (s_state.layer_borrowed) {
        ppu->screenEnabled[0] = (uint8_t)(ppu->screenEnabled[0] | TREE_BG);
        s_state.layer_borrowed = false;
    }
    if (s_state.window_borrowed) {
        ppu->window1left = s_state.window_left;
        ppu->window1right = s_state.window_right;
        s_state.window_borrowed = false;
    }
    Lufia2IntroTreesReset();
    s_state.reported_static_lines = 0;
    fprintf(stderr, "[intro] widescreen: scene over, layers returned\n");
}
