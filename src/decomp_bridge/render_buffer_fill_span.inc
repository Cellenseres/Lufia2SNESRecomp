static int Lufia2DecompRunOamHide(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                       uint64_t dynamic_epoch, uint64_t write_epoch) {
    const bool upper = pc == 0x81b5abu;
    const unsigned decrement = upper ? 2u : 4u;
    const unsigned greatest_index = upper ? 0x1eu : 0x1fcu;
    const uint32_t buffer = ((uint32_t)registers->db << 16) | (upper ? 0x0300u : 0x0100u);
    if ((!upper && pc != 0x81b5b8u) || opcode != 0x9du || registers->e || registers->mf || registers->xf ||
        registers->x > greatest_index || (registers->x & (decrement - 1u)) || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        !Lufia2DecompIsWramByte(buffer + greatest_index + 1u))
        return 0;

    const unsigned instruction_cycles = 7u + 2u * decrement;
    const uint64_t room = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / (instruction_cycles + 3u);
    unsigned count = registers->x / decrement + 1u;
    if (count > 8u)
        count = 8u;
    if (room < count)
        count = (unsigned)room;
    if (!count)
        return 0;

    unsigned offset;
    const unsigned length = 5u + decrement;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = upper ? "\x00\x03\xca\xca\x10\xf9" : "\x00\x01\xca\xca\xca\xca\x10\xf7";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    unsigned cycles = 0;
    for (unsigned sprite = 0; sprite < count; ++sprite) {
        Lufia2SpanFetch(registers, pc, 3u, sprite != 0u);
        const uint32_t entry = buffer + registers->x;
        Lufia2DecompWriteData(registers, entry, entry + 1u, registers->a, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        for (unsigned offset = 3u; offset < 3u + decrement; ++offset) {
            Lufia2SpanFetch(registers, pc + offset, 1u, true);
            Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
            Lufia2SpanAccountInstruction(cpu, 2u);
        }

        Lufia2SpanFetch(registers, pc + 3u + decrement, 2u, true);
        registers->pc = (uint16_t)(registers->n ? pc + length : pc);
        registers->cyclesUsed = registers->n ? 2u : 3u;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        cycles += instruction_cycles + registers->cyclesUsed;
    }
    return (int)cycles;
}

static int Lufia2DecompRunMenuTileClear(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch) {
    if (pc != 0x82839du || opcode != 0x9eu || registers->e || registers->mf || registers->xf ||
        (registers->db != 0x7eu && registers->db != 0x7fu) || registers->x >= 0x800u ||
        (registers->x & 1u) || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted)
        return 0;

    const uint64_t room = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / 24u;
    unsigned count = (0x800u - registers->x) / 2u;
    if (count > 8u)
        count = 8u;
    if (room < count)
        count = (unsigned)room;
    if (!count)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 13u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x00\x20\x9e\x00\x30\xe8\xe8\xe0\x00\x08\xd0\xf3", 12u))
        return 0;

    const uint32_t bank = (uint32_t)registers->db << 16;
    unsigned cycles = 0;
    for (unsigned word = 0; word < count; ++word) {
        Lufia2SpanFetch(registers, pc, 3u, word != 0u);
        const uint32_t first = bank | (0x2000u + registers->x);
        Lufia2DecompWriteData(registers, first, first + 1u, 0u, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 3u, 3u, true);
        const uint32_t second = bank | (0x3000u + registers->x);
        Lufia2DecompWriteData(registers, second, second + 1u, 0u, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        for (unsigned offset = 6u; offset < 8u; ++offset) {
            Lufia2SpanFetch(registers, pc + offset, 1u, true);
            Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
            Lufia2SpanAccountInstruction(cpu, 2u);
        }

        Lufia2SpanFetch(registers, pc + 8u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xe0u, 0x800u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, pc + 11u, 2u, true);
        registers->pc = registers->z ? 0x83aau : 0x839du;
        registers->cyclesUsed = registers->z ? 2u : 3u;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        cycles += 21u + registers->cyclesUsed;
    }
    return (int)cycles;
}

static int Lufia2DecompRunMenuTileBlock(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                            uint64_t dynamic_epoch, uint64_t write_epoch) {
    if (pc != 0x8280b3u || opcode != 0x9fu || registers->e || registers->mf || registers->xf ||
        registers->y < 1u || registers->y > 4u || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted)
        return 0;

    const uint64_t room = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / 17u;
    const unsigned count = room < registers->y ? (unsigned)room : registers->y;
    if (!count)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 10u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x00\x28\x7e\xe8\xe8\x1a\x88\xd0\xf6", 9u))
        return 0;

    unsigned cycles = 0;
    for (unsigned tile = 0; tile < count; ++tile) {
        Lufia2SpanFetch(registers, pc, 4u, tile != 0u);
        const uint32_t destination = 0x7e2800u + registers->x;
        Lufia2DecompWriteData(registers, destination, destination + 1u, registers->a, true);
        Lufia2SpanAccountInstruction(cpu, 6u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        for (unsigned offset = 4u; offset < 6u; ++offset) {
            Lufia2SpanFetch(registers, pc + offset, 1u, true);
            Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
            Lufia2SpanAccountInstruction(cpu, 2u);
        }

        Lufia2SpanFetch(registers, pc + 6u, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 7u, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y - 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 8u, 2u, true);
        registers->pc = registers->z ? 0x80bdu : 0x80b3u;
        registers->cyclesUsed = registers->z ? 2u : 3u;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        cycles += 14u + registers->cyclesUsed;
    }
    return (int)cycles;
}

static int Lufia2DecompRunBattleOamHideBoundaries(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                       uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool begin = pc == 0x81b5a3u;
    const bool finish = pc == 0x81b5c1u;
    if ((!begin && !finish && pc != 0x81b5b2u) ||
        opcode != (begin ? 0xc2u : finish ? 0xe2u : 0xa2u) ||
        registers->e || registers->xf || (!begin && registers->mf) ||
        (finish && (registers->sp < 0x1f00u || registers->sp > 0x1ffdu)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (begin || finish ? 9u : 6u))
        return 0;

    unsigned offset;
    const unsigned length = begin ? 8u : finish ? 3u : 6u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = begin ? "\x20\xa2\x1e\x00\xa9\x55\x55" :
        finish ? "\x20\x60" : "\xfc\x01\xa9\x01\xe0";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    if (finish) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        registers->mf = true;
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanNearReturn(registers, cpu, pc + 2u);
        *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : -1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        return 9;
    }

    if (begin) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        registers->mf = false;
        Lufia2SpanAccountInstruction(cpu, 3u);
        pc += 2u;
    }
    Lufia2SpanFetch(registers, pc, 3u, begin);
    Lufia2DecompApplyOperand(registers, 0xa2u, begin ? 0x001eu : 0x01fcu, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 3u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, begin ? 0x5555u : 0xe001u, false);
    registers->pc = (uint16_t)(pc + 6u);
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = begin ? -2 : -1;
    return begin ? 9 : 6;
}

static int Lufia2DecompRunFieldOamReset(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                            uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool lower_table = pc == 0x83ac8cu;
    const unsigned bytes = lower_table ? 4u : 2u;
    const unsigned length = bytes * 4u + 2u;
    const unsigned cycles = bytes * 8u;
    const unsigned table = lower_table ? 0x0100u : 0x0300u;
    const uint32_t address = (((uint32_t)registers->db << 16) + table + registers->x) & 0xffffffu;
    const uint32_t last = (address + bytes - 1u) & 0xffffffu;
    if ((!lower_table && pc != 0x83aca1u) || opcode != 0x9eu || registers->e || !registers->mf ||
        registers->xf || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || !Lufia2DecompIsWramByte(address) || !Lufia2DecompIsWramByte(last) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < cycles + 3u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    if (!cart || (lower_table
        ? memcmp(cart->rom + offset + 1u, "\x00\x01\x9d\x01\x01\x9e\x02\x01\x9e"
                 "\x03\x01\xca\xca\xca\xca\x10\xee", 17u)
        : memcmp(cart->rom + offset + 1u, "\x00\x03\x9e\x01\x03\xca\xca\x10\xf6", 9u)))
        return 0;

    long progress = 0;
    for (unsigned byte = 0; byte < bytes; ++byte) {
        Lufia2SpanFetch(registers, pc + byte * 3u, 3u, byte != 0);
        const uint8_t value = lower_table && byte == 1u ? (uint8_t)registers->a : 0u;
        registers->write(registers->mem, (address + byte) & 0xffffffu, value);
        Lufia2SpanAccountInstruction(cpu, 6u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = byte + 1u;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    for (unsigned decrement = 0; decrement < bytes; ++decrement) {
        Lufia2SpanFetch(registers, pc + bytes * 3u + decrement, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + length - 2u, 2u, true);
    registers->pc = (uint16_t)(registers->n ? pc + length : pc);
    registers->cyclesUsed = registers->n ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    const long steps = bytes * 2u + 1u;
    *tail_steps = progress ? steps - progress : 1 - steps;
    return cycles + registers->cyclesUsed;
}

static int Lufia2DecompRunBattleTilePair(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                             uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x81be79u || opcode != 0xc2u || registers->e || !registers->mf || registers->xf ||
        registers->db != 0x7eu || registers->x > 0xffbcu || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 46u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 23u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x20\x9d\x00\x00\xa8\x1a\x9d\x02\x00\x98\x18"
        "\x69\x10\x00\x9d\x40\x00\x1a\x9d\x42\x00\x98", 22u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = 0;
    uint32_t instruction = pc + 2u;
    for (unsigned row = 0; row < 2u; ++row) {
        const uint32_t destination = 0x7e0000u + registers->x + row * 64u;
        Lufia2SpanFetch(registers, instruction, 3u, true);
        Lufia2DecompWriteData(registers, destination, destination + 1u, registers->a, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = row ? 9 : 2;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        instruction += 3u;

        if (!row) {
            Lufia2SpanFetch(registers, instruction++, 1u, true);
            Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
            Lufia2SpanAccountInstruction(cpu, 2u);
        }
        Lufia2SpanFetch(registers, instruction++, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, instruction, 3u, true);
        Lufia2DecompWriteData(registers, destination + 2u, destination + 3u, registers->a, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = row ? 11 : 5;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        instruction += 3u;

        Lufia2SpanFetch(registers, instruction++, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
        Lufia2SpanAccountInstruction(cpu, 2u);
        if (!row) {
            Lufia2SpanFetch(registers, instruction++, 1u, true);
            registers->c = false;
            Lufia2SpanAccountInstruction(cpu, 2u);
            Lufia2SpanFetch(registers, instruction, 3u, true);
            Lufia2DecompApplyOperand(registers, 0x69u, 16u, false);
            Lufia2SpanAccountInstruction(cpu, 3u);
            instruction += 3u;
        }
    }

    registers->pc = (uint16_t)instruction;
    registers->cyclesUsed = 2u;
    *tail_steps = progress ? 12 - progress : -11;
    return 46;
}

static int Lufia2DecompRunBattleTilePairAdvance(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                    uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x81be90u || opcode != 0x69u || registers->e || registers->mf || registers->xf ||
        registers->dp || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 30u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 21u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x02\x00\x89\x0f\x00\xd0\x03\x69\x10\x00"
        "\xe2\x20\xe8\xe8\xe8\xe8\xc6\x13\xd0\xd4", 20u))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    Lufia2DecompApplyOperand(registers, 0x69u, 2u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 3u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0fu, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    const bool next_sheet = registers->z;
    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    Lufia2SpanAccountInstruction(cpu, next_sheet ? 2u : 3u);
    if (next_sheet) {
        Lufia2SpanFetch(registers, pc + 8u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0x69u, 16u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);
    }

    Lufia2SpanFetch(registers, pc + 11u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);
    for (unsigned increment = 0; increment < 4u; ++increment) {
        Lufia2SpanFetch(registers, pc + 13u + increment, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 17u, 2u, true);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x13u, 0x14u, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    const bool counter_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 19u, 2u, true);
    registers->pc = registers->z ? 0xbea5u : 0xbe79u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = counter_progress ? 1 : next_sheet ? -10 : -9;
    return 22u + (next_sheet ? 5u : 3u) + registers->cyclesUsed;
}

static int Lufia2DecompRunGlyphBufferFill(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                               uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x80c7afu || opcode != 0x9du || registers->e || registers->mf || registers->xf ||
        registers->db != 0x7eu || registers->x > 0x2ffcu || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 25u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 12u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x00\xd0\x9d\x02\xd0\xca\xca\xca\xca\x10\xf4", 11u))
        return 0;

    long progress = 0;
    for (unsigned word = 0; word < 2u; ++word) {
        Lufia2SpanFetch(registers, pc + word * 3u, 3u, word != 0u);
        const uint32_t destination = 0x7ed000u + registers->x + word * 2u;
        Lufia2DecompWriteData(registers, destination, destination + 1u, registers->a, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = word + 1u;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    for (unsigned decrement = 0; decrement < 4u; ++decrement) {
        Lufia2SpanFetch(registers, pc + 6u + decrement, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    registers->pc = registers->n ? 0xc7bbu : 0xc7afu;
    registers->cyclesUsed = registers->n ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 7 - progress : -6;
    return 22u + registers->cyclesUsed;
}

/* The NMI bridge batches APU time across WRAM writes. */
static int Lufia2DecompRunBattleBackgroundTileFill(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                       long *extra_steps) {
    if (pc != 0x85a915u || opcode != 0x9du || registers->e || registers->mf || !registers->xf ||
        registers->db != 0x7eu || registers->x >= 0x90u || (registers->x & 1u) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 27u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 18u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\xcc\x40\x88\xd0\x06\xa0\x0c\x18"
        "\x69\x04\x00\xe8\xe8\xe0\x90\xd0\xee", 17u))
        return 0;

    const uint32_t destination = 0x7e40ccu + registers->x;
    const unsigned store_cycles = 6u + (registers->x >= 0x34u);
    Lufia2SpanFetch(registers, pc, 3u, false);
    Lufia2DecompWriteData(registers, destination, destination + 1u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, store_cycles);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint8_t)(registers->y - 1u), true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    const bool next_tile = registers->z;
    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    Lufia2SpanAccountInstruction(cpu, next_tile ? 2u : 3u);
    if (next_tile) {
        Lufia2SpanFetch(registers, pc + 6u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, 12u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 8u, 1u, true);
        registers->c = false;
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, pc + 9u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0x69u, 4u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);
    }

    for (unsigned increment = 0; increment < 2u; ++increment) {
        Lufia2SpanFetch(registers, pc + 12u + increment, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, (uint8_t)(registers->x + 1u), true);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }
    Lufia2SpanFetch(registers, pc + 14u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xe0u, 0x90u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 16u, 2u, true);
    registers->pc = registers->z ? 0xa927u : 0xa915u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *extra_steps = next_tile ? 9 : 6;
    return store_cycles + 8u + (next_tile ? 9u : 3u) + registers->cyclesUsed;
}

static int Lufia2DecompRunBattleTileBlockRow(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x81bea5u || opcode != 0x85u || registers->e || !registers->mf || registers->xf ||
        registers->dp || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 34u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 21u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x11\xc6\x14\xf0\x0f\xc2\x20\xa5\x19\x18"
        "\x69\x80\x00\x85\x19\xaa\xe2\x20\x80\xb6", 20u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->write(registers->mem, 0x11u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x14u, 0x15u, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    if (registers->z) {
        registers->pc = 0xbebau;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 3 - progress : -2;
        return 11;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    const uint16_t destination = Lufia2DecompReadData(registers, 0x19u, 0x1au, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, destination, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 10u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 11u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 128u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 14u, 2u, true);
    Lufia2DecompWriteData(registers, 0x19u, 0x1au, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 16u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 17u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 19u, 2u, true);
    registers->pc = 0xbe70u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 11 - progress : -10;
    return 34;
}

static int Lufia2DecompRunBattleTileBlockColumn(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                     uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x81be70u || opcode != 0xa5u || registers->e || !registers->mf || registers->xf ||
        registers->dp || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 15u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 9u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x02\x85\x13\xa5\x04\xeb\xa5\x11", 8u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    const uint8_t count = registers->read(registers->mem, 0x02u);
    Lufia2DecompApplyOperand(registers, 0xa9u, count, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    registers->write(registers->mem, 0x13u, count);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    const uint8_t attributes = registers->read(registers->mem, 0x04u);
    Lufia2DecompApplyOperand(registers, 0xa9u, attributes, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 6u, 1u, true);
    MenuGlyphExchangeBytes(registers);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    const uint8_t tile = registers->read(registers->mem, 0x11u);
    Lufia2DecompApplyOperand(registers, 0xa9u, tile, true);
    registers->pc = 0xbe79u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 5 - progress : -4;
    return 15;
}

static int Lufia2DecompRunBattleTileBlockBoundaries(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                         uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool begin = pc == 0x81be58u;
    if ((!begin && pc != 0x81bebau) || opcode != (begin ? 0x8bu : 0xabu) || registers->e ||
        !registers->mf || registers->xf || (begin && registers->dp) ||
        registers->sp < (begin ? 0x1f02u : 0x1f00u) || registers->sp > (begin ? 0x1fffu : 0x1ffcu) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (begin ? 42u : 10u))
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, begin ? 24u : 2u, &offset);
    if (!cart || (begin ? memcmp(cart->rom + offset + 1u, "\xa9\x7e\x48\xab\xa5\x00\x0a\x85\x11\x29"
        "\xf0\x65\x11\x85\x11\xa6\x08\x86\x19\xa5\x03\x85\x14", 23u) : cart->rom[offset + 1u] != 0x60u))
        return 0;

    if (!begin) {
        Lufia2SpanFetch(registers, pc, 1u, false);
        registers->db = registers->read(registers->mem, ++registers->sp);
        registers->z = registers->db == 0u;
        registers->n = (registers->db & 0x80u) != 0u;
        Lufia2SpanAccountInstruction(cpu, 4u);
        const bool bank_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanNearReturn(registers, cpu, pc + 1u);
        *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : bank_progress ? 1 : -1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        return 10;
    }

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->write(registers->mem, registers->sp--, registers->db);
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x7eu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 1u, true);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    const uint8_t tile = registers->read(registers->mem, 0x00u);
    Lufia2DecompApplyOperand(registers, 0xa9u, tile, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 7u, 1u, true);
    registers->c = (tile & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(tile << 1), true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    registers->write(registers->mem, 0x11u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 7;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0xf0u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 12u, 2u, true);
    const uint8_t offset_tile = registers->read(registers->mem, 0x11u);
    Lufia2DecompApplyOperand(registers, 0x69u, offset_tile, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 9;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 14u, 2u, true);
    registers->write(registers->mem, 0x11u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 10;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 16u, 2u, true);
    const uint16_t destination = Lufia2DecompReadData(registers, 0x08u, 0x09u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, destination, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 11;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 18u, 2u, true);
    Lufia2DecompWriteData(registers, 0x19u, 0x1au, destination, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 12;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 20u, 2u, true);
    const uint8_t rows = registers->read(registers->mem, 0x03u);
    Lufia2DecompApplyOperand(registers, 0xa9u, rows, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 13;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 22u, 2u, true);
    registers->write(registers->mem, 0x14u, rows);
    registers->pc = 0xbe70u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 14;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 14 - progress : -13;
    return 42;
}

static int Lufia2DecompRunBattleSpriteEdges(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                          uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool begin = pc == 0x81b5c4u;
    if ((!begin && pc != 0x81b67fu) || opcode != (begin ? 0x64u : 0xabu) || registers->e ||
        (begin && (!registers->mf || registers->dp)) ||
        (!begin && (registers->sp < 0x1f00u || registers->sp > 0x1ffeu)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (begin ? 3u : 4u))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, begin ? 2u : 1u, &offset);
    if (!cart || (begin && cart->rom[offset + 1u] != 0x72u))
        return 0;

    Lufia2SpanFetch(registers, pc, begin ? 2u : 1u, false);
    if (begin) {
        registers->write(registers->mem, 0x72u, 0u);
        registers->pc = 0xb5c6u;
    } else {
        registers->db = registers->read(registers->mem, ++registers->sp);
        registers->z = registers->db == 0u;
        registers->n = (registers->db & 0x80u) != 0u;
        registers->pc = 0xb680u;
    }
    registers->cyclesUsed = begin ? 3u : 4u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 0 : -1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return registers->cyclesUsed;
}

static int Lufia2DecompRunBattleSpriteFrameBegin(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                      uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x81b5c9u || opcode != 0x8bu || registers->e ||
        !registers->mf || registers->xf || registers->dp ||
        registers->sp < 0x1f02u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 18u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 10u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\xa9\x7e\x48\xab\x64\x58\xa0\x00\x00", 9u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->write(registers->mem, registers->sp--, registers->db);
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x7eu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 1u, true);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    registers->write(registers->mem, 0x58u, 0u);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 7u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0u, false);
    registers->pc = 0xb5d3u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 6 - progress : -5;
    return 18;
}

static int Lufia2DecompRunBattleSpriteGroup(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    struct SpriteGroup {
        uint16_t start;
        uint16_t enabled;
        uint16_t first_sprite;
        uint16_t source;
        uint16_t count;
        bool load_source;
    };
    static const struct SpriteGroup groups[] = {
        {0xb5d3u, 0x15d7u, 0x15c4u, 0x15d8u, 0x15dau, true},
        {0xb5e6u, 0x15dfu, 0x15c2u, 0x4c8au, 0x15e2u, false},
        {0xb5f9u, 0x15cfu, 0x15c5u, 0x493du, 0x15d2u, false},
        {0xb60cu, 0x15cbu, 0x15bau, 0x15ccu, 0x15ceu, true},
        {0xb61fu, 0x15c7u, 0x15b9u, 0x48c0u, 0x15cau, false},
        {0xb632u, 0x15e3u, 0x15c3u, 0x4dcau, 0x15e6u, false},
        {0xb659u, 0x15dbu, 0x15c1u, 0x4b4au, 0x15deu, false},
        {0xb66cu, 0x15e7u, 0x15c6u, 0x4b36u, 0x15eau, false},
    };
    if (opcode != 0xadu || registers->e || !registers->mf || registers->xf ||
        registers->db != 0x7eu || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted)
        return 0;

    const struct SpriteGroup *group = NULL;
    for (unsigned index = 0; index < sizeof(groups) / sizeof(groups[0]); ++index) {
        if ((0x810000u | groups[index].start) == pc) {
            group = &groups[index];
            break;
        }
    }
    const unsigned source_cycles = group && group->load_source ? 5u : 3u;
    if (!group || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 17u + source_cycles)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 19u, &offset);
    if (!cart)
        return 0;
    const uint8_t *const body = cart->rom + offset;
    const uint8_t expected[] = {
        (uint8_t)group->enabled, (uint8_t)(group->enabled >> 8), 0xf0, 0x0e,
        0xa5, 0x58, 0x8d, (uint8_t)group->first_sprite, (uint8_t)(group->first_sprite >> 8),
        group->load_source ? 0xae : 0xa2, (uint8_t)group->source, (uint8_t)(group->source >> 8),
        0xad, (uint8_t)group->count, (uint8_t)(group->count >> 8), 0x20, 0x05, 0xb7,
    };
    if (memcmp(body + 1u, expected, sizeof(expected)))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint8_t enabled = registers->read(registers->mem, 0x7e0000u | group->enabled);
    Lufia2DecompApplyOperand(registers, 0xa9u, enabled, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    if (!enabled) {
        registers->pc = (uint16_t)(pc + 19u);
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 1 : -1;
        return 7;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    const uint8_t first_sprite = registers->read(registers->mem, 0x58u);
    Lufia2DecompApplyOperand(registers, 0xa9u, first_sprite, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 7u, 3u, true);
    registers->write(registers->mem, 0x7e0000u | group->first_sprite, first_sprite);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 10u, 3u, true);
    uint16_t source = group->source;
    if (group->load_source) {
        const uint32_t address = 0x7e0000u | source;
        source = Lufia2DecompReadData(registers, address, address + 1u, true);
    }
    Lufia2DecompApplyOperand(registers, 0xa2u, source, false);
    Lufia2SpanAccountInstruction(cpu, source_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 13u, 3u, true);
    const uint8_t count = registers->read(registers->mem, 0x7e0000u | group->count);
    Lufia2DecompApplyOperand(registers, 0xa9u, count, true);
    registers->pc = (uint16_t)(pc + 16u);
    registers->cyclesUsed = 4u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    *tail_steps = progress ? 6 - progress : -5;
    return 17u + source_cycles;
}

static int Lufia2DecompRunBattleSpriteOverlayGroup(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                         uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x81b645u || opcode != 0xadu || registers->e || !registers->mf || registers->xf ||
        registers->db != 0x7eu || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 22u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 17u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\xd3\x15\xf0\x0f\xa5\x58\x85\x5f\xa2\x56\x49\xad\xd6\x15\xf0\x03", 16u))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint8_t enabled = registers->read(registers->mem, 0x7e15d3u);
    Lufia2DecompApplyOperand(registers, 0xa9u, enabled, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    if (!enabled) {
        registers->pc = 0xb659u;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 1 : -1;
        return 7;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    const uint8_t first_sprite = registers->read(registers->mem, 0x58u);
    Lufia2DecompApplyOperand(registers, 0xa9u, first_sprite, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    registers->write(registers->mem, 0x5fu, first_sprite);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x4956u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 12u, 3u, true);
    const uint8_t count = registers->read(registers->mem, 0x7e15d6u);
    Lufia2DecompApplyOperand(registers, 0xa9u, count, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 15u, 2u, true);
    registers->pc = count ? 0xb656u : 0xb659u;
    registers->cyclesUsed = count ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 7 - progress : -6;
    return 19u + registers->cyclesUsed;
}

static int Lufia2DecompRunBattleSpriteFrameExit(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                     uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool record_ready = pc == 0x81b700u;
    if ((!record_ready && pc != 0x81b704u) || opcode != (record_ready ? 0xa9u : 0x6bu) ||
        registers->e || (record_ready && (!registers->mf || registers->dp)) ||
        (!record_ready && (registers->sp < 0x1f00u || registers->sp > 0x1ffcu)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (record_ready ? 5u : 6u))
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, record_ready ? 4u : 1u, &offset);
    if (!cart || (record_ready && memcmp(cart->rom + offset + 1u, "\x40\x85\x72", 3u)))
        return 0;

    if (record_ready) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        Lufia2DecompApplyOperand(registers, 0xa9u, 0x40u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 2u, 2u, true);
        registers->write(registers->mem, 0x72u, 0x40u);
        registers->pc = 0xb704u;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ||
            write_epoch != g_interp_bridge_write_epoch ? 0 : -1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        return 5;
    }

    Lufia2SpanFetch(registers, pc, 1u, false);
    uint16_t target = registers->read(registers->mem, ++registers->sp);
    target |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->k = registers->read(registers->mem, ++registers->sp);
    registers->pc = (uint16_t)(target + 1u);
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : -1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 6;
}

static int Lufia2DecompRunSceneGlyphEntry(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x80c7c2u || opcode != 0x08u || registers->e || !registers->mf || registers->xf ||
        registers->dp || registers->sp < 0x1f02u || registers->sp > 0x1fffu ||
        !Lufia2DecompIsWramByte(bank | 0x09adu) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 25u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 15u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x8b\x7b\xad\xad\x09\xaa\xbf\x15\xc8\x80\x85\x57\xc2\x30", 14u) ||
        !Lufia2DecompIsLoRomDataByte(cart, 0x80c914u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->write(registers->mem, registers->sp--, interp816_getFlags(registers));
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 1u, true);
    registers->write(registers->mem, registers->sp--, registers->db);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0u, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 3u, 3u, true);
    const uint8_t font_flags = registers->read(registers->mem, bank | 0x09adu);
    Lufia2DecompApplyOperand(registers, 0xa9u, font_flags, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 6u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, font_flags, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 7u, 4u, true);
    const uint32_t attributes_address = 0x80c815u + registers->x;
    const uint8_t attributes = registers->read(registers->mem, attributes_address);
    Lufia2DecompApplyOperand(registers, 0xa9u, attributes, true);
    Lufia2SpanAccountInstruction(cpu, 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 11u, 2u, true);
    registers->write(registers->mem, 0x57u, attributes);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 7;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 13u, 2u, true);
    registers->mf = false;
    registers->pc = 0xc7d1u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 8 - progress : -7;
    return 25;
}

static int Lufia2DecompRunSceneGlyphSelect(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                 uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t address = ((uint32_t)registers->db << 16) | 0x09afu;
    if (pc != 0x80c7d1u || opcode != 0xadu || registers->e || registers->mf || registers->xf ||
        !Lufia2DecompIsWramByte(address + 1u) || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 21u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 17u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\xaf\x09\xc9\x10\x00\xd0\x03\xa9\x20\x00\x38\xe9\x20\x00\x90\x30", 16u))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint16_t glyph = Lufia2DecompReadData(registers, address, address + 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, glyph, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    const bool read_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x10u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    const bool map_space = registers->z;
    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    Lufia2SpanAccountInstruction(cpu, map_space ? 2u : 3u);

    if (map_space) {
        Lufia2SpanFetch(registers, pc + 8u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, 0x20u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);
    }
    Lufia2SpanFetch(registers, pc + 11u, 1u, true);
    registers->c = true;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 12u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x20u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 15u, 2u, true);
    registers->pc = registers->c ? 0xc7e2u : 0xc812u;
    registers->cyclesUsed = registers->c ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    const long steps = map_space ? 7 : 6;
    *tail_steps = read_progress ? steps - 1 : 1 - steps;
    return (map_space ? 18u : 16u) + registers->cyclesUsed;
}

static int Lufia2DecompRunSceneGlyphSetup(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x80c7e2u || opcode != 0x0au || registers->e || registers->mf || registers->xf ||
        registers->dp || registers->sp < 0x1f01u || registers->sp > 0x1fffu ||
        !Lufia2DecompIsWramByte(bank | 0x09b2u) || !Lufia2DecompIsWramByte(bank | 0x09b6u) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 39u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 22u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x0a\x0a\x0a\x18\xaa\xac\xb1\x09\x8c\xb5\x09"
        "\xe2\x20\xa9\x7e\x48\xab\xa9\x10\x85\x58", 21u))
        return 0;

    for (unsigned shift = 0; shift < 4u; ++shift) {
        Lufia2SpanFetch(registers, pc + shift, 1u, shift != 0u);
        registers->c = (registers->a & 0x8000u) != 0u;
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }
    Lufia2SpanFetch(registers, pc + 4u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 5u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 6u, 3u, true);
    const uint16_t position = Lufia2DecompReadData(registers, bank | 0x09b1u, bank | 0x09b2u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, position, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 7 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 3u, true);
    Lufia2DecompWriteData(registers, bank | 0x09b5u, bank | 0x09b6u, position, true);
    Lufia2SpanAccountInstruction(cpu, 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 12u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 14u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x7eu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 16u, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 11;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 17u, 1u, true);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 12;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 18u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x10u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 20u, 2u, true);
    registers->write(registers->mem, 0x58u, 0x10u);
    registers->pc = 0xc7f8u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 14;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 14 - progress : -13;
    return 39;
}

static int Lufia2DecompRunSceneGlyphRow(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                              uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x80c7f8u || opcode != 0xbfu || registers->e || !registers->mf || registers->xf ||
        registers->db != 0x7eu || registers->dp || registers->y == 0xffffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 34u)
        return 0;

    unsigned offset;
    const uint32_t source = 0x9af970u + registers->x;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 19u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x70\xf9\x9a\x99\x00\x00\xa5\x57"
        "\x99\x01\x00\xe8\xc8\xc8\xc6\x58\xd0\xed", 18u) ||
        !Lufia2DecompIsLoRomDataByte(cart, source))
        return 0;

    const uint32_t destination = 0x7e0000u | registers->y;
    Lufia2SpanFetch(registers, pc, 4u, false);
    const uint8_t pixels = registers->read(registers->mem, source);
    Lufia2DecompApplyOperand(registers, 0xa9u, pixels, true);
    Lufia2SpanAccountInstruction(cpu, 5u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 3u, true);
    registers->write(registers->mem, destination, pixels);
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    const uint8_t attributes = registers->read(registers->mem, 0x57u);
    Lufia2DecompApplyOperand(registers, 0xa9u, attributes, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 3u, true);
    registers->write(registers->mem, destination + 1u, attributes);
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 12u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    for (unsigned byte = 0; byte < 2u; ++byte) {
        Lufia2SpanFetch(registers, pc + 13u + byte, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 15u, 2u, true);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x58u, 0x59u, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 17u, 2u, true);
    registers->pc = registers->z ? 0xc80bu : 0xc7f8u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 9 - progress : -8;
    return 31u + registers->cyclesUsed;
}

static int Lufia2DecompRunSceneGlyphReturn(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                 uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool write_cursor = pc == 0x80c80bu;
    const bool restore = write_cursor || pc == 0x80c812u;
    const unsigned cycles = write_cursor ? 19u : restore ? 8u : 6u;
    if ((!restore && pc != 0x80c814u) || opcode != (write_cursor ? 0xc2u : restore ? 0xabu : 0x6bu) ||
        registers->e || (write_cursor && registers->xf) ||
        registers->sp < 0x1f00u || registers->sp > (restore ? 0x1ffdu : 0x1ffcu) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < cycles)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, write_cursor ? 9u : restore ? 2u : 1u, &offset);
    if (!cart || (write_cursor && memcmp(cart->rom + offset + 1u, "\x20\x98\x8f\xb1\x09\x00\xab\x28", 8u)) ||
        (!write_cursor && restore && cart->rom[offset + 1u] != 0x28u))
        return 0;

    if (!restore) {
        Lufia2SpanFetch(registers, pc, 1u, false);
        uint16_t target = registers->read(registers->mem, ++registers->sp);
        target |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        registers->k = registers->read(registers->mem, ++registers->sp);
        registers->pc = (uint16_t)(target + 1u);
        registers->cyclesUsed = 6u;
        Lufia2SpanAccountInstruction(cpu, 6u);
        *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : -1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        return 6;
    }

    long progress = 0;
    if (write_cursor) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        registers->mf = false;
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanFetch(registers, pc + 2u, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 3u, 4u, true);
        Lufia2DecompWriteData(registers, 0x09b1u, 0x09b2u, registers->a, true);
        Lufia2SpanAccountInstruction(cpu, 6u);
        progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
            write_epoch != g_interp_bridge_write_epoch ? 3 : 0;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    const uint32_t restore_pc = write_cursor ? pc + 7u : pc;
    Lufia2SpanFetch(registers, restore_pc, 1u, write_cursor);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = write_cursor ? 4 : 1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, restore_pc + 1u, 1u, true);
    interp816_setFlags(registers, registers->read(registers->mem, ++registers->sp));
    registers->pc = 0xc814u;
    registers->cyclesUsed = 4u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    const long steps = write_cursor ? 5 : 2;
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = steps;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    *tail_steps = progress ? steps - progress : 1 - steps;
    return cycles;
}
