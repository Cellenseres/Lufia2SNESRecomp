enum { TILEMAP_DMA_CLOCK_RESERVE = 20000u };

static const FixedSpanOpcode tilemap_dma_timing[] = {
    {0x882eu, 3u, 5u, 2u}, {0x8831u, 3u, 5u, 2u},
    {0x8834u, 2u, 2u, 0u}, {0x8836u, 3u, 4u, 1u},
    {0x8839u, 2u, 2u, 0u}, {0x883bu, 3u, 4u, 1u},
    {0x883eu, 2u, 2u, 0u}, {0x8840u, 3u, 4u, 1u},
    {0x8843u, 3u, 3u, 0u}, {0x8846u, 3u, 5u, 2u},
    {0x8849u, 2u, 2u, 0u}, {0x884bu, 3u, 4u, 1u}
};

static bool TilemapDmaClockSupported(const CpuState *cpu) {
    if (!g_snes || !g_snes->inNmi || g_snes->inIrq || !g_snes->dma || !g_snes->apu ||
        cpu != &g_cpu || !cpu->ram || cpu->ram != g_snes->ram || cpu->emulation ||
        cpu->PB != 0x80u || cpu->DB != 0x80u || cpu->D || cpu->_flag_D ||
        cpu->m_flag != 1u || cpu->x_flag ||
        cpu->S < 0x1f00u || cpu->S > 0x1ff3u || cpu->host_return_valid != 2u ||
        (cpu->X != 0x2000u && cpu->X != 0x2800u && cpu->X != 0x3000u) ||
        cpu->Y != (cpu->X - 0x2000u) / 2u ||
        !Lufia2DecompFramePairedCallReady(cpu, 0x80882eu) || g_interp_apu_driving ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        g_snes->beamMasterLast != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - TILEMAP_DMA_CLOCK_RESERVE ||
        g_snes->hIrqEnabled || g_snes->vIrqEnabled || !g_snes->hdmaBeamOff ||
        g_snes->dma->dmaBusy || g_snes->dma->dmaTimer || g_snes->dma->hdmaPendingInit ||
        g_snes->vPos >= 262u || g_snes->hPos >= 1364u || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize < 0x084fu)
        return false;
    for (unsigned channel = 0u; channel < 8u; ++channel)
        if (g_snes->dma->channel[channel].dmaActive)
            return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < TILEMAP_DMA_CLOCK_RESERVE)
        return false;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (deadline && (deadline <= cpu->master_cycles ||
        deadline - cpu->master_cycles < TILEMAP_DMA_CLOCK_RESERVE))
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles) &&
        Lufia2DecompFrameRangeQuiet(0x80882eu, 0x80884fu);
}

static RecompReturn ActorBridgeTimedTilemapDma(CpuState *cpu) {
    if (!TilemapDmaClockSupported(cpu)) {
        const uint32_t entry = (uint32_t)cpu->PB << 16 | 0x882eu;
        return interp_tier_dispatch_tail(cpu, entry, entry, cpu->S, cpu->host_return_valid);
    }
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    NmiDmaClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = 0x800000u;
    clock.span.timing = tilemap_dma_timing;
    clock.span.count = sizeof tilemap_dma_timing / sizeof *tilemap_dma_timing;
    const Lufia2Memory memory = {NmiDmaRead, NmiDmaWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2UploadTilemapBlock(&memory, &state);
    NmiClockBeginOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x80884eu);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
