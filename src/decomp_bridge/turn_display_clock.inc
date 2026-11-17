#include "lufia2/battle_turn_display_steps.h"

static void ActorBridgeTurnDisplayRegion(CpuState *cpu, uint32_t entry);

static uint32_t TurnDisplayRegionEnd(uint32_t entry) {
    return entry == 0x859ddau ? 0x859decu : entry == 0x859df0u ? 0x859e04u : 0u;
}

static bool TurnDisplayRegionHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return TurnDisplayRegionEnd(pc) && hook == ActorBridgeTurnDisplayRegion;
}

static void ActorBridgeTurnDisplayRegion(CpuState *cpu, uint32_t entry) {
    const uint32_t end = TurnDisplayRegionEnd(entry);
    const bool last = entry == 0x859df0u;
    const unsigned count = last ? LUFIA2_TURN_DISPLAY_FINAL_REGION_OPCODES
        : LUFIA2_TURN_DISPLAY_REGION_OPCODES;
    if (!end || cpu != &g_cpu || cpu->PB != 0x85u || (cpu->DB != 0x7eu && cpu->DB != 0x97u) ||
        cpu->m_flag || cpu->x_flag || cpu->Y > 90u || cpu->Y % 6u ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - 320u ||
        !NativeOpcodeWindowSupported(cpu, entry, 320u, 0x1ffcu, true) ||
        !Lufia2DecompFrameRangeAllows(entry, end, TurnDisplayRegionHookOwned) ||
        Lufia2DecompFrameSpanBudget(entry, end, ActorBridgeTurnDisplayRegion) < count)
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    FixedSpanOpcode timing[LUFIA2_TURN_DISPLAY_FINAL_REGION_OPCODES];
    for (unsigned i = 0u; i < LUFIA2_TURN_DISPLAY_REGION_OPCODES; ++i)
        timing[i] = (FixedSpanOpcode){(uint16_t)(entry + 3u * i), 3u,
            i & 1u ? 7u : 3u, i & 1u ? 2u : 0u};
    if (last)
        timing[LUFIA2_TURN_DISPLAY_REGION_OPCODES] = (FixedSpanOpcode){0x9e02u, 2u, 3u, 0u};
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, end - entry, &fetch);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x850000u, .timing = timing,
        .count = count, .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = Lufia2BattleWriteTurnDisplayRegion(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == end);
    assert(cpu->master_cycles - start <= 320u);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
