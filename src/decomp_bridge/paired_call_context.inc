#include "decomp_bridge/frame_returns.h"

typedef struct FramePairedCallContext {
    const struct CpuState *cpu;
    uint32_t entry;
    bool blocked;
    long *steps;
    uint64_t *write_epoch;
    uint64_t *dynamic_epoch;
} FramePairedCallContext;

static FramePairedCallContext s_frame_paired_call;
static uint32_t s_frame_paired_return_pc = UINT32_MAX;

static unsigned FramePairedFieldBudget(uint32_t entry) {
    entry &= 0x7fffffu;
#if defined(LUFIA2_HAVE_NATIVE_FIELD_TICK_CLOCK)
    if (entry == 0x009c72u)
        return LUFIA2_FIELD_CALL_OPCODE_BUDGET;
#endif
#if defined(LUFIA2_HAVE_NATIVE_FIELD_CHILD_CLOCK)
    if (entry == 0x048000u)
        return LUFIA2_FIELD_EFFECT_OPCODES + 1u;
    if (entry == 0x00c21au)
        return LUFIA2_FIELD_TIMER_MAX_OPCODES + 1u;
#endif
#if defined(LUFIA2_HAVE_NATIVE_EVENT_SLOT_CLOCK)
    if (entry == 0x00cbaeu)
        return LUFIA2_EVENT_SLOT_OPCODE_BUDGET;
#endif
#if defined(LUFIA2_HAVE_NATIVE_FIELD_IDLE_CLOCK)
    if (entry == 0x0380cdu)
        return LUFIA2_FIELD_IDLE_OPCODE_BUDGET;
#endif
#if defined(LUFIA2_HAVE_NATIVE_FIELD_TRIGGER_CLOCK)
    if (entry == 0x0381c6u)
        return LUFIA2_FIELD_TRIGGER_OPCODE_BUDGET;
#endif
#if defined(LUFIA2_HAVE_NATIVE_FIELD_REQUEST_CLOCK)
    if (entry == 0x0383a0u || entry == 0x038103u)
        return LUFIA2_FIELD_REQUEST_OPCODE_BUDGET;
#endif
    return 0u;
}

static bool FramePairedCallBlocked(uint32_t entry, bool interrupt_owner,
    bool auto_quiescent, bool observed, long opcodes_left) {
    const unsigned field_budget = FramePairedFieldBudget(entry);
    if (field_budget)
        return interrupt_owner || observed || opcodes_left < (long)field_budget;
#if defined(LUFIA2_HAVE_NATIVE_NMI_CALLBACK_CLOCK)
#if defined(LUFIA2_HAVE_NATIVE_NMI_DISPLAY_CLOCK)
    if ((entry & 0x7fffffu) == 0x0087a7u)
        return !interrupt_owner || auto_quiescent || observed || opcodes_left < 128;
    if ((entry & 0x7fffffu) == 0x0087fcu || (entry & 0x7fffffu) == 0x00882eu ||
        (entry & 0x7fffffu) == 0x00884fu)
        return !interrupt_owner || auto_quiescent || observed || opcodes_left < 16;
    if ((entry & 0x7fffffu) == 0x0086c1u)
        return !interrupt_owner || auto_quiescent || observed || opcodes_left < 32;
#endif
    return (entry & 0x7fffffu) == 0x008703u &&
        (!interrupt_owner || auto_quiescent || observed || opcodes_left < 512);
#else
    (void)entry;
    (void)interrupt_owner;
    (void)auto_quiescent;
    (void)observed;
    (void)opcodes_left;
    return false;
#endif
}

static bool FramePairedBattleCall(uint32_t entry) {
    entry &= 0x7fffffu;
#if defined(LUFIA2_HAVE_NATIVE_SECOND_BATTLER_STAT)
    if (entry == 0x05c07cu)
        return true;
#endif
#if defined(LUFIA2_HAVE_NATIVE_FILLED_WINDOWS)
    if (entry == 0x01e38fu || entry == 0x01e3aeu || entry == 0x01e3cdu ||
        entry == 0x01e7d2u)
        return true;
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_STATUS)
    return entry == 0x05ccceu || entry == 0x05cce3u || entry == 0x05ccf8u ||
        entry == 0x05cd8cu || entry == 0x05cd9bu ||
        entry == 0x05cdfau || entry == 0x05cdaau || entry == 0x05cdd0u ||
        entry == 0x01b5a3u || entry == 0x01b5c4u ||
        entry == 0x01b705u || entry == 0x01be58u ||
        entry == 0x00834cu || entry == 0x008378u || entry == 0x01e808u ||
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
    if (FramePairedFieldBudget(entry))
        return true;
#if defined(LUFIA2_HAVE_NATIVE_NMI_CALLBACK_CLOCK)
#if defined(LUFIA2_HAVE_NATIVE_NMI_DISPLAY_CLOCK)
    if (entry == 0x0086c1u || entry == 0x0087a7u || entry == 0x0087fcu ||
        entry == 0x00882eu || entry == 0x00884fu)
        return true;
#endif
    if (entry == 0x008703u)
        return true;
#endif
    return s_frame_paired_call.cpu || FramePairedBattleCall(entry) || entry == 0x0594e7u ||
        entry == 0x01ad96u || entry == 0x01aeeau || entry == 0x05c5b5u || entry == 0x008866u ||
        entry == 0x008af7u || entry == 0x02f4d7u || entry == 0x02a00fu ||
        entry == 0x02f46cu || entry == 0x02f6bcu || entry == 0x05eb5fu ||
        entry == 0x039d57u || entry == 0x01f3e4u || entry == 0x03fec4u || entry == 0x01e7a6u;
}

bool Lufia2DecompFramePairedCallReady(const struct CpuState *cpu, uint32_t entry) {
    if (!cpu || s_frame_paired_call.cpu != cpu ||
        s_frame_paired_call.entry != (entry & 0x7fffffu) || s_frame_paired_call.blocked)
        return false;
    if (FramePairedFieldBudget(entry))
        return s_frame_paired_call.steps && s_frame_paired_call.write_epoch &&
            s_frame_paired_call.dynamic_epoch;
    return true;
}

uint32_t Lufia2DecompFramePairedReturnPc(const struct CpuState *cpu, uint32_t entry) {
    return Lufia2DecompFramePairedCallReady(cpu, entry)
        ? s_frame_paired_return_pc : UINT32_MAX;
}
