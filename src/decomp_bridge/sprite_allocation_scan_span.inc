static int Lufia2DecompRunSpriteAllocationScan(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    if (registers->e || !registers->mf || !registers->xf || registers->x > 0xffu ||
        registers->y > 0xffu || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted || !step_budget ||
        !Lufia2DecompFrameRangeQuiet(0x83ab8au, 0x83aba5u))
        return 0;
    switch (pc) {
    case 0x83ab8au: case 0x83ab8du: case 0x83ab8fu: case 0x83ab90u: case 0x83ab92u:
    case 0x83ab94u: case 0x83ab95u: case 0x83ab97u: case 0x83ab99u: case 0x83ab9au:
    case 0x83ab9cu: case 0x83ab9eu: case 0x83aba0u: case 0x83aba1u: case 0x83aba3u:
        break;
    default: return 0;
    }
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = (pc & 0xffffu) - 0xab8au;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 27u || memcmp(cart->rom + first,
        "\xbd\x00\xe1\xd0\x08\xc8\xc4\x54\xf0\x14\xe8\x80\xf3\xa5\x56\x18"
        "\x65\x54\x85\x56\x85\x55\xaa\xc9\x80\xd0\xe5", 27u))
        return 0;
    const uint32_t bank = (uint32_t)registers->db << 16;
    const uint16_t request = (uint16_t)(registers->dp + 0x54u);
    const uint16_t start = (uint16_t)(registers->dp + 0x55u);
    const uint16_t cursor = (uint16_t)(registers->dp + 0x56u);
    if (!Lufia2DecompIsWramByte(bank + 0xe100u) ||
        !Lufia2DecompIsWramByte(bank + 0xe1ffu) ||
        !Lufia2DecompIsWramByte(request) || !Lufia2DecompIsWramByte(start) ||
        !Lufia2DecompIsWramByte(cursor))
        return 0;
    const unsigned direct_cycles = 3u + ((registers->dp & 0xffu) != 0u);
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83ab8au: goto load_status;
    case 0x83ab8du: goto branch_status;
    case 0x83ab8fu: goto count_free;
    case 0x83ab90u: goto compare_request;
    case 0x83ab92u: goto branch_found;
    case 0x83ab94u: goto advance_slot;
    case 0x83ab95u: goto repeat_free;
    case 0x83ab97u: goto load_cursor;
    case 0x83ab99u: goto clear_carry;
    case 0x83ab9au: goto advance_cursor;
    case 0x83ab9cu: goto store_cursor;
    case 0x83ab9eu: goto store_start;
    case 0x83aba0u: goto select_group;
    case 0x83aba1u: goto compare_end;
    case 0x83aba3u: goto repeat_group;
    default: return 0;
    }
load_status:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x83ab8au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, bank + 0xe100u + registers->x), true);
    NativeSpanStep(&span, registers, cpu, 4u, 0xab8du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
branch_status:
    {
        const bool occupied = !registers->z;
        const unsigned cycles = occupied ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x83ab8du, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, occupied ? 0xab97u : 0xab8fu);
        if (occupied) goto load_cursor;
    }
count_free:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83ab8fu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint8_t)(registers->y + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xab90u);
compare_request:
    if (!NativeSpanCanStep(&span, cpu, direct_cycles)) goto finished;
    Lufia2SpanFetch(registers, 0x83ab90u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc0u, registers->read(registers->mem, request), true);
    NativeSpanStep(&span, registers, cpu, direct_cycles, 0xab92u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
branch_found:
    {
        const bool found = registers->z;
        const unsigned cycles = found ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x83ab92u, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, found ? 0xaba8u : 0xab94u);
        if (found) goto finished;
    }
advance_slot:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83ab94u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint8_t)(registers->x + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xab95u);
repeat_free:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x83ab95u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xab8au);
    if (++span.slots < 8u) goto load_status;
    goto finished;
load_cursor:
    if (!NativeSpanCanStep(&span, cpu, direct_cycles)) goto finished;
    Lufia2SpanFetch(registers, 0x83ab97u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, cursor), true);
    NativeSpanStep(&span, registers, cpu, direct_cycles, 0xab99u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
clear_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83ab99u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xab9au);
advance_cursor:
    if (!NativeSpanCanStep(&span, cpu, direct_cycles)) goto finished;
    Lufia2SpanFetch(registers, 0x83ab9au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, registers->read(registers->mem, request), true);
    NativeSpanStep(&span, registers, cpu, direct_cycles, 0xab9cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
store_cursor:
    if (!NativeSpanCanStep(&span, cpu, direct_cycles)) goto finished;
    Lufia2SpanFetch(registers, 0x83ab9cu, 2u, span.instructions != 0u);
    registers->write(registers->mem, cursor, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, direct_cycles, 0xab9eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
store_start:
    if (!NativeSpanCanStep(&span, cpu, direct_cycles)) goto finished;
    Lufia2SpanFetch(registers, 0x83ab9eu, 2u, span.instructions != 0u);
    registers->write(registers->mem, start, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, direct_cycles, 0xaba0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
select_group:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83aba0u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint8_t)registers->a, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaba1u);
compare_end:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83aba1u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x80u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaba3u);
repeat_group:
    {
        const bool repeat = !registers->z;
        const unsigned cycles = repeat ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x83aba3u, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, repeat ? 0xab8au : 0xaba5u);
        if (repeat && ++span.slots < 8u) goto load_status;
    }
finished:
    if (!span.instructions) return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
