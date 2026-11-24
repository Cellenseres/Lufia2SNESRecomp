#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuWindowParentSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                               uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                               unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x82810eu && pc < 0x82815eu)
        end_pc = 0x82815eu;
    else if (pc >= 0x82815eu && pc < 0x828161u)
        end_pc = 0x828161u;
    else if (pc >= 0x828161u && pc < 0x82816du)
        end_pc = 0x82816du;
    else if (pc >= 0x82816du && pc < 0x828170u)
        end_pc = 0x828170u;
    else if (pc >= 0x828170u && pc < 0x82817cu)
        end_pc = 0x82817cu;
    else if (pc >= 0x82817cu && pc < 0x82817fu)
        end_pc = 0x82817fu;
    else if (pc >= 0x82817fu && pc < 0x82818eu)
        end_pc = 0x82818eu;
    else if (pc >= 0x82818eu && pc < 0x828191u)
        end_pc = 0x828191u;
    else if (pc >= 0x828191u && pc < 0x8281a0u)
        end_pc = 0x8281a0u;
    else if (pc >= 0x8281a0u && pc < 0x8281a3u)
        end_pc = 0x8281a3u;
    else if (pc >= 0x8281a3u && pc < 0x8281abu)
        end_pc = 0x8281abu;
    else if (pc >= 0x8281abu && pc < 0x8281aeu)
        end_pc = 0x8281aeu;
    else if (pc >= 0x8281aeu && pc < 0x8281b9u)
        end_pc = 0x8281b9u;
    else if (pc >= 0x8281b9u && pc < 0x8281bcu)
        end_pc = 0x8281bcu;
    else if (pc >= 0x8281bcu && pc < 0x8281c7u)
        end_pc = 0x8281c7u;
    else if (pc >= 0x8281c7u && pc < 0x8281cau)
        end_pc = 0x8281cau;
    else if (pc >= 0x8281cau && pc < 0x8281d1u)
        end_pc = 0x8281d1u;
    else if (pc >= 0x8281d1u && pc < 0x8281d4u)
        end_pc = 0x8281d4u;
    else if (pc >= 0x8281d4u && pc < 0x8281e1u)
        end_pc = 0x8281e1u;
    else if (pc >= 0x8281e1u && pc < 0x8281e4u)
        end_pc = 0x8281e4u;
    else if (pc >= 0x8281e4u && pc < 0x8281e5u)
        end_pc = 0x8281e5u;
    else if (pc >= 0x8281e5u && pc < 0x8281e6u)
        end_pc = 0x8281e6u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x828115u: width_neutral = true; break;
    case 0x828117u: byte_accumulator = true; break;
    case 0x828119u: byte_accumulator = true; break;
    case 0x82811au: byte_accumulator = true; break;
    case 0x82811cu: width_neutral = true; break;
    case 0x82811eu: width_neutral = true; break;
    case 0x82811fu: width_neutral = true; break;
    case 0x828121u: byte_accumulator = true; break;
    case 0x828123u: byte_accumulator = true; break;
    case 0x828124u: width_neutral = true; break;
    case 0x828125u: width_neutral = true; break;
    case 0x8281e4u: width_neutral = true; break;
    case 0x8281e5u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x82810eu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 216u ||
        memcmp(cart->rom + first,
               "\x85\x54\x64\x56\x64\x58\x8a\xe2\x20\x85\x58\xeb\x85\x56\xc2\x20\x8b\xe2\x20\xa9\x82\x48\xab"
               "\xc2\x20\xa5\x54\x18\x69\x42\x00\x85\x63\xa5\x58\x3a\x3a\x85\x5a\xa4\x56\x88\x88\xa9\x0f\x08"
               "\xa6\x63\x9f\x00\x20\x7e\xe8\xe8\x88\xd0\xf7\xa5\x63\x18\x69\x40\x00\x85\x63\xc6\x5a\xd0\xe2"
               "\xa0\x00\x00\xa5\x54\x18\x69\x04\x00\x85\x63\x20\xe6\x81\xa0\x02\x00\xa5\x58\x3a\xeb\x4a\x4a"
               "\x18\x65\x63\x20\xe6\x81\xa5\x58\xc9\x03\x00\xf0\x55\xa0\x04\x00\xa5\x54\x20\x0b\x82\xa0\x06"
               "\x00\xa5\x56\x0a\x38\xe9\x04\x00\x85\x5a\x18\x65\x54\x20\x0b\x82\xa0\x08\x00\xa5\x58\x3a\x3a"
               "\xeb\x4a\x4a\x18\x65\x54\x85\x63\x20\x0b\x82\xa0\x0a\x00\xa5\x5a\x18\x65\x63\x20\x0b\x82\xa0"
               "\x0c\x00\xa5\x54\x18\x69\x80\x00\x85\x63\x20\x30\x82\xa0\x0e\x00\xa5\x56\x0a\x18\x65\x63\x3a"
               "\x3a\x20\x30\x82\x80\x18\xa0\x10\x00\xa5\x54\x20\x59\x82\xa0\x12\x00\xa5\x56\x0a\x38\xe9\x04"
               "\x00\x18\x65\x54\x20\x59\x82\xab\x60",
               216u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x82810eu: goto store_window_position;
    case 0x828110u: goto clear_window_width;
    case 0x828112u: goto clear_window_height;
    case 0x828114u: goto read_packed_dimensions;
    case 0x828115u: goto narrow_dimensions;
    case 0x828117u: goto store_window_height;
    case 0x828119u: goto select_width_byte;
    case 0x82811au: goto store_window_width;
    case 0x82811cu: goto widen_window_position;
    case 0x82811eu: goto save_caller_bank;
    case 0x82811fu: goto narrow_window_bank;
    case 0x828121u: goto window_data_bank;
    case 0x828123u: goto push_window_bank;
    case 0x828124u: goto select_window_bank;
    case 0x828125u: goto widen_interior_position;
    case 0x828127u: goto read_interior_position;
    case 0x828129u: goto clear_interior_carry;
    case 0x82812au: goto offset_interior_position;
    case 0x82812du: goto store_interior_position;
    case 0x82812fu: goto read_interior_height;
    case 0x828131u: goto exclude_top_row;
    case 0x828132u: goto exclude_bottom_row;
    case 0x828133u: goto store_interior_height;
    case 0x828135u: goto read_interior_width;
    case 0x828137u: goto exclude_left_column;
    case 0x828138u: goto exclude_right_column;
    case 0x828139u: goto interior_tile;
    case 0x82813cu: goto select_interior_row;
    case 0x82813eu: goto store_interior_tile;
    case 0x828142u: goto next_tile_low_byte;
    case 0x828143u: goto next_tile_high_byte;
    case 0x828144u: goto count_interior_tile;
    case 0x828145u: goto repeat_interior_tiles;
    case 0x828147u: goto read_next_row_position;
    case 0x828149u: goto clear_next_row_carry;
    case 0x82814au: goto offset_next_row;
    case 0x82814du: goto store_next_row_position;
    case 0x82814fu: goto count_interior_row;
    case 0x828151u: goto repeat_interior_rows;
    case 0x828153u: goto select_top_edge;
    case 0x828156u: goto read_top_edge_position;
    case 0x828158u: goto clear_top_edge_carry;
    case 0x828159u: goto offset_top_edge;
    case 0x82815cu: goto store_top_edge_position;
    case 0x82815eu: goto draw_top_edge;
    case 0x828161u: goto select_bottom_edge;
    case 0x828164u: goto read_bottom_row;
    case 0x828166u: goto exclude_bottom_corner;
    case 0x828167u: goto scale_bottom_row;
    case 0x828168u: goto scale_bottom_row_first;
    case 0x828169u: goto scale_bottom_row_second;
    case 0x82816au: goto clear_bottom_edge_carry;
    case 0x82816bu: goto offset_bottom_edge;
    case 0x82816du: goto draw_bottom_edge;
    case 0x828170u: goto read_corner_height;
    case 0x828172u: goto test_three_row_window;
    case 0x828175u: goto choose_three_row_corners;
    case 0x828177u: goto select_top_left_corner;
    case 0x82817au: goto read_top_left_position;
    case 0x82817cu: goto draw_top_left_corner;
    case 0x82817fu: goto select_top_right_corner;
    case 0x828182u: goto read_right_column;
    case 0x828184u: goto scale_right_column;
    case 0x828185u: goto set_right_offset_carry;
    case 0x828186u: goto exclude_right_corners;
    case 0x828189u: goto store_right_offset;
    case 0x82818bu: goto clear_top_right_carry;
    case 0x82818cu: goto offset_top_right_corner;
    case 0x82818eu: goto draw_top_right_corner;
    case 0x828191u: goto select_bottom_left_corner;
    case 0x828194u: goto read_bottom_corner_row;
    case 0x828196u: goto exclude_top_corner_row;
    case 0x828197u: goto exclude_bottom_corner_row;
    case 0x828198u: goto scale_corner_row;
    case 0x828199u: goto scale_corner_row_first;
    case 0x82819au: goto scale_corner_row_second;
    case 0x82819bu: goto clear_bottom_left_carry;
    case 0x82819cu: goto offset_bottom_left_corner;
    case 0x82819eu: goto store_bottom_left_position;
    case 0x8281a0u: goto draw_bottom_left_corner;
    case 0x8281a3u: goto select_bottom_right_corner;
    case 0x8281a6u: goto read_bottom_right_column;
    case 0x8281a8u: goto clear_bottom_right_carry;
    case 0x8281a9u: goto offset_bottom_right_corner;
    case 0x8281abu: goto draw_bottom_right_corner;
    case 0x8281aeu: goto select_left_side;
    case 0x8281b1u: goto read_left_side_position;
    case 0x8281b3u: goto clear_left_side_carry;
    case 0x8281b4u: goto offset_left_side;
    case 0x8281b7u: goto store_left_side_position;
    case 0x8281b9u: goto draw_left_side;
    case 0x8281bcu: goto select_right_side;
    case 0x8281bfu: goto read_right_side_column;
    case 0x8281c1u: goto scale_right_side_column;
    case 0x8281c2u: goto clear_right_side_carry;
    case 0x8281c3u: goto offset_right_side;
    case 0x8281c5u: goto exclude_left_side_tile;
    case 0x8281c6u: goto exclude_right_side_tile;
    case 0x8281c7u: goto draw_right_side;
    case 0x8281cau: goto finish_tall_window;
    case 0x8281ccu: goto select_short_left_corner;
    case 0x8281cfu: goto read_short_left_position;
    case 0x8281d1u: goto draw_short_left_corner;
    case 0x8281d4u: goto select_short_right_corner;
    case 0x8281d7u: goto read_short_right_column;
    case 0x8281d9u: goto scale_short_right_column;
    case 0x8281dau: goto set_short_right_carry;
    case 0x8281dbu: goto exclude_short_right_corners;
    case 0x8281deu: goto clear_short_right_carry;
    case 0x8281dfu: goto offset_short_right_corner;
    case 0x8281e1u: goto draw_short_right_corner;
    case 0x8281e4u: goto restore_caller_bank;
    case 0x8281e5u: goto return_window;
    default: return 0;
    }
store_window_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82810eu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8110u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_window_width: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828110u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8112u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_window_height: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828112u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8114u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_packed_dimensions: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828114u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->x, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8115u);
}
narrow_dimensions: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828115u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8117u);
}
store_window_height: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828117u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8119u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_width_byte: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828119u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    NativeSpanStep(&span, registers, cpu, 3u, 0x811au);
}
store_window_width: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82811au, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x811cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_window_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82811cu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x811eu);
}
save_caller_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82811eu, 1u, span.instructions != 0u);
    const uint16_t value = registers->db;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x811fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_window_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82811fu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8121u);
}
window_data_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828121u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0082u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8123u);
}
push_window_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x828123u, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8124u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_window_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x828124u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x8125u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_interior_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828125u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8127u);
}
read_interior_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828127u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8129u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_interior_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828129u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x812au);
}
offset_interior_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82812au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0042u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x812du);
}
store_interior_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x63u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82812du, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x812fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_interior_height: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82812fu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8131u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
exclude_top_row: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828131u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8132u);
}
exclude_bottom_row: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828132u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8133u);
}
store_interior_height: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5au);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828133u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8135u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_interior_width: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828135u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8137u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
exclude_left_column: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828137u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8138u);
}
exclude_right_column: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828138u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8139u);
}
interior_tile: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828139u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x080fu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x813cu);
}
select_interior_row: {
    const uint32_t address = (uint16_t)(registers->dp + 0x63u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82813cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x813eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_interior_tile: {
    const uint32_t address = (0x7e2000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82813eu, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8142u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_low_byte: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828142u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8143u);
}
next_tile_high_byte: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828143u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8144u);
}
count_interior_tile: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828144u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8145u);
}
repeat_interior_tiles: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828145u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x813eu : 0x8147u);
    if (taken)
        goto finished;
}
read_next_row_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x63u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828147u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8149u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_next_row_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828149u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x814au);
}
offset_next_row: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82814au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0040u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x814du);
}
store_next_row_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x63u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82814du, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x814fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
count_interior_row: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5au);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82814fu, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8151u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_interior_rows: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828151u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8135u : 0x8153u);
    if (taken)
        goto finished;
}
select_top_edge: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828153u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8156u);
}
read_top_edge_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828156u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8158u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_top_edge_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828158u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8159u);
}
offset_top_edge: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828159u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0004u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x815cu);
}
store_top_edge_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x63u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82815cu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x815eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_top_edge: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82815eu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x82815eu, 0x20u, 0x8281e6u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_bottom_edge: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828161u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0002u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8164u);
}
read_bottom_row: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828164u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8166u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
exclude_bottom_corner: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828166u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8167u);
}
scale_bottom_row: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828167u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8168u);
}
scale_bottom_row_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828168u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a & 0xffffu) >> 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8169u);
}
scale_bottom_row_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828169u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a & 0xffffu) >> 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x816au);
}
clear_bottom_edge_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82816au, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x816bu);
}
offset_bottom_edge: {
    const uint32_t address = (uint16_t)(registers->dp + 0x63u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82816bu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x816du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_bottom_edge: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82816du);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x82816du, 0x20u, 0x8281e6u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_corner_height: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828170u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8172u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_three_row_window: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828172u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0003u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8175u);
}
choose_three_row_corners: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828175u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x81ccu : 0x8177u);
    if (taken)
        goto finished;
}
select_top_left_corner: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828177u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0004u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x817au);
}
read_top_left_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82817au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x817cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_top_left_corner: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82817cu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x82817cu, 0x20u, 0x82820bu, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_top_right_corner: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82817fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0006u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8182u);
}
read_right_column: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828182u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8184u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
scale_right_column: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828184u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8185u);
}
set_right_offset_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828185u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8186u);
}
exclude_right_corners: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828186u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0004u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8189u);
}
store_right_offset: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5au);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828189u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x818bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_top_right_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82818bu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x818cu);
}
offset_top_right_corner: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82818cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x818eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_top_right_corner: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82818eu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x82818eu, 0x20u, 0x82820bu, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_bottom_left_corner: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828191u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0008u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8194u);
}
read_bottom_corner_row: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828194u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8196u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
exclude_top_corner_row: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828196u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8197u);
}
exclude_bottom_corner_row: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828197u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8198u);
}
scale_corner_row: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828198u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8199u);
}
scale_corner_row_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828199u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a & 0xffffu) >> 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x819au);
}
scale_corner_row_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82819au, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a & 0xffffu) >> 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x819bu);
}
clear_bottom_left_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82819bu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x819cu);
}
offset_bottom_left_corner: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82819cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x819eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_bottom_left_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x63u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82819eu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81a0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_bottom_left_corner: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8281a0u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8281a0u, 0x20u, 0x82820bu, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_bottom_right_corner: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281a3u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x000au, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x81a6u);
}
read_bottom_right_column: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5au);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281a6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81a8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_bottom_right_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281a8u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x81a9u);
}
offset_bottom_right_corner: {
    const uint32_t address = (uint16_t)(registers->dp + 0x63u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281a9u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81abu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_bottom_right_corner: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8281abu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8281abu, 0x20u, 0x82820bu, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_left_side: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281aeu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x000cu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x81b1u);
}
read_left_side_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281b1u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81b3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_left_side_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281b3u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x81b4u);
}
offset_left_side: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281b4u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0080u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x81b7u);
}
store_left_side_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x63u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281b7u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81b9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_left_side: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8281b9u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8281b9u, 0x20u, 0x828230u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_right_side: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281bcu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x000eu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x81bfu);
}
read_right_side_column: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281bfu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81c1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
scale_right_side_column: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281c1u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x81c2u);
}
clear_right_side_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281c2u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x81c3u);
}
offset_right_side: {
    const uint32_t address = (uint16_t)(registers->dp + 0x63u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281c3u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81c5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
exclude_left_side_tile: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281c5u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x81c6u);
}
exclude_right_side_tile: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281c6u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x81c7u);
    goto finished;
}
draw_right_side: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8281c7u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8281c7u, 0x20u, 0x828230u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
finish_tall_window: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281cau, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x81e4u);
    goto finished;
}
select_short_left_corner: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281ccu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0010u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x81cfu);
}
read_short_left_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281cfu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81d1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_short_left_corner: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8281d1u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8281d1u, 0x20u, 0x828259u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_short_right_corner: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281d4u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0012u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x81d7u);
}
read_short_right_column: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281d7u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81d9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
scale_short_right_column: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281d9u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x81dau);
}
set_short_right_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281dau, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0x81dbu);
}
exclude_short_right_corners: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281dbu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0004u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x81deu);
}
clear_short_right_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281deu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x81dfu);
}
offset_short_right_corner: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8281dfu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81e1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_short_right_corner: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8281e1u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8281e1u, 0x20u, 0x828259u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_caller_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x8281e4u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x81e5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_window: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8281e5u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x8281e5u, 0x60u, false, registers->mf, true);
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
