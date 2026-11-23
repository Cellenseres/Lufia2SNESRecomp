static int Lufia2DecompRunSpritePieceOffsets(Interp816 *registers, CpuState *cpu,
                                           uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch) {
    if (pc != 0x868c3bu || opcode != 0xc2u || registers->e || !registers->mf ||
        registers->xf || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || !cpu->ram ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 21u)
        return 0;

    unsigned rom_offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 10u, &rom_offset);
    if (!cart || memcmp(cart->rom + rom_offset + 1u, "\x20\xb7\x5d\x85\x58\xc8\xc8\xe2\x20", 9u))
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
    const uint16_t offsets = Lufia2DecompReadData(registers, low, (low + 1u) & 0xffffffu, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, offsets, false);
    Lufia2SpanAccountInstruction(cpu, 7u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        bridge_apu_flush(cpu);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    Lufia2DecompWriteData(registers, 0x58u, 0x59u, offsets, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    bridge_apu_flush(cpu);

    for (unsigned offset = 6u; offset < 8u; ++offset) {
        Lufia2SpanFetch(registers, pc + offset, 1u, true);
        ++registers->y;
        registers->z = registers->y == 0;
        registers->n = (registers->y & 0x8000u) != 0;
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    registers->mf = true;
    registers->cyclesUsed = 3u;
    registers->pc = (uint16_t)(pc + 10u);
    Lufia2SpanAccountInstruction(cpu, 3u);
    return 21;
}
