#ifndef LUFIA2_PPU_BACKGROUND_CACHE_H
#define LUFIA2_PPU_BACKGROUND_CACHE_H
#include <stddef.h>
#include "snes/ppu.h"
#include "snesrecomp_platform/memory_compare.h"
#include "lufia2_ppu_memory.h"
#include <string.h>

enum {
    LUFIA2_PPU_BACKGROUND_POLICY_SIZE = offsetof(Ppu, wsMode2Capture) - offsetof(Ppu, extraLeftCur)
};
/* Qualified Mode-1 menu pixel inputs. Only the active BG2 raster mapping is
 * retained; native clamped BG1/BG3 eligibility is enforced by the caller.
 * Palette and captured OBJ colours have independent RGB certificates. */
typedef struct Lufia2PpuBackgroundKey {
    uint8_t registers[PPU_SAVESTATE_REGS_SIZE];
    uint8_t head[LUFIA2_PPU_BACKGROUND_POLICY_SIZE], mosaic[256];
    uint8_t clamp, mirror, repeat, widen_layers, widen_windows;
    uint8_t gap_left, gap_right, clamp_band[4], repeat_band[4], stretch_band[4];
    uint8_t anchor_left, anchor_right, elastic_count, world_active;
    uint16_t world_left, world_right;
    PpuWsElasticSeg elastic[kPpuWsElasticSegs];
    uint8_t mosaic_start, sprites, debug_layers;
    uint32_t reserved; /* word-aligned key rows, canonical zero padding */
} Lufia2PpuBackgroundKey;
/* Serial consumer-owned pixel results, never guest execution snapshots.
 * Sprite selection/status, owner hooks and final UI layout run normally.
 * Reset and every drawing-frame boundary invalidate the VRAM certificate. */
typedef struct Lufia2PpuBackgroundCache {
    Lufia2PpuBackgroundKey keys[224];
    uint16_t object_rows[224][kPpuBufWidth];
    uint16_t background[224][kPpuBufWidth];
    uint16_t planes[2][224][kPpuBufWidth];
    uint32_t rgb[4][224][512];
    uint16_t palette[224][256], captured_objects[224][256];
    uint8_t brightness[224][63], half_brightness[224][64], rgb_valid[224];
    uint16_t capture_palette[2][224][256], capture_banks[2][224];
    uint8_t capture_brightness[2][224][63], capture_valid[224];
    uint64_t rgb_hits, rgb_misses;
    uint16_t vram[0x8000];
    uint8_t valid[224];
    SnesRecompPpuMemoryMask memory[224];
    const Ppu *owner;
    uint32_t writes;
    unsigned begin, width;
    bool checked, memory_valid;
    uint64_t hits, misses;
} Lufia2PpuBackgroundCache;

static void Lufia2PpuBackgroundCacheBegin(Lufia2PpuBackgroundCache *cache) { cache->checked = false; }
static bool Lufia2PpuBackgroundRegistersEqual(const uint8_t *previous,const Ppu *ppu) {
    _Static_assert(offsetof(Ppu,inidisp)==0,"Background register key requires the PPU snapshot origin");
    const uint8_t *current=(const uint8_t *)&ppu->inidisp;
    if(snesrecomp_u8_equal(previous,current,PPU_SAVESTATE_REGS_SIZE)) return true;
    if(PPU_mode(ppu)!=1 || (previous[offsetof(Ppu,bgmode)-offsetof(Ppu,inidisp)]&7u)!=1u)
        return false;
    /* Mode-1 background indices and RGB use the already qualified OBJ plane.
     * OBJ addressing/rotation and Mode-7 coefficients cannot change those
     * results. All BG, window, colour and display dependencies remain exact.
     * Keep the full-byte comparison first: ordinary warm rows do no extra
     * canonicalization or temporary-key construction. */
    return previous[0]==current[0] &&
        snesrecomp_u8_equal(previous+offsetof(Ppu,bgmode),current+offsetof(Ppu,bgmode),
            offsetof(Ppu,m7sel)-offsetof(Ppu,bgmode)) &&
        snesrecomp_u8_equal(previous+offsetof(Ppu,setini),current+offsetof(Ppu,setini),
            offsetof(Ppu,m7matrix)-offsetof(Ppu,setini)) &&
        snesrecomp_u8_equal(previous+offsetof(Ppu,fixedColor),current+offsetof(Ppu,fixedColor),
            PPU_SAVESTATE_REGS_SIZE - offsetof(Ppu,fixedColor));
}
/* Compare live dependencies with the retained row directly. A warm row must
 * not build and clear another complete key just to discard it. The active
 * raster mapping is still resolved from the live policy on every call. */
static bool Lufia2PpuBackgroundKeyMatches(const Lufia2PpuBackgroundKey *key,
    const Ppu *ppu,unsigned line,unsigned debug_layers) {
    if(!Lufia2PpuBackgroundRegistersEqual(key->registers,ppu) ||
       !snesrecomp_u8_equal(key->head,(const uint8_t *)&ppu->extraLeftCur,sizeof key->head) ||
       !snesrecomp_u8_equal(key->mosaic,ppu->mosaicModulo,sizeof key->mosaic) ||
       key->clamp!=ppu->wsLayerClamp || key->mirror!=ppu->wsLayerMirror ||
       key->repeat!=ppu->wsLayerRepeat || key->widen_layers!=ppu->wsWindowExpandLayers ||
       key->widen_windows!=ppu->wsWindowExpandWindows ||
       key->gap_left!=ppu->wsMarginGapL[1] || key->gap_right!=ppu->wsMarginGapR[1] ||
       key->mosaic_start!=ppu->mosaicStartLine || key->sprites!=ppu->lineHasSprites ||
       key->debug_layers!=(uint8_t)debug_layers) return false;
    for(unsigned bg=0;bg<4;++bg)
        if(key->clamp_band[bg]!=(ppu->wsClampY1[bg]>ppu->wsClampY0[bg] &&
              line>=ppu->wsClampY0[bg] && line<ppu->wsClampY1[bg]) ||
           key->repeat_band[bg]!=PpuWidescreenLayerRepeatBandActive(ppu,bg,line) ||
           key->stretch_band[bg]!=PpuWidescreenLayerStretchBandActive(ppu,bg,line)) return false;
    unsigned left=0,right=0;
    for(unsigned slot=0;slot<kPpuWsAnchorBands;++slot)
        if(line>=ppu->wsAnchorY0[slot][1] && line<ppu->wsAnchorY1[slot][1]) {
            left=ppu->wsAnchorLeftEnd[slot][1];right=ppu->wsAnchorRightStart[slot][1];break;
        }
    if(key->anchor_left!=left || key->anchor_right!=right) return false;
    const int world=PpuWidescreenLayerWorldBandAt(ppu,1,line);
    if(key->world_active!=(world>=0) ||
       key->world_left!=(world>=0?ppu->wsWorldLeft[world][1]:0) ||
       key->world_right!=(world>=0?ppu->wsWorldRight[world][1]:0)) return false;
    const int elastic=PpuWidescreenLayerElasticBandAt(ppu,1,line);
    const unsigned count=elastic>=0?ppu->wsElasticNSeg[elastic][1]:0;
    return count<=kPpuWsElasticSegs && key->elastic_count==count &&
        (!count || snesrecomp_u8_equal((const uint8_t *)key->elastic,
            (const uint8_t *)ppu->wsElasticSeg[elastic][1],count*sizeof *key->elastic));
}
static bool Lufia2PpuBackgroundCacheRead(Lufia2PpuBackgroundCache *cache, const Ppu *ppu,
                             unsigned line, unsigned debug_layers) {
    if (!cache->checked || cache->owner != ppu || cache->writes != ppu->vramWriteCount) {
        if (!cache->memory_valid || cache->owner != ppu) {
            memset(cache->valid, 0, sizeof cache->valid);
            memset(cache->rgb_valid, 0, sizeof cache->rgb_valid);
            memset(cache->capture_valid,0,sizeof cache->capture_valid);
            memcpy(cache->vram, ppu->vram, sizeof cache->vram);
        } else if(!snesrecomp_u16_equal(cache->vram,ppu->vram,0x8000)) {
            SnesRecompPpuMemoryMask changed;
            snesrecomp_ppu_memory_changes(cache->vram,ppu->vram,&changed);
            for(unsigned y=0;y<224;++y)
                if(cache->valid[y] && snesrecomp_ppu_memory_intersects(&changed,&cache->memory[y]))
                    cache->valid[y]=cache->rgb_valid[y]=cache->capture_valid[y]=0;
        }
        cache->owner = ppu; cache->writes = ppu->vramWriteCount;
        cache->checked = cache->memory_valid = true;
    }
    cache->begin = kPpuExtraLeftRight - ppu->extraLeftCur;
    cache->width = 256u + ppu->extraLeftCur + ppu->extraRightCur;
    unsigned y = line - 1u;
    if(cache->valid[y] && Lufia2PpuBackgroundKeyMatches(&cache->keys[y],ppu,line,debug_layers) &&
       (!ppu->lineHasSprites || snesrecomp_u16_equal(cache->object_rows[y]+cache->begin,
           ppu->objBuffer.data+cache->begin,cache->width))) {
        ++cache->hits;return true;
    }
    Lufia2PpuBackgroundKey key;
    memset(&key, 0, sizeof key);
    /* A rejected live certificate rebuilds the complete canonical key. The
     * second equality below is only for the cold/key-construction path. */
    memcpy(key.registers, &ppu->inidisp, sizeof key.registers);
    memcpy(key.head, &ppu->extraLeftCur, sizeof key.head);
    key.clamp = ppu->wsLayerClamp; key.mirror = ppu->wsLayerMirror; key.repeat = ppu->wsLayerRepeat;
    key.widen_layers = ppu->wsWindowExpandLayers; key.widen_windows = ppu->wsWindowExpandWindows;
    key.gap_left = ppu->wsMarginGapL[1]; key.gap_right = ppu->wsMarginGapR[1];
    for (unsigned bg = 0; bg < 4; ++bg) {
        key.clamp_band[bg] = ppu->wsClampY1[bg] > ppu->wsClampY0[bg] &&
            line >= ppu->wsClampY0[bg] && line < ppu->wsClampY1[bg];
        key.repeat_band[bg] = PpuWidescreenLayerRepeatBandActive(ppu, bg, line);
        key.stretch_band[bg] = PpuWidescreenLayerStretchBandActive(ppu, bg, line);
    }
    for (unsigned slot = 0; slot < kPpuWsAnchorBands; ++slot)
        if (line >= ppu->wsAnchorY0[slot][1] && line < ppu->wsAnchorY1[slot][1]) {
            key.anchor_left = ppu->wsAnchorLeftEnd[slot][1];
            key.anchor_right = ppu->wsAnchorRightStart[slot][1]; break;
        }
    int world = PpuWidescreenLayerWorldBandAt(ppu, 1, line);
    if (world >= 0) {
        key.world_active = 1; key.world_left = ppu->wsWorldLeft[world][1];
        key.world_right = ppu->wsWorldRight[world][1];
    }
    int elastic = PpuWidescreenLayerElasticBandAt(ppu, 1, line);
    if (elastic >= 0) {
        key.elastic_count = ppu->wsElasticNSeg[elastic][1];
        memcpy(key.elastic, ppu->wsElasticSeg[elastic][1], key.elastic_count * sizeof *key.elastic);
    }
    memcpy(key.mosaic, ppu->mosaicModulo, sizeof key.mosaic);
    cache->begin = kPpuExtraLeftRight - ppu->extraLeftCur;
    cache->width = 256u + ppu->extraLeftCur + ppu->extraRightCur;

    key.mosaic_start = ppu->mosaicStartLine;
    key.sprites = ppu->lineHasSprites;
    key.debug_layers = (uint8_t)debug_layers;
    if (!cache->valid[y] || !snesrecomp_u8_equal(
            (const uint8_t *)&cache->keys[y], (const uint8_t *)&key, sizeof key) ||
        (ppu->lineHasSprites && !snesrecomp_u16_equal(cache->object_rows[y] + cache->begin,
            ppu->objBuffer.data + cache->begin, cache->width))) {
        cache->keys[y] = key;
        if (ppu->lineHasSprites) memcpy(cache->object_rows[y] + cache->begin,
            ppu->objBuffer.data + cache->begin, cache->width * sizeof(uint16_t));
        cache->valid[y] = cache->rgb_valid[y] = cache->capture_valid[y] = 0; ++cache->misses;
        /* OBJ pixels are already qualified and compared above; no guest OBJ
         * memory is read while composing these captured background indices. */
        Lufia2PpuIndexRowMemory(ppu,line,7u,&cache->memory[y]);
        return false;
    }
    ++cache->hits;
    return true;
}
static void Lufia2PpuBackgroundCacheRestore(const Lufia2PpuBackgroundCache *cache, Ppu *ppu, unsigned line) {
    const unsigned y = line - 1u;
    memcpy(ppu->bgBuffers[0].data + cache->begin, cache->background[y] + cache->begin,
           cache->width * sizeof(uint16_t));
    memcpy(ppu->overlayBuffers[0].data, cache->planes[0][y], sizeof cache->planes[0][y]);
    memcpy(ppu->overlayBuffers[2].data, cache->planes[1][y], sizeof cache->planes[1][y]);
}
static void Lufia2PpuBackgroundCacheStore(Lufia2PpuBackgroundCache *cache, const Ppu *ppu, unsigned line) {
    const unsigned y = line - 1u;
    memcpy(cache->background[y] + cache->begin, ppu->bgBuffers[0].data + cache->begin,
           cache->width * sizeof(uint16_t));
    memcpy(cache->planes[0][y], ppu->overlayBuffers[0].data, sizeof cache->planes[0][y]);
    memcpy(cache->planes[1][y], ppu->overlayBuffers[2].data, sizeof cache->planes[1][y]);
    cache->valid[y] = 1;
    cache->capture_valid[y]=0;
}
/* A native menu never computes/stores the discarded full RGB background.
 * Retain its plain BG1/BG3 captures independently. The qualified index-cache
 * key above certifies the current row; only colours actually used by that
 * plane and the live brightness table can change its capture pixels. */
static bool Lufia2PpuBackgroundCaptureRead(Lufia2PpuBackgroundCache *cache,
    Ppu *ppu,unsigned line,unsigned plane,unsigned source) {
    unsigned y=line-1u;
    if(ppu->overlayRenderPitch[source]!=256*sizeof(uint32_t) ||
        !(cache->capture_valid[y]&(1u<<plane)) ||
        !snesrecomp_u8_equal(cache->capture_brightness[plane][y],ppu->brightnessMult,63)) return false;
    for(unsigned bank=0;bank<16;++bank) if(cache->capture_banks[plane][y]&(1u<<bank))
        if(!snesrecomp_u16_equal(cache->capture_palette[plane][y]+bank*16,
            ppu->cgram+bank*16,16)) return false;
    memcpy(ppu->overlayRenderBuffer[source]+y*ppu->overlayRenderPitch[source],
        cache->rgb[plane][y],256*sizeof(uint32_t));
    return true;
}
static void Lufia2PpuBackgroundCaptureStore(Lufia2PpuBackgroundCache *cache,
    const Ppu *ppu,unsigned line,unsigned plane,unsigned source) {
    unsigned y=line-1u,banks=0;
    cache->capture_valid[y]&=(uint8_t)~(1u<<plane);
    if(ppu->overlayRenderPitch[source]!=256*sizeof(uint32_t)) return;
    cache->rgb_valid[y]=0;
    for(unsigned x=0;x<256;++x) {
        unsigned index=ppu->overlayBuffers[source].data[kPpuExtraLeftRight+x]&255u;
        if(index) banks|=1u<<(index>>4);
    }
    cache->capture_banks[plane][y]=(uint16_t)banks;
    memcpy(cache->capture_palette[plane][y],ppu->cgram,sizeof ppu->cgram);
    memcpy(cache->capture_brightness[plane][y],ppu->brightnessMult,63);
    memcpy(cache->rgb[plane][y],ppu->overlayRenderBuffer[source]+
        y*ppu->overlayRenderPitch[source],256*sizeof(uint32_t));
    cache->capture_valid[y]|=(uint8_t)(1u<<plane);
}
/* Only complete, ordinary Mode-1 menu captures. Guest time, sprite selection,
 * captured owners and the live menu callback remain outside this memoization. */
static bool Lufia2PpuBackgroundRgbEligible(const Ppu *ppu) {
    unsigned width = 256u + 2u * ppu->extraLeftRight;
    /* Fixed-colour math depends only on the recorded key, palette and maps.
     * Subscreen-dependent RGB retains the ordinary composition path. */
    if ((PPU_mathEnabled(ppu) && PPU_addSubscreen(ppu)) || width > 512 || !ppu->renderBuffer ||
        ppu->renderPitch < width * 4u) return false;
    const unsigned sources[3] = {0, 2, 4};
    for (unsigned i = 0; i < 3; ++i) {
        unsigned bg = sources[i]; const PpuOverlayCapture *c = ppu->overlayCaptures + bg;
        if (!ppu->overlayRenderBuffer[bg] || ppu->overlayRenderPitch[bg] != 256u * 4u ||
            c->x0 != 0 || c->x1 != 256 || c->y0 != 0 || c->y1 != 224 ||
            c->flags != kPpuOverlayFlag_RemoveFromGame) return false;
    }
    return true;
}
static bool Lufia2PpuBackgroundRgbRead(Lufia2PpuBackgroundCache *cache, const Ppu *ppu, unsigned line) {
    unsigned y = line - 1u;
    if (!Lufia2PpuBackgroundRgbEligible(ppu)) return false;
    if (!cache->rgb_valid[y] ||
        !snesrecomp_u16_equal(cache->palette[y], ppu->cgram, 256) ||
        !snesrecomp_u8_equal(cache->brightness[y], ppu->brightnessMult, 63) ||
        !snesrecomp_u8_equal(cache->half_brightness[y], ppu->brightnessMultHalf, 64) ||
        !snesrecomp_u16_equal(cache->captured_objects[y], ppu->overlayBuffers[4].data + kPpuExtraLeftRight, 256)) {
        ++cache->rgb_misses; return false;
    }
    ++cache->rgb_hits; return true;
}
static void Lufia2PpuBackgroundRgbRestorePlane(const Lufia2PpuBackgroundCache *cache, const Ppu *ppu,
                              unsigned line, unsigned plane, unsigned source) {
    memcpy(ppu->overlayRenderBuffer[source] + (line - 1u) * ppu->overlayRenderPitch[source],
           cache->rgb[plane][line - 1u], 256u * 4u);
}
static void Lufia2PpuBackgroundRgbRestoreMain(const Lufia2PpuBackgroundCache *cache, const Ppu *ppu, unsigned line) {
    memcpy(ppu->renderBuffer + (line - 1u) * ppu->renderPitch,
           cache->rgb[3][line - 1u], (256u + 2u * ppu->extraLeftRight) * 4u);
    Lufia2PpuBackgroundRgbRestorePlane(cache, ppu, line, 2, 4);
}
static void Lufia2PpuBackgroundRgbStore(Lufia2PpuBackgroundCache *cache, const Ppu *ppu, unsigned line) {
    if (!Lufia2PpuBackgroundRgbEligible(ppu)) return;
    unsigned y = line - 1u; const unsigned sources[3] = {0, 2, 4};
    cache->capture_valid[y]=0;
    for (unsigned i = 0; i < 3; ++i)
        memcpy(cache->rgb[i][y], ppu->overlayRenderBuffer[sources[i]] + y * ppu->overlayRenderPitch[sources[i]], 256u * 4u);
    memcpy(cache->rgb[3][y], ppu->renderBuffer + y * ppu->renderPitch, (256u + 2u * ppu->extraLeftRight) * 4u);
    memcpy(cache->palette[y], ppu->cgram, sizeof cache->palette[y]);
    memcpy(cache->brightness[y], ppu->brightnessMult, sizeof cache->brightness[y]);
    memcpy(cache->half_brightness[y], ppu->brightnessMultHalf, sizeof cache->half_brightness[y]);
    memcpy(cache->captured_objects[y], ppu->overlayBuffers[4].data + kPpuExtraLeftRight, sizeof cache->captured_objects[y]);
    cache->rgb_valid[y] = 1;
}

#endif
