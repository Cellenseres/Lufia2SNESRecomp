/* Converts the live per-line PPU history into a portable frame capture. */

#include "lufia2_runtime.h"

#include <stddef.h>
#include <string.h>

#include "snes/ppu.h"

#define PPU_RAW_OFFSET(member) \
    (offsetof(Ppu, member) - offsetof(Ppu, inidisp))

_Static_assert(PPU_RAW_OFFSET(setini) == 13u,
               "portable predicate SETINI offset must match Ppu snapshot");
_Static_assert(PPU_RAW_OFFSET(bgmode) == 4u &&
                   PPU_RAW_OFFSET(mosaic) == 5u &&
                   PPU_RAW_OFFSET(bgXsc) == 6u &&
                   PPU_RAW_OFFSET(bgTileAdr) == 10u,
               "portable predicate BG offsets must match Ppu snapshot");
_Static_assert(PPU_RAW_OFFSET(hScroll) == 14u &&
                   PPU_RAW_OFFSET(vScroll) == 22u,
               "portable predicate scroll offsets must match Ppu snapshot");
_Static_assert(PPU_RAW_OFFSET(m7sel) == 12u &&
                   PPU_RAW_OFFSET(m7matrix) == 30u,
               "portable Mode 7 offsets must match Ppu snapshot");
_Static_assert(PPU_RAW_OFFSET(fixedColor) == 46u &&
                   PPU_RAW_OFFSET(windowsel) == 48u &&
                   PPU_RAW_OFFSET(window1left) == 52u &&
                   PPU_RAW_OFFSET(screenEnabled) == 58u &&
                   PPU_RAW_OFFSET(screenWindowed) == 60u &&
                   PPU_RAW_OFFSET(cgadsub) == 62u &&
                   PPU_RAW_OFFSET(cgwsel) == 63u,
               "portable compositor offsets must match Ppu snapshot");
_Static_assert(PPU_RAW_OFFSET(obsel) == 1u &&
                   PPU_RAW_OFFSET(oamaddl) == 2u &&
                   PPU_RAW_OFFSET(oamaddh) == 3u,
               "portable OBJ offsets must match Ppu snapshot");

extern Ppu *g_ppu;

_Static_assert(sizeof(g_ppu->oam) ==
                   SNES_PPU_OAM_WORDS * sizeof(uint16_t) &&
               sizeof(g_ppu->highOam) == SNES_PPU_HIGH_OAM_BYTES,
               "portable OAM tables must match the emulator");

static void CaptureBandFromRegs(SnesPpuRasterBand *band,
                                const uint8_t *regs,
                                const Ppu *live) {
    static Ppu scratch;
    Ppu *const p = &scratch;

    memcpy(&scratch.inidisp, regs, PPU_SAVESTATE_REGS_SIZE);
    memset(band, 0, sizeof *band);
    _Static_assert(SNES_PPU_REG_BLOCK_BYTES == PPU_SAVESTATE_REGS_SIZE,
                   "portable register block must match Ppu snapshot");
    memcpy(band->regs, regs, SNES_PPU_REG_BLOCK_BYTES);
    band->bg_mode = (uint8_t)PPU_mode(p);
    band->bg3_priority = PPU_bg3priority(p) != 0;
    band->main_enable = p->screenEnabled[0];
    band->sub_enable = p->screenEnabled[1];
    band->main_window_enable = p->screenWindowed[0];
    band->sub_window_enable = p->screenWindowed[1];
    band->brightness = (uint8_t)PPU_brightness(p);
    band->forced_blank = PPU_forcedBlank(p) != 0;
    band->cgwsel = p->cgwsel;
    band->cgadsub = p->cgadsub;
    band->fixed_colour = p->fixedColor;
    band->mosaic = p->mosaic;
    band->window_sel = p->windowsel;
    band->window1_left = p->window1left;
    band->window1_right = p->window1right;
    band->window2_left = p->window2left;
    band->window2_right = p->window2right;

    for (unsigned bg = 0; bg < SNES_PPU_BG_COUNT; bg++) {
        SnesPpuBgState *st = &band->bg[bg];
        st->tilemap_word_addr = (uint16_t)PPU_bgTilemapAdr(p, bg);
        st->char_word_addr = (uint16_t)PPU_bgTileAdr(p, bg);
        st->h_scroll = p->hScroll[bg];
        st->v_scroll = p->vScroll[bg];
        st->wide = PPU_bgTilemapWider(p, bg) != 0;
        st->tall = PPU_bgTilemapHigher(p, bg) != 0;
        st->big_tiles = PPU_bigTiles(p, bg) != 0;
        st->bpp = (band->bg_mode == 1 && bg == 2) ? 2u : 4u;
        st->margin_left = (uint8_t)PpuWidescreenLayerExtra(
            live, bg, 0, (int)live->extraLeftCur);
        st->margin_right = (uint8_t)PpuWidescreenLayerExtra(
            live, bg, 0, (int)live->extraRightCur);
        st->margin_repeats = (live->wsLayerRepeat & (1u << bg)) != 0;
        if (st->margin_repeats) {
            st->margin_left = (uint8_t)live->extraLeftCur;
            st->margin_right = (uint8_t)live->extraRightCur;
        }
    }
}

static bool BandsIdentical(const SnesPpuRasterBand *a,
                           const SnesPpuRasterBand *b) {
    return memcmp((const char *)a + offsetof(SnesPpuRasterBand, bg_mode),
                  (const char *)b + offsetof(SnesPpuRasterBand, bg_mode),
                  sizeof *a - offsetof(SnesPpuRasterBand, bg_mode)) == 0;
}

bool Lufia2CapturePpuRasterEffects(
    unsigned bg,
    uint16_t *h_scroll,
    uint16_t *v_scroll,
    uint8_t *mosaic,
    size_t line_count) {
    const uint8_t *rows = NULL;
    size_t stride = 0;
    if (bg >= 4u || !h_scroll || !v_scroll || !mosaic ||
        line_count > LUFIA2_PPU_VISIBLE_LINES ||
        !Lufia2PpuRasterHistory(&rows, &stride) || !rows ||
        stride < PPU_SAVESTATE_REGS_SIZE) {
        return false;
    }

    for (size_t y = 0; y < line_count; y++) {
        const uint8_t *regs = rows + y * stride;
        memcpy(
            &h_scroll[y], regs + PPU_RAW_OFFSET(hScroll) + bg * 2u,
            sizeof(h_scroll[y]));
        memcpy(
            &v_scroll[y], regs + PPU_RAW_OFFSET(vScroll) + bg * 2u,
            sizeof(v_scroll[y]));
        mosaic[y] = regs[PPU_RAW_OFFSET(mosaic)];
    }
    return true;
}

bool Lufia2CapturePpuFrame(SnesPpuFrameCapture *out,
                           unsigned canvas_width,
                           unsigned canvas_extra) {
    static SnesPpuRasterBand bands[SNES_PPU_MAX_BANDS];
    SnesPpuRasterBand line_band;
    const uint8_t *previous = NULL;
    unsigned count = 0;

    if (!out || !g_ppu)
        return false;
    for (unsigned y = 0; y < LUFIA2_PPU_VISIBLE_LINES; y++) {
        const uint8_t *regs = Lufia2LineRegisters(y);
        if (!regs)
            return false;
        if (count && previous &&
            memcmp(previous, regs, PPU_SAVESTATE_REGS_SIZE) == 0) {
            bands[count - 1u].y_end = (uint16_t)(y + 1u);
            continue;
        }
        previous = regs;
        CaptureBandFromRegs(&line_band, regs, g_ppu);
        if (count && BandsIdentical(&bands[count - 1u], &line_band)) {
            bands[count - 1u].y_end = (uint16_t)(y + 1u);
            continue;
        }
        if (count == SNES_PPU_MAX_BANDS)
            return false;
        line_band.y_begin = (uint16_t)y;
        line_band.y_end = (uint16_t)(y + 1u);
        bands[count++] = line_band;
    }

    memset(out, 0, sizeof *out);
    out->vram = g_ppu->vram;
    out->cgram = g_ppu->cgram;
    out->oam = g_ppu->oam;
    out->high_oam = g_ppu->highOam;
    out->bands = bands;
    out->band_count = count;
    out->native_width = 256u;
    out->canvas_width = (uint16_t)canvas_width;
    out->canvas_extra = (uint16_t)canvas_extra;
    out->visible_height = LUFIA2_PPU_VISIBLE_LINES;
    out->raster_memory_flags = Lufia2PpuRasterMemoryFlags();

    out->layout.extra_left_right = g_ppu->extraLeftRight;
    out->layout.extra_left_cur = g_ppu->extraLeftCur;
    out->layout.extra_right_cur = g_ppu->extraRightCur;
    out->layout.layer_widen_mask = g_ppu->wsLayerWidenMask;
    out->layout.layer_clamp = g_ppu->wsLayerClamp;
    out->layout.layer_mirror = g_ppu->wsLayerMirror;
    out->layout.layer_repeat = g_ppu->wsLayerRepeat;
    out->layout.bg3_widen_y = g_ppu->wsBg3WidenY;
    out->layout.window_expand_layers = g_ppu->wsWindowExpandLayers;
    out->layout.window_expand_windows = g_ppu->wsWindowExpandWindows;
    out->layout.render_flags = g_ppu->renderFlags;
    out->layout.hud_split_height = g_ppu->wsHudSplitHeight;
    out->layout.hud_left_end = g_ppu->wsHudLeftEnd;
    out->layout.hud_right_start = g_ppu->wsHudRightStart;
    out->layout.hud_oam_first_slot = g_ppu->wsHudOamFirstSlot;
    out->layout.hud_oam_slots = g_ppu->wsHudOamSlots;
    out->layout.hud_oam_height = g_ppu->wsHudOamHeight;
    out->layout.hud_oam_first_slot2 = g_ppu->wsHudOamFirstSlot2;
    out->layout.hud_oam_slots2 = g_ppu->wsHudOamSlots2;
    out->layout.oam_left_hint_strict = g_ppu->wsOamLeftHintStrict;
    out->layout.oam_right_hint_strict = g_ppu->wsOamRightHintStrict;
    return true;
}

#undef PPU_RAW_OFFSET
