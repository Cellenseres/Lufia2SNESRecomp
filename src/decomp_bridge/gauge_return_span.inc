#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuPointerReturnSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x808bb1u && pc < 0x808bb2u)
        end_pc = 0x808bb2u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x808bb1u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x808bb1u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 1u || memcmp(cart->rom + first, "\x60", 1u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x808bb1u: goto return_pointer;
    default: return 0;
    }
return_pointer: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808bb1u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x808bb1u, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunBattleGaugeBlockSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                               uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                               unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x81e5c1u && pc < 0x81e5e7u)
        end_pc = 0x81e5e7u;
    else if (pc >= 0x81e5e7u && pc < 0x81e5efu)
        end_pc = 0x81e5efu;
    else if (pc >= 0x81e5efu && pc < 0x81e603u)
        end_pc = 0x81e603u;
    else if (pc >= 0x81e603u && pc < 0x81e604u)
        end_pc = 0x81e604u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x81e603u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x81e5c1u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 67u ||
        memcmp(cart->rom + first,
               "\x18\x69\x05\x00\x9d\x00\x00\x1a\xe8\xe8\x9d\x00\x00\x38\xe9\x05\x00\xe8\xe8\xa0\x04\x00\x84"
               "\x11\x80\x09\x1a\x1a\x9d\x00\x00\xe8\xe8\x3a\x3a\xa0\x06\x00\x9d\x00\x00\xe8\xe8\x88\xd0\xf8"
               "\xc6\x11\xd0\xe8\x09\x00\x40\x18\x69\x05\x00\x9d\x00\x00\xe8\xe8\x3a\x9d\x00\x00\x60",
               67u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x81e5c1u: goto clear_corner_carry;
    case 0x81e5c2u: goto first_corner;
    case 0x81e5c5u: goto store_first_corner;
    case 0x81e5c8u: goto second_corner;
    case 0x81e5c9u: goto next_corner_byte;
    case 0x81e5cau: goto next_corner_word;
    case 0x81e5cbu: goto store_second_corner;
    case 0x81e5ceu: goto set_body_carry;
    case 0x81e5cfu: goto body_tile;
    case 0x81e5d2u: goto first_body_byte;
    case 0x81e5d3u: goto first_body_word;
    case 0x81e5d4u: goto row_count;
    case 0x81e5d7u: goto store_row_count;
    case 0x81e5d9u: goto first_body_row;
    case 0x81e5dbu: goto left_edge_first;
    case 0x81e5dcu: goto left_edge_second;
    case 0x81e5ddu: goto store_left_edge;
    case 0x81e5e0u: goto next_body_byte;
    case 0x81e5e1u: goto next_body_word;
    case 0x81e5e2u: goto restore_body_first;
    case 0x81e5e3u: goto restore_body_second;
    case 0x81e5e4u: goto column_count;
    case 0x81e5e7u: goto store_body_tile;
    case 0x81e5eau: goto next_cell_byte;
    case 0x81e5ebu: goto next_cell_word;
    case 0x81e5ecu: goto count_columns;
    case 0x81e5edu: goto repeat_columns;
    case 0x81e5efu: goto count_rows;
    case 0x81e5f1u: goto repeat_rows;
    case 0x81e5f3u: goto bottom_attributes;
    case 0x81e5f6u: goto clear_bottom_carry;
    case 0x81e5f7u: goto first_bottom_corner;
    case 0x81e5fau: goto store_first_bottom;
    case 0x81e5fdu: goto next_bottom_byte;
    case 0x81e5feu: goto next_bottom_word;
    case 0x81e5ffu: goto second_bottom_corner;
    case 0x81e600u: goto store_second_bottom;
    case 0x81e603u: goto return_block;
    default: return 0;
    }
clear_corner_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5c1u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5c2u);
}
first_corner: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5c2u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0005u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe5c5u);
}
store_first_corner: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5c5u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe5c8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
second_corner: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5c8u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a + 1u) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5c9u);
}
next_corner_byte: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5c9u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5cau);
}
next_corner_word: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5cau, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5cbu);
}
store_second_corner: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5cbu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe5ceu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_body_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5ceu, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5cfu);
}
body_tile: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5cfu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0005u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe5d2u);
}
first_body_byte: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5d2u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5d3u);
}
first_body_word: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5d3u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5d4u);
}
row_count: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5d4u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0004u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe5d7u);
}
store_row_count: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5d7u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->y, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe5d9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_body_row: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5d9u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe5e4u);
    goto finished;
}
left_edge_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5dbu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a + 1u) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5dcu);
}
left_edge_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5dcu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a + 1u) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5ddu);
}
store_left_edge: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5ddu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe5e0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_body_byte: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5e0u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5e1u);
}
next_body_word: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5e1u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5e2u);
}
restore_body_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5e2u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a - 1u) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5e3u);
}
restore_body_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5e3u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a - 1u) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5e4u);
}
column_count: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5e4u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0006u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe5e7u);
    goto finished;
}
store_body_tile: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5e7u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe5eau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_cell_byte: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5eau, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5ebu);
}
next_cell_word: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5ebu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5ecu);
}
count_columns: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5ecu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5edu);
}
repeat_columns: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5edu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xe5e7u : 0xe5efu);
    if (taken)
        goto finished;
    goto finished;
}
count_rows: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5efu, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe5f1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_rows: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5f1u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xe5dbu : 0xe5f3u);
    if (taken)
        goto finished;
}
bottom_attributes: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5f3u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0x4000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe5f6u);
}
clear_bottom_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5f6u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5f7u);
}
first_bottom_corner: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5f7u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0005u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe5fau);
}
store_first_bottom: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5fau, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe5fdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_bottom_byte: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5fdu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5feu);
}
next_bottom_word: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5feu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe5ffu);
}
second_bottom_corner: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e5ffu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a - 1u) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe600u);
}
store_second_bottom: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e600u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe603u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_block: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x81e603u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x81e603u, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunBattleGaugeColumnSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x81e604u && pc < 0x81e639u)
        end_pc = 0x81e639u;
    else if (pc >= 0x81e639u && pc < 0x81e63au)
        end_pc = 0x81e63au;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x81e639u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x81e604u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 54u ||
        memcmp(cart->rom + first,
               "\x18\x69\x05\x00\x9d\x00\x00\x1a\xe8\xe8\x9d\x00\x00\x38\xe9\x04\x00\xa8\xa9\x04\x00\x85\x11"
               "\x80\x03\x9d\x00\x00\x8a\x18\x69\x0e\x00\xaa\x98\xc6\x11\xd0\xf2\x18\x69\x04\x00\x09\x00\x40"
               "\x9d\x00\x00\x3a\x9d\x02\x00\x60",
               54u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x81e604u: goto clear_top_carry;
    case 0x81e605u: goto first_top_corner;
    case 0x81e608u: goto store_first_top;
    case 0x81e60bu: goto second_top_corner;
    case 0x81e60cu: goto next_top_byte;
    case 0x81e60du: goto next_top_word;
    case 0x81e60eu: goto store_second_top;
    case 0x81e611u: goto set_body_carry;
    case 0x81e612u: goto body_tile;
    case 0x81e615u: goto save_body_tile;
    case 0x81e616u: goto row_count;
    case 0x81e619u: goto store_row_count;
    case 0x81e61bu: goto first_body_row;
    case 0x81e61du: goto store_body_tile;
    case 0x81e620u: goto current_column_position;
    case 0x81e621u: goto clear_row_carry;
    case 0x81e622u: goto next_row_position;
    case 0x81e625u: goto select_next_row;
    case 0x81e626u: goto restore_body_tile;
    case 0x81e627u: goto count_rows;
    case 0x81e629u: goto repeat_rows;
    case 0x81e62bu: goto clear_bottom_carry;
    case 0x81e62cu: goto first_bottom_corner;
    case 0x81e62fu: goto bottom_attributes;
    case 0x81e632u: goto store_first_bottom;
    case 0x81e635u: goto second_bottom_corner;
    case 0x81e636u: goto store_second_bottom;
    case 0x81e639u: goto return_column;
    default: return 0;
    }
clear_top_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e604u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe605u);
}
first_top_corner: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e605u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0005u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe608u);
}
store_first_top: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e608u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe60bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
second_top_corner: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e60bu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a + 1u) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe60cu);
}
next_top_byte: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e60cu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe60du);
}
next_top_word: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e60du, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe60eu);
}
store_second_top: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e60eu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe611u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_body_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e611u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe612u);
}
body_tile: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e612u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0004u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe615u);
}
save_body_tile: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e615u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe616u);
}
row_count: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e616u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0004u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe619u);
}
store_row_count: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e619u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe61bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_body_row: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e61bu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe620u);
    goto finished;
}
store_body_tile: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e61du, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe620u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
current_column_position: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e620u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->x, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe621u);
}
clear_row_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e621u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe622u);
}
next_row_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e622u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x000eu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe625u);
}
select_next_row: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e625u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe626u);
}
restore_body_tile: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e626u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe627u);
}
count_rows: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e627u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe629u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_rows: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e629u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xe61du : 0xe62bu);
    if (taken)
        goto finished;
}
clear_bottom_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e62bu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe62cu);
}
first_bottom_corner: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e62cu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0004u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe62fu);
}
bottom_attributes: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e62fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0x4000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe632u);
}
store_first_bottom: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e632u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe635u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
second_bottom_corner: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e635u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a - 1u) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe636u);
}
store_second_bottom: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0002u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e636u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe639u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_column: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x81e639u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x81e639u, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
static int Lufia2DecompRunGaugeReturnSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
    uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget, long *step_count) {
    if (pc == 0x808bb1u)
        return Lufia2DecompRunMenuPointerReturnSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x81e5c1u && pc <= 0x81e603u)
        return Lufia2DecompRunBattleGaugeBlockSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x81e604u && pc <= 0x81e639u)
        return Lufia2DecompRunBattleGaugeColumnSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    return 0;
}
