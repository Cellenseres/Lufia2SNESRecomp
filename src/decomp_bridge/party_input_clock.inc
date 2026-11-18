#include "lufia2/battle_party_input_steps.h"

static void ActorBridgePartyInputStep(CpuState *cpu, uint32_t entry);

static bool PartyInputHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return pc == 0x81ccecu && hook == ActorBridgePartyInputStep;
}

static unsigned PreparePartyInputClock(const CpuState *cpu, FixedSpanOpcode *timing,
    uint32_t *stop) {
    static const FixedSpanOpcode input[] = {
        {0xccecu, 2u, 3u, 1u}, {0xcceeu, 2u, 2u, 0u},
        {0xccf0u, 2u, 2u, 0u}, {0xccf2u, 2u, 3u, 1u},
        {0xccf4u, 2u, 3u, 0u}
    };
    memcpy(timing, input, sizeof input);
    if (cpu->ram[0xddu] & 0xa0u) {
        timing[2].cycles = 3u;
        timing[3] = (FixedSpanOpcode){0xcd39u, 2u, 2u, 0u};
        *stop = 0x81cd3bu;
        return 4u;
    }
    const bool cancel = (cpu->ram[0xdeu] & 0x80u) != 0u;
    timing[4].cycles -= cancel;
    *stop = cancel ? 0x81ccf6u : 0x81cc93u;
    return LUFIA2_BATTLE_PARTY_INPUT_MAX_OPCODES;
}

static void ActorBridgePartyInputStep(CpuState *cpu, uint32_t entry) {
    enum { PARTY_INPUT_CLOCK_BUDGET = 208u };
    if (!PartyInputHookOwned(entry, ActorBridgePartyInputStep) || cpu != &g_cpu ||
        cpu->PB != 0x81u || cpu->DB != 0x97u || cpu->D || cpu->_flag_D ||
        cpu->m_flag != 1u || cpu->x_flag || cpu->S < 0x1f00u ||
        (cpu->host_return_valid && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        !Lufia2DecompOpcodeWindowReady() || !Lufia2DecompFrameReturnCpuReady() ||
        !NativeOpcodeWindowWithDeferredHdmaSupported(cpu, entry,
            PARTY_INPUT_CLOCK_BUDGET, 0x1fffu, false, true) ||
        !Lufia2DecompFrameRangeAllows(entry, 0x81cd3bu, PartyInputHookOwned))
        return;
    FixedSpanOpcode timing[LUFIA2_BATTLE_PARTY_INPUT_MAX_OPCODES];
    uint32_t stop;
    const unsigned count = PreparePartyInputClock(cpu, timing, &stop);
    if (Lufia2DecompFrameSpanBudget(entry, 0x81cd3bu, ActorBridgePartyInputStep) < count)
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, 10u, &fetch);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x810000u, .timing = timing,
        .count = count, .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = Lufia2BattlePartyInputStep(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == stop);
    assert(clock.index == clock.count && !clock.remaining);
    assert(cpu->master_cycles - start <= PARTY_INPUT_CLOCK_BUDGET);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
