static int Lufia2DecompRunSpritePieceTile(Interp816 *registers, CpuState *cpu,
                                        uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                        uint64_t write_epoch) {
    if (pc != 0x868c60u || opcode != 0xc2u || registers->e || !registers->mf ||
        registers->xf || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || !cpu->ram ||
        registers->sp < 0x1f02u || registers->sp > 0x1fffu ||
        !Lufia2DecompIsWramByte(((uint32_t)registers->db << 16) | 0x300u) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 37u)
        return 0;

    const unsigned oam_index = cpu->ram[0x5au] | ((unsigned)cpu->ram[0x5bu] << 8);
    if (oam_index > 0x1feu)
        return 0;

    unsigned rom_offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 16u, &rom_offset);
    if (!cart || memcmp(cart->rom + rom_offset + 1u,
        "\x20\xb7\x5d\xc8\xc8\x5a\xa4\x5a\x99\x00\x01\xc8\xc8\x84\x5a", 15u))
        return 0;

    const uint32_t pointer = cpu->ram[0x5du] |
        ((uint32_t)cpu->ram[0x5eu] << 8) | ((uint32_t)cpu->ram[0x5fu] << 16);
    const uint32_t expected_low = (pointer + registers->y) & 0xffffffu;
    const uint32_t expected_high = (expected_low + 1u) & 0xffffffu;
    if ((!Lufia2DecompIsWramByte(expected_low) && !Lufia2DecompIsLoRomDataByte(cart, expected_low)) ||
        (!Lufia2DecompIsWramByte(expected_high) && !Lufia2DecompIsLoRomDataByte(cart, expected_high)))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    uint32_t source = Lufia2DecompReadData(registers, 0x5du, 0x5eu, true);
    source |= (uint32_t)registers->read(registers->mem, 0x5fu) << 16;
    const uint32_t low = (source + registers->y) & 0xffffffu;
    const uint16_t tile_attributes = Lufia2DecompReadData(registers, low, (low + 1u) & 0xffffffu, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, tile_attributes, false);
    Lufia2SpanAccountInstruction(cpu, 7u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    for (unsigned offset = 4u; offset < 6u; ++offset) {
        Lufia2SpanFetch(registers, pc + offset, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 6u, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)(registers->y >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->y);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    const uint16_t destination = Lufia2DecompReadData(registers, 0x5au, 0x5bu, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, destination, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 3u, true);
    const uint32_t oam = ((uint32_t)registers->db << 16) | (0x100u + destination);
    Lufia2DecompWriteData(registers, oam, oam + 1u, tile_attributes, true);
    const unsigned store_cycles = 7u;
    Lufia2SpanAccountInstruction(cpu, store_cycles);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    for (unsigned offset = 12u; offset < 14u; ++offset) {
        Lufia2SpanFetch(registers, pc + offset, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 14u, 2u, true);
    Lufia2DecompWriteData(registers, 0x5au, 0x5bu, registers->y, true);
    registers->pc = (uint16_t)(pc + 16u);
    registers->cyclesUsed = 4u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 30u + store_cycles;
}

static int Lufia2DecompRunSpritePieceHighWord(Interp816 *registers, CpuState *cpu,
                                            uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                            uint64_t write_epoch) {
    if (pc != 0x868c70u || opcode != 0xc2u || registers->e ||
        registers->xf || registers->dp || registers->d || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || !cpu->ram ||
        !Lufia2DecompIsWramByte(((uint32_t)registers->db << 16) | 0x3ffu) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 33u)
        return 0;

    const unsigned piece_index = cpu->ram[0x54u] | ((unsigned)cpu->ram[0x55u] << 8);
    if (piece_index >= 0x800u)
        return 0;

    unsigned rom_offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 20u, &rom_offset);
    if (!cart || memcmp(cart->rom + rom_offset + 1u,
        "\x20\xa5\x54\x4a\x4a\x4a\x29\xfe\x00\x18\x69\x00\x02\xa8\xb9\x00\x01\x85\x63", 19u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    pc += 2u;

    Lufia2SpanFetch(registers, pc, 2u, true);
    const uint16_t index = Lufia2DecompReadData(registers, 0x54u, 0x55u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, index, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    for (unsigned offset = 2u; offset < 5u; ++offset) {
        Lufia2SpanFetch(registers, pc + offset, 1u, true);
        registers->c = (registers->a & 1u) != 0;
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a >> 1), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 5u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0xfeu, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 8u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 9u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x200u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 12u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 13u, 3u, true);
    const uint32_t oam = ((uint32_t)registers->db << 16) | (0x100u + registers->y);
    const uint16_t high_word = Lufia2DecompReadData(registers, oam, oam + 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, high_word, false);
    Lufia2SpanAccountInstruction(cpu, 6u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 16u, 2u, true);
    Lufia2DecompWriteData(registers, 0x63u, 0x64u, high_word, true);
    registers->pc = (uint16_t)(pc + 18u);
    registers->cyclesUsed = 4u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 33;
}
