static bool FieldMetatileReadableWord(const Cart *cart, uint32_t address) {
    const uint32_t next = (address + 1u) & 0xffffffu;
    return (Lufia2DecompIsWramByte(address) || Lufia2DecompIsLoRomDataByte(cart, address)) &&
        (Lufia2DecompIsWramByte(next) || Lufia2DecompIsLoRomDataByte(cart, next));
}

static int RunFieldMetatileSelect(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                    uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x80f6dfu || opcode != 0xdau || registers->e || registers->mf || registers->xf ||
        registers->sp < 0x1f02u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 23u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 15u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\xbd\x00\x00\x29\x00\x30\xc9\x00\x30\xd0\x03\xae\x08\xd0", 14u))
        return 0;

    const uint32_t bank = (uint32_t)registers->db << 16;
    const uint32_t cell = bank + registers->x;
    if (!FieldMetatileReadableWord(cart, cell) || !FieldMetatileReadableWord(cart, bank | 0xd008u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->write(registers->mem, registers->sp--, (uint8_t)(registers->x >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->x);
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = (dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch) ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 3u, true);
    const uint16_t value = Lufia2DecompReadData(registers, cell, (cell + 1u) & 0xffffffu, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, value, false);
    const unsigned read_cycles = registers->x >= 0x100u ? 6u : 5u;
    Lufia2SpanAccountInstruction(cpu, read_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x3000u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 7u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x3000u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    registers->pc = 0xf6eeu;
    if (!registers->z) {
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 5 - progress : -4;
        return 13u + read_cycles;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 12u, 3u, true);
    const uint16_t shared = Lufia2DecompReadData(registers, bank | 0xd008u, bank | 0xd009u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, shared, false);
    registers->cyclesUsed = 5u;
    Lufia2SpanAccountInstruction(cpu, 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 6 - progress : -5;
    return 17u + read_cycles;
}

static int RunFieldMetatileIndex(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x80f6eeu || opcode != 0xbdu || registers->e || registers->mf || registers->xf ||
        registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 21u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 12u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x00\x00\x29\xff\x03\x0a\x0a\x0a\x65\x65\xaa", 11u))
        return 0;

    const uint32_t cell = ((uint32_t)registers->db << 16) + registers->x;
    const uint32_t next = (cell + 1u) & 0xffffffu;
    if ((!Lufia2DecompIsWramByte(cell) && !Lufia2DecompIsLoRomDataByte(cart, cell)) ||
        (!Lufia2DecompIsWramByte(next) && !Lufia2DecompIsLoRomDataByte(cart, next)))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint16_t value = Lufia2DecompReadData(registers, cell, next, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, value, false);
    const unsigned read_cycles = registers->x >= 0x100u ? 6u : 5u;
    Lufia2SpanAccountInstruction(cpu, read_cycles);
    const bool cell_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x03ffu, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    for (unsigned shift = 0; shift < 3u; ++shift) {
        Lufia2SpanFetch(registers, pc + 6u + shift, 1u, true);
        registers->c = (registers->a & 0x8000u) != 0;
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 9u, 2u, true);
    const uint16_t table = Lufia2DecompReadData(registers, 0x65u, 0x66u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, table, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool table_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 11u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    registers->pc = 0xf6fau;
    registers->cyclesUsed = 2u;
    Lufia2SpanAccountInstruction(cpu, 2u);
    *tail_steps = table_progress ? 1 : cell_progress ? 6 : -6;
    return 15u + read_cycles;
}

static int RunFieldMetatileWrite(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x80f6fau || opcode != 0xbdu || registers->e || registers->mf || registers->xf ||
        registers->dp || !cpu->ram || registers->x > 0xfff8u || registers->y >= 64u ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 63u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 27u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x00\x00\x97\x5d\xbd\x02\x00\x97\x60\x98\x1a\x1a\x29\x3f\x00\xa8"
        "\xbd\x04\x00\x97\x5d\xbd\x06\x00\x97\x60", 26u))
        return 0;

    const uint32_t table = ((uint32_t)registers->db << 16) + registers->x;
    for (unsigned byte = 0; byte < 8u; ++byte)
        if (!Lufia2DecompIsWramByte(table + byte) && !Lufia2DecompIsLoRomDataByte(cart, table + byte))
            return 0;

    for (unsigned row = 0; row < 2u; ++row) {
        const unsigned scratch = 0x5du + row * 3u;
        const uint32_t base = cpu->ram[scratch] | ((uint32_t)cpu->ram[scratch + 1u] << 8) |
            ((uint32_t)cpu->ram[scratch + 2u] << 16);
        if (base < 0x7e0100u || base > 0x7fffc0u - 1u)
            return 0;
    }

    uint32_t instruction = pc;
    unsigned cycles = 0;
    long executed = 0;
    long progress = 0;
    for (unsigned word = 0; word < 4u; ++word) {
        if (word == 2u) {
            Lufia2SpanFetch(registers, instruction++, 1u, true);
            Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
            Lufia2SpanAccountInstruction(cpu, 2u);

            for (unsigned increment = 0; increment < 2u; ++increment) {
                Lufia2SpanFetch(registers, instruction++, 1u, true);
                Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
                Lufia2SpanAccountInstruction(cpu, 2u);
            }

            Lufia2SpanFetch(registers, instruction, 3u, true);
            Lufia2DecompApplyOperand(registers, 0x29u, 0x003fu, false);
            Lufia2SpanAccountInstruction(cpu, 3u);
            instruction += 3u;

            Lufia2SpanFetch(registers, instruction++, 1u, true);
            Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
            Lufia2SpanAccountInstruction(cpu, 2u);
            cycles += 11u;
            executed += 5;
        }

        Lufia2SpanFetch(registers, instruction, 3u, word != 0u);
        const uint32_t source = table + word * 2u;
        const uint16_t value = Lufia2DecompReadData(registers, source, source + 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, value, false);
        const unsigned read_cycles = registers->x + word * 2u >= 0x100u ? 6u : 5u;
        Lufia2SpanAccountInstruction(cpu, read_cycles);
        ++executed;
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = executed;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        instruction += 3u;

        Lufia2SpanFetch(registers, instruction, 2u, true);
        const unsigned scratch = 0x5du + (word & 1u) * 3u;
        uint32_t base = Lufia2DecompReadData(registers, scratch, scratch + 1u, true);
        base |= (uint32_t)registers->read(registers->mem, scratch + 2u) << 16;
        const uint32_t destination = base + registers->y;
        Lufia2DecompWriteData(registers, destination, destination + 1u, value, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        ++executed;
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = executed;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        instruction += 2u;
        cycles += read_cycles + 7u;
    }

    registers->pc = 0xf715u;
    registers->cyclesUsed = 7u;
    *tail_steps = progress ? executed - progress : 1 - executed;
    return cycles;
}

static int FieldMetatileRefresh(Interp816 *registers, CpuState *cpu, int cycles, bool can_call,
                                   uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (!can_call)
        return cycles;

    Lufia2SpanFetch(registers, 0x80f72cu, 3u, true);
    registers->write(registers->mem, registers->sp--, 0xf7u);
    registers->write(registers->mem, registers->sp--, 0x2eu);
    registers->pc = 0xf6aau;
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        *tail_steps = 0;
    else
        *tail_steps += *tail_steps < 0 ? -1 : 1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return cycles + 6;
}

static int RunFieldMetatileAdvance(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                     uint64_t dynamic_epoch, uint64_t write_epoch, long room, long *tail_steps) {
    if (pc != 0x80f715u || opcode != 0xfau || registers->e || registers->mf || registers->xf ||
        registers->dp || registers->sp < 0x1f00u || registers->sp > 0x1ffdu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 42u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 23u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x98\x1a\x1a\x29\x3f\x00\xa8\xe8\xe8\xa5\x87\x1a\x85\x87\xf0\x06"
        "\xc5\x83\x90\x05\x64\x87", 22u))
        return 0;

    const bool can_call = room >= 16 && BufferSpanCycleBudget(cpu, s_lle_master_deadline) >= 48u &&
        offset + 26u <= cart->romSize && !memcmp(cart->rom + offset + 23u, "\x20\xaa\xf6", 3u);
    Lufia2SpanFetch(registers, pc, 1u, false);
    uint16_t index = registers->read(registers->mem, ++registers->sp);
    index |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, index, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    for (unsigned step = 0; step < 2u; ++step) {
        Lufia2SpanFetch(registers, pc + 2u + step, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 4u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x003fu, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 7u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    for (unsigned step = 0; step < 2u; ++step) {
        Lufia2SpanFetch(registers, pc + 8u + step, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    const uint16_t column = Lufia2DecompReadData(registers, 0x87u, 0x88u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, column, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 9;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 12u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 13u, 2u, true);
    Lufia2DecompWriteData(registers, 0x87u, 0x88u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 11;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 15u, 2u, true);
    if (registers->z) {
        registers->pc = 0xf72cu;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 12 - progress : -11;
        return FieldMetatileRefresh(registers, cpu, 33, can_call, dynamic_epoch, write_epoch, tail_steps);
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 17u, 2u, true);
    const uint16_t width = Lufia2DecompReadData(registers, 0x83u, 0x84u, true);
    Lufia2DecompApplyOperand(registers, 0xc9u, width, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 13;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 19u, 2u, true);
    if (!registers->c) {
        registers->pc = 0xf72fu;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 14 - progress : -13;
        return 39;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 21u, 2u, true);
    Lufia2DecompWriteData(registers, 0x87u, 0x88u, 0u, true);
    registers->pc = 0xf72cu;
    registers->cyclesUsed = 4u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 15;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 15 - progress : -14;
    return FieldMetatileRefresh(registers, cpu, 42, can_call, dynamic_epoch, write_epoch, tail_steps);
}

static int RunFieldMetatileCount(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x80f72fu || opcode != 0xc6u || registers->e || registers->mf || registers->xf ||
        registers->dp || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 10u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 5u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x26\xd0\xac\x60", 4u))
        return 0;

    const bool can_return = registers->sp >= 0x1f00u && registers->sp <= 0x1ffdu &&
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) >= 15u;
    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x26u, 0x27u, true);
    Lufia2SpanAccountInstruction(cpu, 7u);
    const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    const bool done = registers->z;
    registers->pc = done ? 0xf733u : 0xf6dfu;
    registers->cyclesUsed = done ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 1 : -1;
    if (!done || !can_return)
        return done ? 9 : 10;

    Lufia2SpanNearReturn(registers, cpu, 0x80f733u);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : progress ? 2 : -2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 15;
}

static int RunFieldTileSeedMove(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long room, long *tail_steps) {
    if (pc != 0x80f351u || opcode != 0x54u || registers->e || registers->xf || registers->mf ||
        registers->x < 0x8000u || registers->y >= 0x2000u || room <= 0 ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 3u, &offset);
    if (!cart || cart->rom[offset + 1u] != 0u || cart->rom[offset + 2u] != 0x9bu)
        return 0;

    const uint64_t time_budget = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / 7u;
    unsigned count = room < 8 ? (unsigned)room : 8u;
    if (count > time_budget)
        count = (unsigned)time_budget;
    if (count > 0x10000u - registers->x)
        count = 0x10000u - registers->x;
    if (count > 0x2000u - registers->y)
        count = 0x2000u - registers->y;
    if (count > (unsigned)registers->a + 1u)
        count = (unsigned)registers->a + 1u;
    if (!count || !Lufia2DecompIsLoRomDataByte(cart, 0x9b0000u + registers->x + count - 1u))
        return 0;

    long progress = 0;
    for (unsigned byte = 0; byte < count; ++byte) {
        Lufia2SpanFetch(registers, pc, 3u, byte != 0u);
        registers->db = 0u;
        const uint8_t value = registers->read(registers->mem, 0x9b0000u | registers->x);
        registers->write(registers->mem, registers->y++, value);
        ++registers->x;
        --registers->a;
        Lufia2SpanAccountInstruction(cpu, 7u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = byte + 1u;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    registers->pc = registers->a == 0xffffu ? 0xf354u : 0xf351u;
    registers->cyclesUsed = 7u;
    *tail_steps = progress ? (long)count - progress : 1L - (long)count;
    return 7u * count;
}

static int RunFieldTileSeedReturn(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                    uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x80f354u || opcode != 0xabu || registers->e || registers->mf || !cpu->ram ||
        registers->sp < 0x1f00u || registers->sp > 0x1ffbu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 18u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 7u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x9c\x20\x03\xe2\x20\x6b", 6u) ||
        !Lufia2DecompIsWramByte(((uint32_t)cpu->ram[registers->sp + 1u] << 16) | 0x0320u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->n = (registers->db & 0x80u) != 0;
    registers->z = registers->db == 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool bank_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 3u, true);
    const uint32_t address = ((uint32_t)registers->db << 16) | 0x0320u;
    Lufia2DecompWriteData(registers, address, address + 1u, 0u, true);
    Lufia2SpanAccountInstruction(cpu, 5u);
    const bool write_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 6u, 1u, true);
    uint16_t target = registers->read(registers->mem, ++registers->sp);
    target |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->k = registers->read(registers->mem, ++registers->sp);
    registers->pc = (uint16_t)(target + 1u);
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 :
        write_progress ? 2 : bank_progress ? 3 : -3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 18;
}

static int Lufia2DecompRunFieldMetatile(Interp816 *registers, CpuState *cpu,
                                         uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                         uint64_t write_epoch, long room, long *tail_steps) {
    switch (pc) {
    case 0x80f351u:
        return RunFieldTileSeedMove(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, room, tail_steps);
    case 0x80f354u:
        if (room >= 4)
            return RunFieldTileSeedReturn(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x80f6dfu:
        if (room >= 6)
            return RunFieldMetatileSelect(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x80f6eeu:
        if (room >= 7)
            return RunFieldMetatileIndex(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x80f6fau:
        if (room >= 13)
            return RunFieldMetatileWrite(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x80f715u:
        if (room >= 15)
            return RunFieldMetatileAdvance(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, room, tail_steps);
        break;
    case 0x80f72fu:
        if (room >= 3)
            return RunFieldMetatileCount(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    }
    return 0;
}
