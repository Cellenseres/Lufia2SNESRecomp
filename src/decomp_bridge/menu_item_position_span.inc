#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuItemPositionSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                               uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                               unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x8288cbu && pc < 0x8288ddu)
        end_pc = 0x8288ddu;
    else if (pc >= 0x8288ddu && pc < 0x8288e1u)
        end_pc = 0x8288e1u;
    else if (pc >= 0x8288e1u && pc < 0x828906u)
        end_pc = 0x828906u;
    else if (pc >= 0x828906u && pc < 0x82890au)
        end_pc = 0x82890au;
    else if (pc >= 0x82890au && pc < 0x82891du)
        end_pc = 0x82891du;
    else if (pc >= 0x82891du && pc < 0x82891eu)
        end_pc = 0x82891eu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x8288cbu: byte_accumulator = true; break;
    case 0x8288ceu: byte_accumulator = true; break;
    case 0x8288d1u: byte_accumulator = true; break;
    case 0x8288d4u: byte_accumulator = true; break;
    case 0x8288d7u: byte_accumulator = true; break;
    case 0x8288dau: byte_accumulator = true; break;
    case 0x8288ddu: byte_accumulator = true; break;
    case 0x8288e1u: byte_accumulator = true; break;
    case 0x8288e4u: byte_accumulator = true; break;
    case 0x8288e5u: byte_accumulator = true; break;
    case 0x8288e8u: byte_accumulator = true; break;
    case 0x8288ebu: byte_accumulator = true; break;
    case 0x8288eeu: byte_accumulator = true; break;
    case 0x8288f1u: byte_accumulator = true; break;
    case 0x8288f4u: byte_accumulator = true; break;
    case 0x8288f7u: byte_accumulator = true; break;
    case 0x8288fau: byte_accumulator = true; break;
    case 0x8288fdu: byte_accumulator = true; break;
    case 0x828900u: byte_accumulator = true; break;
    case 0x828903u: byte_accumulator = true; break;
    case 0x828906u: byte_accumulator = true; break;
    case 0x82890au: byte_accumulator = true; break;
    case 0x82890du: byte_accumulator = true; break;
    case 0x82890eu: byte_accumulator = true; break;
    case 0x828911u: byte_accumulator = true; break;
    case 0x828914u: byte_accumulator = true; break;
    case 0x828917u: byte_accumulator = true; break;
    case 0x82891au: byte_accumulator = true; break;
    case 0x82891du: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x8288cbu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 83u ||
        memcmp(cart->rom + first,
               "\xbd\xfd\x14\x8d\x70\x15\x9c\x71\x15\xbd\xd9\x14\x8d\x72\x15\x9c\x73\x15\x22\x00\x80\x82\xad"
               "\x74\x15\x18\x7d\xcd\x14\x9d\x8d\x13\xad\x75\x15\x7d\xd3\x14\x9d\xbd\x13\xbd\x03\x15\x8d\x70"
               "\x15\x9c\x71\x15\xbd\xeb\x14\x8d\x72\x15\x9c\x73\x15\x22\x00\x80\x82\xad\x74\x15\x18\x7d\xdf"
               "\x14\x9d\xed\x13\xad\x75\x15\x7d\xe5\x14\x9d\x1d\x14\x60",
               83u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x8288cbu: goto read_cell_width;
    case 0x8288ceu: goto store_width_factor;
    case 0x8288d1u: goto clear_width_factor_high;
    case 0x8288d4u: goto read_column_count;
    case 0x8288d7u: goto store_column_factor;
    case 0x8288dau: goto clear_column_factor_high;
    case 0x8288ddu: goto multiply_horizontal_position;
    case 0x8288e1u: goto read_horizontal_product_low;
    case 0x8288e4u: goto clear_horizontal_carry;
    case 0x8288e5u: goto add_horizontal_origin_low;
    case 0x8288e8u: goto store_horizontal_position_low;
    case 0x8288ebu: goto read_horizontal_product_high;
    case 0x8288eeu: goto add_horizontal_origin_high;
    case 0x8288f1u: goto store_horizontal_position_high;
    case 0x8288f4u: goto read_cell_height;
    case 0x8288f7u: goto store_height_factor;
    case 0x8288fau: goto clear_height_factor_high;
    case 0x8288fdu: goto read_row_count;
    case 0x828900u: goto store_row_factor;
    case 0x828903u: goto clear_row_factor_high;
    case 0x828906u: goto multiply_vertical_position;
    case 0x82890au: goto read_vertical_product_low;
    case 0x82890du: goto clear_vertical_carry;
    case 0x82890eu: goto add_vertical_origin_low;
    case 0x828911u: goto store_vertical_position_low;
    case 0x828914u: goto read_vertical_product_high;
    case 0x828917u: goto add_vertical_origin_high;
    case 0x82891au: goto store_vertical_position_high;
    case 0x82891du: goto return_item_position;
    default: return 0;
    }
read_cell_width: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14fdu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14fdu >> 8) != ((0x14fdu + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288cbu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88ceu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_width_factor: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1570u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288ceu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88d1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_width_factor_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1571u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288d1u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88d4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_column_count: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14d9u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14d9u >> 8) != ((0x14d9u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288d4u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88d7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_column_factor: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1572u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288d7u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88dau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_column_factor_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1573u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288dau, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88ddu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
multiply_horizontal_position: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8288ddu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8288ddu, 0x22u, 0x828000u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_horizontal_product_low: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1574u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288e1u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88e4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_horizontal_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288e4u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x88e5u);
}
add_horizontal_origin_low: {
    const uint32_t address = ((((uint32_t)registers->db << 16) | 0x14cdu) + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14cdu >> 8) != ((0x14cdu + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288e5u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88e8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_horizontal_position_low: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x138du + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288e8u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88ebu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_horizontal_product_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1575u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288ebu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88eeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
add_horizontal_origin_high: {
    const uint32_t address = ((((uint32_t)registers->db << 16) | 0x14d3u) + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14d3u >> 8) != ((0x14d3u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288eeu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88f1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_horizontal_position_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13bdu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288f1u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88f4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_cell_height: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1503u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x1503u >> 8) != ((0x1503u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288f4u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88f7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_height_factor: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1570u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288f7u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88fau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_height_factor_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1571u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288fau, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88fdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_row_count: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14ebu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14ebu >> 8) != ((0x14ebu + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288fdu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8900u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_row_factor: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1572u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828900u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8903u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_row_factor_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1573u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828903u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8906u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
multiply_vertical_position: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828906u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828906u, 0x22u, 0x828000u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_vertical_product_low: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1574u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82890au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x890du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_vertical_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82890du, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x890eu);
}
add_vertical_origin_low: {
    const uint32_t address = ((((uint32_t)registers->db << 16) | 0x14dfu) + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14dfu >> 8) != ((0x14dfu + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82890eu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8911u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_vertical_position_low: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13edu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828911u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8914u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_vertical_product_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1575u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828914u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8917u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
add_vertical_origin_high: {
    const uint32_t address = ((((uint32_t)registers->db << 16) | 0x14e5u) + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14e5u >> 8) != ((0x14e5u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828917u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x891au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_vertical_position_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x141du + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82891au, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x891du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_item_position: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82891du);
    const int cycles = Lufia2DecompRunReturn(registers, 0x82891du, 0x60u, false, registers->mf, true);
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
