#include "action_clear_clock.h"

static void ActorBridgeActionClearStep(CpuState *cpu, uint32_t entry);

bool Lufia2DecompActionClearStepHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    /* The hook table shares the high and low LoROM aliases. */
    return (pc == 0x85ccd7u || pc == 0x85ccecu ||
        pc == 0x05ccd7u || pc == 0x05ccecu) && hook == ActorBridgeActionClearStep;
}

static void ActorBridgeActionClearStep(CpuState *cpu, uint32_t entry) {
    if (!Lufia2DecompActionClearStepHookOwned(entry, ActorBridgeActionClearStep) ||
        cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag || cpu->_flag_D ||
        cpu->X > 0x82u || (cpu->X & 1u) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        Lufia2DecompFrameSpanBudget(entry, entry + 8u, ActorBridgeActionClearStep) < 4u)
        return;
    const unsigned budget = 8u * cpu_region_speed(entry) + 2u * 8u + 3u * 6u + 40u;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles) ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !NativeOpcodeWindowSupported(cpu, entry, budget, 0x1fffu, true))
        return;
    const uint16_t pc = (uint16_t)entry;
    const FixedSpanOpcode timing[] = {
        {pc, 4u, 6u, 2u}, {(uint16_t)(pc + 4u), 1u, 2u, 0u},
        {(uint16_t)(pc + 5u), 1u, 2u, 0u},
        {(uint16_t)(pc + 6u), 2u, cpu->X ? 3u : 2u, 0u}
    };
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x850000u;
    clock.timing = timing;
    clock.count = 4u;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleClearActionWordStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
