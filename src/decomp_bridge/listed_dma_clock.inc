enum { LISTED_DMA_CPU_RESERVE = 512u };

static const FixedSpanOpcode listed_dma_vram_timing[] = {
    {0x884fu, 1u, 3u, 1u}, {0x8850u, 2u, 2u, 0u},
    {0x8852u, 2u, 2u, 0u}, {0x8854u, 2u, 2u, 0u},
    {0x8856u, 3u, 5u, 2u}, {0x8859u, 2u, 3u, 0u},
    {0x885fu, 1u, 4u, 1u}, {0x8860u, 2u, 2u, 0u},
    {0x8862u, 3u, 4u, 1u}
};

static const FixedSpanOpcode listed_dma_cgram_timing[] = {
    {0x884fu, 1u, 3u, 1u}, {0x8850u, 2u, 2u, 0u},
    {0x8852u, 2u, 2u, 0u}, {0x8854u, 2u, 3u, 0u},
    {0x885bu, 1u, 2u, 0u}, {0x885cu, 3u, 4u, 1u},
    {0x885fu, 1u, 4u, 1u}, {0x8860u, 2u, 2u, 0u},
    {0x8862u, 3u, 4u, 1u}
};

static unsigned ListedDmaByteCount(const DmaChannel *channel) {
    return channel->size ? channel->size : 0x10000u;
}

static bool ListedDmaTouchesStack(const DmaChannel *channel) {
    const uint16_t address = channel->aAdr;
    if (address >= 0x1f00u && address <= 0x1fffu)
        return true;
    if (channel->fixed)
        return false;
    const uint16_t distance = channel->decrement
        ? (uint16_t)(address - 0x1fffu) : (uint16_t)(0x1f00u - address);
    return distance < ListedDmaByteCount(channel);
}

static bool ListedDmaChannelSupported(const DmaChannel *channel) {
    if (channel->mode > 7u || channel->bAdr > 0x3cu)
        return false;
    if (channel->fromB) {
        /* Stack writes keep the original control-transfer route. */
        return channel->aBank == 0x7fu ||
            (channel->aBank == 0x7eu && !ListedDmaTouchesStack(channel));
    }
    if (channel->aBank == 0x7eu || channel->aBank == 0x7fu)
        return true;
    if (channel->aAdr < 0x8000u)
        return false;
    if (channel->fixed)
        return true;
    const unsigned room = channel->decrement
        ? (unsigned)channel->aAdr - 0x8000u + 1u : 0x10000u - channel->aAdr;
    return ListedDmaByteCount(channel) <= room;
}

static bool ListedDmaClockSupported(const CpuState *cpu, unsigned *reserve) {
    if (!g_snes || !g_snes->inNmi || g_snes->inIrq || !g_snes->dma || !g_snes->apu ||
        cpu != &g_cpu || !cpu->ram || cpu->ram != g_snes->ram || cpu->emulation ||
        cpu->PB != 0x80u || cpu->DB != 0x80u || cpu->D || cpu->_flag_D ||
        cpu->m_flag != 1u || cpu->x_flag ||
        cpu->S < 0x1f00u || cpu->S > 0x1ff3u || cpu->host_return_valid != 2u ||
        !Lufia2DecompFramePairedCallReady(cpu, 0x80884fu) || g_interp_apu_driving ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        g_snes->beamMasterLast != cpu->master_cycles ||
        g_snes->hIrqEnabled || g_snes->vIrqEnabled || !g_snes->hdmaBeamOff ||
        g_snes->dma->dmaBusy || g_snes->dma->dmaTimer || g_snes->dma->hdmaPendingInit ||
        g_snes->vPos >= 262u || g_snes->hPos >= 1364u || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize < 0x0866u)
        return false;
    for (unsigned channel = 0u; channel < 8u; ++channel)
        if (g_snes->dma->channel[channel].dmaActive)
            return false;
    unsigned budget = LISTED_DMA_CPU_RESERVE + 12u;
    const unsigned mask = cpu->A & 0x3fu;
    for (unsigned channel = 0u; channel < 6u; ++channel) {
        if (!(mask & (1u << channel)))
            continue;
        const DmaChannel *transfer = &g_snes->dma->channel[channel];
        if (!ListedDmaChannelSupported(transfer))
            return false;
        budget += 8u + ListedDmaByteCount(transfer) * 8u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (cpu->master_cycles > UINT64_MAX - budget)
        return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < budget)
        return false;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (deadline && (deadline <= cpu->master_cycles ||
        deadline - cpu->master_cycles < budget))
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || (current && current != cpu->master_cycles) ||
        !Lufia2DecompFrameRangeQuiet(0x80884fu, 0x808866u))
        return false;
    *reserve = budget;
    return true;
}

static RecompReturn ActorBridgeTimedListedDma(CpuState *cpu) {
    unsigned reserve;
    if (!ListedDmaClockSupported(cpu, &reserve)) {
        const uint32_t entry = (uint32_t)cpu->PB << 16 | 0x884fu;
        return interp_tier_dispatch_tail(cpu, entry, entry, cpu->S, cpu->host_return_valid);
    }
    const uint64_t start = cpu->master_cycles;
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const bool cgram = (cpu->A & 0xc0u) == 0x80u;
    NmiDmaClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = 0x800000u;
    clock.span.timing = cgram ? listed_dma_cgram_timing : listed_dma_vram_timing;
    clock.span.count = sizeof listed_dma_vram_timing / sizeof *listed_dma_vram_timing;
    const Lufia2Memory memory = {NmiDmaRead, NmiDmaWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2StartListedDma(&memory, &state);
    NmiClockBeginOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x808865u);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    const RecompReturn returned = ReturnTimedMenuCall(cpu, &frame, result.pc);
    assert(cpu->master_cycles - start <= reserve);
    return returned;
}
