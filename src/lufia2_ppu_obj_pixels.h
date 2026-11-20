#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "snesrecomp_platform/memory_compare.h"

/* Consumer bridge between the core's native priority buffer and the portable
 * qualified OBJ plane. This performs no OAM selection or guest accesses. */
static inline uint16_t Lufia2PpuObjPackPixel(uint16_t native) {
    return (native & 255u) ?
        (uint16_t)((native & 255u) | (((native >> 12) - 2u) / 4u << 8) |
                   (((native >> 8) & 15u) == 4u ? 0x400u : 0u) | 0x800u) : 0;
}

static inline bool Lufia2PpuObjRowMatches(bool valid, bool previous_has,
    const uint16_t *previous, const bool *previous_mask, bool has,
    const uint16_t *native, const bool *mask, unsigned width, unsigned extra) {
    if (!valid || previous_has != has) return false;
    if (!has) return true;
    if (!snesrecomp_u16_equal(previous, native, width)) return false;
    if (!mask) return !previous_mask;
    if (!previous_mask) return false;
    /* Centre sprites are never clipped by the battle margin policy. */
    return snesrecomp_u8_equal((const uint8_t *)previous_mask, (const uint8_t *)mask, extra) &&
        snesrecomp_u8_equal((const uint8_t *)previous_mask + extra + 256u,
            (const uint8_t *)mask + extra + 256u, width - extra - 256u);
}

static inline void Lufia2PpuObjPackRow(uint16_t *out, const uint16_t *native,
    bool has, const bool *mask, unsigned width, unsigned extra) {
    if (!has) { memset(out, 0, width * sizeof *out); return; }
    for (unsigned x = 0; x < width; ++x)
        out[x] = mask && (x < extra || x >= extra + 256u) && !mask[x] ?
            0 : Lufia2PpuObjPackPixel(native[x]);
}

/* The source was already qualified. Normalize precisely as the old
 * packed -> native -> packed round trip, including transparent scratch. */
static inline void Lufia2PpuObjCopyRow(uint16_t *out, const uint16_t *source,
    const bool *mask, unsigned width, unsigned extra) {
    for (unsigned x = 0; x < width; ++x) {
        const uint16_t pixel = source[x];
        out[x] = (pixel & 0x800u) && (pixel & 255u) &&
            (!mask || (x >= extra && x < extra + 256u) || mask[x]) ?
            (uint16_t)(pixel & 0xfffu) : 0;
    }
}
