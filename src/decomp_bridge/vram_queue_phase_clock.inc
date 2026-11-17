#include "lufia2/battle_queue_steps.h"

static void TryVramQueuePhase(CpuState *cpu, uint32_t entry) {
    const bool begin = entry == 0x85ecdbu;
    const bool advance = entry == 0x85ece3u;
    const uint32_t end = advance ? 0x85eceeu : 0x85ece3u;
    const unsigned count = advance ? LUFIA2_QUEUE_ADVANCE_OPCODES : begin
        ? LUFIA2_QUEUE_BEGIN_HEADER_OPCODES : LUFIA2_QUEUE_ROW_HEADER_OPCODES;
    const unsigned reserve = advance ? 176u : begin ? 128u : 112u;
    if (!BattleQueueHookOwned(entry, ActorBridgeBattleQueueStep) || cpu != &g_cpu ||
        cpu->PB != 0x85u || (cpu->DB != 0x7eu && cpu->DB != 0x97u) || cpu->x_flag ||
        (!begin && (cpu->Y > 90u || cpu->Y % 6u)) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1fffu, true) ||
        !Lufia2DecompFrameRangeAllows(entry, end, BattleQueueHookOwned))
        return;
    const unsigned budget = begin
        ? Lufia2DecompFrameOwnedSpanBudget(entry, end, ActorBridgeBattleQueueStep)
        : Lufia2DecompFrameSpanBudget(entry, end, ActorBridgeBattleQueueStep);
    if (budget < count)
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    FixedSpanOpcode timing[LUFIA2_QUEUE_ADVANCE_OPCODES];
    if (advance) {
        for (unsigned i = 0u; i < 6u; ++i)
            timing[i] = (FixedSpanOpcode){(uint16_t)(0xece3u + i), 1u, 2u, 0u};
        timing[6] = (FixedSpanOpcode){0xece9u, 3u, 3u, 0u};
        timing[7] = (FixedSpanOpcode){0xececu, 2u, cpu->Y == 90u ? 2u : 3u, 0u};
    } else {
        unsigned offset = 0u;
        if (begin)
            timing[offset++] = (FixedSpanOpcode){0xecdbu, 3u, 3u, 0u};
        timing[offset++] = (FixedSpanOpcode){0xecdeu, 3u, 5u, 2u};
        timing[offset] = (FixedSpanOpcode){0xece1u, 2u, 2u, 0u};
    }
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, end - entry, &fetch);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x850000u, .timing = timing,
        .count = count, .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, NULL, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = advance
        ? Lufia2BattleAdvanceVramQueueRowStep(&memory, &state)
        : Lufia2BattleReadVramQueueRowStep(&memory, &state, entry);
    if (!advance)
        timing[count - 1u].cycles = result.pc == 0x85ecefu ? 3u : 2u;
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
