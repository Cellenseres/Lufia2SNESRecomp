#include "lufia2/battle_action_work_steps.h"

enum {
    ACTION_COPY_LAST_BYTE = 0x87u,
    ACTION_COPY_MAX_OPCODES = 4u * LUFIA2_BATTLE_ACTION_COPY_SPAN_MAX_BYTES
};
_Static_assert(ACTION_COPY_MAX_OPCODES <= 32u, "Action copy progress mask");

static void ActorBridgeActionCopySpan(CpuState *cpu, uint32_t entry);

static bool ActionCopyHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return (pc == 0x85cd8fu || pc == 0x85cd9eu) && hook == ActorBridgeActionCopySpan;
}

static void ActorBridgeActionCopySpan(CpuState *cpu, uint32_t entry) {
    if (!ActionCopyHookOwned(entry, ActorBridgeActionCopySpan) || cpu != &g_cpu ||
        cpu->PB != 0x85u || cpu->m_flag != 1u || cpu->x_flag ||
        cpu->X > ACTION_COPY_LAST_BYTE || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !Lufia2DecompFrameRangeAllows(entry, entry + 11u, ActionCopyHookOwned))
        return;
    const unsigned opcodes =
        Lufia2DecompFrameSpanBudget(entry, entry + 11u, ActorBridgeActionCopySpan);
    unsigned bytes = opcodes / 4u;
    if (bytes > LUFIA2_BATTLE_ACTION_COPY_SPAN_MAX_BYTES)
        bytes = LUFIA2_BATTLE_ACTION_COPY_SPAN_MAX_BYTES;
    if (bytes > cpu->X + 1u)
        bytes = cpu->X + 1u;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (!bytes || deadline <= cpu->master_cycles)
        return;
    uint64_t room = deadline - cpu->master_cycles;
    if (room > 1363u)
        room = 1363u;
    const unsigned iteration = 11u * cpu_region_speed(entry) + 28u;
    const unsigned timed_bytes = room > 40u ? (unsigned)((room - 40u) / iteration) : 0u;
    if (bytes > timed_bytes)
        bytes = timed_bytes;
    const unsigned reserve = bytes * iteration + 40u;
    if (!bytes || cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1fffu, false))
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    FixedSpanOpcode timing[ACTION_COPY_MAX_OPCODES];
    const uint16_t pc = (uint16_t)entry;
    for (unsigned index = 0u; index < bytes; ++index) {
        timing[4u * index] = (FixedSpanOpcode){pc, 4u, 5u, 1u};
        timing[4u * index + 1u] = (FixedSpanOpcode){(uint16_t)(pc + 4u), 4u, 5u, 1u};
        timing[4u * index + 2u] = (FixedSpanOpcode){(uint16_t)(pc + 8u), 1u, 2u, 0u};
        timing[4u * index + 3u] =
            (FixedSpanOpcode){(uint16_t)(pc + 9u), 2u, cpu->X == index ? 2u : 3u, 0u};
    }
    FixedSpanClock clock = {
        .cpu = cpu, .bank = 0x850000u, .timing = timing, .count = 4u * bytes
    };
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2BattleCopyActionWorkSpan(&memory, &state, entry, bytes);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
