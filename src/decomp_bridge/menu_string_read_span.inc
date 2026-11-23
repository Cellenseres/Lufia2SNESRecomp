static int Lufia2DecompRunMenuStringRead(Interp816 *registers, CpuState *cpu,
                              uint32_t pc, uint8_t opcode,
                              uint64_t dynamic_epoch, long *tail_steps) {
    const bool reload_cursor = pc == 0x8088aau;
    if ((!reload_cursor && pc != 0x8088adu) || opcode != (reload_cursor ? 0xaeu : 0xb7u) ||
        (reload_cursor && registers->db != 0x7eu) || registers->e || !registers->mf ||
        registers->xf || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || !cpu->ram ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (reload_cursor ? 18u : 13u))
        return 0;

    unsigned offset;
    const unsigned length = reload_cursor ? 10u : 7u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = reload_cursor ? "\x75\x05\xb7\x5d\xc8\xc9\x10\x90\x05" :
                                           "\x5d\xc8\xc9\x10\x90\x05";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    const uint32_t pointer = cpu->ram[0x5du] |
        ((uint32_t)cpu->ram[0x5eu] << 8) | ((uint32_t)cpu->ram[0x5fu] << 16);
    const uint32_t expected = (pointer + registers->y) & 0xffffffu;
    if (!Lufia2DecompIsWramByte(expected) && !Lufia2DecompIsLoRomDataByte(cart, expected))
        return 0;

    bool cursor_progress = false;
    if (reload_cursor) {
        Lufia2SpanFetch(registers, pc, 3u, false);
        const uint16_t cursor = Lufia2DecompReadData(registers, 0x7e0575u, 0x7e0576u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, cursor, false);
        Lufia2SpanAccountInstruction(cpu, 5u);
        cursor_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
        if (cursor_progress) {
            dynamic_epoch = s_interp_dynamic_progress_epoch;
            bridge_apu_flush(cpu);
        }
        pc += 3u;
    }
    Lufia2SpanFetch(registers, pc, 2u, reload_cursor);
    uint32_t source = Lufia2DecompReadData(registers, 0x5du, 0x5eu, true);
    source |= (uint32_t)registers->read(registers->mem, 0x5fu) << 16;
    const uint8_t character = registers->read(registers->mem, (source + registers->y) & 0xffffffu);
    Lufia2DecompApplyOperand(registers, 0xa9u, character, true);
    Lufia2SpanAccountInstruction(cpu, 6u);
    const bool read_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    if (read_progress)
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
    *tail_steps = read_progress ? 3 : cursor_progress ? 4 : reload_cursor ? -4 : -3;
    return (reload_cursor ? 15u : 10u) + registers->cyclesUsed;
}
