#pragma once
#include "snes/ppu.h"
#include "snesrecomp_platform/memory_compare.h"
#include <string.h>

typedef struct Lufia2PpuPaletteCache {
    uint32 rgb[256];
    uint16 palette[256];
    uint8 multipliers[32];
    unsigned brightness;
    bool valid;
} Lufia2PpuPaletteCache;

/* Recheck after each scanline's callbacks; HDMA palette/fade changes count. */
static inline const uint32 *Lufia2PpuPaletteRgb(Ppu *ppu, Lufia2PpuPaletteCache *cache) {
    const bool all = !cache->valid || cache->brightness != PPU_brightness(ppu) ||
        !snesrecomp_u8_equal(cache->multipliers, ppu->brightnessMult,
                            sizeof cache->multipliers);
    if (all || !snesrecomp_u16_equal(cache->palette, ppu->cgram, 256)) {
        for (unsigned p = 0; p < 256; ++p) {
            const unsigned c = ppu->cgram[p];
            if (!all && cache->palette[p] == c) continue;
            cache->rgb[p] = ppu->brightnessMult[c & 31u] << 16 |
                            ppu->brightnessMult[(c >> 5) & 31u] << 8 |
                            ppu->brightnessMult[(c >> 10) & 31u];
        }
        memcpy(cache->palette, ppu->cgram, sizeof cache->palette);
        memcpy(cache->multipliers, ppu->brightnessMult, sizeof cache->multipliers);
        cache->brightness = PPU_brightness(ppu);
        cache->valid = true;
    }
    return cache->rgb;
}
