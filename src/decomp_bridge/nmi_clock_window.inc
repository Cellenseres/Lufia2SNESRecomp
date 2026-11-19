static bool NmiClockWindowWithDeferredHdma(const CpuState *cpu, bool byte_index, uint32_t first, uint32_t end,
    unsigned budget, unsigned reserve, bool allow_line_crossing,
    bool (*owned)(uint32_t, InterpPreOpcodeHook), bool defer_hdma_init) {
    if (!g_snes || !g_snes->inNmi || g_snes->inIrq || !g_snes->dma ||
        !cpu->ram || cpu->ram != g_snes->ram || cpu->emulation ||
        cpu->PB != 0x80u || cpu->DB != 0x80u || cpu->D || cpu->_flag_D ||
        cpu->m_flag > 1u || cpu->x_flag != byte_index || cpu->S < 0x1f00u || cpu->S > 0x1ff3u ||
        !Lufia2DecompNmiHasInterruptOwner() || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        g_snes->beamMasterLast != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        g_snes->hIrqEnabled || g_snes->vIrqEnabled ||
        g_snes->dma->dmaBusy || g_snes->dma->dmaTimer ||
        !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize < (end & 0xffffu) + 1u - 0x8000u)
        return false;
    if (g_snes->dma->hdmaPendingInit) {
#if defined(LUFIA2_HAVE_BEAM_HOLD_QUERY) || defined(SNESRECOMP_PLATFORM_BEAM_HOLD_QUERY)
        // Only spans that cannot enable HDMA may defer pending descriptors.
        if (!defer_hdma_init || !g_snes->hdmaBeamOff || snesrecomp_beam_hold_active())
            return false;
#else
        (void)defer_hdma_init;
        return false;
#endif
    }
    const bool joypad_or_scroll = first == 0x808770u || first == 0x8087b2u;
    const bool beam_hdma = !g_snes->hdmaBeamOff &&
        (!joypad_or_scroll || ActorBridgeHdmaMayTransfer());
    const bool before_hdma = beam_hdma && g_snes->vPos < 225u &&
        (!joypad_or_scroll || g_snes->hPos < 1024u);
    const unsigned fence = before_hdma ? 1024u : 1364u;
    if ((unsigned)g_snes->hPos + budget >= fence) {
#if defined(LUFIA2_HAVE_BEAM_HOLD_QUERY) || defined(SNESRECOMP_PLATFORM_BEAM_HOLD_QUERY)
        if (!allow_line_crossing || g_snes->hPos >= 1364u ||
            g_snes->vPos >= 262u || snesrecomp_beam_hold_active())
            return false;
        if (beam_hdma && (!joypad_or_scroll ||
            (g_snes->vPos < 225u && (before_hdma ||
                budget >= 1364u - g_snes->hPos + 1024u))))
            return false;
        const unsigned next_line = g_snes->vPos < 225u ? 225u : 262u;
        const uint64_t before_edge =
            (uint64_t)(next_line - g_snes->vPos) * 1364u - g_snes->hPos;
        if (budget >= before_edge)
            return false;
#else
        (void)allow_line_crossing;
        return false;
#endif
    }
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

static bool NmiClockWindow(const CpuState *cpu, bool byte_index, uint32_t first, uint32_t end,
    unsigned budget, unsigned reserve, bool allow_line_crossing,
    bool (*owned)(uint32_t, InterpPreOpcodeHook)) {
    return NmiClockWindowWithDeferredHdma(cpu, byte_index, first, end, budget, reserve,
        allow_line_crossing, owned, false);
}
