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
