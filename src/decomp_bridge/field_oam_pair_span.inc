#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunFieldOamPairSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                           uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget,
                                           long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x83a4a2u && pc < 0x83a4bbu)
        end_pc = 0x83a4bbu;
    else if (pc == 0x83a4bbu)
        end_pc = 0x83a4beu;
    else if (pc == 0x83a4beu)
        end_pc = 0x83a4bfu;
    else if (pc >= 0x83a4bfu && pc < 0x83a4fau)
        end_pc = 0x83a4fau;
    else if (pc == 0x83a4fau)
        end_pc = 0x83a4fdu;
    else if (pc == 0x83a4fdu)
        end_pc = 0x83a500u;
    else if (pc == 0x83a500u)
        end_pc = 0x83a501u;
    else if (pc >= 0x83a501u && pc < 0x83a52du)
        end_pc = 0x83a52du;
    else if (pc == 0x83a52du)
        end_pc = 0x83a530u;
    else if (pc == 0x83a530u)
        end_pc = 0x83a533u;
    else if (pc == 0x83a533u)
        end_pc = 0x83a534u;
    else if (pc >= 0x83a669u && pc < 0x83a686u)
        end_pc = 0x83a686u;
    else
        return 0;
    const bool returning = pc == 0x83a4beu || pc == 0x83a500u || pc == 0x83a533u || pc == 0x83a685u;
    if (!cpu || !cpu->ram || registers->e || !step_budget || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted || (!returning && (!registers->mf || registers->xf)) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x83a4a2u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 484u ||
        memcmp(cart->rom + first,
               "\xbb\xa5\x9d\x9d\x02\x00\xa5\x97\x9d\x03\x00\xa5\x91\x9d\x01\x00\xa5\x8f\x9d\x00\x00\xa5\x90"
               "\xf0\x03\x20\x69\xa6\x60\xbb\xa5\x9d\x9d\x02\x00\x1a\x1a\x9d\x06\x00\xa5\x97\x9d\x03\x00\x9d"
               "\x07\x00\x89\x80\xd0\x0d\xa5\x91\x9d\x01\x00\x18\x69\x10\x9d\x05\x00\x80\x0b\xa5\x91\x9d\x05"
               "\x00\x18\x69\x10\x9d\x01\x00\xa5\x8f\x9d\x00\x00\x9d\x04\x00\xa5\x90\xf0\x06\x20\x69\xa6\x20"
               "\x69\xa6\x60\xbb\xa5\x9d\x9d\x02\x00\x1a\x1a\x9d\x06\x00\xa5\x91\x9d\x01\x00\x9d\x05\x00\xa5"
               "\x97\x9d\x03\x00\x9d\x07\x00\xa5\x8f\x9d\x00\x00\x18\x69\x10\x9d\x04\x00\xb0\x04\xa5\x90\xf0"
               "\x06\x20\x69\xa6\x20\x69\xa6\x60",
               146u) ||
        memcmp(cart->rom + first + 455u,
               "\x7b\xa5\x58\x29\x03\xaa\xbd\x10\xac\x85\x55\xa5\x58\xe6\x58\x4a\x4a\x29\x1f\xaa\xbd\x00\x03"
               "\x05\x55\x9d\x00\x03\x60",
               29u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83a4a2u: goto single_index_output;
    case 0x83a4a3u: goto single_load_tile;
    case 0x83a4a5u: goto single_store_tile;
    case 0x83a4a8u: goto single_load_attributes;
    case 0x83a4aau: goto single_store_attributes;
    case 0x83a4adu: goto single_load_y;
    case 0x83a4afu: goto single_store_y;
    case 0x83a4b2u: goto single_load_x;
    case 0x83a4b4u: goto single_store_x;
    case 0x83a4b7u: goto single_load_high_bit;
    case 0x83a4b9u: goto single_check_high_bit;
    case 0x83a4bbu: goto single_set_high_bit;
    case 0x83a4beu: goto single_return;
    case 0x83a4bfu: goto vertical_index_output;
    case 0x83a4c0u: goto vertical_load_tile;
    case 0x83a4c2u: goto vertical_store_top_tile;
    case 0x83a4c5u: goto vertical_next_tile_1;
    case 0x83a4c6u: goto vertical_next_tile_2;
    case 0x83a4c7u: goto vertical_store_bottom_tile;
    case 0x83a4cau: goto vertical_load_attributes;
    case 0x83a4ccu: goto vertical_store_top_attributes;
    case 0x83a4cfu: goto vertical_store_bottom_attributes;
    case 0x83a4d2u: goto vertical_test_flip;
    case 0x83a4d4u: goto vertical_choose_flip;
    case 0x83a4d6u: goto vertical_load_top_y;
    case 0x83a4d8u: goto vertical_store_top_y;
    case 0x83a4dbu: goto vertical_clear_y_carry;
    case 0x83a4dcu: goto vertical_add_height;
    case 0x83a4deu: goto vertical_store_bottom_y;
    case 0x83a4e1u: goto vertical_join_positions;
    case 0x83a4e3u: goto vertical_load_flipped_y;
    case 0x83a4e5u: goto vertical_store_flipped_bottom_y;
    case 0x83a4e8u: goto vertical_clear_flipped_carry;
    case 0x83a4e9u: goto vertical_add_flipped_height;
    case 0x83a4ebu: goto vertical_store_flipped_top_y;
    case 0x83a4eeu: goto vertical_load_x;
    case 0x83a4f0u: goto vertical_store_top_x;
    case 0x83a4f3u: goto vertical_store_bottom_x;
    case 0x83a4f6u: goto vertical_load_high_bit;
    case 0x83a4f8u: goto vertical_check_high_bit;
    case 0x83a4fau: goto vertical_set_first_high_bit;
    case 0x83a4fdu: goto vertical_set_second_high_bit;
    case 0x83a500u: goto vertical_return;
    case 0x83a501u: goto horizontal_index_output;
    case 0x83a502u: goto horizontal_load_tile;
    case 0x83a504u: goto horizontal_store_left_tile;
    case 0x83a507u: goto horizontal_next_tile_1;
    case 0x83a508u: goto horizontal_next_tile_2;
    case 0x83a509u: goto horizontal_store_right_tile;
    case 0x83a50cu: goto horizontal_load_y;
    case 0x83a50eu: goto horizontal_store_left_y;
    case 0x83a511u: goto horizontal_store_right_y;
    case 0x83a514u: goto horizontal_load_attributes;
    case 0x83a516u: goto horizontal_store_left_attributes;
    case 0x83a519u: goto horizontal_store_right_attributes;
    case 0x83a51cu: goto horizontal_load_x;
    case 0x83a51eu: goto horizontal_store_left_x;
    case 0x83a521u: goto horizontal_clear_x_carry;
    case 0x83a522u: goto horizontal_add_width;
    case 0x83a524u: goto horizontal_store_right_x;
    case 0x83a527u: goto horizontal_check_wrap;
    case 0x83a529u: goto horizontal_load_high_bit;
    case 0x83a52bu: goto horizontal_check_high_bit;
    case 0x83a52du: goto horizontal_set_first_high_bit;
    case 0x83a530u: goto horizontal_set_second_high_bit;
    case 0x83a533u: goto horizontal_return;
    case 0x83a669u: goto mask_load_direct_page;
    case 0x83a66au: goto mask_load_bit_index;
    case 0x83a66cu: goto mask_low_bits;
    case 0x83a66eu: goto mask_index_table;
    case 0x83a66fu: goto mask_read_table;
    case 0x83a672u: goto mask_save_bits;
    case 0x83a674u: goto mask_reload_bit_index;
    case 0x83a676u: goto mask_advance_bit_index;
    case 0x83a678u: goto mask_shift_low;
    case 0x83a679u: goto mask_shift_high;
    case 0x83a67au: goto mask_byte_index;
    case 0x83a67cu: goto mask_select_byte;
    case 0x83a67du: goto mask_read_byte;
    case 0x83a680u: goto mask_merge_bits;
    case 0x83a682u: goto mask_store_byte;
    case 0x83a685u: goto mask_return;
    default: return 0;
    }
single_index_output: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4a2u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa4a3u);
}
single_load_tile: {
    const uint32_t address = (uint16_t)(registers->dp + 0x9du);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4a3u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4a5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
single_store_tile: {
    const bool crossed = (0x0002u >> 8) != ((0x0002u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0002u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4a5u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4a8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
single_load_attributes: {
    const uint32_t address = (uint16_t)(registers->dp + 0x97u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4a8u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4aau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
single_store_attributes: {
    const bool crossed = (0x0003u >> 8) != ((0x0003u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0003u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4aau, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4adu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
single_load_y: {
    const uint32_t address = (uint16_t)(registers->dp + 0x91u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4adu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4afu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
single_store_y: {
    const bool crossed = (0x0001u >> 8) != ((0x0001u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0001u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4afu, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4b2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
single_load_x: {
    const uint32_t address = (uint16_t)(registers->dp + 0x8fu);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4b2u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4b4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
single_store_x: {
    const bool crossed = (0x0000u >> 8) != ((0x0000u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4b4u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4b7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
single_load_high_bit: {
    const uint32_t address = (uint16_t)(registers->dp + 0x90u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4b7u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4b9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
single_check_high_bit: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4b9u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa4beu : 0xa4bbu);
    if (taken) {
        if (0x83a4beu >= end_pc)
            goto finished;
        goto single_return;
    }
    goto finished;
}
single_set_high_bit: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a4bbu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x83a4bbu, 0x20u, 0x83a669u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
single_return: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a4beu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83a4beu, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
vertical_index_output: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4bfu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa4c0u);
}
vertical_load_tile: {
    const uint32_t address = (uint16_t)(registers->dp + 0x9du);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4c0u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4c2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
vertical_store_top_tile: {
    const bool crossed = (0x0002u >> 8) != ((0x0002u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0002u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4c2u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4c5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
vertical_next_tile_1: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4c5u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa4c6u);
}
vertical_next_tile_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4c6u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa4c7u);
}
vertical_store_bottom_tile: {
    const bool crossed = (0x0006u >> 8) != ((0x0006u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0006u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4c7u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4cau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
vertical_load_attributes: {
    const uint32_t address = (uint16_t)(registers->dp + 0x97u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4cau, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4ccu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
vertical_store_top_attributes: {
    const bool crossed = (0x0003u >> 8) != ((0x0003u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0003u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4ccu, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4cfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
vertical_store_bottom_attributes: {
    const bool crossed = (0x0007u >> 8) != ((0x0007u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0007u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4cfu, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4d2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
vertical_test_flip: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4d2u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x80u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa4d4u);
}
vertical_choose_flip: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4d4u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa4e3u : 0xa4d6u);
    if (taken) {
        if (0x83a4e3u >= end_pc)
            goto finished;
        goto vertical_load_flipped_y;
    }
}
vertical_load_top_y: {
    const uint32_t address = (uint16_t)(registers->dp + 0x91u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4d6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4d8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
vertical_store_top_y: {
    const bool crossed = (0x0001u >> 8) != ((0x0001u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0001u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4d8u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4dbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
vertical_clear_y_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4dbu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa4dcu);
}
vertical_add_height: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4dcu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x10u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa4deu);
}
vertical_store_bottom_y: {
    const bool crossed = (0x0005u >> 8) != ((0x0005u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0005u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4deu, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4e1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
vertical_join_positions: {
    const bool taken = true;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4e1u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa4eeu : 0xa4e3u);
    if (taken) {
        if (0x83a4eeu >= end_pc)
            goto finished;
        goto vertical_load_x;
    }
}
vertical_load_flipped_y: {
    const uint32_t address = (uint16_t)(registers->dp + 0x91u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4e3u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4e5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
vertical_store_flipped_bottom_y: {
    const bool crossed = (0x0005u >> 8) != ((0x0005u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0005u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4e5u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4e8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
vertical_clear_flipped_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4e8u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa4e9u);
}
vertical_add_flipped_height: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4e9u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x10u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa4ebu);
}
vertical_store_flipped_top_y: {
    const bool crossed = (0x0001u >> 8) != ((0x0001u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0001u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4ebu, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4eeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
vertical_load_x: {
    const uint32_t address = (uint16_t)(registers->dp + 0x8fu);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4eeu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4f0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
vertical_store_top_x: {
    const bool crossed = (0x0000u >> 8) != ((0x0000u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4f0u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4f3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
vertical_store_bottom_x: {
    const bool crossed = (0x0004u >> 8) != ((0x0004u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0004u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4f3u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4f6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
vertical_load_high_bit: {
    const uint32_t address = (uint16_t)(registers->dp + 0x90u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4f6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa4f8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
vertical_check_high_bit: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a4f8u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa500u : 0xa4fau);
    if (taken) {
        if (0x83a500u >= end_pc)
            goto finished;
        goto vertical_return;
    }
    goto finished;
}
vertical_set_first_high_bit: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a4fau);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x83a4fau, 0x20u, 0x83a669u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
vertical_set_second_high_bit: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a4fdu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x83a4fdu, 0x20u, 0x83a669u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
vertical_return: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a500u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83a500u, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
horizontal_index_output: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a501u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa502u);
}
horizontal_load_tile: {
    const uint32_t address = (uint16_t)(registers->dp + 0x9du);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a502u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa504u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
horizontal_store_left_tile: {
    const bool crossed = (0x0002u >> 8) != ((0x0002u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0002u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a504u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa507u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
horizontal_next_tile_1: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a507u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa508u);
}
horizontal_next_tile_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a508u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa509u);
}
horizontal_store_right_tile: {
    const bool crossed = (0x0006u >> 8) != ((0x0006u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0006u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a509u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa50cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
horizontal_load_y: {
    const uint32_t address = (uint16_t)(registers->dp + 0x91u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a50cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa50eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
horizontal_store_left_y: {
    const bool crossed = (0x0001u >> 8) != ((0x0001u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0001u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a50eu, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa511u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
horizontal_store_right_y: {
    const bool crossed = (0x0005u >> 8) != ((0x0005u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0005u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a511u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa514u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
horizontal_load_attributes: {
    const uint32_t address = (uint16_t)(registers->dp + 0x97u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a514u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa516u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
horizontal_store_left_attributes: {
    const bool crossed = (0x0003u >> 8) != ((0x0003u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0003u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a516u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa519u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
horizontal_store_right_attributes: {
    const bool crossed = (0x0007u >> 8) != ((0x0007u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0007u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a519u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa51cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
horizontal_load_x: {
    const uint32_t address = (uint16_t)(registers->dp + 0x8fu);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a51cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa51eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
horizontal_store_left_x: {
    const bool crossed = (0x0000u >> 8) != ((0x0000u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a51eu, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa521u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
horizontal_clear_x_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a521u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa522u);
}
horizontal_add_width: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a522u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x10u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa524u);
}
horizontal_store_right_x: {
    const bool crossed = (0x0004u >> 8) != ((0x0004u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0004u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a524u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa527u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
horizontal_check_wrap: {
    const bool taken = registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a527u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa52du : 0xa529u);
    if (taken) {
        if (0x83a52du >= end_pc)
            goto finished;
        goto horizontal_set_first_high_bit;
    }
}
horizontal_load_high_bit: {
    const uint32_t address = (uint16_t)(registers->dp + 0x90u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a529u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa52bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
horizontal_check_high_bit: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a52bu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa533u : 0xa52du);
    if (taken) {
        if (0x83a533u >= end_pc)
            goto finished;
        goto horizontal_return;
    }
    goto finished;
}
horizontal_set_first_high_bit: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a52du);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x83a52du, 0x20u, 0x83a669u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
horizontal_set_second_high_bit: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a530u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x83a530u, 0x20u, 0x83a669u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
horizontal_return: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a533u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83a533u, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
mask_load_direct_page: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a669u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->dp, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa66au);
}
mask_load_bit_index: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a66au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa66cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_low_bits: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a66cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x03u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa66eu);
}
mask_index_table: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a66eu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa66fu);
}
mask_read_table: {
    const bool crossed = (0xac10u >> 8) != ((0xac10u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xac10u + registers->x) & 0xffffffu;
    const unsigned cycles = 4u + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a66fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa672u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_save_bits: {
    const uint32_t address = (uint16_t)(registers->dp + 0x55u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a672u, 2u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa674u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_reload_bit_index: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a674u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa676u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_advance_bit_index: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a676u, 2u, span.instructions != 0u);
    const uint8_t value = (uint8_t)(registers->read(registers->mem, address) + 1u);
    registers->write(registers->mem, address, value);
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, cycles, 0xa678u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_shift_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a678u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (registers->a & 0xffu) >> 1, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa679u);
}
mask_shift_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a679u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (registers->a & 0xffu) >> 1, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa67au);
}
mask_byte_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a67au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x1fu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa67cu);
}
mask_select_byte: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a67cu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa67du);
}
mask_read_byte: {
    const bool crossed = (0x0300u >> 8) != ((0x0300u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0300u + registers->x) & 0xffffffu;
    const unsigned cycles = 4u + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a67du, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa680u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_merge_bits: {
    const uint32_t address = (uint16_t)(registers->dp + 0x55u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a680u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa682u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_store_byte: {
    const bool crossed = (0x0300u >> 8) != ((0x0300u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0300u + registers->x) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a682u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa685u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_return: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a685u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83a685u, 0x60u, false, registers->mf, true);
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
