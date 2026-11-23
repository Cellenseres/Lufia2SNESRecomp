static int RunItemRecordSetup(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                              uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool initial = pc == 0x81f1c5u;
    const bool name_call = pc == 0x81f1d7u;
    const bool record_call = pc == 0x81f1dbu;
    const bool pointer = pc == 0x81f1e3u;
    const uint8_t expected = initial ? 0x08u : name_call ? 0x22u : record_call ? 0xa9u : 0xacu;
    const unsigned cycles = initial ? 15u : name_call ? 8u : record_call ? 17u : 8u;
    if ((!initial && !name_call && !record_call && !pointer) || opcode != expected || registers->e ||
        (!initial && (!registers->mf || registers->xf)) ||
        registers->sp < 0x1f03u || registers->sp > (record_call ? 0x1ffeu : 0x1fffu) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < cycles)
        return 0;

    unsigned offset;
    const unsigned length = initial ? 9u : name_call ? 4u : record_call ? 8u : 6u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = initial ? "\x8b\xe2\x20\xc2\x10\xa2\xad\x0b" :
        name_call ? "\xa9\xf2\x81" : record_call ? "\x96\x48\xab\x22\x91\xf2\x81" :
        "\x09\x0a\xa2\x00\x00";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    if (initial) {
        Lufia2SpanFetch(registers, pc, 1u, false);
        registers->write(registers->mem, registers->sp--, interp816_getFlags(registers));
        Lufia2SpanAccountInstruction(cpu, 3u);
        long progress = dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch
            ? 1 : 0;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 1u, 1u, true);
        registers->write(registers->mem, registers->sp--, registers->db);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 2;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 2u, 2u, true);
        interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, pc + 4u, 2u, true);
        interp816_setFlags(registers, interp816_getFlags(registers) & ~0x10u);
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, pc + 6u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, 0x0badu, false);
        registers->pc = 0xf1ceu;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 5 - progress : -4;
        return 15;
    }

    if (pointer) {
        const uint32_t address = ((uint32_t)registers->db << 16) | 0x0a09u;
        if (!Lufia2DecompIsWramByte(address) || !Lufia2DecompIsWramByte(address + 1u))
            return 0;

        Lufia2SpanFetch(registers, pc, 3u, false);
        const uint16_t value = Lufia2DecompReadData(registers, address, address + 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
        Lufia2SpanAccountInstruction(cpu, 5u);
        const bool read_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 3u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, 0, false);
        registers->pc = 0xf1e9u;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = read_progress ? 1 : -1;
        return 8;
    }

    long progress = 0;
    if (record_call) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        Lufia2DecompApplyOperand(registers, 0xa9u, 0x96u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 2u, 1u, true);
        registers->write(registers->mem, registers->sp--, 0x96u);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 2;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 3u, 1u, true);
        registers->db = registers->read(registers->mem, ++registers->sp);
        registers->n = (registers->db & 0x80u) != 0u;
        registers->z = registers->db == 0u;
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 3;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    const uint32_t call_site = pc + (record_call ? 4u : 0u);
    const uint16_t return_address = (uint16_t)(call_site + 3u);
    Lufia2SpanFetch(registers, call_site, 4u, record_call);
    registers->write(registers->mem, registers->sp--, registers->k);
    registers->write(registers->mem, registers->sp--, (uint8_t)(return_address >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)return_address);
    registers->pc = record_call ? 0xf291u : 0xf2a9u;
    registers->cyclesUsed = 8u;
    Lufia2SpanAccountInstruction(cpu, 8u);
    if (!record_call || dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        *tail_steps = 0;
    else
        *tail_steps = progress ? 4 - progress : -3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return cycles;
}

static int RunItemRecordHeaderCopy(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                      uint64_t dynamic_epoch, uint64_t write_epoch, long room, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x81f1e9u || opcode != 0xb9u || registers->e || !registers->mf || registers->xf ||
        registers->x >= 9u || room < 6 || !Lufia2DecompIsWramByte(bank | 0x0b8cu) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 13u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x00\x00\x9d\x84\x0b\xc8\xe8\xe0\x09\x00\xd0\xf3", 12u))
        return 0;

    const uint64_t budget = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / 21u;
    unsigned count = 9u - registers->x;
    if (count > room / 6)
        count = (unsigned)(room / 6);
    if (count > budget)
        count = (unsigned)budget;

    unsigned cycles = 0;
    long executed = 0;
    long progress = 0;
    for (unsigned record = 0; record < count; ++record) {
        const uint32_t source = bank | registers->y;
        if (!Lufia2DecompIsWramByte(source) && !Lufia2DecompIsLoRomDataByte(cart, source))
            break;

        Lufia2SpanFetch(registers, pc, 3u, record != 0u);
        const uint8_t value = registers->read(registers->mem, source);
        Lufia2DecompApplyOperand(registers, 0xa9u, value, true);
        const unsigned read_cycles = registers->y >= 0x0100u ? 5u : 4u;
        Lufia2SpanAccountInstruction(cpu, read_cycles);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = executed + 1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 3u, 3u, true);
        registers->write(registers->mem, bank | (0x0b84u + registers->x), value);
        Lufia2SpanAccountInstruction(cpu, 6u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = executed + 2;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 6u, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 7u, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 8u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xe0u, 9u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, pc + 11u, 2u, true);
        registers->pc = registers->z ? 0xf1f6u : 0xf1e9u;
        registers->cyclesUsed = registers->z ? 2u : 3u;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        cycles += read_cycles + 13u + registers->cyclesUsed;
        executed += 6;
    }

    *tail_steps = progress ? executed - progress : 1 - executed;
    return cycles;
}

static int RunItemRecordMasks(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x81f1f6u || opcode != 0xb9u || registers->e || !registers->mf || registers->xf ||
        registers->y > 0xfffcu || !Lufia2DecompIsWramByte(bank | 0x09ffu) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 44u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 28u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x00\x00\x8d\xfc\x09\xb9\x01\x00\x8d\xfd\x09\xb9\x02\x00\x8d\xfe\x09"
        "\xb9\x03\x00\x8d\xff\x09\xc8\xc8\xc8\xc8", 27u))
        return 0;

    for (unsigned byte = 0; byte < 4u; ++byte) {
        const uint32_t address = bank + registers->y + byte;
        if (!Lufia2DecompIsWramByte(address) && !Lufia2DecompIsLoRomDataByte(cart, address))
            return 0;
    }

    long progress = 0;
    unsigned cycles = 0;
    for (unsigned byte = 0; byte < 4u; ++byte) {
        Lufia2SpanFetch(registers, pc + byte * 6u, 3u, byte != 0u);
        const uint8_t mask = registers->read(registers->mem, bank + registers->y + byte);
        Lufia2DecompApplyOperand(registers, 0xa9u, mask, true);
        const unsigned read_cycles = registers->y + byte >= 0x0100u ? 5u : 4u;
        Lufia2SpanAccountInstruction(cpu, read_cycles);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 1 + byte * 2u;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + byte * 6u + 3u, 3u, true);
        registers->write(registers->mem, bank | (0x09fcu + byte), mask);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 2 + byte * 2u;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        cycles += read_cycles + 4u;
    }

    for (unsigned step = 0; step < 4u; ++step) {
        Lufia2SpanFetch(registers, pc + 24u + step, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    registers->pc = 0xf212u;
    registers->cyclesUsed = 2u;
    *tail_steps = progress ? 12 - progress : -11;
    return cycles + 8u;
}

static int RunItemRecordPayload(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                     uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc < 0x81f217u || pc > 0x81f274u || (pc - 0x81f217u) % 31u ||
        opcode != 0x4eu || registers->e || !registers->mf || registers->xf ||
        !Lufia2DecompIsWramByte(bank | 0x0a00u) || registers->x > 0x147au || registers->y >= 0xffffu ||
        !Lufia2DecompIsWramByte(bank | (0x0b85u + registers->x)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 47u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 26u, &offset);
    const unsigned mask_byte = 0xfcu + (pc - 0x81f217u) / 31u;
    if (!cart || cart->rom[offset + 1u] != mask_byte || memcmp(cart->rom + offset + 2u,
        "\x09\x90\x0e\xb9\x00\x00\xc8\x9d\x84\x0b\xb9\x00\x00\xc8\x9d\x85\x0b"
        "\xe8\xe8\xce\x00\x0a\xd0\xe6", 24u))
        return 0;

    const uint32_t source = bank | registers->y;
    if ((!Lufia2DecompIsWramByte(source) && !Lufia2DecompIsLoRomDataByte(cart, source)) ||
        (!Lufia2DecompIsWramByte(source + 1u) && !Lufia2DecompIsLoRomDataByte(cart, source + 1u)))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint32_t mask = bank | (0x0900u + mask_byte);
    Lufia2DecompModifyWram(registers, 0x4eu, mask, mask + 1u, false);
    Lufia2SpanAccountInstruction(cpu, 6u);
    long progress = (dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch) ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    const bool copy = registers->c;
    Lufia2SpanAccountInstruction(cpu, copy ? 2u : 3u);
    unsigned cycles = copy ? 8u : 9u;
    if (copy) {
        for (unsigned byte = 0; byte < 2u; ++byte) {
            const uint32_t instruction = pc + 5u + byte * 7u;
            Lufia2SpanFetch(registers, instruction, 3u, true);
            const uint8_t value = registers->read(registers->mem, bank | registers->y);
            Lufia2DecompApplyOperand(registers, 0xa9u, value, true);
            const unsigned read_cycles = registers->y >= 0x100u ? 5u : 4u;
            Lufia2SpanAccountInstruction(cpu, read_cycles);
            if (dynamic_epoch != s_interp_dynamic_progress_epoch)
                progress = 3 + byte * 3u;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

            Lufia2SpanFetch(registers, instruction + 3u, 1u, true);
            Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
            Lufia2SpanAccountInstruction(cpu, 2u);

            Lufia2SpanFetch(registers, instruction + 4u, 3u, true);
            registers->write(registers->mem, bank | (0x0b84u + registers->x + byte), value);
            Lufia2SpanAccountInstruction(cpu, 6u);
            if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
                progress = 5 + byte * 3u;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
            cycles += read_cycles + 8u;
        }
    }

    for (unsigned step = 0; step < 2u; ++step) {
        Lufia2SpanFetch(registers, pc + 19u + step, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }
    cycles += 4u;

    Lufia2SpanFetch(registers, pc + 21u, 3u, true);
    const uint32_t remaining = bank | 0x0a00u;
    Lufia2DecompModifyWram(registers, 0xceu, remaining, remaining + 1u, false);
    Lufia2SpanAccountInstruction(cpu, 6u);
    const long executed = copy ? 12 : 6;
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = executed - 1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 24u, 2u, true);
    registers->pc = (uint16_t)(registers->z ? pc + 26u : pc);
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? executed - progress : 1 - executed;
    return cycles + 6u + registers->cyclesUsed;
}

static int RunItemRecordCounter(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                 uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t count_address = ((uint32_t)registers->db << 16) | 0x0a00u;
    if (pc < 0x81f212u || pc > 0x81f26fu || (pc - 0x81f212u) % 31u ||
        opcode != 0xa9u || registers->e || !registers->mf ||
        !Lufia2DecompIsWramByte(count_address) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 6u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 5u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x08\x8d\x00\x0a", 4u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompApplyOperand(registers, 0xa9u, 8u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 2u, 3u, true);
    registers->write(registers->mem, count_address, 8u);
    registers->pc = (uint16_t)(pc + 5u);
    registers->cyclesUsed = 4u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch
        ? 0 : -1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 6;
}

static int RunItemRecordReturn(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x81f28eu || opcode != 0xabu || registers->e ||
        registers->sp < 0x1f00u || registers->sp > 0x1ffau ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 14u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 3u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x28\x6b", 2u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->n = (registers->db & 0x80u) != 0u;
    registers->z = registers->db == 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 1u, true);
    interp816_setFlags(registers, registers->read(registers->mem, ++registers->sp));
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    uint16_t target = registers->read(registers->mem, ++registers->sp);
    target |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->k = registers->read(registers->mem, ++registers->sp);
    registers->pc = (uint16_t)(target + 1u);
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    *tail_steps = progress ? 3 - progress : -2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 14;
}

static int RunItemNameCopy(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                            uint64_t dynamic_epoch, uint64_t write_epoch, long room, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x81f2c2u || opcode != 0xb9u || registers->e || registers->mf || registers->xf ||
        registers->x >= 12u || (registers->x & 1u) || registers->y > 0x3816u || room < 8 ||
        !Lufia2DecompIsWramByte(bank | 0x0b82u) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 15u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\xe8\xc7\x9d\x77\x0b\xc8\xc8\xe8\xe8\xe0\x0c\x00\xd0\xf1", 14u))
        return 0;

    unsigned words = (12u - registers->x) / 2u;
    const uint64_t budget = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / 27u;
    if (words > room / 8)
        words = (unsigned)(room / 8);
    if (words > budget)
        words = (unsigned)budget;
    const uint32_t source = (bank | 0xc7e8u) + registers->y;
    if (!words || source + 2u * words - 1u > (bank | 0xffffu) ||
        !Lufia2DecompIsLoRomDataByte(cart, source) ||
        !Lufia2DecompIsLoRomDataByte(cart, source + 2u * words - 1u))
        return 0;

    long executed = 0;
    long progress = 0;
    unsigned cycles = 0;
    for (unsigned word = 0; word < words; ++word) {
        Lufia2SpanFetch(registers, pc, 3u, word != 0u);
        const uint32_t address = source + 2u * word;
        const uint16_t value = Lufia2DecompReadData(registers, address, address + 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, value, false);
        const unsigned read_cycles = 5u + (registers->y >= 0x18u);
        Lufia2SpanAccountInstruction(cpu, read_cycles);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = executed + 1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 3u, 3u, true);
        const uint32_t destination = bank | (0x0b77u + registers->x);
        Lufia2DecompWriteData(registers, destination, destination + 1u, value, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = executed + 2;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        for (unsigned index = 0; index < 4u; ++index) {
            Lufia2SpanFetch(registers, pc + 6u + index, 1u, true);
            if (index < 2u)
                Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
            else
                Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
            Lufia2SpanAccountInstruction(cpu, 2u);
        }

        Lufia2SpanFetch(registers, pc + 10u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xe0u, 12u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, pc + 13u, 2u, true);
        registers->pc = registers->z ? 0xf2d1u : 0xf2c2u;
        registers->cyclesUsed = registers->z ? 2u : 3u;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        cycles += read_cycles + 18u + registers->cyclesUsed;
        executed += 8;
    }

    *tail_steps = progress ? executed - progress : 1 - executed;
    return cycles;
}

static int Lufia2DecompRunItemRecord(Interp816 *registers, CpuState *cpu,
                                    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                    uint64_t write_epoch, long room, long *tail_steps) {
    if (pc == 0x81f2c2u)
        return RunItemNameCopy(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, room, tail_steps);
    if (pc < 0x81f1e9u) {
        if (room < 5)
            return 0;
        return RunItemRecordSetup(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
    }
    if (pc == 0x81f1e9u)
        return RunItemRecordHeaderCopy(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, room, tail_steps);
    if (pc == 0x81f212u || pc == 0x81f231u || pc == 0x81f250u || pc == 0x81f26fu) {
        if (room < 2)
            return 0;
        return RunItemRecordCounter(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
    }
    if (pc == 0x81f28eu) {
        if (room < 3)
            return 0;
        return RunItemRecordReturn(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
    }
    if (room < 12)
        return 0;
    if (pc == 0x81f1f6u)
        return RunItemRecordMasks(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
    return RunItemRecordPayload(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
}
