static int SpritePieceEdge(Interp816 *registers, CpuState *cpu,
                              uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                              uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x868c84u || opcode != 0xa5u || registers->e || registers->mf ||
        registers->xf || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 24u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 16u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x54\x29\x0f\x00\x1a\x85\x65\xe2\x20\xa5\x59\x29\x80\xd0\x10", 15u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    const uint16_t index = Lufia2DecompReadData(registers, 0x54u, 0x55u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, index, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x0fu, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 5u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    Lufia2DecompWriteData(registers, 0x65u, 0x66u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    const uint8_t horizontal_offset = registers->read(registers->mem, 0x59u);
    Lufia2DecompApplyOperand(registers, 0xa9u, horizontal_offset, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 12u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x80u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 14u, 2u, true);
    registers->pc = registers->z ? 0x8c94u : 0x8ca4u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 2 : 4;
    return 21u + registers->cyclesUsed;
}

static int SpritePieceCarry(Interp816 *registers, CpuState *cpu,
                               uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                               uint64_t write_epoch, long *tail_steps) {
    const bool negative_offset = pc == 0x868ca4u;
    if ((!negative_offset && pc != 0x868c94u) ||
        opcode != (negative_offset ? 0xa5u : 0xbdu) || registers->e || !registers->mf ||
        registers->xf || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || registers->x >= 48u ||
        !Lufia2DecompIsWramByte(((uint32_t)registers->db << 16) | 0x13e7u) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 23u)
        return 0;

    unsigned offset;
    const unsigned length = negative_offset ? 17u : 16u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = negative_offset ?
        "\x59\x49\xff\x1a\x18\x7d\x88\x13\xbd\xb8\x13\x69\x00\x18\x69\xff" :
        "\x88\x13\x38\xe5\x59\xbd\xb8\x13\xe9\x00\x18\x69\xff\x80\x11";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    const uint64_t initial_epoch = dynamic_epoch;
    unsigned last_progress = 0;
    const uint32_t bank = (uint32_t)registers->db << 16;
    const uint32_t x_low = bank | (0x1388u + registers->x);
    const uint32_t x_high = bank | (0x13b8u + registers->x);
    if (negative_offset) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        const uint8_t delta = registers->read(registers->mem, 0x59u);
        Lufia2DecompApplyOperand(registers, 0xa9u, delta, true);
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        if (dynamic_epoch != initial_epoch)
            last_progress = 1u;

        Lufia2SpanFetch(registers, pc + 2u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0x49u, 0xffu, true);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 4u, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 5u, 1u, true);
        registers->c = false;
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 6u, 3u, true);
        const uint8_t coordinate = registers->read(registers->mem, x_low);
        Lufia2DecompApplyOperand(registers, 0x69u, coordinate, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            last_progress = 5u;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        pc += 9u;
    } else {
        Lufia2SpanFetch(registers, pc, 3u, false);
        const uint8_t coordinate = registers->read(registers->mem, x_low);
        Lufia2DecompApplyOperand(registers, 0xa9u, coordinate, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        if (dynamic_epoch != initial_epoch)
            last_progress = 1u;

        Lufia2SpanFetch(registers, pc + 3u, 1u, true);
        registers->c = true;
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 4u, 2u, true);
        const uint8_t delta = registers->read(registers->mem, 0x59u);
        Lufia2DecompApplyOperand(registers, 0xe9u, delta, true);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            last_progress = 3u;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        pc += 6u;
    }

    Lufia2SpanFetch(registers, pc, 3u, true);
    const uint8_t coordinate_high = registers->read(registers->mem, x_high);
    Lufia2DecompApplyOperand(registers, 0xa9u, coordinate_high, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        last_progress = negative_offset ? 6u : 4u;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    Lufia2DecompApplyOperand(registers, negative_offset ? 0x69u : 0xe9u, 0u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 5u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 0xffu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    if (!negative_offset) {
        Lufia2SpanFetch(registers, pc + 8u, 2u, true);
        Lufia2SpanAccountInstruction(cpu, 3u);
    }
    registers->pc = 0x8cb5u;
    registers->cyclesUsed = negative_offset ? 2u : 3u;
    const unsigned instructions = negative_offset ? 9u : 8u;
    *tail_steps = last_progress ? (long)(instructions - last_progress) : 1 - (long)instructions;
    return negative_offset ? 23 : 22;
}

static int SpritePiecePack(Interp816 *registers, CpuState *cpu,
                              uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                              uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x868cb5u || opcode != 0xc2u || registers->e || !registers->mf ||
        registers->xf || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 16u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 10u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x20\x2a\x29\x01\x00\x05\x56\x18\x6a", 9u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    const uint16_t rotated = (uint16_t)((registers->a << 1u) | registers->c);
    registers->c = (registers->a & 0x8000u) != 0;
    Lufia2DecompApplyOperand(registers, 0xa9u, rotated, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 3u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 1u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    const uint16_t size = Lufia2DecompReadData(registers, 0x56u, 0x57u, true);
    Lufia2DecompApplyOperand(registers, 0x09u, size, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 8u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 9u, 1u, true);
    registers->c = (registers->a & 1u) != 0;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a >> 1u), false);
    registers->pc = (uint16_t)(pc + 10u);
    registers->cyclesUsed = 2u;
    Lufia2SpanAccountInstruction(cpu, 2u);
    *tail_steps = progress ? 2 : -5;
    return 16;
}

static int SpritePieceCommit(Interp816 *registers, CpuState *cpu,
                                uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x868cc4u || opcode != 0x05u || registers->e || registers->mf ||
        registers->xf || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || registers->y > 0x2feu ||
        !Lufia2DecompIsWramByte(((uint32_t)registers->db << 16) | 0x3ffu) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 25u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 9u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x63\x99\x00\x01\xe6\x54\xe6\x54", 8u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    const uint16_t previous_bits = Lufia2DecompReadData(registers, 0x63u, 0x64u, true);
    Lufia2DecompApplyOperand(registers, 0x09u, previous_bits, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 3u, true);
    const uint32_t oam = ((uint32_t)registers->db << 16) | (0x100u + registers->y);
    Lufia2DecompWriteData(registers, oam, oam + 1u, registers->a, true);
    const unsigned store_cycles = 7u;
    Lufia2SpanAccountInstruction(cpu, store_cycles);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    for (unsigned offset = 5u; offset < 9u; offset += 2u) {
        Lufia2SpanFetch(registers, pc + offset, 2u, true);
        Lufia2DecompModifyWram(registers, 0xe6u, 0x54u, 0x55u, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    registers->pc = (uint16_t)(pc + 9u);
    registers->cyclesUsed = 7u;
    *tail_steps = 0;
    return 18u + store_cycles;
}

static int SpritePieceNext(Interp816 *registers, CpuState *cpu,
                              uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                              uint64_t write_epoch, long room, long *tail_steps) {
    if (pc != 0x868ccdu || opcode != 0xe2u || registers->e || registers->mf ||
        registers->xf || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || !cpu->ram ||
        registers->sp < 0x1f00u || registers->sp > 0x1ffdu ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 21u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 12u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x20\x7a\xb7\x5d\xc9\xff\xf0\x03\x4c\x3b\x8c", 11u))
        return 0;

    const bool can_return = room >= 6 && registers->sp <= 0x1ffbu &&
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) >= 25u &&
        offset + 12u < cart->romSize && cart->rom[offset + 12u] == 0x60u;

    const uint32_t pointer = cpu->ram[0x5du] |
        ((uint32_t)cpu->ram[0x5eu] << 8) | ((uint32_t)cpu->ram[0x5fu] << 16);
    const unsigned stacked_index = cpu->ram[registers->sp + 1u] |
        ((unsigned)cpu->ram[registers->sp + 2u] << 8);
    const uint32_t expected = (pointer + stacked_index) & 0xffffffu;
    if (!Lufia2DecompIsWramByte(expected) && !Lufia2DecompIsLoRomDataByte(cart, expected))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    uint16_t index = registers->read(registers->mem, ++registers->sp);
    index |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, index, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    const bool stack_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    uint32_t source = Lufia2DecompReadData(registers, 0x5du, 0x5eu, true);
    source |= (uint32_t)registers->read(registers->mem, 0x5fu) << 16;
    const uint8_t next_byte = registers->read(registers->mem, (source + index) & 0xffffffu);
    Lufia2DecompApplyOperand(registers, 0xa9u, next_byte, true);
    Lufia2SpanAccountInstruction(cpu, 6u);
    const bool data_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0xffu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    const bool done = registers->z;
    Lufia2SpanAccountInstruction(cpu, done ? 3u : 2u);
    if (!done) {
        Lufia2SpanFetch(registers, pc + 9u, 3u, true);
        Lufia2SpanAccountInstruction(cpu, 3u);
    }
    registers->pc = done ? 0x8cd9u : 0x8c3bu;
    registers->cyclesUsed = 3u;
    if (data_progress)
        *tail_steps = done ? 2 : 3;
    else if (stack_progress)
        *tail_steps = done ? 3 : 4;
    else
        *tail_steps = done ? -4 : -5;
    if (!done || !can_return)
        return done ? 19 : 21;

    Lufia2SpanNearReturn(registers, cpu, 0x868cd9u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        *tail_steps = 0;
    else
        *tail_steps += *tail_steps < 0 ? -1 : 1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 25;
}

static int Lufia2DecompRunSpritePieceFinish(Interp816 *registers, CpuState *cpu,
                                           uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                           uint64_t write_epoch, long room, long *tail_steps) {
    switch (pc) {
    case 0x868c84u:
        if (room >= 8)
            return SpritePieceEdge(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x868c94u:
    case 0x868ca4u:
        if (room >= (pc == 0x868c94u ? 8 : 9))
            return SpritePieceCarry(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x868cb5u:
        if (room >= 6)
            return SpritePiecePack(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x868cc4u:
        if (room >= 4)
            return SpritePieceCommit(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x868ccdu:
        if (room >= 6)
            return SpritePieceNext(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, room, tail_steps);
        break;
    }
    return 0;
}
