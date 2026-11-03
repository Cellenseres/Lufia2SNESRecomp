#include "decomp_bridge/frame_returns.h"

typedef struct FramePairedCallContext {
    const struct CpuState *cpu;
    uint32_t entry;
    bool blocked;
} FramePairedCallContext;

static FramePairedCallContext s_frame_paired_call;
static uint32_t s_frame_paired_return_pc = UINT32_MAX;

static bool FramePairedBattleCall(uint32_t entry) {
    entry &= 0x7fffffu;
#if defined(LUFIA2_HAVE_NATIVE_SECOND_BATTLER_STAT)
    if (entry == 0x05c07cu)
        return true;
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_STATUS)
    return entry == 0x00834cu || entry == 0x008378u || entry == 0x01e808u ||
        entry == 0x01e542u || entry == 0x01e570u || entry == 0x01e503u ||
        entry == 0x01e2afu || entry == 0x01e2c8u || entry == 0x01e308u ||
        entry == 0x01e1b5u || entry == 0x01d081u;
#else
    (void)entry;
    return false;
#endif
}

static bool FramePairedCallTracks(uint32_t entry) {
    entry &= 0x7fffffu;
    return s_frame_paired_call.cpu || FramePairedBattleCall(entry) || entry == 0x0594e7u ||
        entry == 0x01ad96u || entry == 0x01aeeau || entry == 0x05c5b5u || entry == 0x008866u ||
        entry == 0x008af7u || entry == 0x02f4d7u || entry == 0x02a00fu ||
        entry == 0x02f46cu || entry == 0x02f6bcu || entry == 0x05eb5fu ||
        entry == 0x039d57u || entry == 0x01f3e4u || entry == 0x03fec4u || entry == 0x01e7a6u;
}

bool Lufia2DecompFramePairedCallReady(const struct CpuState *cpu, uint32_t entry) {
    return cpu && s_frame_paired_call.cpu == cpu &&
        s_frame_paired_call.entry == (entry & 0x7fffffu) &&
        !s_frame_paired_call.blocked;
}

uint32_t Lufia2DecompFramePairedReturnPc(const struct CpuState *cpu, uint32_t entry) {
    return Lufia2DecompFramePairedCallReady(cpu, entry)
        ? s_frame_paired_return_pc : UINT32_MAX;
}
