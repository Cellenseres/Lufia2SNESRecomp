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

    Lufia2SpanNearReturn(registers, cpu, 0x868b8cu);
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

static int RunNestedMenuStringContext(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                      uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x808bcau || opcode != 0xe2u || registers->e || registers->dp ||
        registers->sp < 0x1f05u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 36u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 18u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x20\xa5\x5f\x48\xa5\x62\x85\x5f\xc2\x30\xa5\x5d\x48\x5a\xa5\x60\xa8", 17u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    const uint8_t prior_bank = registers->read(registers->mem, 0x5fu);
    Lufia2DecompApplyOperand(registers, 0xa9u, prior_bank, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 2 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 1u, true);
    registers->write(registers->mem, registers->sp--, prior_bank);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    const uint8_t nested_bank = registers->read(registers->mem, 0x62u);
    Lufia2DecompApplyOperand(registers, 0xa9u, nested_bank, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    registers->write(registers->mem, 0x5fu, nested_bank);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 2u, true);
    registers->mf = registers->xf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 11u, 2u, true);
    const uint16_t prior_offset = Lufia2DecompReadData(registers, 0x5du, 0x5eu, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, prior_offset, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 7;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    for (unsigned push = 0; push < 2u; ++push) {
        Lufia2SpanFetch(registers, pc + 13u + push, 1u, true);
        const uint16_t value = push ? registers->y : prior_offset;
        registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
        registers->write(registers->mem, registers->sp--, (uint8_t)value);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 8 + push;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    Lufia2SpanFetch(registers, pc + 15u, 2u, true);
    const uint16_t nested_offset = Lufia2DecompReadData(registers, 0x60u, 0x61u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, nested_offset, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 10;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 17u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, nested_offset, false);
    registers->pc = 0x8bdcu;
    registers->cyclesUsed = 2u;
    Lufia2SpanAccountInstruction(cpu, 2u);
    *tail_steps = progress ? 11 - progress : -10;
    return 36;
}

static int RunNestedMenuStringCall(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x808bdcu || opcode != 0xadu || registers->e || registers->mf || registers->xf ||
        !Lufia2DecompIsWramByte(bank | 0x0575u) || registers->sp < 0x1f04u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 30u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 17u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x75\x05\x18\x69\x40\x00\xaa\xad\x73\x05\x48\xe2\x20\x20\x8c\x88", 16u))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint16_t current_cursor = Lufia2DecompReadData(registers, bank | 0x0575u, bank | 0x0576u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, current_cursor, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 4u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0040u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 7u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 8u, 3u, true);
    const uint16_t row = Lufia2DecompReadData(registers, bank | 0x0573u, bank | 0x0574u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, row, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 11u, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)(row >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)row);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 12u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 14u, 3u, true);
    registers->write(registers->mem, registers->sp--, 0x8bu);
    registers->write(registers->mem, registers->sp--, 0xecu);
    registers->pc = 0x888cu;
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 8 - progress : -7;
    return 30;
}

static int RunNestedMenuStringReturn(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                     uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x808bedu || opcode != 0xc2u || registers->e || registers->dp ||
        !Lufia2DecompIsWramByte(bank | 0x0573u) || registers->sp < 0x1f00u || registers->sp > 0x1ff6u ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 43u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 16u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x30\x68\x8d\x73\x05\x7a\x68\x85\x5d\xe2\x20\x68\x85\x5f\x60", 15u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = registers->xf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    uint32_t instruction = pc + 2u;
    long executed = 1;
    long progress = 0;
    for (unsigned word = 0; word < 3u; ++word) {
        Lufia2SpanFetch(registers, instruction++, 1u, true);
        uint16_t value = registers->read(registers->mem, ++registers->sp);
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        Lufia2DecompApplyOperand(registers, word == 1u ? 0xa0u : 0xa9u, value, false);
        Lufia2SpanAccountInstruction(cpu, 5u);
        ++executed;
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = executed;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        if (word != 1u) {
            const uint32_t destination = word ? 0x5du : bank | 0x0573u;
            const unsigned length = word ? 2u : 3u;
            Lufia2SpanFetch(registers, instruction, length, true);
            Lufia2DecompWriteData(registers, destination, destination + 1u, value, true);
            Lufia2SpanAccountInstruction(cpu, word ? 4u : 5u);
            ++executed;
            if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
                progress = executed;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
            instruction += length;
        }
    }

    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 12u, 1u, true);
    const uint8_t prior_bank = registers->read(registers->mem, ++registers->sp);
    Lufia2DecompApplyOperand(registers, 0xa9u, prior_bank, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 13u, 2u, true);
    registers->write(registers->mem, 0x5fu, prior_bank);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 9;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanNearReturn(registers, cpu, 0x808bfcu);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 10;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 10 - progress : -9;
    return 43;
}

static int Lufia2DecompRunMenuControl(Interp816 *registers, CpuState *cpu,
                                      uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                      uint64_t write_epoch, long room, long *tail_steps) {
    switch (pc) {
    case 0x808bcau:
        if (room >= 11)
            return RunNestedMenuStringContext(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x808bdcu:
        if (room >= 8)
            return RunNestedMenuStringCall(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x808bedu:
        if (room >= 10)
            return RunNestedMenuStringReturn(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
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
                                uint64_t dynamic_epoch, uint64_t write_epoch, long room, long *tail_steps) {
    if (pc != 0x8088e9u || opcode != 0xaeu || registers->e || registers->xf ||
        registers->db != 0x7eu || registers->sp < 0x1f00u || registers->sp >= 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 9u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 4u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x75\x05\xab", 3u))
        return 0;

    const bool can_return = room >= 3 && registers->sp <= 0x1ffcu &&
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) >= 15u &&
        offset + 4u < cart->romSize && cart->rom[offset + 4u] == 0x60u;

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
    if (!can_return)
        return 9;

    Lufia2SpanNearReturn(registers, cpu, 0x8088edu);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        *tail_steps = 0;
    else
        *tail_steps += *tail_steps < 0 ? -1 : 1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 15;
}

static int Lufia2DecompRunMenuConditionalCommand(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                 uint64_t dynamic_epoch, uint64_t write_epoch, long room, long *tail_steps) {
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
    const uint16_t call_site = registers->pc;
    if (room < 13 || registers->sp < 0x1f02u ||
        (call_site != 0x8c19u && call_site != 0x8c3eu && call_site != 0x8c4eu) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 6u)
        return 38;

    const unsigned call_offset = offset + call_site - (uint16_t)pc;
    if (call_offset + 3u > cart->romSize || memcmp(cart->rom + call_offset, "\x20\x9e\x8c", 3u))
        return 38;

    Lufia2SpanFetch(registers, 0x800000u | call_site, 3u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)((call_site + 2u) >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)(call_site + 2u));
    registers->pc = 0x8c9eu;
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        *tail_steps = 0;
    else
        ++*tail_steps;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 44;
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

static int RunMenuTextRow(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                             uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool advance = pc == 0x808df9u;
    const uint32_t bank = (uint32_t)registers->db << 16;
    if ((!advance && pc != 0x808e0fu) || opcode != (advance ? 0xc2u : 0xadu) ||
        registers->e || (!advance && !registers->mf) || (advance && (registers->xf || registers->d)) ||
        !Lufia2DecompIsWramByte(bank | 0x0578u) ||
        registers->sp < 0x1f00u || registers->sp > 0x1ffdu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (advance ? 38u : 14u))
        return 0;

    unsigned offset;
    const unsigned length = advance ? 22u : 7u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = advance ? "\x20\xad\x73\x05\x18\x69\x40\x00\x8d\x73\x05\x8d\x75\x05\xaa\xe2\x20\x9c\x78\x05\x60" :
        "\x65\x05\x8d\x66\x05\x60";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    long progress = 0;
    if (advance) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        registers->mf = false;
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, pc + 2u, 3u, true);
        const uint16_t row = Lufia2DecompReadData(registers, bank | 0x0573u, bank | 0x0574u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, row, false);
        Lufia2SpanAccountInstruction(cpu, 5u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 2;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 5u, 1u, true);
        registers->c = false;
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 6u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0x69u, 64u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);

        for (unsigned entry = 0; entry < 2; ++entry) {
            Lufia2SpanFetch(registers, pc + 9u + 3u * entry, 3u, true);
            const uint32_t destination = bank | (0x0573u + 2u * entry);
            Lufia2DecompWriteData(registers, destination, destination + 1u, registers->a, true);
            Lufia2SpanAccountInstruction(cpu, 5u);
            if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
                progress = 5u + entry;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        }

        Lufia2SpanFetch(registers, pc + 15u, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 16u, 2u, true);
        registers->mf = true;
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, pc + 18u, 3u, true);
        registers->write(registers->mem, bank | 0x0578u, 0u);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 9;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    } else {
        Lufia2SpanFetch(registers, pc, 3u, false);
        const uint8_t width = registers->read(registers->mem, bank | 0x0565u);
        Lufia2DecompApplyOperand(registers, 0xa9u, width, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 3u, 3u, true);
        registers->write(registers->mem, bank | 0x0566u, width);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 2;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    const long executed = advance ? 10 : 3;
    Lufia2SpanNearReturn(registers, cpu, pc + length - 1u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = executed;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? executed - progress : 1 - executed;
    return advance ? 38 : 14;
}

static int RunMenuCursorBlink(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                 uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x82c627u || opcode != 0xadu || registers->e || !registers->mf ||
        registers->sp < 0x1f00u || registers->sp > 0x1ffdu ||
        !Lufia2DecompIsWramByte(bank | 0x09c8u) || !Lufia2DecompIsWramByte(bank | 0x1554u) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 46u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 32u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\xc8\x09\xf0\x1a\xad\x53\x15\xf0\x15\xee\x54\x15\xad\x54\x15\xc9"
        "\x20\xd0\x0b\x9c\x54\x15\xad\x52\x15\x49\x01\x8d\x52\x15\x60", 31u))
        return 0;

    long executed = 0;
    long progress = 0;
    unsigned cycles = 0;
    const uint32_t checks[] = {0x09c8u, 0x1553u};
    for (unsigned test = 0; test < 2; ++test) {
        const uint32_t site = pc + 5u * test;
        Lufia2SpanFetch(registers, site, 3u, test != 0u);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, bank | checks[test]), true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = executed + 1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, site + 3u, 2u, true);
        Lufia2SpanAccountInstruction(cpu, registers->z ? 3u : 2u);
        cycles += registers->z ? 7u : 6u;
        executed += 2;
        if (registers->z)
            break;
    }

    if (!registers->z) {
        const uint32_t timer = bank | 0x1554u;
        Lufia2SpanFetch(registers, pc + 10u, 3u, true);
        Lufia2DecompModifyWram(registers, 0xeeu, timer, timer + 1u, false);
        Lufia2SpanAccountInstruction(cpu, 6u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 5;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 13u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, timer), true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 6;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 16u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xc9u, 0x20u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 18u, 2u, true);
        Lufia2SpanAccountInstruction(cpu, registers->z ? 2u : 3u);
        cycles += registers->z ? 14u : 15u;
        executed += 4;
        if (registers->z) {
            Lufia2SpanFetch(registers, pc + 20u, 3u, true);
            registers->write(registers->mem, timer, 0u);
            Lufia2SpanAccountInstruction(cpu, 4u);
            if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
                progress = 9;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

            const uint32_t visible = bank | 0x1552u;
            Lufia2SpanFetch(registers, pc + 23u, 3u, true);
            Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, visible), true);
            Lufia2SpanAccountInstruction(cpu, 4u);
            if (dynamic_epoch != s_interp_dynamic_progress_epoch)
                progress = 10;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

            Lufia2SpanFetch(registers, pc + 26u, 2u, true);
            Lufia2DecompApplyOperand(registers, 0x49u, 1u, true);
            Lufia2SpanAccountInstruction(cpu, 2u);

            Lufia2SpanFetch(registers, pc + 28u, 3u, true);
            registers->write(registers->mem, visible, (uint8_t)registers->a);
            Lufia2SpanAccountInstruction(cpu, 4u);
            if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
                progress = 12;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
            cycles += 14u;
            executed += 4;
        }
    }

    Lufia2SpanNearReturn(registers, cpu, 0x82c646u);
    ++executed;
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = executed;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? executed - progress : 1 - executed;
    return cycles + 6u;
}

static int Lufia2DecompRunMenuPresentationBoundary(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                                   uint64_t dynamic_epoch, uint64_t write_epoch, long room,
                                                   long *tail_steps) {
    if (pc == 0x82c627u) {
        if (room < 13)
            return 0;
        return RunMenuCursorBlink(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
    }
    if (room < (pc == 0x808df9u ? 10 : 3))
        return 0;
    return RunMenuTextRow(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
}

static int Lufia2DecompRunMenuNewlineDispatch(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool jump = pc == 0x808d0au;
    const bool new_row = pc == 0x808d04u;
    const unsigned cycles = jump ? 4u : 6u;
    if ((!jump && !new_row && pc != 0x808d07u) || opcode != (jump ? 0x82u : 0x20u) ||
        registers->e || (!jump && (registers->sp < 0x1f02u || registers->sp > 0x1fffu)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < cycles)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 3u, &offset);
    const char *operand = jump ? "\xa0\xfb" : new_row ? "\xf9\x8d" : "\x0f\x8e";
    if (!cart || memcmp(cart->rom + offset + 1u, operand, 2u))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    if (!jump) {
        const uint16_t return_address = (uint16_t)(pc + 2u);
        registers->write(registers->mem, registers->sp--, (uint8_t)(return_address >> 8));
        registers->write(registers->mem, registers->sp--, (uint8_t)return_address);
    }
    registers->pc = jump ? 0x88adu : new_row ? 0x8df9u : 0x8e0fu;
    registers->cyclesUsed = cycles;
    Lufia2SpanAccountInstruction(cpu, cycles);
    *tail_steps = (dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch) ? 0 : -1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return (int)cycles;
}

static int Lufia2DecompRunMenuTileIteration(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                               uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool front_layer = pc == 0x82813eu || pc == 0x828142u;
    const uint32_t word_pc = front_layer ? 0x82813eu : 0x828408u;
    const unsigned word_length = front_layer ? 4u : 3u;
    const bool store_tile = pc == word_pc;
    const unsigned store_cycles = !store_tile ? 0u : front_layer ? 6u : 7u;
    const uint32_t destination = (front_layer ? 0x7e2000u : 0x7e3000u) + registers->x;
    const uint8_t store_opcode = front_layer ? 0x9fu : 0x9eu;
    if ((!store_tile && pc != word_pc + word_length) || opcode != (store_tile ? store_opcode : 0xe8u) ||
        registers->e || registers->xf ||
        (store_tile && (registers->mf || (!front_layer && registers->db != 0x7eu) || destination >= 0x7effffu)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < store_cycles + 9u)
        return 0;

    static const char *const signatures[2][2] = {
        {"\xe8\x88\xd0\xf8", "\x00\x30\xe8\xe8\x88\xd0\xf8"},
        {"\xe8\x88\xd0\xf7", "\x00\x20\x7e\xe8\xe8\x88\xd0\xf7"},
    };
    unsigned offset;
    const unsigned length = (store_tile ? word_length : 0u) + 5u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, signatures[front_layer][store_tile], length - 1u))
        return 0;

    long progress = 0;
    if (store_tile) {
        Lufia2SpanFetch(registers, pc, word_length, false);
        Lufia2DecompWriteData(registers, destination, destination + 1u, front_layer ? registers->a : 0u, true);
        Lufia2SpanAccountInstruction(cpu, store_cycles);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        pc += word_length;
    }

    for (unsigned increment = 0; increment < 2; ++increment) {
        Lufia2SpanFetch(registers, pc + increment, 1u, store_tile || increment != 0u);
        ++registers->x;
        registers->n = (registers->x & 0x8000u) != 0;
        registers->z = registers->x == 0;
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    --registers->y;
    registers->n = (registers->y & 0x8000u) != 0;
    registers->z = registers->y == 0;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    registers->pc = (uint16_t)(registers->z ? pc + 5u : word_pc);
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    const long executed = store_tile ? 5 : 4;
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = executed;
    *tail_steps = progress ? executed - progress : 1 - executed;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return store_cycles + 6u + registers->cyclesUsed;
}

static int Lufia2DecompRunMenuTileRow(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                           uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool front_layer = pc == 0x828135u || pc == 0x828147u;
    const bool begin = pc == 0x828404u || pc == 0x828135u;
    const uint32_t position = front_layer ? 0x63u : 0x54u;
    const uint32_t rows = front_layer ? 0x5au : 0x58u;
    if ((!begin && pc != 0x828410u && pc != 0x828147u) || opcode != (begin ? 0xa4u : 0xa5u) ||
        registers->e || registers->mf || registers->xf || registers->dp || registers->d ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (begin ? (front_layer ? 15u : 8u) : 23u))
        return 0;

    unsigned offset;
    const unsigned length = begin ? (front_layer ? 9u : 4u) : 12u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    static const char *const signatures[2][2] = {
        {"\x54\x18\x69\x40\x00\x85\x54\xc6\x58\xd0\xe8", "\x56\xa6\x54"},
        {"\x63\x18\x69\x40\x00\x85\x63\xc6\x5a\xd0\xe2", "\x56\x88\x88\xa9\x0f\x08\xa6\x63"},
    };
    if (!cart || memcmp(cart->rom + offset + 1u, signatures[front_layer][begin], length - 1u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    const uint16_t first = begin ? Lufia2DecompReadData(registers, 0x56u, 0x57u, true) :
        Lufia2DecompReadData(registers, position, position + 1u, true);
    Lufia2DecompApplyOperand(registers, begin ? 0xa0u : 0xa9u, first, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    if (begin) {
        if (front_layer) {
            for (unsigned index = 0; index < 2u; ++index) {
                Lufia2SpanFetch(registers, pc + 2u + index, 1u, true);
                --registers->y;
                registers->n = (registers->y & 0x8000u) != 0u;
                registers->z = registers->y == 0u;
                Lufia2SpanAccountInstruction(cpu, 2u);
            }
            Lufia2SpanFetch(registers, pc + 4u, 3u, true);
            Lufia2DecompApplyOperand(registers, 0xa9u, 0x080fu, false);
            Lufia2SpanAccountInstruction(cpu, 3u);
            pc += 5u;
        }
        Lufia2SpanFetch(registers, pc + 2u, 2u, true);
        const uint16_t column = Lufia2DecompReadData(registers, position, position + 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, column, false);
        registers->pc = (uint16_t)(pc + 4u);
        registers->cyclesUsed = 4u;
        Lufia2SpanAccountInstruction(cpu, 4u);
        const long executed = front_layer ? 5 : 2;
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = executed;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        *tail_steps = progress ? executed - progress : 1 - executed;
        return front_layer ? 15 : 8;
    }

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 3u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 64u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    Lufia2DecompWriteData(registers, position, position + 1u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    Lufia2DecompModifyWram(registers, 0xc6u, rows, rows + 1u, true);
    Lufia2SpanAccountInstruction(cpu, 7u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    registers->pc = (uint16_t)(registers->z ? pc + 12u : front_layer ? 0x8135u : 0x8404u);
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 6 - progress : -5;
    return 20u + registers->cyclesUsed;
}

static int RunMenuClearRectangleReturn(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                       uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (opcode != 0xabu || registers->sp < 0x1f00u || registers->sp > 0x1ffcu ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 10u)
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 2u, &offset);
    if (!cart || cart->rom[offset + 1u] != 0x60u)
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->n = (registers->db & 0x80u) != 0u;
    registers->z = registers->db == 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool bank_read_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanNearReturn(registers, cpu, pc + 1u);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : bank_read_progress ? 1 : -1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 10;
}

static int Lufia2DecompRunMenuClearRectangleFrame(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                      uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (registers->e || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted)
        return 0;
    if (pc == 0x82841cu)
        return RunMenuClearRectangleReturn(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
    if (pc != 0x8283ebu || opcode != 0x85u || registers->mf || registers->xf || registers->dp ||
        registers->sp < 0x1f02u || registers->sp > 0x1ffdu ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 47u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 25u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x54\x64\x56\x64\x58\x8a\xe2\x20\x85\x58\xeb"
        "\x85\x56\xc2\x20\x8b\xe2\x20\xa9\x7e\x48\xab\xc2\x20", 24u))
        return 0;

    long progress = 0;
    for (unsigned word = 0; word < 3u; ++word) {
        const uint32_t address = 0x54u + 2u * word;
        Lufia2SpanFetch(registers, pc + 2u * word, 2u, word != 0u);
        Lufia2DecompWriteData(registers, address, address + 1u, word ? 0u : registers->a, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = word + 1u;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }
    Lufia2SpanFetch(registers, 0x8283f1u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->x, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, 0x8283f2u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, 0x8283f4u, 2u, true);
    registers->write(registers->mem, 0x58u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, 0x8283f6u, 1u, true);
    MenuGlyphExchangeBytes(registers);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, 0x8283f7u, 2u, true);
    registers->write(registers->mem, 0x56u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, 0x8283f9u, 2u, true);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, 0x8283fbu, 1u, true);
    registers->write(registers->mem, registers->sp--, registers->db);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 10;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, 0x8283fcu, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, 0x8283feu, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x7eu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, 0x828400u, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 13;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, 0x828401u, 1u, true);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->n = (registers->db & 0x80u) != 0u;
    registers->z = registers->db == 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 14;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, 0x828402u, 2u, true);
    registers->mf = false;
    registers->pc = 0x8404u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 15 - progress : -14;
    return 47;
}

static int Lufia2DecompRunMenuStringCallFrame(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool enter = pc == 0x808878u;
    const uint32_t bank = (uint32_t)registers->db << 16;
    if ((!enter && pc != 0x808889u) || opcode != (enter ? 0x08u : 0x7au) ||
        registers->e || (!enter && registers->xf) ||
        registers->sp < (enter ? 0x1f04u : 0x1f00u) ||
        registers->sp > (enter ? 0x1fffu : 0x1ff9u) ||
        (enter && !Lufia2DecompIsWramByte(bank | 0x0566u)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (enter ? 29u : 15u))
        return 0;

    unsigned offset;
    const unsigned length = enter ? 17u : 3u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = enter ? "\xe2\x20\xc2\x10\x5a\xa9\x20\x8d\x65\x05\x8d\x66\x05\x20\x8c\x88" :
        "\x28\x6b";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    long progress = 0;
    if (!enter) {
        Lufia2SpanFetch(registers, pc, 1u, false);
        uint16_t index = registers->read(registers->mem, ++registers->sp);
        index |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        Lufia2DecompApplyOperand(registers, 0xa0u, index, false);
        Lufia2SpanAccountInstruction(cpu, 5u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 1u, 1u, true);
        const uint8_t status = registers->read(registers->mem, ++registers->sp);
        interp816_setFlags(registers, status);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 2;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 2u, 1u, true);
        uint16_t address = registers->read(registers->mem, ++registers->sp);
        address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        registers->k = registers->read(registers->mem, ++registers->sp);
        registers->pc = (uint16_t)(address + 1u);
        registers->cyclesUsed = 6u;
        Lufia2SpanAccountInstruction(cpu, 6u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 3;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        *tail_steps = progress ? 3 - progress : -2;
        return 15;
    }

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->write(registers->mem, registers->sp--, interp816_getFlags(registers));
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    registers->xf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 5u, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)(registers->y >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->y);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x20u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    for (unsigned field = 0; field < 2u; ++field) {
        Lufia2SpanFetch(registers, pc + 8u + 3u * field, 3u, true);
        registers->write(registers->mem, bank | (0x0565u + field), (uint8_t)registers->a);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 6u + field;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    Lufia2SpanFetch(registers, pc + 14u, 3u, true);
    registers->write(registers->mem, registers->sp--, 0x88u);
    registers->write(registers->mem, registers->sp--, 0x88u);
    registers->pc = 0x888cu;
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 8 - progress : -7;
    return 29;
}

static int Lufia2DecompRunMenuPaletteApply(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                               uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t palette_flags = ((uint32_t)registers->db << 16) | 0x0564u;
    if (pc != 0x808d5du || opcode != 0x0au || registers->e || !registers->mf ||
        registers->sp < 0x1f01u || registers->sp > 0x1ffdu || !Lufia2DecompIsWramByte(palette_flags) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 31u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 15u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x0a\x48\xad\x64\x05\x29\xe3\x03\x01\x8d\x64\x05\x68\x60", 14u))
        return 0;

    for (unsigned shift = 0; shift < 2u; ++shift) {
        Lufia2SpanFetch(registers, pc + shift, 1u, shift != 0u);
        const uint8_t palette = (uint8_t)registers->a;
        registers->c = (palette & 0x80u) != 0u;
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(palette << 1), true);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }
    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = (dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch) ? 3 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, palette_flags), true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0xe3u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    const uint8_t palette = registers->read(registers->mem, registers->sp + 1u);
    Lufia2DecompApplyOperand(registers, 0x09u, palette, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 10u, 3u, true);
    registers->write(registers->mem, palette_flags, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 7;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 13u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, ++registers->sp), true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanNearReturn(registers, cpu, pc + 14u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 9;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 9 - progress : -8;
    return 31;
}

static int Lufia2DecompRunMenuPaletteDispatch(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool call = pc == 0x808d22u;
    if ((!call && pc != 0x808d28u) || opcode != (call ? 0xb7u : 0x82u) || registers->e ||
        (call && (!registers->mf || registers->xf || registers->dp || !cpu->ram ||
        cpu->ram != g_snes->ram || registers->sp < 0x1f02u || registers->sp > 0x1fffu)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (call ? 14u : 4u))
        return 0;

    unsigned offset;
    const unsigned length = call ? 6u : 3u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = call ? "\x5d\xc8\x20\x5d\x8d" : "\x7f\xfb";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    if (!call) {
        Lufia2SpanFetch(registers, pc, 3u, false);
        registers->pc = 0x88aau;
        registers->cyclesUsed = 4u;
        Lufia2SpanAccountInstruction(cpu, 4u);
        *tail_steps = -1;
        return 4;
    }

    const uint32_t pointer = cpu->ram[0x5du] | ((uint32_t)cpu->ram[0x5eu] << 8) |
        ((uint32_t)cpu->ram[0x5fu] << 16);
    const uint32_t source = (pointer + registers->y) & 0xffffffu;
    if (!Lufia2DecompIsLoRomDataByte(cart, source) && !Lufia2DecompIsWramByte(source))
        return 0;
    Lufia2SpanFetch(registers, pc, 2u, false);
    uint32_t address = Lufia2DecompReadData(registers, 0x5du, 0x5eu, true);
    address |= (uint32_t)registers->read(registers->mem, 0x5fu) << 16;
    const uint8_t palette = registers->read(registers->mem, (address + registers->y) & 0xffffffu);
    Lufia2DecompApplyOperand(registers, 0xa9u, palette, true);
    Lufia2SpanAccountInstruction(cpu, 6u);
    const bool read_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->y + 1u, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 3u, 3u, true);
    registers->write(registers->mem, registers->sp--, 0x8du);
    registers->write(registers->mem, registers->sp--, 0x27u);
    registers->pc = 0x8d5du;
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    const bool call_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    *tail_steps = call_progress ? 0 : read_progress ? 2 : -2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 14;
}

static int Lufia2DecompRunMenuTileBlockStart(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x8280a5u || opcode != 0xa2u || registers->e || registers->mf || registers->xf ||
        registers->dp || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 19u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 11u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x04\x00\x86\x56\x86\x58\xa6\x54\xa5\x5a", 10u))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    Lufia2DecompApplyOperand(registers, 0xa2u, 4u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = 0;
    for (unsigned field = 0; field < 2; ++field) {
        const uint32_t address = 0x56u + 2u * field;
        Lufia2SpanFetch(registers, pc + 3u + 2u * field, 2u, true);
        Lufia2DecompWriteData(registers, address, address + 1u, registers->x, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 2u + field;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    const uint16_t position = Lufia2DecompReadData(registers, 0x54u, 0x55u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, position, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 2u, true);
    const uint16_t tile = Lufia2DecompReadData(registers, 0x5au, 0x5bu, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, tile, false);
    registers->pc = 0x80b0u;
    registers->cyclesUsed = 4u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 5 - progress : -4;
    return 19;
}

static int Lufia2DecompRunMenuTileBlockRow(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool first = pc == 0x8280b0u;
    const bool finish = pc == 0x8280c9u;
    if ((!first && !finish && pc != 0x8280bdu) ||
        opcode != (first ? 0xa4u : finish ? 0x60u : 0xa8u) ||
        registers->e || registers->mf || registers->xf || registers->dp ||
        (!first && !finish && registers->d) ||
        registers->sp < (first ? 0x1f01u : 0x1f00u) || registers->sp > (first ? 0x1fffu : 0x1ffdu) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (first ? 8u : finish ? 6u : 26u))
        return 0;

    unsigned offset;
    const unsigned length = first ? 3u : finish ? 1u : 12u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = first ? "\x56\xda" : "\x68\x18\x69\x40\x00\xaa\x98\xc6\x58\xd0\xe7";
    if (!cart || (!finish && memcmp(cart->rom + offset + 1u, signature, length - 1u)))
        return 0;

    if (finish) {
        Lufia2SpanFetch(registers, pc, 1u, false);
        uint16_t address = registers->read(registers->mem, ++registers->sp);
        address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        registers->pc = (uint16_t)(address + 1u);
        registers->cyclesUsed = 6u;
        Lufia2SpanAccountInstruction(cpu, 6u);
        *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : -1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        return 6;
    }

    if (first) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        const uint16_t columns = Lufia2DecompReadData(registers, 0x56u, 0x57u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, columns, false);
        Lufia2SpanAccountInstruction(cpu, 4u);
        const bool read_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 2u, 1u, true);
        registers->write(registers->mem, registers->sp--, (uint8_t)(registers->x >> 8));
        registers->write(registers->mem, registers->sp--, (uint8_t)registers->x);
        registers->pc = 0x80b3u;
        registers->cyclesUsed = 4u;
        Lufia2SpanAccountInstruction(cpu, 4u);
        const bool stack_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
            write_epoch != g_interp_bridge_write_epoch;
        *tail_steps = stack_progress ? 0 : read_progress ? 1 : -1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        return 8;
    }

    Lufia2SpanFetch(registers, pc, 1u, false);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 1u, 1u, true);
    uint16_t start = registers->read(registers->mem, ++registers->sp);
    start |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa9u, start, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 2 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 3u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 64u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 6u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 7u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x58u, 0x59u, true);
    Lufia2SpanAccountInstruction(cpu, 7u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 7;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    registers->pc = registers->z ? 0x80c9u : 0x80b0u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 8 - progress : -7;
    return 23u + registers->cyclesUsed;
}

static int Lufia2DecompRunMenuSlotPaletteSetup(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                    uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool change_width = pc == 0x86911fu;
    const bool initialize = change_width || pc == 0x869121u;
    const uint32_t bank = (uint32_t)registers->db << 16;
    if ((!initialize && pc != 0x869130u) || opcode != (change_width ? 0xc2u : initialize ? 0xa2u : 0xa6u) ||
        registers->e || (!change_width && registers->mf) || registers->xf || registers->dp ||
        !Lufia2DecompIsWramByte(bank | 0x0b7bu) ||
        (!initialize && (registers->d || registers->sp < 0x1f00u || registers->sp > 0x1fffu ||
        !cpu->ram || cpu->ram != g_snes->ram || cpu->ram[0x05u])) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (change_width ? 27u : initialize ? 24u : 51u))
        return 0;

    unsigned offset;
    const unsigned length = change_width ? 17u : initialize ? 15u : 32u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = initialize ? "\xa0\x04\x86\x02\xad\x7a\x0a\x29\xff\x00\x85\x15\x64\x04" :
        "\x04\xbd\x7b\x0a\x29\xff\x00\xaa\x8b\xbf\x65\x91\x86\x29\xff\x00\x0a\x0a\x0a\x0a\x0a"
        "\x18\x69\xf8\xcc\xaa\xa4\x02\xa9\x1f\x00";
    const unsigned width_bytes = change_width ? 2u : 0u;
    if (!cart || (change_width && memcmp(cart->rom + offset + 1u, "\x20\xa2", 2u)) ||
        memcmp(cart->rom + offset + 1u + width_bytes, signature, length - 1u - width_bytes) ||
        (!initialize && !Lufia2DecompIsLoRomDataByte(cart, 0x869265u)))
        return 0;

    long progress = 0;
    if (initialize) {
        if (change_width) {
            Lufia2SpanFetch(registers, pc, 2u, false);
            registers->mf = false;
            Lufia2SpanAccountInstruction(cpu, 3u);
            pc += 2u;
        }
        Lufia2SpanFetch(registers, pc, 3u, change_width);
        Lufia2DecompApplyOperand(registers, 0xa2u, 0x04a0u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanFetch(registers, pc + 3u, 2u, true);
        Lufia2DecompWriteData(registers, 0x02u, 0x03u, registers->x, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 2;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 5u, 3u, true);
        const uint16_t count = Lufia2DecompReadData(registers, bank | 0x0a7au, bank | 0x0a7bu, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, count, false);
        Lufia2SpanAccountInstruction(cpu, 5u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 3;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 8u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0x29u, 0xffu, false);
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanFetch(registers, pc + 11u, 2u, true);
        Lufia2DecompWriteData(registers, 0x15u, 0x16u, registers->a, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 5;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 13u, 2u, true);
        Lufia2DecompWriteData(registers, 0x04u, 0x05u, 0u, true);
        registers->pc = 0x9130u;
        registers->cyclesUsed = 4u;
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 6;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        *tail_steps = progress ? 6 - progress : change_width ? -6 : -5;
        return change_width ? 27 : 24;
    }

    Lufia2SpanFetch(registers, pc, 2u, false);
    const uint16_t entry_index = Lufia2DecompReadData(registers, 0x04u, 0x05u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, entry_index, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 3u, true);
    const uint32_t entry = (bank | 0x0a7bu) + registers->x;
    const uint16_t entry_word = Lufia2DecompReadData(registers, entry, entry + 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, entry_word, false);
    const unsigned list_cycles = entry_index > 0x84u ? 6u : 5u;
    Lufia2SpanAccountInstruction(cpu, list_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 5u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0xffu, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 8u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 9u, 1u, true);
    registers->write(registers->mem, registers->sp--, registers->db);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 10u, 4u, true);
    const uint32_t class_address = 0x869165u + registers->x;
    const uint16_t palette_class = Lufia2DecompReadData(registers, class_address, class_address + 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, palette_class, false);
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 14u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0xffu, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    for (unsigned shift = 0; shift < 5u; ++shift) {
        Lufia2SpanFetch(registers, pc + 17u + shift, 1u, true);
        registers->c = (registers->a & 0x8000u) != 0u;
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 22u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 23u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 0xccf8u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 26u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 27u, 2u, true);
    const uint16_t destination = Lufia2DecompReadData(registers, 0x02u, 0x03u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, destination, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 16;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 29u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 31u, false);
    registers->pc = 0x9150u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 17 - progress : -16;
    return 45u + list_cycles;
}

static int Lufia2DecompRunMenuSlotPaletteAdvance(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                     uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool finish = pc == 0x869162u;
    if ((!finish && pc != 0x869153u) || opcode != (finish ? 0xe2u : 0xabu) ||
        registers->e || registers->mf || registers->xf ||
        (!finish && (registers->dp || registers->d)) ||
        registers->sp < 0x1f00u || registers->sp > (finish ? 0x1ffcu : 0x1ffeu) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (finish ? 9u : 34u))
        return 0;

    unsigned offset;
    const unsigned length = finish ? 3u : 15u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = finish ? "\x20\x6b" : "\xa5\x02\x18\x69\x20\x00\x85\x02\xe6\x04\xc6\x15\xd0\xce";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    if (finish) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        registers->mf = true;
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanFetch(registers, pc + 2u, 1u, true);
        uint16_t address = registers->read(registers->mem, ++registers->sp);
        address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        registers->k = registers->read(registers->mem, ++registers->sp);
        registers->pc = (uint16_t)(address + 1u);
        registers->cyclesUsed = 6u;
        Lufia2SpanAccountInstruction(cpu, 6u);
        *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : -1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        return 9;
    }

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->n = (registers->db & 0x80u) != 0u;
    registers->z = registers->db == 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 2u, true);
    const uint16_t destination = Lufia2DecompReadData(registers, 0x02u, 0x03u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, destination, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 4u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 32u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    Lufia2DecompWriteData(registers, 0x02u, 0x03u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    for (unsigned phase = 0; phase < 2; ++phase) {
        const uint32_t counter = phase ? 0x15u : 0x04u;
        Lufia2SpanFetch(registers, pc + 9u + 2u * phase, 2u, true);
        Lufia2DecompModifyWram(registers, phase ? 0xc6u : 0xe6u, counter, counter + 1u, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 6u + phase;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }
    Lufia2SpanFetch(registers, pc + 13u, 2u, true);
    registers->pc = registers->z ? 0x9162u : 0x9130u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 8 - progress : -7;
    return 31u + registers->cyclesUsed;
}

static int Lufia2DecompRunMenuWindowEdgeBoundaries(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                        uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool begin = pc == 0x8281e6u;
    if ((!begin && pc != 0x82820au) || opcode != (begin ? 0xbeu : 0x60u) ||
        registers->e || registers->mf || registers->xf || (begin && (registers->dp || registers->d)) ||
        (!begin && (registers->sp < 0x1f00u || registers->sp > 0x1ffdu)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (begin ? 30u : 6u))
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, begin ? 19u : 1u, &offset);
    if (!cart || (begin && memcmp(cart->rom + offset + 1u, "\x7e\x82\x86\x5d\xaa\xa0\x00\x00"
        "\xa5\x56\x38\xe9\x04\x00\xf0\x13\x85\x5a", 18u)))
        return 0;

    if (!begin) {
        Lufia2SpanFetch(registers, pc, 1u, false);
        uint16_t address = registers->read(registers->mem, ++registers->sp);
        address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        registers->pc = (uint16_t)(address + 1u);
        registers->cyclesUsed = 6u;
        Lufia2SpanAccountInstruction(cpu, 6u);
        *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : -1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        return 6;
    }

    const unsigned table_index = 0x827eu + registers->y;
    const uint32_t source = (((uint32_t)registers->db << 16) + table_index) & 0xffffffu;
    const uint32_t high = (source + 1u) & 0xffffffu;
    if (!(Lufia2DecompIsWramByte(source) && Lufia2DecompIsWramByte(high)) &&
        !(Lufia2DecompIsLoRomDataByte(cart, source) && Lufia2DecompIsLoRomDataByte(cart, high)))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint16_t tiles = Lufia2DecompReadData(registers, source, high, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, tiles, false);
    const unsigned read_cycles = 5u + ((table_index >> 8) != 0x82u);
    Lufia2SpanAccountInstruction(cpu, read_cycles);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    Lufia2DecompWriteData(registers, 0x5du, 0x5eu, registers->x, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 5u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 6u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 9u, 2u, true);
    const uint16_t width = Lufia2DecompReadData(registers, 0x56u, 0x57u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, width, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 11u, 1u, true);
    registers->c = true;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 12u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xe9u, 4u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 15u, 2u, true);
    if (registers->z) {
        registers->pc = 0x820au;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 8 - progress : -7;
        return read_cycles + 21u;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 17u, 2u, true);
    Lufia2DecompWriteData(registers, 0x5au, 0x5bu, registers->a, true);
    registers->pc = 0x81f9u;
    registers->cyclesUsed = 4u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 9;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 9 - progress : -8;
    return read_cycles + 24u;
}

static int Lufia2DecompRunMenuTileRecolor(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t address = 0x7e2800u + registers->x;
    if (pc != 0x8280e4u || opcode != 0xbfu || registers->e || registers->mf || registers->xf ||
        registers->dp || address >= 0x7effffu || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 28u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 18u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x00\x28\x7e\x29\xff\xe3\x05\x5a"
        "\x9f\x00\x28\x7e\xe8\xe8\x88\xd0\xee", 17u))
        return 0;

    Lufia2SpanFetch(registers, pc, 4u, false);
    const uint16_t tile = Lufia2DecompReadData(registers, address, address + 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, tile, false);
    Lufia2SpanAccountInstruction(cpu, 6u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0xe3ffu, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    const uint16_t palette = Lufia2DecompReadData(registers, 0x5au, 0x5bu, true);
    Lufia2DecompApplyOperand(registers, 0x09u, palette, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 4u, true);
    Lufia2DecompWriteData(registers, address, address + 1u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    for (unsigned increment = 0; increment < 2; ++increment) {
        Lufia2SpanFetch(registers, pc + 13u + increment, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }
    Lufia2SpanFetch(registers, pc + 15u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y - 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 16u, 2u, true);
    registers->pc = registers->z ? 0x80f6u : 0x80e4u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 8 - progress : -7;
    return 25u + registers->cyclesUsed;
}

static int Lufia2DecompRunMenuWindowEdgeTile(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t destination = 0x7e2000u + registers->x;
    if (pc != 0x8281f9u || opcode != 0xb1u || registers->e || registers->mf || registers->xf ||
        registers->dp || destination >= 0x7effffu || !cpu->ram || cpu->ram != g_snes->ram ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 34u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 17u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x5d\x9f\x00\x20\x7e\xe8\xe8\x98\x49\x02\x00"
        "\xa8\xc6\x5a\xd0\xef", 16u))
        return 0;
    const unsigned base = cpu->ram[0x5du] | ((unsigned)cpu->ram[0x5eu] << 8);
    const uint32_t bank = (uint32_t)registers->db << 16;
    const uint32_t first = (bank + base + registers->y) & 0xffffffu;
    const uint32_t last = (first + 1u) & 0xffffffu;
    if (!(Lufia2DecompIsWramByte(first) && Lufia2DecompIsWramByte(last)) &&
        !(Lufia2DecompIsLoRomDataByte(cart, first) && Lufia2DecompIsLoRomDataByte(cart, last)))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    const uint16_t pointer = Lufia2DecompReadData(registers, 0x5du, 0x5eu, true);
    const uint32_t source = (bank + pointer + registers->y) & 0xffffffu;
    const uint16_t tile = Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, tile, false);
    const unsigned read_cycles = 6u + ((pointer >> 8) != ((pointer + registers->y) >> 8));
    Lufia2SpanAccountInstruction(cpu, read_cycles);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 4u, true);
    Lufia2DecompWriteData(registers, destination, destination + 1u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    for (unsigned step = 0; step < 2; ++step) {
        Lufia2SpanFetch(registers, pc + 6u + step, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 8u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 9u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x49u, 2u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 12u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 13u, 2u, true);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x5au, 0x5bu, true);
    Lufia2SpanAccountInstruction(cpu, 7u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 15u, 2u, true);
    registers->pc = registers->z ? 0x820au : 0x81f9u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 9 - progress : -8;
    return read_cycles + 24u + registers->cyclesUsed;
}

static int Lufia2DecompRunMenuRecolorRow(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                             uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x8280f6u || opcode != 0xa5u || registers->e || registers->mf || registers->xf ||
        registers->dp || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 23u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 12u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x54\x18\x69\x40\x00\x85\x54\xc6\x58\xd0\xde", 11u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    const uint16_t position = Lufia2DecompReadData(registers, 0x54u, 0x55u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, position, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 3u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 64u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    Lufia2DecompWriteData(registers, 0x54u, 0x55u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x58u, 0x59u, true);
    Lufia2SpanAccountInstruction(cpu, 7u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    registers->pc = registers->z ? 0x8102u : 0x80e0u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 6 - progress : -5;
    return 20u + registers->cyclesUsed;
}

static int Lufia2DecompRunMenuRecolorColumns(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                 uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x8280e0u || opcode != 0xa4u || registers->e || registers->mf || registers->xf ||
        registers->dp || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 8u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 4u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x56\xa6\x54", 3u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    const uint16_t columns = Lufia2DecompReadData(registers, 0x56u, 0x57u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, columns, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool read_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    const uint16_t position = Lufia2DecompReadData(registers, 0x54u, 0x55u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, position, false);
    registers->pc = 0x80e4u;
    registers->cyclesUsed = 4u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : read_progress ? 1 : -1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 8;
}

static int Lufia2DecompRunMenuRecolorSetup(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                               uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x8280cau || opcode != 0x85u || registers->e || registers->mf || registers->xf ||
        registers->dp || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 42u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 22u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x54\x64\x56\x64\x58\x98\xeb\x0a\x0a"
        "\x85\x5a\x8a\xe2\x20\x85\x58\xeb\x85\x56\xc2\x20", 21u))
        return 0;

    long progress = 0;
    for (unsigned word = 0; word < 3u; ++word) {
        const uint32_t destination = 0x54u + word * 2u;
        Lufia2SpanFetch(registers, pc + word * 2u, 2u, word != 0);
        Lufia2DecompWriteData(registers, destination, destination + 1u, word ? 0u : registers->a, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = word + 1u;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    Lufia2SpanFetch(registers, pc + 6u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 7u, 1u, true);
    MenuGlyphExchangeBytes(registers);
    Lufia2SpanAccountInstruction(cpu, 3u);
    for (unsigned shift = 0; shift < 2u; ++shift) {
        Lufia2SpanFetch(registers, pc + 8u + shift, 1u, true);
        registers->c = (registers->a & 0x8000u) != 0;
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    Lufia2DecompWriteData(registers, 0x5au, 0x5bu, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 12u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->x, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 13u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 15u, 2u, true);
    registers->write(registers->mem, 0x58u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 11;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 17u, 1u, true);
    MenuGlyphExchangeBytes(registers);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 18u, 2u, true);
    registers->write(registers->mem, 0x56u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 13;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 20u, 2u, true);
    registers->mf = false;
    registers->pc = 0x80e0u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 14 - progress : -13;
    return 42;
}

static int Lufia2DecompRunMenuRecolorRedraw(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool returning = pc == 0x82810bu;
    if ((!returning && pc != 0x828102u) || opcode != (returning ? 0xc2u : 0xe2u) ||
        registers->e || registers->mf != returning || registers->xf || registers->dp ||
        registers->sp < 0x1f02u || registers->sp > (returning ? 0x1ffdu : 0x1fffu) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (returning ? 9u : 16u))
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, returning ? 3u : 9u, &offset);
    if (!cart || (returning ? memcmp(cart->rom + offset + 1u, "\x20\x60", 2u) :
        memcmp(cart->rom + offset + 1u, "\x20\xa9\x20\x04\x74\x20\xc2\x93", 8u)))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = !returning;
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (returning) {
        Lufia2SpanFetch(registers, pc + 2u, 1u, true);
        uint16_t address = registers->read(registers->mem, ++registers->sp);
        address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        registers->pc = (uint16_t)(address + 1u);
        registers->cyclesUsed = 6u;
        Lufia2SpanAccountInstruction(cpu, 6u);
        *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : -1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        return 9;
    }

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x20u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    Lufia2DecompModifyWram(registers, 0x04u, 0x74u, 0x75u, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    const bool redraw_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 6u, 3u, true);
    registers->write(registers->mem, registers->sp--, 0x81u);
    registers->write(registers->mem, registers->sp--, 0x0au);
    registers->pc = 0x93c2u;
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    const bool call_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    *tail_steps = call_progress ? 0 : redraw_progress ? 1 : -3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 16;
}

static int Lufia2DecompRunMenuSlideTick(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                            uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x828afau || opcode != 0xc6u || registers->e || !registers->mf ||
        registers->dp || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 8u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 4u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\xed\xd0\x08", 3u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompModifyWram(registers, 0xc6u, 0xedu, 0xeeu, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    const bool counter_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    registers->pc = registers->z ? 0x8afeu : 0x8b06u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = counter_progress ? 1 : -1;
    return 5u + registers->cyclesUsed;
}

static int Lufia2DecompRunMenuSlideRestart(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x828afeu || opcode != 0xa9u || registers->e || !registers->mf ||
        registers->dp || registers->sp < 0x1f03u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 13u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 8u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x10\x85\xed\x22\x55\x8b\x86", 7u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x10u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    registers->write(registers->mem, 0xedu, 0x10u);
    Lufia2SpanAccountInstruction(cpu, 3u);
    const bool restart_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 4u, true);
    registers->write(registers->mem, registers->sp--, registers->k);
    registers->write(registers->mem, registers->sp--, 0x8bu);
    registers->write(registers->mem, registers->sp--, 0x05u);
    registers->k = 0x86u;
    registers->pc = 0x8b55u;
    registers->cyclesUsed = 8u;
    Lufia2SpanAccountInstruction(cpu, 8u);
    const bool call_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    *tail_steps = call_progress ? 0 : restart_progress ? 1 : -2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 13;
}

static int Lufia2DecompRunMenuLayerTransfer(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool save = pc == 0x828353u;
    if ((!save && pc != 0x828378u) || opcode != 0xbdu || registers->e || registers->mf || registers->xf ||
        registers->db != 0x7eu || registers->x >= 0x800u || (registers->x & 1u) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 36u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 19u, &offset);
    const char *signature = save ? "\x00\x20\x9d\xc0\x83\xbd\x00\x30\x9d\xc0\x8b\xe8\xe8\xe0\x00\x08\xd0\xed" :
        "\xc0\x83\x9d\x00\x20\xbd\xc0\x8b\x9d\x00\x30\xe8\xe8\xe0\x00\x08\xd0\xed";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, 18u))
        return 0;

    const unsigned read_cycles = 5u + (registers->x >= (save ? 0x100u : 0x40u));
    long progress = 0;
    for (unsigned layer = 0; layer < 2u; ++layer) {
        const uint32_t visible = 0x7e2000u + layer * 0x1000u + registers->x;
        const uint32_t backup = 0x7e83c0u + layer * 0x800u + registers->x;
        const uint32_t source = save ? visible : backup;
        const uint32_t destination = save ? backup : visible;
        const uint32_t instruction = pc + layer * 6u;

        Lufia2SpanFetch(registers, instruction, 3u, layer != 0u);
        const uint16_t tile = Lufia2DecompReadData(registers, source, source + 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, tile, false);
        Lufia2SpanAccountInstruction(cpu, read_cycles);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = layer * 2u + 1u;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, instruction + 3u, 3u, true);
        Lufia2DecompWriteData(registers, destination, destination + 1u, tile, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = layer * 2u + 2u;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    for (unsigned increment = 0; increment < 2u; ++increment) {
        Lufia2SpanFetch(registers, pc + 12u + increment, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }
    Lufia2SpanFetch(registers, pc + 14u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xe0u, 0x0800u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 17u, 2u, true);
    registers->pc = (uint16_t)(registers->z ? pc + 19u : pc);
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 8 - progress : -7;
    return read_cycles * 2u + 21u + registers->cyclesUsed;
}

static int Lufia2DecompRunMenuLayerFrame(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                              uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool begin = pc == 0x828345u || pc == 0x82836au;
    if ((!begin && pc != 0x828366u && pc != 0x82838bu) ||
        opcode != (begin ? 0xc2u : 0xabu) || registers->e || registers->mf != begin || registers->xf ||
        registers->sp < (begin ? 0x1f02u : 0x1f00u) || registers->sp > (begin ? 0x1fffu : 0x1ffcu) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (begin ? 24u : 13u))
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, begin ? 14u : 4u, &offset);
    if (!cart || (begin ? memcmp(cart->rom + offset + 1u, "\x20\x8b\xe2\x20\xa9\x7e\x48\xab\xc2\x20\xa2\x00\x00", 13u) :
        memcmp(cart->rom + offset + 1u, "\xe2\x20\x60", 3u)))
        return 0;

    if (!begin) {
        Lufia2SpanFetch(registers, pc, 1u, false);
        registers->db = registers->read(registers->mem, ++registers->sp);
        registers->z = registers->db == 0u;
        registers->n = (registers->db & 0x80u) != 0u;
        Lufia2SpanAccountInstruction(cpu, 4u);
        const bool bank_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 1u, 2u, true);
        registers->mf = true;
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanNearReturn(registers, cpu, pc + 3u);
        *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : bank_progress ? 2 : -2;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        return 13;
    }

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    registers->write(registers->mem, registers->sp--, registers->db);
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 2 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x7eu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 7u, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 8u, 1u, true);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 2u, true);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 11u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0u, false);
    registers->pc = (uint16_t)(pc + 14u);
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 8 - progress : -7;
    return 24;
}
