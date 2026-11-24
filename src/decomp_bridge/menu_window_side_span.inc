static int Lufia2DecompRunMenuWindowSide(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    if (pc < 0x828230u || pc > 0x828258u || !cpu || !cpu->ram || registers->e ||
        (pc != 0x828258u && (registers->mf || registers->xf)) ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || !step_budget ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x828230u, 0x828259u))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned phase = pc - 0x828230u;
    if (!cart || offset < phase) return 0;
    const unsigned first = offset - phase;
    if (cart->romSize < first + 41u || memcmp(cart->rom + first,
        "\xbe\x7e\x82\x86\x5d\xaa\xa0\x00\x00\xa5\x58\x38\xe9\x04\x00\xf0\x17\x85\x5a"
        "\xb1\x5d\x9f\x00\x20\x7e\x8a\x18\x69\x40\x00\xaa\x98\x49\x02\x00\xa8\xc6\x5a\xd0\xeb\x60", 41u))
        return 0;
    const uint16_t pointer_low = (uint16_t)(registers->dp + 0x5du);
    const uint16_t pointer_high = (uint16_t)(registers->dp + 0x5eu);
    const uint16_t height_low = (uint16_t)(registers->dp + 0x58u);
    const uint16_t height_high = (uint16_t)(registers->dp + 0x59u);
    const uint16_t count_low = (uint16_t)(registers->dp + 0x5au);
    const uint16_t count_high = (uint16_t)(registers->dp + 0x5bu);
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (phase) {
    case 0u: goto load_pattern_pointer;
    case 3u: goto store_pattern_pointer;
    case 5u: goto load_tile_offset;
    case 6u: goto clear_pattern_offset;
    case 9u: goto load_height;
    case 11u: goto set_height_carry;
    case 12u: goto subtract_corners;
    case 15u: goto skip_empty_edge;
    case 17u: goto store_height;
    case 19u: goto load_pattern;
    case 21u: goto store_pattern;
    case 25u: goto load_row_offset;
    case 26u: goto clear_row_carry;
    case 27u: goto advance_row;
    case 30u: goto store_row_offset;
    case 31u: goto load_pattern_offset;
    case 32u: goto alternate_pattern;
    case 35u: goto store_pattern_offset;
    case 36u: goto count_rows;
    case 38u: goto repeat_row;
    case 40u: goto return_short;
    default: return 0;
    }
load_pattern_pointer:
    {
        const uint32_t address = (((uint32_t)registers->db << 16) + 0x827eu + registers->y) & 0xffffffu;
        const uint32_t high = (address + 1u) & 0xffffffu;
        const unsigned cycles = 5u + (((0x827eu + registers->y) >> 8) != 0x82u);
        if (!NativeSpanCanStep(&span, cpu, cycles) ||
            !Lufia2DecompIsWindowPatternByte(cart, address) || !Lufia2DecompIsWindowPatternByte(cart, high))
            goto finished;
        Lufia2SpanFetch(registers, 0x828230u, 3u, span.instructions != 0u);
        Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
        NativeSpanStep(&span, registers, cpu, cycles, 0x8233u);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
store_pattern_pointer:
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) ||
        !Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high)) goto finished;
    Lufia2SpanFetch(registers, 0x828233u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, pointer_low, pointer_high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0x8235u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_tile_offset:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x828235u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8236u);
clear_pattern_offset:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x828236u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8239u);
load_height:
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) ||
        !Lufia2DecompIsWramByte(height_low) || !Lufia2DecompIsWramByte(height_high)) goto finished;
    Lufia2SpanFetch(registers, 0x828239u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, height_low, height_high, true), false);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0x823bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
set_height_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x82823bu, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0x823cu);
subtract_corners:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x82823cu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 4u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x823fu);
skip_empty_edge:
    {
        const unsigned cycles = registers->z ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x82823fu, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, registers->z ? 0x8258u : 0x8241u);
        if (registers->z) goto return_short;
    }
store_height:
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) ||
        !Lufia2DecompIsWramByte(count_low) || !Lufia2DecompIsWramByte(count_high)) goto finished;
    Lufia2SpanFetch(registers, 0x828241u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, count_low, count_high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0x8243u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_pattern:
    {
        if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high)) goto finished;
        const unsigned expected = cpu->ram[pointer_low] | ((unsigned)cpu->ram[pointer_high] << 8);
        const uint32_t address = (((uint32_t)registers->db << 16) + expected + registers->y) & 0xffffffu;
        const uint32_t high = (address + 1u) & 0xffffffu;
        const unsigned cycles = 6u + direct_penalty + ((expected >> 8) != ((expected + registers->y) >> 8));
        if (!NativeSpanCanStep(&span, cpu, cycles) ||
            !Lufia2DecompIsWindowPatternByte(cart, address) || !Lufia2DecompIsWindowPatternByte(cart, high)) goto finished;
        Lufia2SpanFetch(registers, 0x828243u, 2u, span.instructions != 0u);
        const uint16_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
        const uint32_t source = (((uint32_t)registers->db << 16) + pointer + registers->y) & 0xffffffu;
        Lufia2DecompApplyOperand(registers, 0xa9u,
            Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
        NativeSpanStep(&span, registers, cpu, cycles, 0x8245u);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
store_pattern:
    {
        const uint32_t address = 0x7e2000u + registers->x;
        if (!NativeSpanCanStep(&span, cpu, 6u) || !Lufia2DecompIsWramByte(address) ||
            !Lufia2DecompIsWramByte(address + 1u)) goto finished;
        Lufia2SpanFetch(registers, 0x828245u, 4u, span.instructions != 0u);
        Lufia2DecompWriteData(registers, address, address + 1u, registers->a, true);
        NativeSpanStep(&span, registers, cpu, 6u, 0x8249u);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
load_row_offset:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x828249u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->x, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x824au);
clear_row_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x82824au, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x824bu);
advance_row:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x82824bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x40u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x824eu);
store_row_offset:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x82824eu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x824fu);
load_pattern_offset:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x82824fu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8250u);
alternate_pattern:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x828250u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x49u, 2u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8253u);
store_pattern_offset:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x828253u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8254u);
count_rows:
    if (!NativeSpanCanStep(&span, cpu, 7u + direct_penalty) ||
        !Lufia2DecompIsWramByte(count_low) || !Lufia2DecompIsWramByte(count_high)) goto finished;
    Lufia2SpanFetch(registers, 0x828254u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, count_low, count_high, true);
    NativeSpanStep(&span, registers, cpu, 7u + direct_penalty, 0x8256u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
repeat_row:
    {
        const unsigned cycles = registers->z ? 2u : 3u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x828256u, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, registers->z ? 0x8258u : 0x8243u);
        if (!registers->z) goto load_pattern;
    }
return_short:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu) goto finished;
    Lufia2SpanFetch(registers, 0x828258u, 1u, span.instructions != 0u);
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
