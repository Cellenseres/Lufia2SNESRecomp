#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunBattleGlyphSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                          uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget,
                                          long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x81e835u && pc < 0x81e847u)
        end_pc = 0x81e847u;
    else if (pc >= 0x81e847u && pc < 0x81e848u)
        end_pc = 0x81e848u;
    else if (pc >= 0x81e848u && pc < 0x81e86eu)
        end_pc = 0x81e86eu;
    else if (pc >= 0x81e86eu && pc < 0x81e86fu)
        end_pc = 0x81e86fu;
    else if (pc >= 0x81e86fu && pc < 0x81e871u)
        end_pc = 0x81e871u;
    else if (pc >= 0x81e871u && pc < 0x81e872u)
        end_pc = 0x81e872u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x81e835u: byte_accumulator = true; break;
    case 0x81e837u: byte_accumulator = true; break;
    case 0x81e839u: byte_accumulator = true; break;
    case 0x81e83bu: byte_accumulator = true; break;
    case 0x81e83du: byte_accumulator = true; break;
    case 0x81e83fu: byte_accumulator = true; break;
    case 0x81e841u: byte_accumulator = true; break;
    case 0x81e842u: byte_accumulator = true; break;
    case 0x81e844u: byte_accumulator = true; break;
    case 0x81e845u: byte_accumulator = true; break;
    case 0x81e847u: byte_accumulator = true; break;
    case 0x81e848u: byte_accumulator = true; break;
    case 0x81e849u: byte_accumulator = true; break;
    case 0x81e84au: byte_accumulator = true; break;
    case 0x81e84cu: width_neutral = true; break;
    case 0x81e852u: width_neutral = true; break;
    case 0x81e854u: byte_accumulator = true; break;
    case 0x81e855u: byte_accumulator = true; break;
    case 0x81e859u: byte_accumulator = true; break;
    case 0x81e85au: byte_accumulator = true; break;
    case 0x81e85bu: byte_accumulator = true; break;
    case 0x81e85cu: byte_accumulator = true; break;
    case 0x81e85eu: byte_accumulator = true; break;
    case 0x81e860u: byte_accumulator = true; break;
    case 0x81e862u: byte_accumulator = true; break;
    case 0x81e864u: byte_accumulator = true; break;
    case 0x81e866u: byte_accumulator = true; break;
    case 0x81e868u: byte_accumulator = true; break;
    case 0x81e86au: byte_accumulator = true; break;
    case 0x81e86cu: byte_accumulator = true; break;
    case 0x81e86eu: byte_accumulator = true; break;
    case 0x81e86fu: byte_accumulator = true; break;
    case 0x81e871u: byte_accumulator = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || !step_budget ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x81e835u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 61u ||
        memcmp(cart->rom + first,
               "\xc9\xcd\xb0\x0f\xc9\x10\xb0\x07\xc9\x00\xf0\x03\x18\x69\xdf\xeb\xa9\x00\x60\xda\x38\xe9\xcd"
               "\xc2\x20\x29\xff\x00\xaa\xe2\x20\x48\xbf\xc9\xb2\x97\xeb\x68\xfa\xc9\x14\x90\x0f\xc9\x19\x90"
               "\x08\xc9\x2d\x90\x07\xc9\x32\xf0\x03\xa9\xce\x60\xa9\xcd\x60",
               61u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x81e835u: goto test_large_character;
    case 0x81e837u: goto select_large_character;
    case 0x81e839u: goto test_control_character;
    case 0x81e83bu: goto select_plain_character;
    case 0x81e83du: goto test_blank_character;
    case 0x81e83fu: goto skip_blank_character;
    case 0x81e841u: goto clear_character_carry;
    case 0x81e842u: goto small_character_offset;
    case 0x81e844u: goto bottom_tile_to_high_byte;
    case 0x81e845u: goto clear_small_top_tile;
    case 0x81e847u: goto return_small_character;
    case 0x81e848u: goto save_character_index;
    case 0x81e849u: goto set_character_carry;
    case 0x81e84au: goto large_character_index;
    case 0x81e84cu: goto widen_character_index;
    case 0x81e84eu: goto mask_character_index;
    case 0x81e851u: goto select_character_index;
    case 0x81e852u: goto narrow_character_tile;
    case 0x81e854u: goto save_character_tile;
    case 0x81e855u: goto read_bottom_tile;
    case 0x81e859u: goto bottom_tile_to_accumulator;
    case 0x81e85au: goto restore_character_tile;
    case 0x81e85bu: goto restore_character_index;
    case 0x81e85cu: goto test_first_accent;
    case 0x81e85eu: goto select_first_plain_tile;
    case 0x81e860u: goto test_first_accent_end;
    case 0x81e862u: goto select_first_accent;
    case 0x81e864u: goto test_second_accent;
    case 0x81e866u: goto select_second_plain_tile;
    case 0x81e868u: goto test_last_character;
    case 0x81e86au: goto select_last_plain_tile;
    case 0x81e86cu: goto accent_top_tile;
    case 0x81e86eu: goto return_accent_character;
    case 0x81e86fu: goto plain_top_tile;
    case 0x81e871u: goto return_plain_character;
    default: return 0;
    }
test_large_character: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e835u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x00cdu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe837u);
}
select_large_character: {
    const bool taken = registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e837u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xe848u : 0xe839u);
    if (taken)
        goto finished;
}
test_control_character: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e839u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe83bu);
}
select_plain_character: {
    const bool taken = registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e83bu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xe844u : 0xe83du);
    if (taken)
        goto bottom_tile_to_high_byte;
}
test_blank_character: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e83du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe83fu);
}
skip_blank_character: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e83fu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xe844u : 0xe841u);
    if (taken)
        goto bottom_tile_to_high_byte;
}
clear_character_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e841u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe842u);
}
small_character_offset: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e842u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x00dfu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe844u);
}
bottom_tile_to_high_byte: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e844u, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->a = (uint16_t)((value << 8) | (value >> 8));
    registers->z = (registers->a & 0xffu) == 0u;
    registers->n = (registers->a & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 3u, 0xe845u);
}
clear_small_top_tile: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e845u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe847u);
    goto finished;
}
return_small_character: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x81e847u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x81e847u, 0x60u, false, true, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
save_character_index: {
    const unsigned cycles = registers->xf ? 3u : 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < (registers->xf ? 0x1f01u : 0x1f02u) ||
        registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e848u, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    if (!registers->xf)
        registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe849u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_character_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e849u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe84au);
}
large_character_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e84au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x00cdu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe84cu);
}
widen_character_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e84cu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe84eu);
}
mask_character_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e84eu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe851u);
}
select_character_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e851u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->xf ? (uint8_t)registers->a : registers->a,
                             registers->xf);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe852u);
}
narrow_character_tile: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e852u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe854u);
}
save_character_tile: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e854u, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe855u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_bottom_tile: {
    const uint32_t address = (0x97b2c9u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e855u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe859u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
bottom_tile_to_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e859u, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->a = (uint16_t)((value << 8) | (value >> 8));
    registers->z = (registers->a & 0xffu) == 0u;
    registers->n = (registers->a & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 3u, 0xe85au);
}
restore_character_tile: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e85au, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, ++registers->sp), true);
    NativeSpanStep(&span, registers, cpu, 4u, 0xe85bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_character_index: {
    const unsigned cycles = registers->xf ? 4u : 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u ||
        registers->sp > (registers->xf ? 0x1ffeu : 0x1ffdu))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e85bu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (!registers->xf)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, registers->xf);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe85cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_first_accent: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e85cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0014u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe85eu);
}
select_first_plain_tile: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e85eu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xe86fu : 0xe860u);
    if (taken)
        goto finished;
}
test_first_accent_end: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e860u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0019u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe862u);
}
select_first_accent: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e862u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xe86cu : 0xe864u);
    if (taken)
        goto accent_top_tile;
}
test_second_accent: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e864u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x002du, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe866u);
}
select_second_plain_tile: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e866u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xe86fu : 0xe868u);
    if (taken)
        goto finished;
}
test_last_character: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e868u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0032u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe86au);
}
select_last_plain_tile: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e86au, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xe86fu : 0xe86cu);
    if (taken)
        goto finished;
}
accent_top_tile: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e86cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00ceu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe86eu);
    goto finished;
}
return_accent_character: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x81e86eu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x81e86eu, 0x60u, false, true, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
plain_top_tile: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e86fu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00cdu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe871u);
    goto finished;
}
return_plain_character: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x81e871u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x81e871u, 0x60u, false, true, true);
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
