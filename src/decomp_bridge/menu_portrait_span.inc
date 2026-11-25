/* Calls, returns and the table jump start their own phase: the interpreter loop
 * takes its call and menu-return decisions from the first opcode of a span. */
#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuPlacePortraitsSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                 uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                 unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x8293cfu && pc < 0x8293d6u)
        end_pc = 0x8293d6u;
    else if (pc >= 0x8293d6u && pc < 0x8293dau)
        end_pc = 0x8293dau;
    else if (pc >= 0x8293dau && pc < 0x8293f5u)
        end_pc = 0x8293f5u;
    else if (pc >= 0x8293f5u && pc < 0x8293f6u)
        end_pc = 0x8293f6u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x8293cfu: byte_accumulator = true; break;
    case 0x8293d1u: byte_accumulator = true; break;
    case 0x8293d3u: byte_accumulator = true; break;
    case 0x8293d4u: byte_accumulator = true; break;
    case 0x8293d5u: byte_accumulator = true; break;
    case 0x8293d6u: byte_accumulator = true; break;
    case 0x8293dau: width_neutral = true; break;
    case 0x8293deu: width_neutral = true; break;
    case 0x8293e0u: byte_accumulator = true; break;
    case 0x8293e3u: byte_accumulator = true; break;
    case 0x8293e4u: byte_accumulator = true; break;
    case 0x8293e7u: byte_accumulator = true; break;
    case 0x8293eau: byte_accumulator = true; break;
    case 0x8293edu: byte_accumulator = true; break;
    case 0x8293eeu: byte_accumulator = true; break;
    case 0x8293f1u: byte_accumulator = true; break;
    case 0x8293f3u: byte_accumulator = true; break;
    case 0x8293f5u: width_neutral = true; break;
    default: break;
    }
    bool byte_index = false;
    switch (pc) {
    case 0x8293d1u:
    case 0x8293d3u:
    case 0x8293d4u:
    case 0x8293d5u:
    case 0x8293d6u:
    case 0x8293dau:
    case 0x8293dcu:
    case 0x8293deu:
    case 0x8293e0u:
    case 0x8293e3u:
    case 0x8293e4u:
    case 0x8293e7u:
    case 0x8293eau:
    case 0x8293edu:
    case 0x8293eeu:
    case 0x8293f1u:
    case 0x8293f3u: byte_index = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e ||
        registers->xf != byte_index || (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x8293cfu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 39u ||
        memcmp(cart->rom + first,
               "\xe2\x10\xa2\x00\x8a\x0a\xa8\x22\xda\x8c\x86\xc2\x20\xb7\x5d\xe2\x20\x9d\x88\x13\xeb\x9d\xe8"
               "\x13\x9e\x18\x14\x9e\xb8\x13\xe8\xec\x7a\x0a\xd0\xe0\xc2\x10\x60",
               39u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x8293cfu: goto narrow_indices;
    case 0x8293d1u: goto set_x;
    case 0x8293d3u: goto copy_x_to_accumulator;
    case 0x8293d4u: goto double_accumulator;
    case 0x8293d5u: goto copy_accumulator_to_y;
    case 0x8293d6u: goto call_868cda;
    case 0x8293dau: goto widen_accumulator;
    case 0x8293dcu: goto load_table_pointer;
    case 0x8293deu: goto narrow_accumulator;
    case 0x8293e0u: goto store_sprite_x;
    case 0x8293e3u: goto exchange_bytes;
    case 0x8293e4u: goto store_sprite_y;
    case 0x8293e7u: goto clear_sprite_y_high;
    case 0x8293eau: goto clear_sprite_x_high;
    case 0x8293edu: goto next_x;
    case 0x8293eeu: goto compare_x_party_count;
    case 0x8293f1u: goto branch_different;
    case 0x8293f3u: goto widen_indices;
    case 0x8293f5u: goto return_caller;
    default: return 0;
    }
narrow_indices: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293cfu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x10u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x93d1u);
}
set_x: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293d1u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x93d3u);
}
copy_x_to_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293d3u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->x), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x93d4u);
}
double_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293d4u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x93d5u);
}
copy_accumulator_to_y: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293d5u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint8_t)(registers->a), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x93d6u);
    goto finished;
}
call_868cda: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8293d6u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8293d6u, 0x22u, 0x868cdau, true, true, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
widen_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293dau, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x93dcu);
}
load_table_pointer: {
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
    Lufia2SpanFetch(registers, 0x8293dcu, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    pointer |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    const uint32_t source = (pointer + registers->y) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x93deu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293deu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x93e0u);
}
store_sprite_x: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1388u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x1388u >> 8) != ((0x1388u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293e0u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x93e3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
exchange_bytes: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293e3u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    NativeSpanStep(&span, registers, cpu, 3u, 0x93e4u);
}
store_sprite_y: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13e8u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x13e8u >> 8) != ((0x13e8u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293e4u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x93e7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_sprite_y_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1418u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x1418u >> 8) != ((0x1418u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293e7u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x93eau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_sprite_x_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13b8u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x13b8u >> 8) != ((0x13b8u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293eau, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x93edu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_x: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293edu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint8_t)((uint16_t)(registers->x + 1u)), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x93eeu);
}
compare_x_party_count: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0a7au) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 4u) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293eeu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, 4u, 0x93f1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_different: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293f1u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x93d3u : 0x93f3u);
    if (taken)
        goto finished;
}
widen_indices: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293f3u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x10u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x93f5u);
    goto finished;
}
return_caller: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8293f5u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x8293f5u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuPrepareTextSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                              uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                              unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x8293f6u && pc < 0x82942eu)
        end_pc = 0x82942eu;
    else if (pc >= 0x82942eu && pc < 0x82942fu)
        end_pc = 0x82942fu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x8293f6u: width_neutral = true; break;
    case 0x82940au: width_neutral = true; break;
    case 0x82940cu: byte_accumulator = true; break;
    case 0x82940eu: byte_accumulator = true; break;
    case 0x829410u: byte_accumulator = true; break;
    case 0x829412u: byte_accumulator = true; break;
    case 0x829414u: byte_accumulator = true; break;
    case 0x829416u: byte_accumulator = true; break;
    case 0x829418u: byte_accumulator = true; break;
    case 0x82941au: byte_accumulator = true; break;
    case 0x82941cu: byte_accumulator = true; break;
    case 0x82941eu: byte_accumulator = true; break;
    case 0x829420u: byte_accumulator = true; break;
    case 0x829423u: byte_accumulator = true; break;
    case 0x829425u: byte_accumulator = true; break;
    case 0x829427u: byte_accumulator = true; break;
    case 0x829429u: byte_accumulator = true; break;
    case 0x82942bu: byte_accumulator = true; break;
    case 0x82942eu: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x8293f6u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 57u ||
        memcmp(cart->rom + first,
               "\xc2\x20\x29\xff\x00\x0a\xaa\xbf\x7e\xe5\x8e\x8d\x96\x05\xbf\xa0\xe5\x8e\x85\xf4\xe2\x20\xa9"
               "\x8e\x85\xf6\xa9\x10\x04\xf2\xa9\x40\x85\xf3\xa9\x12\x85\xfa\xa9\x02\x85\xfb\xa2\xc0\x80\x86"
               "\xf7\xa9\x7e\x85\xf9\xa9\x03\x8d\x65\x15\x60",
               57u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x8293f6u: goto widen_accumulator;
    case 0x8293f8u: goto mask_accumulator;
    case 0x8293fbu: goto double_accumulator;
    case 0x8293fcu: goto copy_accumulator_to_x;
    case 0x8293fdu: goto load_table;
    case 0x829401u: goto store_display_text;
    case 0x829404u: goto load_table_2;
    case 0x829408u: goto store_text_pointer;
    case 0x82940au: goto narrow_accumulator;
    case 0x82940cu: goto set_accumulator;
    case 0x82940eu: goto store_text_bank;
    case 0x829410u: goto set_accumulator_2;
    case 0x829412u: goto set_bits_in_display_flags;
    case 0x829414u: goto set_accumulator_3;
    case 0x829416u: goto store_display_mode;
    case 0x829418u: goto set_accumulator_4;
    case 0x82941au: goto store_text_columns;
    case 0x82941cu: goto set_accumulator_5;
    case 0x82941eu: goto store_text_rows;
    case 0x829420u: goto set_x;
    case 0x829423u: goto store_x_text_target;
    case 0x829425u: goto set_accumulator_6;
    case 0x829427u: goto store_text_target_bank;
    case 0x829429u: goto set_accumulator_7;
    case 0x82942bu: goto store_display_request;
    case 0x82942eu: goto return_caller;
    default: return 0;
    }
widen_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293f6u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x93f8u);
}
mask_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293f8u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x93fbu);
}
double_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293fbu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x93fcu);
}
copy_accumulator_to_x: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293fcu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x93fdu);
}
load_table: {
    const uint32_t address = (0x8ee57eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8293fdu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9401u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_display_text: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0596u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x829401u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9404u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_table_2: {
    const uint32_t address = (0x8ee5a0u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x829404u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9408u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_text_pointer: {
    const uint32_t address = (uint16_t)(registers->dp + 0xf4u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x829408u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x940au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82940au, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x940cu);
}
set_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82940cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x008eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x940eu);
}
store_text_bank: {
    const uint32_t address = (uint16_t)(registers->dp + 0xf6u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82940eu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9410u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x829410u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9412u);
}
set_bits_in_display_flags: {
    const uint32_t address = (uint16_t)(registers->dp + 0xf2u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x829412u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0x04u, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9414u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator_3: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x829414u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0040u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9416u);
}
store_display_mode: {
    const uint32_t address = (uint16_t)(registers->dp + 0xf3u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x829416u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9418u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator_4: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x829418u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0012u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x941au);
}
store_text_columns: {
    const uint32_t address = (uint16_t)(registers->dp + 0xfau);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82941au, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x941cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator_5: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82941cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0002u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x941eu);
}
store_text_rows: {
    const uint32_t address = (uint16_t)(registers->dp + 0xfbu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82941eu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9420u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_x: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x829420u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x80c0u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9423u);
}
store_x_text_target: {
    const uint32_t address = (uint16_t)(registers->dp + 0xf7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x829423u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9425u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator_6: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x829425u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x007eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9427u);
}
store_text_target_bank: {
    const uint32_t address = (uint16_t)(registers->dp + 0xf9u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x829427u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9429u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator_7: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x829429u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0003u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x942bu);
}
store_display_request: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1565u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82942bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x942eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_caller: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82942eu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x82942eu, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuExperienceTotalsSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                   uint8_t opcode, uint64_t dynamic_epoch,
                                                   uint64_t write_epoch, unsigned step_budget,
                                                   long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x82942fu && pc < 0x829448u)
        end_pc = 0x829448u;
    else if (pc >= 0x829448u && pc < 0x829449u)
        end_pc = 0x829449u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x82942fu: byte_accumulator = true; break;
    case 0x829432u: byte_accumulator = true; break;
    case 0x829433u: byte_accumulator = true; break;
    case 0x829436u: byte_accumulator = true; break;
    case 0x829438u: byte_accumulator = true; break;
    case 0x82943bu: byte_accumulator = true; break;
    case 0x82943eu: byte_accumulator = true; break;
    case 0x829440u: byte_accumulator = true; break;
    case 0x829443u: byte_accumulator = true; break;
    case 0x829446u: byte_accumulator = true; break;
    case 0x829448u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x82942fu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 26u || memcmp(cart->rom + first,
                                              "\xbd\x62\x00\x38\xfd\x5f\x00\x85\x08\xbd\x63\x00\xfd\x60\x00"
                                              "\x85\x09\xbd\x64\x00\xfd\x61\x00\x85\x0a\x60",
                                              26u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x82942fu: goto load_next_total_low;
    case 0x829432u: goto set_carry;
    case 0x829433u: goto subtract_member_total_low;
    case 0x829436u: goto store_difference_low;
    case 0x829438u: goto load_next_total_middle;
    case 0x82943bu: goto subtract_member_total_middle;
    case 0x82943eu: goto store_difference_middle;
    case 0x829440u: goto load_next_total_high;
    case 0x829443u: goto subtract_member_total_high;
    case 0x829446u: goto store_difference_high;
    case 0x829448u: goto return_caller;
    default: return 0;
    }
load_next_total_low: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0062u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0062u >> 8) != ((0x0062u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82942fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9432u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x829432u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9433u);
}
subtract_member_total_low: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x005fu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x005fu >> 8) != ((0x005fu + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x829433u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9436u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_difference_low: {
    const uint32_t address = (uint16_t)(registers->dp + 0x08u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x829436u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9438u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_next_total_middle: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0063u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0063u >> 8) != ((0x0063u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x829438u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x943bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
subtract_member_total_middle: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0060u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0060u >> 8) != ((0x0060u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82943bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x943eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_difference_middle: {
    const uint32_t address = (uint16_t)(registers->dp + 0x09u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82943eu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9440u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_next_total_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0064u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0064u >> 8) != ((0x0064u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x829440u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9443u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
subtract_member_total_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0061u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0061u >> 8) != ((0x0061u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x829443u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9446u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_difference_high: {
    const uint32_t address = (uint16_t)(registers->dp + 0x0au);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x829446u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9448u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_caller: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x829448u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x829448u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuEquipmentMemberSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                  uint8_t opcode, uint64_t dynamic_epoch,
                                                  uint64_t write_epoch, unsigned step_budget,
                                                  long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x829971u && pc < 0x829983u)
        end_pc = 0x829983u;
    else if (pc >= 0x829983u && pc < 0x829984u)
        end_pc = 0x829984u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x829971u: width_neutral = true; break;
    case 0x829972u: width_neutral = true; break;
    case 0x829973u: width_neutral = true; break;
    case 0x829982u: width_neutral = true; break;
    case 0x829983u: width_neutral = true; break;
    default: break;
    }
    bool any_index = false;
    switch (pc) {
    case 0x829971u:
    case 0x829972u:
    case 0x829973u:
    case 0x829982u:
    case 0x829983u: any_index = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e ||
        (!any_index && registers->xf) || (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x829971u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 19u ||
        memcmp(cart->rom + first,
               "\x5a\x08\xc2\x30\xa5\x22\x29\xff\x00\x0a\xa8\xb9\x80\x0a\x85\x2a\x28\x7a\x60", 19u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x829971u: goto push_y;
    case 0x829972u: goto push_flags;
    case 0x829973u: goto widen_registers;
    case 0x829975u: goto load_equipment_slot;
    case 0x829977u: goto mask_accumulator;
    case 0x82997au: goto double_accumulator;
    case 0x82997bu: goto copy_accumulator_to_y;
    case 0x82997cu: goto load_party_record;
    case 0x82997fu: goto store_equipment_member;
    case 0x829981u: goto pull_flags;
    case 0x829982u: goto pull_y;
    case 0x829983u: goto return_caller;
    default: return 0;
    }
push_y: {
    const bool byte = registers->xf;
    const unsigned cycles = byte ? 3u : 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < (byte ? 0x1f01u : 0x1f02u) ||
        registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x829971u, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    if (!byte)
        registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9972u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
push_flags: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x829972u, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, interp816_getFlags(registers));
    NativeSpanStep(&span, registers, cpu, 3u, 0x9973u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_registers: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x829973u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x30u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9975u);
}
load_equipment_slot: {
    const uint32_t address = (uint16_t)(registers->dp + 0x22u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x829975u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9977u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x829977u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x997au);
}
double_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82997au, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x997bu);
}
copy_accumulator_to_y: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82997bu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x997cu);
}
load_party_record: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0a80u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0a80u >> 8) != ((0x0a80u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82997cu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x997fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_equipment_member: {
    const uint32_t address = (uint16_t)(registers->dp + 0x2au);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82997fu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9981u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
pull_flags: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x829981u, 1u, span.instructions != 0u);
    interp816_setFlags(registers, registers->read(registers->mem, ++registers->sp));
    NativeSpanStep(&span, registers, cpu, 4u, 0x9982u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
pull_y: {
    const bool byte = registers->xf;
    const unsigned cycles = byte ? 4u : 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u ||
        registers->sp > (byte ? 0x1ffeu : 0x1ffdu))
        goto finished;
    Lufia2SpanFetch(registers, 0x829982u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (!byte)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, byte);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9983u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_caller: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x829983u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x829983u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuHighlightPartySpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                 uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                 unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x829984u && pc < 0x829995u)
        end_pc = 0x829995u;
    else if (pc >= 0x829995u && pc < 0x829998u)
        end_pc = 0x829998u;
    else if (pc >= 0x829998u && pc < 0x82999au)
        end_pc = 0x82999au;
    else if (pc >= 0x82999au && pc < 0x82999bu)
        end_pc = 0x82999bu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x829984u: width_neutral = true; break;
    case 0x829998u: width_neutral = true; break;
    case 0x82999au: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x829984u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 23u ||
        memcmp(cart->rom + first,
               "\xc2\x20\xad\xb3\x14\x0a\xaa\xbf\x76\xe5\x8e\xa2\x05\x0f\xa0\x02\x00\x20\xca\x80\xe2\x20\x60",
               23u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x829984u: goto widen_accumulator;
    case 0x829986u: goto load_list_index;
    case 0x829989u: goto double_accumulator;
    case 0x82998au: goto copy_accumulator_to_x;
    case 0x82998bu: goto load_table;
    case 0x82998fu: goto set_x;
    case 0x829992u: goto set_y;
    case 0x829995u: goto call_8280ca;
    case 0x829998u: goto narrow_accumulator;
    case 0x82999au: goto return_caller;
    default: return 0;
    }
widen_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x829984u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9986u);
}
load_list_index: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14b3u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x829986u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9989u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
double_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x829989u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x998au);
}
copy_accumulator_to_x: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82998au, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x998bu);
}
load_table: {
    const uint32_t address = (0x8ee576u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82998bu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x998fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_x: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82998fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0f05u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9992u);
}
set_y: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x829992u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0002u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9995u);
    goto finished;
}
call_8280ca: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x829995u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x829995u, 0x20u, 0x8280cau, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
narrow_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x829998u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x999au);
    goto finished;
}
return_caller: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82999au);
    const int cycles = Lufia2DecompRunReturn(registers, 0x82999au, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuInitializePortraitsSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                      uint8_t opcode, uint64_t dynamic_epoch,
                                                      uint64_t write_epoch, unsigned step_budget,
                                                      long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x82999bu && pc < 0x82999eu)
        end_pc = 0x82999eu;
    else if (pc >= 0x82999eu && pc < 0x8299a2u)
        end_pc = 0x8299a2u;
    else if (pc >= 0x8299a2u && pc < 0x8299b0u)
        end_pc = 0x8299b0u;
    else if (pc >= 0x8299b0u && pc < 0x8299b3u)
        end_pc = 0x8299b3u;
    else if (pc >= 0x8299b3u && pc < 0x8299bdu)
        end_pc = 0x8299bdu;
    else if (pc >= 0x8299bdu && pc < 0x8299beu)
        end_pc = 0x8299beu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x82999bu: byte_accumulator = true; break;
    case 0x82999eu: byte_accumulator = true; break;
    case 0x8299a2u: byte_accumulator = true; break;
    case 0x8299a5u: width_neutral = true; break;
    case 0x8299aeu: width_neutral = true; break;
    case 0x8299b0u: byte_accumulator = true; break;
    case 0x8299b3u: byte_accumulator = true; break;
    case 0x8299b5u: byte_accumulator = true; break;
    case 0x8299b6u: byte_accumulator = true; break;
    case 0x8299b9u: byte_accumulator = true; break;
    case 0x8299bbu: byte_accumulator = true; break;
    case 0x8299bdu: width_neutral = true; break;
    default: break;
    }
    bool byte_index = false;
    switch (pc) {
    case 0x8299b5u:
    case 0x8299b6u:
    case 0x8299b9u: byte_index = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e ||
        registers->xf != byte_index || (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x82999bu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 35u ||
        memcmp(cart->rom + first,
               "\xa2\x1f\x00\x22\x14\x92\x82\xa2\x00\x00\xc2\x20\x8a\x0a\xa8\xb9\x80\x0a\xa8\xe2\x20\x20\xbe"
               "\x99\xe2\x10\xe8\xec\x7a\x0a\xc2\x10\xd0\xe8\x60",
               35u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x82999bu: goto set_x;
    case 0x82999eu: goto call_829214;
    case 0x8299a2u: goto set_x_2;
    case 0x8299a5u: goto widen_accumulator;
    case 0x8299a7u: goto copy_x_to_accumulator;
    case 0x8299a8u: goto double_accumulator;
    case 0x8299a9u: goto copy_accumulator_to_y;
    case 0x8299aau: goto load_party_record;
    case 0x8299adu: goto copy_accumulator_to_y_2;
    case 0x8299aeu: goto narrow_accumulator;
    case 0x8299b0u: goto call_8299be;
    case 0x8299b3u: goto narrow_indices;
    case 0x8299b5u: goto next_x;
    case 0x8299b6u: goto compare_x_party_count;
    case 0x8299b9u: goto widen_indices;
    case 0x8299bbu: goto branch_different;
    case 0x8299bdu: goto return_caller;
    default: return 0;
    }
set_x: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82999bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x001fu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x999eu);
    goto finished;
}
call_829214: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82999eu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x82999eu, 0x22u, 0x829214u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
set_x_2: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299a2u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x99a5u);
}
widen_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299a5u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x99a7u);
}
copy_x_to_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299a7u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->x, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x99a8u);
}
double_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299a8u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x99a9u);
}
copy_accumulator_to_y: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299a9u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x99aau);
}
load_party_record: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0a80u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0a80u >> 8) != ((0x0a80u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299aau, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x99adu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
copy_accumulator_to_y_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299adu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x99aeu);
}
narrow_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299aeu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x99b0u);
    goto finished;
}
call_8299be: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8299b0u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8299b0u, 0x20u, 0x8299beu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
narrow_indices: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299b3u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x10u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x99b5u);
}
next_x: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299b5u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint8_t)((uint16_t)(registers->x + 1u)), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x99b6u);
}
compare_x_party_count: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0a7au) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 4u) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299b6u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, 4u, 0x99b9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_indices: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299b9u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x10u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x99bbu);
}
branch_different: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299bbu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x99a5u : 0x99bdu);
    if (taken)
        goto finished;
    goto finished;
}
return_caller: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8299bdu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x8299bdu, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuInitializePortraitSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                     uint8_t opcode, uint64_t dynamic_epoch,
                                                     uint64_t write_epoch, unsigned step_budget,
                                                     long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x8299beu && pc < 0x8299eeu)
        end_pc = 0x8299eeu;
    else if (pc >= 0x8299eeu && pc < 0x8299f2u)
        end_pc = 0x8299f2u;
    else if (pc >= 0x8299f2u && pc < 0x8299f3u)
        end_pc = 0x8299f3u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x8299beu: byte_accumulator = true; break;
    case 0x8299c0u: byte_accumulator = true; break;
    case 0x8299c3u: byte_accumulator = true; break;
    case 0x8299c6u: byte_accumulator = true; break;
    case 0x8299c9u: byte_accumulator = true; break;
    case 0x8299cbu: byte_accumulator = true; break;
    case 0x8299cdu: byte_accumulator = true; break;
    case 0x8299cfu: byte_accumulator = true; break;
    case 0x8299d1u: width_neutral = true; break;
    case 0x8299dcu: width_neutral = true; break;
    case 0x8299deu: byte_accumulator = true; break;
    case 0x8299e0u: byte_accumulator = true; break;
    case 0x8299e2u: byte_accumulator = true; break;
    case 0x8299e4u: byte_accumulator = true; break;
    case 0x8299e6u: byte_accumulator = true; break;
    case 0x8299e8u: byte_accumulator = true; break;
    case 0x8299eau: byte_accumulator = true; break;
    case 0x8299ecu: byte_accumulator = true; break;
    case 0x8299eeu: byte_accumulator = true; break;
    case 0x8299f2u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x8299beu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 53u ||
        memcmp(cart->rom + first,
               "\xa9\x01\x9d\xd8\x11\x9e\x48\x14\xb9\x0f\x00\x89\x04\xd0\x17\x89\x01\xd0\x17\xc2\x20\xb9\x25"
               "\x00\x4a\x4a\x4a\xd9\x11\x00\xe2\x20\x90\x0c\xa9\x02\x80\x0a\xa9\x04\x80\x06\xa9\x01\x80\x02"
               "\xa9\x00\x22\xf5\x8c\x86\x60",
               53u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x8299beu: goto set_accumulator;
    case 0x8299c0u: goto store_sprite_active;
    case 0x8299c3u: goto clear_sprite_frame;
    case 0x8299c6u: goto load_member_status;
    case 0x8299c9u: goto test_accumulator_bits;
    case 0x8299cbu: goto branch_different;
    case 0x8299cdu: goto test_accumulator_bits_2;
    case 0x8299cfu: goto branch_different_2;
    case 0x8299d1u: goto widen_accumulator;
    case 0x8299d3u: goto load_maximum_hp;
    case 0x8299d6u: goto halve_accumulator;
    case 0x8299d7u: goto halve_accumulator_2;
    case 0x8299d8u: goto halve_accumulator_3;
    case 0x8299d9u: goto compare_current_hp;
    case 0x8299dcu: goto narrow_accumulator;
    case 0x8299deu: goto branch_no_carry;
    case 0x8299e0u: goto set_accumulator_2;
    case 0x8299e2u: goto skip_ahead;
    case 0x8299e4u: goto set_accumulator_3;
    case 0x8299e6u: goto skip_ahead_2;
    case 0x8299e8u: goto set_accumulator_4;
    case 0x8299eau: goto skip_ahead_3;
    case 0x8299ecu: goto set_accumulator_5;
    case 0x8299eeu: goto call_868cf5;
    case 0x8299f2u: goto return_caller;
    default: return 0;
    }
set_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299beu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x99c0u);
}
store_sprite_active: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x11d8u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299c0u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x99c3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_sprite_frame: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1448u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299c3u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x99c6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_member_status: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000fu + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x000fu >> 8) != ((0x000fu + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299c6u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x99c9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_accumulator_bits: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299c9u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0004u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x99cbu);
}
branch_different: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299cbu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x99e4u : 0x99cdu);
    if (taken)
        goto set_accumulator_3;
}
test_accumulator_bits_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299cdu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x99cfu);
}
branch_different_2: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299cfu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x99e8u : 0x99d1u);
    if (taken)
        goto set_accumulator_4;
}
widen_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299d1u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x99d3u);
}
load_maximum_hp: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0025u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0025u >> 8) != ((0x0025u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299d3u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x99d6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
halve_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299d6u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a & 0xffffu) >> 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x99d7u);
}
halve_accumulator_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299d7u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a & 0xffffu) >> 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x99d8u);
}
halve_accumulator_3: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299d8u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a & 0xffffu) >> 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x99d9u);
}
compare_current_hp: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0011u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0011u >> 8) != ((0x0011u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299d9u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x99dcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299dcu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x99deu);
}
branch_no_carry: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299deu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x99ecu : 0x99e0u);
    if (taken)
        goto set_accumulator_5;
}
set_accumulator_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299e0u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0002u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x99e2u);
}
skip_ahead: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299e2u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x99eeu);
    goto finished;
}
set_accumulator_3: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299e4u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0004u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x99e6u);
}
skip_ahead_2: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299e6u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x99eeu);
    goto finished;
}
set_accumulator_4: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299e8u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x99eau);
}
skip_ahead_3: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299eau, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x99eeu);
    goto finished;
}
set_accumulator_5: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8299ecu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x99eeu);
    goto finished;
}
call_868cf5: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8299eeu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8299eeu, 0x22u, 0x868cf5u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_caller: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8299f2u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x8299f2u, 0x60u, false, registers->mf, true);
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

static int Lufia2DecompRunMenuPortraitSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                           uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget,
                                           long *step_count) {
    if (pc >= 0x8293cfu && pc <= 0x8293f5u)
        return Lufia2DecompRunMenuPlacePortraitsSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                     step_budget, step_count);
    if (pc >= 0x8293f6u && pc <= 0x82942eu)
        return Lufia2DecompRunMenuPrepareTextSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                  step_budget, step_count);
    if (pc >= 0x82942fu && pc <= 0x829448u)
        return Lufia2DecompRunMenuExperienceTotalsSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                       step_budget, step_count);
    if (pc >= 0x829971u && pc <= 0x829983u)
        return Lufia2DecompRunMenuEquipmentMemberSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                      step_budget, step_count);
    if (pc >= 0x829984u && pc <= 0x82999au)
        return Lufia2DecompRunMenuHighlightPartySpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                     step_budget, step_count);
    if (pc >= 0x82999bu && pc <= 0x8299bdu)
        return Lufia2DecompRunMenuInitializePortraitsSpan(registers, cpu, pc, opcode, dynamic_epoch,
                                                          write_epoch, step_budget, step_count);
    if (pc >= 0x8299beu && pc <= 0x8299f2u)
        return Lufia2DecompRunMenuInitializePortraitSpan(registers, cpu, pc, opcode, dynamic_epoch,
                                                         write_epoch, step_budget, step_count);
    return 0;
}
