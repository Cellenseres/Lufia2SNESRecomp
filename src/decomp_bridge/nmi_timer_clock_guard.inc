static bool NmiTimerClockSupported(const CpuState *cpu, uint32_t entry,
                                   unsigned opcode_reservation,
                                   uint64_t master_budget) {
    if (!((entry == 0x85a8e7u && opcode_reservation == 555u) ||
          (entry == 0x85a736u && opcode_reservation == 277u)) ||
        !Lufia2DecompNmiHasInterruptOwner() ||
        !Lufia2DecompFrameReturnCpuReady() || !master_budget ||
        cpu->PB != 0x85u || cpu->DB != 0x85u || cpu->D ||
        cpu->emulation || cpu->_flag_D || cpu->m_flag != 1u || cpu->x_flag ||
        !cpu->ram || cpu->S < 0x1f20u || cpu->S > 0x1ffbu ||
        cpu->master_cycles > UINT64_MAX - master_budget ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return false;
    if (!g_snes || g_snes->ram != cpu->ram || !g_snes->inNmi || g_snes->inIrq ||
        !g_snes->hdmaBeamOff || g_snes->hIrqEnabled || g_snes->vIrqEnabled ||
        g_snes->hPos >= 1364u || g_snes->vPos >= 262u ||
        g_snes->beamMasterLast != cpu->master_cycles || !g_snes->dma ||
        g_snes->dma->dmaBusy || g_snes->dma->dmaTimer)
        return false;
    for (unsigned channel = 0u; channel < 8u; ++channel)
        if (g_snes->dma->channel[channel].dmaActive)
            return false;
    if (cpu->ram[(uint16_t)(cpu->S + 1u)] != 0x3bu ||
        cpu->ram[(uint16_t)(cpu->S + 2u)] != 0x8fu)
        return false;
    uint64_t refresh_phase, refresh_clock;
    snes_refresh_state_get(&refresh_phase, &refresh_clock);
    if (refresh_phase >= 1364u || refresh_clock != cpu->master_cycles)
        return false;
    const unsigned next_line = g_snes->vPos < 225u ? 225u : 262u;
    const uint64_t before_edge =
        (uint64_t)(next_line - g_snes->vPos) * 1364u - g_snes->hPos;
    return master_budget < before_edge;
}

