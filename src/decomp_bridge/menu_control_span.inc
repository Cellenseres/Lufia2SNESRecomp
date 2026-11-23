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
                           uint64_t dynamic_epoch, uint64_t write_epoch, long room, long *tail_steps) {
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

    const bool can_return = room >= 3 && registers->sp >= 0x1f00u && registers->sp <= 0x1ffdu &&
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) >= 15u &&
        offset + 5u < cart->romSize && cart->rom[offset + 5u] == 0x60u;

    Lufia2SpanFetch(registers, pc, 3u, false);
    Lufia2DecompModifyWram(registers, 0xdeu, timer, timer + 1u, false);
    Lufia2SpanAccountInstruction(cpu, 7u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    registers->pc = registers->z ? 0x8b8du : 0x8b8cu;
    registers->cyclesUsed = registers->z ? 3u : 2u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = 1;
    if (registers->z || !can_return)
        return 7u + registers->cyclesUsed;

    SpriteSpanReturn(registers, cpu, 0x868b8cu);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 15;
}

static int RunMenuRawGlyphFlow(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool save_cursor = pc == 0x8088b9u;
    const bool raw_glyph = pc == 0x8088d5u;
    if ((!save_cursor && !raw_glyph && pc != 0x8088d7u) ||
        opcode != (save_cursor ? 0x8eu : raw_glyph ? 0x38u : 0xebu) ||
        registers->e || !registers->mf || (save_cursor && registers->xf) ||
        registers->sp < 0x1f02u || registers->sp > (save_cursor ? 0x1fffu : 0x1ffdu) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (raw_glyph ? 8u : 11u))
        return 0;

    unsigned offset;
    const unsigned length = save_cursor ? 6u : raw_glyph ? 2u : 3u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = save_cursor ? "\x75\x05\x20\xc8\x88" : raw_glyph ? "\x60" : "\x18\x60";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    if (save_cursor) {
        const uint32_t destination = ((uint32_t)registers->db << 16) | 0x0575u;
        if (!Lufia2DecompIsWramByte(destination))
            return 0;

        Lufia2SpanFetch(registers, pc, 3u, false);
        Lufia2DecompWriteData(registers, destination, destination + 1u, registers->x, true);
        Lufia2SpanAccountInstruction(cpu, 5u);
        const bool cursor_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
            write_epoch != g_interp_bridge_write_epoch;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 3u, 3u, true);
        registers->write(registers->mem, registers->sp--, 0x88u);
        registers->write(registers->mem, registers->sp--, 0xbeu);
        registers->pc = 0x88c8u;
        registers->cyclesUsed = 6u;
        Lufia2SpanAccountInstruction(cpu, 6u);
        const bool push_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
            write_epoch != g_interp_bridge_write_epoch;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        *tail_steps = push_progress ? 0 : cursor_progress ? 1 : -1;
        return 11;
    }

    if (!raw_glyph) {
        Lufia2SpanFetch(registers, pc, 1u, false);
        MenuGlyphExchangeBytes(registers);
        Lufia2SpanAccountInstruction(cpu, 3u);
    }

    Lufia2SpanFetch(registers, pc + !raw_glyph, 1u, !raw_glyph);
    registers->c = raw_glyph;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + length - 1u, 1u, true);
    uint16_t return_address = registers->read(registers->mem, ++registers->sp);
    return_address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->pc = (uint16_t)(return_address + 1u);
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : raw_glyph ? -1 : -2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return raw_glyph ? 8u : 11u;
}

static int Lufia2DecompRunMenuControl(Interp816 *registers, CpuState *cpu,
                                      uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                      uint64_t write_epoch, long room, long *tail_steps) {
    switch (pc) {
    case 0x8088b9u:
    case 0x8088d5u:
        if (room >= 2)
            return RunMenuRawGlyphFlow(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x8088bfu:
        if (room >= 6)
            return MenuStringControl(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x8088c8u:
    case 0x8088ceu:
    case 0x8088d7u:
        if (pc == 0x8088d7u && room >= 3) {
            const int cycles = RunMenuRawGlyphFlow(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
            if (cycles)
                return cycles;
        }
        if (room >= (pc == 0x8088c8u ? 3 : 2))
            return MenuStringRawCheck(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x868b87u:
        if (room >= 2) {
            return SpriteTimer(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, room, tail_steps);
        }
        break;
    }
    return 0;
}

static int Lufia2DecompRunMenuStringStart(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                               uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool save_bank = pc == 0x80888cu;
    if ((!save_bank && pc != 0x808897u) || opcode != (save_bank ? 0x8bu : 0xc2u) ||
        registers->e || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (save_bank ? 22u : 28u))
        return 0;

    if (save_bank ? (registers->sp < 0x1f02u || registers->sp > 0x1fffu) :
                    (registers->db != 0x7eu || registers->xf))
        return 0;

    unsigned offset;
    const unsigned length = save_bank ? 11u : 19u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = save_bank ? "\xe2\x20\xc2\x10\xa9\x7e\x48\xab\x84\x5d" :
        "\x20\x8a\x38\xe9\x40\x00\x8d\x73\x05\x8d\x75\x05\xaa\xe2\x20\xa0\x00\x00";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    if (save_bank) {
        Lufia2SpanFetch(registers, pc, 1u, false);
        registers->write(registers->mem, registers->sp--, registers->db);
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 1u, 2u, true);
        registers->mf = true;
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, pc + 3u, 2u, true);
        registers->xf = false;
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, pc + 5u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, 0x7eu, true);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 7u, 1u, true);
        registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 8u, 1u, true);
        registers->db = registers->read(registers->mem, ++registers->sp);
        registers->z = registers->db == 0;
        registers->n = (registers->db & 0x80u) != 0;
        Lufia2SpanAccountInstruction(cpu, 4u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 9u, 2u, true);
        Lufia2DecompWriteData(registers, 0x5du, 0x5eu, registers->y, true);
        registers->pc = 0x8897u;
        registers->cyclesUsed = 4u;
        Lufia2SpanAccountInstruction(cpu, 4u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        *tail_steps = 0;
        return 22;
    }

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->x, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->c = true;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 4u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x40u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    for (unsigned offset = 7u; offset < 13u; offset += 3u) {
        Lufia2SpanFetch(registers, pc + offset, 3u, true);
        const uint32_t destination = offset == 7u ? 0x7e0573u : 0x7e0575u;
        Lufia2DecompWriteData(registers, destination, destination + 1u, registers->a, true);
        Lufia2SpanAccountInstruction(cpu, 5u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    Lufia2SpanFetch(registers, pc + 13u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 14u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 16u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0u, false);
    registers->pc = 0x88aau;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = 3;
    return 28;
}

static int Lufia2DecompRunMenuStringFinish(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x8088e9u || opcode != 0xaeu || registers->e || registers->xf ||
        registers->db != 0x7eu || registers->sp < 0x1f00u || registers->sp >= 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 9u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 4u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x75\x05\xab", 3u))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint16_t cursor = Lufia2DecompReadData(registers, 0x7e0575u, 0x7e0576u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, cursor, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    const bool cursor_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0;
    registers->pc = 0x88edu;
    registers->cyclesUsed = 4u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : cursor_progress ? 1 : -1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 9;
}

static int Lufia2DecompRunMenuConditionalCommand(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                 uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x808bfdu || opcode != 0xb7u || registers->e || !registers->mf || registers->xf ||
        registers->dp || registers->db != 0x7eu || !cpu->ram ||
        registers->sp < 0x1f01u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 38u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 20u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x5d\xc8\x48\x29\xf8\x8d\x7b\x05\xa9\x00\xeb\x68\x29\x07\x0a\xaa\x7c\x11\x8c", 19u) ||
        !Lufia2DecompIsLoRomDataByte(cart, 0x808c20u))
        return 0;

    const uint32_t pointer = cpu->ram[0x5du] |
        ((uint32_t)cpu->ram[0x5eu] << 8) | ((uint32_t)cpu->ram[0x5fu] << 16);
    const uint32_t expected = (pointer + registers->y) & 0xffffffu;
    if (!Lufia2DecompIsWramByte(expected) && !Lufia2DecompIsLoRomDataByte(cart, expected))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    uint32_t source = Lufia2DecompReadData(registers, 0x5du, 0x5eu, true);
    source |= (uint32_t)registers->read(registers->mem, 0x5fu) << 16;
    const uint8_t command = registers->read(registers->mem, (source + registers->y) & 0xffffffu);
    Lufia2DecompApplyOperand(registers, 0xa9u, command, true);
    Lufia2SpanAccountInstruction(cpu, 6u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->write(registers->mem, registers->sp--, command);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0xf8u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 6u, 3u, true);
    registers->write(registers->mem, 0x7e057bu, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 11u, 1u, true);
    MenuGlyphExchangeBytes(registers);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 12u, 1u, true);
    const uint8_t saved_command = registers->read(registers->mem, ++registers->sp);
    Lufia2DecompApplyOperand(registers, 0xa9u, saved_command, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool stack_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 13u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 7u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 15u, 1u, true);
    const uint8_t index = (uint8_t)(registers->a << 1u);
    registers->c = false;
    Lufia2DecompApplyOperand(registers, 0xa9u, index, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 16u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 17u, 3u, true);
    const uint32_t entry = 0x808c11u + registers->x;
    registers->pc = Lufia2DecompReadData(registers, entry, entry + 1u, true);
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : stack_progress ? 4 : 7;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 38;
}

static int Lufia2DecompRunMenuGlyphCall(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                             uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x8088b4u || opcode != 0x20u || registers->e ||
        registers->sp < 0x1f03u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 12u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 3u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\xda\x88", 2u) ||
        offset + 0x29u > cart->romSize || memcmp(cart->rom + offset + 0x26u, "\x20\xb3\x8d", 3u))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    registers->write(registers->mem, registers->sp--, 0x88u);
    registers->write(registers->mem, registers->sp--, 0xb6u);
    Lufia2SpanAccountInstruction(cpu, 6u);
    const bool outer_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, 0x8088dau, 3u, true);
    registers->write(registers->mem, registers->sp--, 0x88u);
    registers->write(registers->mem, registers->sp--, 0xdcu);
    registers->pc = 0x8db3u;
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    const bool inner_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = inner_progress ? 0 : outer_progress ? 1 : -1;
    return 12;
}
