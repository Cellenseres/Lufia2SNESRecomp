#ifndef LUFIA2_PPU_LAYER_CACHE_H
#define LUFIA2_PPU_LAYER_CACHE_H

#include "snes/ppu.h"
#include "snesrecomp_platform/memory_compare.h"
#include "lufia2_ppu_memory.h"
#include <string.h>

/* Palette-index/priority rows, before RGB conversion and composition. Only
 * isolated, native-width Mode-1 BG1/BG3 planes are eligible. No guest state,
 * sprite evaluation, owner hooks or palette conversion is cached here. */
typedef struct Lufia2PpuLayerKey {
    uint8_t registers[PPU_SAVESTATE_REGS_SIZE];
    uint16_t high_priority, low_priority;
    uint16_t extra_left, extra_right;
    uint8_t debug_layers, expand_layers, expand_windows;
} Lufia2PpuLayerKey;

typedef struct Lufia2PpuLayerCache {
    uint16_t pixels[2][224][256];
    Lufia2PpuLayerKey keys[2][224];
    uint8_t valid[2][224];
    SnesRecompPpuMemoryMask memory[2][224];
    uint16_t vram[0x8000];
    const Ppu *owner;
    uint32_t write_count;
    bool checked, memory_valid;
} Lufia2PpuLayerCache;

static inline void Lufia2PpuLayerCacheBegin(Lufia2PpuLayerCache *cache) {
    cache->checked = false;
}

static inline bool Lufia2PpuLayerCacheRead(Lufia2PpuLayerCache *cache,
    const Ppu *ppu, unsigned layer, unsigned line, unsigned high_priority,
    unsigned low_priority, unsigned debug_layers, PpuPixelPrioBufs *output) {
    if (!cache->checked || cache->owner != ppu ||
        cache->write_count != ppu->vramWriteCount) {
        if (!cache->memory_valid || cache->owner != ppu) {
            memset(cache->valid, 0, sizeof cache->valid);
            memcpy(cache->vram, ppu->vram, sizeof cache->vram);
        } else if(!snesrecomp_u16_equal(cache->vram,ppu->vram,0x8000)) {
            SnesRecompPpuMemoryMask changed;
            snesrecomp_ppu_memory_changes(cache->vram,ppu->vram,&changed);
            for(unsigned p=0;p<2;++p) for(unsigned y=0;y<224;++y)
                if(cache->valid[p][y] && snesrecomp_ppu_memory_intersects(&changed,&cache->memory[p][y]))
                    cache->valid[p][y]=0;
        }
        cache->owner = ppu;
        cache->write_count = ppu->vramWriteCount;
        cache->checked = cache->memory_valid = true;
    }
    Lufia2PpuLayerKey key;
    memset(&key, 0, sizeof key);
    Lufia2PpuRowRegisters(key.registers,ppu,1u<<layer);
    key.high_priority = (uint16_t)high_priority;
    key.low_priority = (uint16_t)low_priority;
    key.extra_left = ppu->extraLeftCur;
    key.extra_right = ppu->extraRightCur;
    key.debug_layers = (uint8_t)debug_layers;
    key.expand_layers = ppu->wsWindowExpandLayers;
    key.expand_windows = ppu->wsWindowExpandWindows;
    unsigned slot = layer == 2, y = line - 1;
    if (!cache->valid[slot][y] ||
        memcmp(&key, &cache->keys[slot][y], sizeof key)) {
        cache->keys[slot][y] = key;
        cache->valid[slot][y] = 0;
        Lufia2PpuIndexRowMemory(ppu,line,1u<<layer,&cache->memory[slot][y]);
        return false;
    }
    memcpy(output->data + kPpuExtraLeftRight, cache->pixels[slot][y],
           sizeof cache->pixels[slot][y]);
    return true;
}

static inline void Lufia2PpuLayerCacheStore(Lufia2PpuLayerCache *cache,
    unsigned layer, unsigned line, const PpuPixelPrioBufs *output) {
    unsigned slot = layer == 2, y = line - 1;
    memcpy(cache->pixels[slot][y], output->data + kPpuExtraLeftRight,
           sizeof cache->pixels[slot][y]);
    cache->valid[slot][y] = 1;
}

#endif
