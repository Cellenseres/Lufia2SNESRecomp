#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuInventoryRowsSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x8284d5u && pc < 0x8284e1u)
        end_pc = 0x8284e1u;
    else if (pc >= 0x8284e1u && pc < 0x8284e4u)
        end_pc = 0x8284e4u;
    else if (pc >= 0x8284e4u && pc < 0x8284f6u)
        end_pc = 0x8284f6u;
    else if (pc >= 0x8284f6u && pc < 0x8284f7u)
        end_pc = 0x8284f7u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x8284d5u: byte_accumulator = true; break;
    case 0x8284d7u: byte_accumulator = true; break;
    case 0x8284d9u: width_neutral = true; break;
    case 0x8284f0u: width_neutral = true; break;
    case 0x8284f2u: byte_accumulator = true; break;
    case 0x8284f4u: byte_accumulator = true; break;
    case 0x8284f6u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x8284d5u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 34u ||
        memcmp(cart->rom + first,
               "\xa9\x8e\x85\x5f\xc2\x20\xa5\x00\x0a\xaa\xa5\x11\x20\xf7\x84\xa5\x11\x18\x69\x80\x00\x85\x11"
               "\xc6\x15\xd0\xef\xe2\x20\xa9\x08\x04\x74\x60",
               34u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x8284d5u: goto name_source_bank;
    case 0x8284d7u: goto store_name_source_bank;
    case 0x8284d9u: goto widen_inventory_index;
    case 0x8284dbu: goto read_first_inventory_item;
    case 0x8284ddu: goto scale_inventory_word_index;
    case 0x8284deu: goto select_inventory_item;
    case 0x8284dfu: goto read_row_position;
    case 0x8284e1u: goto draw_inventory_row;
    case 0x8284e4u: goto read_next_row_position;
    case 0x8284e6u: goto clear_row_carry;
    case 0x8284e7u: goto offset_next_row;
    case 0x8284eau: goto store_next_row;
    case 0x8284ecu: goto count_inventory_row;
    case 0x8284eeu: goto repeat_inventory_rows;
    case 0x8284f0u: goto narrow_dirty_bits;
    case 0x8284f2u: goto row_dirty_mask;
    case 0x8284f4u: goto request_tile_upload;
    case 0x8284f6u: goto return_inventory_rows;
    default: return 0;
    }
name_source_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284d5u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x008eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x84d7u);
}
store_name_source_bank: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284d7u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x84d9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_inventory_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284d9u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x84dbu);
}
read_first_inventory_item: {
    const uint32_t address = (uint16_t)(registers->dp + 0x00u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284dbu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x84ddu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
scale_inventory_word_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284ddu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x84deu);
}
select_inventory_item: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284deu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x84dfu);
}
read_row_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284dfu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x84e1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_inventory_row: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8284e1u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8284e1u, 0x20u, 0x8284f7u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_next_row_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284e4u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x84e6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_row_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284e6u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x84e7u);
}
offset_next_row: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284e7u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0080u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x84eau);
}
store_next_row: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284eau, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x84ecu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
count_inventory_row: {
    const uint32_t address = (uint16_t)(registers->dp + 0x15u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284ecu, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x84eeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_inventory_rows: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284eeu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x84dfu : 0x84f0u);
    if (taken)
        goto finished;
}
narrow_dirty_bits: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284f0u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x84f2u);
}
row_dirty_mask: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284f2u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0008u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x84f4u);
}
request_tile_upload: {
    const uint32_t address = (uint16_t)(registers->dp + 0x74u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284f4u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0x04u, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x84f6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_inventory_rows: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8284f6u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x8284f6u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuSpellRowsSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                            uint64_t dynamic_epoch, uint64_t write_epoch,
                                            unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x828526u && pc < 0x828530u)
        end_pc = 0x828530u;
    else if (pc >= 0x828530u && pc < 0x828533u)
        end_pc = 0x828533u;
    else if (pc >= 0x828533u && pc < 0x828539u)
        end_pc = 0x828539u;
    else if (pc >= 0x828539u && pc < 0x82853cu)
        end_pc = 0x82853cu;
    else if (pc >= 0x82853cu && pc < 0x82854eu)
        end_pc = 0x82854eu;
    else if (pc >= 0x82854eu && pc < 0x82854fu)
        end_pc = 0x82854fu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x828526u: byte_accumulator = true; break;
    case 0x828528u: byte_accumulator = true; break;
    case 0x82852au: byte_accumulator = true; break;
    case 0x82852cu: width_neutral = true; break;
    case 0x828548u: width_neutral = true; break;
    case 0x82854au: byte_accumulator = true; break;
    case 0x82854cu: byte_accumulator = true; break;
    case 0x82854eu: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x828526u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 41u ||
        memcmp(cart->rom + first,
               "\xa9\x8e\x85\x5f\xa4\x00\xc2\x20\xa5\x11\x20\x4f\x85\xa5\x11\x18\x69\x1c\x00\x20\x4f\x85\xa5"
               "\x11\x18\x69\x80\x00\x85\x11\xc6\x15\xd0\xe6\xe2\x20\xa9\x08\x04\x74\x60",
               41u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x828526u: goto name_source_bank;
    case 0x828528u: goto store_name_source_bank;
    case 0x82852au: goto read_first_spell;
    case 0x82852cu: goto widen_row_position;
    case 0x82852eu: goto read_first_row_position;
    case 0x828530u: goto draw_left_spell;
    case 0x828533u: goto read_right_row_position;
    case 0x828535u: goto clear_right_column_carry;
    case 0x828536u: goto offset_right_column;
    case 0x828539u: goto draw_right_spell;
    case 0x82853cu: goto read_next_row_position;
    case 0x82853eu: goto clear_row_carry;
    case 0x82853fu: goto offset_next_row;
    case 0x828542u: goto store_next_row;
    case 0x828544u: goto count_spell_row;
    case 0x828546u: goto repeat_spell_rows;
    case 0x828548u: goto narrow_dirty_bits;
    case 0x82854au: goto row_dirty_mask;
    case 0x82854cu: goto request_tile_upload;
    case 0x82854eu: goto return_spell_rows;
    default: return 0;
    }
name_source_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828526u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x008eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8528u);
}
store_name_source_bank: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828528u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x852au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_first_spell: {
    const uint32_t address = (uint16_t)(registers->dp + 0x00u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82852au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x852cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_row_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82852cu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x852eu);
}
read_first_row_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82852eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8530u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_left_spell: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828530u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828530u, 0x20u, 0x82854fu, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_right_row_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828533u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8535u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_right_column_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828535u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8536u);
}
offset_right_column: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828536u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x001cu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8539u);
    goto finished;
}
draw_right_spell: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828539u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828539u, 0x20u, 0x82854fu, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_next_row_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82853cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x853eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_row_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82853eu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x853fu);
}
offset_next_row: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82853fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0080u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8542u);
}
store_next_row: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828542u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8544u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
count_spell_row: {
    const uint32_t address = (uint16_t)(registers->dp + 0x15u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828544u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8546u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_spell_rows: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828546u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x852eu : 0x8548u);
    if (taken)
        goto finished;
}
narrow_dirty_bits: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828548u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x854au);
}
row_dirty_mask: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82854au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0008u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x854cu);
}
request_tile_upload: {
    const uint32_t address = (uint16_t)(registers->dp + 0x74u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82854cu, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0x04u, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x854eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_spell_rows: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82854eu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x82854eu, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuScenarioRowsSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                               uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                               unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x8286c4u && pc < 0x8286cfu)
        end_pc = 0x8286cfu;
    else if (pc >= 0x8286cfu && pc < 0x8286d2u)
        end_pc = 0x8286d2u;
    else if (pc >= 0x8286d2u && pc < 0x8286e4u)
        end_pc = 0x8286e4u;
    else if (pc >= 0x8286e4u && pc < 0x8286e5u)
        end_pc = 0x8286e5u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x8286c4u: byte_accumulator = true; break;
    case 0x8286c6u: byte_accumulator = true; break;
    case 0x8286c8u: width_neutral = true; break;
    case 0x8286deu: width_neutral = true; break;
    case 0x8286e0u: byte_accumulator = true; break;
    case 0x8286e2u: byte_accumulator = true; break;
    case 0x8286e4u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x8286c4u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 33u ||
        memcmp(cart->rom + first,
               "\xa9\x8e\x85\x5f\xc2\x20\xa5\x00\xaa\xa5\x11\x20\xe5\x86\xa5\x11\x18\x69\x80\x00\x85\x11\xc6"
               "\x15\xd0\xef\xe2\x20\xa9\x08\x04\x74\x60",
               33u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x8286c4u: goto name_source_bank;
    case 0x8286c6u: goto store_name_source_bank;
    case 0x8286c8u: goto widen_scenario_index;
    case 0x8286cau: goto read_first_scenario_item;
    case 0x8286ccu: goto select_scenario_item;
    case 0x8286cdu: goto read_row_position;
    case 0x8286cfu: goto draw_scenario_row;
    case 0x8286d2u: goto read_next_row_position;
    case 0x8286d4u: goto clear_row_carry;
    case 0x8286d5u: goto offset_next_row;
    case 0x8286d8u: goto store_next_row;
    case 0x8286dau: goto count_scenario_row;
    case 0x8286dcu: goto repeat_scenario_rows;
    case 0x8286deu: goto narrow_dirty_bits;
    case 0x8286e0u: goto row_dirty_mask;
    case 0x8286e2u: goto request_tile_upload;
    case 0x8286e4u: goto return_scenario_rows;
    default: return 0;
    }
name_source_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286c4u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x008eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x86c6u);
}
store_name_source_bank: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286c6u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x86c8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_scenario_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286c8u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x86cau);
}
read_first_scenario_item: {
    const uint32_t address = (uint16_t)(registers->dp + 0x00u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286cau, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x86ccu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_scenario_item: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286ccu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x86cdu);
}
read_row_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286cdu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x86cfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_scenario_row: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8286cfu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8286cfu, 0x20u, 0x8286e5u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_next_row_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286d2u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x86d4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_row_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286d4u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x86d5u);
}
offset_next_row: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286d5u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0080u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x86d8u);
}
store_next_row: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286d8u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x86dau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
count_scenario_row: {
    const uint32_t address = (uint16_t)(registers->dp + 0x15u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286dau, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x86dcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_scenario_rows: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286dcu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x86cdu : 0x86deu);
    if (taken)
        goto finished;
}
narrow_dirty_bits: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286deu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x86e0u);
}
row_dirty_mask: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286e0u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0008u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x86e2u);
}
request_tile_upload: {
    const uint32_t address = (uint16_t)(registers->dp + 0x74u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286e2u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0x04u, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x86e4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_scenario_rows: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8286e4u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x8286e4u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuInventoryRowSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                               uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                               unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x8284f7u && pc < 0x828509u)
        end_pc = 0x828509u;
    else if (pc >= 0x828509u && pc < 0x82850cu)
        end_pc = 0x82850cu;
    else if (pc >= 0x82850cu && pc < 0x828513u)
        end_pc = 0x828513u;
    else if (pc >= 0x828513u && pc < 0x828517u)
        end_pc = 0x828517u;
    else if (pc >= 0x828517u && pc < 0x82851eu)
        end_pc = 0x82851eu;
    else if (pc >= 0x82851eu && pc < 0x828522u)
        end_pc = 0x828522u;
    else if (pc >= 0x828522u && pc < 0x828525u)
        end_pc = 0x828525u;
    else if (pc >= 0x828525u && pc < 0x828526u)
        end_pc = 0x828526u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x828507u: width_neutral = true; break;
    case 0x828509u: byte_accumulator = true; break;
    case 0x82850cu: width_neutral = true; break;
    case 0x828525u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x8284f7u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 47u ||
        memcmp(
            cart->rom + first,
            "\x85\x17\xda\xbd\x8d\x0a\xf0\x23\x85\x00\x29\xff\x01\x8d\x06\x0a\xe2\x20\x20\x1e\x84\xc2\x20\xa0"
            "\xb1\xc9\xa6\x17\x22\x78\x88\x80\x46\x00\xa0\xb7\xc9\xa6\x17\x22\x78\x88\x80\xfa\xe8\xe8\x60",
            47u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x8284f7u: goto store_row_position;
    case 0x8284f9u: goto save_inventory_item;
    case 0x8284fau: goto read_inventory_record;
    case 0x8284fdu: goto skip_empty_record;
    case 0x8284ffu: goto store_inventory_record;
    case 0x828501u: goto isolate_item_id;
    case 0x828504u: goto store_item_id;
    case 0x828507u: goto narrow_item_attribute;
    case 0x828509u: goto draw_item_attribute;
    case 0x82850cu: goto widen_text_position;
    case 0x82850eu: goto item_name_text;
    case 0x828511u: goto read_name_position;
    case 0x828513u: goto draw_item_name;
    case 0x828517u: goto scale_quantity_index;
    case 0x828519u: goto item_quantity_text;
    case 0x82851cu: goto read_quantity_position;
    case 0x82851eu: goto draw_item_quantity;
    case 0x828522u: goto restore_inventory_item;
    case 0x828523u: goto next_item_low;
    case 0x828524u: goto next_item_high;
    case 0x828525u: goto return_inventory_row;
    default: return 0;
    }
store_row_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x17u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284f7u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x84f9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_inventory_item: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x8284f9u, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x84fau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_inventory_record: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0a8du + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0a8du >> 8) != ((0x0a8du + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284fau, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x84fdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_empty_record: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284fdu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8522u : 0x84ffu);
    if (taken)
        goto finished;
}
store_inventory_record: {
    const uint32_t address = (uint16_t)(registers->dp + 0x00u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8284ffu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8501u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
isolate_item_id: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828501u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x01ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8504u);
}
store_item_id: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0a06u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828504u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8507u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_item_attribute: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828507u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8509u);
    goto finished;
}
draw_item_attribute: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828509u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828509u, 0x20u, 0x82841eu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
widen_text_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82850cu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x850eu);
}
item_name_text: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82850eu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0xc9b1u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8511u);
}
read_name_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x17u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828511u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8513u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_item_name: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828513u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828513u, 0x22u, 0x808878u, true, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
scale_quantity_index: {
    const uint32_t address = (uint16_t)(registers->dp + 0x00u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828517u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0x46u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8519u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
item_quantity_text: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828519u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0xc9b7u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x851cu);
}
read_quantity_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x17u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82851cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x851eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_item_quantity: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82851eu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x82851eu, 0x22u, 0x808878u, true, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_inventory_item: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x828522u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x8523u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_item_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828523u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8524u);
}
next_item_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828524u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8525u);
    goto finished;
}
return_inventory_row: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828525u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x828525u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuSpellRowSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                           uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget,
                                           long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x82854fu && pc < 0x82855fu)
        end_pc = 0x82855fu;
    else if (pc >= 0x82855fu && pc < 0x828562u)
        end_pc = 0x828562u;
    else if (pc >= 0x828562u && pc < 0x828567u)
        end_pc = 0x828567u;
    else if (pc >= 0x828567u && pc < 0x82856bu)
        end_pc = 0x82856bu;
    else if (pc >= 0x82856bu && pc < 0x828570u)
        end_pc = 0x828570u;
    else if (pc >= 0x828570u && pc < 0x828574u)
        end_pc = 0x828574u;
    else if (pc >= 0x828574u && pc < 0x828578u)
        end_pc = 0x828578u;
    else if (pc >= 0x828578u && pc < 0x828579u)
        end_pc = 0x828579u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x828551u: width_neutral = true; break;
    case 0x828553u: byte_accumulator = true; break;
    case 0x828554u: byte_accumulator = true; break;
    case 0x828556u: byte_accumulator = true; break;
    case 0x828558u: byte_accumulator = true; break;
    case 0x82855au: byte_accumulator = true; break;
    case 0x82855cu: byte_accumulator = true; break;
    case 0x82855fu: byte_accumulator = true; break;
    case 0x828562u: byte_accumulator = true; break;
    case 0x828565u: byte_accumulator = true; break;
    case 0x828567u: byte_accumulator = true; break;
    case 0x82856bu: byte_accumulator = true; break;
    case 0x82856eu: byte_accumulator = true; break;
    case 0x828570u: byte_accumulator = true; break;
    case 0x828574u: width_neutral = true; break;
    case 0x828578u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x82854fu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 42u ||
        memcmp(cart->rom + first,
               "\x85\x17\xe2\x20\x5a\xb1\x2a\xc9\xff\xf0\x1a\x85\x00\x8d\x0b\x0a\x20\x83\x84\xa0\x0c\xca\xa6"
               "\x17\x22\x78\x88\x80\xa0\x34\xca\xa6\x17\x22\x78\x88\x80\xc2\x20\x7a\xc8\x60",
               42u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x82854fu: goto store_row_position;
    case 0x828551u: goto narrow_spell_id;
    case 0x828553u: goto save_spell_index;
    case 0x828554u: goto read_spell_id;
    case 0x828556u: goto test_empty_spell;
    case 0x828558u: goto skip_empty_spell;
    case 0x82855au: goto store_spell_id;
    case 0x82855cu: goto store_spell_record;
    case 0x82855fu: goto draw_spell_attribute;
    case 0x828562u: goto spell_name_text;
    case 0x828565u: goto read_name_position;
    case 0x828567u: goto draw_spell_name;
    case 0x82856bu: goto spell_cost_text;
    case 0x82856eu: goto read_cost_position;
    case 0x828570u: goto draw_spell_cost;
    case 0x828574u: goto widen_spell_index;
    case 0x828576u: goto restore_spell_index;
    case 0x828577u: goto next_spell;
    case 0x828578u: goto return_spell_row;
    default: return 0;
    }
store_row_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x17u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82854fu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8551u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_spell_id: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828551u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8553u);
}
save_spell_index: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x828553u, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x8554u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_spell_id: {
    const uint32_t pointer_low = (uint16_t)(registers->dp + 0x2au);
    const uint32_t pointer_high = (uint16_t)(pointer_low + 1u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high))
        goto finished;
    const unsigned expected = cpu->ram[pointer_low] | ((unsigned)cpu->ram[pointer_high] << 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + expected + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + direct_penalty + ((expected >> 8) != ((expected + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828554u, 2u, span.instructions != 0u);
    const uint16_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    const uint32_t source = (((uint32_t)registers->db << 16) + pointer + registers->y) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8556u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_empty_spell: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828556u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8558u);
}
skip_empty_spell: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828558u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8574u : 0x855au);
    if (taken)
        goto finished;
}
store_spell_id: {
    const uint32_t address = (uint16_t)(registers->dp + 0x00u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82855au, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x855cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_spell_record: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0a0bu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82855cu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x855fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_spell_attribute: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82855fu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x82855fu, 0x20u, 0x828483u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
spell_name_text: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828562u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0xca0cu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8565u);
}
read_name_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x17u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828565u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8567u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_spell_name: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828567u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828567u, 0x22u, 0x808878u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
spell_cost_text: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82856bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0xca34u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x856eu);
}
read_cost_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x17u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82856eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8570u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_spell_cost: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828570u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828570u, 0x22u, 0x808878u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
widen_spell_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828574u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8576u);
}
restore_spell_index: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x828576u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x8577u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_spell: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828577u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8578u);
    goto finished;
}
return_spell_row: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828578u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x828578u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuScenarioRowSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                              uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                              unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x8286e5u && pc < 0x8286e8u)
        end_pc = 0x8286e8u;
    else if (pc >= 0x8286e8u && pc < 0x8286ebu)
        end_pc = 0x8286ebu;
    else if (pc >= 0x8286ebu && pc < 0x8286fdu)
        end_pc = 0x8286fdu;
    else if (pc >= 0x8286fdu && pc < 0x828701u)
        end_pc = 0x828701u;
    else if (pc >= 0x828701u && pc < 0x828703u)
        end_pc = 0x828703u;
    else if (pc >= 0x828703u && pc < 0x828704u)
        end_pc = 0x828704u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x8286efu: width_neutral = true; break;
    case 0x8286f1u: byte_accumulator = true; break;
    case 0x8286f3u: byte_accumulator = true; break;
    case 0x8286f6u: width_neutral = true; break;
    case 0x828703u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x8286e5u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 31u ||
        memcmp(cart->rom + first,
               "\xda\x85\x17\x20\x3f\xfc\xb0\x14\x85\x00\xe2\x20\xa9\x20\x8d\x64\x05\xc2\x20\xa0\xb1\xc9\xa6"
               "\x17\x22\x78\x88\x80\xfa\xe8\x60",
               31u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x8286e5u: goto save_scenario_index;
    case 0x8286e6u: goto store_row_position;
    case 0x8286e8u: goto find_scenario_item;
    case 0x8286ebu: goto skip_missing_item;
    case 0x8286edu: goto store_item_id;
    case 0x8286efu: goto narrow_item_attribute;
    case 0x8286f1u: goto scenario_text_attribute;
    case 0x8286f3u: goto store_text_attribute;
    case 0x8286f6u: goto widen_text_position;
    case 0x8286f8u: goto scenario_name_text;
    case 0x8286fbu: goto read_name_position;
    case 0x8286fdu: goto draw_scenario_name;
    case 0x828701u: goto restore_scenario_index;
    case 0x828702u: goto next_scenario_item;
    case 0x828703u: goto return_scenario_row;
    default: return 0;
    }
save_scenario_index: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x8286e5u, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x86e6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_row_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x17u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286e6u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x86e8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
find_scenario_item: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8286e8u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8286e8u, 0x20u, 0x82fc3fu, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
skip_missing_item: {
    const bool taken = registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286ebu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8701u : 0x86edu);
    if (taken)
        goto finished;
}
store_item_id: {
    const uint32_t address = (uint16_t)(registers->dp + 0x00u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286edu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x86efu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_item_attribute: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286efu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x86f1u);
}
scenario_text_attribute: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286f1u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0020u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x86f3u);
}
store_text_attribute: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0564u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286f3u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x86f6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_text_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286f6u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x86f8u);
}
scenario_name_text: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286f8u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0xc9b1u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x86fbu);
}
read_name_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x17u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8286fbu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x86fdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_scenario_name: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8286fdu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8286fdu, 0x22u, 0x808878u, true, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_scenario_index: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x828701u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x8702u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_scenario_item: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828702u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8703u);
    goto finished;
}
return_scenario_row: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828703u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x828703u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuScenarioIndexSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x82fc3fu && pc < 0x82fc70u)
        end_pc = 0x82fc70u;
    else if (pc >= 0x82fc70u && pc < 0x82fc7cu)
        end_pc = 0x82fc7cu;
    else if (pc >= 0x82fc7cu && pc < 0x82fc7du)
        end_pc = 0x82fc7du;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x82fc70u: width_neutral = true; break;
    case 0x82fc7cu: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x82fc3fu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 62u ||
        memcmp(cart->rom + first,
               "\xe8\x64\x54\x64\x56\xa0\x00\x00\xb9\x1e\x09\xf0\x12\x84\x58\xa0\x10\x00\xe6\x54\x4a\x90\x03"
               "\xca\xf0\x18\x88\xd0\xf5\xa4\x58\xa5\x56\x18\x69\x10\x00\x85\x56\x85\x54\xc8\xc8\xc0\x08\x00"
               "\xd0\xd8\x38\x60\xc6\x54\x06\x54\xa6\x54\xbf\xa0\xfd\x97\x18\x60",
               62u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x82fc3fu: goto next_requested_item;
    case 0x82fc40u: goto clear_item_position;
    case 0x82fc42u: goto clear_word_position;
    case 0x82fc44u: goto first_flag_word;
    case 0x82fc47u: goto read_flag_word;
    case 0x82fc4au: goto skip_empty_word;
    case 0x82fc4cu: goto save_word_position;
    case 0x82fc4eu: goto bits_per_word;
    case 0x82fc51u: goto count_item_bit;
    case 0x82fc53u: goto shift_item_flag;
    case 0x82fc54u: goto skip_unset_item;
    case 0x82fc56u: goto count_requested_item;
    case 0x82fc57u: goto found_requested_item;
    case 0x82fc59u: goto next_flag_bit;
    case 0x82fc5au: goto repeat_word_bits;
    case 0x82fc5cu: goto restore_word_position;
    case 0x82fc5eu: goto read_word_position;
    case 0x82fc60u: goto clear_word_carry;
    case 0x82fc61u: goto offset_next_word;
    case 0x82fc64u: goto store_word_position;
    case 0x82fc66u: goto store_item_position;
    case 0x82fc68u: goto next_word_low;
    case 0x82fc69u: goto next_word_high;
    case 0x82fc6au: goto test_last_word;
    case 0x82fc6du: goto repeat_flag_words;
    case 0x82fc6fu: goto mark_item_missing;
    case 0x82fc70u: goto return_missing_item;
    case 0x82fc71u: goto previous_item_position;
    case 0x82fc73u: goto scale_item_position;
    case 0x82fc75u: goto read_item_table_index;
    case 0x82fc77u: goto read_scenario_item_id;
    case 0x82fc7bu: goto mark_item_found;
    case 0x82fc7cu: goto return_found_item;
    default: return 0;
    }
next_requested_item: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc3fu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xfc40u);
}
clear_item_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc40u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfc42u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_word_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc42u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfc44u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_flag_word: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc44u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xfc47u);
}
read_flag_word: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x091eu + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x091eu >> 8) != ((0x091eu + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc47u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfc4au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_empty_word: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc4au, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xfc5eu : 0xfc4cu);
    if (taken)
        goto read_word_position;
}
save_word_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc4cu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->y, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfc4eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
bits_per_word: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc4eu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0010u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xfc51u);
}
count_item_bit: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc51u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfc53u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
shift_item_flag: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc53u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a & 0xffffu) >> 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xfc54u);
}
skip_unset_item: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc54u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xfc59u : 0xfc56u);
    if (taken)
        goto next_flag_bit;
}
count_requested_item: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc56u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xfc57u);
}
found_requested_item: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc57u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xfc71u : 0xfc59u);
    if (taken)
        goto finished;
}
next_flag_bit: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc59u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xfc5au);
}
repeat_word_bits: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc5au, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xfc51u : 0xfc5cu);
    if (taken)
        goto finished;
}
restore_word_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc5cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfc5eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_word_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc5eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfc60u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_word_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc60u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xfc61u);
}
offset_next_word: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc61u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0010u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xfc64u);
}
store_word_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc64u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfc66u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_item_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc66u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfc68u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_word_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc68u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xfc69u);
}
next_word_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc69u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xfc6au);
}
test_last_word: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc6au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc0u, 0x0008u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xfc6du);
}
repeat_flag_words: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc6du, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xfc47u : 0xfc6fu);
    if (taken)
        goto finished;
}
mark_item_missing: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc6fu, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xfc70u);
    goto finished;
}
return_missing_item: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82fc70u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x82fc70u, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
previous_item_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc71u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfc73u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
scale_item_position: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc73u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0x06u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfc75u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_item_table_index: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc75u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfc77u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_scenario_item_id: {
    const uint32_t address = (0x97fda0u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc77u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfc7bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mark_item_found: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82fc7bu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xfc7cu);
    goto finished;
}
return_found_item: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82fc7cu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x82fc7cu, 0x60u, false, registers->mf, true);
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

static int Lufia2DecompRunMenuSelectionRowsSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                unsigned step_budget, long *step_count) {
    if (pc >= 0x8284d5u && pc < 0x8284f7u)
        return Lufia2DecompRunMenuInventoryRowsSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                    step_budget, step_count);
    if (pc >= 0x828526u && pc < 0x82854fu)
        return Lufia2DecompRunMenuSpellRowsSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                step_budget, step_count);
    if (pc >= 0x8286c4u && pc < 0x8286e5u)
        return Lufia2DecompRunMenuScenarioRowsSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                   step_budget, step_count);
    if (pc >= 0x8284f7u && pc < 0x828526u)
        return Lufia2DecompRunMenuInventoryRowSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                   step_budget, step_count);
    if (pc >= 0x82854fu && pc < 0x828579u)
        return Lufia2DecompRunMenuSpellRowSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                               step_budget, step_count);
    if (pc >= 0x8286e5u && pc < 0x828704u)
        return Lufia2DecompRunMenuScenarioRowSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                  step_budget, step_count);
    if (pc >= 0x82fc3fu && pc < 0x82fc7du)
        return Lufia2DecompRunMenuScenarioIndexSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                    step_budget, step_count);
    return 0;
}
