static void Lufia2CursorPixelAccountInstruction(CpuState *cpu, unsigned cycles) {
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
