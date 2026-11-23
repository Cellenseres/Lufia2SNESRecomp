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

static int Lufia2DecompRunBattleVelocityEntry(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                    uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x85dd63u || opcode != 0x5au || registers->e || !registers->mf || registers->xf ||
        registers->dp || registers->sp < 0x1f02u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 12u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 7u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\xa5\x54\x89\x3f\xd0\x46", 6u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->write(registers->mem, registers->sp--, (uint8_t)(registers->y >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->y);
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 2u, true);
    const uint8_t angle = registers->read(registers->mem, 0x54u);
    Lufia2DecompApplyOperand(registers, 0xa9u, angle, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x3fu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    registers->pc = registers->z ? 0xdd6au : 0xddb0u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 4 - progress : -3;
    return 9u + registers->cyclesUsed;
}

static int Lufia2DecompRunBattleVelocityAxisBranch(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                         long *tail_steps) {
    if (pc != 0x85dd6au || opcode != 0x29u || registers->e || !registers->mf ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 17u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 14u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\xc0\xf0\x36\x38\xe9\x40\xf0\x25\x38\xe9\x40\xf0\x10", 13u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompApplyOperand(registers, 0x29u, 0xc0u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    if (registers->z) {
        registers->pc = 0xdda4u;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = -1;
        return 5;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    for (unsigned axis = 0; axis < 2u; ++axis) {
        const uint32_t instruction = pc + 4u + axis * 5u;
        Lufia2SpanFetch(registers, instruction, 1u, true);
        registers->c = true;
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, instruction + 1u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xe9u, 0x40u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, instruction + 3u, 2u, true);
        if (registers->z) {
            registers->pc = axis ? 0xdd88u : 0xdd98u;
            registers->cyclesUsed = 3u;
            Lufia2SpanAccountInstruction(cpu, 3u);
            *tail_steps = -4 - (long)axis * 3L;
            return 11u + axis * 6u;
        }
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    registers->pc = 0xdd78u;
    registers->cyclesUsed = 2u;
    *tail_steps = -7;
    return 16;
}

static int Lufia2DecompRunBattleVelocityAxisStore(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                        uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool negative = pc == 0x85dd78u || pc == 0x85dd88u;
    const bool first_axis = pc == 0x85dd78u || pc == 0x85dd98u;
    const bool store = negative || first_axis || pc == 0x85dda4u;
    const bool restore_y = pc == 0x85de1cu;
    const bool returning = pc == 0x85dd87u || pc == 0x85dd97u || pc == 0x85dda3u ||
        pc == 0x85ddafu || pc == 0x85de1du;
    const unsigned cycles = store ? (negative ? 28u : 23u) : restore_y ? 5u : 6u;
    if ((!store && !restore_y && !returning) || opcode != (store ? 0xc2u : restore_y ? 0x7au : 0x6bu) ||
        registers->e || (store && (!registers->mf || registers->dp)) ||
        ((store || restore_y) && registers->xf) ||
        registers->sp < 0x1f00u || registers->sp > (returning ? 0x1ffcu : 0x1ffdu) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < cycles)
        return 0;

    unsigned offset;
    const unsigned length = store ? (negative ? 15u : 11u) : 1u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    if (!cart)
        return 0;

    if (!store) {
        Lufia2SpanFetch(registers, pc, 1u, false);
        uint16_t value = registers->read(registers->mem, ++registers->sp);
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        if (restore_y) {
            Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
            registers->pc = 0xde1du;
        } else {
            registers->k = registers->read(registers->mem, ++registers->sp);
            registers->pc = (uint16_t)(value + 1u);
        }
        registers->cyclesUsed = cycles;
        Lufia2SpanAccountInstruction(cpu, cycles);
        *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : -1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        return cycles;
    }

    const uint8_t result = first_axis ? 0x56u : 0x58u;
    const uint8_t cleared = first_axis ? 0x58u : 0x56u;
    const uint8_t suffix[] = {0x85, result, 0x64, cleared, 0xe2, 0x20, 0x7a};
    const uint8_t *body = cart->rom + offset;
    if (memcmp(body + 1u, "\x20\xa5\x5a", 3u) ||
        (negative && memcmp(body + 4u, "\x49\xff\xff\x1a", 4u)) ||
        memcmp(body + (negative ? 8u : 4u), suffix, sizeof(suffix)))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    const uint16_t speed = Lufia2DecompReadData(registers, 0x5au, 0x5bu, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, speed, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 2 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    uint32_t instruction = pc + 4u;
    if (negative) {
        Lufia2SpanFetch(registers, instruction, 3u, true);
        Lufia2DecompApplyOperand(registers, 0x49u, 0xffffu, false);
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanFetch(registers, instruction + 3u, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
        instruction += 4u;
    }

    Lufia2SpanFetch(registers, instruction, 2u, true);
    Lufia2DecompWriteData(registers, result, result + 1u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = negative ? 5 : 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, instruction + 2u, 2u, true);
    Lufia2DecompWriteData(registers, cleared, cleared + 1u, 0u, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = negative ? 6 : 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, instruction + 4u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, instruction + 6u, 1u, true);
    uint16_t saved_y = registers->read(registers->mem, ++registers->sp);
    saved_y |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, saved_y, false);
    registers->pc = (uint16_t)(pc + length);
    registers->cyclesUsed = 5u;
    Lufia2SpanAccountInstruction(cpu, 5u);
    const long steps = negative ? 8 : 6;
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = steps;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? steps - progress : 1 - steps;
    return cycles;
}

static int Lufia2DecompRunBattleVelocityFactor(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                     uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool first_axis = pc == 0x85ddb4u;
    if ((!first_axis && pc != 0x85ddeau) || opcode != 0xa9u || registers->e ||
        !registers->mf || registers->xf || registers->dp || registers->waiting ||
        registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 18u)
        return 0;

    unsigned offset;
    const uint8_t destination = first_axis ? 0x56u : 0x58u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 12u, &offset);
    const uint8_t signature[] = {0x01, 0x24, 0x64, 0xf0, 0x06, 0xa6, 0x5a, 0x86, destination, 0x80, 0x26};
    if (!cart || memcmp(cart->rom + offset + 1u, signature, sizeof(signature)))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompApplyOperand(registers, 0xa9u, 1u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    const uint8_t factor_flags = registers->read(registers->mem, 0x64u);
    registers->z = (factor_flags & 1u) == 0u;
    registers->n = (factor_flags & 0x80u) != 0u;
    registers->v = (factor_flags & 0x40u) != 0u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 2 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    if (registers->z) {
        registers->pc = (uint16_t)(pc + 12u);
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 1 : -2;
        return 8;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    const uint16_t speed = Lufia2DecompReadData(registers, 0x5au, 0x5bu, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, speed, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    Lufia2DecompWriteData(registers, destination, destination + 1u, speed, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    registers->pc = (uint16_t)(pc + 0x32u);
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 6 - progress : -5;
    return 18;
}

static int Lufia2DecompRunBattleVelocityMultiplySetup(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                            uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if ((pc != 0x85ddc0u && pc != 0x85ddf6u) || opcode != 0xa5u || registers->e ||
        !registers->mf || registers->xf || registers->dp ||
        registers->sp < 0x1f02u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 24u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 11u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x63\x85\x50\xa6\x5a\x86\x4e\x8b\x4b\xab", 10u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    const uint8_t factor = registers->read(registers->mem, 0x63u);
    Lufia2DecompApplyOperand(registers, 0xa9u, factor, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    registers->write(registers->mem, 0x50u, factor);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    const uint16_t speed = Lufia2DecompReadData(registers, 0x5au, 0x5bu, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, speed, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    Lufia2DecompWriteData(registers, 0x4eu, 0x4fu, speed, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    for (unsigned bank = 0; bank < 2u; ++bank) {
        Lufia2SpanFetch(registers, pc + 8u + bank, 1u, true);
        registers->write(registers->mem, registers->sp--, bank ? registers->k : registers->db);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = bank + 5u;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    Lufia2SpanFetch(registers, pc + 10u, 1u, true);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    registers->pc = (uint16_t)(pc + 11u);
    registers->cyclesUsed = 4u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 7;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 7 - progress : -6;
    return 24;
}

static int Lufia2DecompRunBattleVelocityMultiplyResult(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                             uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool first_axis = pc == 0x85ddcfu;
    if ((!first_axis && pc != 0x85de05u) || opcode != 0xabu || registers->e ||
        !registers->mf || registers->xf || registers->dp ||
        registers->sp < 0x1f00u || registers->sp > 0x1ffeu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 31u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 23u, &offset);
    const uint8_t destination = first_axis ? 0x56u : 0x58u;
    const uint8_t signature[] = {
        0xa5, 0x64, 0x10, 0x0e, 0xc2, 0x20, 0xa5, 0x52, 0x49, 0xff, 0xff, 0x1a,
        0x85, destination, 0xe2, 0x20, 0x80, 0x04, 0xa6, 0x52, 0x86, destination,
    };
    if (!cart || memcmp(cart->rom + offset + 1u, signature, sizeof(signature)))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 2u, true);
    const uint8_t flags = registers->read(registers->mem, 0x64u);
    Lufia2DecompApplyOperand(registers, 0xa9u, flags, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    const bool negative = registers->n;
    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    Lufia2SpanAccountInstruction(cpu, negative ? 2u : 3u);
    if (negative) {
        Lufia2SpanFetch(registers, pc + 5u, 2u, true);
        registers->mf = false;
        Lufia2SpanAccountInstruction(cpu, 3u);
    }

    const uint32_t load = pc + (negative ? 7u : 19u);
    Lufia2SpanFetch(registers, load, 2u, true);
    const uint16_t product = Lufia2DecompReadData(registers, 0x52u, 0x53u, true);
    Lufia2DecompApplyOperand(registers, negative ? 0xa9u : 0xa2u, product, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = negative ? 5 : 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    if (negative) {
        Lufia2SpanFetch(registers, load + 2u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0x49u, 0xffffu, false);
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanFetch(registers, load + 5u, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    const uint32_t store = load + (negative ? 6u : 2u);
    Lufia2SpanFetch(registers, store, 2u, true);
    Lufia2DecompWriteData(registers, destination, destination + 1u, negative ? registers->a : product, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = negative ? 8 : 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    if (negative) {
        Lufia2SpanFetch(registers, store + 2u, 2u, true);
        registers->mf = true;
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanFetch(registers, store + 4u, 2u, true);
        Lufia2SpanAccountInstruction(cpu, 3u);
    }
    registers->pc = (uint16_t)(pc + 23u);
    registers->cyclesUsed = negative ? 3u : 4u;
    const long steps = negative ? 10 : 5;
    *tail_steps = progress ? steps - progress : 1 - steps;
    return negative ? 31 : 18;
}
