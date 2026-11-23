static void WaveAdvanceIndex(Interp816 *registers, CpuState *cpu,
                             uint32_t pc, uint16_t *index) {
    Lufia2SpanFetch(registers, pc, 1u, true);
    ++*index;
    registers->n = (*index & 0x8000u) != 0;
    registers->z = *index == 0;
    Lufia2SpanAccountInstruction(cpu, 2u);
}

static int Lufia2DecompRunWaveHdmaFill(Interp816 *registers, CpuState *cpu,
                            uint32_t pc, uint8_t opcode, bool auto_quiescent,
                            unsigned step_budget, long *tail_steps) {
    if (pc != 0x85ae74u || opcode != 0xc2u || registers->e || !registers->mf ||
        registers->xf || registers->dp || registers->d || registers->db != 0x85u ||
        registers->waiting || registers->stopped || registers->nmiWanted ||
        registers->irqWanted || (registers->x & 1u) || registers->x >= 0x0160u ||
        (registers->y & 1u) || registers->y >= 0x0040u)
        return 0;

    const uint64_t room = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / 46u;
    unsigned count = room < 8u ? (unsigned)room : 8u;
    const unsigned remaining = (0x0160u - registers->x) / 2u;
    if (count > step_budget / 15u)
        count = step_budget / 15u;
    if (count > remaining)
        count = remaining;
    if (!count)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 32u, &offset);
    const char signature[] = "\x20\xb9\x9c\xa2\x18\x6d\x9e\x05\x9f\x00\x40\x7e\xe2\x20"
        "\xc8\xc8\xe8\xe8\xc0\x40\x00\xd0\x03\xa0\x00\x00\xe0\x60\x01\xd0\xe0";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, sizeof signature - 1u))
        return 0;

    unsigned cycles = 0;
    unsigned instructions = 0;
    for (unsigned i = 0; i < count; ++i) {
        Lufia2SpanFetch(registers, pc, 2u, i != 0u);
        registers->mf = false;
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, pc + 2u, 3u, true);
        const uint32_t sample = 0x85a29cu + registers->y;
        const uint16_t wave = Lufia2DecompReadData(registers, sample, sample + 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, wave, false);
        Lufia2SpanAccountInstruction(cpu, 5u);

        Lufia2SpanFetch(registers, pc + 5u, 1u, true);
        registers->c = false;
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 6u, 3u, true);
        const uint64_t read_epoch = s_interp_dynamic_progress_epoch;
        const uint16_t base = Lufia2DecompReadData(registers, 0x85059eu, 0x85059fu, true);
        Lufia2DecompApplyOperand(registers, 0x69u, base, false);
        Lufia2SpanAccountInstruction(cpu, 5u);
        if (auto_quiescent && s_interp_dynamic_progress_epoch != read_epoch)
            bridge_apu_flush(cpu);

        Lufia2SpanFetch(registers, pc + 9u, 4u, true);
        const uint32_t destination = 0x7e4000u + registers->x;
        Lufia2DecompWriteData(registers, destination, destination + 1u, registers->a, true);
        Lufia2SpanAccountInstruction(cpu, 6u);
        if (auto_quiescent)
            bridge_apu_flush(cpu);

        Lufia2SpanFetch(registers, pc + 13u, 2u, true);
        registers->mf = true;
        Lufia2SpanAccountInstruction(cpu, 3u);
        WaveAdvanceIndex(registers, cpu, pc + 15u, &registers->y);
        WaveAdvanceIndex(registers, cpu, pc + 16u, &registers->y);
        WaveAdvanceIndex(registers, cpu, pc + 17u, &registers->x);
        WaveAdvanceIndex(registers, cpu, pc + 18u, &registers->x);

        Lufia2SpanFetch(registers, pc + 19u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xc0u, 0x0040u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);
        const bool reset_phase = registers->z;
        Lufia2SpanFetch(registers, pc + 22u, 2u, true);
        Lufia2SpanAccountInstruction(cpu, reset_phase ? 2u : 3u);
        if (reset_phase) {
            Lufia2SpanFetch(registers, pc + 24u, 3u, true);
            Lufia2DecompApplyOperand(registers, 0xa0u, 0u, false);
            Lufia2SpanAccountInstruction(cpu, 3u);
        }

        Lufia2SpanFetch(registers, pc + 27u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xe0u, 0x0160u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanFetch(registers, pc + 30u, 2u, true);
        registers->cyclesUsed = registers->z ? 2u : 3u;
        registers->pc = registers->z ? 0xae94u : 0xae74u;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        cycles += 41u + 2u * reset_phase + registers->cyclesUsed;
        instructions += 14u + reset_phase;
        *tail_steps = auto_quiescent ? (reset_phase ? 10 : 9) : (long)instructions - 1;
    }
    return (int)cycles;
}
