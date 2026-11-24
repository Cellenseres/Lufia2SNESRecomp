static int Lufia2DecompRunFieldLocate(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                      uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget,
                                      long *step_count) {
    if (pc < 0x80f734u || pc > 0x80f81bu || !cpu || !cpu->ram || registers->e ||
        (pc != 0x80f81bu && registers->xf) ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || !step_budget ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x80f734u, 0x80f81cu))
        return 0;
    const bool byte_entry = (pc >= 0x80f75au && pc <= 0x80f760u) || (pc >= 0x80f772u && pc <= 0x80f778u) ||
                            (pc >= 0x80f7a5u && pc <= 0x80f7abu) || (pc >= 0x80f7c1u && pc <= 0x80f7c7u) ||
                            (pc >= 0x80f7e8u && pc <= 0x80f7f6u);
    const bool changes_width = pc == 0x80f758u || pc == 0x80f770u || pc == 0x80f7a3u || pc == 0x80f7bfu ||
                               pc == 0x80f7e6u || pc == 0x80f7f8u;
    if (pc != 0x80f81bu && !changes_width && registers->mf != byte_entry)
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x80f734u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 232u ||
        memcmp(cart->rom + first,
               "\x4a\x4a\x4a\x4a\x85\x30\x85\x22\xbd\x10\xd0\x85\x83\xbd\x18\xd0"
               "\x85\x85\xa5\x22\x89\x00\x08\xf0\x1f\x09\x00\xf0\x49\xff\xff\x1a"
               "\x8f\x04\x42\x00\xe2\x20\xa5\x83\x8f\x06\x42\x00\x20\x1c\xf8\xa5"
               "\x83\x38\xef\x16\x42\x00\x80\x13\x8f\x04\x42\x00\xe2\x20\xa5\x83"
               "\x8f\x06\x42\x00\x20\x1c\xf8\xaf\x16\x42\x00\x85\x87\x85\x26\xa5"
               "\x30\x0a\x0a\x29\x3e\x00\x85\x22\x98\x4a\x4a\x4a\x4a\x85\x28\x89"
               "\x00\x08\xf0\x1f\x09\x00\xf0\x49\xff\xff\x1a\x8f\x04\x42\x00\xe2"
               "\x20\xa5\x85\x8f\x06\x42\x00\x20\x1c\xf8\xa5\x85\x38\xef\x16\x42"
               "\x00\x80\x17\xc5\x85\x90\x13\x8f\x04\x42\x00\xe2\x20\xa5\x85\x8f"
               "\x06\x42\x00\x20\x1c\xf8\xaf\x16\x42\x00\x85\x89\x85\x28\x98\x29"
               "\xf0\x00\x0a\x0a\x0a\x85\x24\x18\x65\x22\x85\x2d\xbf\xf0\x8f\x83"
               "\x85\x2a\xe2\x20\xa5\x28\x8f\x02\x42\x00\xa5\x83\x8f\x03\x42\x00"
               "\xa9\x7e\x85\x2c\xc2\x20\xbd\x08\xd0\x85\x8d\xaf\x16\x42\x00\x18"
               "\x65\x26\x0a\x65\x8d\x85\x30\xad\x3c\xd0\x85\x65\xe0\x04\x00\x90"
               "\x05\xad\x40\xd0\x85\x65\x18\x60",
               232u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    const uint32_t bank = (uint32_t)registers->db << 16;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x80f734u:
        goto shift_x_1;
    case 0x80f735u:
        goto shift_x_2;
    case 0x80f736u:
        goto shift_x_3;
    case 0x80f737u:
        goto shift_x_4;
    case 0x80f738u:
        goto save_cell_x;
    case 0x80f73au:
        goto save_buffer_column;
    case 0x80f73cu:
        goto load_layer_width;
    case 0x80f73fu:
        goto save_layer_width;
    case 0x80f741u:
        goto load_layer_height;
    case 0x80f744u:
        goto save_layer_height;
    case 0x80f746u:
        goto load_x_for_wrap;
    case 0x80f748u:
        goto test_x_sign;
    case 0x80f74bu:
        goto skip_negative_x;
    case 0x80f74du:
        goto extend_negative_x;
    case 0x80f750u:
        goto invert_negative_x;
    case 0x80f753u:
        goto absolute_negative_x;
    case 0x80f754u:
        goto write_negative_x_dividend;
    case 0x80f758u:
        goto narrow_negative_x_divisor;
    case 0x80f75au:
        goto load_negative_x_divisor;
    case 0x80f75cu:
        goto write_negative_x_divisor;
    case 0x80f760u:
        goto wait_negative_x_division;
    case 0x80f763u:
        goto reload_negative_x_width;
    case 0x80f765u:
        goto set_negative_x_carry;
    case 0x80f766u:
        goto subtract_negative_x_remainder;
    case 0x80f76au:
        goto finish_negative_x_wrap;
    case 0x80f76cu:
        goto write_x_dividend;
    case 0x80f770u:
        goto narrow_x_divisor;
    case 0x80f772u:
        goto load_x_divisor;
    case 0x80f774u:
        goto write_x_divisor;
    case 0x80f778u:
        goto wait_x_division;
    case 0x80f77bu:
        goto read_x_remainder;
    case 0x80f77fu:
        goto save_wrapped_x;
    case 0x80f781u:
        goto save_row_cell_x;
    case 0x80f783u:
        goto reload_cell_x;
    case 0x80f785u:
        goto scale_column_1;
    case 0x80f786u:
        goto scale_column_2;
    case 0x80f787u:
        goto mask_buffer_column;
    case 0x80f78au:
        goto save_buffer_column_offset;
    case 0x80f78cu:
        goto load_y;
    case 0x80f78du:
        goto shift_y_1;
    case 0x80f78eu:
        goto shift_y_2;
    case 0x80f78fu:
        goto shift_y_3;
    case 0x80f790u:
        goto shift_y_4;
    case 0x80f791u:
        goto save_cell_y;
    case 0x80f793u:
        goto test_y_sign;
    case 0x80f796u:
        goto skip_negative_y;
    case 0x80f798u:
        goto extend_negative_y;
    case 0x80f79bu:
        goto invert_negative_y;
    case 0x80f79eu:
        goto absolute_negative_y;
    case 0x80f79fu:
        goto write_negative_y_dividend;
    case 0x80f7a3u:
        goto narrow_negative_y_divisor;
    case 0x80f7a5u:
        goto load_negative_y_divisor;
    case 0x80f7a7u:
        goto write_negative_y_divisor;
    case 0x80f7abu:
        goto wait_negative_y_division;
    case 0x80f7aeu:
        goto reload_negative_y_height;
    case 0x80f7b0u:
        goto set_negative_y_carry;
    case 0x80f7b1u:
        goto subtract_negative_y_remainder;
    case 0x80f7b5u:
        goto finish_negative_y_wrap;
    case 0x80f7b7u:
        goto compare_y_height;
    case 0x80f7b9u:
        goto skip_y_division;
    case 0x80f7bbu:
        goto write_y_dividend;
    case 0x80f7bfu:
        goto narrow_y_divisor;
    case 0x80f7c1u:
        goto load_y_divisor;
    case 0x80f7c3u:
        goto write_y_divisor;
    case 0x80f7c7u:
        goto wait_y_division;
    case 0x80f7cau:
        goto read_y_remainder;
    case 0x80f7ceu:
        goto save_wrapped_y;
    case 0x80f7d0u:
        goto save_column_cell_y;
    case 0x80f7d2u:
        goto reload_y_for_buffer;
    case 0x80f7d3u:
        goto mask_buffer_row;
    case 0x80f7d6u:
        goto scale_row_1;
    case 0x80f7d7u:
        goto scale_row_2;
    case 0x80f7d8u:
        goto scale_row_3;
    case 0x80f7d9u:
        goto save_buffer_row;
    case 0x80f7dbu:
        goto clear_buffer_offset_carry;
    case 0x80f7dcu:
        goto add_buffer_column;
    case 0x80f7deu:
        goto save_buffer_offset;
    case 0x80f7e0u:
        goto load_buffer_base;
    case 0x80f7e4u:
        goto save_buffer_base;
    case 0x80f7e6u:
        goto narrow_multiplicands;
    case 0x80f7e8u:
        goto load_y_multiplicand;
    case 0x80f7eau:
        goto write_y_multiplicand;
    case 0x80f7eeu:
        goto load_width_multiplicand;
    case 0x80f7f0u:
        goto write_width_multiplicand;
    case 0x80f7f4u:
        goto load_buffer_bank;
    case 0x80f7f6u:
        goto save_buffer_bank;
    case 0x80f7f8u:
        goto widen_cell_address;
    case 0x80f7fau:
        goto load_layer_cells;
    case 0x80f7fdu:
        goto save_layer_cells;
    case 0x80f7ffu:
        goto read_row_product;
    case 0x80f803u:
        goto clear_cell_x_carry;
    case 0x80f804u:
        goto add_cell_x;
    case 0x80f806u:
        goto scale_cell_address;
    case 0x80f807u:
        goto add_layer_cells;
    case 0x80f809u:
        goto save_cell_address;
    case 0x80f80bu:
        goto load_metatile_base;
    case 0x80f80eu:
        goto save_metatile_base;
    case 0x80f810u:
        goto compare_layer;
    case 0x80f813u:
        goto keep_primary_metatiles;
    case 0x80f815u:
        goto load_alternate_metatiles;
    case 0x80f818u:
        goto save_alternate_metatiles;
    case 0x80f81au:
        goto clear_return_carry;
    case 0x80f81bu:
        goto return_short;
    default:
        return 0;
    }
shift_x_1:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f734u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->a >> 1, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf735u);
shift_x_2:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f735u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->a >> 1, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf736u);
shift_x_3:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f736u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->a >> 1, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf737u);
shift_x_4:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f737u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->a >> 1, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf738u);
save_cell_x: {
    const uint16_t low = (uint16_t)(registers->dp + 0x30u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f738u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf73au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_buffer_column: {
    const uint16_t low = (uint16_t)(registers->dp + 0x22u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f73au, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf73cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_layer_width: {
    const uint32_t address = (bank + 0xd010u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + (((0xd010u + registers->x) >> 8) != 0xd0u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !FieldMetatileReadableWord(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f73cu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xf73fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_layer_width: {
    const uint16_t low = (uint16_t)(registers->dp + 0x83u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f73fu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf741u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_layer_height: {
    const uint32_t address = (bank + 0xd018u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + (((0xd018u + registers->x) >> 8) != 0xd0u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !FieldMetatileReadableWord(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f741u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xf744u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_layer_height: {
    const uint16_t low = (uint16_t)(registers->dp + 0x85u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f744u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf746u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_x_for_wrap: {
    const uint16_t low = (uint16_t)(registers->dp + 0x22u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f746u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, low, high, true), false);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf748u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_x_sign:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f748u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0800u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf74bu);
skip_negative_x: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f74bu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xf76cu : 0xf74du);
    if (taken)
        goto write_x_dividend;
}
extend_negative_x:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f74du, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0xf000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf750u);
invert_negative_x:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f750u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x49u, 0xffffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf753u);
absolute_negative_x:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f753u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf754u);
write_negative_x_dividend:
    if (!NativeSpanCanStep(&span, cpu, 6u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f754u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x004204u, 0x004205u, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 6u, 0xf758u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
narrow_negative_x_divisor:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f758u, 2u, span.instructions != 0u);
    registers->mf = true;
    NativeSpanStep(&span, registers, cpu, 3u, 0xf75au);
load_negative_x_divisor: {
    const uint16_t low = (uint16_t)(registers->dp + 0x83u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 3u + direct_penalty) || !Lufia2DecompIsWramByte(low))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f75au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, low, high, false), true);
    NativeSpanStep(&span, registers, cpu, 3u + direct_penalty, 0xf75cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
write_negative_x_divisor:
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f75cu, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x004206u, 0x004207u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xf760u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
wait_negative_x_division:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    {
        if (span.instructions)
            Lufia2SpanProbeOpcode(registers, 0x80f760u);
        const int cycles =
            Lufia2DecompRunDirectCallWithWidths(registers, 0x80f760u, 0x20u, 0x80f81cu, false, true, false);
        if (!cycles)
            goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
reload_negative_x_width: {
    const uint16_t low = (uint16_t)(registers->dp + 0x83u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f763u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, low, high, true), false);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf765u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_negative_x_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f765u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xf766u);
subtract_negative_x_remainder:
    if (!NativeSpanCanStep(&span, cpu, 6u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f766u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, 0x004216u, 0x004217u, true),
                             false);
    NativeSpanStep(&span, registers, cpu, 6u, 0xf76au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
finish_negative_x_wrap:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f76au, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf77fu);
    goto save_wrapped_x;
write_x_dividend:
    if (!NativeSpanCanStep(&span, cpu, 6u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f76cu, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x004204u, 0x004205u, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 6u, 0xf770u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
narrow_x_divisor:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f770u, 2u, span.instructions != 0u);
    registers->mf = true;
    NativeSpanStep(&span, registers, cpu, 3u, 0xf772u);
load_x_divisor: {
    const uint16_t low = (uint16_t)(registers->dp + 0x83u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 3u + direct_penalty) || !Lufia2DecompIsWramByte(low))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f772u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, low, high, false), true);
    NativeSpanStep(&span, registers, cpu, 3u + direct_penalty, 0xf774u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
write_x_divisor:
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f774u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x004206u, 0x004207u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xf778u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
wait_x_division:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    {
        if (span.instructions)
            Lufia2SpanProbeOpcode(registers, 0x80f778u);
        const int cycles =
            Lufia2DecompRunDirectCallWithWidths(registers, 0x80f778u, 0x20u, 0x80f81cu, false, true, false);
        if (!cycles)
            goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
read_x_remainder:
    if (!NativeSpanCanStep(&span, cpu, 6u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f77bu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, 0x004216u, 0x004217u, true),
                             false);
    NativeSpanStep(&span, registers, cpu, 6u, 0xf77fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
save_wrapped_x: {
    const uint16_t low = (uint16_t)(registers->dp + 0x87u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f77fu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf781u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_row_cell_x: {
    const uint16_t low = (uint16_t)(registers->dp + 0x26u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f781u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf783u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reload_cell_x: {
    const uint16_t low = (uint16_t)(registers->dp + 0x30u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f783u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, low, high, true), false);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf785u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
scale_column_1:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f785u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf786u);
scale_column_2:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f786u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf787u);
mask_buffer_column:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f787u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x003eu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf78au);
save_buffer_column_offset: {
    const uint16_t low = (uint16_t)(registers->dp + 0x22u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f78au, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf78cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_y:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f78cu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf78du);
shift_y_1:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f78du, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->a >> 1, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf78eu);
shift_y_2:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f78eu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->a >> 1, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf78fu);
shift_y_3:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f78fu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->a >> 1, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf790u);
shift_y_4:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f790u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->a >> 1, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf791u);
save_cell_y: {
    const uint16_t low = (uint16_t)(registers->dp + 0x28u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f791u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf793u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_y_sign:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f793u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0800u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf796u);
skip_negative_y: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f796u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xf7b7u : 0xf798u);
    if (taken)
        goto compare_y_height;
}
extend_negative_y:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f798u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0xf000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf79bu);
invert_negative_y:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f79bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x49u, 0xffffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf79eu);
absolute_negative_y:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f79eu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf79fu);
write_negative_y_dividend:
    if (!NativeSpanCanStep(&span, cpu, 6u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f79fu, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x004204u, 0x004205u, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 6u, 0xf7a3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
narrow_negative_y_divisor:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7a3u, 2u, span.instructions != 0u);
    registers->mf = true;
    NativeSpanStep(&span, registers, cpu, 3u, 0xf7a5u);
load_negative_y_divisor: {
    const uint16_t low = (uint16_t)(registers->dp + 0x85u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 3u + direct_penalty) || !Lufia2DecompIsWramByte(low))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7a5u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, low, high, false), true);
    NativeSpanStep(&span, registers, cpu, 3u + direct_penalty, 0xf7a7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
write_negative_y_divisor:
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7a7u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x004206u, 0x004207u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xf7abu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
wait_negative_y_division:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    {
        if (span.instructions)
            Lufia2SpanProbeOpcode(registers, 0x80f7abu);
        const int cycles =
            Lufia2DecompRunDirectCallWithWidths(registers, 0x80f7abu, 0x20u, 0x80f81cu, false, true, false);
        if (!cycles)
            goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
reload_negative_y_height: {
    const uint16_t low = (uint16_t)(registers->dp + 0x85u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7aeu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, low, high, true), false);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf7b0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_negative_y_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7b0u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xf7b1u);
subtract_negative_y_remainder:
    if (!NativeSpanCanStep(&span, cpu, 6u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7b1u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, 0x004216u, 0x004217u, true),
                             false);
    NativeSpanStep(&span, registers, cpu, 6u, 0xf7b5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
finish_negative_y_wrap:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7b5u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf7ceu);
    goto save_wrapped_y;
compare_y_height: {
    const uint16_t low = (uint16_t)(registers->dp + 0x85u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7b7u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, low, high, true), false);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf7b9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_y_division: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7b9u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xf7ceu : 0xf7bbu);
    if (taken)
        goto save_wrapped_y;
}
write_y_dividend:
    if (!NativeSpanCanStep(&span, cpu, 6u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7bbu, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x004204u, 0x004205u, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 6u, 0xf7bfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
narrow_y_divisor:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7bfu, 2u, span.instructions != 0u);
    registers->mf = true;
    NativeSpanStep(&span, registers, cpu, 3u, 0xf7c1u);
load_y_divisor: {
    const uint16_t low = (uint16_t)(registers->dp + 0x85u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 3u + direct_penalty) || !Lufia2DecompIsWramByte(low))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7c1u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, low, high, false), true);
    NativeSpanStep(&span, registers, cpu, 3u + direct_penalty, 0xf7c3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
write_y_divisor:
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7c3u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x004206u, 0x004207u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xf7c7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
wait_y_division:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    {
        if (span.instructions)
            Lufia2SpanProbeOpcode(registers, 0x80f7c7u);
        const int cycles =
            Lufia2DecompRunDirectCallWithWidths(registers, 0x80f7c7u, 0x20u, 0x80f81cu, false, true, false);
        if (!cycles)
            goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
read_y_remainder:
    if (!NativeSpanCanStep(&span, cpu, 6u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7cau, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, 0x004216u, 0x004217u, true),
                             false);
    NativeSpanStep(&span, registers, cpu, 6u, 0xf7ceu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
save_wrapped_y: {
    const uint16_t low = (uint16_t)(registers->dp + 0x89u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7ceu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf7d0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_column_cell_y: {
    const uint16_t low = (uint16_t)(registers->dp + 0x28u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7d0u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf7d2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reload_y_for_buffer:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7d2u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf7d3u);
mask_buffer_row:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7d3u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00f0u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf7d6u);
scale_row_1:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7d6u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf7d7u);
scale_row_2:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7d7u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf7d8u);
scale_row_3:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7d8u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf7d9u);
save_buffer_row: {
    const uint16_t low = (uint16_t)(registers->dp + 0x24u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7d9u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf7dbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_buffer_offset_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7dbu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xf7dcu);
add_buffer_column: {
    const uint16_t low = (uint16_t)(registers->dp + 0x22u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7dcu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, low, high, true), false);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf7deu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_buffer_offset: {
    const uint16_t low = (uint16_t)(registers->dp + 0x2du);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7deu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf7e0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_buffer_base: {
    const uint32_t address = (0x838ff0u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !FieldMetatileReadableWord(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7e0u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xf7e4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_buffer_base: {
    const uint16_t low = (uint16_t)(registers->dp + 0x2au);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7e4u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf7e6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_multiplicands:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7e6u, 2u, span.instructions != 0u);
    registers->mf = true;
    NativeSpanStep(&span, registers, cpu, 3u, 0xf7e8u);
load_y_multiplicand: {
    const uint16_t low = (uint16_t)(registers->dp + 0x28u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 3u + direct_penalty) || !Lufia2DecompIsWramByte(low))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7e8u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, low, high, false), true);
    NativeSpanStep(&span, registers, cpu, 3u + direct_penalty, 0xf7eau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
write_y_multiplicand:
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7eau, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x004202u, 0x004203u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xf7eeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_width_multiplicand: {
    const uint16_t low = (uint16_t)(registers->dp + 0x83u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 3u + direct_penalty) || !Lufia2DecompIsWramByte(low))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7eeu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, low, high, false), true);
    NativeSpanStep(&span, registers, cpu, 3u + direct_penalty, 0xf7f0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
write_width_multiplicand:
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7f0u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x004203u, 0x004204u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xf7f4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_buffer_bank:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7f4u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x007eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf7f6u);
save_buffer_bank: {
    const uint16_t low = (uint16_t)(registers->dp + 0x2cu);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 3u + direct_penalty) || !Lufia2DecompIsWramByte(low))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7f6u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 3u + direct_penalty, 0xf7f8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_cell_address:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7f8u, 2u, span.instructions != 0u);
    registers->mf = false;
    NativeSpanStep(&span, registers, cpu, 3u, 0xf7fau);
load_layer_cells: {
    const uint32_t address = (bank + 0xd008u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + (((0xd008u + registers->x) >> 8) != 0xd0u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !FieldMetatileReadableWord(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7fau, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xf7fdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_layer_cells: {
    const uint16_t low = (uint16_t)(registers->dp + 0x8du);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7fdu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf7ffu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_row_product:
    if (!NativeSpanCanStep(&span, cpu, 6u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f7ffu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, 0x004216u, 0x004217u, true),
                             false);
    NativeSpanStep(&span, registers, cpu, 6u, 0xf803u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
clear_cell_x_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f803u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xf804u);
add_cell_x: {
    const uint16_t low = (uint16_t)(registers->dp + 0x26u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f804u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, low, high, true), false);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf806u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
scale_cell_address:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f806u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf807u);
add_layer_cells: {
    const uint16_t low = (uint16_t)(registers->dp + 0x8du);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f807u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, low, high, true), false);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf809u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_cell_address: {
    const uint16_t low = (uint16_t)(registers->dp + 0x30u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f809u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf80bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_metatile_base: {
    const uint32_t address = bank | 0xd03cu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !FieldMetatileReadableWord(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f80bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xf80eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_metatile_base: {
    const uint16_t low = (uint16_t)(registers->dp + 0x65u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f80eu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf810u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_layer:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f810u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, 0x0004u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf813u);
keep_primary_metatiles: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f813u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xf81au : 0xf815u);
    if (taken)
        goto clear_return_carry;
}
load_alternate_metatiles: {
    const uint32_t address = bank | 0xd040u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !FieldMetatileReadableWord(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f815u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xf818u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_alternate_metatiles: {
    const uint16_t low = (uint16_t)(registers->dp + 0x65u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f818u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf81au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_return_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f81au, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xf81bu);
return_short:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80f81bu, 1u, span.instructions != 0u);
    {
        uint16_t address = registers->read(registers->mem, ++registers->sp);
        address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        NativeSpanStep(&span, registers, cpu, 6u, (uint16_t)(address + 1u));
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
