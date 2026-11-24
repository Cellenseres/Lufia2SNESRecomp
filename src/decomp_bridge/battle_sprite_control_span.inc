#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunBattleSpriteEffectSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                 uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                 unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x819df6u && pc < 0x819e02u) {
        end_pc = 0x819e02u;
    } else if (pc >= 0x819e02u && pc < 0x819e06u) {
        end_pc = 0x819e06u;
    } else if (pc >= 0x819e06u && pc < 0x819e13u) {
        end_pc = 0x819e13u;
    } else
        return 0;
    const bool byte_accumulator = pc != 0x819dfau;
    const bool changes_width = pc == 0x819df8u || pc == 0x819dfcu;
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->nmiWanted ||
        registers->irqWanted || (!changes_width && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x819df6u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 29u || memcmp(cart->rom + first,
                                              "\xa7\xc3\xc2\x20\xe6\xc3\xe2\x20\x5a\x8d\xab\x15\x22\x2f\x8a"
                                              "\x85\xa9\xff\x8d\xae\x15\xa9\xff\x8f\xf3\x12\x00\x7a\x60",
                                              29u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x819df6u: goto read_rebuild_mode;
    case 0x819df8u: goto widen_stream_pointer;
    case 0x819dfau: goto next_stream_byte;
    case 0x819dfcu: goto narrow_rebuild_mode;
    case 0x819dfeu: goto save_effect_index;
    case 0x819dffu: goto store_rebuild_mode;
    case 0x819e02u: goto rebuild_sprites;
    case 0x819e06u: goto background_request;
    case 0x819e08u: goto store_background_request;
    case 0x819e0bu: goto frame_request;
    case 0x819e0du: goto store_frame_request;
    case 0x819e11u: goto restore_effect_index;
    case 0x819e12u: goto return_effect;
    default: return 0;
    }
read_rebuild_mode: {
    const uint16_t pointer_low = (uint16_t)(registers->dp + 0xc3u);
    const uint16_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint16_t pointer_bank = (uint16_t)(pointer_low + 2u);
    const unsigned cycles = 6u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(pointer_low) ||
        !Lufia2DecompIsWramByte(pointer_high) || !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t pointer = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                             ((uint32_t)cpu->ram[pointer_bank] << 16);
    if (!Lufia2DecompIsDecompressSourceByte(cart, pointer))
        goto finished;
    Lufia2SpanFetch(registers, 0x819df6u, 2u, span.instructions != 0u);
    uint32_t source = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    source |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9df8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_stream_pointer: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819df8u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9dfau);
}
next_stream_byte: {
    const uint32_t address = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819dfau, 2u, span.instructions != 0u);
    const uint16_t previous = Lufia2DecompReadData(registers, address, high, true);
    const uint16_t updated = (uint16_t)(previous + 1u);
    registers->z = updated == 0u;
    registers->n = (updated & 0x8000u) != 0u;
    registers->write(registers->mem, high, (uint8_t)(updated >> 8));
    registers->write(registers->mem, address, (uint8_t)updated);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9dfcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_rebuild_mode: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819dfcu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9dfeu);
}
save_effect_index: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819dfeu, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp, (uint8_t)(value >> 8));
    --registers->sp;
    registers->write(registers->mem, registers->sp, (uint8_t)value);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, 4u, 0x9dffu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_rebuild_mode: {
    const uint32_t address = ((uint32_t)registers->db << 16) + 0x15abu;
    if (!NativeSpanCanStep(&span, cpu, 4u) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819dffu, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9e02u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
rebuild_sprites: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819e02u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819e02u, 0x22u, 0x858a2fu, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
background_request: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e06u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0xffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e08u);
}
store_background_request: {
    const uint32_t address = ((uint32_t)registers->db << 16) + 0x15aeu;
    if (!NativeSpanCanStep(&span, cpu, 4u) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e08u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9e0bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
frame_request: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e0bu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0xffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e0du);
}
store_frame_request: {
    const uint32_t address = 0x0012f3u;
    if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e0du, 4u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9e11u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_effect_index: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819e11u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9e12u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
return_effect: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819e12u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x819e12u, 0x60u, false, true, true);
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
static int Lufia2DecompRunBattleSpriteParentSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                 uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                 unsigned step_budget, long *step_count) {
    uint32_t first_pc, end_pc;
    if (pc >= 0x858a2fu && pc < 0x858a39u) {
        first_pc = 0x858a2fu;
        end_pc = 0x858a39u;
    } else if (pc >= 0x858a83u && pc < 0x858a90u) {
        first_pc = 0x858a83u;
        end_pc = 0x858a90u;
    } else if (pc >= 0x858a90u && pc < 0x858a94u) {
        first_pc = 0x858a90u;
        end_pc = 0x858a94u;
    } else if (pc >= 0x858a94u && pc < 0x858a95u) {
        first_pc = 0x858a94u;
        end_pc = 0x858a95u;
    } else if (pc >= 0x858a9eu && pc < 0x858aafu) {
        first_pc = 0x858a9eu;
        end_pc = 0x858aafu;
    } else
        return 0;
    const bool byte_accumulator = true;
    const bool changes_width = false;
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->nmiWanted ||
        registers->irqWanted || (!changes_width && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x858a2fu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 128u ||
        memcmp(cart->rom + first,
               "\xad\xab\x15\x3a\xf0\x37\x3a\xf0\x66\x6b\xad\xde\x11\xf0\x05\xa9\x01\x8d\xab\x15\xa2\x00\x28"
               "\xa0\xa0\x02\x8e\x81\x21\x9c\x83\x21\xa9\x01\x9c\x80\x21\x8d\x80\x21\x88\xd0\xf7\xad\xab\x15"
               "\x3a\xf0\x04\x3a\xf0\x33\x6b\x22\xaf\x8a\x85\x22\x22\x8b\x85\x22\x4b\x8b\x85\x22\xc0\x8b\x85"
               "\x22\x27\x8c\x85\xad\xde\x11\xd0\x06\x22\x2e\x8d\x85\x80\x12\xad\x5f\x12\xd0\x0d\x9c\xd3\x15"
               "\x8b\xa9\x7e\x48\xab\x22\x2e\x97\x85\xab\x6b\x22\xaf\x8a\x85\x22\xf4\x8a\x85\x22\x4b\x8b\x85"
               "\x22\xc0\x8b\x85\x22\x27\x8c\x85\x22\x98\x8c\x85\x6b",
               128u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x858a2fu: goto read_sprite_mode;
    case 0x858a32u: goto first_mode;
    case 0x858a33u: goto test_first_mode;
    case 0x858a35u: goto second_mode;
    case 0x858a36u: goto test_second_mode;
    case 0x858a38u: goto return_disabled;
    case 0x858a83u: goto read_tile_hold;
    case 0x858a86u: goto skip_tile_grid;
    case 0x858a88u: goto clear_tile_request;
    case 0x858a8bu: goto save_data_bank;
    case 0x858a8cu: goto tile_bank;
    case 0x858a8eu: goto save_tile_bank;
    case 0x858a8fu: goto select_tile_bank;
    case 0x858a90u: goto draw_tile_grid;
    case 0x858a94u: goto restore_data_bank;
    case 0x858a9eu: goto draw_second_records;
    case 0x858aa2u: goto draw_second_single;
    case 0x858aa6u: goto draw_second_markers;
    case 0x858aaau: goto draw_second_party;
    case 0x858aaeu: goto return_second_mode;
    default: return 0;
    }
read_sprite_mode: {
    const uint32_t address = ((uint32_t)registers->db << 16) + 0x15abu;
    if (!NativeSpanCanStep(&span, cpu, 4u) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x858a2fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, 4u, 0x8a32u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_mode: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x858a32u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a - 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8a33u);
}
test_first_mode: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x858a33u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8a6cu : 0x8a35u);
    if (taken)
        goto finished;
}
second_mode: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x858a35u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a - 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8a36u);
}
test_second_mode: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x858a36u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8a9eu : 0x8a38u);
    if (taken) {
        if (0x858a9eu < first_pc || 0x858a9eu >= end_pc)
            goto finished;
        goto draw_second_records;
    }
}
return_disabled: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x858a38u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x858a38u, 0x6bu, true, true, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_tile_hold: {
    const uint32_t address = ((uint32_t)registers->db << 16) + 0x125fu;
    if (!NativeSpanCanStep(&span, cpu, 4u) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x858a83u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, 4u, 0x8a86u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_tile_grid: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x858a86u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8a95u : 0x8a88u);
    if (taken)
        goto finished;
}
clear_tile_request: {
    const uint32_t address = ((uint32_t)registers->db << 16) + 0x15d3u;
    if (!NativeSpanCanStep(&span, cpu, 4u) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x858a88u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, 0u);
    NativeSpanStep(&span, registers, cpu, 4u, 0x8a8bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_data_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x858a8bu, 1u, span.instructions != 0u);
    const uint16_t value = registers->db;
    registers->write(registers->mem, registers->sp, (uint8_t)value);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, 3u, 0x8a8cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
tile_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x858a8cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x7eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8a8eu);
}
save_tile_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x858a8eu, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->write(registers->mem, registers->sp, (uint8_t)value);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, 3u, 0x8a8fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_tile_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x858a8fu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x8a90u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_tile_grid: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x858a90u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x858a90u, 0x22u, 0x85972eu, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_data_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x858a94u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x8a95u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_second_records: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x858a9eu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x858a9eu, 0x22u, 0x858b4bu, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_second_single: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x858aa2u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x858aa2u, 0x22u, 0x858bc0u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_second_markers: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x858aa6u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x858aa6u, 0x22u, 0x858c27u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_second_party: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x858aaau);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x858aaau, 0x22u, 0x858c98u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_second_mode: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x858aaeu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x858aaeu, 0x6bu, true, true, true);
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
static int Lufia2DecompRunBattleSpriteStripCallSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                    uint8_t opcode, uint64_t dynamic_epoch,
                                                    uint64_t write_epoch, unsigned step_budget,
                                                    long *step_count) {
    if ((pc != 0x858c18u && pc != 0x858c1eu) || !cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram ||
        registers->e || !step_budget || registers->waiting || registers->stopped || registers->nmiWanted ||
        registers->irqWanted || (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) ||
        registers->sp < 0x1f03u || registers->sp > 0x1fffu || !Lufia2DecompFrameRangeQuiet(pc + 1u, pc + 4u))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 4u, &offset);
    const uint32_t target = pc == 0x858c18u ? 0x81bd47u : 0x81bdc8u;
    const char *signature = pc == 0x858c18u ? "\x22\x47\xbd\x81" : "\x22\xc8\xbd\x81";
    if (!cart || memcmp(cart->rom + offset, signature, 4u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    if (!NativeSpanCanStep(&span, cpu, 8u))
        return 0;
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, pc, 0x22u, target, true, registers->mf, registers->xf);
    if (!cycles)
        return 0;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunBattleTileGridSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                             uint64_t dynamic_epoch, uint64_t write_epoch,
                                             unsigned step_budget, long *step_count) {
    if (pc < 0x85972eu || pc >= 0x8597d1u)
        return 0;
    const bool row = pc >= 0x859790u;
    const bool changes_width = pc == 0x85972eu || pc == 0x85978du;
    const bool byte_accumulator = pc == 0x85972eu || pc == 0x85978fu;
    const uint32_t end_pc = row              ? (pc < 0x8597d0u ? 0x8597d0u : 0x8597d1u)
                            : pc < 0x859733u ? 0x859733u
                            : pc < 0x85978du ? pc + 3u
                            : pc < 0x85978fu ? 0x85978fu
                                             : 0x859790u;
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->nmiWanted ||
        registers->irqWanted || (!changes_width && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x85972eu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 163u ||
        memcmp(
            cart->rom + first,
            "\xc2\x20\xa9\x10\x37\xa2\x16\x28\x20\x90\x97\xa2\x56\x28\x20\x90\x97\xa2\x96\x28\x20\x90\x97\xa2"
            "\xd6\x28\x20\x90\x97\xa2\x16\x29\x20\x90\x97\xa2\x56\x29\x20\x90\x97\xa2\x96\x29\x20\x90\x97\xa2"
            "\xd6\x29\x20\x90\x97\xa2\x16\x2a\x20\x90\x97\xa2\x56\x2a\x20\x90\x97\xa2\x96\x2a\x20\x90\x97\xa2"
            "\xd6\x2a\x20\x90\x97\xa2\x16\x2b\x20\x90\x97\xa2\x56\x2b\x20\x90\x97\xa2\x96\x2b\x20\x90\x97\xe2"
            "\x20\x6b\x9d\x00\x00\x1a\x9d\x02\x00\x1a\x9d\x04\x00\x1a\x9d\x06\x00\x1a\x9d\x08\x00\x1a\x9d\x0a"
            "\x00\x1a\x9d\x0c\x00\x1a\x9d\x0e\x00\x1a\x9d\x10\x00\x1a\x9d\x12\x00\x1a\x9d\x14\x00\x1a\x9d\x16"
            "\x00\x1a\x9d\x18\x00\x1a\x9d\x1a\x00\x1a\x9d\x1c\x00\x1a\x9d\x1e\x00\x1a\x60",
            163u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x85972eu: goto widen_tile_ids;
    case 0x859730u: goto first_tile_id;
    case 0x859733u: goto select_row_0;
    case 0x859736u: goto write_row_0;
    case 0x859739u: goto select_row_1;
    case 0x85973cu: goto write_row_1;
    case 0x85973fu: goto select_row_2;
    case 0x859742u: goto write_row_2;
    case 0x859745u: goto select_row_3;
    case 0x859748u: goto write_row_3;
    case 0x85974bu: goto select_row_4;
    case 0x85974eu: goto write_row_4;
    case 0x859751u: goto select_row_5;
    case 0x859754u: goto write_row_5;
    case 0x859757u: goto select_row_6;
    case 0x85975au: goto write_row_6;
    case 0x85975du: goto select_row_7;
    case 0x859760u: goto write_row_7;
    case 0x859763u: goto select_row_8;
    case 0x859766u: goto write_row_8;
    case 0x859769u: goto select_row_9;
    case 0x85976cu: goto write_row_9;
    case 0x85976fu: goto select_row_10;
    case 0x859772u: goto write_row_10;
    case 0x859775u: goto select_row_11;
    case 0x859778u: goto write_row_11;
    case 0x85977bu: goto select_row_12;
    case 0x85977eu: goto write_row_12;
    case 0x859781u: goto select_row_13;
    case 0x859784u: goto write_row_13;
    case 0x859787u: goto select_row_14;
    case 0x85978au: goto write_row_14;
    case 0x85978du: goto narrow_tile_ids;
    case 0x85978fu: goto return_grid;
    case 0x859790u: goto store_tile_0;
    case 0x859793u: goto next_tile_0;
    case 0x859794u: goto store_tile_1;
    case 0x859797u: goto next_tile_1;
    case 0x859798u: goto store_tile_2;
    case 0x85979bu: goto next_tile_2;
    case 0x85979cu: goto store_tile_3;
    case 0x85979fu: goto next_tile_3;
    case 0x8597a0u: goto store_tile_4;
    case 0x8597a3u: goto next_tile_4;
    case 0x8597a4u: goto store_tile_5;
    case 0x8597a7u: goto next_tile_5;
    case 0x8597a8u: goto store_tile_6;
    case 0x8597abu: goto next_tile_6;
    case 0x8597acu: goto store_tile_7;
    case 0x8597afu: goto next_tile_7;
    case 0x8597b0u: goto store_tile_8;
    case 0x8597b3u: goto next_tile_8;
    case 0x8597b4u: goto store_tile_9;
    case 0x8597b7u: goto next_tile_9;
    case 0x8597b8u: goto store_tile_10;
    case 0x8597bbu: goto next_tile_10;
    case 0x8597bcu: goto store_tile_11;
    case 0x8597bfu: goto next_tile_11;
    case 0x8597c0u: goto store_tile_12;
    case 0x8597c3u: goto next_tile_12;
    case 0x8597c4u: goto store_tile_13;
    case 0x8597c7u: goto next_tile_13;
    case 0x8597c8u: goto store_tile_14;
    case 0x8597cbu: goto next_tile_14;
    case 0x8597ccu: goto store_tile_15;
    case 0x8597cfu: goto next_tile_15;
    case 0x8597d0u: goto return_row;
    default: return 0;
    }
widen_tile_ids: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x85972eu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9730u);
}
first_tile_id: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x859730u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x3710u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9733u);
    goto finished;
}
select_row_0: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x859733u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x2816u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9736u);
    goto finished;
}
write_row_0: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x859736u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x859736u, 0x20u, 0x859790u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_row_1: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x859739u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x2856u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x973cu);
    goto finished;
}
write_row_1: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x85973cu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x85973cu, 0x20u, 0x859790u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_row_2: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x85973fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x2896u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9742u);
    goto finished;
}
write_row_2: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x859742u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x859742u, 0x20u, 0x859790u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_row_3: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x859745u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x28d6u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9748u);
    goto finished;
}
write_row_3: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x859748u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x859748u, 0x20u, 0x859790u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_row_4: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x85974bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x2916u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x974eu);
    goto finished;
}
write_row_4: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x85974eu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x85974eu, 0x20u, 0x859790u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_row_5: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x859751u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x2956u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9754u);
    goto finished;
}
write_row_5: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x859754u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x859754u, 0x20u, 0x859790u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_row_6: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x859757u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x2996u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x975au);
    goto finished;
}
write_row_6: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x85975au);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x85975au, 0x20u, 0x859790u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_row_7: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x85975du, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x29d6u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9760u);
    goto finished;
}
write_row_7: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x859760u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x859760u, 0x20u, 0x859790u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_row_8: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x859763u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x2a16u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9766u);
    goto finished;
}
write_row_8: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x859766u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x859766u, 0x20u, 0x859790u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_row_9: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x859769u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x2a56u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x976cu);
    goto finished;
}
write_row_9: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x85976cu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x85976cu, 0x20u, 0x859790u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_row_10: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x85976fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x2a96u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9772u);
    goto finished;
}
write_row_10: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x859772u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x859772u, 0x20u, 0x859790u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_row_11: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x859775u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x2ad6u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9778u);
    goto finished;
}
write_row_11: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x859778u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x859778u, 0x20u, 0x859790u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_row_12: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x85977bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x2b16u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x977eu);
    goto finished;
}
write_row_12: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x85977eu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x85977eu, 0x20u, 0x859790u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_row_13: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x859781u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x2b56u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9784u);
    goto finished;
}
write_row_13: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x859784u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x859784u, 0x20u, 0x859790u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_row_14: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x859787u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x2b96u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x978au);
    goto finished;
}
write_row_14: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x85978au);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x85978au, 0x20u, 0x859790u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
narrow_tile_ids: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x85978du, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x978fu);
    goto finished;
}
return_grid: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x85978fu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x85978fu, 0x6bu, true, true, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
store_tile_0: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 7u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x859790u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x9793u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_0: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x859793u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9794u);
}
store_tile_1: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0002u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 7u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x859794u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x9797u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_1: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x859797u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9798u);
}
store_tile_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0004u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 7u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x859798u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x979bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x85979bu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x979cu);
}
store_tile_3: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0006u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 7u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x85979cu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x979fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_3: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x85979fu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x97a0u);
}
store_tile_4: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0008u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 7u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597a0u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x97a3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_4: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597a3u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x97a4u);
}
store_tile_5: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000au + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 7u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597a4u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x97a7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_5: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597a7u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x97a8u);
}
store_tile_6: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000cu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 7u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597a8u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x97abu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_6: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597abu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x97acu);
}
store_tile_7: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 7u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597acu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x97afu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_7: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597afu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x97b0u);
}
store_tile_8: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0010u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 7u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597b0u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x97b3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_8: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597b3u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x97b4u);
}
store_tile_9: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0012u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 7u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597b4u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x97b7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_9: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597b7u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x97b8u);
}
store_tile_10: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0014u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 7u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597b8u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x97bbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_10: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597bbu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x97bcu);
}
store_tile_11: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0016u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 7u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597bcu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x97bfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_11: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597bfu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x97c0u);
}
store_tile_12: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0018u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 7u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597c0u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x97c3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_12: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597c3u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x97c4u);
}
store_tile_13: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x001au + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 7u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597c4u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x97c7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_13: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597c7u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x97c8u);
}
store_tile_14: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x001cu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 7u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597c8u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x97cbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_14: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597cbu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x97ccu);
}
store_tile_15: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x001eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 7u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597ccu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0x97cfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_15: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8597cfu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x97d0u);
    goto finished;
}
return_row: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8597d0u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x8597d0u, 0x60u, false, false, true);
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

static int Lufia2DecompRunBattleSpriteControlSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                  uint8_t opcode, uint64_t dynamic_epoch,
                                                  uint64_t write_epoch, unsigned step_budget,
                                                  long *step_count) {
    if (pc >= 0x819df6u && pc <= 0x819e12u)
        return Lufia2DecompRunBattleSpriteEffectSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                     step_budget, step_count);
    if (pc >= 0x858a2fu && pc <= 0x858aaeu)
        return Lufia2DecompRunBattleSpriteParentSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                     step_budget, step_count);
    if (pc >= 0x85972eu && pc <= 0x8597d0u)
        return Lufia2DecompRunBattleTileGridSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                 step_budget, step_count);
    return Lufia2DecompRunBattleSpriteStripCallSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                    step_budget, step_count);
}
