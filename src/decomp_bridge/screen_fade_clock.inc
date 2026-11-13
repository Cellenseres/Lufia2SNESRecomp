enum { SCREEN_FADE_MAX_OPCODES = 26u, SCREEN_FADE_CLOCK_RESERVE = 640u };

typedef struct ScreenFadeClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[SCREEN_FADE_MAX_OPCODES];
} ScreenFadeClock;

static void PrepareScreenFadeClock(ScreenFadeClock *clock, CpuState *cpu) {
    static const FixedSpanOpcode common[] = {
        {0x86c1u, 3u, 4u, 1u}, {0x86c4u, 2u, 2u, 0u},
        {0x86c6u, 1u, 3u, 0u}, {0x86c7u, 3u, 4u, 1u},
        {0x86cau, 2u, 2u, 0u}, {0x86ccu, 1u, 2u, 0u},
        {0x86cdu, 3u, 4u, 1u}, {0x86d0u, 3u, 4u, 1u},
        {0x86d3u, 1u, 2u, 0u}, {0x86d4u, 1u, 2u, 0u},
        {0x86d5u, 1u, 2u, 0u}, {0x86d6u, 1u, 3u, 0u},
        {0x86d7u, 2u, 2u, 0u}, {0x86d9u, 2u, 2u, 0u}
    };
    static const FixedSpanOpcode fade_in[] = {
        {0x86f2u, 1u, 3u, 0u}, {0x86f3u, 3u, 4u, 1u},
        {0x86f6u, 2u, 2u, 0u}, {0x86f8u, 2u, 2u, 0u},
        {0x86fau, 3u, 4u, 1u}, {0x86fdu, 2u, 2u, 0u},
        {0x86ffu, 3u, 4u, 1u}
    };
    static const FixedSpanOpcode fade_out[] = {
        {0x86dbu, 1u, 3u, 0u}, {0x86dcu, 1u, 2u, 0u},
        {0x86ddu, 2u, 2u, 0u}, {0x86dfu, 2u, 2u, 0u},
        {0x86e1u, 1u, 2u, 0u}, {0x86e2u, 3u, 4u, 1u},
        {0x86e5u, 1u, 2u, 0u}, {0x86e6u, 2u, 2u, 0u},
        {0x86e8u, 3u, 4u, 1u}, {0x86ebu, 2u, 2u, 0u},
        {0x86edu, 3u, 4u, 1u}, {0x86f0u, 2u, 3u, 0u}
    };
    for (unsigned i = 0u; i < 14u; ++i)
        clock->timing[i] = common[i];
    unsigned count = 2u;
    const uint8_t control = cpu->ram[0x0581u];
    if (!(control & 0x80u)) {
        clock->timing[1].cycles = 3u;
    } else {
        const bool fading_out = (control & 0x40u) != 0u;
        const uint8_t next_level = cpu->ram[0x0582u] + (control & 0x3fu);
        const bool finished = (next_level >> 3u) >= 15u;
        const FixedSpanOpcode *tail = fading_out ? fade_out : fade_in;
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
        Lufia2DecompFrameRangeQuiet(0x8086c1u, 0x808703u);
}

static RecompReturn ActorBridgeTimedScreenFade(CpuState *cpu) {
    ScreenFadeClock clock = {0};
    if (!ScreenFadeClockSupported(cpu, &clock))
        return interp_tier_dispatch_tail(cpu, 0x8086c1u, 0x8086c1u,
            cpu->S, cpu->host_return_valid);
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
