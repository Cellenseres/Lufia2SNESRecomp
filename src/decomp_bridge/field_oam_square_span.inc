#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunFieldOamSquareSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                             uint64_t dynamic_epoch, uint64_t write_epoch,
                                             unsigned step_budget, long *step_count) {
    uint32_t first_pc, end_pc;
    if (pc >= 0x83a534u && pc < 0x83a553u) {
        first_pc = 0x83a534u;
        end_pc = 0x83a553u;
    } else if (pc >= 0x83a553u && pc < 0x83a556u) {
        first_pc = 0x83a553u;
        end_pc = 0x83a556u;
    } else if (pc >= 0x83a556u && pc < 0x83a563u) {
        first_pc = 0x83a556u;
        end_pc = 0x83a563u;
    } else if (pc >= 0x83a563u && pc < 0x83a566u) {
        first_pc = 0x83a563u;
        end_pc = 0x83a566u;
    } else if (pc >= 0x83a566u && pc < 0x83a568u) {
        first_pc = 0x83a566u;
        end_pc = 0x83a568u;
    } else if (pc >= 0x83a568u && pc < 0x83a56bu) {
        first_pc = 0x83a568u;
        end_pc = 0x83a56bu;
    } else if (pc >= 0x83a56bu && pc < 0x83a56cu) {
        first_pc = 0x83a56bu;
        end_pc = 0x83a56cu;
    } else if (pc >= 0x83a56cu && pc < 0x83a573u) {
        first_pc = 0x83a56cu;
        end_pc = 0x83a573u;
    } else if (pc >= 0x83a573u && pc < 0x83a576u) {
        first_pc = 0x83a573u;
        end_pc = 0x83a576u;
    } else if (pc >= 0x83a576u && pc < 0x83a579u) {
        first_pc = 0x83a576u;
        end_pc = 0x83a579u;
    } else if (pc >= 0x83a579u && pc < 0x83a57cu) {
        first_pc = 0x83a579u;
        end_pc = 0x83a57cu;
    } else if (pc >= 0x83a57cu && pc < 0x83a57fu) {
        first_pc = 0x83a57cu;
        end_pc = 0x83a57fu;
    } else if (pc >= 0x83a57fu && pc < 0x83a580u) {
        first_pc = 0x83a57fu;
        end_pc = 0x83a580u;
    } else if (pc >= 0x83a580u && pc < 0x83a583u) {
        first_pc = 0x83a580u;
        end_pc = 0x83a583u;
    } else if (pc >= 0x83a583u && pc < 0x83a585u) {
        first_pc = 0x83a583u;
        end_pc = 0x83a585u;
    } else if (pc >= 0x83a585u && pc < 0x83a588u) {
        first_pc = 0x83a585u;
        end_pc = 0x83a588u;
    } else if (pc >= 0x83a588u && pc < 0x83a589u) {
        first_pc = 0x83a588u;
        end_pc = 0x83a589u;
    } else if (pc >= 0x83a591u && pc < 0x83a5c7u) {
        first_pc = 0x83a591u;
        end_pc = 0x83a5c7u;
    } else if (pc >= 0x83a5c7u && pc < 0x83a5fdu) {
        first_pc = 0x83a5c7u;
        end_pc = 0x83a5fdu;
    } else if (pc >= 0x83a5fdu && pc < 0x83a633u) {
        first_pc = 0x83a5fdu;
        end_pc = 0x83a633u;
    } else if (pc >= 0x83a633u && pc < 0x83a669u) {
        first_pc = 0x83a633u;
        end_pc = 0x83a669u;
    } else
        return 0;
    const bool returning = pc == 0x83a56bu || pc == 0x83a57fu || pc == 0x83a588u || pc == 0x83a5c6u ||
                           pc == 0x83a5fcu || pc == 0x83a632u || pc == 0x83a668u;
    const bool changes_accumulator = pc == 0x83a544u || pc == 0x83a549u;
    const bool byte_accumulator = !(pc >= 0x83a546u && pc <= 0x83a549u);
    if (!cpu || !cpu->ram || registers->e || !step_budget || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        (!returning && (registers->xf || (!changes_accumulator && registers->mf != byte_accumulator))) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x83a534u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 309u ||
        memcmp(
            cart->rom + first,
            "\xbb\x7b\xa5\x97\x9d\x03\x00\x9d\x07\x00\x9d\x0b\x00\x9d\x0f\x00\xc2\x20\x0a\x0a\x0a\xe2\x20\xa9"
            "\x00\xeb\x29\x06\xaa\xa5\x9d\xfc\x89\xa5\xa5\x90\x30\x12\xa5\x8f\x18\x69\x10\x90\x27\xe6\x58\x20"
            "\x69\xa6\xe6\x58\x20\x69\xa6\x60\xa5\x8f\x18\x69\x10\xb0\x0d\x20\x69\xa6\x20\x69\xa6\x20\x69\xa6"
            "\x20\x69\xa6\x60\x20\x69\xa6\xe6\x58\x20\x69\xa6\x60\x91\xa5\xc7\xa5\xfd\xa5\x33\xa6\xbb\x9d\x02"
            "\x00\x1a\x1a\x9d\x06\x00\x1a\x1a\x9d\x0a\x00\x1a\x1a\x9d\x0e\x00\xa5\x91\x9d\x01\x00\x9d\x05\x00"
            "\x18\x69\x10\x9d\x09\x00\x9d\x0d\x00\xa5\x8f\x9d\x00\x00\x9d\x08\x00\x18\x69\x10\x9d\x04\x00\x9d"
            "\x0c\x00\x60\xbb\x9d\x06\x00\x1a\x1a\x9d\x02\x00\x1a\x1a\x9d\x0e\x00\x1a\x1a\x9d\x0a\x00\xa5\x91"
            "\x9d\x01\x00\x9d\x05\x00\x18\x69\x10\x9d\x09\x00\x9d\x0d\x00\xa5\x8f\x9d\x00\x00\x9d\x08\x00\x18"
            "\x69\x10\x9d\x04\x00\x9d\x0c\x00\x60\xbb\x9d\x02\x00\x1a\x1a\x9d\x06\x00\x1a\x1a\x9d\x0a\x00\x1a"
            "\x1a\x9d\x0e\x00\xa5\x91\x9d\x09\x00\x9d\x0d\x00\x18\x69\x10\x9d\x01\x00\x9d\x05\x00\xa5\x8f\x9d"
            "\x00\x00\x9d\x08\x00\x18\x69\x10\x9d\x04\x00\x9d\x0c\x00\x60\xbb\x9d\x06\x00\x1a\x1a\x9d\x02\x00"
            "\x1a\x1a\x9d\x0e\x00\x1a\x1a\x9d\x0a\x00\xa5\x91\x9d\x09\x00\x9d\x0d\x00\x18\x69\x10\x9d\x01\x00"
            "\x9d\x05\x00\xa5\x8f\x9d\x00\x00\x9d\x08\x00\x18\x69\x10\x9d\x04\x00\x9d\x0c\x00\x60",
            309u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83a534u: goto square_index_output;
    case 0x83a535u: goto square_seed_attribute_high;
    case 0x83a536u: goto square_load_attributes;
    case 0x83a538u: goto square_store_first_attributes;
    case 0x83a53bu: goto square_store_second_attributes;
    case 0x83a53eu: goto square_store_third_attributes;
    case 0x83a541u: goto square_store_fourth_attributes;
    case 0x83a544u: goto square_widen_flip;
    case 0x83a546u: goto square_shift_flip_first;
    case 0x83a547u: goto square_shift_flip_second;
    case 0x83a548u: goto square_shift_flip_third;
    case 0x83a549u: goto square_narrow_flip;
    case 0x83a54bu: goto square_clear_flip_low;
    case 0x83a54du: goto square_exchange_flip;
    case 0x83a54eu: goto square_mask_flip;
    case 0x83a550u: goto square_index_flip;
    case 0x83a551u: goto square_load_tile;
    case 0x83a553u: goto square_draw_quad;
    case 0x83a556u: goto square_load_high_x;
    case 0x83a558u: goto square_test_high_x;
    case 0x83a55au: goto square_load_left_x;
    case 0x83a55cu: goto square_clear_right_carry;
    case 0x83a55du: goto square_add_right_width;
    case 0x83a55fu: goto square_right_is_visible;
    case 0x83a561u: goto square_skip_first_bit;
    case 0x83a563u: goto square_set_second_bit;
    case 0x83a566u: goto square_skip_third_bit;
    case 0x83a568u: goto square_set_fourth_bit;
    case 0x83a56bu: goto square_return_right_overflow;
    case 0x83a56cu: goto square_load_hidden_left_x;
    case 0x83a56eu: goto square_clear_hidden_carry;
    case 0x83a56fu: goto square_add_hidden_width;
    case 0x83a571u: goto square_left_is_hidden;
    case 0x83a573u: goto square_set_first_hidden_bit;
    case 0x83a576u: goto square_set_second_hidden_bit;
    case 0x83a579u: goto square_set_third_hidden_bit;
    case 0x83a57cu: goto square_set_fourth_hidden_bit;
    case 0x83a57fu: goto square_return_hidden_pair;
    case 0x83a580u: goto square_set_first_bit;
    case 0x83a583u: goto square_skip_second_bit;
    case 0x83a585u: goto square_set_third_bit;
    case 0x83a588u: goto square_return_square;
    case 0x83a591u: goto quad_0_index_output;
    case 0x83a592u: goto quad_0_store_first_tile;
    case 0x83a595u: goto quad_0_next_tile_first;
    case 0x83a596u: goto quad_0_next_tile_second;
    case 0x83a597u: goto quad_0_store_second_tile;
    case 0x83a59au: goto quad_0_next_tile_third;
    case 0x83a59bu: goto quad_0_next_tile_fourth;
    case 0x83a59cu: goto quad_0_store_third_tile;
    case 0x83a59fu: goto quad_0_next_tile_fifth;
    case 0x83a5a0u: goto quad_0_next_tile_sixth;
    case 0x83a5a1u: goto quad_0_store_fourth_tile;
    case 0x83a5a4u: goto quad_0_load_top_y;
    case 0x83a5a6u: goto quad_0_store_first_y;
    case 0x83a5a9u: goto quad_0_store_second_y;
    case 0x83a5acu: goto quad_0_clear_bottom_carry;
    case 0x83a5adu: goto quad_0_add_height;
    case 0x83a5afu: goto quad_0_store_third_y;
    case 0x83a5b2u: goto quad_0_store_fourth_y;
    case 0x83a5b5u: goto quad_0_load_left_x;
    case 0x83a5b7u: goto quad_0_store_first_x;
    case 0x83a5bau: goto quad_0_store_third_x;
    case 0x83a5bdu: goto quad_0_clear_right_carry;
    case 0x83a5beu: goto quad_0_add_width;
    case 0x83a5c0u: goto quad_0_store_second_x;
    case 0x83a5c3u: goto quad_0_store_fourth_x;
    case 0x83a5c6u: goto quad_0_return_quad;
    case 0x83a5c7u: goto quad_1_index_output;
    case 0x83a5c8u: goto quad_1_store_first_tile;
    case 0x83a5cbu: goto quad_1_next_tile_first;
    case 0x83a5ccu: goto quad_1_next_tile_second;
    case 0x83a5cdu: goto quad_1_store_second_tile;
    case 0x83a5d0u: goto quad_1_next_tile_third;
    case 0x83a5d1u: goto quad_1_next_tile_fourth;
    case 0x83a5d2u: goto quad_1_store_third_tile;
    case 0x83a5d5u: goto quad_1_next_tile_fifth;
    case 0x83a5d6u: goto quad_1_next_tile_sixth;
    case 0x83a5d7u: goto quad_1_store_fourth_tile;
    case 0x83a5dau: goto quad_1_load_top_y;
    case 0x83a5dcu: goto quad_1_store_first_y;
    case 0x83a5dfu: goto quad_1_store_second_y;
    case 0x83a5e2u: goto quad_1_clear_bottom_carry;
    case 0x83a5e3u: goto quad_1_add_height;
    case 0x83a5e5u: goto quad_1_store_third_y;
    case 0x83a5e8u: goto quad_1_store_fourth_y;
    case 0x83a5ebu: goto quad_1_load_left_x;
    case 0x83a5edu: goto quad_1_store_first_x;
    case 0x83a5f0u: goto quad_1_store_third_x;
    case 0x83a5f3u: goto quad_1_clear_right_carry;
    case 0x83a5f4u: goto quad_1_add_width;
    case 0x83a5f6u: goto quad_1_store_second_x;
    case 0x83a5f9u: goto quad_1_store_fourth_x;
    case 0x83a5fcu: goto quad_1_return_quad;
    case 0x83a5fdu: goto quad_2_index_output;
    case 0x83a5feu: goto quad_2_store_first_tile;
    case 0x83a601u: goto quad_2_next_tile_first;
    case 0x83a602u: goto quad_2_next_tile_second;
    case 0x83a603u: goto quad_2_store_second_tile;
    case 0x83a606u: goto quad_2_next_tile_third;
    case 0x83a607u: goto quad_2_next_tile_fourth;
    case 0x83a608u: goto quad_2_store_third_tile;
    case 0x83a60bu: goto quad_2_next_tile_fifth;
    case 0x83a60cu: goto quad_2_next_tile_sixth;
    case 0x83a60du: goto quad_2_store_fourth_tile;
    case 0x83a610u: goto quad_2_load_top_y;
    case 0x83a612u: goto quad_2_store_first_y;
    case 0x83a615u: goto quad_2_store_second_y;
    case 0x83a618u: goto quad_2_clear_bottom_carry;
    case 0x83a619u: goto quad_2_add_height;
    case 0x83a61bu: goto quad_2_store_third_y;
    case 0x83a61eu: goto quad_2_store_fourth_y;
    case 0x83a621u: goto quad_2_load_left_x;
    case 0x83a623u: goto quad_2_store_first_x;
    case 0x83a626u: goto quad_2_store_third_x;
    case 0x83a629u: goto quad_2_clear_right_carry;
    case 0x83a62au: goto quad_2_add_width;
    case 0x83a62cu: goto quad_2_store_second_x;
    case 0x83a62fu: goto quad_2_store_fourth_x;
    case 0x83a632u: goto quad_2_return_quad;
    case 0x83a633u: goto quad_3_index_output;
    case 0x83a634u: goto quad_3_store_first_tile;
    case 0x83a637u: goto quad_3_next_tile_first;
    case 0x83a638u: goto quad_3_next_tile_second;
    case 0x83a639u: goto quad_3_store_second_tile;
    case 0x83a63cu: goto quad_3_next_tile_third;
    case 0x83a63du: goto quad_3_next_tile_fourth;
    case 0x83a63eu: goto quad_3_store_third_tile;
    case 0x83a641u: goto quad_3_next_tile_fifth;
    case 0x83a642u: goto quad_3_next_tile_sixth;
    case 0x83a643u: goto quad_3_store_fourth_tile;
    case 0x83a646u: goto quad_3_load_top_y;
    case 0x83a648u: goto quad_3_store_first_y;
    case 0x83a64bu: goto quad_3_store_second_y;
    case 0x83a64eu: goto quad_3_clear_bottom_carry;
    case 0x83a64fu: goto quad_3_add_height;
    case 0x83a651u: goto quad_3_store_third_y;
    case 0x83a654u: goto quad_3_store_fourth_y;
    case 0x83a657u: goto quad_3_load_left_x;
    case 0x83a659u: goto quad_3_store_first_x;
    case 0x83a65cu: goto quad_3_store_third_x;
    case 0x83a65fu: goto quad_3_clear_right_carry;
    case 0x83a660u: goto quad_3_add_width;
    case 0x83a662u: goto quad_3_store_second_x;
    case 0x83a665u: goto quad_3_store_fourth_x;
    case 0x83a668u: goto quad_3_return_quad;
    default: return 0;
    }
square_index_output: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a534u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa535u);
}
square_seed_attribute_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a535u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->dp, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa536u);
}
square_load_attributes: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x97u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a536u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa538u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
square_store_first_attributes: {
    const bool wide = false;
    const bool crossed = (0x0003u >> 8) != ((0x0003u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0003u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a538u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa53bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
square_store_second_attributes: {
    const bool wide = false;
    const bool crossed = (0x0007u >> 8) != ((0x0007u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0007u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a53bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa53eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
square_store_third_attributes: {
    const bool wide = false;
    const bool crossed = (0x000bu >> 8) != ((0x000bu + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000bu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a53eu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa541u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
square_store_fourth_attributes: {
    const bool wide = false;
    const bool crossed = (0x000fu >> 8) != ((0x000fu + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000fu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a541u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa544u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
square_widen_flip: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a544u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa546u);
}
square_shift_flip_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a546u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa547u);
}
square_shift_flip_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a547u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa548u);
}
square_shift_flip_third: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a548u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa549u);
}
square_narrow_flip: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a549u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa54bu);
}
square_clear_flip_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a54bu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa54du);
}
square_exchange_flip: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a54du, 1u, span.instructions != 0u);
    registers->a = (uint16_t)((registers->a >> 8) | (registers->a << 8));
    registers->z = (registers->a & 0xffu) == 0u;
    registers->n = (registers->a & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 3u, 0xa54eu);
}
square_mask_flip: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a54eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x0006u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa550u);
}
square_index_flip: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a550u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa551u);
}
square_load_tile: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x9du);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a551u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa553u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
square_draw_quad: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu ||
        registers->x > 6u || (registers->x & 1u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a553u, 3u, span.instructions != 0u);
    const uint32_t pointer = 0x83a589u + registers->x;
    const uint16_t target = Lufia2DecompReadData(registers, pointer, pointer + 1u, true);
    registers->write(registers->mem, registers->sp, 0xa5u);
    --registers->sp;
    registers->write(registers->mem, registers->sp, 0x55u);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, 8u, target);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
square_load_high_x: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x90u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a556u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa558u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
square_test_high_x: {
    const bool taken = registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a558u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa56cu : 0xa55au);
    if (taken) {
        if (0x83a56cu < first_pc || 0x83a56cu >= end_pc)
            goto finished;
        goto square_load_hidden_left_x;
    }
}
square_load_left_x: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x8fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a55au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa55cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
square_clear_right_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a55cu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa55du);
}
square_add_right_width: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a55du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa55fu);
}
square_right_is_visible: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a55fu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa588u : 0xa561u);
    if (taken) {
        if (0x83a588u < first_pc || 0x83a588u >= end_pc)
            goto finished;
        goto square_return_square;
    }
}
square_skip_first_bit: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + 2u * wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a561u, 2u, span.instructions != 0u);
    const uint8_t value = (uint8_t)(registers->read(registers->mem, address) + 1u);
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    registers->write(registers->mem, address, value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa563u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
square_set_second_bit: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a563u);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83a563u, 0x20u, 0x83a669u, false,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
square_skip_third_bit: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + 2u * wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a566u, 2u, span.instructions != 0u);
    const uint8_t value = (uint8_t)(registers->read(registers->mem, address) + 1u);
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    registers->write(registers->mem, address, value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa568u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
square_set_fourth_bit: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a568u);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83a568u, 0x20u, 0x83a669u, false,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
square_return_right_overflow: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a56bu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83a56bu, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
square_load_hidden_left_x: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x8fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a56cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa56eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
square_clear_hidden_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a56eu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa56fu);
}
square_add_hidden_width: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a56fu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa571u);
}
square_left_is_hidden: {
    const bool taken = registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a571u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa580u : 0xa573u);
    if (taken) {
        if (0x83a580u < first_pc || 0x83a580u >= end_pc)
            goto finished;
        goto square_set_first_bit;
    }
    goto finished;
}
square_set_first_hidden_bit: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a573u);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83a573u, 0x20u, 0x83a669u, false,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
square_set_second_hidden_bit: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a576u);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83a576u, 0x20u, 0x83a669u, false,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
square_set_third_hidden_bit: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a579u);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83a579u, 0x20u, 0x83a669u, false,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
square_set_fourth_hidden_bit: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a57cu);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83a57cu, 0x20u, 0x83a669u, false,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
square_return_hidden_pair: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a57fu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83a57fu, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
square_set_first_bit: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a580u);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83a580u, 0x20u, 0x83a669u, false,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
square_skip_second_bit: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + 2u * wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a583u, 2u, span.instructions != 0u);
    const uint8_t value = (uint8_t)(registers->read(registers->mem, address) + 1u);
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    registers->write(registers->mem, address, value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa585u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
square_set_third_bit: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a585u);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83a585u, 0x20u, 0x83a669u, false,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
square_return_square: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a588u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83a588u, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
quad_0_index_output: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a591u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa592u);
}
quad_0_store_first_tile: {
    const bool wide = false;
    const bool crossed = (0x0002u >> 8) != ((0x0002u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0002u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a592u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa595u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_0_next_tile_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a595u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa596u);
}
quad_0_next_tile_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a596u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa597u);
}
quad_0_store_second_tile: {
    const bool wide = false;
    const bool crossed = (0x0006u >> 8) != ((0x0006u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0006u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a597u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa59au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_0_next_tile_third: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a59au, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa59bu);
}
quad_0_next_tile_fourth: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a59bu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa59cu);
}
quad_0_store_third_tile: {
    const bool wide = false;
    const bool crossed = (0x000au >> 8) != ((0x000au + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000au + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a59cu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa59fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_0_next_tile_fifth: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a59fu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5a0u);
}
quad_0_next_tile_sixth: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5a0u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5a1u);
}
quad_0_store_fourth_tile: {
    const bool wide = false;
    const bool crossed = (0x000eu >> 8) != ((0x000eu + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5a1u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5a4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_0_load_top_y: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x91u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5a4u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5a6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_0_store_first_y: {
    const bool wide = false;
    const bool crossed = (0x0001u >> 8) != ((0x0001u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0001u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5a6u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5a9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_0_store_second_y: {
    const bool wide = false;
    const bool crossed = (0x0005u >> 8) != ((0x0005u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0005u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5a9u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5acu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_0_clear_bottom_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5acu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5adu);
}
quad_0_add_height: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5adu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5afu);
}
quad_0_store_third_y: {
    const bool wide = false;
    const bool crossed = (0x0009u >> 8) != ((0x0009u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0009u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5afu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5b2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_0_store_fourth_y: {
    const bool wide = false;
    const bool crossed = (0x000du >> 8) != ((0x000du + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000du + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5b2u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5b5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_0_load_left_x: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x8fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5b5u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5b7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_0_store_first_x: {
    const bool wide = false;
    const bool crossed = (0x0000u >> 8) != ((0x0000u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5b7u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5bau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_0_store_third_x: {
    const bool wide = false;
    const bool crossed = (0x0008u >> 8) != ((0x0008u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0008u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5bau, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5bdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_0_clear_right_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5bdu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5beu);
}
quad_0_add_width: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5beu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5c0u);
}
quad_0_store_second_x: {
    const bool wide = false;
    const bool crossed = (0x0004u >> 8) != ((0x0004u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0004u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5c0u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5c3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_0_store_fourth_x: {
    const bool wide = false;
    const bool crossed = (0x000cu >> 8) != ((0x000cu + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000cu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5c3u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5c6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_0_return_quad: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a5c6u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83a5c6u, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
quad_1_index_output: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5c7u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5c8u);
}
quad_1_store_first_tile: {
    const bool wide = false;
    const bool crossed = (0x0006u >> 8) != ((0x0006u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0006u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5c8u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5cbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_1_next_tile_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5cbu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5ccu);
}
quad_1_next_tile_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5ccu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5cdu);
}
quad_1_store_second_tile: {
    const bool wide = false;
    const bool crossed = (0x0002u >> 8) != ((0x0002u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0002u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5cdu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5d0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_1_next_tile_third: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5d0u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5d1u);
}
quad_1_next_tile_fourth: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5d1u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5d2u);
}
quad_1_store_third_tile: {
    const bool wide = false;
    const bool crossed = (0x000eu >> 8) != ((0x000eu + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5d2u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5d5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_1_next_tile_fifth: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5d5u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5d6u);
}
quad_1_next_tile_sixth: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5d6u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5d7u);
}
quad_1_store_fourth_tile: {
    const bool wide = false;
    const bool crossed = (0x000au >> 8) != ((0x000au + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000au + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5d7u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5dau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_1_load_top_y: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x91u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5dau, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5dcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_1_store_first_y: {
    const bool wide = false;
    const bool crossed = (0x0001u >> 8) != ((0x0001u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0001u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5dcu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5dfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_1_store_second_y: {
    const bool wide = false;
    const bool crossed = (0x0005u >> 8) != ((0x0005u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0005u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5dfu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5e2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_1_clear_bottom_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5e2u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5e3u);
}
quad_1_add_height: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5e3u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5e5u);
}
quad_1_store_third_y: {
    const bool wide = false;
    const bool crossed = (0x0009u >> 8) != ((0x0009u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0009u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5e5u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5e8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_1_store_fourth_y: {
    const bool wide = false;
    const bool crossed = (0x000du >> 8) != ((0x000du + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000du + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5e8u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5ebu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_1_load_left_x: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x8fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5ebu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5edu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_1_store_first_x: {
    const bool wide = false;
    const bool crossed = (0x0000u >> 8) != ((0x0000u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5edu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5f0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_1_store_third_x: {
    const bool wide = false;
    const bool crossed = (0x0008u >> 8) != ((0x0008u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0008u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5f0u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5f3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_1_clear_right_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5f3u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5f4u);
}
quad_1_add_width: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5f4u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5f6u);
}
quad_1_store_second_x: {
    const bool wide = false;
    const bool crossed = (0x0004u >> 8) != ((0x0004u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0004u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5f6u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5f9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_1_store_fourth_x: {
    const bool wide = false;
    const bool crossed = (0x000cu >> 8) != ((0x000cu + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000cu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5f9u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa5fcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_1_return_quad: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a5fcu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83a5fcu, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
quad_2_index_output: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5fdu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa5feu);
}
quad_2_store_first_tile: {
    const bool wide = false;
    const bool crossed = (0x0002u >> 8) != ((0x0002u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0002u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a5feu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa601u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_2_next_tile_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a601u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa602u);
}
quad_2_next_tile_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a602u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa603u);
}
quad_2_store_second_tile: {
    const bool wide = false;
    const bool crossed = (0x0006u >> 8) != ((0x0006u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0006u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a603u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa606u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_2_next_tile_third: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a606u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa607u);
}
quad_2_next_tile_fourth: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a607u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa608u);
}
quad_2_store_third_tile: {
    const bool wide = false;
    const bool crossed = (0x000au >> 8) != ((0x000au + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000au + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a608u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa60bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_2_next_tile_fifth: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a60bu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa60cu);
}
quad_2_next_tile_sixth: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a60cu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa60du);
}
quad_2_store_fourth_tile: {
    const bool wide = false;
    const bool crossed = (0x000eu >> 8) != ((0x000eu + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a60du, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa610u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_2_load_top_y: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x91u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a610u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa612u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_2_store_first_y: {
    const bool wide = false;
    const bool crossed = (0x0009u >> 8) != ((0x0009u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0009u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a612u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa615u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_2_store_second_y: {
    const bool wide = false;
    const bool crossed = (0x000du >> 8) != ((0x000du + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000du + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a615u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa618u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_2_clear_bottom_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a618u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa619u);
}
quad_2_add_height: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a619u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa61bu);
}
quad_2_store_third_y: {
    const bool wide = false;
    const bool crossed = (0x0001u >> 8) != ((0x0001u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0001u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a61bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa61eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_2_store_fourth_y: {
    const bool wide = false;
    const bool crossed = (0x0005u >> 8) != ((0x0005u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0005u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a61eu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa621u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_2_load_left_x: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x8fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a621u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa623u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_2_store_first_x: {
    const bool wide = false;
    const bool crossed = (0x0000u >> 8) != ((0x0000u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a623u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa626u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_2_store_third_x: {
    const bool wide = false;
    const bool crossed = (0x0008u >> 8) != ((0x0008u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0008u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a626u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa629u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_2_clear_right_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a629u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa62au);
}
quad_2_add_width: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a62au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa62cu);
}
quad_2_store_second_x: {
    const bool wide = false;
    const bool crossed = (0x0004u >> 8) != ((0x0004u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0004u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a62cu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa62fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_2_store_fourth_x: {
    const bool wide = false;
    const bool crossed = (0x000cu >> 8) != ((0x000cu + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000cu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a62fu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa632u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_2_return_quad: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a632u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83a632u, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
quad_3_index_output: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a633u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa634u);
}
quad_3_store_first_tile: {
    const bool wide = false;
    const bool crossed = (0x0006u >> 8) != ((0x0006u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0006u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a634u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa637u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_3_next_tile_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a637u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa638u);
}
quad_3_next_tile_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a638u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa639u);
}
quad_3_store_second_tile: {
    const bool wide = false;
    const bool crossed = (0x0002u >> 8) != ((0x0002u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0002u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a639u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa63cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_3_next_tile_third: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a63cu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa63du);
}
quad_3_next_tile_fourth: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a63du, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa63eu);
}
quad_3_store_third_tile: {
    const bool wide = false;
    const bool crossed = (0x000eu >> 8) != ((0x000eu + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a63eu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa641u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_3_next_tile_fifth: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a641u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa642u);
}
quad_3_next_tile_sixth: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a642u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa643u);
}
quad_3_store_fourth_tile: {
    const bool wide = false;
    const bool crossed = (0x000au >> 8) != ((0x000au + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000au + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a643u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa646u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_3_load_top_y: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x91u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a646u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa648u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_3_store_first_y: {
    const bool wide = false;
    const bool crossed = (0x0009u >> 8) != ((0x0009u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0009u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a648u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa64bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_3_store_second_y: {
    const bool wide = false;
    const bool crossed = (0x000du >> 8) != ((0x000du + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000du + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a64bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa64eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_3_clear_bottom_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a64eu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa64fu);
}
quad_3_add_height: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a64fu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa651u);
}
quad_3_store_third_y: {
    const bool wide = false;
    const bool crossed = (0x0001u >> 8) != ((0x0001u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0001u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a651u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa654u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_3_store_fourth_y: {
    const bool wide = false;
    const bool crossed = (0x0005u >> 8) != ((0x0005u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0005u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a654u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa657u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_3_load_left_x: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x8fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a657u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa659u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_3_store_first_x: {
    const bool wide = false;
    const bool crossed = (0x0000u >> 8) != ((0x0000u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a659u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa65cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_3_store_third_x: {
    const bool wide = false;
    const bool crossed = (0x0008u >> 8) != ((0x0008u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0008u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a65cu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa65fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_3_clear_right_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a65fu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa660u);
}
quad_3_add_width: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a660u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa662u);
}
quad_3_store_second_x: {
    const bool wide = false;
    const bool crossed = (0x0004u >> 8) != ((0x0004u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0004u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a662u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa665u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_3_store_fourth_x: {
    const bool wide = false;
    const bool crossed = (0x000cu >> 8) != ((0x000cu + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000cu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a665u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa668u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
quad_3_return_quad: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a668u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83a668u, 0x60u, false, registers->mf, true);
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
