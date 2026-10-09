#pragma once
#include "snes/ppu.h"
#include <string.h>

typedef struct Lufia2PpuPaletteCache {
    uint32 rgb[256];
    uint16 palette[256];
    unsigned brightness;
    bool valid;
} Lufia2PpuPaletteCache;

/* Recheck after each scanline's callbacks; HDMA palette/fade changes count. */
static inline const uint32 *Lufia2PpuPaletteRgb(Ppu *ppu, Lufia2PpuPaletteCache *cache) {
    if (!cache->valid || cache->brightness != PPU_brightness(ppu) ||
        memcmp(cache->palette, ppu->cgram, sizeof cache->palette)) {
        for (unsigned p = 0; p < 256; ++p) {
            const unsigned c = ppu->cgram[p];
            cache->rgb[p] = ppu->brightnessMult[c & 31u] << 16 |
                            ppu->brightnessMult[(c >> 5) & 31u] << 8 |
                            ppu->brightnessMult[(c >> 10) & 31u];
        }
        memcpy(cache->palette, ppu->cgram, sizeof cache->palette);
        cache->brightness = PPU_brightness(ppu);
        cache->valid = true;
    }
    return cache->rgb;
}
