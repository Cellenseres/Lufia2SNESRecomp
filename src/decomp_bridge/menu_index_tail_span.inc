#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuCursorActionTailSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                   uint8_t opcode, uint64_t dynamic_epoch,
                                                   uint64_t write_epoch, unsigned step_budget,
                                                   long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x82889cu && pc < 0x82889du)
        end_pc = 0x82889du;
    else if (pc >= 0x82889du && pc < 0x82889eu)
        end_pc = 0x82889eu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x82889cu: width_neutral = true; break;
    case 0x82889du: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x82889cu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 2u || memcmp(cart->rom + first, "\x18\x60", 2u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x82889cu: goto clear_action_carry;
    case 0x82889du: goto return_action;
    default: return 0;
    }
clear_action_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82889cu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x889du);
    goto finished;
}
return_action: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82889du);
    const int cycles = Lufia2DecompRunReturn(registers, 0x82889du, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuCursorEmptyTailSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                  uint8_t opcode, uint64_t dynamic_epoch,
                                                  uint64_t write_epoch, unsigned step_budget,
                                                  long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x82889eu && pc < 0x82889fu)
        end_pc = 0x82889fu;
    else if (pc >= 0x82889fu && pc < 0x8288a0u)
        end_pc = 0x8288a0u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x82889eu: width_neutral = true; break;
    case 0x82889fu: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x82889eu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 2u || memcmp(cart->rom + first, "\x38\x60", 2u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x82889eu: goto set_empty_carry;
    case 0x82889fu: goto return_empty;
    default: return 0;
    }
set_empty_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82889eu, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0x889fu);
    goto finished;
}
return_empty: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82889fu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x82889fu, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuItemIndexSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                            uint64_t dynamic_epoch, uint64_t write_epoch,
                                            unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x8288a0u && pc < 0x8288cau)
        end_pc = 0x8288cau;
    else if (pc >= 0x8288cau && pc < 0x8288cbu)
        end_pc = 0x8288cbu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x8288a0u: width_neutral = true; break;
    case 0x8288c8u: width_neutral = true; break;
    case 0x8288cau: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x8288a0u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 43u ||
        memcmp(cart->rom + first,
               "\xc2\x20\xbd\xf1\x14\x29\xff\x00\x8d\x70\x15\xbd\xeb\x14\x29\xff\x00\x8d\x72\x15\x22\x00\x80"
               "\x82\xbd\xd9\x14\x29\xff\x00\x18\x6d\x74\x15\x8d\x74\x15\x8d\xb3\x14\xe2\x20\x60",
               43u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x8288a0u: goto widen_index;
    case 0x8288a2u: goto read_stride;
    case 0x8288a5u: goto mask_stride;
    case 0x8288a8u: goto store_stride;
    case 0x8288abu: goto read_column;
    case 0x8288aeu: goto mask_column;
    case 0x8288b1u: goto store_column;
    case 0x8288b4u: goto multiply_column;
    case 0x8288b8u: goto read_base;
    case 0x8288bbu: goto mask_base;
    case 0x8288beu: goto clear_index_carry;
    case 0x8288bfu: goto add_product;
    case 0x8288c2u: goto store_product;
    case 0x8288c5u: goto store_index;
    case 0x8288c8u: goto narrow_index;
    case 0x8288cau: goto return_index;
    default: return 0;
    }
widen_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288a0u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x88a2u);
}
read_stride: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14f1u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x14f1u >> 8) != ((0x14f1u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288a2u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88a5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_stride: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288a5u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x88a8u);
}
store_stride: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1570u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288a8u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88abu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_column: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14ebu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x14ebu >> 8) != ((0x14ebu + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288abu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88aeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_column: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288aeu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x88b1u);
}
store_column: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1572u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288b1u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88b4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
multiply_column: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8288b4u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8288b4u, 0x22u, 0x828000u, true, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_base: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14d9u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x14d9u >> 8) != ((0x14d9u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288b8u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88bbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_base: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288bbu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x88beu);
}
clear_index_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288beu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x88bfu);
}
add_product: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1574u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288bfu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88c2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_product: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1574u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288c2u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88c5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_index: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14b3u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288c5u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x88c8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8288c8u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x88cau);
    goto finished;
}
return_index: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8288cau);
    const int cycles = Lufia2DecompRunReturn(registers, 0x8288cau, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuIndexTailSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
    uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget, long *step_count) {
    if (pc >= 0x82889cu && pc <= 0x82889du)
        return Lufia2DecompRunMenuCursorActionTailSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x82889eu && pc <= 0x82889fu)
        return Lufia2DecompRunMenuCursorEmptyTailSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x8288a0u && pc <= 0x8288cau)
        return Lufia2DecompRunMenuItemIndexSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    return 0;
}
