/* Test the production PPU with deterministic art. */
#include <assert.h>
#include "lufia2_battle_effects.h"
#include "lufia2_battle_ui.h"
#include "lufia2_margin_assets.h"
#include "lufia2_video_handoff.h"
#include "snes/snes.h"
#include "ppu.c"

uint8_t g_ram[0x20000];
static unsigned art_width = 288;
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
    asset->canvas_width = (uint16_t)art_width;
    asset->canvas_height = 224;
    asset->center_x = (uint16_t)((art_width - 256) / 2 + 8);
    asset->center_width = 240;
    return true;
}

bool snesrecomp_margin_asset_composite_argb8888(
    const SnesRecompMarginAsset *asset,
    const SnesRecompMarginComposite *destination) {
    for (unsigned y = 0; y < 224; ++y) {
        for (unsigned x = 0; x < art_width; ++x) {
            if (x >= asset->center_x && x < asset->center_x + 240u) continue;
            uint32_t colour = 0xff000000u | ((x % 32u) << 19);
            memcpy(destination->pixels + y * destination->pitch + x * 4,
                   &colour, 4);
        }
    }
    (void)asset;
    return true;
}

static void SeedSprite(Ppu *ppu, unsigned slot, int x, uint16_t tile, bool large) {
    const unsigned high = ((unsigned)x >> 8 & 1u) | (large ? 2u : 0u);
    ppu->oam[slot * 2] = (uint16_t)((unsigned)x & 255u);
    ppu->oam[slot * 2 + 1] = tile;
    ppu->highOam[slot / 4] = (uint8_t)(
        (ppu->highOam[slot / 4] & ~(3u << ((slot % 4) * 2))) |
        (high << ((slot % 4) * 2)));
    for (unsigned byte = 0; byte < 4; ++byte)
        g_ram[0x100 + slot * 4 + byte] =
            (uint8_t)(ppu->oam[slot * 2 + byte / 2] >> ((byte % 2) * 8));
    g_ram[0x300 + slot / 4] = ppu->highOam[slot / 4];
}

static void DrawSpriteLine(Ppu *ppu) {
    for (unsigned i = 0; i < kPpuBufWidth; ++i)
        ppu->objBuffer.data[i] = 0x0500;
    ppu->lineHasSprites = ppu_evaluateSprites(ppu, 1);
    PpuDrawWholeLine(ppu, 1);
}

static unsigned TestEffectSprites(Ppu *ppu, uint32_t *frame) {
    static Ppu saved;
    static uint32_t native[256];
    saved = *ppu;
    ppu->screenEnabled[0] = 17;
    ppu->screenEnabled[1] = 0;
    ppu->screenWindowed[0] = ppu->windowsel = 0;
    ppu->cgadsub = ppu->cgwsel = ppu->mosaic = 0;
    ppu->hScroll[0] = 0;
    ppu->wsOamLeftHintStrict = ppu->wsOamRightHintStrict = 1;
    ppu->wsOamMotionGraceOn = 0;
    ppu->cgram[129] = 31u << 5;
    for (unsigned slot = 0; slot < 128; ++slot) ppu->oam[slot * 2] = 0xf000;
    memset(ppu->highOam, 0, sizeof ppu->highOam);
    /* Solid tiles expose every clipped pixel. */
    for (unsigned tile = 0; tile < 256; ++tile)
        for (unsigned row = 0; row < 8; ++row) {
            ppu->vram[0x6000 + tile * 16 + row] = 0xff;
            ppu->vram[0x6008 + tile * 16 + row] = 0;
        }
    unsigned comparisons = 0;
    for (unsigned group = 0; group < 3; ++group) {
        g_ram[0x15c1 + group] = 2;
        g_ram[0x15de + group * 4] = 1;
        g_ram[0x15db + group * 4] = 1;
        for (unsigned size = 8; size <= 64; size *= 2) {
            ppu->obsel = (uint8_t)(3 | (size == 32 ? 32 : size == 64 ? 64 : 0));
            for (unsigned flip = 0; flip < 4; ++flip)
                for (int x = -64; x < 272; x += 7) {
                    SeedSprite(ppu, 2, x, (uint16_t)(0x2001 | (flip << 14)), size != 8);
                    g_ram[0x1bec] = 0;
                    DrawSpriteLine(ppu);
                    memcpy(native, frame + 16, sizeof native);
                    g_ram[0x1bec] = 1;
                    assert(Lufia2BattleEffectsSprite(ppu, 2));
                    DrawSpriteLine(ppu);
                    assert(memcmp(native, frame + 16, sizeof native) == 0);
                    comparisons += 256;
                    for (int at = -16; at < 272; ++at) {
                        if (at >= 0 && at < 256) continue;
                        const unsigned red = (unsigned)(at + 16) % 32;
                        const uint32_t expected = at >= x && at < x + (int)size ?
                            0xff00u : (red * 255 / 31) << 16;
                        assert(frame[at + 16] == expected);
                        ++comparisons;
                    }
                }
        }
        g_ram[0x15db + group * 4] = 0;
        assert(!Lufia2BattleEffectsSprite(ppu, 2));
        g_ram[0x15de + group * 4] = 0;
    }
    ppu->obsel = 3;
    SeedSprite(ppu, 2, 252, 0x2001, false);
    g_ram[0x15c1] = 2;
    g_ram[0x15de] = g_ram[0x15db] = 1;
    DrawSpriteLine(ppu);
    assert(frame[272] == 0xff00u);
    /* Reject stale records and malformed groups. */
    g_ram[0x108] ^= 1;
    assert(!Lufia2BattleEffectsSprite(ppu, 2));
    DrawSpriteLine(ppu);
    assert(frame[272] == (16u * 255 / 31) << 16);
    g_ram[0x108] ^= 1;
    g_ram[0x300] ^= 1u << 4;
    assert(!Lufia2BattleEffectsSprite(ppu, 2));
    g_ram[0x300] ^= 1u << 4;
    g_ram[0x15de] = 127;
    assert(!Lufia2BattleEffectsSprite(ppu, 2));
    g_ram[0x15de] = 1;
    /* Native sprites overwrite effect ownership too. */
    SeedSprite(ppu, 0, 252, 0x2001, false);
    DrawSpriteLine(ppu);
    assert(frame[271] == 0xff00u);
    assert(frame[272] == (16u * 255 / 31) << 16);
    ppu->oam[0] = 0xf000;
    /* Scene windows retain finite sprite masks. */
    ppu->screenWindowed[0] = 16;
    ppu->windowsel = 3u << 16;
    ppu->window1left = 0;
    ppu->window1right = 255;
    DrawSpriteLine(ppu);
    assert(frame[272] == 0xff00u);
    ppu->window1left = 240;
    DrawSpriteLine(ppu);
    assert(frame[272] == (16u * 255 / 31) << 16);
    /* No ownership survives an empty scanline. */
    ppu->oam[4] = 0xf000;
    DrawSpriteLine(ppu);
    assert(!Lufia2BattleEffectsSpriteVisible(ppu, 256));
    *ppu = saved;
    g_ram[0x15de] = g_ram[0x15db] = g_ram[0x1bec] = 0;
    return comparisons;
}

static unsigned TestCulledSprites(Ppu *ppu) {
    static Ppu saved;
    static uint32_t frame[kPpuBufWidth * 224];
    saved = *ppu;
    ppu->screenEnabled[0] = 17;
    ppu->screenEnabled[1] = ppu->screenWindowed[0] = 0;
    ppu->windowsel = ppu->cgadsub = ppu->cgwsel = ppu->mosaic = 0;
    ppu->hScroll[0] = 0;
    ppu->obsel = 3;
    ppu->cgram[161] = 31u << 5;
    ppu->cgram[177] = 31;
    memset(g_ram + 0x54b7, 0, 64 * 0x2d);
    for (unsigned slot = 0; slot < 128; ++slot) ppu->oam[slot * 2] = 0xf000;
    for (unsigned tile = 0; tile < 256; ++tile)
        for (unsigned row = 0; row < 8; ++row) {
            ppu->vram[0x6000 + tile * 16 + row] = 0xff;
            ppu->vram[0x6008 + tile * 16 + row] = 0;
        }
    uint8_t *actor = g_ram + 0x54b7;
    actor[0] = actor[0x26] = actor[0x27] = actor[0x2a] = 1;
    actor[0x1d] = 16;
    actor[0x29] = 2;
    actor[0x28] = 4;
    g_ram[0x1bec] = 1;
    unsigned comparisons = 0;
    for (unsigned width = 352; width <= 800; width += 448) {
        art_width = width;
        ppu->renderBuffer = (uint8_t *)frame;
        ppu->renderPitch = width * 4;
        ppu->extraLeftCur = ppu->extraRightCur = ppu->extraLeftRight = (uint16_t)((width - 256) / 2);
        assert(Lufia2BattleEffectsPrepare(ppu, true, width, 224));
        for (int centre = -(int)ppu->extraLeftRight - 16; centre < 288 + ppu->extraLeftRight; centre += 3) {
            const uint16_t word = (uint16_t)centre;
            actor[0x1b] = (uint8_t)word;
            actor[0x1c] = (uint8_t)(word >> 8);
            DrawSpriteLine(ppu);
            for (int x = -(int)ppu->extraLeftRight; x < 256 + ppu->extraLeftRight; ++x) {
                if (x >= 0 && x < 256) {
                    assert(frame[x + ppu->extraLeftRight] != 0xff00u);
                } else {
                    const unsigned red = (unsigned)(x + ppu->extraLeftRight) % 32;
                    assert(frame[x + ppu->extraLeftRight] ==
                        (x >= centre - 16 && x < centre ? 0xff00u : (red * 255 / 31) << 16));
                }
                ++comparisons;
            }
        }
        actor[0x1b] = 32;
        actor[0x1c] = 1;
        memcpy(actor + 0x2d, actor, 0x2d);
        actor[0x2d + 0x29] = 3;
        actor[0x2d + 0x28] = 6;
        DrawSpriteLine(ppu);
        assert(frame[280 + ppu->extraLeftRight] == 0xff0000u);
        memset(actor + 0x2d, 0, 0x2d);
        actor[0x26] = 0;
        DrawSpriteLine(ppu);
        assert(frame[280 + ppu->extraLeftRight] ==
            (((280u + ppu->extraLeftRight) % 32) * 255 / 31) << 16);
        actor[0x26] = 1;
    }
    memset(g_ram + 0x54b7, 0, 64 * 0x2d);
    g_ram[0x1bec] = 0;
    *ppu = saved;
    art_width = 288;
    assert(Lufia2BattleEffectsPrepare(ppu, true, 288, 224));
    return comparisons;
}

static void SetHudPixel(Ppu *ppu, unsigned tile, unsigned x, unsigned y, unsigned colour) {
    for (unsigned plane = 0; plane < 4; ++plane) {
        uint16_t *word = ppu->vram + 0x4000 + tile * 16 + y + (plane / 2) * 8;
        const unsigned bit = 1u << (7u - x + (plane % 2) * 8);
        *word = (uint16_t)((*word & ~bit) | ((colour >> plane & 1u) ? bit : 0));
    }
}

static unsigned FramePixel(unsigned kind, unsigned x, unsigned y) {
    if (kind == 0) {
        if (y < 2 || x < 2) return 9;
        if (y == 2 || x == 2) return 7;
        return x == 3 ? 9 : 8;
    }
    if (kind == 1) return y < 2 ? 9 : y == 2 ? 7 : y == 3 ? 9 : 8;
    return x < 2 ? 9 : x == 2 ? 7 : x == 3 ? 9 : 8;
}

static void SeedPartyPanel(Ppu *ppu) {
    for (unsigned y = 0; y < 8; ++y)
        for (unsigned x = 0; x < 8; ++x) {
            const unsigned edge = x < 4 ? 9 : x == 4 ? 10 : x == 5 ? 9 : 8;
            const unsigned corner = y == 0 ? 15 : y == 1 || x < 4 ? 9 : y == 2 ? 10 : edge;
            SetHudPixel(ppu, 0x100, x, y, 0);
            SetHudPixel(ppu, 0x150, x, y, 8);
            SetHudPixel(ppu, 0x156, x, y, y == 0 ? 15 : y == 1 || y == 3 ? 9 : y == 2 ? 10 : 8);
            SetHudPixel(ppu, 0x158, x, y, y < 2 ? (y == 0 ? 15 : 9) : 10);
            SetHudPixel(ppu, 0x159, x, y, x == 2 || x == 4 ? 10 : x == 3 ? 9 : 8);
            SetHudPixel(ppu, 0x15a, x, y, y == 0 || x < 2 ? 15 : 9);
            SetHudPixel(ppu, 0x15b, x, y, corner);
            SetHudPixel(ppu, 0x15c, x, y, x < 2 ? 15 : 9);
            SetHudPixel(ppu, 0x15d, x, y, edge);
        }
    ppu->cgram[10] = 31u << 5;
    ppu->cgram[15] = 0;
    /* The execution panel has its own enclosing frame. */
    for (unsigned y = 0; y < 6; ++y)
        for (unsigned x = 0; x < 32; ++x) {
            const bool rim = y == 0 || y == 5;
            const unsigned flip = y == 5 ? 0x8000 : 0;
            unsigned tile = rim ? 0x2156 : 0x2150;
            if (x == 0 || x == 30) tile = rim ? 0x215a : 0x215c;
            if (x == 1 || x == 29) tile = rim ? 0x215b : 0x215d;
            if (x == 8 || x == 15 || x == 22) tile = rim ? 0x2158 : 0x2159;
            if (x == 29 || x == 30) tile |= 0x4000;
            if (x == 31) tile = 0x2100;
            ppu->vram[0x5c00 + (22 + y) * 32 + x] = (uint16_t)(tile | flip);
        }
}

static void SeedBattleHud(Ppu *ppu, unsigned length) {
    ppu->screenEnabled[0] = 2;
    ppu->screenEnabled[1] = ppu->screenWindowed[0] = 0;
    ppu->windowsel = ppu->cgadsub = ppu->cgwsel = ppu->mosaic = 0;
    ppu->bgXsc[1] = 0x5c;
    ppu->bgTileAdr = 0x42;
    ppu->hScroll[0] = ppu->hScroll[1] = 0;
    ppu->vScroll[1] = 223;
    ppu->cgram[3] = 0x7fff;
    ppu->cgram[7] = 31u << 5;
    ppu->cgram[8] = 31u << 10;
    ppu->cgram[9] = 8;
    for (unsigned i = 0; i < 1024; ++i) ppu->vram[0x5c00 + i] = 0x2101;
    for (unsigned tile = 0x100; tile < 0x1bc; ++tile)
        for (unsigned y = 0; y < 8; ++y)
            for (unsigned x = 0; x < 8; ++x) SetHudPixel(ppu, tile, x, y, 9);
    for (unsigned kind = 0; kind < 3; ++kind)
        for (unsigned y = 0; y < 8; ++y)
            for (unsigned x = 0; x < 8; ++x) {
                SetHudPixel(ppu, 0x160 + kind, x, y, FramePixel(kind, x, y));
                SetHudPixel(ppu, 0x155 + kind, x, y, FramePixel(kind, x, y));
            }
    for (unsigned y = 0; y < 4; ++y) {
        const unsigned flip = y == 3 ? 0x8000 : 0;
        ppu->vram[0x5f80 + y * 32] = (uint16_t)((y == 0 || y == 3 ? 0x2160 : 0x2162) | flip);
        ppu->vram[0x5f9f + y * 32] = ppu->vram[0x5f80 + y * 32] | 0x4000;
        for (unsigned x = 1; x < 31; ++x)
            ppu->vram[0x5f80 + y * 32 + x] = (uint16_t)(y == 0 || y == 3 ?
                0x2161 | flip : 0x2180 + (x - 1) * 2 + y - 1);
    }
    const unsigned start = (256u - length * 8u) / 2u + (length < 30 ? 4u : 0);
    for (unsigned y = 0; y < 16; ++y)
        for (unsigned x = 8; x < 248; ++x) {
            const unsigned tile = 0x180 + ((x - 8) / 8) * 2 + y / 8;
            const unsigned letter = x >= start ? (x - start) / 8 : length;
            const unsigned column = (x - start) & 7u;
            const bool ink = letter < length && column >= 1 && column <= 6 &&
                y >= 2 && y <= 13 && !(length >= 7 && letter == 2);
            SetHudPixel(ppu, tile, x & 7, y & 7, ink ? 3 : 8);
        }
    SeedPartyPanel(ppu);
}

static uint32_t HudColour(unsigned pixel) {
    return pixel == 3 ? 0xffffffu : pixel == 7 ? 0xff00u :
        pixel == 8 ? 0xffu : (8u * 255 / 31) << 16;
}

static uint32_t SceneColour(unsigned x, unsigned width) {
    const unsigned left = (width - 256u) / 2u + 8u;
    return ((x >= left && x < left + 240 ? 31 : x % 32) * 255 / 31) << 16;
}

static unsigned TestHudScope(Ppu *ppu) {
    static Ppu saved;
    static uint32_t frame[342 * 224];
    static uint32_t original[256 * 112];
    static uint16_t render_vram[0x8000];
    saved = *ppu;
    art_width = 342;
    ppu->renderBuffer = (uint8_t *)frame;
    ppu->renderPitch = 342 * 4;
    ppu->extraLeftCur = ppu->extraRightCur = ppu->extraLeftRight = 43;
    unsigned comparisons = 0;
    for (unsigned effects = 0; effects < 2; ++effects) {
        SeedBattleHud(ppu, 7);
        g_ram[0x1bec] = (uint8_t)effects;
        ppu->bgmode = 9;
        ppu->bgTileAdr = 0x642;
        ppu->bgXsc[2] = 8;
        ppu->cgram[1] = 0x7fff;
        for (unsigned row = 0; row < 8; ++row) ppu->vram[0x6008 + row] = 0x81;
        for (unsigned tile = 0; tile < 1024; ++tile) ppu->vram[0x0800 + tile] = 0x2001;
        for (unsigned tile = 0; tile < 22 * 32; ++tile) ppu->vram[0x5c00 + tile] = 0x2150;
        /* Selection windows share the frame tile family. */
        for (unsigned row = 0; row < 14; ++row) {
            ppu->vram[0x5c00 + row * 32] = 0x2155;
            ppu->vram[0x5c1f + row * 32] = 0x6155;
        }
        ppu->vScroll[1] = ppu->vScroll[2] = 911;
        ppu->screenEnabled[0] = 7;
        ppu->screenEnabled[1] = 1;
        ppu->cgadsub = 0x42;
        ppu->cgwsel = 2;
        assert(!Lufia2BattleEffectsPrepare(ppu, false, 342, 224));
        for (unsigned y = 0; y < 112; ++y) {
            PpuDrawWholeLine(ppu, y + 113);
            memcpy(original + y * 256, frame + (y + 112) * 342 + 43, 256 * 4);
        }
        assert(Lufia2BattleEffectsPrepare(ppu, true, 342, 224));
        ppu->screenEnabled[0] = 1;
        PpuDrawWholeLine(ppu, 33);
        ppu->screenEnabled[0] = 7;
        for (unsigned y = 0; y < 112; ++y) {
            PpuDrawWholeLine(ppu, y + 113);
            for (unsigned x = 8; x < 248; ++x) {
                assert(frame[(y + 112) * 342 + x + 43] == original[y * 256 + x]);
                ++comparisons;
            }
        }
    }
    /* Execution retains BG3 labels above the scene. */
    for (unsigned presentation = 0; presentation < 4; ++presentation)
    for (unsigned effect_plane = 0; effect_plane < 2; ++effect_plane) {
        ppu->renderVram = NULL;
        SeedBattleHud(ppu, 7);
        g_ram[0x1bec] = (uint8_t)effect_plane;
        ppu->bgmode = 9;
        ppu->bgTileAdr = 0x642;
        ppu->bgXsc[2] = 8;
        ppu->vScroll[1] = 1023;
        ppu->vScroll[2] = 895;
        /* Channel 7 shifts the execution panel four pixels. */
        ppu->hScroll[1] = 1020;
        if (presentation == 1) {
            /* Raster scrolling selects the visible map rows. */
            memcpy(ppu->vram + 0x5e00, ppu->vram + 0x5ec0, 6 * 32 * 2);
            memset(ppu->vram + 0x5ec0, 0, 6 * 32 * 2);
            ppu->vScroll[1] = 975;
        }
        if (presentation == 3) {
            /* Names overwrite portions of the panel's top rim. */
            for (unsigned y = 0; y < 8; ++y)
                for (unsigned x = 0; x < 8; ++x)
                    SetHudPixel(ppu, 0x163, x, y, x > 0 && x < 7 && y >= 2 ? 3 : 8);
            for (unsigned card = 0; card < 4; ++card)
                for (unsigned letter = 0; letter < 5; ++letter)
                    ppu->vram[0x5ec2 + card * 7 + letter] = 0x2163;
        }
        ppu->cgram[1] = 0x7fff;
        ppu->cgram[5] = 31u << 10;
        for (unsigned row = 0; row < 8; ++row) {
            ppu->vram[0x6008 + row] = 0x81;
            ppu->vram[0x6010 + row] = 0xff;
        }
        for (unsigned tile = 0; tile < 1024; ++tile) {
            ppu->vram[0x0800 + tile] = 0x2001;
            ppu->vram[0x1000 + tile] = 0x2402;
        }
        if (presentation == 2) {
            /* Recognition must use the renderer's VRAM snapshot. */
            memcpy(render_vram, ppu->vram, sizeof render_vram);
            memset(ppu->vram + 0x5ec0, 0, 6 * 32 * 2);
            ppu->renderVram = render_vram;
        }
        assert(!Lufia2BattleEffectsPrepare(ppu, false, 342, 224));
        for (unsigned y = 0; y < 48; ++y) {
            ppu->screenEnabled[0] = y < 2 || y >= 46 ? 2 : 6;
            PpuDrawWholeLine(ppu, y + 177);
            memcpy(original + y * 256, frame + (y + 176) * 342 + 43, 256 * 4);
        }
        assert(Lufia2BattleEffectsPrepare(ppu, true, 342, 224));
        ppu->screenEnabled[0] = effect_plane ? 5 : 1;
        ppu->bgXsc[2] = 0x10;
        ppu->cgadsub = effect_plane ? 0x84 : 0x81;
        ppu->fixedColor = effect_plane ? 8u << 10 : 8;
        PpuDrawWholeLine(ppu, 33);
        ppu->bgXsc[2] = 8;
        ppu->cgadsub = ppu->fixedColor = 0;
        for (unsigned y = 0; y < 48; ++y) {
            ppu->screenEnabled[0] = y < 2 || y >= 46 ? 2 : 6;
            PpuDrawWholeLine(ppu, y + 177);
            for (unsigned x = 0; x < 342; ++x) {
                const bool name = presentation == 3 && y < 2 &&
                    x >= 43 + 20 && x < 43 + 228 && (x - 43 - 20) % 56 < 40;
                const bool card = name || (y >= 2 && y < 46 && x >= 43 + 16 && x < 43 + 240);
                unsigned red = x >= 51 && x < 291 ? 31 : x % 32;
                red = red > 8 ? red - 8 : 0;
                const uint32_t scene = effect_plane ? 23u * 255 / 31 : (red * 255 / 31) << 16;
                const uint32_t expected = card ? original[y * 256 + x - 43] : scene;
                if (frame[(y + 176) * 342 + x] != expected)
                    fprintf(stderr, "Party HUD: profile=%u plane=%u x=%u y=%u got=%08x expected=%08x\n",
                        presentation, effect_plane, x, y, frame[(y + 176) * 342 + x], expected);
                assert(frame[(y + 176) * 342 + x] == expected);
                ++comparisons;
            }
        }
    }
    g_ram[0x1bec] = 0;
    *ppu = saved;
    art_width = 288;
    assert(Lufia2BattleEffectsPrepare(ppu, true, 288, 224));
    return comparisons;
}

static unsigned TestHudRipple(Ppu *ppu) {
    static Ppu saved;
    static uint32_t frame[342 * 224], scene[342 * 224];
    saved = *ppu;
    art_width = 342;
    ppu->renderBuffer = (uint8_t *)frame;
    ppu->renderPitch = 342 * 4;
    ppu->extraLeftCur = ppu->extraRightCur = ppu->extraLeftRight = 43;
    unsigned comparisons = 0;
    for (unsigned phase = 0; phase < 8; ++phase)
    for (unsigned effect = 0; effect < 2; ++effect) {
        SeedBattleHud(ppu, 7);
        ppu->bgTileAdr = 0x642;
        ppu->bgXsc[2] = 0x10;
        for (unsigned y = 0; y < 8; ++y)
            for (unsigned x = 0; x < 8; ++x)
                for (unsigned plane = 0; plane < 4; ++plane) {
                    uint16_t *word = ppu->vram + 0x2010 + y + (plane / 2) * 8;
                    const unsigned bit = 1u << (7u - x + (plane % 2) * 8);
                    const unsigned colour = 1u + (x + y * 3u) % 15u;
                    *word = (uint16_t)((*word & ~bit) | ((colour >> plane & 1u) ? bit : 0));
                }
        for (unsigned i = 1; i < 16; ++i) ppu->cgram[32 + i] = (uint16_t)(i * 2);
        for (unsigned i = 0; i < 1024; ++i) ppu->vram[0x1000 + i] = 0x2401;
        for (unsigned y = 0; y < 8; ++y) ppu->vram[0x6008 + y] = 0xff;
        ppu->cgram[5] = 31u << 10;
        g_ram[0x1bec] = (uint8_t)effect;
        assert(Lufia2BattleEffectsPrepare(ppu, true, 342, 224));
        /* Render the uninterrupted scene as the reference. */
        for (unsigned line = 1; line <= 224; ++line) {
            ppu->hScroll[0] = (uint16_t)(((line + phase) % 16u - 8u) & 1023u);
            ppu->vScroll[0] = (uint16_t)((1023u + (line + phase) % 3u) & 1023u);
            ppu->screenEnabled[0] = 1;
            ppu->screenEnabled[1] = effect ? 4 : 0;
            ppu->cgadsub = effect ? 0x41 : 0x81;
            ppu->cgwsel = effect ? 2 : 0;
            ppu->fixedColor = 4;
            PpuDrawWholeLine(ppu, line);
        }
        memcpy(scene, frame, sizeof scene);
        assert(Lufia2BattleEffectsPrepare(ppu, true, 342, 224));
        for (unsigned line = 1; line <= 224; ++line) {
            ppu->hScroll[0] = (uint16_t)(((line + phase) % 16u - 8u) & 1023u);
            ppu->vScroll[0] = (uint16_t)((1023u + (line + phase) % 3u) & 1023u);
            const bool hud = line <= 32 || line > 176;
            ppu->screenEnabled[0] = hud ? 2 : 1;
            ppu->screenEnabled[1] = !hud && effect ? 4 : 0;
            ppu->cgadsub = hud ? 0 : effect ? 0x41 : 0x81;
            ppu->cgwsel = !hud && effect ? 2 : 0;
            ppu->fixedColor = hud ? 0 : 4;
            ppu->hScroll[1] = line > 176 ? 1020 : 0;
            ppu->vScroll[1] = line > 176 ? 1023 : 223;
            uint8_t registers[PPU_SAVESTATE_REGS_SIZE];
            memcpy(registers, &ppu->inidisp, sizeof registers);
            PpuDrawWholeLine(ppu, line);
            assert(memcmp(registers, &ppu->inidisp, sizeof registers) == 0);
        }
        for (unsigned y = 0; y < 224; ++y)
            for (unsigned x = 0; x < 342; ++x) {
                const bool title = y < 32 && x >= 128 && x < 214;
                const bool card = y >= 178 && y < 222 && x >= 59 && x < 283;
                if (title || card) continue;
                if (frame[y * 342 + x] != scene[y * 342 + x])
                    fprintf(stderr, "HUD ripple: phase=%u effect=%u x=%u y=%u\n", phase, effect, x, y);
                assert(frame[y * 342 + x] == scene[y * 342 + x]);
                ++comparisons;
            }
    }
    g_ram[0x1bec] = 0;
    *ppu = saved;
    art_width = 288;
    assert(Lufia2BattleEffectsPrepare(ppu, true, 288, 224));
    return comparisons;
}

static unsigned TestUiLifecycle(const char *path, const uint8_t initial[72]) {
    uint8_t changed[72];
    memcpy(changed, initial, sizeof changed);
    changed[26] = 24;
    uint32_t crc = UINT32_MAX;
    for (unsigned i = 24; i < 72; ++i) {
        crc ^= changed[i];
        for (unsigned bit = 0; bit < 8; ++bit)
            crc = crc >> 1 ^ (0xedb88320u & (0u - (crc & 1u)));
    }
    crc = ~crc;
    for (unsigned byte = 0; byte < 4; ++byte)
        changed[16 + byte] = (uint8_t)(crc >> (byte * 8));
    Lufia2BattleUiLayout layouts[2];
    assert(Lufia2BattleUiDecode(initial, 72, &layouts[0]));
    assert(Lufia2BattleUiDecode(changed, 72, &layouts[1]));
    static uint32_t cards[4][48][64], frame[342 * 224], expected[342 * 224];
    uint32_t row[4][64], original[256];
    uint8_t mask[256] = {0};
    for (unsigned card = 0; card < 4; ++card)
        for (unsigned y = 0; y < 48; ++y)
            for (unsigned x = 0; x < 64; ++x)
                cards[card][y][x] = x % 3 ? 0xff000001u + card * 0x30405u + y : 0;
    memset(original, 0, sizeof original);
    Lufia2BattleUiInit(path, NULL);
    unsigned comparisons = 0;
    for (unsigned stage = 0; stage < 4; ++stage) {
        Lufia2BattleUiBegin(true, 342, 224);
        assert(Lufia2BattleUiActive());
        for (unsigned y = 0; y < 48; ++y) {
            for (unsigned card = 0; card < 4; ++card)
                memcpy(row[card], cards[card][y], sizeof row[card]);
            Lufia2BattleUiRecord(y, 176 + y, row, original, mask);
            if (stage == 0 && y == 24) {
                FILE *file = fopen(path, "wb");
                assert(file && fwrite(changed, 1, 72, file) == 72);
                assert(fclose(file) == 0);
            }
        }
        for (unsigned i = 0; i < 342 * 224; ++i)
            frame[i] = expected[i] = 0x00112233u;
        Lufia2BattleUiFinish((uint8_t *)frame, 342 * 4);
        memset(row, 0, sizeof row);
        Lufia2BattleUiRecord(0, 176, row, original, mask);
        Lufia2BattleUiCompose((uint8_t *)frame, 342, 224, false);
        Lufia2BattleUiDraw(&layouts[stage ? 1 : 0], &cards[0][0][0], expected, 342, 224, 176);
        assert(!memcmp(frame, expected, sizeof frame));
        comparisons += 342 * 224;
        if (stage == 1) {
            FILE *file = fopen(path, "wb");
            assert(file && fwrite("broken", 1, 6, file) == 6);
            assert(fclose(file) == 0);
        }
        if (stage == 2) assert(remove(path) == 0);
    }
    /* Reset removes stale cards after loading or rewinding. */
    Lufia2BattleUiReset();
    memcpy(frame, expected, sizeof frame);
    Lufia2BattleUiCompose((uint8_t *)frame, 342, 224, false);
    assert(!memcmp(frame, expected, sizeof frame));
    Lufia2BattleUiDraw(&layouts[0], &cards[0][0][0], frame, 342, 224, UINT32_MAX);
    assert(!memcmp(frame, expected, sizeof frame));
    FILE *file = fopen(path, "wb");
    assert(file && fwrite(initial, 1, 72, file) == 72);
    assert(fclose(file) == 0);
    return comparisons;
}

static unsigned TestModernParty(Ppu *ppu) {
    static const uint8_t layout_data[72] = {
        0x4c,0x32,0x55,0x49,1,0,0x48,0,0x56,1,0xe0,0,0x30,0,4,0,
        0x6f,0x4e,0x31,0x54,0,0,0,0,
        0,0,8,0,0x40,0,0,0,0,0,0,0,
        0x4d,1,2,0,0x40,0,0,0,1,0,0,0,
        0x9b,2,0xfe,0xff,0x40,0,0,0,2,0,0,0,
        0xe8,3,0xf8,0xff,0x40,0,0,0,3,0,0,0,
    };
    char path[80], preview_path[1024];
    const char *retained_preview = getenv("LUFIA2_BATTLE_UI_TEST_PREVIEW");
    FILE *file = NULL;
    for (unsigned attempt = 0; attempt < 32; ++attempt) {
        snprintf(path, sizeof path, ".lufia2-ui-test-%u.tmp", (unsigned)rand());
        file = fopen(path, "rb");
        if (!file) break;
        fclose(file);
    }
    assert(!file);
    if (retained_preview && retained_preview[0])
        snprintf(preview_path, sizeof preview_path, "%s", retained_preview);
    else
        snprintf(preview_path, sizeof preview_path, "%s.preview", path);
    file = fopen(path, "wb");
    assert(file && fwrite(layout_data, 1, sizeof layout_data, file) == sizeof layout_data);
    fclose(file);
    Lufia2BattleUiLayout layout, untouched;
    assert(Lufia2BattleUiDecode(layout_data, 72, &layout));
    untouched = layout;
    for (unsigned byte = 0; byte < 72; ++byte) {
        uint8_t damaged[72];
        memcpy(damaged, layout_data, 72);
        damaged[byte] ^= 0x80;
        assert(!Lufia2BattleUiDecode(damaged, 72, &layout));
        assert(!memcmp(&layout, &untouched, sizeof layout));
    }
    assert(!Lufia2BattleUiDecode(layout_data, 71, &layout));
    assert(Lufia2BattleUiCardX(&layout, 0, 342) == 8);
    assert(Lufia2BattleUiCardX(&layout, 1, 342) == 95);
    assert(Lufia2BattleUiCardX(&layout, 2, 342) == 183);
    assert(Lufia2BattleUiCardX(&layout, 3, 342) == 270);
    static Ppu saved, guest;
    static uint32_t frame[342 * 224], uninterrupted[342 * 224], pure[48][256], composed[342 * 224];
    static uint8_t ram[sizeof g_ram];
    static uint16_t render_vram[0x8000];
    saved = *ppu;
    unsigned comparisons = 0;
    art_width = 342;
    ppu->renderBuffer = (uint8_t *)frame;
    ppu->renderPitch = 342 * 4;
    ppu->extraLeftCur = ppu->extraRightCur = ppu->extraLeftRight = 43;
    for (unsigned profile = 0; profile < 4; ++profile) {
        ppu->renderVram = NULL;
        SeedBattleHud(ppu, 7);
        ppu->bgmode = 9;
        ppu->obsel = 3;
        ppu->bgTileAdr = 0x642;
        ppu->vScroll[1] = profile == 1 ? 975 : 1023;
        ppu->hScroll[1] = profile == 2 ? 0 : 1020;
        const unsigned map_at = profile == 1 ? 0x5e00 : 0x5ec0;
        if (profile == 1) {
            memcpy(ppu->vram + map_at, ppu->vram + 0x5ec0, 6 * 32 * 2);
            memset(ppu->vram + 0x5ec0, 0, 6 * 32 * 2);
        }
        ppu->cgram[3] = 0x7fff;
        ppu->cgram[4] = 31;
        ppu->cgram[5] = 31u << 5;
        ppu->cgram[6] = 31u | (31u << 5);
        for (unsigned card = 0; card < 4; ++card) {
            for (unsigned y = 0; y < 8; ++y)
                for (unsigned x = 0; x < 8; ++x)
                    SetHudPixel(ppu, 0x163 + card, x, y, x > 0 && x < 7 ? 3 + card : 8);
            for (unsigned letter = 0; letter < 5; ++letter)
                ppu->vram[map_at + 2 + card * 7 + letter] = (uint16_t)(0x2163 + card);
        }
        for (unsigned i = 0; i < 1024; ++i) {
            ppu->vram[0x0800 + i] = 0x2001;
            ppu->vram[0x1000 + i] = 0x2402;
        }
        ppu->cgram[1] = 0x7fff;
        ppu->cgram[9] = 8;
        for (unsigned y = 0; y < 8; ++y) {
            ppu->vram[0x6008 + y] = 0x81;
            ppu->vram[0x6010 + y] = 0xff;
            ppu->vram[0x6800 + y] = 0xff;
            ppu->vram[0x6808 + y] = 0;
        }
        ppu->cgram[129] = 31u | (31u << 10);
        for (unsigned slot = 0; slot < 128; ++slot) ppu->oam[slot * 2] = 0xf000;
        SeedSprite(ppu, 0, 120, 0x3080, false);
        ppu->oam[0] |= 180u << 8;
        if (profile == 3) {
            memcpy(render_vram, ppu->vram, sizeof render_vram);
            memset(ppu->vram + map_at, 0, 6 * 32 * 2);
            ppu->renderVram = render_vram;
        }
        const uint16_t hud_scroll = ppu->hScroll[1];
        Lufia2BattleUiInit(NULL, NULL);
        assert(!Lufia2BattleEffectsPrepare(ppu, false, 342, 224));
        ppu->bgXsc[2] = 8;
        ppu->vScroll[2] = 895;
        for (unsigned y = 0; y < 48; ++y) {
            ppu->screenEnabled[0] = y < 2 || y >= 46 ? 2 : 6;
            PpuDrawBattleMarginLine(ppu, y + 177);
            memcpy(pure[y], frame + (y + 176) * 342 + 43, sizeof pure[y]);
        }
        g_ram[0x1bec] = profile & 1;
        assert(Lufia2BattleEffectsPrepare(ppu, true, 342, 224));
        for (unsigned line = 1; line <= 224; ++line) {
            ppu->hScroll[0] = (uint16_t)(((line + profile) % 16 - 8) & 1023u);
            ppu->vScroll[0] = (uint16_t)((1023 + line % 3) & 1023u);
            ppu->screenEnabled[0] = 17;
            ppu->screenEnabled[1] = profile & 1 ? 4 : 0;
            ppu->bgXsc[2] = 0x10;
            ppu->cgadsub = profile & 1 ? 0x41 : 0x81;
            ppu->cgwsel = profile & 1 ? 2 : 0;
            ppu->fixedColor = 4;
            PpuDrawBattleMarginLine(ppu, line);
        }
        memcpy(uninterrupted, frame, sizeof frame);
        Lufia2BattleUiInit(path, preview_path);
        assert(Lufia2BattleEffectsPrepare(ppu, true, 342, 224));
        assert(Lufia2BattleUiActive());
        for (unsigned line = 1; line <= 224; ++line) {
            const bool hud = line > 176;
            ppu->hScroll[0] = (uint16_t)(((line + profile) % 16 - 8) & 1023u);
            ppu->vScroll[0] = (uint16_t)((1023 + line % 3) & 1023u);
            ppu->screenEnabled[0] = hud ? (line < 179 || line >= 223 ? 18 : 22) : 17;
            ppu->screenEnabled[1] = !hud && (profile & 1) ? 4 : 0;
            ppu->bgXsc[2] = hud ? 8 : 0x10;
            ppu->hScroll[1] = hud_scroll;
            ppu->cgadsub = hud ? 0 : profile & 1 ? 0x41 : 0x81;
            ppu->cgwsel = !hud && (profile & 1) ? 2 : 0;
            ppu->fixedColor = hud ? 0 : 4;
            guest = *ppu;
            memcpy(ram, g_ram, sizeof ram);
            PpuDrawBattleMarginLine(ppu, line);
            assert(!memcmp(&ppu->inidisp, &guest.inidisp, PPU_SAVESTATE_REGS_SIZE));
            assert(!memcmp(ppu->vram, guest.vram, sizeof ppu->vram));
            assert(!memcmp(ppu->oam, guest.oam, sizeof ppu->oam));
            assert(!memcmp(ram, g_ram, sizeof ram));
        }
        Lufia2BattleUiFinish((uint8_t *)frame, 342 * 4);
        assert(Lufia2BattleUiActive() && !Lufia2BattleUiCapturing());
        for (unsigned i = 0; i < 342 * 224; ++i) {
            if (frame[i] != uninterrupted[i]) fprintf(stderr, "Modern scene: profile=%u x=%u y=%u got=%08x want=%08x\n", profile, i % 342, i / 342, frame[i], uninterrupted[i]);
            assert(frame[i] == uninterrupted[i]);
            ++comparisons;
        }
        Lufia2BattleUiCompose((uint8_t *)frame, 342, 224, true);
        for (unsigned card = 0; card < 4; ++card) {
            const unsigned left = (unsigned)Lufia2BattleUiCardX(&layout, card, 342);
            const unsigned source = (17 + card * 56 - hud_scroll) & 511u;
            assert(frame[176 * 342 + left + 9] == (pure[0][source] | 0xff000000u));
            assert(frame[179 * 342 + left + 4] == (pure[3][(12u - hud_scroll) & 511u] | 0xff000000u));
            assert(frame[183 * 342 + left + 52] == (pure[7][(60u + card * 56 - hud_scroll) & 511u] | 0xff000000u));
            comparisons += 3;
            /* Side borders and inner shadows stay continuous. */
            for (unsigned y = 0; y < 48; ++y)
                for (unsigned x = 0; x < 64; ++x) {
                    if (x >= 8 && x < 56) continue;
                    const unsigned map_x = x < 8 ? 8 + x : 232 + x - 56;
                    const unsigned source_x = (map_x - hud_scroll) & 511u;
                    const unsigned at = (176 + y) * 342 + left + x;
                    const uint32_t expected = y >= 2 && y < 46 && x >= 4 && x < 60 ?
                        pure[y][source_x] | 0xff000000u : uninterrupted[at];
                    if (frame[at] != expected)
                        fprintf(stderr, "Card side: profile=%u card=%u x=%u y=%u\n",
                                profile, card, x, y);
                    assert(frame[at] == expected);
                    ++comparisons;
                }
        }
        memcpy(composed, frame, sizeof frame);
        ppu->screenEnabled[0] = 18;
        ppu->bgXsc[2] = 8;
        PpuDrawBattleMarginLine(ppu, 184);
        memcpy(frame, uninterrupted, sizeof frame);
        Lufia2BattleUiCompose((uint8_t *)frame, 342, 224, false);
        assert(!memcmp(frame, composed, sizeof frame));
        comparisons += 342 * 224;
        Lufia2BattleUiBegin(true, 342, 224);
        Lufia2BattleUiFinish((uint8_t *)frame, 342 * 4);
        assert(!Lufia2BattleUiActive());
        memcpy(frame, uninterrupted, sizeof frame);
        Lufia2BattleUiCompose((uint8_t *)frame, 342, 224, false);
        assert(!memcmp(frame, uninterrupted, sizeof frame));
        comparisons += 342 * 224;
        Lufia2BattleUiBegin(false, 342, 224);
        assert(!Lufia2BattleUiActive());
    }
    /* Partial captures restore the previous native UI. */
    ppu->renderVram = NULL;
    SeedBattleHud(ppu, 7);
    ppu->hScroll[1] = 1020;
    ppu->vScroll[1] = 1023;
    for (unsigned consumer = 0; consumer < 2; ++consumer) {
        Lufia2BattleUiInit(consumer ? path : NULL, NULL);
        assert(Lufia2BattleEffectsPrepare(ppu, true, 342, 224));
        ppu->screenEnabled[0] = 1;
        PpuDrawBattleMarginLine(ppu, 176);
        ppu->screenEnabled[0] = 2;
        for (unsigned line = 177; line <= 184; ++line)
            PpuDrawBattleMarginLine(ppu, line);
        Lufia2BattleUiFinish((uint8_t *)frame, 342 * 4);
        assert(!Lufia2BattleUiActive());
        if (!consumer) memcpy(composed, frame, sizeof frame);
        else {
            assert(!memcmp(frame + 176 * 342, composed + 176 * 342, 8 * 342 * 4));
            comparisons += 8 * 342;
        }
    }
    /* Selection windows keep their native rendering. */
    ppu->vram[0x5ec1] = 0x2150;
    for (unsigned consumer = 0; consumer < 2; ++consumer) {
        Lufia2BattleUiInit(consumer ? path : NULL, NULL);
        assert(Lufia2BattleEffectsPrepare(ppu, true, 342, 224));
        PpuDrawBattleMarginLine(ppu, 184);
        Lufia2BattleUiFinish((uint8_t *)frame, 342 * 4);
        assert(!Lufia2BattleUiActive());
        if (!consumer) memcpy(composed, frame, sizeof frame);
        else {
            assert(!memcmp(frame + 183 * 342, composed + 183 * 342, 342 * 4));
            comparisons += 342;
        }
    }
    Lufia2BattleUiInit(NULL, NULL);
    ppu->renderVram = NULL;
    *ppu = saved;
    g_ram[0x1bec] = 0;
    art_width = 288;
    assert(Lufia2BattleEffectsPrepare(ppu, true, 288, 224));
    comparisons += TestUiLifecycle(path, layout_data);
    assert(remove(path) == 0);
    if (!retained_preview || !retained_preview[0]) remove(preview_path);
    return comparisons;
}

static unsigned TestBattleHud(Ppu *ppu) {
    static Ppu saved;
    static Ppu guest;
    static uint8_t ram[sizeof g_ram];
    static uint32_t frame[kPpuBufWidth * 224];
    static uint32_t original[256 * 32];
    static uint32_t original_party[256 * 48];
    saved = *ppu;
    const unsigned lengths[] = {1, 7, 20, 29, 30};
    const unsigned widths[] = {288, 342, 800};
    unsigned comparisons = 0;
    for (unsigned size = 0; size < sizeof widths / sizeof *widths; ++size)
        for (unsigned effects = 0; effects < 2; ++effects)
            for (unsigned test = 0; test < sizeof lengths / sizeof *lengths; ++test) {
                const unsigned length = lengths[test];
                const unsigned width = widths[size];
                const unsigned margin = (width - 256) / 2;
                art_width = width;
                ppu->renderBuffer = (uint8_t *)frame;
                ppu->renderPitch = width * 4;
                ppu->extraLeftCur = ppu->extraRightCur = ppu->extraLeftRight = (uint16_t)margin;
                g_ram[0x1bec] = (uint8_t)effects;
                SeedBattleHud(ppu, length);
                assert(!Lufia2BattleEffectsPrepare(ppu, false, width, 224));
                for (unsigned y = 0; y < 32; ++y) {
                    PpuDrawWholeLine(ppu, y + 1);
                    memcpy(original + y * 256, frame + y * width + margin, 256 * 4);
                }
                ppu->vScroll[1] = 1023;
                for (unsigned y = 0; y < 48; ++y) {
                    PpuDrawWholeLine(ppu, y + 177);
                    memcpy(original_party + y * 256, frame + (y + 176) * width + margin, 256 * 4);
                }
                ppu->vScroll[1] = 223;
                assert(Lufia2BattleEffectsPrepare(ppu, true, width, 224));
                guest = *ppu;
                memcpy(ram, g_ram, sizeof ram);
                for (unsigned y = 1; y <= 32; ++y) PpuDrawWholeLine(ppu, y);
                uint8_t registers[PPU_SAVESTATE_REGS_SIZE];
                memcpy(registers, &ppu->inidisp, sizeof registers);
                ppu->screenEnabled[0] = 1;
                PpuDrawWholeLine(ppu, 33);
                ppu->screenEnabled[0] = 2;
                assert(memcmp(registers, &ppu->inidisp, sizeof registers) == 0);
                const unsigned ink_width = length * 8 - 2;
                const unsigned box_width = ink_width > 224 ? 256 : ink_width + 32;
                const unsigned left = margin + (256 - box_width) / 2;
                const unsigned text_left = margin + (256 - ink_width) / 2;
                for (unsigned y = 0; y < 32; ++y)
                    for (unsigned x = 0; x < width; ++x) {
                        uint32_t expected = SceneColour(x, width);
                        if (x >= left + 2 && x < left + box_width - 2 && y >= 2 && y < 30) {
                            unsigned pixel = 8;
                            if (x < left + 8 || x >= left + box_width - 8) {
                                const unsigned col = x < left + 8 ? x - left : left + box_width - 1 - x;
                                pixel = FramePixel(y < 8 || y >= 24 ? 0 : 2,
                                    col, y >= 24 ? 31 - y : y & 7);
                            } else if (y < 8 || y >= 24) {
                                pixel = FramePixel(1, 0, y >= 24 ? 31 - y : y);
                            } else if (x >= text_left && x < text_left + ink_width) {
                                const unsigned column = (x - text_left + 1) & 7;
                                const unsigned letter = (x - text_left + 1) / 8;
                                if (y >= 10 && y <= 21 && column >= 1 && column <= 6 &&
                                    !(length >= 7 && letter == 2)) pixel = 3;
                            }
                            expected = HudColour(pixel);
                        }
                        assert(frame[y * width + x] == expected);
                        ++comparisons;
                    }
                /* Party padding reveals the same scene, without magic. */
                ppu->vScroll[1] = 1023;
                for (unsigned y = 177; y <= 224; ++y) PpuDrawWholeLine(ppu, y);
                for (unsigned y = 176; y < 224; ++y)
                    for (unsigned x = 0; x < width; ++x) {
                        const bool card = y >= 178 && y < 222 &&
                            x >= margin + 12 && x < margin + 236;
                        const uint32_t expected = card ?
                            original_party[(y - 176) * 256 + x - margin] : SceneColour(x, width);
                        assert(frame[y * width + x] == expected);
                        ++comparisons;
                    }
                assert(memcmp(ppu->vram, guest.vram, sizeof ppu->vram) == 0);
                assert(memcmp(ppu->cgram, guest.cgram, sizeof ppu->cgram) == 0);
                assert(memcmp(ppu->oam, guest.oam, sizeof ppu->oam) == 0);
                assert(memcmp(g_ram, ram, sizeof ram) == 0);
                /* Disable widescreen and restore the original rectangle. */
                ppu->vScroll[1] = 223;
                assert(!Lufia2BattleEffectsPrepare(ppu, false, width, 224));
                for (unsigned y = 0; y < 32; ++y) {
                    PpuDrawWholeLine(ppu, y + 1);
                    assert(memcmp(original + y * 256, frame + y * width + margin, 256 * 4) == 0);
                    comparisons += 256;
                }
            }
    /* A different window must keep its original width. */
    SeedBattleHud(ppu, 7);
    ppu->vram[0x5fa1] = 0x2150;
    assert(Lufia2BattleEffectsPrepare(ppu, true, art_width, 224));
    PpuDrawWholeLine(ppu, 12);
    ppu->screenEnabled[0] = 1;
    PpuDrawWholeLine(ppu, 33);
    assert(frame[11 * art_width + ppu->extraLeftRight + 20] == 0xffu);
    /* Empty text must not produce a tiny phantom frame. */
    SeedBattleHud(ppu, 0);
    assert(Lufia2BattleEffectsPrepare(ppu, true, art_width, 224));
    PpuDrawWholeLine(ppu, 12);
    ppu->screenEnabled[0] = 1;
    PpuDrawWholeLine(ppu, 33);
    assert(frame[11 * art_width + ppu->extraLeftRight + 20] == 0xffu);
    g_ram[0x1bec] = 0;
    *ppu = saved;
    art_width = 288;
    assert(Lufia2BattleEffectsPrepare(ppu, true, 288, 224));
    return comparisons;
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
    /* BG3 effects include both margins. */
    static Ppu saved_ppu;
    saved_ppu = ppu;
    ppu.bgmode = 9;
    ppu.bgTileAdr = 0x0642;
    ppu.screenEnabled[0] = 5;
    ppu.screenEnabled[1] = 0;
    ppu.screenWindowed[0] = 4;
    ppu.windowsel = 3u << 8;
    ppu.wsBg3WidenY = 0;
    for (unsigned i = 0; i < 8; ++i) ppu.vram[0x6008 + i] = 0xff;
    for (unsigned palette = 1; palette < 8; ++palette)
        ppu.cgram[palette * 4 + 1] = (uint16_t)(palette << 10);
    /* Menu text shares the effect tilemaps. */
    ppu.bgXsc[2] = 8;
    ppu.window1left = 0;
    ppu.window1right = 255;
    for (unsigned tile = 0; tile < 1024; ++tile) ppu.vram[0x0800 + tile] = 0x2401;
    assert(!Lufia2BattleEffectsPlane(&ppu, 2));
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[7] == (7u * 255 / 31) << 16);
    assert(frame[116] == 255u / 31);
    g_ram[0x1bec] = 1;
    static uint32_t native_line[256];
    for (unsigned map = 0x0800; map <= 0x1400; map += 0x0400) {
        ppu.bgXsc[2] = (uint8_t)(map >> 8);
        for (unsigned tile = 0; tile < 1024; ++tile)
            ppu.vram[map + tile] = (uint16_t)(0x2001 | ((tile % 32 % 7 + 1) << 10));
        for (int shift = -9; shift <= 13; ++shift) {
            ppu.hScroll[2] = (uint16_t)(shift & 1023);
            ppu.window1left = 0;
            ppu.window1right = 255;
            assert(!Lufia2BattleEffectsPrepare(&ppu, false, 288, 224));
            PpuDrawWholeLine(&ppu, 1);
            memcpy(native_line, frame + 16, sizeof native_line);
            assert(Lufia2BattleEffectsPrepare(&ppu, true, 288, 224));
            PpuDrawWholeLine(&ppu, 1);
            for (unsigned x = 0; x < 256; ++x)
                if (native_line[x] != frame[x + 16]) {
                    fprintf(stderr, "BG3 map=%04x shift=%d x=%u native=%06x wide=%06x\n",
                            map, shift, x, native_line[x], frame[x + 16]);
                    break;
                }
            assert(memcmp(native_line, frame + 16, sizeof native_line) == 0);
            assert(ppu.wsBg3WidenY == 0);
            ppu.window1left = 8;
            ppu.window1right = 247;
            PpuDrawWholeLine(&ppu, 1);
            for (int x = -16; x < 272; ++x) {
                unsigned column = ((unsigned)(x + shift) & 255u) / 8;
                unsigned blue = column % 7 + 1;
                assert(frame[x + 16] == blue * 255 / 31);
                ++comparisons;
            }
        }
    }
    /* Colour windows also cover spell margins. */
    ppu.windowsel = 2u << 20;
    ppu.cgwsel = 0x40;
    ppu.screenWindowed[0] = 0;
    ppu.bgXsc[1] = 0x58;
    ppu.hScroll[2] = 0;
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[7] == 3u * 255 / 31);
    assert(frame[280] == 2u * 255 / 31);
    /* Subscreen spells blend across the same viewport. */
    ppu.cgwsel = 2;
    ppu.windowsel = 0;
    ppu.cgadsub = 0x41;
    ppu.screenEnabled[0] = 1;
    ppu.screenEnabled[1] = 4;
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[7] == ((3u * 255 / 31) << 16) + (1u * 255 / 31));
    assert(frame[280] == ((12u * 255 / 31) << 16) + (1u * 255 / 31));
    /* Other BG3 maps keep their native bounds. */
    ppu.bgXsc[2] = 0x18;
    assert(!Lufia2BattleEffectsPlane(&ppu, 2));
    ppu.cgwsel = ppu.cgadsub = 0;
    ppu.screenEnabled[0] = 5;
    ppu.screenEnabled[1] = 0;
    for (unsigned tile = 0; tile < 1024; ++tile) ppu.vram[0x1800 + tile] = 0x2401;
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[7] == (7u * 255 / 31) << 16);
    /* Replacement BG1 maps retain their own pixels. */
    ppu.screenEnabled[0] = 1;
    ppu.bgXsc[0] = 0x18;
    for (unsigned tile = 0; tile < 1024; ++tile) ppu.vram[0x1800 + tile] = 0x0801;
    assert(!Lufia2BattleEffectsBackground(&ppu));
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[7] == 0xff0000 && frame[280] == 0xff0000);
    ppu = saved_ppu;
    g_ram[0x1bec] = 0;
    comparisons += TestEffectSprites(&ppu, frame);
    comparisons += TestCulledSprites(&ppu);
    comparisons += TestHudScope(&ppu);
    comparisons += TestBattleHud(&ppu);
    comparisons += TestHudRipple(&ppu);
    comparisons += TestModernParty(&ppu);
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
    /* Nonuniform palette fades also affect authored art. */
    uint8_t *palette_rom = calloc(0xc0000, 1);
    assert(palette_rom);
    for (unsigned i = 0; i < 32; ++i)
        palette_rom[0xbcd58 + i * 2] = (uint8_t)i;
    Lufia2BattleEffectsInit(palette_rom, 0xc0000);
    free(palette_rom);
    ppu.hScroll[0] = 0;
    ppu.screenEnabled[0] = 1;
    ppu.screenEnabled[1] = 0;
    ppu.windowsel = ppu.cgadsub = ppu.cgwsel = 0;
    assert(Lufia2BattleEffectsPrepare(&ppu, true, 288, 224));
    g_ram[0x1bec] = 1;
    for (int fade = -31; fade <= 31; ++fade) {
        for (unsigned i = 0; i < 32; ++i) {
            int red = (int)i + fade;
            if (red < 0) red = 0;
            if (red > 31) red = 31;
            unsigned extra = fade > 0 ? (unsigned)fade : 0;
            ppu.cgram[32 + i] = (uint16_t)(red | (extra << 5) | (extra << 10));
        }
        PpuDrawWholeLine(&ppu, 1);
        for (unsigned side = 0; side < 2; ++side) {
            int red = (side ? 24 : 7) + fade;
            if (red < 0) red = 0;
            if (red > 31) red = 31;
            unsigned extra = fade > 0 ? (unsigned)fade * 255 / 31 : 0;
            assert(frame[side ? 280 : 7] ==
                ((unsigned)(red * 255 / 31) << 16) + (extra << 8) + extra);
            ++comparisons;
        }
    }
    g_ram[0x1bec] = 0;
    for (unsigned i = 32; i < 64; ++i) ppu.cgram[i] = 0;
    ppu.cgram[33] = 31;
    PpuDrawWholeLine(&ppu, 1);
    assert(frame[7] == (7u * 255 / 31) << 16);
    g_ram[0x6a] = 0;
    assert(!Lufia2BattleEffectsPrepare(&ppu, true, 288, 224));
    Lufia2BattleEffectsShutdown();
    printf("Battle effects PASS: %u source/colour/mosaic comparisons\n", comparisons);
    return 0;
}
