static int RunEventRecordHeaderCopy(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
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

static int RunEventRecordMasks(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
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

static int RunEventRecordPayload(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
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

static int Lufia2DecompRunEventRecord(Interp816 *registers, CpuState *cpu,
                                    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                    uint64_t write_epoch, long room, long *tail_steps) {
    if (pc == 0x81f1e9u)
        return RunEventRecordHeaderCopy(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, room, tail_steps);
    if (room < 12)
        return 0;
    if (pc == 0x81f1f6u)
        return RunEventRecordMasks(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
    return RunEventRecordPayload(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
}
