/* Test the production PPU with deterministic art. */
#include <assert.h>
#include "lufia2_battle_effects.h"
#include "lufia2_margin_assets.h"
#include "lufia2_video_handoff.h"
#include "snes/snes.h"
#include "ppu.c"

uint8_t g_ram[0x20000];
Snes *g_snes;
unsigned char g_snesrecomp_last_hdmaen;
int snes_frame_counter;
void ppu_draw_whole_line_legacy(Ppu *ppu, int line) {
    (void)ppu;
    (void)line;
    abort(); /* Require the production fast renderer. */
}

bool Lufia2MarginAssetFind(Lufia2MarginScene scene, uint8_t id,
                         SnesRecompMarginAsset *asset) {
    (void)scene;
    (void)id;
    memset(asset, 0, sizeof *asset);
    asset->canvas_width = 288;
    asset->canvas_height = 224;
    asset->center_x = 24;
    asset->center_width = 240;
    return true;
}

bool snesrecomp_margin_asset_composite_argb8888(
    const SnesRecompMarginAsset *asset,
    const SnesRecompMarginComposite *destination) {
    for (unsigned y = 0; y < 224; ++y) {
        for (unsigned x = 0; x < 288; ++x) {
            if (x >= 24 && x < 264) continue;
            uint32_t colour = 0xff000000u | ((x % 32u) << 19);
            memcpy(destination->pixels + y * destination->pitch + x * 4,
                   &colour, 4);
        }
    }
    (void)asset;
    return true;
}

int main(void) {
    static Ppu ppu;
    static uint32_t frame[288 * 224];
    ppu.renderBuffer = (uint8_t *)frame;
    ppu.renderPitch = 288 * 4;
    ppu.inidisp = 15;
    ppu.bgmode = 1;
    ppu.bgTileAdr = 2;
    ppu.screenEnabled[0] = 1;
    ppu.vScroll[0] = 1023;
    ppu.extraLeftCur = ppu.extraRightCur = ppu.extraLeftRight = 16;
    for (unsigned i = 0; i < 63; ++i) {
        ppu.brightnessMult[i] = (uint8_t)((i > 31 ? 31 : i) * 255 / 31);
        ppu.brightnessMultHalf[i] = (uint8_t)((i / 2) * 255 / 31);
    }
    for (unsigned i = 0; i < 1024; ++i) ppu.vram[i] = 0x0801;
    for (unsigned i = 0; i < 8; ++i) ppu.vram[0x2010 + i] = 0xff;
    ppu.cgram[33] = 31;
    g_ram[0x67] = 0x22;
    g_ram[0x68] = 0xc5;
    g_ram[0x69] = 0x8d;
    g_ram[0x6a] = 0x85;
    g_ram[0x6b] = 0x60;
    g_ram[0x583] = 15;
    assert(Lufia2BattleEffectsPrepare(&ppu, true, 288, 224));
    unsigned comparisons = 0;
    for (int shift = -8; shift <= 8; ++shift) {
        ppu.hScroll[0] = (uint16_t)(shift & 1023);
        for (unsigned math = 0; math < 4; ++math) {
            ppu.cgadsub = math == 0 ? 0 : (uint8_t)(1 | (math == 2 ? 0x80 : 0) |
                                                   (math == 3 ? 0x40 : 0));
            ppu.fixedColor = 16;
            PpuDrawWholeLine(&ppu, 1);
            for (int x = -16; x < 272; ++x) {
                int source = x + 16 + shift;
                if (source < 0) source = 0;
                if (source > 287) source = 287;
                unsigned red = source >= 24 && source < 264 ? 31 : source % 32;
                if (math == 1 || math == 3) red += 16;
                if (math == 2) red = red >= 16 ? red - 16 : 0;
                if (math == 3) red /= 2;
                if (red > 31) red = 31;
                assert(frame[x + 16] == (red * 255 / 31) << 16);
                ++comparisons;
            }
        }
    }
    /* Index-zero art remains opaque on the subscreen. */
    ppu.screenEnabled[1] = 1;
    ppu.cgwsel = 2;
    ppu.cgadsub = 0x41;
    ppu.hScroll[0] = 0;
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[7] == (7u * 255 / 31) << 16);
    /* Sprites retain their palette colours. */
    assert(Lufia2BattleEffectsColour(&ppu, 0xe621, kPpuExtraLeftRight - 9, false) == 31);
    ppu.screenEnabled[1] = 0;
    ppu.cgwsel = 0;
    ppu.cgadsub = 0;
    ppu.lineHasSprites = true;
    ppu.screenEnabled[0] |= 16;
    for (unsigned i = 0; i < kPpuBufWidth; ++i) ppu.objBuffer.data[i] = 0x0500;
    ppu.objBuffer.data[kPpuExtraLeftRight - 9] = 0xa681;
    ppu.objBuffer.data[kPpuExtraLeftRight + 5] = 0xa681;
    ppu.cgram[129] = 31u << 5;
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[21] == 0xff00u); /* Original enemies keep their priority. */
    assert(frame[7] == (7u * 255 / 31) << 16); /* Wrapped OBJ stays offscreen. */
    for (unsigned i = 0; i < 1024; ++i) ppu.vram[i] |= 0x2000;
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[21] == (21u * 255 / 31) << 16); /* High-priority BG beats OBJ. */
    for (unsigned i = 0; i < 1024; ++i) ppu.vram[i] &= ~0x2000u;
    ppu.lineHasSprites = false;
    ppu.screenEnabled[0] = 1;
    ppu.mosaic = 0x31;
    for (unsigned i = 0; i < 256; ++i) ppu.mosaicModulo[i] = i - i % 4;
    PpuDrawWholeLine(&ppu, 4);
    for (int x = -16; x < 272; ++x) {
        int sample = x - ((x % 4 + 4) % 4) + 16;
        unsigned red = sample >= 24 && sample < 264 ? 31u : sample % 32;
        assert(frame[3 * 288 + x + 16] == (red * 255 / 31) << 16);
        ++comparisons;
    }
    ppu.mosaic = 0;
    /* Apply layer windows before colour math. */
    ppu.windowsel = 2;
    ppu.screenWindowed[0] = 1;
    ppu.window1left = 32;
    ppu.window1right = 224;
    PpuSetWidescreenWindowExpansion(&ppu, 0x3f, 3);
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[16 + 100] == 0);
    assert(frame[7] == (7u * 255 / 31) << 16);
    ppu.window1left = 0;
    ppu.window1right = 255;
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[7] == 0); /* A full-screen mask includes both margins. */
    ppu.screenWindowed[0] = 0;
    for (unsigned i = 32; i < 64; ++i) ppu.cgram[i] = 0;
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[7] == 0 && frame[116] == 0);
    for (unsigned i = 32; i < 64; ++i) ppu.cgram[i] = 0x7fff;
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[7] == 0xffffff && frame[116] == 0xffffff);
    ppu.cgram[32] = 0;
    ppu.cgram[33] = 31;
    ppu.cgram[1] = 31u << 5;
    ppu.bgTileAdr = 0x42;
    ppu.bgXsc[1] = 0x5c;
    ppu.screenEnabled[0] = 3;
    for (unsigned i = 0; i < 1024; ++i) ppu.vram[0x5c00 + i] = 0x2001;
    for (unsigned i = 0; i < 8; ++i) ppu.vram[0x4010 + i] = 0xff;
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[116] == 0xff00u); /* BG2 HUD remains above the background. */
    assert(frame[7] == (7u * 255 / 31) << 16); /* HUD tilemap cannot wrap. */
    assert(frame[280] == (24u * 255 / 31) << 16);
    /* A replacement effect map can span the viewport. */
    ppu.bgXsc[1] = 0x58;
    for (unsigned i = 0; i < 1024; ++i) ppu.vram[0x5800 + i] = 0x2001;
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[7] == 0xff00u && frame[280] == 0xff00u);
    /* Name windows retain their native width. */
    ppu.bgXsc[1] = 0x5c;
    ppu.screenEnabled[0] = 1;
    PpuSetWidescreenWindowExpansion(&ppu, 0x3f, 2);
    ppu.windowsel = 2u << 20;
    ppu.cgwsel = 0x10;
    ppu.cgadsub = 0x41;
    ppu.fixedColor = 0;
    ppu.window1left = 64;
    ppu.window1right = 160;
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[16 + 63] == 0xff0000 && frame[16 + 64] == (15u * 255 / 31) << 16);
    assert(frame[16 + 160] == (15u * 255 / 31) << 16 && frame[16 + 161] == 0xff0000);
    assert(frame[7] == (7u * 255 / 31) << 16);
    ppu.window1left = ppu.window1right = 0;
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[7] == (7u * 255 / 31) << 16 && frame[16] == (16u * 255 / 31) << 16);
    /* Neither name window may expand twice. */
    ppu.windowsel = 8u << 20;
    ppu.window2left = 64;
    ppu.window2right = 160;
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[16 + 63] == 0xff0000 && frame[16 + 64] == (15u * 255 / 31) << 16);
    assert(frame[16 + 160] == (15u * 255 / 31) << 16 && frame[16 + 161] == 0xff0000);
    assert(frame[7] == (7u * 255 / 31) << 16);
    ppu.window2left = ppu.window2right = 0;
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[7] == (7u * 255 / 31) << 16 && frame[16] == (16u * 255 / 31) << 16);
    /* Scene clipping must preserve both margins. */
    ppu.windowsel = 8u << 20;
    ppu.cgwsel = 0x40;
    ppu.cgadsub = 0;
    ppu.window2left = 5;
    ppu.window2right = 250;
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[1] == (1u * 255 / 31) << 16 && frame[287] == 0xff0000);
    /* Scene crops and name windows differ. */
    for (unsigned inset = 0; inset <= 16; ++inset) {
        ppu.window2left = (uint8_t)inset;
        ppu.window2right = (uint8_t)(255u - inset);
        PpuDrawWholeLine(&ppu, 1);
        assert(frame[1] == (1u * 255 / 31) << 16 && frame[287] == 0xff0000);
        comparisons += 2;
    }
    ppu.windowsel = 2u << 20;
    for (unsigned inset = 0; inset <= 16; ++inset) {
        ppu.window1left = (uint8_t)inset;
        ppu.window1right = (uint8_t)(255u - inset);
        PpuDrawWholeLine(&ppu, 1);
        assert(frame[1] == (1u * 255 / 31) << 16 && frame[287] == 0xff0000);
        comparisons += 2;
    }
    /* An inverted BG1 crop must preserve both margins. */
    ppu.windowsel = 3u;
    ppu.screenWindowed[0] = 1;
    ppu.cgwsel = ppu.cgadsub = 0;
    ppu.window1left = 8;
    ppu.window1right = 247;
    PpuSetWidescreenWindowExpansion(&ppu, 0x3f, 2);
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[7] == (7u * 255 / 31) << 16);
    assert(frame[280] == (24u * 255 / 31) << 16);
    for (unsigned inset = 0; inset <= 16; ++inset) {
        ppu.window1left = (uint8_t)inset;
        ppu.window1right = (uint8_t)(255u - inset);
        PpuDrawWholeLine(&ppu, 1);
        assert(frame[7] == (7u * 255 / 31) << 16);
        assert(frame[280] == (24u * 255 / 31) << 16);
        comparisons += 2;
    }
    ppu.screenWindowed[0] = 0;
    /* The handoff must preserve live heat shimmer. */
    static uint32_t saved_frame[288 * 224];
    static uint32_t expected_frame[288 * 224];
    Lufia2VideoHandoff handoff;
    uint16_t scroll_x[224], scroll_y[224];
    uint8_t mosaic_size[224];
    Lufia2VideoHandoffReset(&handoff);
    for (unsigned y = 0; y < 224; ++y) {
        scroll_x[y] = (uint16_t)(y % 8);
        scroll_y[y] = 1023;
        mosaic_size[y] = 1;
        ppu.hScroll[0] = scroll_x[y];
        PpuDrawWholeLine(&ppu, y + 1);
    }
    memcpy(expected_frame, frame, sizeof frame);
    Lufia2VideoHandoffObserve(&handoff,
        LUFIA2_VIDEO_HANDOFF_SCENE_WIDE_EFFECTS, 15, 1, 0, 1023);
    Lufia2VideoHandoffObserveRaster(&handoff,
        scroll_x, scroll_y, mosaic_size, 224);
    Lufia2VideoHandoffApply(&handoff, (uint8_t *)frame,
        (uint8_t *)saved_frame, 288, 224, 16, 0);
    assert(!handoff.current_raster_effect);
    assert(memcmp(frame, expected_frame, sizeof frame) == 0);
    comparisons += 288 * 224;
    for (unsigned size = 1; size <= 16; ++size)
        for (int x = -272; x < 528; ++x) {
            const int source = Lufia2BattleEffectsMosaic(x, size);
            assert(source <= x && x - source < (int)size && source % (int)size == 0);
            ++comparisons;
        }
    g_ram[0x6a] = 0;
    assert(!Lufia2BattleEffectsPrepare(&ppu, true, 288, 224));
    Lufia2BattleEffectsShutdown();
    printf("Battle effects PASS: %u source/colour/mosaic comparisons\n", comparisons);
    return 0;
}
