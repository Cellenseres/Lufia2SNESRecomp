static int Lufia2DecompRunFieldRowBufferSetup(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    if (pc < 0x80f6c6u || pc > 0x80f6deu || !cpu || !cpu->ram ||
        registers->e || registers->mf || registers->xf || !step_budget ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x80f6c6u, 0x80f6dfu))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x80f6c6u;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 25u || memcmp(cart->rom + first,
        "\xa9\x10\x00\x85\x26\x98\x29\xc0\xff\x18\x65\x2a\x85\x5d\x18"
        "\x69\x40\x00\x85\x60\x98\x29\x3f\x00\xa8", 25u))
        return 0;
    const uint16_t count_low = (uint16_t)(registers->dp + 0x26u);
    const uint16_t count_high = (uint16_t)(registers->dp + 0x27u);
    const uint16_t buffer_low = (uint16_t)(registers->dp + 0x2au);
    const uint16_t buffer_high = (uint16_t)(registers->dp + 0x2bu);
    const uint16_t row_low = (uint16_t)(registers->dp + 0x5du);
    const uint16_t row_high = (uint16_t)(registers->dp + 0x5eu);
    const uint16_t next_row_low = (uint16_t)(registers->dp + 0x60u);
    const uint16_t next_row_high = (uint16_t)(registers->dp + 0x61u);
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x80f6c6u: goto load_count;
    case 0x80f6c9u: goto store_count;
    case 0x80f6cbu: goto load_row;
    case 0x80f6ccu: goto mask_row;
    case 0x80f6cfu: goto clear_buffer_carry;
    case 0x80f6d0u: goto add_buffer;
    case 0x80f6d2u: goto store_row;
    case 0x80f6d4u: goto clear_row_carry;
    case 0x80f6d5u: goto add_row_stride;
    case 0x80f6d8u: goto store_next_row;
    case 0x80f6dau: goto load_column;
    case 0x80f6dbu: goto mask_column;
    case 0x80f6deu: goto store_column;
    default: return 0;
    }
load_count:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f6c6u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 16u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf6c9u);
store_count:
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) ||
        !Lufia2DecompIsWramByte(count_low) || !Lufia2DecompIsWramByte(count_high)) goto finished;
    Lufia2SpanFetch(registers, 0x80f6c9u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, count_low, count_high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf6cbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_row:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f6cbu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf6ccu);
mask_row:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f6ccu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0xffc0u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf6cfu);
clear_buffer_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f6cfu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xf6d0u);
add_buffer:
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) ||
        !Lufia2DecompIsWramByte(buffer_low) || !Lufia2DecompIsWramByte(buffer_high)) goto finished;
    Lufia2SpanFetch(registers, 0x80f6d0u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u,
        Lufia2DecompReadData(registers, buffer_low, buffer_high, true), false);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf6d2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
store_row:
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) ||
        !Lufia2DecompIsWramByte(row_low) || !Lufia2DecompIsWramByte(row_high)) goto finished;
    Lufia2SpanFetch(registers, 0x80f6d2u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, row_low, row_high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf6d4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
clear_row_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f6d4u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xf6d5u);
add_row_stride:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f6d5u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 64u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf6d8u);
store_next_row:
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) ||
        !Lufia2DecompIsWramByte(next_row_low) || !Lufia2DecompIsWramByte(next_row_high)) goto finished;
    Lufia2SpanFetch(registers, 0x80f6d8u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, next_row_low, next_row_high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0xf6dau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_column:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f6dau, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf6dbu);
mask_column:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f6dbu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 63u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf6deu);
store_column:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f6deu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf6dfu);
finished:
    if (!span.instructions) return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
