#pragma once
#include "patches/frame_wait.h"

static inline uint64_t Lufia2LogoWaitPairs(uint64_t current, uint64_t deadline,
                                          uint64_t pair_master, uint64_t steps) {
    if (!pair_master || current > UINT64_MAX - 80u) return 0;
    uint64_t pairs = L2FrameWaitBatchPairs(current + 80u, deadline, pair_master, steps);
    const uint64_t limit = 1200u / pair_master;
    return pairs < limit ? pairs : limit;
}

static inline void Lufia2LogoWaitState(Interp816 *in, uint8_t value) {
    in->a = (in->a & 0xff00u) | value;
    in->z = false;
    in->n = (value & 0x80u) != 0;
    in->pc = 0x8084u;
    in->cyclesUsed = 3;
}
