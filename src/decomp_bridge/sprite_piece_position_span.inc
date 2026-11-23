static void SpritePositionProgress(CpuState *cpu, uint64_t *dynamic_epoch,
                                   uint64_t *write_epoch) {
    if (*dynamic_epoch != s_interp_dynamic_progress_epoch ||
        *write_epoch != g_interp_bridge_write_epoch) {
        *dynamic_epoch = s_interp_dynamic_progress_epoch;
        *write_epoch = g_interp_bridge_write_epoch;
        bridge_apu_flush(cpu);
    }
}

static int Lufia2DecompRunSpritePiecePosition(Interp816 *registers, CpuState *cpu,
                                            uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                            uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x868c45u || opcode != 0x5au || registers->e || !registers->mf ||
        registers->xf || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || !cpu->ram ||
        registers->sp < 0x1f02u || registers->sp > 0x1fffu || registers->x >= 48u ||
        !Lufia2DecompIsWramByte(((uint32_t)registers->db << 16) | 0x1417u) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 54u)
        return 0;

    const unsigned oam_index = cpu->ram[0x5au] | ((unsigned)cpu->ram[0x5bu] << 8);
    if (oam_index > 0x1feu)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 27u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\xa4\x5a\xbd\x88\x13\x38\xe5\x59\x99\x00\x01\xc8\xea"
        "\xbd\xe8\x13\x38\xe5\x58\x99\x00\x01\xc8\x84\x5a\x7a", 26u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->write(registers->mem, registers->sp--, (uint8_t)(registers->y >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->y);
    Lufia2SpanAccountInstruction(cpu, 4u);
    SpritePositionProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 2u, true);
    const uint16_t destination = Lufia2DecompReadData(registers, 0x5au, 0x5bu, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, destination, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    SpritePositionProgress(cpu, &dynamic_epoch, &write_epoch);

    unsigned cycles = 8u;
    const uint32_t bank = (uint32_t)registers->db << 16;
    uint32_t instruction = pc + 3u;
    for (unsigned axis = 0; axis < 2u; ++axis) {
        const unsigned actor_base = axis ? 0x13e8u : 0x1388u;
        const unsigned coordinate = actor_base + registers->x;
        const unsigned load_cycles = 4u + ((coordinate >> 8) != (actor_base >> 8));
        Lufia2SpanFetch(registers, instruction, 3u, true);
        const uint8_t actor_position = registers->read(registers->mem, bank | coordinate);
        Lufia2DecompApplyOperand(registers, 0xa9u, actor_position, true);
        Lufia2SpanAccountInstruction(cpu, load_cycles);
        SpritePositionProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, instruction + 3u, 1u, true);
        registers->c = true;
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, instruction + 4u, 2u, true);
        const uint8_t piece_offset = registers->read(registers->mem, 0x59u - axis);
        Lufia2DecompApplyOperand(registers, 0xe9u, piece_offset, true);
        Lufia2SpanAccountInstruction(cpu, 3u);
        SpritePositionProgress(cpu, &dynamic_epoch, &write_epoch);

        const unsigned store_cycles = 6u;
        Lufia2SpanFetch(registers, instruction + 6u, 3u, true);
        registers->write(registers->mem, bank | (0x100u + registers->y), (uint8_t)registers->a);
        Lufia2SpanAccountInstruction(cpu, store_cycles);
        SpritePositionProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, instruction + 9u, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
        cycles += load_cycles + store_cycles + 7u;
        if (!axis) {
            Lufia2SpanFetch(registers, instruction + 10u, 1u, true);
            Lufia2SpanAccountInstruction(cpu, 2u);
            cycles += 2u;
            instruction += 11u;
        }
    }

    Lufia2SpanFetch(registers, pc + 24u, 2u, true);
    Lufia2DecompWriteData(registers, 0x5au, 0x5bu, registers->y, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    SpritePositionProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 26u, 1u, true);
    uint16_t source_index = registers->read(registers->mem, ++registers->sp);
    source_index |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, source_index, false);
    registers->pc = (uint16_t)(pc + 27u);
    registers->cyclesUsed = 5u;
    Lufia2SpanAccountInstruction(cpu, 5u);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : 1;
    SpritePositionProgress(cpu, &dynamic_epoch, &write_epoch);
    return cycles + 9u;
}
