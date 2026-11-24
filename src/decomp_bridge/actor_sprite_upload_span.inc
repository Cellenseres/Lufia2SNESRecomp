#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunActorQueueSceneSpriteUpload(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                      uint8_t opcode, uint64_t dynamic_epoch,
                                                      uint64_t write_epoch, unsigned step_budget,
                                                      long *step_count) {
    if (pc < 0x83aae5u || pc >= 0x83ab4fu || !cpu || !cpu->ram || registers->e || !step_budget ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x83aae6u, 0x83ab4fu))
        return 0;
    const bool changing_width = pc == 0x83aae5u || pc == 0x83aaf2u || pc == 0x83aafcu || pc == 0x83ab25u;
    const bool byte_accumulator = pc < 0x83aaf4u || (pc >= 0x83aafeu && pc <= 0x83ab25u);
    if (pc != 0x83aae5u &&
        (!registers->xf || (!changing_width && pc != 0x83ab4eu && registers->mf != byte_accumulator)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x83aae5u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 106u ||
        memcmp(cart->rom + first,
               "\xe2\x30\xa6\xa7\x7b\xbd\x6a\x06\x18\x7f\xee\xe2\x7f\xc2\x20\xa4\xa9\x18\x79\xd1\x13\x85\x54"
               "\xe2\x20\xa9\x83\x85\x56\xa6\xa7\xa7\x54\x29\x7f\x8d\x02\x42\x7b\xbf\x16\xe2\x7f\xaa\xbf\xf8"
               "\xab\x83\x8d\x03\x42\xa4\xab\xb9\xbb\x12\x85\x54\xad\x16\x42\xeb\xa9\x00\xc2\x20\x4a\x18\x79"
               "\xb9\x12\x85\x56\xac\x32\x07\xa6\xa9\xbd\x81\x13\x99\xf9\x11\xbd\x31\x13\x99\xe9\x11\xa5\x54"
               "\x99\xd9\x11\xa5\x56\x99\xc2\x05\xc8\xc8\x8c\x32\x07\x60",
               106u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83aae5u: goto narrow_actor;
    case 0x83aae7u: goto load_actor_slot;
    case 0x83aae9u: goto clear_frame_high;
    case 0x83aaeau: goto read_frame;
    case 0x83aaedu: goto clear_frame_carry;
    case 0x83aaeeu: goto add_motion_offset;
    case 0x83aaf2u: goto widen_motion;
    case 0x83aaf4u: goto load_actor_word_offset;
    case 0x83aaf6u: goto clear_table_carry;
    case 0x83aaf7u: goto add_motion_table;
    case 0x83aafau: goto store_frame_pointer;
    case 0x83aafcu: goto narrow_pointer_bank;
    case 0x83aafeu: goto load_frame_bank;
    case 0x83ab00u: goto store_frame_bank;
    case 0x83ab02u: goto reload_actor_slot;
    case 0x83ab04u: goto read_frame_flags;
    case 0x83ab06u: goto mask_frame;
    case 0x83ab08u: goto write_multiplier_a;
    case 0x83ab0bu: goto clear_type_high;
    case 0x83ab0cu: goto read_type;
    case 0x83ab10u: goto index_type;
    case 0x83ab11u: goto read_frame_stride;
    case 0x83ab15u: goto write_multiplier_b;
    case 0x83ab18u: goto load_actor_record;
    case 0x83ab1au: goto read_source_bank;
    case 0x83ab1du: goto store_source_bank;
    case 0x83ab1fu: goto read_product_low;
    case 0x83ab22u: goto exchange_product;
    case 0x83ab23u: goto clear_product_low;
    case 0x83ab25u: goto widen_product;
    case 0x83ab27u: goto halve_product;
    case 0x83ab28u: goto clear_source_carry;
    case 0x83ab29u: goto add_source_offset;
    case 0x83ab2cu: goto store_source_offset;
    case 0x83ab2eu: goto read_upload_cursor;
    case 0x83ab31u: goto reload_actor_word_offset;
    case 0x83ab33u: goto read_animation_table;
    case 0x83ab36u: goto queue_animation;
    case 0x83ab39u: goto read_vram_base;
    case 0x83ab3cu: goto queue_vram_target;
    case 0x83ab3fu: goto read_source_bank_word;
    case 0x83ab41u: goto queue_source_bank;
    case 0x83ab44u: goto read_source_offset;
    case 0x83ab46u: goto queue_source_offset;
    case 0x83ab49u: goto advance_cursor_low;
    case 0x83ab4au: goto advance_cursor_high;
    case 0x83ab4bu: goto publish_cursor;
    case 0x83ab4eu: goto return_short;
    default: return 0;
    }
narrow_actor: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aae5u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x30u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xaae7u);
}
load_actor_slot: {
    const bool wide = !registers->xf;
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aae7u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaae9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_frame_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aae9u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->dp, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaaeau);
}
read_frame: {
    const bool wide = false;
    const bool crossed = (0x066au >> 8) != ((0x066au + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x066au + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aaeau, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaaedu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_frame_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aaedu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xaaeeu);
}
add_motion_offset: {
    const bool wide = false;
    const uint32_t address = (0x7fe2eeu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aaeeu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaaf2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_motion: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aaf2u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xaaf4u);
}
load_actor_word_offset: {
    const bool wide = !registers->xf;
    const uint32_t address = (uint16_t)(registers->dp + 0xa9u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aaf4u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaaf6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_table_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aaf6u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xaaf7u);
}
add_motion_table: {
    const bool wide = true;
    const bool crossed = (0x13d1u >> 8) != ((0x13d1u + registers->y) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13d1u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aaf7u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaafau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_frame_pointer: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aafau, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaafcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_pointer_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aafcu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xaafeu);
}
load_frame_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aafeu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0083u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xab00u);
}
store_frame_bank: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab00u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab02u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reload_actor_slot: {
    const bool wide = !registers->xf;
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab02u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab04u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_frame_flags: {
    const uint16_t low = (uint16_t)(registers->dp + 0x54u);
    const uint16_t high = (uint16_t)(low + 1u);
    const uint16_t bank = (uint16_t)(low + 2u);
    const unsigned cycles = 6u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high) || !Lufia2DecompIsWramByte(bank))
        goto finished;
    const uint32_t pointer =
        cpu->ram[low] | ((uint32_t)cpu->ram[high] << 8) | ((uint32_t)cpu->ram[bank] << 16);
    if (!Lufia2DecompIsDecompressSourceByte(cart, pointer))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab04u, 2u, span.instructions != 0u);
    uint32_t address = Lufia2DecompReadData(registers, low, high, true);
    address |= (uint32_t)registers->read(registers->mem, bank) << 16;
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab06u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_frame: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab06u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x007fu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xab08u);
}
write_multiplier_a: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4202u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    const bool mapped_math = !wide && (((address >> 16) & 0x7fu) < 0x40u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || (!Lufia2DecompIsWramByte(address) && !mapped_math) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab08u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab0bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_type_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab0bu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->dp, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xab0cu);
}
read_type: {
    const bool wide = false;
    const uint32_t address = (0x7fe216u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab0cu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab10u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
index_type: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab10u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->xf ? (uint8_t)registers->a : registers->a,
                             registers->xf);
    NativeSpanStep(&span, registers, cpu, 2u, 0xab11u);
}
read_frame_stride: {
    const bool wide = false;
    const uint32_t address = (0x83abf8u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab11u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab15u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
write_multiplier_b: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4203u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    const bool mapped_math = !wide && (((address >> 16) & 0x7fu) < 0x40u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || (!Lufia2DecompIsWramByte(address) && !mapped_math) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab15u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab18u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_actor_record: {
    const bool wide = !registers->xf;
    const uint32_t address = (uint16_t)(registers->dp + 0xabu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab18u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab1au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_source_bank: {
    const bool wide = false;
    const bool crossed = (0x12bbu >> 8) != ((0x12bbu + registers->y) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x12bbu + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab1au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab1du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_source_bank: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab1du, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab1fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_product_low: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4216u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    const bool mapped_math = !wide && (((address >> 16) & 0x7fu) < 0x40u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || (!Lufia2DecompIsWramByte(address) && !mapped_math) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab1fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab22u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
exchange_product: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab22u, 1u, span.instructions != 0u);
    registers->a = (uint16_t)((registers->a >> 8) | (registers->a << 8));
    registers->z = (registers->a & 0xffu) == 0u;
    registers->n = (registers->a & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 3u, 0xab23u);
}
clear_product_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab23u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xab25u);
}
widen_product: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab25u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xab27u);
}
halve_product: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab27u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)registers->a >> 1, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xab28u);
}
clear_source_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab28u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xab29u);
}
add_source_offset: {
    const bool wide = true;
    const bool crossed = (0x12b9u >> 8) != ((0x12b9u + registers->y) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x12b9u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab29u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab2cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_source_offset: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab2cu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab2eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_upload_cursor: {
    const bool wide = !registers->xf;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0732u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab2eu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab31u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reload_actor_word_offset: {
    const bool wide = !registers->xf;
    const uint32_t address = (uint16_t)(registers->dp + 0xa9u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab31u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab33u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_animation_table: {
    const bool wide = true;
    const bool crossed = (0x1381u >> 8) != ((0x1381u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1381u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab33u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab36u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
queue_animation: {
    const bool wide = true;
    const bool crossed = (0x11f9u >> 8) != ((0x11f9u + registers->y) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x11f9u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab36u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab39u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_vram_base: {
    const bool wide = true;
    const bool crossed = (0x1331u >> 8) != ((0x1331u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1331u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab39u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab3cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
queue_vram_target: {
    const bool wide = true;
    const bool crossed = (0x11e9u >> 8) != ((0x11e9u + registers->y) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x11e9u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab3cu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab3fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_source_bank_word: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab3fu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab41u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
queue_source_bank: {
    const bool wide = true;
    const bool crossed = (0x11d9u >> 8) != ((0x11d9u + registers->y) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x11d9u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab41u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab44u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_source_offset: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab44u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab46u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
queue_source_offset: {
    const bool wide = true;
    const bool crossed = (0x05c2u >> 8) != ((0x05c2u + registers->y) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05c2u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab46u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab49u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
advance_cursor_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab49u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u,
                             registers->xf ? (uint8_t)(registers->y + 1u) : (uint16_t)(registers->y + 1u),
                             registers->xf);
    NativeSpanStep(&span, registers, cpu, 2u, 0xab4au);
}
advance_cursor_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab4au, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u,
                             registers->xf ? (uint8_t)(registers->y + 1u) : (uint16_t)(registers->y + 1u),
                             registers->xf);
    NativeSpanStep(&span, registers, cpu, 2u, 0xab4bu);
}
publish_cursor: {
    const bool wide = !registers->xf;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0732u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab4bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->y, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xab4eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
return_short: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83ab4eu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83ab4eu, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunObjectQueueSceneSpriteUpload(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                       uint8_t opcode, uint64_t dynamic_epoch,
                                                       uint64_t write_epoch, unsigned step_budget,
                                                       long *step_count) {
    if (pc < 0x83fcd1u || pc >= 0x83fd0bu || !cpu || !cpu->ram || registers->e || !step_budget ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x83fcd2u, 0x83fd0bu))
        return 0;
    const bool changing_width = pc == 0x83fcddu || pc == 0x83fcefu || pc == 0x83fcfau || pc == 0x83fd03u;
    const bool byte_accumulator = pc < 0x83fcdfu || (pc >= 0x83fcf1u && pc <= 0x83fcfau) || pc >= 0x83fd05u;
    if (!changing_width && pc != 0x83fd0au && registers->mf != byte_accumulator)
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x83fcd1u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 58u ||
        memcmp(cart->rom + first,
               "\xac\x32\x07\x7b\xa6\xa7\xbf\x16\xe2\x7f\x0a\xaa\xc2\x20\xbf\xfc\xab\x83\x99\xf9\x11\xa6\xa9"
               "\xbf\x5e\xe0\x7f\x99\xe9\x11\xe2\x20\xa6\xab\xbf\x96\xd8\x7f\x99\xd9\x11\xc2\x20\xbf\x94\xd8"
               "\x7f\x99\xc2\x05\xe2\x20\xc8\xc8\x8c\x32\x07\x60",
               58u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83fcd1u: goto read_upload_cursor;
    case 0x83fcd4u: goto clear_type_high;
    case 0x83fcd5u: goto load_actor_slot;
    case 0x83fcd7u: goto read_type;
    case 0x83fcdbu: goto double_type;
    case 0x83fcdcu: goto index_type;
    case 0x83fcddu: goto widen_animation;
    case 0x83fcdfu: goto read_animation_table;
    case 0x83fce3u: goto queue_animation;
    case 0x83fce6u: goto load_actor_word_offset;
    case 0x83fce8u: goto read_object_vram;
    case 0x83fcecu: goto queue_vram_target;
    case 0x83fcefu: goto narrow_source_bank;
    case 0x83fcf1u: goto load_actor_record;
    case 0x83fcf3u: goto read_object_bank;
    case 0x83fcf7u: goto queue_source_bank;
    case 0x83fcfau: goto widen_source_offset;
    case 0x83fcfcu: goto read_object_offset;
    case 0x83fd00u: goto queue_source_offset;
    case 0x83fd03u: goto narrow_cursor;
    case 0x83fd05u: goto advance_cursor_low;
    case 0x83fd06u: goto advance_cursor_high;
    case 0x83fd07u: goto publish_cursor;
    case 0x83fd0au: goto return_short;
    default: return 0;
    }
read_upload_cursor: {
    const bool wide = !registers->xf;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0732u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fcd1u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfcd4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_type_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fcd4u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->dp, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xfcd5u);
}
load_actor_slot: {
    const bool wide = !registers->xf;
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fcd5u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfcd7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_type: {
    const bool wide = false;
    const uint32_t address = (0x7fe216u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fcd7u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfcdbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
double_type: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fcdbu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x0080u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a << 1), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xfcdcu);
}
index_type: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fcdcu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->xf ? (uint8_t)registers->a : registers->a,
                             registers->xf);
    NativeSpanStep(&span, registers, cpu, 2u, 0xfcddu);
}
widen_animation: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fcddu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xfcdfu);
}
read_animation_table: {
    const bool wide = true;
    const uint32_t address = (0x83abfcu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fcdfu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfce3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
queue_animation: {
    const bool wide = true;
    const bool crossed = (0x11f9u >> 8) != ((0x11f9u + registers->y) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x11f9u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fce3u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfce6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_actor_word_offset: {
    const bool wide = !registers->xf;
    const uint32_t address = (uint16_t)(registers->dp + 0xa9u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fce6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfce8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_object_vram: {
    const bool wide = true;
    const uint32_t address = (0x7fe05eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fce8u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfcecu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
queue_vram_target: {
    const bool wide = true;
    const bool crossed = (0x11e9u >> 8) != ((0x11e9u + registers->y) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x11e9u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fcecu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfcefu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_source_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fcefu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xfcf1u);
}
load_actor_record: {
    const bool wide = !registers->xf;
    const uint32_t address = (uint16_t)(registers->dp + 0xabu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fcf1u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfcf3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_object_bank: {
    const bool wide = false;
    const uint32_t address = (0x7fd896u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fcf3u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfcf7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
queue_source_bank: {
    const bool wide = false;
    const bool crossed = (0x11d9u >> 8) != ((0x11d9u + registers->y) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x11d9u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fcf7u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfcfau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_source_offset: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fcfau, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xfcfcu);
}
read_object_offset: {
    const bool wide = true;
    const uint32_t address = (0x7fd894u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fcfcu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfd00u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
queue_source_offset: {
    const bool wide = true;
    const bool crossed = (0x05c2u >> 8) != ((0x05c2u + registers->y) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05c2u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fd00u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfd03u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_cursor: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fd03u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xfd05u);
}
advance_cursor_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fd05u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u,
                             registers->xf ? (uint8_t)(registers->y + 1u) : (uint16_t)(registers->y + 1u),
                             registers->xf);
    NativeSpanStep(&span, registers, cpu, 2u, 0xfd06u);
}
advance_cursor_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fd06u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u,
                             registers->xf ? (uint8_t)(registers->y + 1u) : (uint16_t)(registers->y + 1u),
                             registers->xf);
    NativeSpanStep(&span, registers, cpu, 2u, 0xfd07u);
}
publish_cursor: {
    const bool wide = !registers->xf;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0732u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83fd07u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->y, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xfd0au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
return_short: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83fd0au);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83fd0au, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunSpriteUploadSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                           uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget,
                                           long *step_count) {
    if (pc >= 0x83fcd1u)
        return Lufia2DecompRunObjectQueueSceneSpriteUpload(registers, cpu, pc, opcode, dynamic_epoch,
                                                           write_epoch, step_budget, step_count);
    return Lufia2DecompRunActorQueueSceneSpriteUpload(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                      step_budget, step_count);
}
