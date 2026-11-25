#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuSequentialTileBlockSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                      uint8_t opcode, uint64_t dynamic_epoch,
                                                      uint64_t write_epoch, unsigned step_budget,
                                                      long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x82831au && pc < 0x828344u)
        end_pc = 0x828344u;
    else if (pc >= 0x828344u && pc < 0x828345u)
        end_pc = 0x828345u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x828320u: width_neutral = true; break;
    case 0x828322u: byte_accumulator = true; break;
    case 0x828324u: byte_accumulator = true; break;
    case 0x828325u: byte_accumulator = true; break;
    case 0x828327u: width_neutral = true; break;
    case 0x828344u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x82831au;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 43u ||
        memcmp(cart->rom + first,
               "\x48\x64\x56\x64\x58\x8a\xe2\x20\x85\x58\xeb\x85\x56\xc2\x20\xfa\x98\xa4\x56\xda\x9f\x00\x20"
               "\x7e\xe8\xe8\x1a\x88\xd0\xf6\xa8\x68\x18\x69\x40\x00\xaa\x98\xc6\x58\xd0\xe7\x60",
               43u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x82831au: goto save_tile_position;
    case 0x82831bu: goto clear_columns;
    case 0x82831du: goto clear_rows;
    case 0x82831fu: goto packed_dimensions;
    case 0x828320u: goto narrow_dimensions;
    case 0x828322u: goto store_rows;
    case 0x828324u: goto exchange_dimensions;
    case 0x828325u: goto store_columns;
    case 0x828327u: goto widen_tiles;
    case 0x828329u: goto restore_tile_position;
    case 0x82832au: goto first_tile;
    case 0x82832bu: goto read_columns;
    case 0x82832du: goto save_row_position;
    case 0x82832eu: goto store_tile;
    case 0x828332u: goto next_tile_low;
    case 0x828333u: goto next_tile_high;
    case 0x828334u: goto next_tile_number;
    case 0x828335u: goto count_columns;
    case 0x828336u: goto repeat_columns;
    case 0x828338u: goto save_next_tile;
    case 0x828339u: goto restore_row_position;
    case 0x82833au: goto clear_row_carry;
    case 0x82833bu: goto next_row_position;
    case 0x82833eu: goto select_next_row;
    case 0x82833fu: goto restore_next_tile;
    case 0x828340u: goto count_rows;
    case 0x828342u: goto repeat_rows;
    case 0x828344u: goto return_block;
    default: return 0;
    }
save_tile_position: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82831au, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x831bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_columns: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82831bu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x831du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_rows: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82831du, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x831fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
packed_dimensions: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82831fu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->x, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8320u);
}
narrow_dimensions: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828320u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8322u);
}
store_rows: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828322u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8324u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
exchange_dimensions: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828324u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8325u);
}
store_columns: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828325u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8327u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_tiles: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828327u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8329u);
}
restore_tile_position: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x828329u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x832au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_tile: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82832au, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x832bu);
}
read_columns: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82832bu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x832du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_row_position: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82832du, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x832eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_tile: {
    const uint32_t address = (0x7e2000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82832eu, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8332u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_tile_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828332u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8333u);
}
next_tile_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828333u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8334u);
}
next_tile_number: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828334u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a + 1u) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8335u);
}
count_columns: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828335u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8336u);
}
repeat_columns: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828336u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x832eu : 0x8338u);
    if (taken)
        goto finished;
}
save_next_tile: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828338u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8339u);
}
restore_row_position: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x828339u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa9u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x833au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_row_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82833au, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x833bu);
}
next_row_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82833bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0040u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x833eu);
}
select_next_row: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82833eu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x833fu);
}
restore_next_tile: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82833fu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8340u);
}
count_rows: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828340u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8342u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_rows: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828342u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x832bu : 0x8344u);
    if (taken)
        goto finished;
    goto finished;
}
return_block: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828344u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x828344u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuCursorStyleSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                              uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                              unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x82895bu && pc < 0x82897cu)
        end_pc = 0x82897cu;
    else if (pc >= 0x82897cu && pc < 0x82897du)
        end_pc = 0x82897du;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x82895bu: byte_accumulator = true; break;
    case 0x82895eu: width_neutral = true; break;
    case 0x82896au: width_neutral = true; break;
    case 0x82896cu: byte_accumulator = true; break;
    case 0x82896fu: byte_accumulator = true; break;
    case 0x828972u: byte_accumulator = true; break;
    case 0x828974u: byte_accumulator = true; break;
    case 0x828977u: byte_accumulator = true; break;
    case 0x828978u: byte_accumulator = true; break;
    case 0x82897cu: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x82895bu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 34u ||
        memcmp(cart->rom + first,
               "\x8e\xa9\x14\xc2\x20\x29\xff\x00\x3a\x0a\xaa\xbf\x7d\x89\x82\xe2\x20\xae\xa9\x14\x9d\x48\x14"
               "\xa9\x01\x9d\xd8\x11\xeb\x22\xf5\x8c\x86\x60",
               34u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x82895bu: goto save_cursor_slot;
    case 0x82895eu: goto widen_style;
    case 0x828960u: goto mask_style;
    case 0x828963u: goto previous_style;
    case 0x828964u: goto double_style;
    case 0x828965u: goto select_style;
    case 0x828966u: goto read_style;
    case 0x82896au: goto narrow_style;
    case 0x82896cu: goto restore_cursor_slot;
    case 0x82896fu: goto store_frame;
    case 0x828972u: goto enable_cursor;
    case 0x828974u: goto store_enabled;
    case 0x828977u: goto exchange_style;
    case 0x828978u: goto refresh_cursor;
    case 0x82897cu: goto return_style;
    default: return 0;
    }
save_cursor_slot: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14a9u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82895bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, 5u, 0x895eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_style: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82895eu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8960u);
}
mask_style: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828960u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8963u);
}
previous_style: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828963u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a - 1u) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8964u);
}
double_style: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828964u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8965u);
}
select_style: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828965u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8966u);
}
read_style: {
    const uint32_t address = (0x82897du + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828966u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x896au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_style: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82896au, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x896cu);
}
restore_cursor_slot: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14a9u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82896cu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x896fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_frame: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1448u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82896fu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8972u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
enable_cursor: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828972u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8974u);
}
store_enabled: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x11d8u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828974u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8977u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
exchange_style: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828977u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8978u);
}
refresh_cursor: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828978u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828978u, 0x22u, 0x868cf5u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_style: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82897cu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x82897cu, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuStyleBlockSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
    uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget, long *step_count) {
    if (pc >= 0x82831au && pc <= 0x828344u)
        return Lufia2DecompRunMenuSequentialTileBlockSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x82895bu && pc <= 0x82897cu)
        return Lufia2DecompRunMenuCursorStyleSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    return 0;
}
