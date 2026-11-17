#include "lufia2/battle_command_steps.h"

static void ActorBridgeBattleCommandLoop(CpuState *cpu, uint32_t entry);

static bool BattleCommandLoopHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return pc == 0x81cbb2u && hook == ActorBridgeBattleCommandLoop;
}

static void ActorBridgeBattleCommandLoop(CpuState *cpu, uint32_t entry) {
    if (!BattleCommandLoopHookOwned(entry, ActorBridgeBattleCommandLoop) ||
        cpu != &g_cpu || cpu->PB != 0x81u || cpu->x_flag ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - 160u ||
        !NativeOpcodeWindowSupported(cpu, entry, 160u, 0x1ffcu, true) ||
        !Lufia2DecompFrameRangeAllows(entry, 0x81cbb8u, BattleCommandLoopHookOwned) ||
        Lufia2DecompFrameSpanBudget(entry, 0x81cbb8u,
            ActorBridgeBattleCommandLoop) < LUFIA2_BATTLE_COMMAND_LOOP_OPCODES)
        return;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || (current && current != cpu->master_cycles))
        return;
    const uint64_t start = cpu->master_cycles;
    FixedSpanOpcode timing[LUFIA2_BATTLE_COMMAND_LOOP_OPCODES] = {
        {0xcbb2u, 1u, 5u, 2u}, {0xcbb3u, 3u, 3u, 0u},
        {0xcbb6u, 2u, 2u, 0u}
    };
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x810000u,
        .timing = timing, .count = LUFIA2_BATTLE_COMMAND_LOOP_OPCODES};
    const Lufia2Memory memory = {FixedSpanRead, NULL, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleContinueCommandTiles(&memory, &state);
    timing[2].cycles = state.zero ? 2u : 3u;
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY &&
        (result.pc == 0x81cb99u || result.pc == 0x81cbb8u));
    ActorBridgeStore(cpu, &state);
    assert(cpu->master_cycles - start <= 160u);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
