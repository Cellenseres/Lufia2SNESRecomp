#include "decomp_bridge/frame_returns.h"

typedef struct FramePairedCallContext {
    const struct CpuState *cpu;
    uint32_t entry;
    bool blocked;
} FramePairedCallContext;

static FramePairedCallContext s_frame_paired_call;

static bool FramePairedCallTracks(uint32_t entry) {
    entry &= 0x7fffffu;
    return s_frame_paired_call.cpu || entry == 0x0594e7u ||
        entry == 0x01ad96u || entry == 0x01aeeau;
}

bool Lufia2DecompFramePairedCallReady(const struct CpuState *cpu, uint32_t entry) {
    return cpu && s_frame_paired_call.cpu == cpu &&
        s_frame_paired_call.entry == (entry & 0x7fffffu) &&
        !s_frame_paired_call.blocked;
}
