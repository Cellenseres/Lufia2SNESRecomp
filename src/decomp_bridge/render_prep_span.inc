static int MenuGlyphRow(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                           uint64_t dynamic_epoch, uint64_t write_epoch) {
    const uint32_t remaining = ((uint32_t)registers->db << 16) | 0x0566u;
    if (pc != 0x8088ddu || opcode != 0xceu || registers->e || !registers->mf ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        !Lufia2DecompIsWramByte(remaining) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 9u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 5u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x66\x05\xd0\x06", 4u))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    Lufia2DecompModifyWram(registers, 0xceu, remaining, remaining + 1u, false);
    Lufia2SpanAccountInstruction(cpu, 6u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    registers->pc = registers->z ? 0x88e2u : 0x88e8u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    return 6u + registers->cyclesUsed;
}

static int MenuDecimalDigitFill(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch) {
    if (pc != 0x8089fau || opcode != 0x99u || registers->e || !registers->mf ||
        registers->xf || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || registers->y > 7u ||
        !Lufia2DecompIsWramByte(((uint32_t)registers->db << 16) | 0x056eu))
        return 0;

    const uint64_t room = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / 11u;
    unsigned count = registers->y + 1u;
    if (room < count)
        count = (unsigned)room;
    if (!count)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 6u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x67\x05\x88\x10\xfa", 5u))
        return 0;

    const uint32_t buffer = ((uint32_t)registers->db << 16) | 0x0567u;
    unsigned cycles = 0;
    for (unsigned digit = 0; digit < count; ++digit) {
        Lufia2SpanFetch(registers, pc, 3u, digit != 0u);
        registers->write(registers->mem, buffer + registers->y, (uint8_t)registers->a);
        Lufia2SpanAccountInstruction(cpu, 6u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 3u, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y - 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 4u, 2u, true);
        registers->pc = registers->n ? 0x8a00u : 0x89fau;
        registers->cyclesUsed = registers->n ? 2u : 3u;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        cycles += 8u + registers->cyclesUsed;
    }
    return (int)cycles;
}

static int SpritePieceStart(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                              uint64_t dynamic_epoch, uint64_t write_epoch) {
    if (pc != 0x868c1fu || opcode != 0xe2u || registers->e || registers->xf ||
        registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || !cpu->ram || registers->x >= 48u ||
        !Lufia2DecompIsWramByte(((uint32_t)registers->db << 16) | 0x1387u) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 43u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 28u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x20\xbd\x28\x13\x85\x5d\xbd\x58\x13\x85\x5e\xbd\x38\x12"
        "\x85\x5f\xa0\x01\x00\xb7\x5d\x0a\x85\x56\x64\x57\xc8", 27u))
        return 0;

    static const uint16_t frame_parts[] = {0x1328u, 0x1358u, 0x1238u};
    const unsigned wram_bank = registers->db == 0x7fu ? 0x10000u : 0u;
    uint32_t pointer = 0;
    for (unsigned byte = 0; byte < 3u; ++byte)
        pointer |= (uint32_t)cpu->ram[wram_bank + frame_parts[byte] + registers->x] << (byte * 8u);

    const uint32_t expected = (pointer + 1u) & 0xffffffu;
    if (!Lufia2DecompIsWramByte(expected) && !Lufia2DecompIsLoRomDataByte(cart, expected))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    const uint32_t bank = (uint32_t)registers->db << 16;
    for (unsigned byte = 0; byte < 3u; ++byte) {
        const uint32_t instruction = pc + 2u + byte * 5u;
        Lufia2SpanFetch(registers, instruction, 3u, true);
        const uint8_t value = registers->read(registers->mem, bank | (frame_parts[byte] + registers->x));
        Lufia2DecompApplyOperand(registers, 0xa9u, value, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, instruction + 3u, 2u, true);
        registers->write(registers->mem, 0x5du + byte, value);
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    Lufia2SpanFetch(registers, pc + 17u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, 1u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 20u, 2u, true);
    uint32_t source = Lufia2DecompReadData(registers, 0x5du, 0x5eu, true);
    source |= (uint32_t)registers->read(registers->mem, 0x5fu) << 16;
    const uint8_t input_size = registers->read(registers->mem, (source + registers->y) & 0xffffffu);
    Lufia2DecompApplyOperand(registers, 0xa9u, input_size, true);
    Lufia2SpanAccountInstruction(cpu, 6u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 22u, 1u, true);
    registers->c = (input_size & 0x80u) != 0;
    const uint8_t size_bits = (uint8_t)(input_size << 1u);
    Lufia2DecompApplyOperand(registers, 0xa9u, size_bits, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 23u, 2u, true);
    registers->write(registers->mem, 0x56u, size_bits);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 25u, 2u, true);
    registers->write(registers->mem, 0x57u, 0);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 27u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, 2u, false);
    registers->pc = 0x8c3bu;
    registers->cyclesUsed = 2u;
    Lufia2SpanAccountInstruction(cpu, 2u);
    return 43;
}

static int Lufia2DecompRunRenderPrep(Interp816 *registers, CpuState *cpu,
                                    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                    uint64_t write_epoch, long room, long *tail_steps) {
    switch (pc) {
    case 0x8088ddu:
        if (room >= 2) {
            *tail_steps = 1;
            return MenuGlyphRow(registers, cpu, pc, opcode, dynamic_epoch, write_epoch);
        }
        break;
    case 0x8089fau:
        if (room >= 3) {
            *tail_steps = 2;
            return MenuDecimalDigitFill(registers, cpu, pc, opcode, dynamic_epoch, write_epoch);
        }
        break;
    case 0x868c1fu:
        if (room >= 13) {
            *tail_steps = 1;
            return SpritePieceStart(registers, cpu, pc, opcode, dynamic_epoch, write_epoch);
        }
        break;
    }
    return 0;
}
