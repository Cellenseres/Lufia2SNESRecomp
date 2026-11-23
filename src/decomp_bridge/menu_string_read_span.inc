static int Lufia2DecompRunMenuStringRead(Interp816 *registers, CpuState *cpu,
                              uint32_t pc, uint8_t opcode,
                              uint64_t dynamic_epoch, bool *progress) {
    if (pc != 0x8088adu || opcode != 0xb7u || registers->e || !registers->mf ||
        registers->xf || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || !cpu->ram ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 13u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 7u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x5d\xc8\xc9\x10\x90\x05", 6u))
        return 0;

    const uint32_t pointer = cpu->ram[0x5du] |
        ((uint32_t)cpu->ram[0x5eu] << 8) | ((uint32_t)cpu->ram[0x5fu] << 16);
    const uint32_t expected = (pointer + registers->y) & 0xffffffu;
    if (!Lufia2DecompIsWramByte(expected) && !Lufia2DecompIsLoRomDataByte(cart, expected))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    uint32_t source = Lufia2DecompReadData(registers, 0x5du, 0x5eu, true);
    source |= (uint32_t)registers->read(registers->mem, 0x5fu) << 16;
    const uint8_t character = registers->read(registers->mem, (source + registers->y) & 0xffffffu);
    Lufia2DecompApplyOperand(registers, 0xa9u, character, true);
    Lufia2SpanAccountInstruction(cpu, 6u);
    *progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    if (*progress)
        bridge_apu_flush(cpu);

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    ++registers->y;
    registers->z = registers->y == 0;
    registers->n = (registers->y & 0x8000u) != 0;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x10u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    registers->cyclesUsed = registers->c ? 2u : 3u;
    registers->pc = registers->c ? 0x88b4u : 0x88b9u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    return 10u + registers->cyclesUsed;
}
