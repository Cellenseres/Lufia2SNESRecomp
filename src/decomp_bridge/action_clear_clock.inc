#include "action_clear_clock.h"
#if defined(LUFIA2_HAVE_NATIVE_ACTION_CLEAR_SPANS)
#include "lufia2/battle_action_work_steps.h"
#endif

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
    unsigned words = 1u;
#if defined(LUFIA2_HAVE_NATIVE_ACTION_CLEAR_SPANS)
    words = Lufia2DecompFrameSpanBudget(entry, entry + 8u, ActorBridgeActionClearStep) / 4u;
    if (words > LUFIA2_BATTLE_ACTION_CLEAR_MAX_WORDS)
        words = LUFIA2_BATTLE_ACTION_CLEAR_MAX_WORDS;
    if (words > cpu->X / 2u + 1u)
        words = cpu->X / 2u + 1u;
#endif
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (deadline <= cpu->master_cycles)
        return;
    uint64_t room = deadline - cpu->master_cycles;
    if (room > 1363u)
        room = 1363u;
    const unsigned iteration = 8u * cpu_region_speed(entry) + 34u;
    const unsigned timed_words = room > 40u ? (unsigned)((room - 40u) / iteration) : 0u;
    if (words > timed_words)
        words = timed_words;
    const unsigned budget = words * iteration + 40u;
    if (!words)
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles) ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !NativeOpcodeWindowWithDeferredHdmaSupported(cpu, entry, budget, 0x1fffu, true, true))
        return;
    const uint16_t pc = (uint16_t)entry;
#if defined(LUFIA2_HAVE_NATIVE_ACTION_CLEAR_SPANS)
    FixedSpanOpcode timing[4u * LUFIA2_BATTLE_ACTION_CLEAR_MAX_WORDS];
    _Static_assert(4u * LUFIA2_BATTLE_ACTION_CLEAR_MAX_WORDS <= 32u,
        "Action clear progress mask");
#else
    FixedSpanOpcode timing[4u];
#endif
    for (unsigned index = 0u; index < words; ++index) {
        FixedSpanOpcode *step = timing + 4u * index;
        step[0] = (FixedSpanOpcode){pc, 4u, 6u, 2u};
        step[1] = (FixedSpanOpcode){(uint16_t)(pc + 4u), 1u, 2u, 0u};
        step[2] = (FixedSpanOpcode){(uint16_t)(pc + 5u), 1u, 2u, 0u};
        step[3] = (FixedSpanOpcode){(uint16_t)(pc + 6u), 2u,
            cpu->X == 2u * index ? 2u : 3u, 0u};
    }
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, 8u, &fetch);
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x850000u;
    clock.timing = timing;
    clock.count = 4u * words;
    clock.opcode_span = prepared ? &fetch : NULL;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
#if defined(LUFIA2_HAVE_NATIVE_ACTION_CLEAR_SPANS)
    const Lufia2ExecutionResult result =
        Lufia2BattleClearActionWordSpan(&memory, &state, entry, words);
#else
    const Lufia2ExecutionResult result = Lufia2BattleClearActionWordStep(&memory, &state, entry);
#endif
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
