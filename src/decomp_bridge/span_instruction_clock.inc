static void Lufia2SpanAccountInstruction(CpuState *cpu, unsigned cycles) {
    s_interp_bus_timing_active = 0;
    const unsigned internal = cycles > s_interp_bus_cycles
        ? cycles - s_interp_bus_cycles : 0u;
    const uint64_t master = s_interp_bus_master + (uint64_t)internal * 6u;
    ++g_interp_bridge_steps;
    Lufia2DecompNmiAccountCycles(cpu, cycles, master);
    s_interp_bus_master = 0u;
    s_interp_bus_cycles = 0u;
    s_interp_bus_timing_active = 1;
}

/* Preserve the bridge lookup read without charging it to guest time. */
static void Lufia2SpanProbeOpcode(Interp816 *registers, uint32_t pc) {
    s_interp_bus_timing_active = 0;
    (void)registers->read(registers->mem, pc);
    s_interp_bus_timing_active = 1;
}

static void Lufia2SpanFetch(Interp816 *registers, uint32_t pc, unsigned length, bool probe) {
    if (probe)
        Lufia2SpanProbeOpcode(registers, pc);
    (void)registers->read(registers->mem, pc);
    g_interp816_cur_pc = pc;
    for (unsigned i = 1; i < length; ++i)
        (void)registers->read(registers->mem, pc + i);
}

static void Lufia2SpanSyncProgress(CpuState *cpu, uint64_t *dynamic_epoch,
                                   uint64_t *write_epoch) {
    if (*dynamic_epoch != s_interp_dynamic_progress_epoch ||
        *write_epoch != g_interp_bridge_write_epoch) {
        *dynamic_epoch = s_interp_dynamic_progress_epoch;
        *write_epoch = g_interp_bridge_write_epoch;
        bridge_apu_flush(cpu);
    }
}

static void SpriteSpanReturn(Interp816 *registers, CpuState *cpu, uint32_t pc) {
    Lufia2SpanFetch(registers, pc, 1u, true);
    uint16_t target = registers->read(registers->mem, ++registers->sp);
    target |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->pc = (uint16_t)(target + 1u);
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
}
