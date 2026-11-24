#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuVmControlSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                            uint64_t dynamic_epoch, uint64_t write_epoch,
                                            unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x808cfcu && pc < 0x808d04u)
        end_pc = 0x808d04u;
    else if (pc >= 0x808d1cu && pc < 0x808d22u)
        end_pc = 0x808d22u;
    else if (pc >= 0x808d2bu && pc < 0x808d47u)
        end_pc = 0x808d47u;
    else if (pc >= 0x808d47u && pc < 0x808d4bu)
        end_pc = 0x808d4bu;
    else if (pc >= 0x808d4bu && pc < 0x808d5du)
        end_pc = 0x808d5du;
    else if (pc >= 0x808d91u && pc < 0x808d9eu)
        end_pc = 0x808d9eu;
    else if (pc >= 0x808d9eu && pc < 0x808da7u)
        end_pc = 0x808da7u;
    else if (pc >= 0x808da7u && pc < 0x808db0u)
        end_pc = 0x808db0u;
    else
        return 0;
    const bool byte_accumulator =
        !((pc >= 0x808d2du && pc <= 0x808d39u) || (pc >= 0x808d4du && pc <= 0x808d53u));
    const bool changes_width = pc == 0x808d2bu || pc == 0x808d39u || pc == 0x808d4bu || pc == 0x808d53u;
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->nmiWanted ||
        registers->irqWanted || (!changes_width && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x808cfcu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 180u ||
        memcmp(cart->rom + first,
               "\xa9\xff\x8d\x78\x05\x82\xa6\xfb\x20\xf9\x8d\x20\x0f\x8e\x82\xa0\xfb\xc2\x20\xb7\x5d\xc8\xc8"
               "\x8d\x73\x05\x8d\x75\x05\xaa\xe2\x20\x20\x0f\x8e\x82\x8b\xfb\xb7\x5d\xc8\x20\x5d\x8d\x82\x7f"
               "\xfb\xc2\x20\xb7\x5d\x85\x56\xc8\xc8\xb7\x5d\x85\x58\xc8\xc8\xe2\x20\x5a\xa9\x00\xeb\xb2\x58"
               "\xa8\xb1\x56\x20\x5d\x8d\x7a\x82\x5f\xfb\xc2\x20\xb7\x5d\x85\x56\xc8\xc8\xe2\x20\xb2\x56\x20"
               "\x5d\x8d\x82\x4d\xfb\x0a\x0a\x48\xad\x64\x05\x29\xe3\x03\x01\x8d\x64\x05\x68\x60\xc2\x20\xad"
               "\x75\x05\x8d\x79\x05\xe2\x20\x82\x31\xfb\xc2\x20\xad\x79\x05\x18\x77\x5d\xc8\xc8\x8d\x75\x05"
               "\x8d\x73\x05\xe2\x20\x20\x0f\x8e\x82\x19\xfb\xad\x64\x05\x4a\x4a\x29\x07\x8d\x77\x05\x82\x0c"
               "\xfb\xad\x77\x05\x20\x5d\x8d\x82\x03\xfb\xb7\x5d\x8d\x64\x05\xc8\x82\xfa\xfa",
               180u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x808cfcu: goto raw_mode;
    case 0x808cfeu: goto store_raw_mode;
    case 0x808d01u: goto resume_raw;
    case 0x808d1cu: goto reset_width;
    case 0x808d1fu: goto resume_position;
    case 0x808d2bu: goto widen_table_pointer;
    case 0x808d2du: goto read_palette_table;
    case 0x808d2fu: goto save_palette_table;
    case 0x808d31u: goto next_table_first;
    case 0x808d32u: goto next_table_second;
    case 0x808d33u: goto read_palette_index;
    case 0x808d35u: goto save_palette_index;
    case 0x808d37u: goto next_index_first;
    case 0x808d38u: goto next_index_second;
    case 0x808d39u: goto narrow_table_value;
    case 0x808d3bu: goto save_text_index;
    case 0x808d3cu: goto clear_palette_high;
    case 0x808d3eu: goto exchange_palette_bytes;
    case 0x808d3fu: goto read_palette_index_value;
    case 0x808d41u: goto select_palette_index;
    case 0x808d42u: goto read_palette_value;
    case 0x808d44u: goto apply_table_palette;
    case 0x808d47u: goto restore_text_index;
    case 0x808d48u: goto resume_table_palette;
    case 0x808d4bu: goto widen_palette_pointer;
    case 0x808d4du: goto read_palette_pointer;
    case 0x808d4fu: goto save_palette_pointer;
    case 0x808d51u: goto next_pointer_first;
    case 0x808d52u: goto next_pointer_second;
    case 0x808d53u: goto narrow_palette_value;
    case 0x808d55u: goto read_indirect_palette;
    case 0x808d57u: goto apply_indirect_palette;
    case 0x808d5au: goto resume_indirect_palette;
    case 0x808d91u: goto read_attribute;
    case 0x808d94u: goto shift_attribute_first;
    case 0x808d95u: goto shift_attribute_second;
    case 0x808d96u: goto mask_palette;
    case 0x808d98u: goto save_palette;
    case 0x808d9bu: goto resume_palette_save;
    case 0x808d9eu: goto read_saved_palette;
    case 0x808da1u: goto apply_saved_palette;
    case 0x808da4u: goto resume_saved_palette;
    case 0x808da7u: goto read_stream_attribute;
    case 0x808da9u: goto store_stream_attribute;
    case 0x808dacu: goto next_attribute;
    case 0x808dadu: goto resume_attribute;
    default: return 0;
    }
raw_mode: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808cfcu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0xffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8cfeu);
}
store_raw_mode: {
    const uint32_t address = ((uint32_t)registers->db << 16) + 0x0578u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + 0u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsWramByte(address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x808cfeu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8d01u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
resume_raw: {
    if (!NativeSpanCanStep(&span, cpu, 4u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d01u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 4u, 0x88aau);
    goto finished;
}
reset_width: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808d1cu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808d1cu, 0x20u, 0x808e0fu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
resume_position: {
    if (!NativeSpanCanStep(&span, cpu, 4u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d1fu, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 4u, 0x88adu);
    goto finished;
}
widen_table_pointer: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d2bu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8d2du);
}
read_palette_table: {
    const uint16_t pointer_low = (uint16_t)(registers->dp + 0x5du);
    const uint16_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint16_t pointer_bank = (uint16_t)(pointer_low + 2u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high) ||
        !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t pointer = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                             ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (pointer + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + 1u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d2du, 2u, span.instructions != 0u);
    uint32_t source = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    source |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    source = (source + registers->y) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8d2fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_palette_table: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + 1u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d2fu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8d31u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_table_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d31u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d32u);
}
next_table_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d32u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d33u);
}
read_palette_index: {
    const uint16_t pointer_low = (uint16_t)(registers->dp + 0x5du);
    const uint16_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint16_t pointer_bank = (uint16_t)(pointer_low + 2u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high) ||
        !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t pointer = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                             ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (pointer + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + 1u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d33u, 2u, span.instructions != 0u);
    uint32_t source = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    source |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    source = (source + registers->y) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8d35u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_palette_index: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + 1u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d35u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8d37u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_index_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d37u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d38u);
}
next_index_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d38u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d39u);
}
narrow_table_value: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d39u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8d3bu);
}
save_text_index: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x808d3bu, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp, (uint8_t)(registers->y >> 8));
    --registers->sp;
    registers->write(registers->mem, registers->sp, (uint8_t)registers->y);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, 4u, 0x8d3cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_palette_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d3cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d3eu);
}
exchange_palette_bytes: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d3eu, 1u, span.instructions != 0u);
    registers->a = (uint16_t)((registers->a << 8) | (registers->a >> 8));
    registers->z = (uint8_t)registers->a == 0u;
    registers->n = (registers->a & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 3u, 0x8d3fu);
}
read_palette_index_value: {
    const uint16_t pointer_low = (uint16_t)(registers->dp + 0x58u);
    const uint16_t pointer_high = (uint16_t)(pointer_low + 1u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high))
        goto finished;
    const uint32_t pointer = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + pointer) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 0u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d3fu, 2u, span.instructions != 0u);
    uint32_t source = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    source += ((uint32_t)registers->db << 16);
    source = (source) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8d41u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_palette_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d41u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d42u);
}
read_palette_value: {
    const uint16_t pointer_low = (uint16_t)(registers->dp + 0x56u);
    const uint16_t pointer_high = (uint16_t)(pointer_low + 1u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high))
        goto finished;
    const uint32_t pointer = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + pointer + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 0u + direct_penalty + ((pointer >> 8) != ((pointer + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d42u, 2u, span.instructions != 0u);
    uint32_t source = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    source += ((uint32_t)registers->db << 16);
    source = (source + registers->y) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8d44u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
apply_table_palette: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808d44u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808d44u, 0x20u, 0x808d5du, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_text_index: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x808d47u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x8d48u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
resume_table_palette: {
    if (!NativeSpanCanStep(&span, cpu, 4u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d48u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 4u, 0x88aau);
    goto finished;
}
widen_palette_pointer: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d4bu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8d4du);
}
read_palette_pointer: {
    const uint16_t pointer_low = (uint16_t)(registers->dp + 0x5du);
    const uint16_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint16_t pointer_bank = (uint16_t)(pointer_low + 2u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high) ||
        !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t pointer = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                             ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (pointer + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + 1u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d4du, 2u, span.instructions != 0u);
    uint32_t source = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    source |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    source = (source + registers->y) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8d4fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_palette_pointer: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + 1u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d4fu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8d51u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_pointer_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d51u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d52u);
}
next_pointer_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d52u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d53u);
}
narrow_palette_value: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d53u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8d55u);
}
read_indirect_palette: {
    const uint16_t pointer_low = (uint16_t)(registers->dp + 0x56u);
    const uint16_t pointer_high = (uint16_t)(pointer_low + 1u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high))
        goto finished;
    const uint32_t pointer = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + pointer) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 0u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d55u, 2u, span.instructions != 0u);
    uint32_t source = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    source += ((uint32_t)registers->db << 16);
    source = (source) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8d57u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
apply_indirect_palette: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808d57u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808d57u, 0x20u, 0x808d5du, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
resume_indirect_palette: {
    if (!NativeSpanCanStep(&span, cpu, 4u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d5au, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 4u, 0x88aau);
    goto finished;
}
read_attribute: {
    const uint32_t address = ((uint32_t)registers->db << 16) + 0x0564u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + 0u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d91u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8d94u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
shift_attribute_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d94u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)registers->a >> 1, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d95u);
}
shift_attribute_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d95u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)registers->a >> 1, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d96u);
}
mask_palette: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d96u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x07u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d98u);
}
save_palette: {
    const uint32_t address = ((uint32_t)registers->db << 16) + 0x0577u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + 0u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsWramByte(address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d98u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8d9bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
resume_palette_save: {
    if (!NativeSpanCanStep(&span, cpu, 4u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d9bu, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 4u, 0x88aau);
    goto finished;
}
read_saved_palette: {
    const uint32_t address = ((uint32_t)registers->db << 16) + 0x0577u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + 0u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x808d9eu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8da1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
apply_saved_palette: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808da1u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808da1u, 0x20u, 0x808d5du, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
resume_saved_palette: {
    if (!NativeSpanCanStep(&span, cpu, 4u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808da4u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 4u, 0x88aau);
    goto finished;
}
read_stream_attribute: {
    const uint16_t pointer_low = (uint16_t)(registers->dp + 0x5du);
    const uint16_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint16_t pointer_bank = (uint16_t)(pointer_low + 2u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high) ||
        !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t pointer = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                             ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (pointer + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + 0u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x808da7u, 2u, span.instructions != 0u);
    uint32_t source = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    source |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    source = (source + registers->y) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8da9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_stream_attribute: {
    const uint32_t address = ((uint32_t)registers->db << 16) + 0x0564u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + 0u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsWramByte(address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x808da9u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8dacu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_attribute: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808dacu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8dadu);
}
resume_attribute: {
    if (!NativeSpanCanStep(&span, cpu, 4u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808dadu, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 4u, 0x88aau);
    goto finished;
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
