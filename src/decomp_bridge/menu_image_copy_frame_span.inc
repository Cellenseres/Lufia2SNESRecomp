static int Lufia2DecompRunMenuImageCopyFrame(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    if (pc < 0x868ff6u || pc > 0x869021u || registers->e || registers->xf || registers->dp ||
        !step_budget || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x868ff6u, 0x869022u))
        return 0;
    if (pc != 0x868ff6u && pc != 0x869008u &&
        registers->mf != (pc < 0x869008u || (pc >= 0x869009u && pc <= 0x869013u) || pc == 0x869021u))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x868ff6u;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 44u || memcmp(cart->rom + first,
        "\xe2\x20\x8b\x20\x09\x90\xe6\x09\xe6\x0c\xe6\x0c\x20\x09\x90\xab\xc2\x20\x60"
        "\xa9\x7e\x8d\x7e\x05\xa5\x1b\x8d\x7f\x05\xc2\x20\xa6\x08\xa4\x0b\xa9\xff\x00"
        "\x20\x7d\x05\xe2\x20\x60", 44u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x868ff6u: goto narrow_block;
    case 0x868ff8u: goto save_bank;
    case 0x868ff9u: goto call_first_row;
    case 0x868ffcu: goto advance_source;
    case 0x868ffeu: goto advance_target;
    case 0x869000u: goto advance_target_again;
    case 0x869002u: goto call_second_row;
    case 0x869005u: goto restore_bank;
    case 0x869006u: goto widen_block;
    case 0x869008u: goto return_block;
    case 0x869009u: goto load_destination_bank;
    case 0x86900bu: goto store_destination_bank;
    case 0x86900eu: goto load_source_bank;
    case 0x869010u: goto store_source_bank;
    case 0x869013u: goto widen_row;
    case 0x869015u: goto load_source;
    case 0x869017u: goto load_target;
    case 0x869019u: goto load_count;
    case 0x86901cu: goto call_move;
    case 0x86901fu: goto narrow_row;
    case 0x869021u: goto return_row;
    default: return 0;
    }
narrow_block:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x868ff6u, 2u, span.instructions != 0u);
    registers->mf = true;
    NativeSpanStep(&span, registers, cpu, 3u, 0x8ff8u);
save_bank:
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x868ff8u, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, registers->db);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8ff9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
call_first_row:
    pc = 0x868ff9u;
    goto call_row;
advance_source:
    if (!NativeSpanCanStep(&span, cpu, 5u)) goto finished;
    Lufia2SpanFetch(registers, 0x868ffcu, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, 0x09u, 0x0au, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x8ffeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
advance_target:
    if (!NativeSpanCanStep(&span, cpu, 5u)) goto finished;
    Lufia2SpanFetch(registers, 0x868ffeu, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, 0x0cu, 0x0du, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9000u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
advance_target_again:
    if (!NativeSpanCanStep(&span, cpu, 5u)) goto finished;
    Lufia2SpanFetch(registers, 0x869000u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, 0x0cu, 0x0du, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9002u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
call_second_row:
    pc = 0x869002u;
call_row:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    {
        if (span.instructions) Lufia2SpanProbeOpcode(registers, pc);
        const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, pc, 0x20u,
            0x869009u, false, true, false);
        if (!cycles) goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
restore_bank:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x869005u, 1u, span.instructions != 0u);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x9006u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
widen_block:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x869006u, 2u, span.instructions != 0u);
    registers->mf = false;
    NativeSpanStep(&span, registers, cpu, 3u, 0x9008u);
return_block:
    pc = 0x869008u;
    goto return_short;
load_destination_bank:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x869009u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x7eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x900bu);
store_destination_bank:
    {
        const uint32_t address = ((uint32_t)registers->db << 16) | 0x057eu;
        if (!NativeSpanCanStep(&span, cpu, 4u) || !Lufia2DecompIsWramByte(address)) goto finished;
        Lufia2SpanFetch(registers, 0x86900bu, 3u, span.instructions != 0u);
        registers->write(registers->mem, address, (uint8_t)registers->a);
        NativeSpanStep(&span, registers, cpu, 4u, 0x900eu);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
load_source_bank:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x86900eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, 0x1bu), true);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9010u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
store_source_bank:
    {
        const uint32_t address = ((uint32_t)registers->db << 16) | 0x057fu;
        if (!NativeSpanCanStep(&span, cpu, 4u) || !Lufia2DecompIsWramByte(address)) goto finished;
        Lufia2SpanFetch(registers, 0x869010u, 3u, span.instructions != 0u);
        registers->write(registers->mem, address, (uint8_t)registers->a);
        NativeSpanStep(&span, registers, cpu, 4u, 0x9013u);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
widen_row:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x869013u, 2u, span.instructions != 0u);
    registers->mf = false;
    NativeSpanStep(&span, registers, cpu, 3u, 0x9015u);
load_source:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x869015u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, 0x08u, 0x09u, true), false);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9017u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_target:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x869017u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, 0x0bu, 0x0cu, true), false);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9019u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_count:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x869019u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x901cu);
call_move:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    {
        if (span.instructions) Lufia2SpanProbeOpcode(registers, 0x86901cu);
        const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x86901cu, 0x20u,
            0x86057du, false, false, false);
        if (!cycles) goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
narrow_row:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x86901fu, 2u, span.instructions != 0u);
    registers->mf = true;
    NativeSpanStep(&span, registers, cpu, 3u, 0x9021u);
return_row:
    pc = 0x869021u;
return_short:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, pc, 1u, span.instructions != 0u);
    {
        uint16_t address = registers->read(registers->mem, ++registers->sp);
        address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        NativeSpanStep(&span, registers, cpu, 6u, (uint16_t)(address + 1u));
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
finished:
    if (!span.instructions) return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
