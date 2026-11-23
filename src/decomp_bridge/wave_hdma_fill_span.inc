static void WaveAdvanceIndex(Interp816 *registers, CpuState *cpu,
                             uint32_t pc, uint16_t *index, bool probe) {
    Lufia2SpanFetch(registers, pc, 1u, probe);
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
    if (pc == 0x85aeb0u || pc == 0x85a76du)
        return Lufia2DecompRunBattleEffectDispatchReturn(registers, cpu, pc, opcode,
            dynamic_epoch, write_epoch, tail_steps);
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
    WaveAdvanceIndex(registers, cpu, pc + 4u, &registers->y, true);
    WaveAdvanceIndex(registers, cpu, pc + 5u, &registers->y, true);
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


static const char s_wave_word_code[] =
    "\xc2\x20\xb9\x9c\xa2\x18\x6d\x9e\x05\x9f\x00\x40\x7e\xe2\x20"
    "\xc8\xc8\xe8\xe8\xc0\x40\x00\xd0\x03\xa0\x00\x00\xe0\x60\x01\xd0\xe0";

static inline void WaveLoadPattern(Interp816 *registers, CpuState *cpu, uint32_t pc, bool probe) {
    Lufia2SpanFetch(registers, pc, 3u, probe);
    const uint32_t sample = 0x85a29cu + registers->y;
    const uint16_t wave = Lufia2DecompReadData(registers, sample, sample + 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, wave, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
}

static inline bool WaveAddScroll(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                 bool probe, bool auto_quiescent) {
    Lufia2SpanFetch(registers, pc, 3u, probe);
    const uint64_t read_epoch = s_interp_dynamic_progress_epoch;
    const uint16_t base = Lufia2DecompReadData(registers, 0x85059eu, 0x85059fu, true);
    Lufia2DecompApplyOperand(registers, 0x69u, base, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    const bool flush = auto_quiescent && s_interp_dynamic_progress_epoch != read_epoch;
    if (flush)
        bridge_apu_flush(cpu);
    return flush;
}

static inline void WaveStoreWord(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                 bool probe, bool auto_quiescent) {
    Lufia2SpanFetch(registers, pc, 4u, probe);
    const uint32_t destination = 0x7e4000u + registers->x;
    Lufia2DecompWriteData(registers, destination, destination + 1u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (auto_quiescent)
        bridge_apu_flush(cpu);
}

typedef struct WaveWordSpan {
    unsigned cycles;
    unsigned instructions;
    unsigned progress;
    unsigned cycle_budget;
    unsigned step_budget;
} WaveWordSpan;

static bool WaveWordCanStep(const WaveWordSpan *span, unsigned cycles) {
    // The first instruction retains the original atomic deadline boundary.
    return span->instructions < span->step_budget &&
        (!span->instructions || span->cycles + cycles <= span->cycle_budget);
}

static void WaveWordStep(WaveWordSpan *span, Interp816 *registers,
                          unsigned cycles, uint16_t next) {
    span->cycles += cycles;
    ++span->instructions;
    registers->cyclesUsed = cycles;
    registers->pc = next;
}

static int Lufia2DecompRunWaveWordContinuation(Interp816 *registers, CpuState *cpu,
                            uint32_t pc, uint8_t opcode, bool auto_quiescent,
                            unsigned step_budget, long *tail_steps) {
    static const uint8_t entries[] = {0, 2, 5, 6, 9, 13, 15, 16, 17, 18, 19, 22, 24, 27, 30};
    if (pc < 0x85ae74u || pc > 0x85ae92u || registers->e || registers->xf || registers->dp ||
        registers->d || registers->db != 0x85u || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted || !step_budget ||
        registers->x > 0x0160u || registers->y > 0x0040u ||
        !Lufia2DecompFrameRangeQuiet(0x85ae74u, 0x85ae94u))
        return 0;
    const unsigned entry = pc - 0x85ae74u;
    unsigned position = 0;
    while (position < sizeof entries && entries[position] != entry)
        ++position;
    if (position == sizeof entries || opcode != (uint8_t)s_wave_word_code[entry] ||
        (entry >= 2u && entry <= 9u && registers->mf) ||
        (entry <= 9u && ((registers->x & 1u) || registers->x >= 0x0160u ||
                         (registers->y & 1u) || registers->y >= 0x0040u)))
        return 0;
    unsigned offset;
    const unsigned length = sizeof s_wave_word_code - 1u - entry;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    if (!cart || memcmp(cart->rom + offset, s_wave_word_code + entry, length))
        return 0;
    WaveWordSpan span = {0, 0, 0, (unsigned)BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget};
    switch (entry) {
    case 0: goto widen_accumulator;
    case 2: goto load_pattern;
    case 5: goto clear_carry;
    case 6: goto add_scroll;
    case 9: goto store_word;
    case 13: goto restore_width;
    case 15: goto advance_phase_first;
    case 16: goto advance_phase_second;
    case 17: goto advance_destination_first;
    case 18: goto advance_destination_second;
    case 19: goto compare_phase;
    case 22: goto branch_phase;
    case 24: goto reset_phase;
    case 27: goto compare_destination;
    case 30: goto branch_destination;
    default: return 0;
    }

widen_accumulator:
    if (!WaveWordCanStep(&span, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x85ae74u, 2u, span.instructions != 0u);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    WaveWordStep(&span, registers, 3u, 0xae76u);
load_pattern:
    if (!WaveWordCanStep(&span, 5u)) goto finished;
    WaveLoadPattern(registers, cpu, 0x85ae76u, span.instructions != 0u);
    WaveWordStep(&span, registers, 5u, 0xae79u);
clear_carry:
    if (!WaveWordCanStep(&span, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x85ae79u, 1u, span.instructions != 0u);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    WaveWordStep(&span, registers, 2u, 0xae7au);
add_scroll:
    if (!WaveWordCanStep(&span, 5u)) goto finished;
    {
        const bool flush = WaveAddScroll(registers, cpu, 0x85ae7au,
            span.instructions != 0u, auto_quiescent);
        WaveWordStep(&span, registers, 5u, 0xae7du);
        if (flush) span.progress = span.instructions;
    }
store_word:
    if (!WaveWordCanStep(&span, 6u)) goto finished;
    WaveStoreWord(registers, cpu, 0x85ae7du, span.instructions != 0u, auto_quiescent);
    WaveWordStep(&span, registers, 6u, 0xae81u);
    if (auto_quiescent) span.progress = span.instructions;
restore_width:
    if (!WaveWordCanStep(&span, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x85ae81u, 2u, span.instructions != 0u);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);
    WaveWordStep(&span, registers, 3u, 0xae83u);
advance_phase_first:
    if (!WaveWordCanStep(&span, 2u)) goto finished;
    WaveAdvanceIndex(registers, cpu, 0x85ae83u, &registers->y, span.instructions != 0u);
    WaveWordStep(&span, registers, 2u, 0xae84u);
advance_phase_second:
    if (!WaveWordCanStep(&span, 2u)) goto finished;
    WaveAdvanceIndex(registers, cpu, 0x85ae84u, &registers->y, span.instructions != 0u);
    WaveWordStep(&span, registers, 2u, 0xae85u);
advance_destination_first:
    if (!WaveWordCanStep(&span, 2u)) goto finished;
    WaveAdvanceIndex(registers, cpu, 0x85ae85u, &registers->x, span.instructions != 0u);
    WaveWordStep(&span, registers, 2u, 0xae86u);
advance_destination_second:
    if (!WaveWordCanStep(&span, 2u)) goto finished;
    WaveAdvanceIndex(registers, cpu, 0x85ae86u, &registers->x, span.instructions != 0u);
    WaveWordStep(&span, registers, 2u, 0xae87u);
compare_phase:
    if (!WaveWordCanStep(&span, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x85ae87u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc0u, 0x0040u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    WaveWordStep(&span, registers, 3u, 0xae8au);
branch_phase:
    {
        const unsigned cycles = registers->z ? 2u : 3u;
        if (!WaveWordCanStep(&span, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x85ae8au, 2u, span.instructions != 0u);
        Lufia2SpanAccountInstruction(cpu, cycles);
        const bool reset = registers->z;
        WaveWordStep(&span, registers, cycles, reset ? 0xae8cu : 0xae8fu);
        if (!reset) goto compare_destination;
    }
reset_phase:
    if (!WaveWordCanStep(&span, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x85ae8cu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    WaveWordStep(&span, registers, 3u, 0xae8fu);
compare_destination:
    if (!WaveWordCanStep(&span, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x85ae8fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, 0x0160u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    WaveWordStep(&span, registers, 3u, 0xae92u);
branch_destination:
    {
        const unsigned cycles = registers->z ? 2u : 3u;
        if (!WaveWordCanStep(&span, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x85ae92u, 2u, span.instructions != 0u);
        Lufia2SpanAccountInstruction(cpu, cycles);
        WaveWordStep(&span, registers, cycles, registers->z ? 0xae94u : 0xae74u);
    }
finished:
    if (!span.instructions) return 0;
    *tail_steps = span.progress ? span.instructions - span.progress : *tail_steps + span.instructions - 1;
    return (int)span.cycles;
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
        return Lufia2DecompRunWaveWordContinuation(registers, cpu, pc, opcode,
            auto_quiescent, step_budget, tail_steps);

    const uint64_t room = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / 46u;
    unsigned count = room < 8u ? (unsigned)room : 8u;
    const unsigned remaining = (0x0160u - registers->x) / 2u;
    if (count > step_budget / 15u)
        count = step_budget / 15u;
    if (count > remaining)
        count = remaining;
    if (!count)
        return Lufia2DecompRunWaveWordContinuation(registers, cpu, pc, opcode,
            auto_quiescent, step_budget, tail_steps);

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 32u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, s_wave_word_code + 1u, sizeof s_wave_word_code - 2u))
        return 0;

    const long initial_steps = *tail_steps;
    unsigned cycles = 0;
    unsigned instructions = 0;
    for (unsigned i = 0; i < count; ++i) {
        Lufia2SpanFetch(registers, pc, 2u, i != 0u);
        registers->mf = false;
        Lufia2SpanAccountInstruction(cpu, 3u);

        WaveLoadPattern(registers, cpu, pc + 2u, true);

        Lufia2SpanFetch(registers, pc + 5u, 1u, true);
        registers->c = false;
        Lufia2SpanAccountInstruction(cpu, 2u);

        (void)WaveAddScroll(registers, cpu, pc + 6u, true, auto_quiescent);
        WaveStoreWord(registers, cpu, pc + 9u, true, auto_quiescent);

        Lufia2SpanFetch(registers, pc + 13u, 2u, true);
        registers->mf = true;
        Lufia2SpanAccountInstruction(cpu, 3u);
        WaveAdvanceIndex(registers, cpu, pc + 15u, &registers->y, true);
        WaveAdvanceIndex(registers, cpu, pc + 16u, &registers->y, true);
        WaveAdvanceIndex(registers, cpu, pc + 17u, &registers->x, true);
        WaveAdvanceIndex(registers, cpu, pc + 18u, &registers->x, true);

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
        *tail_steps = auto_quiescent ? (reset_phase ? 10 : 9) : initial_steps + (long)instructions - 1;
    }
    return (int)cycles;
}
