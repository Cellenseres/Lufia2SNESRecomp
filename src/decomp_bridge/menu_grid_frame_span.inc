static int Lufia2DecompRunMenuGridFrame(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    if (pc < 0x828069u || pc > 0x8280a4u || registers->e || registers->xf || registers->dp ||
        !step_budget || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x828069u, 0x8280a5u))
        return 0;
    if (pc != 0x828069u && pc != 0x8280a4u && registers->mf != (pc >= 0x82809du))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x828069u;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 60u ||
        memcmp(cart->rom + first,
            "\xc2\x20\xa9\x10\x0c\x18\x6d\x5e\x15\x85\x5a\x64\x54\xa2\x08\x00\x86\x65\xa2\x08"
            "\x00\x86\x63\x20\xa5\x80\xa5\x54\x18\x69\x08\x00\x85\x54\xc6\x63\xd0\xf1\xa5\x54"
            "\x18\x69\xc0\x00\x85\x54\xc6\x65\xd0\xe0\xe2\x20\xa9\x20\x85\x74\x20\xc2\x93\x60", 60u))
        return 0;
    const uint32_t bank = (uint32_t)registers->db << 16;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x828069u: goto widen;
    case 0x82806bu: goto load_base_tile;
    case 0x82806eu: goto clear_tile_carry;
    case 0x82806fu: goto add_tile_offset;
    case 0x828072u: goto store_tile;
    case 0x828074u: goto clear_position;
    case 0x828076u: goto load_rows;
    case 0x828079u: goto store_rows;
    case 0x82807bu: goto load_columns;
    case 0x82807eu: goto store_columns;
    case 0x828080u: goto call_tile_block;
    case 0x828083u: goto load_position;
    case 0x828085u: goto clear_column_carry;
    case 0x828086u: goto advance_column;
    case 0x828089u: goto store_column;
    case 0x82808bu: goto count_columns;
    case 0x82808du: goto repeat_column;
    case 0x82808fu: goto load_row_position;
    case 0x828091u: goto clear_row_carry;
    case 0x828092u: goto advance_row;
    case 0x828095u: goto store_row;
    case 0x828097u: goto count_rows;
    case 0x828099u: goto repeat_row;
    case 0x82809bu: goto narrow;
    case 0x82809du: goto load_redraw;
    case 0x82809fu: goto store_redraw;
    case 0x8280a1u: goto wait_for_redraw;
    case 0x8280a4u: goto return_short;
    default: return 0;
    }
widen:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x828069u, 2u, span.instructions != 0u);
    registers->mf = false;
    NativeSpanStep(&span, registers, cpu, 3u, 0x806bu);
load_base_tile:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x82806bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0c10u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x806eu);
clear_tile_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x82806eu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x806fu);
add_tile_offset:
    if (!NativeSpanCanStep(&span, cpu, 5u) ||
        !Lufia2DecompIsWramByte(bank + 0x155eu) || !Lufia2DecompIsWramByte(bank + 0x155fu)) goto finished;
    Lufia2SpanFetch(registers, 0x82806fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, bank + 0x155eu, bank + 0x155fu, true), false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x8072u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
store_tile:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x828072u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x5au, 0x5bu, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u, 0x8074u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
clear_position:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x828074u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x54u, 0x55u, 0u, true);
    NativeSpanStep(&span, registers, cpu, 4u, 0x8076u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_rows:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x828076u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 8u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8079u);
store_rows:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x828079u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x65u, 0x66u, registers->x, true);
    NativeSpanStep(&span, registers, cpu, 4u, 0x807bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_columns:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x82807bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 8u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x807eu);
store_columns:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x82807eu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x63u, 0x64u, registers->x, true);
    NativeSpanStep(&span, registers, cpu, 4u, 0x8080u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
call_tile_block:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    {
        if (span.instructions) Lufia2SpanProbeOpcode(registers, 0x828080u);
        const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x828080u, 0x20u,
            0x8280a5u, false, false, false);
        if (!cycles) goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
load_position:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x828083u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, 0x54u, 0x55u, true), false);
    NativeSpanStep(&span, registers, cpu, 4u, 0x8085u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
clear_column_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x828085u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8086u);
advance_column:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x828086u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 8u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8089u);
store_column:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x828089u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x54u, 0x55u, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u, 0x808bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
count_columns:
    if (!NativeSpanCanStep(&span, cpu, 7u)) goto finished;
    Lufia2SpanFetch(registers, 0x82808bu, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x63u, 0x64u, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x808du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
repeat_column:
    {
        const bool repeat = !registers->z;
        const unsigned cycles = repeat ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x82808du, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, repeat ? 0x8080u : 0x808fu);
        if (repeat) goto call_tile_block;
    }
load_row_position:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x82808fu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, 0x54u, 0x55u, true), false);
    NativeSpanStep(&span, registers, cpu, 4u, 0x8091u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
clear_row_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x828091u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8092u);
advance_row:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x828092u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0xc0u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8095u);
store_row:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x828095u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x54u, 0x55u, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u, 0x8097u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
count_rows:
    if (!NativeSpanCanStep(&span, cpu, 7u)) goto finished;
    Lufia2SpanFetch(registers, 0x828097u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x65u, 0x66u, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x8099u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
repeat_row:
    {
        const bool repeat = !registers->z;
        const unsigned cycles = repeat ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x828099u, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, repeat ? 0x807bu : 0x809bu);
        if (repeat) goto load_columns;
    }
narrow:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x82809bu, 2u, span.instructions != 0u);
    registers->mf = true;
    NativeSpanStep(&span, registers, cpu, 3u, 0x809du);
load_redraw:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x82809du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x20u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x809fu);
store_redraw:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x82809fu, 2u, span.instructions != 0u);
    registers->write(registers->mem, 0x74u, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 3u, 0x80a1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
wait_for_redraw:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    {
        if (span.instructions) Lufia2SpanProbeOpcode(registers, 0x8280a1u);
        const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x8280a1u, 0x20u,
            0x8293c2u, false, true, false);
        if (!cycles) goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
return_short:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x8280a4u, 1u, span.instructions != 0u);
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
