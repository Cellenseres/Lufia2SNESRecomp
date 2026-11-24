#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunDecompressReference(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                              uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                              unsigned step_budget, long *step_count) {
    const uint32_t alias = pc >= 0x808fe8u ? 0xa4u : 0u;
    const uint32_t logical_pc = pc - alias;
    uint32_t end_pc;
    if (logical_pc >= 0x808f44u && logical_pc < 0x808f5cu)
        end_pc = 0x808f5cu;
    else if (logical_pc >= 0x808f5cu && logical_pc < 0x808f66u)
        end_pc = 0x808f66u;
    else if (logical_pc >= 0x808f66u && logical_pc < 0x808f69u)
        end_pc = 0x808f69u;
    else if (logical_pc >= 0x808f69u && logical_pc < 0x808f6fu)
        end_pc = 0x808f6fu;
    else if (logical_pc >= 0x808f6fu && logical_pc < 0x808f93u)
        end_pc = 0x808f93u;
    else
        return 0;
    const bool changing_width = logical_pc == 0x808f53u || logical_pc == 0x808f6bu || logical_pc == 0x808f80u;
    const bool byte_entry = logical_pc < 0x808f55u || (logical_pc >= 0x808f6du && logical_pc < 0x808f82u);
    if (!cpu || !cpu->ram || registers->e || registers->xf || !step_budget ||
        (!changing_width && registers->mf != byte_entry) || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        (logical_pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc + alias)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = logical_pc - 0x808f44u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    const uint8_t work_bank = alias ? 0x7fu : 0x7eu;
    if (cart->romSize < first + 79u ||
        memcmp(cart->rom + first,
               "\xc8\xf0\xeb\xeb\xb7\x5d\x29\x0f\xf0\x21\x1a\x85\x56\xb7\x5d\xc2\x20\x4a\x4a\x4a\x4a\x09\x00"
               "\xf0\x86\x5a\x5a\x9b\x18\x65\x5a\xaa\xa5\x56\x54",
               35u) ||
        cart->rom[first + 35u] != work_bank || cart->rom[first + 36u] != work_bank ||
        memcmp(cart->rom + first + 37u,
               "\xbb\x7a\xe2\x20\x80\xa5\xb7\x5d\xda\xaa\xc8\xf0\xc3\xb7\x5d\x85\x5b\x29\x3f\x1a\x1a\x85\x56"
               "\xc2\x20\x8a\x4a\x4a\x4a\x4a\x09\x00\xf0\x06\x5a\x2a\x06\x5a\x2a\xfa\x80\xc9",
               42u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (logical_pc) {
    case 0x808f44u: goto advance_header;
    case 0x808f45u: goto header_wrapped;
    case 0x808f47u: goto exchange_header;
    case 0x808f48u: goto read_header;
    case 0x808f4au: goto mask_short_length;
    case 0x808f4cu: goto choose_long;
    case 0x808f4eu: goto increment_short_length;
    case 0x808f4fu: goto store_short_length;
    case 0x808f51u: goto read_short_offset;
    case 0x808f53u: goto widen_short_offset;
    case 0x808f55u: goto shift_short_1;
    case 0x808f56u: goto shift_short_2;
    case 0x808f57u: goto shift_short_3;
    case 0x808f58u: goto shift_short_4;
    case 0x808f59u: goto extend_short_offset;
    case 0x808f5cu: goto store_copy_destination;
    case 0x808f5eu: goto push_stream_position;
    case 0x808f5fu: goto select_copy_destination;
    case 0x808f60u: goto clear_copy_carry;
    case 0x808f61u: goto add_destination;
    case 0x808f63u: goto select_copy_source;
    case 0x808f64u: goto load_copy_count;
    case 0x808f66u: goto move_byte;
    case 0x808f69u: goto take_copy_destination;
    case 0x808f6au: goto restore_stream_position;
    case 0x808f6bu: goto narrow_copy_result;
    case 0x808f6du: goto continue_flags;
    case 0x808f6fu: goto read_long_offset;
    case 0x808f71u: goto push_copy_destination;
    case 0x808f72u: goto save_long_offset;
    case 0x808f73u: goto advance_long_header;
    case 0x808f74u: goto long_header_wrapped;
    case 0x808f76u: goto read_long_length;
    case 0x808f78u: goto store_long_bits;
    case 0x808f7au: goto mask_long_length;
    case 0x808f7cu: goto increment_long_1;
    case 0x808f7du: goto increment_long_2;
    case 0x808f7eu: goto store_long_length;
    case 0x808f80u: goto widen_long_offset;
    case 0x808f82u: goto restore_long_offset;
    case 0x808f83u: goto shift_long_1;
    case 0x808f84u: goto shift_long_2;
    case 0x808f85u: goto shift_long_3;
    case 0x808f86u: goto shift_long_4;
    case 0x808f87u: goto extend_long_offset;
    case 0x808f8au: goto shift_long_bits_1;
    case 0x808f8cu: goto rotate_long_1;
    case 0x808f8du: goto shift_long_bits_2;
    case 0x808f8fu: goto rotate_long_2;
    case 0x808f90u: goto restore_copy_destination;
    case 0x808f91u: goto begin_copy;
    default: return 0;
    }
advance_header:
    pc = 0x808f44u;
    goto advance_stream;
header_wrapped:
    pc = 0x808f45u;
    goto branch_zero;
exchange_header:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f47u + alias, 1u, span.instructions != 0u);
    registers->a = (uint16_t)((registers->a << 8) | (registers->a >> 8));
    registers->z = (uint8_t)registers->a == 0u;
    registers->n = (registers->a & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 3u, (uint16_t)(0x8f48u + alias));
read_header:
    pc = 0x808f48u;
    goto read_stream;
mask_short_length:
    pc = 0x808f4au;
    goto mask_length;
choose_long:
    pc = 0x808f4cu;
    goto branch_zero;
increment_short_length:
    pc = 0x808f4eu;
    goto increment_length;
store_short_length:
    pc = 0x808f4fu;
    goto direct_operand;
read_short_offset:
    pc = 0x808f51u;
    goto read_stream;
widen_short_offset:
    pc = 0x808f53u;
    goto change_width;
shift_short_1:
    pc = 0x808f55u;
    goto shift_offset;
shift_short_2:
    pc = 0x808f56u;
    goto shift_offset;
shift_short_3:
    pc = 0x808f57u;
    goto shift_offset;
shift_short_4:
    pc = 0x808f58u;
    goto shift_offset;
extend_short_offset:
    pc = 0x808f59u;
    goto extend_offset;
store_copy_destination:
    pc = 0x808f5cu;
    goto direct_operand;
push_stream_position:
    pc = 0x808f5eu;
    goto push_index;
select_copy_destination:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f5fu + alias, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->x, false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(0x8f60u + alias));
clear_copy_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f60u + alias, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(0x8f61u + alias));
add_destination:
    pc = 0x808f61u;
    goto direct_operand;
select_copy_source:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f63u + alias, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(0x8f64u + alias));
load_copy_count:
    pc = 0x808f64u;
    goto direct_operand;
move_byte:
    if (!NativeSpanCanStep(&span, cpu, 7u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f66u + alias, 3u, span.instructions != 0u);
    registers->db = work_bank;
    registers->write(registers->mem, ((uint32_t)work_bank << 16) | registers->y,
                     registers->read(registers->mem, ((uint32_t)work_bank << 16) | registers->x));
    registers->a = (uint16_t)(registers->a - 1u);
    registers->x = (uint16_t)(registers->x + 1u);
    registers->y = (uint16_t)(registers->y + 1u);
    NativeSpanStep(&span, registers, cpu, 7u,
                   (uint16_t)((registers->a == 0xffffu ? 0x8f69u : 0x8f66u) + alias));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
take_copy_destination:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f69u + alias, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(0x8f6au + alias));
restore_stream_position:
    pc = 0x808f6au;
    goto pull_index;
narrow_copy_result:
    pc = 0x808f6bu;
    goto change_width;
continue_flags:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f6du + alias, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, (uint16_t)(0x8f14u + alias));
    goto finished;
read_long_offset:
    pc = 0x808f6fu;
    goto read_stream;
push_copy_destination:
    pc = 0x808f71u;
    goto push_index;
save_long_offset:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f72u + alias, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(0x8f73u + alias));
advance_long_header:
    pc = 0x808f73u;
    goto advance_stream;
long_header_wrapped:
    pc = 0x808f74u;
    goto branch_zero;
read_long_length:
    pc = 0x808f76u;
    goto read_stream;
store_long_bits:
    pc = 0x808f78u;
    goto direct_operand;
mask_long_length:
    pc = 0x808f7au;
    goto mask_length;
increment_long_1:
    pc = 0x808f7cu;
    goto increment_length;
increment_long_2:
    pc = 0x808f7du;
    goto increment_length;
store_long_length:
    pc = 0x808f7eu;
    goto direct_operand;
widen_long_offset:
    pc = 0x808f80u;
    goto change_width;
restore_long_offset:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f82u + alias, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->x, false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(0x8f83u + alias));
shift_long_1:
    pc = 0x808f83u;
    goto shift_offset;
shift_long_2:
    pc = 0x808f84u;
    goto shift_offset;
shift_long_3:
    pc = 0x808f85u;
    goto shift_offset;
shift_long_4:
    pc = 0x808f86u;
    goto shift_offset;
extend_long_offset:
    pc = 0x808f87u;
    goto extend_offset;
shift_long_bits_1:
    pc = 0x808f8au;
    goto direct_operand;
rotate_long_1:
    pc = 0x808f8cu;
    goto rotate_offset;
shift_long_bits_2:
    pc = 0x808f8du;
    goto direct_operand;
rotate_long_2:
    pc = 0x808f8fu;
    goto rotate_offset;
restore_copy_destination:
    pc = 0x808f90u;
    goto pull_index;
begin_copy:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f91u + alias, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, (uint16_t)(0x8f5cu + alias));
    goto finished;
advance_stream:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, pc + alias, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(pc + alias + 1u));
    if (pc == 0x808f44u)
        goto header_wrapped;
    goto long_header_wrapped;
branch_zero: {
    const unsigned cycles = registers->z ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, pc + alias, 2u, span.instructions != 0u);
    const uint32_t target = pc == 0x808f45u ? 0x808f32u : pc == 0x808f4cu ? 0x808f6fu : 0x808f39u;
    NativeSpanStep(&span, registers, cpu, cycles, (uint16_t)((registers->z ? target : pc + 2u) + alias));
    if (registers->z)
        goto finished;
    if (pc == 0x808f45u)
        goto exchange_header;
    if (pc == 0x808f4cu)
        goto increment_short_length;
    goto read_long_length;
}
read_stream: {
    const uint16_t low = (uint16_t)(registers->dp + 0x5du);
    const uint16_t high = (uint16_t)(low + 1u);
    const uint16_t bank = (uint16_t)(low + 2u);
    const unsigned cycles = 6u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high) || !Lufia2DecompIsWramByte(bank))
        goto finished;
    const uint32_t pointer =
        cpu->ram[low] | ((uint32_t)cpu->ram[high] << 8) | ((uint32_t)cpu->ram[bank] << 16);
    if (!Lufia2DecompIsDecompressSourceByte(cart, (pointer + registers->y) & 0xffffffu))
        goto finished;
    Lufia2SpanFetch(registers, pc + alias, 2u, span.instructions != 0u);
    uint32_t address = Lufia2DecompReadData(registers, low, high, true);
    address |= (uint32_t)registers->read(registers->mem, bank) << 16;
    address = (address + registers->y) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, (uint16_t)(pc + alias + 2u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    if (pc == 0x808f48u)
        goto mask_short_length;
    if (pc == 0x808f51u)
        goto widen_short_offset;
    if (pc == 0x808f6fu)
        goto push_copy_destination;
    goto store_long_bits;
}
increment_length:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, pc + alias, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(pc + alias + 1u));
    if (pc == 0x808f4eu)
        goto store_short_length;
    if (pc == 0x808f7cu)
        goto increment_long_2;
    goto store_long_length;
mask_length:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, pc + alias, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, pc == 0x808f4au ? 0x0fu : 0x3fu, true);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(pc + alias + 2u));
    if (pc == 0x808f4au)
        goto choose_long;
    goto increment_long_1;
change_width:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, pc + alias, 2u, span.instructions != 0u);
    registers->mf = pc == 0x808f6bu;
    NativeSpanStep(&span, registers, cpu, 3u, (uint16_t)(pc + alias + 2u));
    if (pc == 0x808f53u)
        goto shift_short_1;
    if (pc == 0x808f6bu)
        goto continue_flags;
    goto restore_long_offset;
shift_offset:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, pc + alias, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->a >> 1, false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(pc + alias + 1u));
    switch (pc) {
    case 0x808f55u: goto shift_short_2;
    case 0x808f56u: goto shift_short_3;
    case 0x808f57u: goto shift_short_4;
    case 0x808f58u: goto extend_short_offset;
    case 0x808f83u: goto shift_long_2;
    case 0x808f84u: goto shift_long_3;
    case 0x808f85u: goto shift_long_4;
    default: goto extend_long_offset;
    }
extend_offset:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, pc + alias, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0xf000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, (uint16_t)(pc + alias + 3u));
    if (pc == 0x808f59u)
        goto finished;
    goto shift_long_bits_1;
rotate_offset: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, pc + alias, 1u, span.instructions != 0u);
    const uint16_t value = (uint16_t)((registers->a << 1) | registers->c);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, value, false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(pc + alias + 1u));
    if (pc == 0x808f8cu)
        goto shift_long_bits_2;
    goto restore_copy_destination;
}
push_index:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, pc + alias, 1u, span.instructions != 0u);
    {
        const uint16_t value = pc == 0x808f5eu ? registers->y : registers->x;
        registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
        registers->write(registers->mem, registers->sp--, (uint8_t)value);
    }
    NativeSpanStep(&span, registers, cpu, 4u, (uint16_t)(pc + alias + 1u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    if (pc == 0x808f5eu)
        goto select_copy_destination;
    goto save_long_offset;
pull_index:
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, pc + alias, 1u, span.instructions != 0u);
    {
        uint16_t value = registers->read(registers->mem, ++registers->sp);
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        Lufia2DecompApplyOperand(registers, pc == 0x808f6au ? 0xa0u : 0xa2u, value, false);
    }
    NativeSpanStep(&span, registers, cpu, 5u, (uint16_t)(pc + alias + 1u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    if (pc == 0x808f6au)
        goto narrow_copy_result;
    goto begin_copy;
direct_operand: {
    const bool shift = pc == 0x808f8au || pc == 0x808f8du;
    const bool word = shift || pc == 0x808f5cu || pc == 0x808f61u || pc == 0x808f64u;
    const unsigned displacement = pc == 0x808f78u                                 ? 0x5bu
                                  : (shift || pc == 0x808f5cu || pc == 0x808f61u) ? 0x5au
                                                                                  : 0x56u;
    const uint16_t low = (uint16_t)(registers->dp + displacement);
    const uint16_t high = (uint16_t)(low + 1u);
    const unsigned cycles = (shift ? 7u : word ? 4u : 3u) + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(low) ||
        (word && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, pc + alias, 2u, span.instructions != 0u);
    if (shift)
        Lufia2DecompModifyWram(registers, 0x06u, low, high, true);
    else if (pc == 0x808f61u || pc == 0x808f64u)
        Lufia2DecompApplyOperand(registers, pc == 0x808f61u ? 0x69u : 0xa9u,
                                 Lufia2DecompReadData(registers, low, high, true), false);
    else
        Lufia2DecompWriteData(registers, low, high, pc == 0x808f5cu ? registers->x : registers->a, word);
    NativeSpanStep(&span, registers, cpu, cycles, (uint16_t)(pc + alias + 2u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    switch (pc) {
    case 0x808f4fu: goto read_short_offset;
    case 0x808f5cu: goto push_stream_position;
    case 0x808f61u: goto select_copy_source;
    case 0x808f64u: goto finished;
    case 0x808f78u: goto mask_long_length;
    case 0x808f7eu: goto widen_long_offset;
    case 0x808f8au: goto rotate_long_1;
    default: goto rotate_long_2;
    }
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
