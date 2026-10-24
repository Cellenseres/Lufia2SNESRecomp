#ifndef LUFIA2_PPU_ROW_CACHE_H
#define LUFIA2_PPU_ROW_CACHE_H

#include "snes/ppu.h"
#include "snesrecomp_platform/memory_compare.h"
#include "snesrecomp_platform/snes_ppu_row_memory.h"
#include "lufia2_ppu_memory.h"
#include <stddef.h>
#include <string.h>

enum { LUFIA2_PPU_CACHE_ROWS = 224, LUFIA2_PPU_CACHE_PASSES = 4,
       LUFIA2_PPU_CACHE_WIDTH = 512 };
enum {
    LUFIA2_PPU_ROW_POLICY_HEAD = offsetof(Ppu, wsMode2Capture) - offsetof(Ppu, extraLeftCur),
    LUFIA2_PPU_ROW_POLICY_TAIL = offsetof(Ppu, wsOamMotionGraceOn) - offsetof(Ppu, wsLayerClamp),
    LUFIA2_PPU_ROW_OBJECT_POLICY = offsetof(Ppu, renderFlags) - offsetof(Ppu, wsOamMotionGraceOn)
};

typedef struct Lufia2PpuRowKey {
    uint8_t registers[PPU_SAVESTATE_REGS_SIZE];
    uint8_t policy_head[LUFIA2_PPU_ROW_POLICY_HEAD];
    uint8_t policy_tail[LUFIA2_PPU_ROW_POLICY_TAIL];
    uint8_t brightness[63], half_brightness[64], mosaic[256];
    uint8_t mosaic_start, even_frame, overscan, interlace, debug_layers;
    uint32_t render_flags;
} Lufia2PpuRowKey;

/* Owned, immutable policies shared by every clone in one prepared job. */
typedef struct Lufia2PpuRowPolicy {
    uint8_t tail[LUFIA2_PPU_ROW_POLICY_TAIL];
} Lufia2PpuRowPolicy;
static inline void Lufia2PpuRowPolicyCapture(Lufia2PpuRowPolicy *policy,const Ppu *ppu) {
    memcpy(policy->tail,&ppu->wsLayerClamp,sizeof policy->tail);
}

/* Host-only results of immediate, isolated rendering. Never guest snapshots. */
typedef struct Lufia2PpuRowCache {
    Lufia2PpuRowKey keys[LUFIA2_PPU_CACHE_ROWS];
    uint8_t registers[LUFIA2_PPU_CACHE_PASSES][LUFIA2_PPU_CACHE_ROWS][PPU_SAVESTATE_REGS_SIZE];
    uint32_t pixels[LUFIA2_PPU_CACHE_PASSES][LUFIA2_PPU_CACHE_ROWS][LUFIA2_PPU_CACHE_WIDTH];
    uint16_t vram[0x8000];
    uint16_t oam[LUFIA2_PPU_CACHE_ROWS][256];
    uint8_t high_oam[LUFIA2_PPU_CACHE_ROWS][32];
    uint8_t object_policy[LUFIA2_PPU_CACHE_ROWS][LUFIA2_PPU_ROW_OBJECT_POLICY];
    uint8_t object_parity[LUFIA2_PPU_CACHE_ROWS];
    uint8_t object_passes[LUFIA2_PPU_CACHE_ROWS], objects_valid[LUFIA2_PPU_CACHE_ROWS];
    uint8_t valid[LUFIA2_PPU_CACHE_ROWS];
    SnesRecompPpuMemoryMask memory[LUFIA2_PPU_CACHE_PASSES][LUFIA2_PPU_CACHE_ROWS];
    uint8_t memory_layers[LUFIA2_PPU_CACHE_PASSES][LUFIA2_PPU_CACHE_ROWS];
    uint16_t palettes[LUFIA2_PPU_CACHE_PASSES][LUFIA2_PPU_CACHE_ROWS][256];
    uint16_t palette_blocks[LUFIA2_PPU_CACHE_PASSES][LUFIA2_PPU_CACHE_ROWS];
    uint16_t palette_extra[LUFIA2_PPU_CACHE_PASSES][LUFIA2_PPU_CACHE_ROWS];
    const uint16_t *frame_vram;
    uint32_t frame_write_count;
    unsigned width, pending_line, pending_pass;
    bool checked, frame_disabled, memory_valid, pending;
    Lufia2PpuRowPolicy shared_policy;
    bool policy_checked, policy_valid, policy_bound;
    uint64_t hits, misses;
} Lufia2PpuRowCache;

static inline void Lufia2PpuRowCacheBegin(Lufia2PpuRowCache *cache) {
    if (!cache) return;
    cache->checked = cache->frame_disabled = cache->pending = false;
    cache->policy_checked=false;
}
static inline void Lufia2PpuRowCacheBindPolicy(Lufia2PpuRowCache *cache,
    const Lufia2PpuRowPolicy *policy) {
    if(!cache) return;
    if(cache->policy_bound!=(policy!=NULL)) cache->policy_checked=false;
    cache->policy_bound=policy!=NULL;
    if(policy) {
        if(!cache->policy_checked) {
            if(!cache->policy_valid || !snesrecomp_u8_equal(cache->shared_policy.tail,
                policy->tail,sizeof policy->tail)) {
                memset(cache->valid,0,sizeof cache->valid);
                cache->shared_policy=*policy;
            }
            cache->policy_checked=cache->policy_valid=true;
        }
    } else if(cache->policy_valid) {
        /* Ordinary live rendering resumes individual row certificates. */
        memset(cache->valid,0,sizeof cache->valid);
        cache->policy_valid=false;
    }
}
static inline unsigned Lufia2PpuRowSubscreen(const Ppu *ppu) {
    return PPU_preventMathMode(ppu)!=3 && PPU_addSubscreen(ppu) && PPU_mathEnabled(ppu)?
        ppu->screenEnabled[1]:0u;
}

/* Explicit layer consumers certify their read set. Unknown and sprite draws
 * keep every register. Common host policy remains shared; only each pass's
 * independent BG address/scroll/window inputs receive this smaller key. */
static inline bool Lufia2PpuRowCacheReadViewInputs(Lufia2PpuRowCache *cache,
    const Ppu *ppu, unsigned line, unsigned pass, uint32_t *row, unsigned width,
    bool objects, unsigned layers, uint16_t extra_palette,
    const uint16_t *vram,unsigned main_screen) {
    if (!cache) return false;
    cache->pending = false;
    /* Full-span rows are completely written. Narrower draws leave retained
     * scratch pixels outside their live span and cannot use this cache. */
    if (!ppu || !row || !line || line > LUFIA2_PPU_CACHE_ROWS ||
        pass >= LUFIA2_PPU_CACHE_PASSES || width > LUFIA2_PPU_CACHE_WIDTH ||
        width != 256u + 2u * ppu->extraLeftRight || PPU_mode(ppu) != 1 ||
        ppu->extraLeftCur != ppu->extraLeftRight ||
        ppu->extraRightCur != ppu->extraLeftRight || ppu->wsMode2CaptureLayer ||
        ppu->mode7Hd.pixels || cache->frame_disabled) return false;
    if(!vram) return false;
    if (!cache->checked) {
        if (!cache->memory_valid || cache->width != width) {
            memset(cache->valid, 0, sizeof cache->valid);
            memcpy(cache->vram, vram, sizeof cache->vram);
        } else {
            SnesRecompPpuMemoryMask changed;
            snesrecomp_ppu_memory_changes(cache->vram, vram, &changed);
            for (unsigned y = 0; y < LUFIA2_PPU_CACHE_ROWS; ++y)
                for (unsigned p = 0; p < LUFIA2_PPU_CACHE_PASSES; ++p)
                    if ((cache->valid[y] & (1u << p)) &&
                        snesrecomp_ppu_memory_intersects(&changed, &cache->memory[p][y]))
                        cache->valid[y] &= (uint8_t)~(1u << p);
        }
        cache->width = width;
        cache->frame_vram = vram;
        cache->frame_write_count = ppu->vramWriteCount;
        cache->memory_valid = cache->checked = true;
    } else if (cache->frame_vram != vram || cache->frame_write_count != ppu->vramWriteCount) {
        /* All core VRAM stores advance this counter, including word DMA and
         * state loading. A changed borrowed view also abandons reuse. */
        memset(cache->valid, 0, sizeof cache->valid);
        cache->frame_disabled = true;
        return false;
    }
    const unsigned y = line - 1u;
    Lufia2PpuRowKey *key=cache->keys+y;
    /* Compare borrowed inputs directly. An unchanged row needs neither a
     * cleared temporary key nor another copy of all elastic policies. */
    if(key->mosaic_start!=ppu->mosaicStartLine || key->render_flags!=ppu->renderFlags ||
        key->debug_layers!=g_snes_ppu_dbg_layer_mask || key->overscan!=ppu->frameOverscan ||
        key->interlace!=ppu->frameInterlace ||
        !snesrecomp_u8_equal(key->policy_head,(const uint8_t *)&ppu->extraLeftCur,sizeof key->policy_head) ||
        !snesrecomp_u8_equal(key->brightness,ppu->brightnessMult,sizeof key->brightness) ||
        !snesrecomp_u8_equal(key->half_brightness,ppu->brightnessMultHalf,sizeof key->half_brightness) ||
        !snesrecomp_u8_equal(key->mosaic,ppu->mosaicModulo,sizeof key->mosaic) ||
        (!cache->policy_bound && !snesrecomp_u8_equal(key->policy_tail,&ppu->wsLayerClamp,sizeof key->policy_tail))) {
        memcpy(key->policy_head,&ppu->extraLeftCur,sizeof key->policy_head);
        if(!cache->policy_bound) memcpy(key->policy_tail,&ppu->wsLayerClamp,sizeof key->policy_tail);
        memcpy(key->brightness,ppu->brightnessMult,sizeof key->brightness);
        memcpy(key->half_brightness,ppu->brightnessMultHalf,sizeof key->half_brightness);
        memcpy(key->mosaic,ppu->mosaicModulo,sizeof key->mosaic);
        key->mosaic_start=ppu->mosaicStartLine;key->render_flags=ppu->renderFlags;
        key->debug_layers=g_snes_ppu_dbg_layer_mask;key->overscan=ppu->frameOverscan;
        key->interlace=ppu->frameInterlace;
        cache->valid[y] = 0;
    }
    const uint8_t bit = (uint8_t)(1u << pass);
    /* Unknown memory consumers and OBJ retain the full-memory certificate. */
    if (objects || layers & ~7u) layers = 255u;
    uint8_t registers[PPU_SAVESTATE_REGS_SIZE];
    Lufia2PpuRowRegisters(registers,ppu,layers);
    registers[offsetof(Ppu,screenEnabled)-offsetof(Ppu,inidisp)]=(uint8_t)main_screen;
    if(layers!=255u) registers[offsetof(Ppu,screenEnabled)-offsetof(Ppu,inidisp)]&=
        (uint8_t)(0xf0u | layers);
    if(!snesrecomp_u8_equal(cache->registers[pass][y],registers,sizeof registers)) {
        memcpy(cache->registers[pass][y],registers,sizeof registers);
        cache->valid[y]&=(uint8_t)~bit;
    }
    if (cache->memory_layers[pass][y] != layers) cache->valid[y] &= (uint8_t)~bit;
    if (cache->palette_extra[pass][y] != extra_palette) cache->valid[y] &= (uint8_t)~bit;
    if (!!(cache->object_passes[y] & bit) != objects) cache->valid[y] &= (uint8_t)~bit;
    if (objects) {
        /* Isolated BG passes cannot observe OAM animation. Sprite-dependent
         * passes retain exact bytes, including changes without dirty hints. */
        if (!cache->objects_valid[y] ||
            !snesrecomp_u16_equal(cache->oam[y], ppu->oam, 256) ||
            !snesrecomp_u8_equal(cache->high_oam[y], ppu->highOam, 32) ||
            !snesrecomp_u8_equal(cache->object_policy[y], &ppu->wsOamMotionGraceOn,
                                 LUFIA2_PPU_ROW_OBJECT_POLICY) ||
            cache->object_parity[y] != (PPU_objInterlace(ppu) ? ppu->evenFrame : 0)) {
            cache->valid[y] &= (uint8_t)~cache->object_passes[y];
            memcpy(cache->oam[y], ppu->oam, sizeof cache->oam[y]);
            memcpy(cache->high_oam[y], ppu->highOam, sizeof cache->high_oam[y]);
            memcpy(cache->object_policy[y], &ppu->wsOamMotionGraceOn,
                   LUFIA2_PPU_ROW_OBJECT_POLICY);
            cache->object_parity[y] = PPU_objInterlace(ppu) ? ppu->evenFrame : 0;
            cache->objects_valid[y] = 1;
        }
        cache->object_passes[y] |= bit;
    } else {
        cache->object_passes[y] &= (uint8_t)~bit;
    }
    if (cache->valid[y] & bit) for (unsigned block = 0; block < 16; ++block)
        if ((cache->palette_blocks[pass][y] & (1u << block)) &&
            !snesrecomp_u16_equal(cache->palettes[pass][y] + block * 16u,
                                  ppu->cgram + block * 16u, 16)) {
            cache->valid[y] &= (uint8_t)~bit;
            break;
        }
    if (cache->valid[y] & bit) {
        memcpy(row, cache->pixels[pass][y], width * sizeof *row);
        ++cache->hits;
        return true;
    }
    cache->pending = true;
    cache->pending_line = line;
    cache->pending_pass = pass;
    cache->memory_layers[pass][y] = (uint8_t)layers;
    SnesRecompPpuMemoryMask *memory = &cache->memory[pass][y];
    memset(memory, layers == 255u ? 255 : 0, sizeof *memory);
    uint16_t colours = layers == 255u ? UINT16_MAX : (uint16_t)(1u | extra_palette);
    if (layers != 255u) for (unsigned bg = 0; bg < 3; ++bg) if (layers & (1u << bg)) {
        SnesPpuBgState state = {0};
        state.tilemap_word_addr = (uint16_t)PPU_bgTilemapAdr(ppu, bg);
        state.char_word_addr = (uint16_t)PPU_bgTileAdr(ppu, bg);
        state.bpp = bg == 2 ? 2u : 4u;
        state.big_tiles = PPU_bigTiles(ppu, bg) != 0;
        state.wide = PPU_bgTilemapWider(ppu, bg) != 0;
        state.tall = PPU_bgTilemapHigher(ppu, bg) != 0;
        unsigned source_y = (PPU_mosaicEnabled(ppu, bg) && PPU_mosaicSize(ppu) > 1 ?
                            ppu->mosaicModulo[line] : line)
                            + ppu->vScroll[bg];
        unsigned banks = snesrecomp_ppu_mode1_row_memory(vram, &state, source_y, memory);
        for (unsigned bank = 0; bank < 8; ++bank) if (banks & (1u << bank))
            colours |= (uint16_t)(1u << ((bank << state.bpp) >> 4));
    }
    cache->palette_blocks[pass][y] = colours;
    cache->palette_extra[pass][y] = extra_palette;
    memcpy(cache->palettes[pass][y], ppu->cgram, sizeof cache->palettes[pass][y]);
    ++cache->misses;
    return false;
}

static inline bool Lufia2PpuRowCacheReadInputs(Lufia2PpuRowCache *cache,
    const Ppu *ppu,unsigned line,unsigned pass,uint32_t *row,unsigned width,
    bool objects,unsigned layers,uint16_t extra_palette) {
    return Lufia2PpuRowCacheReadViewInputs(cache,ppu,line,pass,row,width,objects,layers,
        extra_palette,ppu?PpuRenderVram(ppu):NULL,ppu?ppu->screenEnabled[0]:0u);
}

/* Isolated draws differ only in their VRAM view and main-screen selection.
 * Certify that view before constructing the full callback-compatible clone. */
static inline bool Lufia2PpuRowCacheReadViewLayers(Lufia2PpuRowCache *cache,
    const Ppu *ppu,unsigned line,unsigned pass,uint32_t *row,unsigned width,
    bool objects,unsigned layers,const uint16_t *vram,unsigned main_screen) {
    return Lufia2PpuRowCacheReadViewInputs(cache,ppu,line,pass,row,width,objects,layers,
        0,vram,main_screen);
}

static inline bool Lufia2PpuRowCacheReadLayers(Lufia2PpuRowCache *cache,
    const Ppu *ppu, unsigned line, unsigned pass, uint32_t *row, unsigned width,
    bool objects, unsigned layers) {
    return Lufia2PpuRowCacheReadInputs(cache, ppu, line, pass, row, width, objects, layers, 0);
}

static inline bool Lufia2PpuRowCacheReadQualified(Lufia2PpuRowCache *cache,
    const Ppu *ppu, unsigned line, unsigned pass, uint32_t *row, unsigned width,
    bool objects) {
    return Lufia2PpuRowCacheReadLayers(cache, ppu, line, pass, row, width, objects, 255u);
}

/* Unknown consumers conservatively retain every sprite dependency. */
static inline bool Lufia2PpuRowCacheRead(Lufia2PpuRowCache *cache,
    const Ppu *ppu, unsigned line, unsigned pass, uint32_t *row, unsigned width) {
    return Lufia2PpuRowCacheReadQualified(cache, ppu, line, pass, row, width, true);
}

/* Read's source stays unchanged during the synchronous clone draw. */
static inline void Lufia2PpuRowCacheStore(Lufia2PpuRowCache *cache,
    const uint32_t *row) {
    if (!cache || !cache->pending || !row) return;
    const unsigned y = cache->pending_line - 1u, pass = cache->pending_pass;
    memcpy(cache->pixels[pass][y], row, cache->width * sizeof *row);
    cache->valid[y] |= (uint8_t)(1u << pass);
    cache->pending = false;
}

#endif
