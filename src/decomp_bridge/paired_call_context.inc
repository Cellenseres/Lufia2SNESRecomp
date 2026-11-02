#include "decomp_bridge/frame_returns.h"

typedef struct FramePairedCallContext {
    const struct CpuState *cpu;
    uint32_t entry;
    bool blocked;
} FramePairedCallContext;

static FramePairedCallContext s_frame_paired_call;

static bool FramePairedCallTracks(uint32_t entry) {
    return s_frame_paired_call.cpu || (entry & 0x7fffffu) == 0x0594e7u;
}

bool Lufia2DecompFramePairedCallReady(const struct CpuState *cpu, uint32_t entry) {
    return cpu && s_frame_paired_call.cpu == cpu &&
        s_frame_paired_call.entry == (entry & 0x7fffffu) &&
        !s_frame_paired_call.blocked;
}
