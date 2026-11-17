#include "lufia2/battle_display_steps.h"

static void ActorBridgeBattleDisplayBlock(CpuState *cpu, uint32_t entry);

static bool BattleDisplayHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return (pc == 0x859c68u || pc == 0x859cc4u) &&
        hook == ActorBridgeBattleDisplayBlock;
}

static bool BattleDisplayClockSupported(const CpuState *cpu, uint32_t entry) {
    if (!BattleDisplayHookOwned(entry, ActorBridgeBattleDisplayBlock) ||
        cpu != &g_cpu || !cpu->ram || cpu->D || cpu->_flag_D ||
        cpu->PB != 0x85u || cpu->DB != 0x7eu || cpu->m_flag || cpu->x_flag ||
        cpu->Y > 90u || cpu->Y % 6u || cpu->S < 0x1f20u || cpu->S > 0x1ff3u ||
        (cpu->host_return_valid && cpu->host_return_valid != 2u && cpu->host_return_valid != 3u) ||
        g_interp_bridge_pc_hook || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - 320u ||
        !NativeOpcodeWindowSupported(cpu, entry, 320u, 0x1ff3u, true) ||
        g_snes->cart->romSize <= 0x29cd6u)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static void ActorBridgeBattleDisplayBlock(CpuState *cpu, uint32_t entry) {
    if (!BattleDisplayClockSupported(cpu, entry) ||
        Lufia2DecompFrameSpanBudget(entry, entry + 18u,
            ActorBridgeBattleDisplayBlock) < LUFIA2_BATTLE_DISPLAY_STEP_OPCODES)
        return;
    FixedSpanOpcode timing[LUFIA2_BATTLE_DISPLAY_STEP_OPCODES];
    for (unsigned i = 0u; i < LUFIA2_BATTLE_DISPLAY_STEP_OPCODES; ++i)
        timing[i] = (FixedSpanOpcode){(uint16_t)(entry + i * 3u), 3u,
            i & 1u ? 7u : 3u, i & 1u ? 2u : 0u};
    const uint64_t start = cpu->master_cycles;
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x850000u,
        .timing = timing, .count = LUFIA2_BATTLE_DISPLAY_STEP_OPCODES};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleFinishDisplayBlock(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == LUFIA2_BATTLE_DISPLAY_STEP_OPCODES && !clock.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == entry + 18u);
    ActorBridgeStore(cpu, &state);
    assert(cpu->master_cycles - start <= 320u);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
