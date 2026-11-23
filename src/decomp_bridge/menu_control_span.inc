static int MenuStringControl(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x8088bfu || opcode != 0xebu || registers->e || !registers->mf || registers->xf ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 18u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 9u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\xa9\x00\xeb\x0a\xaa\x7c\x7c\x8e", 8u) ||
        !Lufia2DecompIsLoRomDataByte(cart, 0x808f7bu))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    MenuGlyphExchangeBytes(registers);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 1u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    MenuGlyphExchangeBytes(registers);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 4u, 1u, true);
    const uint8_t command = (uint8_t)registers->a;
    registers->c = (command & 0x80u) != 0;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(command << 1), true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 5u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 6u, 3u, true);
    const uint32_t entry = 0x808e7cu + registers->x;
    registers->pc = Lufia2DecompReadData(registers, entry, entry + 1u, true);
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : -5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 18;
}

static int MenuStringRawCheck(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                 uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool no_raw_glyph = pc == 0x8088d7u;
    const bool restore_cursor = pc == 0x8088ceu;
    if ((!no_raw_glyph && !restore_cursor && pc != 0x8088c8u) || opcode != 0xebu || registers->e || !registers->mf ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (no_raw_glyph ? 5u : restore_cursor ? 8u : 10u))
        return 0;

    if (restore_cursor && registers->xf)
        return 0;

    const uint32_t flag = ((uint32_t)registers->db << 16) | 0x0578u;
    if (!no_raw_glyph && !Lufia2DecompIsWramByte(flag))
        return 0;

    unsigned offset;
    const unsigned length = no_raw_glyph ? 2u : restore_cursor ? 4u : 6u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = no_raw_glyph ? "\x18" : restore_cursor ? "\xae\x75\x05" : "\xad\x78\x05\xf0\x09";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    MenuGlyphExchangeBytes(registers);
    Lufia2SpanAccountInstruction(cpu, 3u);

    if (restore_cursor) {
        Lufia2SpanFetch(registers, pc + 1u, 3u, true);
        const uint32_t cursor = flag - 3u;
        const uint16_t x = Lufia2DecompReadData(registers, cursor, cursor + 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, x, false);
        registers->pc = 0x88d2u;
        registers->cyclesUsed = 5u;
        Lufia2SpanAccountInstruction(cpu, 5u);
        *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : -1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        return 8;
    }

    if (no_raw_glyph) {
        Lufia2SpanFetch(registers, pc + 1u, 1u, true);
        registers->c = false;
        registers->pc = 0x88d9u;
        registers->cyclesUsed = 2u;
        Lufia2SpanAccountInstruction(cpu, 2u);
        *tail_steps = -1;
        return 5;
    }

    Lufia2SpanFetch(registers, pc + 1u, 3u, true);
    const uint8_t pending = registers->read(registers->mem, flag);
    Lufia2DecompApplyOperand(registers, 0xa9u, pending, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    registers->pc = pending ? 0x88ceu : 0x88d7u;
    registers->cyclesUsed = pending ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 1 : -2;
    return 7u + registers->cyclesUsed;
}

static int SpriteTimer(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                           uint64_t dynamic_epoch, uint64_t write_epoch) {
    const uint32_t timer = ((uint32_t)registers->db << 16) | (0x1478u + registers->x);
    if (pc != 0x868b87u || opcode != 0xdeu || registers->e || !registers->mf ||
        !registers->xf || registers->x >= 48u || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || !Lufia2DecompIsWramByte(timer) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 10u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 5u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x78\x14\xf0\x01", 4u))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    Lufia2DecompModifyWram(registers, 0xdeu, timer, timer + 1u, false);
    Lufia2SpanAccountInstruction(cpu, 7u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    registers->pc = registers->z ? 0x8b8du : 0x8b8cu;
    registers->cyclesUsed = registers->z ? 3u : 2u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    return 7u + registers->cyclesUsed;
}

static int Lufia2DecompRunMenuControl(Interp816 *registers, CpuState *cpu,
                                      uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                      uint64_t write_epoch, long room, long *tail_steps) {
    switch (pc) {
    case 0x8088bfu:
        if (room >= 6)
            return MenuStringControl(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x8088c8u:
    case 0x8088ceu:
    case 0x8088d7u:
        if (room >= (pc == 0x8088c8u ? 3 : 2))
            return MenuStringRawCheck(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x868b87u:
        if (room >= 2) {
            *tail_steps = 1;
            return SpriteTimer(registers, cpu, pc, opcode, dynamic_epoch, write_epoch);
        }
        break;
    }
    return 0;
}
