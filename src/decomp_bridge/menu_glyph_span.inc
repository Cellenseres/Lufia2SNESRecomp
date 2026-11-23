static void MenuGlyphExchangeBytes(Interp816 *registers) {
    registers->a = (uint16_t)((registers->a << 8) | (registers->a >> 8));
    const uint8_t low = (uint8_t)registers->a;
    registers->z = low == 0;
    registers->n = (low & 0x80u) != 0;
}

static int Lufia2DecompRunMenuGlyph(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool write_tile = pc == 0x808dbeu;
    if ((!write_tile && pc != 0x808db3u) || opcode != (write_tile ? 0x9du : 0xebu) ||
        registers->e || !registers->mf || registers->xf || registers->dp ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        !Lufia2DecompIsWramByte(((uint32_t)registers->db << 16) | 0x0578u) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 20u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 11u, &offset);
    const char *signature = write_tile ? "\x40\x00\xad\x64\x05\x9d\x41\x00\xe8\xe8" :
                                        "\xad\x78\x05\xd0\x11\xeb\xc9\xcc\xb0\x12";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, 10u))
        return 0;

    const uint32_t bank = (uint32_t)registers->db << 16;
    if (write_tile) {
        const uint32_t tile = bank + 0x0040u + registers->x;
        if (!Lufia2DecompIsWramByte(tile) || !Lufia2DecompIsWramByte(tile + 1u))
            return 0;

        Lufia2SpanFetch(registers, pc, 3u, false);
        registers->write(registers->mem, tile, (uint8_t)registers->a);
        Lufia2SpanAccountInstruction(cpu, 6u);
        SpritePositionProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 3u, 3u, true);
        const uint8_t attribute = registers->read(registers->mem, bank | 0x0564u);
        Lufia2DecompApplyOperand(registers, 0xa9u, attribute, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        SpritePositionProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 6u, 3u, true);
        registers->write(registers->mem, tile + 1u, attribute);
        Lufia2SpanAccountInstruction(cpu, 6u);
        SpritePositionProgress(cpu, &dynamic_epoch, &write_epoch);

        for (unsigned offset = 9u; offset < 11u; ++offset) {
            Lufia2SpanFetch(registers, pc + offset, 1u, true);
            Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
            Lufia2SpanAccountInstruction(cpu, 2u);
        }

        registers->pc = 0x8dc9u;
        registers->cyclesUsed = 2u;
        *tail_steps = 2;
        return 20;
    }

    Lufia2SpanFetch(registers, pc, 1u, false);
    MenuGlyphExchangeBytes(registers);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 1u, 3u, true);
    const uint8_t raw = registers->read(registers->mem, bank | 0x0578u);
    Lufia2DecompApplyOperand(registers, 0xa9u, raw, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    SpritePositionProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    Lufia2SpanAccountInstruction(cpu, raw ? 3u : 2u);
    if (raw) {
        registers->pc = 0x8dcau;
        registers->cyclesUsed = 3u;
        *tail_steps = progress ? 1 : -2;
        return 10;
    }

    Lufia2SpanFetch(registers, pc + 6u, 1u, true);
    MenuGlyphExchangeBytes(registers);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0xccu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 9u, 2u, true);
    registers->pc = registers->c ? 0x8dd0u : 0x8dbeu;
    registers->cyclesUsed = registers->c ? 3u : 2u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 4 : -5;
    return 14u + registers->cyclesUsed;
}
