static unsigned s_frame_return_opcodes;
static unsigned s_frame_return_progress_mask;

void Lufia2DecompFrameReturnWriteByte(CpuState *cpu, uint32_t address,
                                      uint8_t value, bool *progress) {
    const uint64_t before = g_interp_bridge_write_epoch;
    bridge_bus_write(cpu, address, value);
    *progress = before != g_interp_bridge_write_epoch;
}

uint8_t Lufia2DecompFrameReturnReadByte(CpuState *cpu, uint32_t address,
                                        bool *progress) {
    const uint64_t before = s_interp_dynamic_progress_epoch;
    const uint8_t value = bridge_bus_read(cpu, address);
    *progress = before != s_interp_dynamic_progress_epoch;
    return value;
}

void Lufia2DecompFrameReturnAccountOpcode(CpuState *cpu, unsigned cycles,
                                          uint64_t master, bool progress) {
    Lufia2DecompNmiAccountCycles(cpu, cycles, master);
    if (progress && s_frame_return_auto_quiescent)
        bridge_apu_flush(cpu);
}

void Lufia2DecompFrameReturnComplete(unsigned count, unsigned progress_mask) {
    s_frame_return_opcodes = count;
    s_frame_return_progress_mask = progress_mask;
}

static void Lufia2DecompFrameReturnContinue(long *steps, int auto_quiescent,
                                            uint64_t *write_epoch,
                                            uint64_t *dynamic_epoch) {
    if (!s_frame_return_opcodes)
        return;
    *steps += (long)s_frame_return_opcodes - 1;
    if (auto_quiescent) {
        for (unsigned opcode = 0; opcode < s_frame_return_opcodes; ++opcode)
            if (s_frame_return_progress_mask & (1u << opcode))
                *steps = (long)(s_frame_return_opcodes - opcode - 1u);
        *write_epoch = g_interp_bridge_write_epoch;
        *dynamic_epoch = s_interp_dynamic_progress_epoch;
    }
    s_frame_return_opcodes = 0u;
    s_frame_return_progress_mask = 0u;
}
