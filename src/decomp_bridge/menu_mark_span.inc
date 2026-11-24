static int Lufia2DecompRunMenuMark(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    bool wide;
    switch (pc) {
    case 0x808d6cu: case 0x808d76u: case 0x808d79u: case 0x808d8bu: case 0x808d8eu:
        wide = false;
        break;
    case 0x808d6eu: case 0x808d71u: case 0x808d74u: case 0x808d7bu: case 0x808d7eu:
    case 0x808d7fu: case 0x808d81u: case 0x808d82u: case 0x808d83u: case 0x808d86u:
    case 0x808d89u:
        wide = true;
        break;
    default: return 0;
    }
    const bool save = pc <= 0x808d76u;
    if (registers->e || registers->mf == wide || registers->xf || !step_budget ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(save ? 0x808d6cu : 0x808d79u, save ? 0x808d79u : 0x808d91u))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = (pc & 0xffffu) - 0x8d6cu;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 37u || memcmp(cart->rom + first,
        "\xc2\x20\xad\x75\x05\x8d\x79\x05\xe2\x20\x82\x31\xfb"
        "\xc2\x20\xad\x79\x05\x18\x77\x5d\xc8\xc8\x8d\x75\x05"
        "\x8d\x73\x05\xe2\x20\x20\x0f\x8e\x82\x19\xfb", 37u))
        return 0;
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x808d76u && pc != 0x808d8bu && pc != 0x808d8eu &&
        (!Lufia2DecompIsWramByte(bank + 0x0573u) || !Lufia2DecompIsWramByte(bank + 0x057au)))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x808d6cu: case 0x808d79u: goto widen;
    case 0x808d6eu: case 0x808d7bu: goto load_position;
    case 0x808d71u: goto store_mark;
    case 0x808d74u: case 0x808d89u: goto narrow;
    case 0x808d76u: case 0x808d8eu: goto resume_text;
    case 0x808d7eu: goto clear_carry;
    case 0x808d7fu: goto add_offset;
    case 0x808d81u: goto skip_low;
    case 0x808d82u: goto skip_high;
    case 0x808d83u: goto store_cursor;
    case 0x808d86u: goto store_row;
    case 0x808d8bu: goto reset_width;
    default: return 0;
    }
widen:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, save ? 0x808d6cu : 0x808d79u, 2u, false);
    registers->mf = false;
    NativeSpanStep(&span, registers, cpu, 3u, save ? 0x8d6eu : 0x8d7bu);
load_position:
    if (!NativeSpanCanStep(&span, cpu, 5u)) goto finished;
    Lufia2SpanFetch(registers, save ? 0x808d6eu : 0x808d7bu, 3u, span.instructions != 0u);
    {
        const uint32_t address = bank + (save ? 0x0575u : 0x0579u);
        const uint16_t value = Lufia2DecompReadData(registers, address, address + 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, value, false);
        NativeSpanStep(&span, registers, cpu, 5u, save ? 0x8d71u : 0x8d7eu);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
        if (!save) goto clear_carry;
    }
store_mark:
    if (!NativeSpanCanStep(&span, cpu, 5u)) goto finished;
    Lufia2SpanFetch(registers, 0x808d71u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, bank + 0x0579u, bank + 0x057au, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 5u, 0x8d74u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto narrow;
clear_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x808d7eu, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0x18u);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d7fu);
add_offset:
    {
        const unsigned cycles = 7u + ((registers->dp & 0xffu) != 0u);
        const uint16_t low = (uint16_t)(registers->dp + 0x5du);
        const uint16_t high = (uint16_t)(low + 1u);
        const uint16_t last = (uint16_t)(low + 2u);
        if (!NativeSpanCanStep(&span, cpu, cycles) || !cpu->ram || cpu->ram != g_snes->ram ||
            !Lufia2DecompIsWramByte(low) || !Lufia2DecompIsWramByte(high) || !Lufia2DecompIsWramByte(last))
            goto finished;
        const uint32_t pointer = cpu->ram[low] | (uint32_t)cpu->ram[high] << 8 | (uint32_t)cpu->ram[last] << 16;
        const uint32_t source = (pointer + registers->y) & 0xffffffu;
        const uint32_t next = (source + 1u) & 0xffffffu;
        if ((!Lufia2DecompIsWramByte(source) && !Lufia2DecompIsLoRomDataByte(cart, source | 0x800000u)) ||
            (!Lufia2DecompIsWramByte(next) && !Lufia2DecompIsLoRomDataByte(cart, next | 0x800000u)))
            goto finished;
        Lufia2SpanFetch(registers, 0x808d7fu, 2u, span.instructions != 0u);
        uint32_t address = Lufia2DecompReadData(registers, low, high, true);
        address |= (uint32_t)registers->read(registers->mem, last) << 16;
        address = (address + registers->y) & 0xffffffu;
        const uint16_t value = Lufia2DecompReadData(registers, address, (address + 1u) & 0xffffffu, true);
        Lufia2DecompApplyOperand(registers, 0x69u, value, false);
        NativeSpanStep(&span, registers, cpu, cycles, 0x8d81u);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
skip_low:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x808d81u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d82u);
skip_high:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x808d82u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d83u);
store_cursor:
    if (!NativeSpanCanStep(&span, cpu, 5u)) goto finished;
    Lufia2SpanFetch(registers, 0x808d83u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, bank + 0x0575u, bank + 0x0576u, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 5u, 0x8d86u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
store_row:
    if (!NativeSpanCanStep(&span, cpu, 5u)) goto finished;
    Lufia2SpanFetch(registers, 0x808d86u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, bank + 0x0573u, bank + 0x0574u, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 5u, 0x8d89u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
narrow:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, save ? 0x808d74u : 0x808d89u, 2u, span.instructions != 0u);
    registers->mf = true;
    NativeSpanStep(&span, registers, cpu, 3u, save ? 0x8d76u : 0x8d8bu);
    if (!save) goto finished;
resume_text:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, save ? 0x808d76u : 0x808d8eu, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 4u, 0x88aau);
    goto finished;
reset_width:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    {
        const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, pc, opcode, 0x808e0fu, false, true, false);
        if (!cycles) goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
finished:
    if (!span.instructions) return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
