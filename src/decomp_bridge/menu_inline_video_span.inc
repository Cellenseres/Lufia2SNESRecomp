/* Calls, returns and the table jump start their own phase: the interpreter loop
 * takes its call and menu-return decisions from the first opcode of a span. */
#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuInlineTableSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                              uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                              unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x828028u && pc < 0x828041u)
        end_pc = 0x828041u;
    else if (pc >= 0x828041u && pc < 0x828044u)
        end_pc = 0x828044u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x828028u: width_neutral = true; break;
    case 0x828033u: width_neutral = true; break;
    case 0x828035u: byte_accumulator = true; break;
    case 0x828036u: byte_accumulator = true; break;
    case 0x828037u: byte_accumulator = true; break;
    case 0x828039u: width_neutral = true; break;
    case 0x82803fu: width_neutral = true; break;
    case 0x828041u: byte_accumulator = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x828028u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 28u || memcmp(cart->rom + first,
                                              "\xc2\x20\x29\xff\x00\x0a\x1a\xa8\x68\x85\x5d\xe2\x20\x4b\x68"
                                              "\x85\x5f\xc2\x20\xb7\x5d\x85\x60\xe2\x20\x6c\x60\x00",
                                              28u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x828028u: goto widen_accumulator;
    case 0x82802au: goto mask_accumulator;
    case 0x82802du: goto double_accumulator;
    case 0x82802eu: goto increment_accumulator;
    case 0x82802fu: goto copy_accumulator_to_y;
    case 0x828030u: goto pull_accumulator;
    case 0x828031u: goto store_table_pointer;
    case 0x828033u: goto narrow_accumulator;
    case 0x828035u: goto push_program_bank;
    case 0x828036u: goto pull_accumulator_2;
    case 0x828037u: goto store_table_bank;
    case 0x828039u: goto widen_accumulator_2;
    case 0x82803bu: goto load_table_pointer;
    case 0x82803du: goto store_jump_target;
    case 0x82803fu: goto narrow_accumulator_2;
    case 0x828041u: goto jump_table_target;
    default: return 0;
    }
widen_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828028u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x802au);
}
mask_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82802au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x802du);
}
double_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82802du, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x802eu);
}
increment_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82802eu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a + 1u) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x802fu);
}
copy_accumulator_to_y: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82802fu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8030u);
}
pull_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x828030u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa9u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x8031u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_table_pointer: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5du);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828031u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8033u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828033u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8035u);
}
push_program_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x828035u, 1u, span.instructions != 0u);
    const uint16_t value = registers->k;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8036u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
pull_accumulator_2: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x828036u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    Lufia2DecompApplyOperand(registers, 0xa9u, value, true);
    NativeSpanStep(&span, registers, cpu, 4u, 0x8037u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_table_bank: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828037u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8039u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_accumulator_2: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828039u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x803bu);
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
    Lufia2SpanFetch(registers, 0x82803bu, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    pointer |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    const uint32_t source = (pointer + registers->y) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x803du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_jump_target: {
    const uint32_t address = (uint16_t)(registers->dp + 0x60u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82803du, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x803fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_accumulator_2: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82803fu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8041u);
    goto finished;
}
jump_table_target: {
    const uint32_t pointer = 0x0060u;
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828041u, 3u, span.instructions != 0u);
    const uint16_t target = Lufia2DecompReadData(registers, pointer, pointer + 1u, true);
    NativeSpanStep(&span, registers, cpu, 5u, target);
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
static int Lufia2DecompRunMenuQueueVideoSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                             uint64_t dynamic_epoch, uint64_t write_epoch,
                                             unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x828044u && pc < 0x828065u)
        end_pc = 0x828065u;
    else if (pc >= 0x828065u && pc < 0x828068u)
        end_pc = 0x828068u;
    else if (pc >= 0x828068u && pc < 0x828069u)
        end_pc = 0x828069u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x828044u: byte_accumulator = true; break;
    case 0x828046u: byte_accumulator = true; break;
    case 0x828049u: byte_accumulator = true; break;
    case 0x82804bu: byte_accumulator = true; break;
    case 0x82804eu: byte_accumulator = true; break;
    case 0x828050u: byte_accumulator = true; break;
    case 0x828053u: byte_accumulator = true; break;
    case 0x828055u: byte_accumulator = true; break;
    case 0x828058u: byte_accumulator = true; break;
    case 0x82805au: byte_accumulator = true; break;
    case 0x82805du: byte_accumulator = true; break;
    case 0x82805fu: byte_accumulator = true; break;
    case 0x828061u: byte_accumulator = true; break;
    case 0x828063u: byte_accumulator = true; break;
    case 0x828065u: byte_accumulator = true; break;
    case 0x828068u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x828044u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 37u ||
        memcmp(cart->rom + first,
               "\xa9\x01\x8d\x00\x43\xa6\x5d\x8e\x02\x43\xa5\x5f\x8d\x04\x43\xa6\x58\x8e\x05\x43\xa9\x18\x8d"
               "\x01\x43\xa6\x60\x86\x79\xa9\x41\x85\x75\x20\xc2\x93\x6b",
               37u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x828044u: goto set_accumulator;
    case 0x828046u: goto store_dma_mode;
    case 0x828049u: goto load_x_table_pointer;
    case 0x82804bu: goto store_x_dma_source;
    case 0x82804eu: goto load_table_bank;
    case 0x828050u: goto store_dma_bank;
    case 0x828053u: goto load_x_first_word;
    case 0x828055u: goto store_x_dma_size;
    case 0x828058u: goto set_accumulator_2;
    case 0x82805au: goto store_dma_port;
    case 0x82805du: goto load_x_jump_target;
    case 0x82805fu: goto store_x_transfer_address;
    case 0x828061u: goto set_accumulator_3;
    case 0x828063u: goto store_transfer_start;
    case 0x828065u: goto call_8293c2;
    case 0x828068u: goto return_caller;
    default: return 0;
    }
set_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828044u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8046u);
}
store_dma_mode: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4300u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const bool mapped_port = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 4u) || (!mapped_port && (!Lufia2DecompIsWramByte(address))))
        goto finished;
    Lufia2SpanFetch(registers, 0x828046u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 4u, 0x8049u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_x_table_pointer: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5du);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828049u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x804bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_x_dma_source: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4302u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const bool mapped_port = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 5u) ||
        (!mapped_port && (!Lufia2DecompIsWramByte(address) || !Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x82804bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, 5u, 0x804eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_table_bank: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82804eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8050u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_dma_bank: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4304u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const bool mapped_port = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 4u) || (!mapped_port && (!Lufia2DecompIsWramByte(address))))
        goto finished;
    Lufia2SpanFetch(registers, 0x828050u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 4u, 0x8053u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_x_first_word: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828053u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8055u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_x_dma_size: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4305u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const bool mapped_port = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 5u) ||
        (!mapped_port && (!Lufia2DecompIsWramByte(address) || !Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x828055u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, 5u, 0x8058u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828058u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0018u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x805au);
}
store_dma_port: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4301u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const bool mapped_port = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 4u) || (!mapped_port && (!Lufia2DecompIsWramByte(address))))
        goto finished;
    Lufia2SpanFetch(registers, 0x82805au, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 4u, 0x805du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_x_jump_target: {
    const uint32_t address = (uint16_t)(registers->dp + 0x60u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82805du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x805fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_x_transfer_address: {
    const uint32_t address = (uint16_t)(registers->dp + 0x79u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82805fu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8061u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator_3: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828061u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0041u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8063u);
}
store_transfer_start: {
    const uint32_t address = (uint16_t)(registers->dp + 0x75u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828063u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8065u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
call_8293c2: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828065u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828065u, 0x20u, 0x8293c2u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_caller: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828068u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x828068u, 0x6bu, true, registers->mf, true);
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

static int Lufia2DecompRunMenuInlineVideoSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                              uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                              unsigned step_budget, long *step_count) {
    if (pc >= 0x828028u && pc <= 0x828043u)
        return Lufia2DecompRunMenuInlineTableSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                  step_budget, step_count);
    if (pc >= 0x828044u && pc <= 0x828068u)
        return Lufia2DecompRunMenuQueueVideoSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                 step_budget, step_count);
    return 0;
}
