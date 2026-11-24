static int Lufia2DecompRunFieldPendingScan(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    switch (pc) {
    case 0x80ee7bu: case 0x80ee7cu: case 0x80ee80u: case 0x80ee82u:
    case 0x80eedeu: case 0x80eedfu: case 0x80eee0u: case 0x80eee3u:
        break;
    default: return 0;
    }
    if (registers->e || !registers->mf || registers->xf || !step_budget ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x80ee7bu, 0x80ee84u) ||
        !Lufia2DecompFrameRangeQuiet(0x80eedeu, 0x80eee5u))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = (pc & 0xffffu) - 0xee7bu;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 0x6au ||
        memcmp(cart->rom + first, "\xda\xbf\x9c\xd6\x7f\xc9\xff\xf0\x5a", 9u) ||
        memcmp(cart->rom + first + 0x63u, "\xfa\xe8\xe0\x30\x00\xd0\x96", 7u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x80ee7bu: goto save_index;
    case 0x80ee7cu: goto load_record;
    case 0x80ee80u: goto compare_empty;
    case 0x80ee82u: goto branch_empty;
    case 0x80eedeu: goto restore_index;
    case 0x80eedfu: goto advance_index;
    case 0x80eee0u: goto compare_end;
    case 0x80eee3u: goto repeat_scan;
    default: return 0;
    }
save_index:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee7bu, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, (uint8_t)(registers->x >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->x);
    NativeSpanStep(&span, registers, cpu, 4u, 0xee7cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_record:
    {
        const uint32_t address = 0x7fd69cu + registers->x;
        if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsWramByte(address))
            goto finished;
        Lufia2SpanFetch(registers, 0x80ee7cu, 4u, span.instructions != 0u);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
        NativeSpanStep(&span, registers, cpu, 5u, 0xee80u);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
compare_empty:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x80ee80u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0xffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee82u);
branch_empty:
    {
        const bool empty = registers->z;
        const unsigned cycles = empty ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x80ee82u, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, empty ? 0xeedeu : 0xee84u);
        if (!empty) goto finished;
    }
restore_index:
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80eedeu, 1u, span.instructions != 0u);
    {
        uint16_t value = registers->read(registers->mem, ++registers->sp);
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        Lufia2DecompApplyOperand(registers, 0xa2u, value, false);
        NativeSpanStep(&span, registers, cpu, 5u, 0xeedfu);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
advance_index:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x80eedfu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xeee0u);
compare_end:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x80eee0u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, 0x0030u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xeee3u);
repeat_scan:
    {
        const bool repeat = !registers->z;
        const unsigned cycles = repeat ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x80eee3u, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, repeat ? 0xee7bu : 0xeee5u);
        if (repeat && ++span.slots < 8u) goto save_index;
    }
finished:
    if (!span.instructions) return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
