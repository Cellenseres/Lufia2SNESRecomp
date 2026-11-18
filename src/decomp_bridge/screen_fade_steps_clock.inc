#include "lufia2/screen_fade_steps.h"
#include "screen_fade_timing.inc"

static void ActorBridgeScreenFadeStep(CpuState *cpu, uint32_t entry);

static uint32_t ScreenFadeStepHookEnd(uint32_t pc, InterpPreOpcodeHook hook) {
    if (hook != ActorBridgeScreenFadeStep)
        return 0u;
    switch (pc & 0x7fffffu) {
    case 0x0086c6u: return pc + 0x15u;
    case 0x0086dbu: return pc + 0x17u;
    case 0x0086f2u: return pc + 0x10u;
    default: return 0u;
    }
}

static bool ScreenFadeStepHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return ScreenFadeStepHookEnd(pc, hook) != 0u;
}

static bool ScreenFadeClockHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return ScreenFadeStepHookOwned(pc, hook)
#if defined(LUFIA2_HAVE_NATIVE_SCREEN_FADE_REQUEST)
        || ScreenFadeRequestHookOwned(pc, hook)
#endif
        ;
}

static unsigned PrepareScreenFadeStepClock(
    const CpuState *cpu, uint32_t entry, FixedSpanOpcode *timing) {
    const FixedSpanOpcode *source;
    unsigned count;
    bool finished = false;
    if (entry == 0x8086c6u) {
        source = screen_fade_common_timing + 2u;
        count = LUFIA2_NMI_FADE_ACCUMULATE_OPCODES;
    } else if (entry == 0x8086dbu) {
        const uint8_t remaining_brightness = (uint8_t)(0x0fu - (cpu->A >> 8u));
        finished = ((uint8_t)(remaining_brightness - 1u) & 0x80u) != 0u;
        source = screen_fade_out_timing;
        count = finished ? LUFIA2_NMI_FADE_OUT_OPCODES : 8u;
    } else {
        finished = (cpu->A >> 8u) >= 0x0fu;
        source = screen_fade_in_timing;
        count = finished ? LUFIA2_NMI_FADE_IN_OPCODES : 4u;
    }
    memcpy(timing, source, count * sizeof timing[0]);
    if (entry == 0x8086c6u)
        timing[count - 1u].cycles += !(cpu->A & 0x40u);
    else if (!finished)
        ++timing[count - 1u].cycles;
    return count;
}

static void ActorBridgeScreenFadeStep(CpuState *cpu, uint32_t entry) {
    const uint32_t end = ScreenFadeStepHookEnd(entry, ActorBridgeScreenFadeStep);
    const unsigned budget = entry == 0x8086f2u ? 256u : 352u;
    if (!end || entry < 0x800000u || cpu != &g_cpu || cpu->m_flag != 1u ||
        !g_snes || !g_snes->apu || g_snes->vPos >= 262u ||
        !NmiClockWindowWithDeferredHdma(cpu, cpu->x_flag != 0u, entry, end,
            budget, budget, true, ScreenFadeStepHookOwned, true))
        return;
    FixedSpanOpcode timing[LUFIA2_SCREEN_FADE_STEPS_MAX_OPCODES];
    const unsigned count = PrepareScreenFadeStepClock(cpu, entry, timing);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x800000u, .timing = timing,
        .count = count};
    const Lufia2Memory memory = {NmiClockRead, NmiClockWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = Lufia2ScreenFadeStep(&memory, &state, entry);
    NmiClockBeginOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY &&
        (result.pc == 0x8086dbu || result.pc == 0x8086f2u || result.pc == 0x808702u));
    assert(cpu->master_cycles - start <= budget);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
