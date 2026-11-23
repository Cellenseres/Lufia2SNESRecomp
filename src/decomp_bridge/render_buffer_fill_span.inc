static int Lufia2DecompRunOamHide(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                       uint64_t dynamic_epoch, uint64_t write_epoch) {
    if (pc != 0x81b5b8u || opcode != 0x9du || registers->e || registers->mf || registers->xf ||
        registers->x > 0x1fcu || (registers->x & 3u) || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        !Lufia2DecompIsWramByte(((uint32_t)registers->db << 16) | 0x02fdu))
        return 0;

    const uint64_t room = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / 18u;
    unsigned count = registers->x / 4u + 1u;
    if (count > 8u)
        count = 8u;
    if (room < count)
        count = (unsigned)room;
    if (!count)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 9u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x00\x01\xca\xca\xca\xca\x10\xf7", 8u))
        return 0;

    const uint32_t buffer = ((uint32_t)registers->db << 16) | 0x0100u;
    unsigned cycles = 0;
    for (unsigned sprite = 0; sprite < count; ++sprite) {
        Lufia2SpanFetch(registers, pc, 3u, sprite != 0u);
        const uint32_t entry = buffer + registers->x;
        Lufia2DecompWriteData(registers, entry, entry + 1u, registers->a, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        for (unsigned offset = 3u; offset < 7u; ++offset) {
            Lufia2SpanFetch(registers, pc + offset, 1u, true);
            Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
            Lufia2SpanAccountInstruction(cpu, 2u);
        }

        Lufia2SpanFetch(registers, pc + 7u, 2u, true);
        registers->pc = registers->n ? 0xb5c1u : 0xb5b8u;
        registers->cyclesUsed = registers->n ? 2u : 3u;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        cycles += 15u + registers->cyclesUsed;
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
