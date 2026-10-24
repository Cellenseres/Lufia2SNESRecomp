#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "snesrecomp_platform/snes_ppu_resample.h"
#include "snesrecomp_platform/snes_bitplanes.h"
#include "snesrecomp_platform/snes_obj_ranges.h"
#include "lufia2_ppu_obj_capture.h"

extern bool lufia2_ppu_host_costs_enabled;
void Lufia2PpuHostConfigure(void);

/* One viewport intersection per window span replaces the identical per-pixel
 * predicate. Source insets include the window's source-space bias. */
static inline bool Lufia2PpuClipViewport(bool enhancer, unsigned layer,
    unsigned inset_left, unsigned inset_right, int bias, int *left, int *right) {
    if (enhancer) {
        if (inset_left | inset_right) {
            const int minimum = (int)inset_left - bias;
            const int maximum = 256 - (int)inset_right - bias;
            if (*left < minimum) *left = minimum;
            if (*right > maximum) *right = maximum;
        } else if (layer == 0) {
            if (*left < 0) *left = 0;
            if (*right > 256) *right = 256;
        }
    }
    return *left < *right;
}

/* Spread each byte into eight nibbles, then combine the four bitplanes.
 * No cache invalidation is needed: every row uses its current VRAM words. */
static inline uint32_t Lufia2PpuSpreadByte(uint32_t bits) {
    return snesrecomp_ppu_spread_plane(bits);
}
static inline uint32_t Lufia2PpuDecode4bpp(uint32_t bits) {
    return snesrecomp_ppu_unpack_4bpp(bits);
}
