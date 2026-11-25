#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunBattleTargetMoveSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                               uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                               unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x819bbeu && pc < 0x819c75u)
        end_pc = 0x819c75u;
    else if (pc >= 0x819c75u && pc < 0x819c76u)
        end_pc = 0x819c76u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x819bbeu: byte_accumulator = true; break;
    case 0x819bc0u: byte_accumulator = true; break;
    case 0x819bc1u: byte_accumulator = true; break;
    case 0x819bc2u: byte_accumulator = true; break;
    case 0x819bc3u: width_neutral = true; break;
    case 0x819bedu: width_neutral = true; break;
    case 0x819befu: byte_accumulator = true; break;
    case 0x819bf0u: byte_accumulator = true; break;
    case 0x819bf3u: byte_accumulator = true; break;
    case 0x819bf5u: byte_accumulator = true; break;
    case 0x819bf7u: byte_accumulator = true; break;
    case 0x819bf9u: byte_accumulator = true; break;
    case 0x819bfdu: byte_accumulator = true; break;
    case 0x819bfeu: byte_accumulator = true; break;
    case 0x819c00u: byte_accumulator = true; break;
    case 0x819c04u: byte_accumulator = true; break;
    case 0x819c06u: byte_accumulator = true; break;
    case 0x819c07u: byte_accumulator = true; break;
    case 0x819c08u: byte_accumulator = true; break;
    case 0x819c09u: byte_accumulator = true; break;
    case 0x819c0cu: byte_accumulator = true; break;
    case 0x819c0du: byte_accumulator = true; break;
    case 0x819c0eu: byte_accumulator = true; break;
    case 0x819c0fu: byte_accumulator = true; break;
    case 0x819c13u: byte_accumulator = true; break;
    case 0x819c14u: byte_accumulator = true; break;
    case 0x819c17u: byte_accumulator = true; break;
    case 0x819c18u: byte_accumulator = true; break;
    case 0x819c1bu: byte_accumulator = true; break;
    case 0x819c1eu: byte_accumulator = true; break;
    case 0x819c21u: byte_accumulator = true; break;
    case 0x819c22u: byte_accumulator = true; break;
    case 0x819c24u: byte_accumulator = true; break;
    case 0x819c27u: byte_accumulator = true; break;
    case 0x819c2au: byte_accumulator = true; break;
    case 0x819c2du: byte_accumulator = true; break;
    case 0x819c2eu: byte_accumulator = true; break;
    case 0x819c31u: byte_accumulator = true; break;
    case 0x819c34u: byte_accumulator = true; break;
    case 0x819c37u: byte_accumulator = true; break;
    case 0x819c38u: byte_accumulator = true; break;
    case 0x819c3au: byte_accumulator = true; break;
    case 0x819c3du: byte_accumulator = true; break;
    case 0x819c40u: byte_accumulator = true; break;
    case 0x819c42u: byte_accumulator = true; break;
    case 0x819c44u: byte_accumulator = true; break;
    case 0x819c48u: byte_accumulator = true; break;
    case 0x819c49u: byte_accumulator = true; break;
    case 0x819c4bu: byte_accumulator = true; break;
    case 0x819c4fu: byte_accumulator = true; break;
    case 0x819c50u: byte_accumulator = true; break;
    case 0x819c51u: byte_accumulator = true; break;
    case 0x819c53u: byte_accumulator = true; break;
    case 0x819c54u: byte_accumulator = true; break;
    case 0x819c55u: width_neutral = true; break;
    case 0x819c70u: width_neutral = true; break;
    case 0x819c72u: byte_accumulator = true; break;
    case 0x819c73u: byte_accumulator = true; break;
    case 0x819c74u: byte_accumulator = true; break;
    case 0x819c75u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x819bbeu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 184u ||
        memcmp(cart->rom + first,
               "\xa9\x7e\x48\xab\x5a\xc2\x20\xa7\xc3\xe6\xc3\x89\x80\x00\xf0\x05\x09\x00\xff\x80\x03\x29\xff"
               "\x00\x8d\xef\x15\xa7\xc3\xe6\xc3\x89\x80\x00\xf0\x05\x09\x00\xff\x80\x03\x29\xff\x00\x8d\xf1"
               "\x15\xe2\x20\x7b\xb9\x25\x00\x30\x4d\xc9\x04\xf0\x7b\x8f\x02\x42\x00\xeb\xa9\x0d\x8f\x03\x42"
               "\x00\xa9\x00\xeb\x0a\x0a\x79\x25\x00\x0a\x0a\xa8\xaf\x16\x42\x00\xaa\xbd\xa0\x13\x18\x6d\xef"
               "\x15\x99\xc0\x48\x99\xca\x48\x18\x69\x10\x99\xc5\x48\x99\xcf\x48\xbd\xa0\x13\x18\x6d\xf1\x15"
               "\x99\xc1\x48\x99\xc6\x48\x18\x69\x10\x99\xcb\x48\x99\xd0\x48\x80\x32\x29\x7f\x8f\x02\x42\x00"
               "\xeb\xa9\x0f\x8f\x03\x42\x00\xeb\xbb\x85\x11\x5a\x8b\xc2\x20\xaf\x16\x42\x00\xaa\xbd\xe3\x13"
               "\x18\x6d\xef\x15\x9d\xe3\x13\xbd\xe5\x13\x18\x6d\xf1\x15\x9d\xe5\x13\xe2\x20\xab\x7a\x7a\x60",
               184u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x819bbeu: goto data_bank;
    case 0x819bc0u: goto push_data_bank;
    case 0x819bc1u: goto select_bank;
    case 0x819bc2u: goto save_target;
    case 0x819bc3u: goto widen_offsets;
    case 0x819bc5u: goto read_horizontal;
    case 0x819bc7u: goto advance_horizontal;
    case 0x819bc9u: goto test_horizontal_sign;
    case 0x819bccu: goto positive_horizontal;
    case 0x819bceu: goto extend_horizontal;
    case 0x819bd1u: goto skip_horizontal_mask;
    case 0x819bd3u: goto mask_horizontal;
    case 0x819bd6u: goto store_horizontal;
    case 0x819bd9u: goto read_vertical;
    case 0x819bdbu: goto advance_vertical;
    case 0x819bddu: goto test_vertical_sign;
    case 0x819be0u: goto positive_vertical;
    case 0x819be2u: goto extend_vertical;
    case 0x819be5u: goto skip_vertical_mask;
    case 0x819be7u: goto mask_vertical;
    case 0x819beau: goto store_vertical;
    case 0x819bedu: goto narrow_offsets;
    case 0x819befu: goto clear_target_high;
    case 0x819bf0u: goto read_target;
    case 0x819bf3u: goto select_enemy_target;
    case 0x819bf5u: goto test_special_target;
    case 0x819bf7u: goto skip_special_target;
    case 0x819bf9u: goto write_party_index;
    case 0x819bfdu: goto exchange_party_index;
    case 0x819bfeu: goto party_stride;
    case 0x819c00u: goto write_party_stride;
    case 0x819c04u: goto clear_party_high;
    case 0x819c06u: goto exchange_party_high;
    case 0x819c07u: goto scale_party_first;
    case 0x819c08u: goto scale_party_second;
    case 0x819c09u: goto add_party_target;
    case 0x819c0cu: goto scale_party_third;
    case 0x819c0du: goto scale_party_fourth;
    case 0x819c0eu: goto select_party_sprite;
    case 0x819c0fu: goto read_party_product;
    case 0x819c13u: goto select_party_origin;
    case 0x819c14u: goto read_horizontal_origin;
    case 0x819c17u: goto clear_horizontal_carry;
    case 0x819c18u: goto add_horizontal_offset;
    case 0x819c1bu: goto store_horizontal_first;
    case 0x819c1eu: goto store_horizontal_second;
    case 0x819c21u: goto clear_horizontal_size_carry;
    case 0x819c22u: goto horizontal_size;
    case 0x819c24u: goto store_horizontal_third;
    case 0x819c27u: goto store_horizontal_fourth;
    case 0x819c2au: goto read_vertical_origin;
    case 0x819c2du: goto clear_vertical_carry;
    case 0x819c2eu: goto add_vertical_offset;
    case 0x819c31u: goto store_vertical_first;
    case 0x819c34u: goto store_vertical_second;
    case 0x819c37u: goto clear_vertical_size_carry;
    case 0x819c38u: goto vertical_size;
    case 0x819c3au: goto store_vertical_third;
    case 0x819c3du: goto store_vertical_fourth;
    case 0x819c40u: goto skip_enemy_motion;
    case 0x819c42u: goto mask_enemy_index;
    case 0x819c44u: goto write_enemy_index;
    case 0x819c48u: goto exchange_enemy_index;
    case 0x819c49u: goto enemy_stride;
    case 0x819c4bu: goto write_enemy_stride;
    case 0x819c4fu: goto exchange_enemy_high;
    case 0x819c50u: goto preserve_target_index;
    case 0x819c51u: goto store_target_number;
    case 0x819c53u: goto save_enemy_target;
    case 0x819c54u: goto save_enemy_bank;
    case 0x819c55u: goto widen_enemy;
    case 0x819c57u: goto read_enemy_product;
    case 0x819c5bu: goto select_enemy;
    case 0x819c5cu: goto read_enemy_horizontal;
    case 0x819c5fu: goto clear_enemy_horizontal_carry;
    case 0x819c60u: goto add_enemy_horizontal;
    case 0x819c63u: goto store_enemy_horizontal;
    case 0x819c66u: goto read_enemy_vertical;
    case 0x819c69u: goto clear_enemy_vertical_carry;
    case 0x819c6au: goto add_enemy_vertical;
    case 0x819c6du: goto store_enemy_vertical;
    case 0x819c70u: goto narrow_enemy;
    case 0x819c72u: goto restore_enemy_bank;
    case 0x819c73u: goto restore_enemy_target;
    case 0x819c74u: goto restore_target;
    case 0x819c75u: goto return_motion;
    default: return 0;
    }
data_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bbeu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x007eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9bc0u);
}
push_data_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819bc0u, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9bc1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819bc1u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x9bc2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819bc2u, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9bc3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_offsets: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bc3u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9bc5u);
}
read_horizontal: {
    const uint32_t pointer_low = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint32_t pointer_bank = (uint16_t)(pointer_low + 2u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high) ||
        !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t expected = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                              ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (expected) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bc5u, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    pointer |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    const uint32_t source = (pointer) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9bc7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
advance_horizontal: {
    const uint32_t address = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bc7u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9bc9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_horizontal_sign: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bc9u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0080u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9bccu);
}
positive_horizontal: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bccu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9bd3u : 0x9bceu);
    if (taken)
        goto mask_horizontal;
}
extend_horizontal: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bceu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0xff00u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9bd1u);
}
skip_horizontal_mask: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bd1u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9bd6u);
    goto finished;
}
mask_horizontal: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bd3u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9bd6u);
}
store_horizontal: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x15efu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bd6u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9bd9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_vertical: {
    const uint32_t pointer_low = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint32_t pointer_bank = (uint16_t)(pointer_low + 2u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high) ||
        !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t expected = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                              ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (expected) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bd9u, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    pointer |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    const uint32_t source = (pointer) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9bdbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
advance_vertical: {
    const uint32_t address = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bdbu, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9bddu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_vertical_sign: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bddu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0080u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9be0u);
}
positive_vertical: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819be0u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9be7u : 0x9be2u);
    if (taken)
        goto mask_vertical;
}
extend_vertical: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819be2u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0xff00u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9be5u);
}
skip_vertical_mask: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819be5u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9beau);
    goto finished;
}
mask_vertical: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819be7u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9beau);
}
store_vertical: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x15f1u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819beau, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9bedu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_offsets: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bedu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9befu);
}
clear_target_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819befu, 1u, span.instructions != 0u);
    registers->a = registers->dp;
    registers->z = registers->a == 0u;
    registers->n = (registers->a & 0x8000u) != 0u;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9bf0u);
}
read_target: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0025u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0025u >> 8) != ((0x0025u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bf0u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9bf3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_enemy_target: {
    const bool taken = registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bf3u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9c42u : 0x9bf5u);
    if (taken)
        goto mask_enemy_index;
}
test_special_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bf5u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0004u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9bf7u);
}
skip_special_target: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bf7u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9c74u : 0x9bf9u);
    if (taken)
        goto restore_target;
}
write_party_index: {
    const uint32_t address = 0x004202u;
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bf9u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, address + 1u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9bfdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
exchange_party_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bfdu, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9bfeu);
}
party_stride: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bfeu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x000du, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c00u);
}
write_party_stride: {
    const uint32_t address = 0x004203u;
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c00u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, address + 1u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9c04u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_party_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c04u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c06u);
}
exchange_party_high: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c06u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9c07u);
}
scale_party_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c07u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c08u);
}
scale_party_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c08u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c09u);
}
add_party_target: {
    const uint32_t address = ((((uint32_t)registers->db << 16) | 0x0025u) + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0025u >> 8) != ((0x0025u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c09u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c0cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
scale_party_third: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c0cu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c0du);
}
scale_party_fourth: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c0du, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c0eu);
}
select_party_sprite: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c0eu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c0fu);
}
read_party_product: {
    const uint32_t address = 0x004216u;
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c0fu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, address + 1u, false),
                             true);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9c13u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_party_origin: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c13u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c14u);
}
read_horizontal_origin: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13a0u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x13a0u >> 8) != ((0x13a0u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c14u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c17u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_horizontal_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c17u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c18u);
}
add_horizontal_offset: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x15efu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c18u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c1bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_horizontal_first: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x48c0u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c1bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c1eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_horizontal_second: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x48cau + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c1eu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c21u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_horizontal_size_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c21u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c22u);
}
horizontal_size: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c22u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c24u);
}
store_horizontal_third: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x48c5u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c24u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c27u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_horizontal_fourth: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x48cfu + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c27u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c2au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_vertical_origin: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13a0u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x13a0u >> 8) != ((0x13a0u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c2au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c2du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_vertical_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c2du, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c2eu);
}
add_vertical_offset: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x15f1u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c2eu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c31u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_vertical_first: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x48c1u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c31u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c34u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_vertical_second: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x48c6u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c34u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c37u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_vertical_size_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c37u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c38u);
}
vertical_size: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c38u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c3au);
}
store_vertical_third: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x48cbu + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c3au, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c3du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_vertical_fourth: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x48d0u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c3du, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c40u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_enemy_motion: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c40u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9c74u);
    goto finished;
}
mask_enemy_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c42u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x007fu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c44u);
}
write_enemy_index: {
    const uint32_t address = 0x004202u;
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c44u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, address + 1u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9c48u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
exchange_enemy_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c48u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9c49u);
}
enemy_stride: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c49u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x000fu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c4bu);
}
write_enemy_stride: {
    const uint32_t address = 0x004203u;
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c4bu, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, address + 1u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9c4fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
exchange_enemy_high: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c4fu, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9c50u);
}
preserve_target_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c50u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c51u);
}
store_target_number: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c51u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c53u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_enemy_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819c53u, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9c54u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_enemy_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819c54u, 1u, span.instructions != 0u);
    const uint16_t value = registers->db;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9c55u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_enemy: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c55u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9c57u);
}
read_enemy_product: {
    const uint32_t address = 0x004216u;
    if (!NativeSpanCanStep(&span, cpu, 6u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c57u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, address + 1u, true),
                             false);
    NativeSpanStep(&span, registers, cpu, 6u, 0x9c5bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_enemy: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c5bu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c5cu);
}
read_enemy_horizontal: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13e3u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x13e3u >> 8) != ((0x13e3u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c5cu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c5fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_enemy_horizontal_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c5fu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c60u);
}
add_enemy_horizontal: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x15efu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c60u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c63u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_enemy_horizontal: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13e3u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c63u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c66u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_enemy_vertical: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13e5u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x13e5u >> 8) != ((0x13e5u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c66u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c69u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_enemy_vertical_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c69u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c6au);
}
add_enemy_vertical: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x15f1u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c6au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c6du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_enemy_vertical: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13e5u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c6du, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c70u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_enemy: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c70u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9c72u);
}
restore_enemy_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819c72u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x9c73u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_enemy_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819c73u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9c74u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819c74u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9c75u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_motion: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819c75u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x819c75u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunBattleTargetStatusSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                 uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                 unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x819c76u && pc < 0x819ccbu)
        end_pc = 0x819ccbu;
    else if (pc >= 0x819ccbu && pc < 0x819cccu)
        end_pc = 0x819cccu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x819c76u: byte_accumulator = true; break;
    case 0x819c78u: byte_accumulator = true; break;
    case 0x819c79u: byte_accumulator = true; break;
    case 0x819c7au: byte_accumulator = true; break;
    case 0x819c7bu: width_neutral = true; break;
    case 0x819c81u: width_neutral = true; break;
    case 0x819c83u: byte_accumulator = true; break;
    case 0x819c86u: byte_accumulator = true; break;
    case 0x819c87u: byte_accumulator = true; break;
    case 0x819c8au: byte_accumulator = true; break;
    case 0x819c8cu: byte_accumulator = true; break;
    case 0x819c8du: byte_accumulator = true; break;
    case 0x819c8fu: byte_accumulator = true; break;
    case 0x819c90u: byte_accumulator = true; break;
    case 0x819c91u: byte_accumulator = true; break;
    case 0x819c93u: byte_accumulator = true; break;
    case 0x819c97u: byte_accumulator = true; break;
    case 0x819c99u: byte_accumulator = true; break;
    case 0x819c9du: width_neutral = true; break;
    case 0x819ca5u: width_neutral = true; break;
    case 0x819ca7u: byte_accumulator = true; break;
    case 0x819caau: byte_accumulator = true; break;
    case 0x819cacu: byte_accumulator = true; break;
    case 0x819cafu: byte_accumulator = true; break;
    case 0x819cb2u: byte_accumulator = true; break;
    case 0x819cb3u: byte_accumulator = true; break;
    case 0x819cb4u: byte_accumulator = true; break;
    case 0x819cb6u: byte_accumulator = true; break;
    case 0x819cb8u: byte_accumulator = true; break;
    case 0x819cbau: byte_accumulator = true; break;
    case 0x819cbdu: byte_accumulator = true; break;
    case 0x819cbfu: byte_accumulator = true; break;
    case 0x819cc2u: byte_accumulator = true; break;
    case 0x819cc5u: byte_accumulator = true; break;
    case 0x819cc7u: byte_accumulator = true; break;
    case 0x819ccau: byte_accumulator = true; break;
    case 0x819ccbu: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x819c76u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 86u ||
        memcmp(cart->rom + first,
               "\xa9\x7e\x48\xab\x5a\xc2\x20\xa7\xc3\xe6\xc3\xe2\x20\x8d\xef\x15\x7b\xb9\x25\x00\x10\x2a\xbb"
               "\x85\x11\x5a\x8b\x29\x7f\x8f\x02\x42\x00\xa9\x0f\x8f\x03\x42\x00\xc2\x20\xea\xaf\x16\x42\x00"
               "\xaa\xe2\x20\xbd\xdb\x13\x29\x7f\x0d\xef\x15\x9d\xdb\x13\xab\x7a\x80\x0f\xc9\x04\xd0\x0b\xad"
               "\xce\x13\x29\x7f\x0d\xef\x15\x8d\xce\x13\xa9\xff\x8d\xb0\x15\x7a\x60",
               86u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x819c76u: goto data_bank;
    case 0x819c78u: goto save_bank;
    case 0x819c79u: goto select_bank;
    case 0x819c7au: goto save_target;
    case 0x819c7bu: goto widen_stream;
    case 0x819c7du: goto read_status;
    case 0x819c7fu: goto advance_stream;
    case 0x819c81u: goto narrow_status;
    case 0x819c83u: goto store_status;
    case 0x819c86u: goto clear_target_high;
    case 0x819c87u: goto read_target;
    case 0x819c8au: goto select_party_target;
    case 0x819c8cu: goto preserve_target_index;
    case 0x819c8du: goto store_target_number;
    case 0x819c8fu: goto save_enemy_target;
    case 0x819c90u: goto save_enemy_bank;
    case 0x819c91u: goto mask_enemy_index;
    case 0x819c93u: goto write_enemy_index;
    case 0x819c97u: goto enemy_stride;
    case 0x819c99u: goto write_enemy_stride;
    case 0x819c9du: goto widen_product;
    case 0x819c9fu: goto wait_product;
    case 0x819ca0u: goto read_product;
    case 0x819ca4u: goto select_enemy;
    case 0x819ca5u: goto narrow_enemy;
    case 0x819ca7u: goto read_enemy_status;
    case 0x819caau: goto clear_enemy_status;
    case 0x819cacu: goto or_enemy_status;
    case 0x819cafu: goto store_enemy_status;
    case 0x819cb2u: goto restore_enemy_bank;
    case 0x819cb3u: goto restore_enemy_target;
    case 0x819cb4u: goto skip_special_target;
    case 0x819cb6u: goto test_special_target;
    case 0x819cb8u: goto skip_other_target;
    case 0x819cbau: goto read_special_status;
    case 0x819cbdu: goto clear_special_status;
    case 0x819cbfu: goto or_special_status;
    case 0x819cc2u: goto store_special_status;
    case 0x819cc5u: goto request_status;
    case 0x819cc7u: goto store_request;
    case 0x819ccau: goto restore_target;
    case 0x819ccbu: goto return_status;
    default: return 0;
    }
data_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c76u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x007eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c78u);
}
save_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819c78u, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9c79u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819c79u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x9c7au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819c7au, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9c7bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_stream: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c7bu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9c7du);
}
read_status: {
    const uint32_t pointer_low = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint32_t pointer_bank = (uint16_t)(pointer_low + 2u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high) ||
        !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t expected = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                              ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (expected) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c7du, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    pointer |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    const uint32_t source = (pointer) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c7fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
advance_stream: {
    const uint32_t address = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c7fu, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c81u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_status: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c81u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9c83u);
}
store_status: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x15efu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c83u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c86u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_target_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c86u, 1u, span.instructions != 0u);
    registers->a = registers->dp;
    registers->z = registers->a == 0u;
    registers->n = (registers->a & 0x8000u) != 0u;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c87u);
}
read_target: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0025u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0025u >> 8) != ((0x0025u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c87u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c8au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_party_target: {
    const bool taken = !registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c8au, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9cb6u : 0x9c8cu);
    if (taken)
        goto test_special_target;
}
preserve_target_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c8cu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c8du);
}
store_target_number: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c8du, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9c8fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_enemy_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819c8fu, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9c90u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_enemy_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819c90u, 1u, span.instructions != 0u);
    const uint16_t value = registers->db;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9c91u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_enemy_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c91u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x007fu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c93u);
}
write_enemy_index: {
    const uint32_t address = 0x004202u;
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c93u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, address + 1u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9c97u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
enemy_stride: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c97u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x000fu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9c99u);
}
write_enemy_stride: {
    const uint32_t address = 0x004203u;
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c99u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, address + 1u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9c9du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_product: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c9du, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9c9fu);
}
wait_product: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819c9fu, 1u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ca0u);
}
read_product: {
    const uint32_t address = 0x004216u;
    if (!NativeSpanCanStep(&span, cpu, 6u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ca0u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, address + 1u, true),
                             false);
    NativeSpanStep(&span, registers, cpu, 6u, 0x9ca4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_enemy: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ca4u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ca5u);
}
narrow_enemy: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ca5u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9ca7u);
}
read_enemy_status: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13dbu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x13dbu >> 8) != ((0x13dbu + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ca7u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9caau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_enemy_status: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819caau, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x007fu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9cacu);
}
or_enemy_status: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x15efu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cacu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9cafu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_enemy_status: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13dbu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cafu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9cb2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_enemy_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819cb2u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x9cb3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_enemy_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819cb3u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9cb4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_special_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cb4u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9cc5u);
    goto finished;
}
test_special_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cb6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0004u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9cb8u);
}
skip_other_target: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cb8u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9cc5u : 0x9cbau);
    if (taken)
        goto request_status;
}
read_special_status: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13ceu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cbau, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9cbdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_special_status: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cbdu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x007fu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9cbfu);
}
or_special_status: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x15efu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cbfu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9cc2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_special_status: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13ceu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cc2u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9cc5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
request_status: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cc5u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9cc7u);
}
store_request: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x15b0u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cc7u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ccau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819ccau, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9ccbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_status: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819ccbu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x819ccbu, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunBattleTargetStateSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
    uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget, long *step_count) {
    if (pc >= 0x819bbeu && pc <= 0x819c75u)
        return Lufia2DecompRunBattleTargetMoveSpan(registers, cpu, pc, opcode, dynamic_epoch,
            write_epoch, step_budget, step_count);
    return Lufia2DecompRunBattleTargetStatusSpan(registers, cpu, pc, opcode, dynamic_epoch,
        write_epoch, step_budget, step_count);
}
