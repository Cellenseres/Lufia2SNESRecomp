enum { NMI_SCROLL_MAX_OPCODES = 100u, NMI_SCROLL_CLOCK_RESERVE = 4096u };

typedef struct NmiScrollClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[NMI_SCROLL_MAX_OPCODES];
} NmiScrollClock;

static void PrepareNmiScrollClock(NmiScrollClock *clock, CpuState *cpu,
                                 bool tilemaps_only) {
    static const FixedSpanOpcode tilemaps[] = {
        {0x87fcu, 2u, 3u, 1u}, {0x87feu, 2u, 2u, 0u},
        {0x8800u, 2u, 3u, 0u}, {0x880du, 2u, 2u, 0u},
        {0x880fu, 2u, 3u, 0u}, {0x881cu, 2u, 2u, 0u},
        {0x881eu, 2u, 3u, 0u}, {0x8829u, 2u, 2u, 0u},
        {0x882bu, 2u, 5u, 2u}
    };
    unsigned count = 0u;
    if (!tilemaps_only) {
        static const FixedSpanOpcode setup[] = {
            {0x87a7u, 1u, 3u, 1u}, {0x87a8u, 2u, 3u, 0u},
            {0x87aau, 2u, 3u, 0u}, {0x87acu, 3u, 3u, 0u},
            {0x87afu, 3u, 3u, 0u}
        };
        static const FixedSpanOpcode scroll_pair[] = {
            {0x87b2u, 3u, 4u, 1u}, {0x87b5u, 2u, 4u, 1u},
            {0x87b7u, 3u, 4u, 1u}, {0x87bau, 2u, 4u, 1u},
            {0x87bcu, 1u, 2u, 0u}, {0x87bdu, 1u, 2u, 0u},
            {0x87beu, 1u, 2u, 0u}, {0x87bfu, 3u, 3u, 0u},
            {0x87c2u, 2u, 3u, 0u}
        };
        for (unsigned i = 0u; i < 5u; ++i)
            clock->timing[count++] = setup[i];
        for (unsigned scroll = 0u; scroll < 8u; ++scroll)
            for (unsigned i = 0u; i < 9u; ++i)
                clock->timing[count++] = scroll_pair[i];
        --clock->timing[count - 1u].cycles;
        for (unsigned request = 0u; request < 4u; ++request) {
            const uint16_t pc = (uint16_t)(0x87c4u + request * 11u);
            clock->timing[count++] = (FixedSpanOpcode){pc, 2u, 3u, 1u};
            clock->timing[count++] = (FixedSpanOpcode){pc + 2u, 2u, 3u, 0u};
        }
        clock->timing[count++] = (FixedSpanOpcode){0x87f0u, 3u, 6u, 2u};
    }
    for (unsigned i = 0u; i < 9u; ++i)
        clock->timing[count++] = tilemaps[i];
    if (!tilemaps_only) {
        clock->timing[count++] = (FixedSpanOpcode){0x882du, 1u, 6u, 2u};
        clock->timing[count++] = (FixedSpanOpcode){0x87f3u, 2u, 3u, 1u};
        const bool enable_hdma = cpu->ram[0x81u] != 0u;
        clock->timing[count++] = (FixedSpanOpcode){0x87f5u, 2u,
            (uint8_t)(enable_hdma ? 2u : 3u), 0u};
        if (enable_hdma)
            clock->timing[count++] = (FixedSpanOpcode){0x87f7u, 3u, 4u, 1u};
        clock->timing[count++] = (FixedSpanOpcode){0x87fau, 1u, 4u, 1u};
    }
    assert(count <= NMI_SCROLL_MAX_OPCODES);
    clock->span.cpu = cpu;
    clock->span.bank = 0x800000u;
    clock->span.timing = clock->timing;
    clock->span.count = count;
}

static bool NmiScrollClockSupported(CpuState *cpu, bool tilemaps_only,
                                   NmiScrollClock *clock) {
    const uint32_t entry = tilemaps_only ? 0x8087fcu : 0x8087a7u;
    if (!g_snes || !g_snes->inNmi || g_snes->inIrq || !g_snes->dma || !g_snes->apu ||
        cpu != &g_cpu || !cpu->ram || cpu->ram != g_snes->ram || cpu->emulation ||
        cpu->PB != 0x80u || cpu->DB != 0x80u || cpu->D || cpu->_flag_D ||
        cpu->m_flag > 1u || cpu->x_flag > 1u ||
        (tilemaps_only && (cpu->m_flag != 1u || cpu->x_flag)) ||
        cpu->S < 0x1f04u || cpu->S > 0x1ff3u || cpu->host_return_valid != 2u ||
        !Lufia2DecompFramePairedCallReady(cpu, entry) || g_interp_apu_driving ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        g_snes->beamMasterLast != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - NMI_SCROLL_CLOCK_RESERVE ||
        g_snes->hIrqEnabled || g_snes->vIrqEnabled || !g_snes->hdmaBeamOff ||
        g_snes->dma->dmaBusy || g_snes->dma->dmaTimer || g_snes->dma->hdmaPendingInit ||
        g_snes->vPos >= 262u || g_snes->hPos >= 1364u || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize < 0x082eu)
        return false;
    for (unsigned channel = 0u; channel < 8u; ++channel)
        if (g_snes->dma->channel[channel].dmaActive)
            return false;
    if (cpu->ram[0x74u] & 0xfcu)
        return false;
    if (!tilemaps_only)
        for (unsigned request = 0u; request < 4u; ++request)
            if (cpu->ram[0x75u + request])
                return false;
    PrepareNmiScrollClock(clock, cpu, tilemaps_only);
    const uint32_t return_pc = tilemaps_only ? 0x80882du : 0x8087fbu;
    unsigned budget = cpu_region_speed(return_pc) + 2u * 8u + 3u * 6u +
        (tilemaps_only ? 40u : 120u);
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *timing = clock->timing + i;
        budget += timing->fetched * cpu_region_speed(0x800000u | timing->pc) +
            timing->data_bytes * 8u +
            (timing->cycles - timing->fetched - timing->data_bytes) * 6u;
    }
    assert(budget < NMI_SCROLL_CLOCK_RESERVE);
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < budget)
        return false;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (deadline && (deadline <= cpu->master_cycles ||
        deadline - cpu->master_cycles < budget))
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles) &&
        Lufia2DecompFrameRangeQuiet(entry, 0x80882eu);
}

static RecompReturn ActorBridgeTimedNmiScroll(CpuState *cpu, bool tilemaps_only) {
    const uint32_t entry = tilemaps_only ? 0x8087fcu : 0x8087a7u;
    NmiScrollClock clock = {0};
    if (!NmiScrollClockSupported(cpu, tilemaps_only, &clock)) {
        const uint32_t original = (uint32_t)cpu->PB << 16 | (entry & 0xffffu);
        return interp_tier_dispatch_tail(cpu, original, original, cpu->S, cpu->host_return_valid);
    }
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {NmiClockRead, NmiClockWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = tilemaps_only
        ? Lufia2NmiTilemapUploads(&memory, &state)
        : Lufia2NmiScrollAndUploads(&memory, &state);
    NmiClockBeginOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED &&
        result.pc == (tilemaps_only ? 0x80882du : 0x8087fbu));
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
