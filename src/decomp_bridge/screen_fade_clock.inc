#include "screen_fade_timing.inc"

enum { SCREEN_FADE_MAX_OPCODES = 26u, SCREEN_FADE_CLOCK_RESERVE = 640u };

typedef struct ScreenFadeClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[SCREEN_FADE_MAX_OPCODES];
} ScreenFadeClock;

static void PrepareScreenFadeClock(ScreenFadeClock *clock, CpuState *cpu) {
    for (unsigned i = 0u; i < 14u; ++i)
        clock->timing[i] = screen_fade_common_timing[i];
    unsigned count = 2u;
    const uint8_t control = cpu->ram[0x0581u];
    if (!(control & 0x80u)) {
        clock->timing[1].cycles = 3u;
    } else {
        const bool fading_out = (control & 0x40u) != 0u;
        const uint8_t next_level = cpu->ram[0x0582u] + (control & 0x3fu);
        const bool finished = (next_level >> 3u) >= 15u;
        const FixedSpanOpcode *tail = fading_out ? screen_fade_out_timing : screen_fade_in_timing;
        const unsigned tail_count = fading_out ? (finished ? 12u : 8u)
                                               : (finished ? 7u : 4u);
        clock->timing[13].cycles += !fading_out;
        for (unsigned i = 0u; i < tail_count; ++i)
            clock->timing[14u + i] = tail[i];
        clock->timing[fading_out ? 21u : 17u].cycles += !finished;
        count = 14u + tail_count;
    }
    clock->span.cpu = cpu;
    clock->span.bank = 0x800000u;
    clock->span.timing = clock->timing;
    clock->span.count = count;
}

static bool ScreenFadeClockSupported(CpuState *cpu, ScreenFadeClock *clock) {
    if (!g_snes || !g_snes->inNmi || g_snes->inIrq || !g_snes->dma || !g_snes->apu ||
        cpu != &g_cpu || !cpu->ram || cpu->ram != g_snes->ram || cpu->emulation ||
        cpu->PB != 0x80u || cpu->DB != 0x80u || cpu->D || cpu->_flag_D ||
        cpu->m_flag != 1u || cpu->x_flag > 1u ||
        cpu->S < 0x1f00u || cpu->S > 0x1ff3u || cpu->host_return_valid != 2u ||
        !Lufia2DecompFramePairedCallReady(cpu, 0x8086c1u) || g_interp_apu_driving ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        g_snes->beamMasterLast != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - SCREEN_FADE_CLOCK_RESERVE ||
        g_snes->hIrqEnabled || g_snes->vIrqEnabled ||
        g_snes->dma->dmaBusy || g_snes->dma->dmaTimer || g_snes->dma->hdmaPendingInit ||
        g_snes->vPos >= 262u || g_snes->hPos >= 1364u || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize < 0x0703u)
        return false;
    for (unsigned channel = 0u; channel < 8u; ++channel)
        if (g_snes->dma->channel[channel].dmaActive)
            return false;
    PrepareScreenFadeClock(clock, cpu);
    unsigned budget = cpu_region_speed(0x808702u) + 2u * 8u + 3u * 6u + 40u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *timing = clock->timing + i;
        budget += timing->fetched * cpu_region_speed(0x800000u | timing->pc) +
            timing->data_bytes * 8u +
            (timing->cycles - timing->fetched - timing->data_bytes) * 6u;
    }
    assert(budget < SCREEN_FADE_CLOCK_RESERVE);
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    if ((unsigned)g_snes->hPos + budget >= fence)
        return false;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (deadline && (deadline <= cpu->master_cycles ||
        deadline - cpu->master_cycles < budget))
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles) &&
#if defined(LUFIA2_HAVE_NATIVE_SCREEN_FADE_STEPS)
        Lufia2DecompFrameRangeAllows(0x8086c1u, 0x808703u, ScreenFadeClockHookOwned);
#elif defined(LUFIA2_HAVE_NATIVE_SCREEN_FADE_REQUEST)
        Lufia2DecompFrameRangeAllows(0x8086c1u, 0x808703u, ScreenFadeRequestHookOwned);
#else
        Lufia2DecompFrameRangeQuiet(0x8086c1u, 0x808703u);
#endif
}

static RecompReturn ActorBridgeTimedScreenFade(CpuState *cpu) {
    ScreenFadeClock clock = {0};
    if (!ScreenFadeClockSupported(cpu, &clock)) {
        const uint32_t entry = (uint32_t)cpu->PB << 16 | 0x86c1u;
        return interp_tier_dispatch_tail(cpu, entry, entry, cpu->S, cpu->host_return_valid);
    }
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {NmiClockRead, NmiClockWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2ScreenFade(&memory, &state);
    NmiClockBeginOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x808702u);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
