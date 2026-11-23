static int Lufia2DecompRunBattleStatusIcon(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                         uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc == 0x8588f0u) {
        if (!Lufia2DecompFrameRangeQuiet(pc, pc + 2u))
            return 0;
        return Lufia2DecompRunBattleTilemapReturn(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
    }
    const bool select = pc == 0x85885au;
    const bool advance = pc == 0x8588d4u;
    const bool hide = pc == 0x8588f3u;
    const uint32_t bank = (uint32_t)registers->db << 16;
    const unsigned length = select ? 8u : advance ? 28u : 18u;
    if ((!select && !advance && !hide) || opcode != (select ? 0xb9u : advance ? 0xc2u : 0x5au) ||
        registers->e || registers->xf || (!advance && !registers->mf) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        !Lufia2DecompFrameRangeQuiet(pc, pc + length) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (select ? 10u : advance ? 41u : 36u))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = select ? "\x79\x14\xd0\x03\x4c\xf3\x88" : advance ?
        "\x20\xad\x91\x14\x18\x69\x0d\x00\x8d\x91\x14\xe2\x20\xe8\xe8\xc8\xc8\xc8\xc8\xc0\x14\x00\xf0\x03\x4c\x5a\x88" :
        "\xac\x91\x14\x99\x38\x14\xb9\x35\x14\x29\x7f\x99\x35\x14\x7a\x80\xcf";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    if (select) {
        const uint32_t record = bank + 0x1479u + registers->y;
        if (registers->y > 0xffffu - 0x1479u || !Lufia2DecompIsWramByte(record))
            return 0;
        Lufia2SpanFetch(registers, pc, 3u, false);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, record), true);
        const unsigned read_cycles = 4u + (registers->y >= 0x87u);
        Lufia2SpanAccountInstruction(cpu, read_cycles);
        const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        Lufia2SpanFetch(registers, pc + 3u, 2u, true);
        const bool present = !registers->z;
        Lufia2SpanAccountInstruction(cpu, present ? 3u : 2u);
        if (!present) {
            Lufia2SpanFetch(registers, pc + 5u, 3u, true);
            Lufia2SpanAccountInstruction(cpu, 3u);
        }
        registers->pc = present ? 0x8862u : 0x88f3u;
        registers->cyclesUsed = 3u;
        const long instructions = present ? 2 : 3;
        *tail_steps = progress ? instructions - 1 : 1 - instructions;
        return read_cycles + (present ? 3u : 5u);
    }

    const uint32_t cursor = bank + 0x1491u;
    if (!Lufia2DecompIsWramByte(cursor) || !Lufia2DecompIsWramByte(cursor + 1u))
        return 0;
    if (advance) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        registers->mf = false;
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanFetch(registers, pc + 2u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, cursor, cursor + 1u, true), false);
        Lufia2SpanAccountInstruction(cpu, 5u);
        long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 2 : 0;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        Lufia2SpanFetch(registers, pc + 5u, 1u, true);
        registers->c = false;
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, pc + 6u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0x69u, 13u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanFetch(registers, pc + 9u, 3u, true);
        Lufia2DecompWriteData(registers, cursor, cursor + 1u, registers->a, true);
        Lufia2SpanAccountInstruction(cpu, 5u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 5;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        Lufia2SpanFetch(registers, pc + 12u, 2u, true);
        registers->mf = true;
        Lufia2SpanAccountInstruction(cpu, 3u);
        for (unsigned step = 0; step < 6u; ++step) {
            Lufia2SpanFetch(registers, pc + 14u + step, 1u, true);
            const uint16_t value = step < 2u ? registers->x : registers->y;
            Lufia2DecompApplyOperand(registers, step < 2u ? 0xa2u : 0xa0u, (uint16_t)(value + 1u), false);
            Lufia2SpanAccountInstruction(cpu, 2u);
        }
        Lufia2SpanFetch(registers, pc + 20u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xc0u, 20u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanFetch(registers, pc + 23u, 2u, true);
        const bool done = registers->z;
        Lufia2SpanAccountInstruction(cpu, done ? 3u : 2u);
        if (!done) {
            Lufia2SpanFetch(registers, pc + 25u, 3u, true);
            Lufia2SpanAccountInstruction(cpu, 3u);
        }
        registers->pc = done ? 0x88f0u : 0x885au;
        registers->cyclesUsed = 3u;
        const long instructions = done ? 14 : 15;
        *tail_steps = progress ? instructions - progress : 1 - instructions;
        return done ? 39 : 41;
    }

    if (!cpu->ram || cpu->ram != g_snes->ram || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        return 0;
    const unsigned cursor_offset = (registers->db == 0x7fu ? 0x10000u : 0u) + 0x1491u;
    const uint16_t sprite_offset = cpu->ram[cursor_offset] | ((uint16_t)cpu->ram[cursor_offset + 1u] << 8);
    if (sprite_offset > 0xffffu - 0x1438u ||
        !Lufia2DecompIsWramByte(bank + 0x1435u + sprite_offset) ||
        !Lufia2DecompIsWramByte(bank + 0x1438u + sprite_offset))
        return 0;
    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->write(registers->mem, registers->sp--, (uint8_t)(registers->y >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->y);
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 1u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, cursor, cursor + 1u, true), false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    const uint32_t tile = (bank + 0x1438u + registers->y) & 0xffffffu;
    const uint32_t state = (bank + 0x1435u + registers->y) & 0xffffffu;
    Lufia2SpanFetch(registers, pc + 4u, 3u, true);
    registers->write(registers->mem, tile, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 7u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, state), true);
    const unsigned read_cycles = 4u + (registers->y >= 0xcbu);
    Lufia2SpanAccountInstruction(cpu, read_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x7fu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 12u, 3u, true);
    registers->write(registers->mem, state, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 15u, 1u, true);
    uint16_t saved_y = registers->read(registers->mem, ++registers->sp);
    saved_y |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, saved_y, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 7;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 16u, 2u, true);
    registers->pc = 0x88d4u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 8 - progress : -7;
    return 31u + read_cycles;
}
