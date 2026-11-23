static int Lufia2DecompRunBattleSineEntry(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool cosine = pc == 0x85de1eu;
    if ((!cosine && pc != 0x85de2au) || opcode != 0x08u || registers->e ||
        registers->dp || registers->sp < 0x1f01u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (cosine ? 19u : 12u))
        return 0;

    unsigned offset;
    const unsigned length = cosine ? 12u : 7u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = cosine ? "\xe2\x30\xa5\x54\x18\x69\x40\x85\x55\x80\x07" :
        "\xe2\x30\xa5\x54\x85\x55";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->write(registers->mem, registers->sp--, interp816_getFlags(registers));
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 2u, true);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x30u);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    const uint8_t angle = registers->read(registers->mem, 0x54u);
    Lufia2DecompApplyOperand(registers, 0xa9u, angle, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    uint32_t store = pc + 5u;
    if (cosine) {
        Lufia2SpanFetch(registers, store++, 1u, true);
        registers->c = false;
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, store, 2u, true);
        Lufia2DecompApplyOperand(registers, 0x69u, 64u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
        store += 2u;
    }
    Lufia2SpanFetch(registers, store, 2u, true);
    registers->write(registers->mem, 0x55u, (uint8_t)registers->a);
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = cosine ? 6 : 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    if (cosine) {
        Lufia2SpanFetch(registers, store + 2u, 2u, true);
        Lufia2SpanAccountInstruction(cpu, 3u);
    }

    registers->pc = 0xde31u;
    const long steps = cosine ? 7 : 4;
    *tail_steps = progress ? steps - progress : 1 - steps;
    return cosine ? 19 : 12;
}

static int Lufia2DecompRunBattleSineQuadrant(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x85de31u || opcode != 0xc9u || registers->e || !registers->mf || !registers->xf ||
        registers->dp || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 23u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 36u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x40\xb0\x04\x0a\xaa\x80\x29\xc9\x80\xb0\x09\xa9\x80\x38\xe5\x55\x0a"
        "\xaa\x80\x1c\xc9\xc0\xb0\x06\x29\x3f\x0a\xaa\x80\x05\x49\xff\x1a\x0a\xaa", 35u))
        return 0;

    static const unsigned compare_offsets[] = {0u, 8u, 21u};
    static const unsigned shift_offsets[] = {4u, 17u, 27u, 34u};
    const unsigned quadrant = (uint8_t)registers->a >> 6;
    unsigned cycles = 0;
    for (unsigned boundary = 0; boundary < 3u; ++boundary) {
        const uint32_t compare = pc + compare_offsets[boundary];
        Lufia2SpanFetch(registers, compare, 2u, boundary != 0u);
        Lufia2DecompApplyOperand(registers, 0xc9u, (boundary + 1u) * 64u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, compare + 2u, 2u, true);
        Lufia2SpanAccountInstruction(cpu, registers->c ? 3u : 2u);
        cycles += registers->c ? 5u : 4u;
        if (!registers->c)
            break;
    }

    bool angle_progress = false;
    switch (quadrant) {
    case 1:
        Lufia2SpanFetch(registers, pc + 12u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, 0x80u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, pc + 14u, 1u, true);
        registers->c = true;
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, pc + 15u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xe9u, registers->read(registers->mem, 0x55u), true);
        Lufia2SpanAccountInstruction(cpu, 3u);
        angle_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        cycles += 7u;
        break;
    case 2:
        Lufia2SpanFetch(registers, pc + 25u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0x29u, 0x3fu, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
        cycles += 2u;
        break;
    case 3:
        Lufia2SpanFetch(registers, pc + 31u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0x49u, 0xffu, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, pc + 33u, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
        Lufia2SpanAccountInstruction(cpu, 2u);
        cycles += 4u;
        break;
    }

    const uint32_t shift = pc + shift_offsets[quadrant];
    Lufia2SpanFetch(registers, shift, 1u, true);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a << 1), true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, shift + 1u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint8_t)registers->a, true);
    registers->cyclesUsed = 2u;
    Lufia2SpanAccountInstruction(cpu, 2u);
    cycles += 4u;

    if (quadrant != 3u) {
        Lufia2SpanFetch(registers, shift + 2u, 2u, true);
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        cycles += 3u;
    }
    registers->pc = quadrant < 2u ? 0xde62u : 0xde55u;
    *tail_steps = angle_progress ? 3 : quadrant == 0u ? -4 : -9;
    return cycles;
}

static int Lufia2DecompRunBattleSineTable(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool negative = pc == 0x85de55u;
    const bool lookup = negative || pc == 0x85de62u;
    if ((!lookup && pc != 0x85de6bu && pc != 0x85de61u) || opcode != (lookup ? 0xc2u : 0x6bu) || registers->e ||
        (lookup && (!registers->mf || !registers->xf || registers->dp || registers->x > 0x80u)) ||
        registers->sp < 0x1f00u || registers->sp > (lookup ? 0x1ffeu : 0x1ffcu) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (lookup ? (negative ? 20u : 17u) : 6u))
        return 0;

    unsigned offset;
    const unsigned length = lookup ? (negative ? 12u : 9u) : 1u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    if (!cart)
        return 0;

    if (!lookup) {
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

    const uint32_t address = 0x97b226u + registers->x;
    const char *signature = negative ? "\x20\xbf\x26\xb2\x97\x09\x00\x80\x85\x63\x28" :
        "\x20\xbf\x26\xb2\x97\x85\x63\x28";
    if (memcmp(cart->rom + offset + 1u, signature, length - 1u) ||
        !Lufia2DecompIsLoRomDataByte(cart, address + 1u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 2u, 4u, true);
    const uint16_t value = Lufia2DecompReadData(registers, address, address + 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, value, false);
    Lufia2SpanAccountInstruction(cpu, 6u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 2 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    uint32_t store = pc + 6u;
    if (negative) {
        Lufia2SpanFetch(registers, store, 3u, true);
        Lufia2DecompApplyOperand(registers, 0x09u, 0x8000u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);
        store += 3u;
    }
    Lufia2SpanFetch(registers, store, 2u, true);
    Lufia2DecompWriteData(registers, 0x63u, 0x64u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = negative ? 4 : 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, store + 2u, 1u, true);
    interp816_setFlags(registers, registers->read(registers->mem, ++registers->sp));
    registers->pc = (uint16_t)(pc + length);
    registers->cyclesUsed = 4u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    const long steps = negative ? 5 : 4;
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = steps;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? steps - progress : 1 - steps;
    return negative ? 20 : 17;
}
