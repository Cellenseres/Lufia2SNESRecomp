#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "snesrecomp_platform/snes_ppu_resample.h"
#include "snesrecomp_platform/snes_bitplanes.h"
#include "snesrecomp_platform/snes_obj_ranges.h"
#include "lufia2_ppu_obj_capture.h"

/* Private synchronous source view, never a guest/captured/GPU-frame flag.
 * Keep the original window coordinates and colour context; only unused
 * margin pixels of an isolated 256-pixel UI source are omitted. Row caches
 * include renderFlags, so a narrow source cannot populate a full-row hit. */
#define LUFIA2_PPU_RENDER_NATIVE_SOURCE (UINT32_C(1) << 31)

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
