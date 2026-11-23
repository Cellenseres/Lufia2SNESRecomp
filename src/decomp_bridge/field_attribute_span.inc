static int FieldAttributeLookup(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x80edeau || opcode != 0x5au || registers->e || !registers->mf || registers->xf ||
        registers->dp || registers->x == 0xffffu || registers->sp < 0x1f02u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        !cpu->ram || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 35u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 21u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\xbf\x01\x00\x7f\x29\x03\xeb\xbf\x00\x00\x7f\xa8\x64\x56\xb7\x60\x89\xf0\xf0\x06", 20u))
        return 0;

    const uint32_t pointer = cpu->ram[0x60u] |
        ((uint32_t)cpu->ram[0x61u] << 8) | ((uint32_t)cpu->ram[0x62u] << 16);
    const unsigned cell_offset = 0x10000u + registers->x;
    const unsigned tile_index = cpu->ram[cell_offset] | ((unsigned)(cpu->ram[cell_offset + 1u] & 3u) << 8);
    const uint32_t expected = (pointer + tile_index) & 0xffffffu;
    if (!Lufia2DecompIsWramByte(expected) && !Lufia2DecompIsLoRomDataByte(cart, expected))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->write(registers->mem, registers->sp--, (uint8_t)(registers->y >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->y);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 4u, true);
    const uint8_t tile_high = registers->read(registers->mem, 0x7f0001u + registers->x);
    Lufia2DecompApplyOperand(registers, 0xa9u, tile_high, true);
    Lufia2SpanAccountInstruction(cpu, 5u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 3u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 7u, 1u, true);
    MenuGlyphExchangeBytes(registers);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 8u, 4u, true);
    const uint8_t tile_low = registers->read(registers->mem, 0x7f0000u + registers->x);
    Lufia2DecompApplyOperand(registers, 0xa9u, tile_low, true);
    Lufia2SpanAccountInstruction(cpu, 5u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 12u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 13u, 2u, true);
    registers->write(registers->mem, 0x56u, 0u);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 15u, 2u, true);
    uint32_t source = Lufia2DecompReadData(registers, 0x60u, 0x61u, true);
    source |= (uint32_t)registers->read(registers->mem, 0x62u) << 16;
    const uint8_t classification = registers->read(registers->mem, (source + registers->y) & 0xffffffu);
    Lufia2DecompApplyOperand(registers, 0xa9u, classification, true);
    Lufia2SpanAccountInstruction(cpu, 6u);
    const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 17u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x89u, 0xf0u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 19u, 2u, true);
    registers->pc = registers->z ? 0xee05u : 0xedffu;
    registers->cyclesUsed = registers->z ? 3u : 2u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 2 : 3;
    return 32u + registers->cyclesUsed;
}

static int FieldAttributeClass(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool high_class_bits = pc == 0x80edffu;
    if ((!high_class_bits && pc != 0x80ee05u) || opcode != (high_class_bits ? 0xa9u : 0xc9u) ||
        registers->e || !registers->mf || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (high_class_bits ? 8u : 17u))
        return 0;

    unsigned offset;
    const unsigned length = high_class_bits ? 6u : 18u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = high_class_bits ? "\x08\x85\x56\x80\x12" :
        "\x08\xf0\x0a\xc9\x09\xf0\x02\x80\x08\xa9\x80\x80\x02\xa9\x02\x85\x56";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    if (high_class_bits) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        Lufia2DecompApplyOperand(registers, 0xa9u, 8u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 2u, 2u, true);
        registers->write(registers->mem, 0x56u, 8u);
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 4u, 2u, true);
        registers->pc = 0xee17u;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = 1;
        return 8;
    }

    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompApplyOperand(registers, 0xc9u, 8u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    const bool class_eight = registers->z;
    unsigned cycles = class_eight ? 5u : 4u;
    Lufia2SpanAccountInstruction(cpu, class_eight ? 3u : 2u);

    if (!class_eight) {
        Lufia2SpanFetch(registers, pc + 4u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xc9u, 9u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, pc + 6u, 2u, true);
        Lufia2SpanAccountInstruction(cpu, registers->z ? 3u : 2u);

        if (!registers->z) {
            Lufia2SpanFetch(registers, pc + 8u, 2u, true);
            registers->pc = 0xee17u;
            registers->cyclesUsed = 3u;
            Lufia2SpanAccountInstruction(cpu, 3u);
            *tail_steps = -4;
            return 11;
        }
        cycles += 5u;
    }

    const uint8_t bits = class_eight ? 2u : 0x80u;
    Lufia2SpanFetch(registers, class_eight ? 0x80ee13u : 0x80ee0fu, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, bits, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    if (!class_eight) {
        Lufia2SpanFetch(registers, 0x80ee11u, 2u, true);
        Lufia2SpanAccountInstruction(cpu, 3u);
        cycles += 3u;
    }

    Lufia2SpanFetch(registers, 0x80ee15u, 2u, true);
    registers->write(registers->mem, 0x56u, bits);
    registers->pc = 0xee17u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = 0;
    return cycles + 5u;
}

static int FieldAttributeWrite(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                 uint64_t dynamic_epoch, uint64_t write_epoch) {
    const bool restore_cursor = pc == 0x80ee17u;
    if ((!restore_cursor && pc != 0x80ee18u) || opcode != (restore_cursor ? 0x7au : 0xbfu) || registers->e || !registers->mf || registers->xf ||
        registers->dp || registers->db != 0x7eu || registers->x == 0xffffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (restore_cursor ? 39u : 34u))
        return 0;

    if (restore_cursor && (!cpu->ram || registers->sp < 0x1f00u || registers->sp > 0x1ffdu))
        return 0;
    unsigned cursor = registers->y;
    if (restore_cursor)
        cursor = cpu->ram[registers->sp + 1u] | ((unsigned)cpu->ram[registers->sp + 2u] << 8);
    if (cursor >= 0xc000u)
        return 0;

    unsigned offset;
    const unsigned length = restore_cursor ? 21u : 20u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = restore_cursor ?
        "\xbf\x01\x00\x7f\x29\x0c\x0a\x0a\x05\x56\x99\x00\x40\xc8\xe8\xe8\xc6\x54\xd0\xbe" :
        "\x01\x00\x7f\x29\x0c\x0a\x0a\x05\x56\x99\x00\x40\xc8\xe8\xe8\xc6\x54\xd0\xbe";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    if (restore_cursor) {
        Lufia2SpanFetch(registers, pc++, 1u, false);
        uint16_t y = registers->read(registers->mem, ++registers->sp);
        y |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        Lufia2DecompApplyOperand(registers, 0xa0u, y, false);
        Lufia2SpanAccountInstruction(cpu, 5u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    Lufia2SpanFetch(registers, pc, 4u, restore_cursor);
    const uint8_t source = registers->read(registers->mem, 0x7f0001u + registers->x);
    Lufia2DecompApplyOperand(registers, 0xa9u, source, true);
    Lufia2SpanAccountInstruction(cpu, 5u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x0cu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    for (unsigned offset = 6u; offset < 8u; ++offset) {
        Lufia2SpanFetch(registers, pc + offset, 1u, true);
        const uint8_t attribute = (uint8_t)registers->a;
        registers->c = (attribute & 0x80u) != 0;
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(attribute << 1u), true);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    const uint8_t classification = registers->read(registers->mem, 0x56u);
    Lufia2DecompApplyOperand(registers, 0x09u, classification, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 10u, 3u, true);
    registers->write(registers->mem, 0x7e4000u + registers->y, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 6u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 13u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    for (unsigned offset = 14u; offset < 16u; ++offset) {
        Lufia2SpanFetch(registers, pc + offset, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 16u, 2u, true);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x54u, 0x55u, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 18u, 2u, true);
    registers->pc = registers->z ? 0xee2cu : 0xedeau;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    return (restore_cursor ? 36u : 31u) + registers->cyclesUsed;
}

static int FieldAttributeCount(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch) {
    if (pc != 0x80ee2cu || opcode != 0xc6u || registers->e || !registers->mf ||
        registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 8u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 4u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x55\xd0\xba", 3u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x55u, 0x56u, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    registers->pc = registers->z ? 0xee30u : 0xedeau;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    return 5u + registers->cyclesUsed;
}

static int Lufia2DecompRunFieldAttribute(Interp816 *registers, CpuState *cpu,
                                         uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                         uint64_t write_epoch, long room, long *tail_steps) {
    switch (pc) {
    case 0x80edeau:
        if (room >= 10)
            return FieldAttributeLookup(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x80edffu:
    case 0x80ee05u:
        if (room >= (pc == 0x80edffu ? 3 : 7))
            return FieldAttributeClass(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x80ee17u:
    case 0x80ee18u:
        if (room >= (pc == 0x80ee17u ? 12 : 11)) {
            *tail_steps = 1;
            return FieldAttributeWrite(registers, cpu, pc, opcode, dynamic_epoch, write_epoch);
        }
        break;
    case 0x80ee2cu:
        if (room >= 2) {
            *tail_steps = 1;
            return FieldAttributeCount(registers, cpu, pc, opcode, dynamic_epoch, write_epoch);
        }
        break;
    }
    return 0;
}
