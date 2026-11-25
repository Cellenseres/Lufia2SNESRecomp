#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunFieldMapEventsSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                             uint64_t dynamic_epoch, uint64_t write_epoch,
                                             unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x80e844u && pc < 0x80e897u)
        end_pc = 0x80e897u;
    else if (pc >= 0x80e897u && pc < 0x80e898u)
        end_pc = 0x80e898u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x80e844u: width_neutral = true; break;
    case 0x80e87au: width_neutral = true; break;
    case 0x80e87cu: byte_accumulator = true; break;
    case 0x80e880u: byte_accumulator = true; break;
    case 0x80e88au: width_neutral = true; break;
    case 0x80e88cu: byte_accumulator = true; break;
    case 0x80e890u: byte_accumulator = true; break;
    case 0x80e891u: byte_accumulator = true; break;
    case 0x80e893u: byte_accumulator = true; break;
    case 0x80e897u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x80e844u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 84u ||
        memcmp(cart->rom + first,
               "\xc2\x20\xa2\x7c\x00\x7b\x9f\x66\xd4\x7f\x9f\x68\xd4\x7f\xca\xca\xca\xca\x10\xf2\xad\xac\x05"
               "\x0a\xaa\xbf\xb5\xb8\x91\x8f\xa0\xd1\x7f\xad\xac\x05\x0a\x6d\xac\x05\xaa\xbf\x00\x80\x8f\xc9"
               "\xff\xff\xd0\x0c\x8f\x94\xd1\x7f\xe2\x20\x8f\x96\xd1\x7f\x80\x15\x18\x69\x00\x80\x8f\x94\xd1"
               "\x7f\xe2\x20\xbf\x02\x80\x8f\x18\x69\x8f\x8f\x96\xd1\x7f\x6b",
               84u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x80e844u: goto widen_event_offsets;
    case 0x80e846u: goto last_event_record;
    case 0x80e849u: goto clear_record_value;
    case 0x80e84au: goto store_record_low;
    case 0x80e84eu: goto store_record_high;
    case 0x80e852u: goto previous_record_byte;
    case 0x80e853u: goto previous_record_word;
    case 0x80e854u: goto previous_record_third;
    case 0x80e855u: goto previous_record_fourth;
    case 0x80e856u: goto repeat_event_records;
    case 0x80e858u: goto read_map_id;
    case 0x80e85bu: goto map_parameter_index;
    case 0x80e85cu: goto select_map_parameter;
    case 0x80e85du: goto read_map_parameter;
    case 0x80e861u: goto store_map_parameter;
    case 0x80e865u: goto read_script_map_id;
    case 0x80e868u: goto script_double_index;
    case 0x80e869u: goto script_triple_index;
    case 0x80e86cu: goto select_script_index;
    case 0x80e86du: goto read_script_offset;
    case 0x80e871u: goto test_missing_script;
    case 0x80e874u: goto select_script_offset;
    case 0x80e876u: goto store_missing_offset;
    case 0x80e87au: goto narrow_missing_bank;
    case 0x80e87cu: goto store_missing_bank;
    case 0x80e880u: goto finish_missing_script;
    case 0x80e882u: goto clear_script_carry;
    case 0x80e883u: goto script_window;
    case 0x80e886u: goto store_script_base;
    case 0x80e88au: goto narrow_script_bank;
    case 0x80e88cu: goto read_script_bank;
    case 0x80e890u: goto clear_bank_carry;
    case 0x80e891u: goto script_bank_origin;
    case 0x80e893u: goto store_script_bank;
    case 0x80e897u: goto return_map_events;
    default: return 0;
    }
widen_event_offsets: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e844u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe846u);
}
last_event_record: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e846u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x007cu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe849u);
}
clear_record_value: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e849u, 1u, span.instructions != 0u);
    registers->a = registers->dp;
    registers->z = registers->a == 0u;
    registers->n = (registers->a & 0x8000u) != 0u;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe84au);
}
store_record_low: {
    const uint32_t address = (0x7fd466u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e84au, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe84eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_record_high: {
    const uint32_t address = (0x7fd468u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e84eu, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe852u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
previous_record_byte: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e852u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe853u);
}
previous_record_word: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e853u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe854u);
}
previous_record_third: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e854u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe855u);
}
previous_record_fourth: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e855u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe856u);
}
repeat_event_records: {
    const bool taken = !registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e856u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xe84au : 0xe858u);
    if (taken)
        goto finished;
}
read_map_id: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05acu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e858u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe85bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
map_parameter_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e85bu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe85cu);
}
select_map_parameter: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e85cu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe85du);
}
read_map_parameter: {
    const uint32_t address = (0x91b8b5u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e85du, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe861u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_map_parameter: {
    const uint32_t address = 0x7fd1a0u;
    const uint32_t high = address + 1u;
    if (!NativeSpanCanStep(&span, cpu, 6u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e861u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 6u, 0xe865u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_script_map_id: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05acu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e865u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe868u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
script_double_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e868u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe869u);
}
script_triple_index: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05acu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e869u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe86cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_script_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e86cu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe86du);
}
read_script_offset: {
    const uint32_t address = (0x8f8000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e86du, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe871u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_missing_script: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e871u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0xffffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe874u);
}
select_script_offset: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e874u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xe882u : 0xe876u);
    if (taken)
        goto clear_script_carry;
}
store_missing_offset: {
    const uint32_t address = 0x7fd194u;
    const uint32_t high = address + 1u;
    if (!NativeSpanCanStep(&span, cpu, 6u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e876u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 6u, 0xe87au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_missing_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e87au, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe87cu);
}
store_missing_bank: {
    const uint32_t address = 0x7fd196u;
    const uint32_t high = address + 1u;
    if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e87cu, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xe880u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
finish_missing_script: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e880u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe897u);
    goto finished;
}
clear_script_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e882u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe883u);
}
script_window: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e883u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x8000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe886u);
}
store_script_base: {
    const uint32_t address = 0x7fd194u;
    const uint32_t high = address + 1u;
    if (!NativeSpanCanStep(&span, cpu, 6u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e886u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 6u, 0xe88au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_script_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e88au, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe88cu);
}
read_script_bank: {
    const uint32_t address = (0x8f8002u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e88cu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe890u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_bank_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e890u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe891u);
}
script_bank_origin: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e891u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x008fu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe893u);
}
store_script_bank: {
    const uint32_t address = 0x7fd196u;
    const uint32_t high = address + 1u;
    if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80e893u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xe897u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_map_events: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x80e897u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x80e897u, 0x6bu, true, registers->mf, true);
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
static int Lufia2DecompRunFieldCellIndexSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                             uint64_t dynamic_epoch, uint64_t write_epoch,
                                             unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x80f6aau && pc < 0x80f6c5u)
        end_pc = 0x80f6c5u;
    else if (pc >= 0x80f6c5u && pc < 0x80f6c6u)
        end_pc = 0x80f6c6u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x80f6aau: width_neutral = true; break;
    case 0x80f6acu: byte_accumulator = true; break;
    case 0x80f6aeu: byte_accumulator = true; break;
    case 0x80f6b2u: byte_accumulator = true; break;
    case 0x80f6b4u: byte_accumulator = true; break;
    case 0x80f6b8u: width_neutral = true; break;
    case 0x80f6c5u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x80f6aau;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 28u || memcmp(cart->rom + first,
                                              "\xe2\x20\xa5\x89\x8f\x02\x42\x00\xa5\x83\x8f\x03\x42\x00\xc2"
                                              "\x20\xa5\x87\x18\x6f\x16\x42\x00\x0a\x65\x8d\xaa\x60",
                                              28u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x80f6aau: goto narrow_cell_coordinates;
    case 0x80f6acu: goto read_cell_y;
    case 0x80f6aeu: goto multiply_cell_y;
    case 0x80f6b2u: goto read_layer_width;
    case 0x80f6b4u: goto multiply_layer_width;
    case 0x80f6b8u: goto widen_cell_index;
    case 0x80f6bau: goto read_cell_x;
    case 0x80f6bcu: goto clear_cell_carry;
    case 0x80f6bdu: goto add_row_product;
    case 0x80f6c1u: goto cell_word_offset;
    case 0x80f6c2u: goto add_cell_base;
    case 0x80f6c4u: goto select_cell_index;
    case 0x80f6c5u: goto return_cell_index;
    default: return 0;
    }
narrow_cell_coordinates: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f6aau, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf6acu);
}
read_cell_y: {
    const uint32_t address = (uint16_t)(registers->dp + 0x89u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f6acu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xf6aeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
multiply_cell_y: {
    const uint32_t address = 0x004202u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f6aeu, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xf6b2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_layer_width: {
    const uint32_t address = (uint16_t)(registers->dp + 0x83u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f6b2u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xf6b4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
multiply_layer_width: {
    const uint32_t address = 0x004203u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f6b4u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xf6b8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_cell_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f6b8u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf6bau);
}
read_cell_x: {
    const uint32_t address = (uint16_t)(registers->dp + 0x87u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f6bau, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xf6bcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_cell_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f6bcu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xf6bdu);
}
add_row_product: {
    const uint32_t address = 0x004216u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 6u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f6bdu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, 6u, 0xf6c1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
cell_word_offset: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f6c1u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf6c2u);
}
add_cell_base: {
    const uint32_t address = (uint16_t)(registers->dp + 0x8du);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f6c2u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xf6c4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_cell_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80f6c4u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf6c5u);
    goto finished;
}
return_cell_index: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x80f6c5u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x80f6c5u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunFieldPaletteCyclesSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                 uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                 unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x83aeedu && pc < 0x83af53u)
        end_pc = 0x83af53u;
    else if (pc >= 0x83af53u && pc < 0x83af54u)
        end_pc = 0x83af54u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x83aeedu: byte_accumulator = true; break;
    case 0x83aef1u: byte_accumulator = true; break;
    case 0x83aef3u: byte_accumulator = true; break;
    case 0x83aef5u: byte_accumulator = true; break;
    case 0x83aef9u: byte_accumulator = true; break;
    case 0x83aefbu: byte_accumulator = true; break;
    case 0x83aefdu: byte_accumulator = true; break;
    case 0x83af00u: byte_accumulator = true; break;
    case 0x83af03u: byte_accumulator = true; break;
    case 0x83af05u: width_neutral = true; break;
    case 0x83af12u: width_neutral = true; break;
    case 0x83af14u: byte_accumulator = true; break;
    case 0x83af18u: byte_accumulator = true; break;
    case 0x83af1bu: width_neutral = true; break;
    case 0x83af46u: width_neutral = true; break;
    case 0x83af48u: byte_accumulator = true; break;
    case 0x83af49u: byte_accumulator = true; break;
    case 0x83af4bu: byte_accumulator = true; break;
    case 0x83af4du: byte_accumulator = true; break;
    case 0x83af4eu: byte_accumulator = true; break;
    case 0x83af4fu: byte_accumulator = true; break;
    case 0x83af51u: byte_accumulator = true; break;
    case 0x83af53u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x83aeedu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 103u ||
        memcmp(cart->rom + first,
               "\xaf\xa9\x09\x00\x89\x06\xd0\x5e\xaf\xf7\xd0\x7f\xf0\x58\x85\x58\xa2\x00\x00\xde\x01\xed\xd0"
               "\x48\xc2\x20\xbd\x00\xec\x1a\x1a\x1a\x9d\x00\xec\x9b\xaa\xe2\x20\xbf\x00\x00\xa1\x99\x01\xed"
               "\xc2\x20\xd0\x13\x98\x18\x6d\xf5\xd0\xaa\xbf\x03\x00\xa1\x38\x6d\xf5\xd0\x99\x00\xec\x80\xdf"
               "\xbf\x01\x00\xa1\x85\x54\xb9\x00\xed\x29\xff\x00\x0a\xaa\xa5\x54\x9f\x20\x03\x00\xe2\x20\xbb"
               "\xa9\x01\x04\x73\xe8\xe8\xc6\x58\xd0\xad\x60",
               103u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83aeedu: goto read_scene_flags;
    case 0x83aef1u: goto test_paused_palette;
    case 0x83aef3u: goto skip_paused_palette;
    case 0x83aef5u: goto read_palette_count;
    case 0x83aef9u: goto skip_empty_palettes;
    case 0x83aefbu: goto store_palette_count;
    case 0x83aefdu: goto first_palette_record;
    case 0x83af00u: goto count_palette_timer;
    case 0x83af03u: goto skip_waiting_palette;
    case 0x83af05u: goto widen_palette_cursor;
    case 0x83af07u: goto read_palette_cursor;
    case 0x83af0au: goto next_cursor_first;
    case 0x83af0bu: goto next_cursor_second;
    case 0x83af0cu: goto next_cursor_third;
    case 0x83af0du: goto store_palette_cursor;
    case 0x83af10u: goto save_palette_record;
    case 0x83af11u: goto select_palette_cursor;
    case 0x83af12u: goto narrow_palette_timer;
    case 0x83af14u: goto read_palette_timer;
    case 0x83af18u: goto store_palette_timer;
    case 0x83af1bu: goto widen_palette_data;
    case 0x83af1du: goto select_palette_data;
    case 0x83af1fu: goto palette_record_position;
    case 0x83af20u: goto clear_redirect_carry;
    case 0x83af21u: goto palette_redirect_position;
    case 0x83af24u: goto select_palette_redirect;
    case 0x83af25u: goto read_palette_redirect;
    case 0x83af29u: goto set_redirect_carry;
    case 0x83af2au: goto palette_redirect_base;
    case 0x83af2du: goto store_palette_redirect;
    case 0x83af30u: goto repeat_palette_redirect;
    case 0x83af32u: goto read_palette_color;
    case 0x83af36u: goto store_palette_color;
    case 0x83af38u: goto read_palette_target;
    case 0x83af3bu: goto mask_palette_target;
    case 0x83af3eu: goto palette_color_offset;
    case 0x83af3fu: goto select_palette_color;
    case 0x83af40u: goto restore_palette_color;
    case 0x83af42u: goto store_palette_buffer;
    case 0x83af46u: goto narrow_palette_flags;
    case 0x83af48u: goto restore_palette_record;
    case 0x83af49u: goto palette_upload_flag;
    case 0x83af4bu: goto mark_palette_upload;
    case 0x83af4du: goto next_palette_byte;
    case 0x83af4eu: goto next_palette_word;
    case 0x83af4fu: goto count_palettes;
    case 0x83af51u: goto repeat_palettes;
    case 0x83af53u: goto return_palette_cycles;
    default: return 0;
    }
read_scene_flags: {
    const uint32_t address = 0x0009a9u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aeedu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, 5u, 0xaef1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_paused_palette: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aef1u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0006u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaef3u);
}
skip_paused_palette: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aef3u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xaf53u : 0xaef5u);
    if (taken)
        goto finished;
}
read_palette_count: {
    const uint32_t address = 0x7fd0f7u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aef5u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, 5u, 0xaef9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_empty_palettes: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aef9u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xaf53u : 0xaefbu);
    if (taken)
        goto finished;
}
store_palette_count: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aefbu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaefdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_palette_record: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aefdu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xaf00u);
}
count_palette_timer: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xed01u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 8u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af00u, 3u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xdeu, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaf03u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_waiting_palette: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af03u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xaf4du : 0xaf05u);
    if (taken)
        goto next_palette_byte;
}
widen_palette_cursor: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af05u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xaf07u);
}
read_palette_cursor: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xec00u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0xec00u >> 8) != ((0xec00u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af07u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaf0au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_cursor_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af0au, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a + 1u) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaf0bu);
}
next_cursor_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af0bu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a + 1u) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaf0cu);
}
next_cursor_third: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af0cu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a + 1u) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaf0du);
}
store_palette_cursor: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xec00u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af0du, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaf10u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_palette_record: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af10u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->x, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaf11u);
}
select_palette_cursor: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af11u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaf12u);
}
narrow_palette_timer: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af12u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xaf14u);
}
read_palette_timer: {
    const uint32_t address = (0xa10000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af14u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaf18u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_palette_timer: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xed01u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af18u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaf1bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_palette_data: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af1bu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xaf1du);
}
select_palette_data: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af1du, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xaf32u : 0xaf1fu);
    if (taken)
        goto read_palette_color;
}
palette_record_position: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af1fu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaf20u);
}
clear_redirect_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af20u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xaf21u);
}
palette_redirect_position: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xd0f5u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af21u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaf24u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_palette_redirect: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af24u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaf25u);
}
read_palette_redirect: {
    const uint32_t address = (0xa10003u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af25u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaf29u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_redirect_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af29u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xaf2au);
}
palette_redirect_base: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xd0f5u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af2au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaf2du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_palette_redirect: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xec00u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af2du, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaf30u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_palette_redirect: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af30u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xaf11u);
    goto finished;
}
read_palette_color: {
    const uint32_t address = (0xa10001u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af32u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaf36u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_palette_color: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af36u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaf38u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_palette_target: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xed00u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0xed00u >> 8) != ((0xed00u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af38u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaf3bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_palette_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af3bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xaf3eu);
}
palette_color_offset: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af3eu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaf3fu);
}
select_palette_color: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af3fu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaf40u);
}
restore_palette_color: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af40u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaf42u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_palette_buffer: {
    const uint32_t address = (0x000320u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af42u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaf46u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_palette_flags: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af46u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xaf48u);
}
restore_palette_record: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af48u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaf49u);
}
palette_upload_flag: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af49u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaf4bu);
}
mark_palette_upload: {
    const uint32_t address = (uint16_t)(registers->dp + 0x73u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af4bu, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0x04u, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaf4du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_palette_byte: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af4du, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaf4eu);
}
next_palette_word: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af4eu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaf4fu);
}
count_palettes: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af4fu, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaf51u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_palettes: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83af51u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xaf00u : 0xaf53u);
    if (taken)
        goto finished;
    goto finished;
}
return_palette_cycles: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83af53u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83af53u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunFieldEventSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
    uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget, long *step_count) {
    if (pc >= 0x80e844u && pc <= 0x80e897u)
        return Lufia2DecompRunFieldMapEventsSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x80f6aau && pc <= 0x80f6c5u)
        return Lufia2DecompRunFieldCellIndexSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x83aeedu && pc <= 0x83af53u)
        return Lufia2DecompRunFieldPaletteCyclesSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    return 0;
}
