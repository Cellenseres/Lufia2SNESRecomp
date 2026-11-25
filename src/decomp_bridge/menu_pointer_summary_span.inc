#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuPointerOffsetSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x808af7u && pc < 0x808b08u)
        end_pc = 0x808b08u;
    else if (pc >= 0x808b08u && pc < 0x808b09u)
        end_pc = 0x808b09u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x808af7u: width_neutral = true; break;
    case 0x808b06u: width_neutral = true; break;
    case 0x808b08u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x808af7u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 18u ||
        memcmp(cart->rom + first, "\xc2\x20\xb7\x5d\x85\x58\xc8\xc8\xb2\x58\x18\x65\x56\x85\x60\xe2\x20\x60",
               18u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x808af7u: goto widen_pointer;
    case 0x808af9u: goto read_pointer_operand;
    case 0x808afbu: goto store_pointer_operand;
    case 0x808afdu: goto advance_operand_low;
    case 0x808afeu: goto advance_operand_high;
    case 0x808affu: goto read_indirect_value;
    case 0x808b01u: goto clear_offset_carry;
    case 0x808b02u: goto add_pointer_offset;
    case 0x808b04u: goto store_result_pointer;
    case 0x808b06u: goto narrow_pointer;
    case 0x808b08u: goto return_pointer_offset;
    default: return 0;
    }
widen_pointer: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808af7u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8af9u);
}
read_pointer_operand: {
    const uint32_t pointer_low = (uint16_t)(registers->dp + 0x5du);
    const uint32_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint32_t pointer_bank = (uint16_t)(pointer_low + 2u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high) ||
        !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t expected = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                              ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (expected + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808af9u, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    pointer |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    const uint32_t source = (pointer + registers->y) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8afbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_pointer_operand: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808afbu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8afdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
advance_operand_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808afdu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8afeu);
}
advance_operand_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808afeu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8affu);
}
read_indirect_value: {
    const uint32_t pointer_low = (uint16_t)(registers->dp + 0x58u);
    const uint32_t pointer_high = (uint16_t)(pointer_low + 1u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high))
        goto finished;
    const uint32_t expected =
        cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) | ((uint32_t)registers->db << 16);
    const uint32_t address = (expected) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808affu, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    pointer |= (uint32_t)registers->db << 16;
    const uint32_t source = (pointer) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b01u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_offset_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b01u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8b02u);
}
add_pointer_offset: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b02u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b04u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_result_pointer: {
    const uint32_t address = (uint16_t)(registers->dp + 0x60u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b04u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b06u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_pointer: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b06u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8b08u);
    goto finished;
}
return_pointer_offset: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b08u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x808b08u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuCharacterSummarySpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                   uint8_t opcode, uint64_t dynamic_epoch,
                                                   uint64_t write_epoch, unsigned step_budget,
                                                   long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x8294c0u && pc < 0x829509u)
        end_pc = 0x829509u;
    else if (pc >= 0x829509u && pc < 0x82950du)
        end_pc = 0x82950du;
    else if (pc >= 0x82950du && pc < 0x82950eu)
        end_pc = 0x82950eu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x8294c0u: byte_accumulator = true; break;
    case 0x8294c2u: byte_accumulator = true; break;
    case 0x8294c4u: byte_accumulator = true; break;
    case 0x8294c7u: width_neutral = true; break;
    case 0x8294e6u: width_neutral = true; break;
    case 0x8294e8u: byte_accumulator = true; break;
    case 0x8294eau: byte_accumulator = true; break;
    case 0x8294edu: byte_accumulator = true; break;
    case 0x8294efu: width_neutral = true; break;
    case 0x8294f1u: byte_accumulator = true; break;
    case 0x8294f3u: byte_accumulator = true; break;
    case 0x8294f6u: width_neutral = true; break;
    case 0x8294f8u: byte_accumulator = true; break;
    case 0x8294fau: byte_accumulator = true; break;
    case 0x8294fdu: byte_accumulator = true; break;
    case 0x829500u: byte_accumulator = true; break;
    case 0x829502u: byte_accumulator = true; break;
    case 0x829504u: byte_accumulator = true; break;
    case 0x829506u: byte_accumulator = true; break;
    case 0x829509u: byte_accumulator = true; break;
    case 0x82950du: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x8294c0u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 78u ||
        memcmp(cart->rom + first,
               "\xa4\x2a\xa9\x07\x8d\x50\x15\xc2\x20\xb9\x0f\x00\x89\x05\x00\xd0\x1e\xb9\x25\x00\x4a\x4a\x4a"
               "\xd9\x11\x00\xb0\x13\xb9\x25\x00\x4a\x4a\xd9\x11\x00\x90\x10\xe2\x20\xa9\x03\x8d\x50\x15\x80"
               "\x07\xe2\x20\xa9\x05\x8d\x50\x15\xe2\x20\xa9\x20\x8d\x64\x05\xa0\x60\x15\x84\xe7\xa9\x8e\x85"
               "\x5f\xa0\x39\xd4\x22\x78\x88\x80\x60",
               78u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x8294c0u: goto read_character_record;
    case 0x8294c2u: goto normal_attribute;
    case 0x8294c4u: goto store_normal_attribute;
    case 0x8294c7u: goto widen_status;
    case 0x8294c9u: goto read_status_flags;
    case 0x8294ccu: goto test_critical_flags;
    case 0x8294cfu: goto critical_status_branch;
    case 0x8294d1u: goto read_critical_hp;
    case 0x8294d4u: goto scale_critical_hp_first;
    case 0x8294d5u: goto scale_critical_hp_second;
    case 0x8294d6u: goto scale_critical_hp_third;
    case 0x8294d7u: goto compare_critical_hp;
    case 0x8294dau: goto critical_hp_branch;
    case 0x8294dcu: goto read_warning_hp;
    case 0x8294dfu: goto scale_warning_hp_first;
    case 0x8294e0u: goto scale_warning_hp_second;
    case 0x8294e1u: goto compare_warning_hp;
    case 0x8294e4u: goto normal_hp_branch;
    case 0x8294e6u: goto narrow_warning_attribute;
    case 0x8294e8u: goto warning_attribute;
    case 0x8294eau: goto store_warning_attribute;
    case 0x8294edu: goto finish_warning_attribute;
    case 0x8294efu: goto narrow_critical_attribute;
    case 0x8294f1u: goto critical_attribute;
    case 0x8294f3u: goto store_critical_attribute;
    case 0x8294f6u: goto narrow_summary_mode;
    case 0x8294f8u: goto summary_draw_mode;
    case 0x8294fau: goto store_summary_mode;
    case 0x8294fdu: goto summary_cursor;
    case 0x829500u: goto store_summary_cursor;
    case 0x829502u: goto summary_source_bank;
    case 0x829504u: goto store_summary_source_bank;
    case 0x829506u: goto summary_text;
    case 0x829509u: goto draw_summary_text;
    case 0x82950du: goto return_character_summary;
    default: return 0;
    }
read_character_record: {
    const uint32_t address = (uint16_t)(registers->dp + 0x2au);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294c0u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x94c2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
normal_attribute: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294c2u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0007u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x94c4u);
}
store_normal_attribute: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1550u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294c4u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x94c7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_status: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294c7u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x94c9u);
}
read_status_flags: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000fu + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x000fu >> 8) != ((0x000fu + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294c9u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x94ccu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_critical_flags: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294ccu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0005u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x94cfu);
}
critical_status_branch: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294cfu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x94efu : 0x94d1u);
    if (taken)
        goto narrow_critical_attribute;
}
read_critical_hp: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0025u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0025u >> 8) != ((0x0025u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294d1u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x94d4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
scale_critical_hp_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294d4u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a & 0xffffu) >> 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x94d5u);
}
scale_critical_hp_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294d5u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a & 0xffffu) >> 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x94d6u);
}
scale_critical_hp_third: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294d6u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a & 0xffffu) >> 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x94d7u);
}
compare_critical_hp: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0011u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0011u >> 8) != ((0x0011u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294d7u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x94dau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
critical_hp_branch: {
    const bool taken = registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294dau, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x94efu : 0x94dcu);
    if (taken)
        goto narrow_critical_attribute;
}
read_warning_hp: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0025u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0025u >> 8) != ((0x0025u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294dcu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x94dfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
scale_warning_hp_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294dfu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a & 0xffffu) >> 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x94e0u);
}
scale_warning_hp_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294e0u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a & 0xffffu) >> 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x94e1u);
}
compare_warning_hp: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0011u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0011u >> 8) != ((0x0011u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294e1u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x94e4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
normal_hp_branch: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294e4u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x94f6u : 0x94e6u);
    if (taken)
        goto narrow_summary_mode;
}
narrow_warning_attribute: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294e6u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x94e8u);
}
warning_attribute: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294e8u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0003u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x94eau);
}
store_warning_attribute: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1550u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294eau, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x94edu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
finish_warning_attribute: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294edu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x94f6u);
    goto finished;
}
narrow_critical_attribute: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294efu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x94f1u);
}
critical_attribute: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294f1u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0005u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x94f3u);
}
store_critical_attribute: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1550u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294f3u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x94f6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_summary_mode: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294f6u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x94f8u);
}
summary_draw_mode: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294f8u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0020u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x94fau);
}
store_summary_mode: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0564u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294fau, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x94fdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
summary_cursor: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8294fdu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x1560u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9500u);
}
store_summary_cursor: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x829500u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->y, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9502u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
summary_source_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x829502u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x008eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9504u);
}
store_summary_source_bank: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x829504u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9506u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
summary_text: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x829506u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0xd439u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9509u);
    goto finished;
}
draw_summary_text: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x829509u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x829509u, 0x22u, 0x808878u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_character_summary: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82950du);
    const int cycles = Lufia2DecompRunReturn(registers, 0x82950du, 0x60u, false, registers->mf, true);
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

static int Lufia2DecompRunMenuPointerSummarySpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                 uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                 unsigned step_budget, long *step_count) {
    if ((pc >> 16) == 0x80u)
        return Lufia2DecompRunMenuPointerOffsetSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                    step_budget, step_count);
    return Lufia2DecompRunMenuCharacterSummarySpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                   step_budget, step_count);
}
