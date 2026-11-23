static int MenuDecimalDivisor(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                 uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x808a01u || opcode != 0xbfu || registers->e || !registers->mf ||
        registers->xf || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || registers->x > 18u ||
        registers->sp < 0x1f02u || registers->sp > 0x1fffu ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 31u)
        return 0;

    unsigned rom_offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 22u, &rom_offset);
    if (!cart || memcmp(cart->rom + rom_offset + 1u,
        "\x57\x8a\x80\x85\x60\xbf\x58\x8a\x80\x85\x61"
        "\xbf\x59\x8a\x80\x85\x62\x5a\xa0\x30\x00", 21u) ||
        !Lufia2DecompIsLoRomDataByte(cart, 0x808a59u + registers->x))
        return 0;

    const uint32_t source = 0x808a57u + registers->x;
    for (unsigned byte = 0; byte < 3u; ++byte) {
        const uint32_t instruction = pc + byte * 6u;
        Lufia2SpanFetch(registers, instruction, 4u, byte != 0u);
        const uint8_t value = registers->read(registers->mem, source + byte);
        Lufia2DecompApplyOperand(registers, 0xa9u, value, true);
        Lufia2SpanAccountInstruction(cpu, 5u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, instruction + 4u, 2u, true);
        registers->write(registers->mem, 0x60u + byte, value);
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    Lufia2SpanFetch(registers, pc + 18u, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)(registers->y >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->y);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 19u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x30u, false);
    registers->pc = 0x8a17u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = 1;
    return 31;
}

static int MenuDecimalSubtract(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                          uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x808a17u || opcode != 0xc2u || registers->e || !registers->mf ||
        registers->xf || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        !Lufia2DecompIsWramByte(((uint32_t)registers->db << 16) | 0x0572u) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 40u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 26u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x20\xad\x70\x05\x38\xe5\x60\x8d\x70\x05\xe2\x20\xad\x72\x05"
        "\xe5\x62\x90\x06\x8d\x72\x05\xc8\x80\xe6", 25u))
        return 0;

    const uint32_t number = ((uint32_t)registers->db << 16) | 0x0570u;
    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 2u, 3u, true);
    const uint16_t low_word = Lufia2DecompReadData(registers, number, number + 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, low_word, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 5u, 1u, true);
    registers->c = true;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    const uint16_t divisor = Lufia2DecompReadData(registers, 0x60u, 0x61u, true);
    Lufia2DecompApplyOperand(registers, 0xe9u, divisor, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 8u, 3u, true);
    Lufia2DecompWriteData(registers, number, number + 1u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 5u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 11u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 13u, 3u, true);
    const uint8_t high_byte = registers->read(registers->mem, number + 2u);
    Lufia2DecompApplyOperand(registers, 0xa9u, high_byte, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool number_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 16u, 2u, true);
    const uint8_t divisor_high = registers->read(registers->mem, 0x62u);
    Lufia2DecompApplyOperand(registers, 0xe9u, divisor_high, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    const bool divisor_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 18u, 2u, true);
    if (!registers->c) {
        Lufia2SpanAccountInstruction(cpu, 3u);
        registers->pc = 0x8a31u;
        registers->cyclesUsed = 3u;
        *tail_steps = divisor_progress ? 1 : number_progress ? 2 : 4;
        return 32;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 20u, 3u, true);
    registers->write(registers->mem, number + 2u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 23u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 24u, 2u, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    registers->pc = (uint16_t)pc;
    registers->cyclesUsed = 3u;
    *tail_steps = 2;
    return 40;
}

static int MenuDecimalPlace(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                               uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x808a31u || opcode != 0xc2u || registers->e || !registers->mf ||
        registers->xf || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || !cpu->ram ||
        registers->sp < 0x1f00u || registers->sp > 0x1ffdu ||
        !Lufia2DecompIsWramByte(((uint32_t)registers->db << 16) | 0x0572u) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 49u)
        return 0;

    const unsigned index = cpu->ram[registers->sp + 1u] |
        ((unsigned)cpu->ram[registers->sp + 2u] << 8);
    if (index > 7u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 27u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x20\xad\x70\x05\x18\x65\x60\x8d\x70\x05\xe2\x20\x98\x7a"
        "\x99\x67\x05\xe8\xe8\xe8\xc8\xc0\x07\x00\xd0\xb5", 26u))
        return 0;

    const uint32_t bank = (uint32_t)registers->db << 16;
    const uint32_t number = bank | 0x0570u;
    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 2u, 3u, true);
    const uint16_t remainder = Lufia2DecompReadData(registers, number, number + 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, remainder, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 5u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    const uint16_t divisor = Lufia2DecompReadData(registers, 0x60u, 0x61u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, divisor, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 8u, 3u, true);
    Lufia2DecompWriteData(registers, number, number + 1u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 5u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 11u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 13u, 1u, true);
    const uint8_t digit = (uint8_t)registers->y;
    Lufia2DecompApplyOperand(registers, 0xa9u, digit, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 14u, 1u, true);
    uint16_t digit_index = registers->read(registers->mem, ++registers->sp);
    digit_index |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, digit_index, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 15u, 3u, true);
    registers->write(registers->mem, bank | (0x0567u + digit_index), digit);
    Lufia2SpanAccountInstruction(cpu, 6u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    for (unsigned offset = 18u; offset < 21u; ++offset) {
        Lufia2SpanFetch(registers, pc + offset, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }
    Lufia2SpanFetch(registers, pc + 21u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 22u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xc0u, 7u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 25u, 2u, true);
    registers->pc = registers->z ? 0x8a4cu : 0x8a01u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = 6;
    return 46u + registers->cyclesUsed;
}

static int MenuDecimalLast(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                              uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x808a4cu || opcode != 0xadu || registers->e || !registers->mf ||
        registers->xf || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        registers->sp < 0x1f00u || registers->sp > 0x1ffdu ||
        !Lufia2DecompIsWramByte(((uint32_t)registers->db << 16) | 0x0570u) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 17u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 10u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x70\x05\x18\x69\x30\x8d\x6e\x05\x7a", 9u))
        return 0;

    const uint32_t bank = (uint32_t)registers->db << 16;
    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint8_t remainder = registers->read(registers->mem, bank | 0x0570u);
    Lufia2DecompApplyOperand(registers, 0xa9u, remainder, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x30u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 6u, 3u, true);
    registers->write(registers->mem, bank | 0x056eu, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 1u, true);
    uint16_t caller_index = registers->read(registers->mem, ++registers->sp);
    caller_index |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, caller_index, false);
    registers->pc = 0x8a56u;
    registers->cyclesUsed = 5u;
    Lufia2SpanAccountInstruction(cpu, 5u);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : 1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 17;
}

static int Lufia2DecompRunMenuDecimal(Interp816 *registers, CpuState *cpu,
                                     uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                     uint64_t write_epoch, long room, long *tail_steps) {
    switch (pc) {
    case 0x808a01u:
        if (room >= 8)
            return MenuDecimalDivisor(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x808a17u:
        if (room >= 12)
            return MenuDecimalSubtract(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x808a31u:
        if (room >= 15)
            return MenuDecimalPlace(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x808a4cu:
        if (room >= 5)
            return MenuDecimalLast(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    }
    return 0;
}

static int Lufia2DecompRunMenuNumberRead(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                              uint64_t dynamic_epoch, uint64_t write_epoch) {
    if (pc != 0x808c7bu || opcode != 0xebu || registers->e || !registers->mf || registers->xf ||
        registers->dp || registers->db != 0x7eu || registers->y > 2u ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 25u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 11u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\xb1\x56\x99\x70\x05\xc8\xeb\x0a\x90\xf5", 10u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    MenuGlyphExchangeBytes(registers);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 1u, 2u, true);
    const unsigned source = Lufia2DecompReadData(registers, 0x56u, 0x57u, true);
    const unsigned source_indexed = source + registers->y;
    const uint8_t byte = registers->read(registers->mem, 0x7e0000u + source_indexed);
    Lufia2DecompApplyOperand(registers, 0xa9u, byte, true);
    const unsigned read_cycles = 5u + ((source >> 8) != (source_indexed >> 8));
    Lufia2SpanAccountInstruction(cpu, read_cycles);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 3u, true);
    registers->write(registers->mem, 0x7e0570u + registers->y, byte);
    Lufia2SpanAccountInstruction(cpu, 6u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 6u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 7u, 1u, true);
    MenuGlyphExchangeBytes(registers);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 8u, 1u, true);
    const uint8_t format = (uint8_t)registers->a;
    registers->c = (format & 0x80u) != 0;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(format << 1u), true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 9u, 2u, true);
    registers->pc = registers->c ? 0x8c86u : 0x8c7bu;
    registers->cyclesUsed = registers->c ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    return 16u + read_cycles + registers->cyclesUsed;
}

static int Lufia2DecompRunMenuDecimalTrim(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                               uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x808956u || opcode != 0xb9u || registers->e || !registers->mf || registers->xf ||
        registers->y >= 7u || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        !Lufia2DecompIsWramByte(((uint32_t)registers->db << 16) | 0x056du) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 24u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 18u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x67\x05\xc9\x30\xd0\x0b\xa9\x20\x99\x67\x05\xc8\xc0\x07\x00\xd0\xee", 17u))
        return 0;

    const uint32_t digit_address = (((uint32_t)registers->db << 16) | 0x0567u) + registers->y;
    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint8_t digit = registers->read(registers->mem, digit_address);
    Lufia2DecompApplyOperand(registers, 0xa9u, digit, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x30u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    if (!registers->z) {
        registers->pc = 0x8968u;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 2 : -2;
        return 9;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x20u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 9u, 3u, true);
    registers->write(registers->mem, digit_address, 0x20u);
    Lufia2SpanAccountInstruction(cpu, 6u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 12u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 13u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xc0u, 7u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 16u, 2u, true);
    registers->pc = registers->z ? 0x8968u : 0x8956u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = 3;
    return 21u + registers->cyclesUsed;
}

static int Lufia2DecompRunMenuNumberSign(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                              uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool fill = pc == 0x808c90u;
    if ((!fill && pc != 0x808c86u) || opcode != (fill ? 0xc0u : 0xadu) ||
        registers->e || !registers->mf || registers->xf || registers->db != 0x7eu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        (fill && registers->y > 3u) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (fill ? 18u : 14u))
        return 0;

    unsigned offset;
    const unsigned length = fill ? 13u : 10u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = fill ? "\x03\x00\xf0\x08\xa9\xff\x99\x70\x05\xc8\x80\xf3" :
                                   "\x7b\x05\x29\x04\xf0\x10\xeb\x10\x0d";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    if (!fill) {
        Lufia2SpanFetch(registers, pc, 3u, false);
        const uint8_t flags = registers->read(registers->mem, 0x7e057bu);
        Lufia2DecompApplyOperand(registers, 0xa9u, flags, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 3u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0x29u, 4u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 5u, 2u, true);
        if (registers->z) {
            registers->pc = 0x8c9du;
            registers->cyclesUsed = 3u;
            Lufia2SpanAccountInstruction(cpu, 3u);
            *tail_steps = progress ? 2 : -2;
            return 9;
        }
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 7u, 1u, true);
        MenuGlyphExchangeBytes(registers);
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, pc + 8u, 2u, true);
        registers->pc = registers->n ? 0x8c90u : 0x8c9du;
        registers->cyclesUsed = registers->n ? 2u : 3u;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        *tail_steps = progress ? 4 : -4;
        return 11u + registers->cyclesUsed;
    }

    Lufia2SpanFetch(registers, pc, 3u, false);
    Lufia2DecompApplyOperand(registers, 0xc0u, 3u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    if (registers->z) {
        registers->pc = 0x8c9du;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = -1;
        return 6;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0xffu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 7u, 3u, true);
    registers->write(registers->mem, 0x7e0570u + registers->y, 0xffu);
    Lufia2SpanAccountInstruction(cpu, 6u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 10u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 11u, 2u, true);
    registers->pc = 0x8c90u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = 2;
    return 18;
}

static int RunMenuDecimalMinus(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool check_sign = pc == 0x808968u;
    const bool search = pc == 0x80896fu;
    if ((!check_sign && !search && pc != 0x808979u) ||
        opcode != (check_sign ? 0xa5u : search ? 0xb9u : 0xa9u) ||
        registers->e || !registers->mf || registers->xf || registers->dp || registers->db != 0x7eu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        (search && registers->y > 7u) ||
        (!check_sign && !search && registers->y > 7u && registers->y != 0xffffu) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (search ? 13u : 8u))
        return 0;

    unsigned offset;
    const unsigned length = check_sign ? 7u : search ? 10u : 5u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = check_sign ? "\x55\xf0\x12\xa0\x07\x00" :
        search ? "\x67\x05\xc9\x20\xf0\x03\x88\x10\xf6" : "\x2d\x99\x67\x05";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    if (!check_sign && !search) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        Lufia2DecompApplyOperand(registers, 0xa9u, 0x2du, true);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 2u, 3u, true);
        registers->write(registers->mem, 0x7e0567u + registers->y, 0x2du);
        registers->pc = 0x897eu;
        registers->cyclesUsed = 6u;
        Lufia2SpanAccountInstruction(cpu, 6u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        *tail_steps = 0;
        return 8;
    }

    Lufia2SpanFetch(registers, pc, check_sign ? 2u : 3u, false);
    const uint8_t value = registers->read(registers->mem, check_sign ? 0x55u : 0x7e0567u + registers->y);
    Lufia2DecompApplyOperand(registers, 0xa9u, value, true);
    Lufia2SpanAccountInstruction(cpu, check_sign ? 3u : 4u);
    const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    if (check_sign) {
        Lufia2SpanFetch(registers, pc + 2u, 2u, true);
        if (registers->z) {
            registers->pc = 0x897eu;
            registers->cyclesUsed = 3u;
            Lufia2SpanAccountInstruction(cpu, 3u);
            *tail_steps = progress ? 1 : -1;
            return 6;
        }
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 4u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, 7u, false);
        registers->pc = 0x896fu;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 2 : -2;
        return 8;
    }

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x20u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    if (registers->z) {
        registers->pc = 0x8979u;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 2 : -2;
        return 9;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 7u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y - 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    registers->pc = registers->n ? 0x8979u : 0x896fu;
    registers->cyclesUsed = registers->n ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 4 : -4;
    return 10u + registers->cyclesUsed;
}

static void MenuDecimalStoreCursor(Interp816 *registers, CpuState *cpu, bool fetch_opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch) {
    Lufia2SpanFetch(registers, 0x80899fu, 2u, fetch_opcode);
    Lufia2DecompWriteData(registers, 0x60u, 0x61u, registers->x, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, 0x8089a1u, 2u, true);
    registers->write(registers->mem, 0x62u, 0u);
    registers->pc = 0x89a3u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
}

static int RunMenuDecimalCursor(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                 uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool select_format = pc == 0x80897eu;
    const bool skip_blank = pc == 0x808988u;
    const bool store_cursor = pc == 0x80899fu;
    if ((!select_format && !skip_blank && !store_cursor && pc != 0x808991u) ||
        opcode != (select_format ? 0xadu : skip_blank ? 0xb5u : store_cursor ? 0x86u : 0xa9u) ||
        registers->e || !registers->mf || registers->xf || registers->dp ||
        registers->db != 0x7eu || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        (skip_blank && (registers->x < 0x0567u || registers->x > 0x056eu)) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (select_format ? 11u : skip_blank ? 13u : store_cursor ? 7u : 28u))
        return 0;

    unsigned offset;
    const unsigned length = select_format ? 10u : skip_blank ? 9u : store_cursor ? 4u : 18u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = select_format ? "\x7b\x05\x29\x10\xf0\x0c\xa2\x67\x05" :
        skip_blank ? "\x00\xc9\x20\xd0\x11\xe8\x80\xf7" :
        store_cursor ? "\x60\x64\x62" :
        "\x00\xeb\xa5\x54\xc2\x20\x18\x69\x67\x05\xaa\xe2\x20\x86\x60\x64\x62";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    if (store_cursor) {
        MenuDecimalStoreCursor(registers, cpu, false, dynamic_epoch, write_epoch);
        *tail_steps = 0;
        return 7;
    }

    if (select_format || skip_blank) {
        Lufia2SpanFetch(registers, pc, select_format ? 3u : 2u, false);
        const uint8_t value = registers->read(registers->mem, select_format ? 0x7e057bu : registers->x);
        Lufia2DecompApplyOperand(registers, 0xa9u, value, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        const unsigned branch = select_format ? 5u : 4u;
        Lufia2SpanFetch(registers, pc + branch - 2u, 2u, true);
        Lufia2DecompApplyOperand(registers, select_format ? 0x29u : 0xc9u,
                                select_format ? 0x10u : 0x20u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + branch, 2u, true);
        const bool taken = select_format ? registers->z : !registers->z;
        Lufia2SpanAccountInstruction(cpu, taken ? 3u : 2u);
        if (taken) {
            registers->pc = select_format ? 0x8991u : 0x899fu;
            registers->cyclesUsed = 3u;
            *tail_steps = progress ? 2 : -2;
            return 9;
        }
        if (select_format) {
            Lufia2SpanFetch(registers, pc + 7u, 3u, true);
            Lufia2DecompApplyOperand(registers, 0xa2u, 0x0567u, false);
            registers->pc = 0x8988u;
            registers->cyclesUsed = 3u;
            Lufia2SpanAccountInstruction(cpu, 3u);
            *tail_steps = progress ? 3 : -3;
            return 11;
        }

        Lufia2SpanFetch(registers, pc + 6u, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, pc + 7u, 2u, true);
        registers->pc = 0x8988u;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 4 : -4;
        return 13;
    }

    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    MenuGlyphExchangeBytes(registers);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    const uint8_t digit_offset = registers->read(registers->mem, 0x54u);
    Lufia2DecompApplyOperand(registers, 0xa9u, digit_offset, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 7u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 8u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0567u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 11u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 12u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    MenuDecimalStoreCursor(registers, cpu, true, dynamic_epoch, write_epoch);
    *tail_steps = 0;
    return 28;
}

static int Lufia2DecompRunMenuDecimalOutput(Interp816 *registers, CpuState *cpu,
                                           uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                           uint64_t write_epoch, long room, long *tail_steps) {
    if (pc < 0x80897eu) {
        const long instructions = pc == 0x80896fu ? 5 : pc == 0x808968u ? 3 : 2;
        return room >= instructions
            ? RunMenuDecimalMinus(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps)
            : 0;
    }

    const long instructions = pc == 0x808991u ? 10 : pc == 0x808988u ? 5 : pc == 0x80897eu ? 4 : 2;
    return room >= instructions
        ? RunMenuDecimalCursor(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps)
        : 0;
}
