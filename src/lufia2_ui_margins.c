#include "lufia2_ui_margins.h"

#include <stdbool.h>
#include <stdint.h>

enum {
    UI_FIRST_LINE = 1,
    UI_LAST_LINE = 224,
    UI_BG2 = 1,
    UI_BG3 = 2,
    UI_TILE_PRIORITY = 0x2000,
    UI_TILE_PALETTE = 0x1c00,
    /* $82:9984 marks the selected member. */
    UI_MENU_HIGHLIGHT = 2 << 10,
    /* $82:8069 pattern, $82:80CA resets. */
    UI_MENU_BACKDROP = 3 << 10,
    UI_SIDE_REPEAT = -1,
    UI_SIDE_DROP = -2,
};

/* Source column per margin, or a UI_SIDE_* value. */
typedef struct UiLinePlan {
    int16_t left;
    int16_t right;
} UiLinePlan;

typedef struct UiBand {
    int first;
    int last;
    UiLinePlan plan;
} UiBand;

typedef int16_t (*UiSidePlanner)(const Ppu *ppu, int line, int x0, int width);

static uint16_t TilemapEntry(const Ppu *ppu, int layer, uint32_t x,
                             uint32_t y) {
    uint32_t address = (uint32_t)PPU_bgTilemapAdr(ppu, layer) +
                       (((y >> 3) & 31u) << 5) + ((x >> 3) & 31u);
    if ((y & 0x100u) && PPU_bgTilemapHigher(ppu, layer))
        address += PPU_bgTilemapWider(ppu, layer) ? 0x800u : 0x400u;
    if ((x & 0x100u) && PPU_bgTilemapWider(ppu, layer))
        address += 0x400u;
    return ppu->vram[address & 0x7fffu];
}

static uint32_t MapX(const Ppu *ppu, int layer, int screen_x) {
    return (uint32_t)(screen_x + ppu->hScroll[layer]);
}

/* Visible lines are numbered from 1. */
static uint32_t MapY(const Ppu *ppu, int layer, int line) {
    return (uint32_t)(line + ppu->vScroll[layer]);
}

static uint16_t ScreenEntry(const Ppu *ppu, int layer, int x, int line) {
    return TilemapEntry(ppu, layer, MapX(ppu, layer, x),
                        MapY(ppu, layer, line));
}

static int NextTileColumn(const Ppu *ppu, int layer, int x) {
    return x + 8 - (int)(MapX(ppu, layer, x) & 7u);
}

static bool RunHasPriority(const Ppu *ppu, int line, int x0, int x1) {
    for (int x = x0; x < x1; x = NextTileColumn(ppu, UI_BG3, x)) {
        if (ScreenEntry(ppu, UI_BG3, x, line) & UI_TILE_PRIORITY)
            return true;
    }
    return false;
}

static bool RunHasHighlight(const Ppu *ppu, int line, int x0, int x1) {
    for (int x = x0; x < x1; x = NextTileColumn(ppu, UI_BG2, x)) {
        const uint16_t entry = ScreenEntry(ppu, UI_BG2, x, line);
        if ((entry & UI_TILE_PALETTE) == UI_MENU_HIGHLIGHT)
            return true;
    }
    return false;
}

/* Same tiles as the run at x0, backdrop palette only. */
static bool RunIsCleanCopy(const Ppu *ppu, int line, int x0, int source,
                           int width) {
    for (int x = x0; x < x0 + width; x = NextTileColumn(ppu, UI_BG2, x)) {
        const uint16_t entry = ScreenEntry(ppu, UI_BG2, x, line);
        const uint16_t copy = ScreenEntry(ppu, UI_BG2, source + x - x0, line);
        if ((copy & UI_TILE_PALETTE) != UI_MENU_BACKDROP ||
            (copy & ~UI_TILE_PALETTE) != (entry & ~UI_TILE_PALETTE))
            return false;
    }
    return true;
}

static int16_t MenuSide(const Ppu *ppu, int line, int x0, int width) {
    if (width <= 0 || !RunHasHighlight(ppu, line, x0, x0 + width))
        return UI_SIDE_REPEAT;
    for (int shift = 8; shift < kPpuXPixels; shift += 8) {
        if (x0 + shift + width <= kPpuXPixels &&
            RunIsCleanCopy(ppu, line, x0, x0 + shift, width))
            return (int16_t)(x0 + shift);
        if (x0 - shift >= 0 &&
            RunIsCleanCopy(ppu, line, x0, x0 - shift, width))
            return (int16_t)(x0 - shift);
    }
    return UI_SIDE_REPEAT;
}

static int16_t MapSide(const Ppu *ppu, int line, int x0, int width) {
    if (width <= 0 || !RunHasPriority(ppu, line, x0, x0 + width))
        return UI_SIDE_REPEAT;
    return UI_SIDE_DROP;
}

static bool PlanIsRepeat(UiLinePlan plan) {
    return plan.left == UI_SIDE_REPEAT && plan.right == UI_SIDE_REPEAT;
}

static bool SamePlan(UiLinePlan a, UiLinePlan b) {
    return a.left == b.left && a.right == b.right;
}

/* Lines sharing a tile row share a plan. */
static void BuildPlans(const Ppu *ppu, int layer, UiSidePlanner planner,
                       UiLinePlan *plans) {
    const int left = ppu->extraLeftCur;
    const int right = ppu->extraRightCur;
    uint32_t previous_row = UINT32_MAX;

    for (int line = UI_FIRST_LINE; line <= UI_LAST_LINE; line++) {
        const uint32_t row = MapY(ppu, layer, line) >> 3;
        if (row == previous_row) {
            plans[line] = plans[line - 1];
            continue;
        }
        previous_row = row;
        plans[line].left = planner(ppu, line, kPpuXPixels - left, left);
        plans[line].right = planner(ppu, line, 0, right);
    }
}

static void InstallBand(Ppu *ppu, int layer, uint8_t slot, UiBand band) {
    const int left = ppu->extraLeftCur;
    const int right = ppu->extraRightCur;
    PpuWsElasticSeg segs[3];
    uint8_t count = 0;

    if (left > 0 && band.plan.left != UI_SIDE_DROP) {
        const int src = band.plan.left == UI_SIDE_REPEAT
                            ? kPpuXPixels - left
                            : band.plan.left;
        segs[count++] = (PpuWsElasticSeg){
            (int16_t)src, (int16_t)(src + left), (int16_t)-left, 0};
    }
    segs[count++] = (PpuWsElasticSeg){0, kPpuXPixels, 0, kPpuXPixels};
    if (right > 0 && band.plan.right != UI_SIDE_DROP) {
        const int src =
            band.plan.right == UI_SIDE_REPEAT ? 0 : band.plan.right;
        segs[count++] = (PpuWsElasticSeg){
            (int16_t)src, (int16_t)(src + right), kPpuXPixels,
            (int16_t)(kPpuXPixels + right)};
    }
    PpuSetWidescreenLayerElasticBandSlot(ppu, slot, (uint8_t)layer,
                                         (uint8_t)band.first,
                                         (uint8_t)(band.last + 1), segs,
                                         count);
}

/* Out of slots: one marginless band, repeat around it. */
static int CollapseBands(UiBand *bands, int first_dirty, int last_dirty) {
    const UiLinePlan repeat = {UI_SIDE_REPEAT, UI_SIDE_REPEAT};
    const UiLinePlan drop = {UI_SIDE_DROP, UI_SIDE_DROP};
    int count = 0;

    if (first_dirty > UI_FIRST_LINE)
        bands[count++] = (UiBand){UI_FIRST_LINE, first_dirty - 1, repeat};
    bands[count++] = (UiBand){first_dirty, last_dirty, drop};
    if (last_dirty < UI_LAST_LINE)
        bands[count++] = (UiBand){last_dirty + 1, UI_LAST_LINE, repeat};
    return count;
}

/* Bands only apply where the layer does not repeat. */
static bool InstallPlans(Ppu *ppu, int layer, const UiLinePlan *plans) {
    UiBand bands[kPpuWsElasticBands];
    int count = 0;
    bool overflow = false;
    int first_dirty = -1;
    int last_dirty = -1;

    for (int line = UI_FIRST_LINE; line <= UI_LAST_LINE; line++) {
        const UiLinePlan plan = plans[line];
        if (!PlanIsRepeat(plan)) {
            if (first_dirty < 0)
                first_dirty = line;
            last_dirty = line;
        }
        if (overflow)
            continue;
        if (count > 0 && SamePlan(bands[count - 1].plan, plan))
            bands[count - 1].last = line;
        else if (count < kPpuWsElasticBands)
            bands[count++] = (UiBand){line, line, plan};
        else
            overflow = true;
    }

    if (first_dirty < 0)
        return false;
    if (overflow)
        count = CollapseBands(bands, first_dirty, last_dirty);
    PpuSetWidescreenLayerRepeat(
        ppu, (uint8_t)(ppu->wsLayerRepeat & ~(1u << layer)));
    for (int band = 0; band < count; band++)
        InstallBand(ppu, layer, (uint8_t)band, bands[band]);
    return true;
}

static bool LayerRepeats(const Ppu *ppu, int layer) {
    return ppu && PPU_mode(ppu) == 1 && !PPU_bigTiles(ppu, layer) &&
           (ppu->wsLayerRepeat & (1u << layer)) &&
           (ppu->extraLeftCur || ppu->extraRightCur);
}

void Lufia2UiMarginsMap(Ppu *ppu) {
    if (!LayerRepeats(ppu, UI_BG3))
        return;
    UiLinePlan plans[UI_LAST_LINE + 1];
    BuildPlans(ppu, UI_BG3, MapSide, plans);
    if (InstallPlans(ppu, UI_BG3, plans))
        PpuSetWidescreenBg3Widen(ppu, UI_FIRST_LINE);
}

void Lufia2UiMarginsMenu(Ppu *ppu) {
    if (!LayerRepeats(ppu, UI_BG2))
        return;
    UiLinePlan plans[UI_LAST_LINE + 1];
    BuildPlans(ppu, UI_BG2, MenuSide, plans);
    InstallPlans(ppu, UI_BG2, plans);
}
