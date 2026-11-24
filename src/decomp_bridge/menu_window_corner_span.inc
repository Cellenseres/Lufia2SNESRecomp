static bool Lufia2DecompIsWindowPatternByte(const Cart *cart, uint32_t address) {
    return Lufia2DecompIsWramByte(address) ||
        Lufia2DecompIsLoRomDataByte(cart, address | 0x800000u);
}

static int Lufia2DecompRunMenuWindowCorner(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    const uint32_t first_pc = pc >= 0x828259u ? 0x828259u : 0x82820bu;
    if (pc < first_pc || pc > first_pc + 36u || !cpu || !cpu->ram || registers->e ||
        (pc != first_pc + 36u && (registers->mf || registers->xf)) ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || !step_budget ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(first_pc, first_pc + 37u))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned phase = pc - first_pc;
    if (!cart || offset < phase) return 0;
    const unsigned first = offset - phase;
    const bool three_rows = first_pc == 0x828259u;
    const char *signature = three_rows
        ? "\xbe\x7e\x82\x86\x5d\xaa\xa0\x00\x00\xb1\x5d\x9f\x00\x20\x7e\xc8\xc8\xb1\x5d\x9f\x02\x20\x7e\xc8\xc8\x8a\x18\x69\x40\x00\xaa\xc0\x0c\x00\xd0\xe5\x60"
        : "\xbe\x7e\x82\x86\x5d\xaa\xa0\x00\x00\xb1\x5d\x9f\x00\x20\x7e\xc8\xc8\xb1\x5d\x9f\x02\x20\x7e\xc8\xc8\x8a\x18\x69\x40\x00\xaa\xc0\x08\x00\xd0\xe5\x60";
    if (cart->romSize < first + 37u || memcmp(cart->rom + first, signature, 37u)) return 0;
    const uint16_t pointer_low = (uint16_t)(registers->dp + 0x5du);
    const uint16_t pointer_high = (uint16_t)(registers->dp + 0x5eu);
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (phase) {
    case 0u: goto load_pattern_pointer;
    case 3u: goto store_pattern_pointer;
    case 5u: goto load_tile_offset;
    case 6u: goto clear_pattern_offset;
    case 9u: goto load_first_word;
    case 11u: goto store_first_word;
    case 15u: goto advance_first_low;
    case 16u: goto advance_first_high;
    case 17u: goto load_second_word;
    case 19u: goto store_second_word;
    case 23u: goto advance_second_low;
    case 24u: goto advance_second_high;
    case 25u: goto load_row_offset;
    case 26u: goto clear_row_carry;
    case 27u: goto advance_row;
    case 30u: goto store_row_offset;
    case 31u: goto compare_rows;
    case 34u: goto repeat_row;
    case 36u: goto return_short;
    default: return 0;
    }
load_pattern_pointer:
    {
        const uint32_t address = (((uint32_t)registers->db << 16) + 0x827eu + registers->y) & 0xffffffu;
        const uint32_t high = (address + 1u) & 0xffffffu;
        const unsigned cycles = 5u + ((0x827eu + registers->y) >> 8 != 0x82u);
        if (!NativeSpanCanStep(&span, cpu, cycles) ||
            !Lufia2DecompIsWindowPatternByte(cart, address) || !Lufia2DecompIsWindowPatternByte(cart, high))
            goto finished;
        Lufia2SpanFetch(registers, first_pc, 3u, span.instructions != 0u);
        Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
        NativeSpanStep(&span, registers, cpu, cycles, (uint16_t)(first_pc + 3u));
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
store_pattern_pointer:
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) ||
        !Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high))
        goto finished;
    Lufia2SpanFetch(registers, first_pc + 3u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, pointer_low, pointer_high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, (uint16_t)(first_pc + 5u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_tile_offset:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, first_pc + 5u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(first_pc + 6u));
clear_pattern_offset:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, first_pc + 6u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0u, false);
    NativeSpanStep(&span, registers, cpu, 3u, (uint16_t)(first_pc + 9u));
load_first_word:
    pc = first_pc + 9u;
    goto load_pattern;
load_second_word:
    pc = first_pc + 17u;
load_pattern:
    {
        if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high)) goto finished;
        const unsigned expected = cpu->ram[pointer_low] | ((unsigned)cpu->ram[pointer_high] << 8);
        const uint32_t address = (((uint32_t)registers->db << 16) + expected + registers->y) & 0xffffffu;
        const uint32_t high = (address + 1u) & 0xffffffu;
        const unsigned cycles = 6u + direct_penalty + ((expected >> 8) != ((expected + registers->y) >> 8));
        if (!NativeSpanCanStep(&span, cpu, cycles) ||
            !Lufia2DecompIsWindowPatternByte(cart, address) || !Lufia2DecompIsWindowPatternByte(cart, high))
            goto finished;
        Lufia2SpanFetch(registers, pc, 2u, span.instructions != 0u);
        const uint16_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
        const uint32_t source = (((uint32_t)registers->db << 16) + pointer + registers->y) & 0xffffffu;
        Lufia2DecompApplyOperand(registers, 0xa9u,
            Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
        NativeSpanStep(&span, registers, cpu, cycles, (uint16_t)(pc + 2u));
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
        if (pc == first_pc + 9u) goto store_first_word;
    }
store_second_word:
    pc = first_pc + 19u;
    goto store_pattern;
store_first_word:
    pc = first_pc + 11u;
store_pattern:
    {
        const uint32_t address = 0x7e2000u + registers->x + (pc == first_pc + 11u ? 0u : 2u);
        if (!NativeSpanCanStep(&span, cpu, 6u) || !Lufia2DecompIsWramByte(address) ||
            !Lufia2DecompIsWramByte(address + 1u))
            goto finished;
        Lufia2SpanFetch(registers, pc, 4u, span.instructions != 0u);
        Lufia2DecompWriteData(registers, address, address + 1u, registers->a, true);
        NativeSpanStep(&span, registers, cpu, 6u, (uint16_t)(pc + 4u));
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
        if (pc == first_pc + 19u) goto advance_second_low;
    }
advance_first_low:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, first_pc + 15u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(first_pc + 16u));
advance_first_high:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, first_pc + 16u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(first_pc + 17u));
    goto load_second_word;
advance_second_low:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, first_pc + 23u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(first_pc + 24u));
advance_second_high:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, first_pc + 24u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(first_pc + 25u));
load_row_offset:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, first_pc + 25u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->x, false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(first_pc + 26u));
clear_row_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, first_pc + 26u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(first_pc + 27u));
advance_row:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, first_pc + 27u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x40u, false);
    NativeSpanStep(&span, registers, cpu, 3u, (uint16_t)(first_pc + 30u));
store_row_offset:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, first_pc + 30u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(first_pc + 31u));
compare_rows:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, first_pc + 31u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc0u, three_rows ? 12u : 8u, false);
    NativeSpanStep(&span, registers, cpu, 3u, (uint16_t)(first_pc + 34u));
repeat_row:
    {
        const unsigned cycles = registers->z ? 2u : 3u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, first_pc + 34u, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, (uint16_t)(first_pc + (registers->z ? 36u : 9u)));
        if (!registers->z) goto load_first_word;
    }
return_short:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, first_pc + 36u, 1u, span.instructions != 0u);
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
