void Lufia2DecompNmiAccountCycles(CpuState *cpu, unsigned cycles, uint64_t master) {
    cpu->cycles += cycles;
    cpu->master_cycles += master;
    snes_refresh_charge();
    cpu->coprocessor_master_cycles = cpu->master_cycles;
    if (g_snes)
        snes_sync_master_clock(g_snes, cpu->master_cycles);
    if (g_snes && g_snes->cart)
        cart_sync_coprocessors(g_snes->cart, cpu->master_cycles);
#ifdef SNES_COSIM
    if (!cosim_apu_shared_clock())
#endif
    {
        if (rtl_apu_extended_frame_timing() ||
            !interp_bridge_use_absolute_apu_timeline(
                rtl_apu_frame_timeline_active(),
                g_snes && cart_has_sa1(g_snes->cart), false)) {
            s_apu_pending_master += master;
            if (s_apu_pending_master >= bridge_bounce_flush_thresh())
                bridge_apu_flush(cpu);
        }
    }
}

uint8_t Lufia2DecompNmiReadByte(CpuState *cpu, uint32_t address) {
    return bridge_bus_read(cpu, address);
}

void Lufia2DecompNmiWriteByte(CpuState *cpu, uint32_t address, uint8_t value) {
    bridge_bus_write(cpu, address, value);
}
