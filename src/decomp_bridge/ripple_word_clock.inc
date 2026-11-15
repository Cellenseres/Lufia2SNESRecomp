#include "lufia2/battle_ripple_steps.h"
#include "nmi_owner.h"

static void ActorBridgeRippleWordClock(CpuState *cpu, uint32_t entry);

static bool RippleWordRowHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return pc == 0x85aa61u && hook == ActorBridgeRippleWordClock;
}

static bool RippleWordClockReady(CpuState *cpu, uint32_t entry) {
    if (entry != 0x85aa61u || cpu->PB != 0x85u || cpu->emulation ||
        cpu->m_flag || !cpu->x_flag || cpu->X >= 0xa8u || (cpu->X & 1u) ||
        !cpu->Y || cpu->Y > 12u || !cpu->ram || !g_snes ||
        g_snes->ram != cpu->ram || g_snes->vPos >= 262u ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) || !Lufia2DecompFrameReturnCpuReady())
        return false;
    const bool next_run = cpu->Y == 1u;
    const unsigned count = next_run ? 10u : 7u;
    const unsigned fetched = next_run ? 19u : 13u;
    const unsigned master = fetched * cpu_region_speed(entry) + 46u -
        (cpu->X == 0xa6u ? 6u : 0u);
    const unsigned budget = master + 40u;
    if (cpu->master_cycles > UINT64_MAX - budget)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || (current && current != cpu->master_cycles))
        return false;
    if (g_snes->inNmi) {
#if defined(LUFIA2_HAVE_NATIVE_NMI_RESTORE_WIDTH)
        if (!Lufia2DecompNmiHasInterruptOwner() || cpu->DB != 0x85u ||
            cpu->D || cpu->_flag_D || cpu->S < 0x1f00u || cpu->S > 0x1ffbu ||
            !g_snes->dma || g_snes->inIrq || g_snes->hIrqEnabled ||
            g_snes->vIrqEnabled || g_snes->hPos >= 1364u ||
            g_snes->beamMasterLast != cpu->master_cycles ||
            g_snes->dma->dmaBusy || g_snes->dma->dmaTimer ||
            cpu->ram[cpu->S + 3u] != 0x3bu || cpu->ram[cpu->S + 4u] != 0x8fu ||
            current != cpu->master_cycles)
            return false;
        for (unsigned channel = 0u; channel < 8u; ++channel)
            if (g_snes->dma->channel[channel].dmaActive)
                return false;
        const unsigned next_line = g_snes->vPos < 225u ? 225u : 262u;
        const uint64_t before_edge =
            (uint64_t)(next_line - g_snes->vPos) * 1364u - g_snes->hPos;
        return budget < before_edge && NmiSpanAvoidsHdma(g_snes, budget, true) &&
            Lufia2DecompFrameRangeAllows(entry, 0x85aa74u, RippleWordRowHookOwned);
#else
        return false;
#endif
    }
    if (Lufia2DecompFrameOwnedSpanBudget(entry, 0x85aa74u,
            ActorBridgeRippleWordClock) < count)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u &&
        g_snes->hPos < 1024u ? 1024u : 1364u;
    return ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu, true,
        fence - budget - 1u, budget);
}

static void ActorBridgeRippleWordClock(CpuState *cpu, uint32_t entry) {
    if (!RippleWordClockReady(cpu, entry))
        return;
    FixedSpanOpcode timing[10] = {
        {0xaa61u, 4u, 6u, 2u}, {0xaa65u, 1u, 2u, 0u},
        {0xaa66u, 2u, cpu->Y == 1u ? 2u : 3u, 0u}
    };
    unsigned count = 3u;
    if (cpu->Y == 1u) {
        timing[count++] = (FixedSpanOpcode){0xaa68u, 2u, 2u, 0u};
        timing[count++] = (FixedSpanOpcode){0xaa6au, 1u, 2u, 0u};
        timing[count++] = (FixedSpanOpcode){0xaa6bu, 3u, 3u, 0u};
    }
    timing[count++] = (FixedSpanOpcode){0xaa6eu, 1u, 2u, 0u};
    timing[count++] = (FixedSpanOpcode){0xaa6fu, 1u, 2u, 0u};
    timing[count++] = (FixedSpanOpcode){0xaa70u, 2u, 2u, 0u};
    timing[count++] = (FixedSpanOpcode){0xaa72u, 2u,
        cpu->X == 0xa6u ? 2u : 3u, 0u};
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x850000u;
    clock.timing = timing;
    clock.count = count;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleRippleWordRowStep(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}

#if defined(LUFIA2_HAVE_NATIVE_RIPPLE_WORD_PHASES)
#include "ripple_word_clock.h"

static void ActorBridgeRippleWordPhaseClock(CpuState *cpu, uint32_t entry);

uint32_t Lufia2DecompRippleWordPhaseHookEnd(uint32_t pc, InterpPreOpcodeHook hook) {
    if (hook != ActorBridgeRippleWordPhaseClock)
        return 0u;
    return pc == 0x85aa3du ? 0x85aa61u : pc == 0x85aa74u ? 0x85aa7eu : 0u;
}

static bool RippleWordPhaseHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return Lufia2DecompRippleWordPhaseHookEnd(pc, hook) != 0u;
}

static bool RippleWordPhaseClockReady(CpuState *cpu, uint32_t entry) {
    const bool prepare = entry == 0x85aa3du;
    const unsigned reserve = prepare ? 512u : 192u;
    const unsigned return_offset = prepare ? 1u : 3u;
    const uint32_t end = prepare ? 0x85aa61u : 0x85aa7eu;
    if ((!prepare && entry != 0x85aa74u) || cpu != &g_cpu ||
        cpu->PB != 0x85u || cpu->DB != 0x85u || cpu->D || cpu->_flag_D ||
        cpu->emulation || cpu->m_flag != prepare || cpu->x_flag != !prepare ||
        !cpu->ram || !g_snes || g_snes->ram != cpu->ram ||
        !g_snes->inNmi || g_snes->inIrq || g_snes->vPos >= 262u ||
        cpu->S < (prepare ? 0x1f02u : 0x1f00u) || cpu->S > 0x1ffbu ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        !Lufia2DecompNmiHasInterruptOwner() || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !g_snes->dma || !g_snes->apu || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize < 0x2aa7eu ||
        g_snes->hIrqEnabled || g_snes->vIrqEnabled || g_snes->hPos >= 1364u ||
        g_snes->beamMasterLast != cpu->master_cycles ||
        g_snes->dma->dmaBusy || g_snes->dma->dmaTimer ||
        g_snes->dma->hdmaPendingInit || snesrecomp_beam_hold_active() ||
        cpu->ram[cpu->S + return_offset] != 0x3bu ||
        cpu->ram[cpu->S + return_offset + 1u] != 0x8fu ||
        !Lufia2DecompFrameRangeAllows(entry, end, RippleWordPhaseHookOwned))
        return false;
    for (unsigned channel = 0u; channel < 8u; ++channel)
        if (g_snes->dma->channel[channel].dmaActive)
            return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || current != cpu->master_cycles)
        return false;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (deadline && (deadline <= cpu->master_cycles ||
        deadline - cpu->master_cycles < reserve))
        return false;
    const unsigned next_line = g_snes->vPos < 225u ? 225u : 262u;
    const uint64_t before_edge =
        (uint64_t)(next_line - g_snes->vPos) * 1364u - g_snes->hPos;
    return reserve < before_edge && NmiSpanAvoidsHdma(g_snes, reserve, true);
}

static void ActorBridgeRippleWordPhaseClock(CpuState *cpu, uint32_t entry) {
    if (!RippleWordPhaseClockReady(cpu, entry))
        return;
    static const FixedSpanOpcode prepare[] = {
        {0xaa3du, 3u, 4u, 1u}, {0xaa40u, 1u, 4u, 2u},
        {0xaa41u, 2u, 3u, 0u}, {0xaa43u, 1u, 2u, 0u},
        {0xaa44u, 2u, 2u, 0u}, {0xaa46u, 2u, 3u, 1u},
        {0xaa48u, 2u, 2u, 0u}, {0xaa4au, 1u, 2u, 0u},
        {0xaa4bu, 2u, 3u, 1u}, {0xaa4du, 2u, 3u, 1u},
        {0xaa4fu, 1u, 2u, 0u}, {0xaa50u, 2u, 3u, 1u},
        {0xaa52u, 1u, 2u, 0u}, {0xaa53u, 1u, 2u, 0u},
        {0xaa54u, 1u, 2u, 0u}, {0xaa55u, 1u, 2u, 0u},
        {0xaa56u, 2u, 2u, 0u}, {0xaa58u, 1u, 2u, 0u},
        {0xaa59u, 2u, 3u, 0u}, {0xaa5bu, 4u, 6u, 2u},
        {0xaa5fu, 2u, 2u, 0u}
    };
    static const FixedSpanOpcode finish[] = {
        {0xaa74u, 2u, 3u, 0u}, {0xaa76u, 2u, 3u, 0u},
        {0xaa78u, 1u, 5u, 2u}, {0xaa79u, 2u, 2u, 0u},
        {0xaa7bu, 3u, 4u, 1u}
    };
    _Static_assert(sizeof prepare / sizeof *prepare ==
        LUFIA2_NMI_RIPPLE_WORD_PREPARE_OPCODES, "word ripple preparation budget");
    _Static_assert(sizeof finish / sizeof *finish ==
        LUFIA2_NMI_RIPPLE_WORD_FINISH_OPCODES, "word ripple completion budget");
    const bool preparing = entry == 0x85aa3du;
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x850000u,
        .timing = preparing ? prepare : finish,
        .count = preparing ? LUFIA2_NMI_RIPPLE_WORD_PREPARE_OPCODES
                           : LUFIA2_NMI_RIPPLE_WORD_FINISH_OPCODES};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = preparing
        ? Lufia2BattleRippleWordPrepareStep(&memory, &state)
        : Lufia2BattleRippleWordFinishStep(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY &&
        result.pc == (preparing ? 0x85aa61u : 0x85aa7eu));
    assert(cpu->master_cycles - start <= (preparing ? 512u : 192u));
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
#endif
