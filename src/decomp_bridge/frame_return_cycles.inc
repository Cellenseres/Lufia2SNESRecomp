static unsigned s_frame_return_opcodes;
static unsigned s_frame_return_progress_mask;

bool Lufia2DecompFrameReturnCpuReady(void) {
    return !s_frame_return_interrupt_blocked;
}

void Lufia2DecompFrameReturnWriteByte(CpuState *cpu, uint32_t address, uint8_t value,
                                      bool *progress) {
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

uint16_t Lufia2DecompFrameReturnReadWord(CpuState *cpu, uint32_t low, uint32_t high,
                                         bool *progress) {
    const uint64_t before = s_interp_dynamic_progress_epoch;
    uint16_t value;
    if (!bridge_bus_read_word(cpu, low, high, &value)) {
        value = bridge_bus_read(cpu, low);
        value |= (uint16_t)bridge_bus_read(cpu, high) << 8;
    }
    *progress = before != s_interp_dynamic_progress_epoch;
    return value;
}

void Lufia2DecompFrameReturnWriteWord(CpuState *cpu, uint32_t low, uint32_t high,
                                      uint16_t value, bool *progress) {
    const uint64_t before = g_interp_bridge_write_epoch;
    if (!bridge_bus_write_word(cpu, low, high, value, false)) {
        bridge_bus_write(cpu, low, (uint8_t)value);
        bridge_bus_write(cpu, high, (uint8_t)(value >> 8));
    }
    *progress = before != g_interp_bridge_write_epoch;
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
        for (unsigned opcode = 0, mask = s_frame_return_progress_mask;
             opcode < s_frame_return_opcodes && mask; ++opcode, mask >>= 1u)
            if (mask & 1u)
                *steps = (long)(s_frame_return_opcodes - opcode - 1u);
        *write_epoch = g_interp_bridge_write_epoch;
        *dynamic_epoch = s_interp_dynamic_progress_epoch;
    }
    s_frame_return_opcodes = 0u;
    s_frame_return_progress_mask = 0u;
}
