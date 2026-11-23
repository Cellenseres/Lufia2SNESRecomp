static void WaveAdvanceIndex(Interp816 *registers, CpuState *cpu,
                             uint32_t pc, uint16_t *index) {
    Lufia2SpanFetch(registers, pc, 1u, true);
    ++*index;
    registers->n = (*index & 0x8000u) != 0;
    registers->z = *index == 0;
    Lufia2SpanAccountInstruction(cpu, 2u);
}

static void WaveFrameProgress(CpuState *cpu, uint64_t *dynamic_epoch,
                              uint64_t *write_epoch, long instruction, long *progress) {
    if (*dynamic_epoch != s_interp_dynamic_progress_epoch ||
        *write_epoch != g_interp_bridge_write_epoch)
        *progress = instruction;
    // NMI keeps its existing threshold and exit flushes.
    if (!g_snes->inNmi)
        Lufia2SpanSyncProgress(cpu, dynamic_epoch, write_epoch);
    else {
        *dynamic_epoch = s_interp_dynamic_progress_epoch;
        *write_epoch = g_interp_bridge_write_epoch;
    }
}

static int Lufia2DecompRunWaveFrame(Interp816 *registers, CpuState *cpu,
                                   uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                   uint64_t write_epoch, long *tail_steps) {
    const bool begin = pc == 0x85ae68u || pc == 0x85ade1u;
    if ((!begin && pc != 0x85ae94u) || opcode != (begin ? 0x8bu : 0xfau) ||
        registers->e || !registers->mf || registers->xf || registers->dp ||
        registers->sp < (begin ? 0x1f03u : 0x1f00u) ||
        registers->sp > (begin ? 0x1fffu : 0x1ffcu) || (!begin && registers->db != 0x85u) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        !Lufia2DecompFrameRangeQuiet(pc, pc + (begin ? 12u : 28u)) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (begin ? 24u : 45u))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, begin ? 12u : 28u, &offset);
    const char *signature = begin ? "\xa9\x85\x48\xab\xac\x22\x1b\xda\xa2\x00\x00" :
        "\xac\x22\x1b\xc8\xc8\xc0\x40\x00\xd0\x03\xa0\x00\x00\xc2\x20\x98\x8d\x22\x1b\xe2\x20\xa9\x01\x8d\x20\x1b\xab";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, begin ? 11u : 27u))
        return 0;
    long progress = 0;
    if (begin) {
        Lufia2SpanFetch(registers, pc, 1u, false);
        registers->write(registers->mem, registers->sp--, registers->db);
        Lufia2SpanAccountInstruction(cpu, 3u);
        WaveFrameProgress(cpu, &dynamic_epoch, &write_epoch, 1, &progress);
        Lufia2SpanFetch(registers, pc + 1u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, 0x85u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, pc + 3u, 1u, true);
        registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
        Lufia2SpanAccountInstruction(cpu, 3u);
        WaveFrameProgress(cpu, &dynamic_epoch, &write_epoch, 3, &progress);
        Lufia2SpanFetch(registers, pc + 4u, 1u, true);
        registers->db = registers->read(registers->mem, ++registers->sp);
        registers->n = (registers->db & 0x80u) != 0;
        registers->z = registers->db == 0;
        Lufia2SpanAccountInstruction(cpu, 4u);
        WaveFrameProgress(cpu, &dynamic_epoch, &write_epoch, 4, &progress);
        Lufia2SpanFetch(registers, pc + 5u, 3u, true);
        const uint16_t phase = Lufia2DecompReadData(registers, 0x851b22u, 0x851b23u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, phase, false);
        Lufia2SpanAccountInstruction(cpu, 5u);
        WaveFrameProgress(cpu, &dynamic_epoch, &write_epoch, 5, &progress);
        Lufia2SpanFetch(registers, pc + 8u, 1u, true);
        registers->write(registers->mem, registers->sp--, (uint8_t)(registers->x >> 8));
        registers->write(registers->mem, registers->sp--, (uint8_t)registers->x);
        Lufia2SpanAccountInstruction(cpu, 4u);
        WaveFrameProgress(cpu, &dynamic_epoch, &write_epoch, 6, &progress);
        Lufia2SpanFetch(registers, pc + 9u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, 0u, false);
        registers->pc = (uint16_t)(pc + 12u);
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 7 - progress : -6;
        return 24;
    }
    Lufia2SpanFetch(registers, pc, 1u, false);
    const uint16_t low = registers->read(registers->mem, ++registers->sp);
    const uint16_t high = registers->read(registers->mem, ++registers->sp);
    Lufia2DecompApplyOperand(registers, 0xa2u, low | (high << 8), false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    WaveFrameProgress(cpu, &dynamic_epoch, &write_epoch, 1, &progress);
    Lufia2SpanFetch(registers, pc + 1u, 3u, true);
    const uint16_t phase = Lufia2DecompReadData(registers, 0x851b22u, 0x851b23u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, phase, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    WaveFrameProgress(cpu, &dynamic_epoch, &write_epoch, 2, &progress);
    WaveAdvanceIndex(registers, cpu, pc + 4u, &registers->y);
    WaveAdvanceIndex(registers, cpu, pc + 5u, &registers->y);
    Lufia2SpanFetch(registers, pc + 6u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xc0u, 0x40u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    const bool reset = registers->z;
    Lufia2SpanFetch(registers, pc + 9u, 2u, true);
    Lufia2SpanAccountInstruction(cpu, reset ? 2u : 3u);
    if (reset) {
        Lufia2SpanFetch(registers, pc + 11u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, 0u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);
    }
    long instructions = 6 + reset;
    Lufia2SpanFetch(registers, pc + 14u, 2u, true);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 16u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 17u, 3u, true);
    Lufia2DecompWriteData(registers, 0x851b22u, 0x851b23u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 5u);
    instructions += 3;
    WaveFrameProgress(cpu, &dynamic_epoch, &write_epoch, instructions, &progress);
    Lufia2SpanFetch(registers, pc + 20u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 22u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 1u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 24u, 3u, true);
    registers->write(registers->mem, 0x851b20u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 4u);
    instructions += 3;
    WaveFrameProgress(cpu, &dynamic_epoch, &write_epoch, instructions, &progress);
    Lufia2SpanFetch(registers, pc + 27u, 1u, true);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->n = (registers->db & 0x80u) != 0;
    registers->z = registers->db == 0;
    registers->pc = 0xaeb0u;
    registers->cyclesUsed = 4u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    ++instructions;
    WaveFrameProgress(cpu, &dynamic_epoch, &write_epoch, instructions, &progress);
    *tail_steps = progress ? instructions - progress : 1 - instructions;
    return 43 + 2 * reset;
}

static int Lufia2DecompRunWaveHdmaFill(Interp816 *registers, CpuState *cpu,
                            uint32_t pc, uint8_t opcode, bool auto_quiescent,
                            unsigned step_budget, long *tail_steps) {
    if (pc != 0x85ae74u || opcode != 0xc2u || registers->e || !registers->mf ||
        registers->xf || registers->dp || registers->d || registers->db != 0x85u ||
        registers->waiting || registers->stopped || registers->nmiWanted ||
        registers->irqWanted || (registers->x & 1u) || registers->x >= 0x0160u ||
        (registers->y & 1u) || registers->y >= 0x0040u ||
        !Lufia2DecompFrameRangeQuiet(pc, pc + 32u))
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
