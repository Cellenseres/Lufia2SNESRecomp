static bool NmiClockWindow(const CpuState *cpu, uint32_t first, uint32_t end,
    unsigned budget, unsigned reserve,
    bool (*owned)(uint32_t, InterpPreOpcodeHook)) {
    if (!g_snes || !g_snes->inNmi || g_snes->inIrq || !g_snes->dma ||
        !cpu->ram || cpu->ram != g_snes->ram || cpu->emulation ||
        cpu->PB != 0x80u || cpu->DB != 0x80u || cpu->D || cpu->_flag_D ||
        cpu->m_flag > 1u || cpu->x_flag || cpu->S < 0x1f00u || cpu->S > 0x1ff3u ||
        !Lufia2DecompNmiHasInterruptOwner() || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        g_snes->beamMasterLast != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        g_snes->hIrqEnabled || g_snes->vIrqEnabled ||
        g_snes->dma->dmaBusy || g_snes->dma->dmaTimer ||
        g_snes->dma->hdmaPendingInit || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize < (end & 0xffffu) + 1u - 0x8000u)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    if ((unsigned)g_snes->hPos + budget >= fence)
        return false;
    for (unsigned channel = 0u; channel < 8u; ++channel)
        if (g_snes->dma->channel[channel].dmaActive)
            return false;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (deadline && (deadline <= cpu->master_cycles ||
        deadline - cpu->master_cycles < budget))
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles) &&
        Lufia2DecompFrameRangeAllows(first, end, owned);
}
