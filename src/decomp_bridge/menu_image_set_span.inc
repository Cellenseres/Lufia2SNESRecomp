#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuImageSetSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                           uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget,
                                           long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x868f6fu && pc < 0x868fa2u)
        end_pc = 0x868fa2u;
    else if (pc >= 0x868fa2u && pc < 0x868fa5u)
        end_pc = 0x868fa5u;
    else if (pc >= 0x868fa5u && pc < 0x868fb5u)
        end_pc = 0x868fb5u;
    else if (pc >= 0x868fb5u && pc < 0x868fb8u)
        end_pc = 0x868fb8u;
    else if (pc >= 0x868fb8u && pc < 0x868fc8u)
        end_pc = 0x868fc8u;
    else if (pc >= 0x868fc8u && pc < 0x868fcbu)
        end_pc = 0x868fcbu;
    else if (pc >= 0x868fcbu && pc < 0x868ff1u)
        end_pc = 0x868ff1u;
    else if (pc >= 0x868ff1u && pc < 0x868ff5u)
        end_pc = 0x868ff5u;
    else if (pc >= 0x868ff5u && pc < 0x868ff6u)
        end_pc = 0x868ff6u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x868f6fu: byte_accumulator = true; break;
    case 0x868f72u: byte_accumulator = true; break;
    case 0x868f74u: byte_accumulator = true; break;
    case 0x868f76u: byte_accumulator = true; break;
    case 0x868f78u: width_neutral = true; break;
    case 0x868f7au: byte_accumulator = true; break;
    case 0x868f7cu: byte_accumulator = true; break;
    case 0x868f7du: byte_accumulator = true; break;
    case 0x868f7fu: byte_accumulator = true; break;
    case 0x868f82u: byte_accumulator = true; break;
    case 0x868f83u: byte_accumulator = true; break;
    case 0x868f84u: byte_accumulator = true; break;
    case 0x868f87u: byte_accumulator = true; break;
    case 0x868f88u: byte_accumulator = true; break;
    case 0x868f8cu: byte_accumulator = true; break;
    case 0x868f8eu: width_neutral = true; break;
    case 0x868fd3u: width_neutral = true; break;
    case 0x868fd5u: byte_accumulator = true; break;
    case 0x868fd7u: byte_accumulator = true; break;
    case 0x868fd9u: byte_accumulator = true; break;
    case 0x868fdcu: byte_accumulator = true; break;
    case 0x868fdeu: byte_accumulator = true; break;
    case 0x868fe1u: byte_accumulator = true; break;
    case 0x868fe3u: byte_accumulator = true; break;
    case 0x868fe5u: byte_accumulator = true; break;
    case 0x868fe7u: byte_accumulator = true; break;
    case 0x868feau: byte_accumulator = true; break;
    case 0x868fecu: byte_accumulator = true; break;
    case 0x868fefu: byte_accumulator = true; break;
    case 0x868ff1u: byte_accumulator = true; break;
    case 0x868ff5u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x868f6fu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 135u ||
        memcmp(cart->rom + first,
               "\xa2\x00\x60\x86\x1c\x64\x04\x64\x05\xe2\x20\xa9\x00\xeb\xa6\x04\xbd\x7b\x0a\x0a\x18\x7d\x7b"
               "\x0a\xaa\xbf\xc4\xe5\x8e\x85\x1b\xc2\x20\xbf\xc2\xe5\x8e\x18\x69\x00\x02\x85\x19\xa5\x19\x85"
               "\x08\xa5\x1c\x85\x0b\x20\xf6\x8f\xa5\x19\x18\x69\x00\x04\x85\x08\xa5\x1c\x18\x69\x00\x01\x85"
               "\x0b\x20\xf6\x8f\xa5\x19\x18\x69\x00\x0a\x85\x08\xa5\x1c\x18\x69\x00\x04\x85\x0b\x20\xf6\x8f"
               "\xa5\x1c\x18\x69\x00\x08\x85\x1c\xe2\x20\xe6\x04\xa5\x04\xcd\x7a\x0a\xd0\x9a\xa2\x00\x60\x86"
               "\x5d\xa9\x7e\x85\x5f\xa2\x00\x30\x86\x60\xa2\x00\x20\x86\x58\x22\x44\x80\x82\x6b",
               135u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x868f6fu: goto initial_image_target;
    case 0x868f72u: goto store_image_target;
    case 0x868f74u: goto clear_list_index_low;
    case 0x868f76u: goto clear_list_index_high;
    case 0x868f78u: goto narrow_image_index;
    case 0x868f7au: goto clear_index_high_source;
    case 0x868f7cu: goto clear_image_index_high;
    case 0x868f7du: goto read_list_index;
    case 0x868f7fu: goto read_image_id;
    case 0x868f82u: goto double_image_id;
    case 0x868f83u: goto clear_image_offset_carry;
    case 0x868f84u: goto triple_image_id;
    case 0x868f87u: goto select_image_descriptor;
    case 0x868f88u: goto read_image_bank;
    case 0x868f8cu: goto store_image_bank;
    case 0x868f8eu: goto widen_image_address;
    case 0x868f90u: goto read_image_base;
    case 0x868f94u: goto clear_image_base_carry;
    case 0x868f95u: goto offset_image_base;
    case 0x868f98u: goto store_image_base;
    case 0x868f9au: goto first_block_source;
    case 0x868f9cu: goto store_first_source;
    case 0x868f9eu: goto first_block_target;
    case 0x868fa0u: goto store_first_target;
    case 0x868fa2u: goto copy_first_block;
    case 0x868fa5u: goto second_block_source;
    case 0x868fa7u: goto clear_second_source_carry;
    case 0x868fa8u: goto offset_second_source;
    case 0x868fabu: goto store_second_source;
    case 0x868fadu: goto second_block_target;
    case 0x868fafu: goto clear_second_target_carry;
    case 0x868fb0u: goto offset_second_target;
    case 0x868fb3u: goto store_second_target;
    case 0x868fb5u: goto copy_second_block;
    case 0x868fb8u: goto third_block_source;
    case 0x868fbau: goto clear_third_source_carry;
    case 0x868fbbu: goto offset_third_source;
    case 0x868fbeu: goto store_third_source;
    case 0x868fc0u: goto third_block_target;
    case 0x868fc2u: goto clear_third_target_carry;
    case 0x868fc3u: goto offset_third_target;
    case 0x868fc6u: goto store_third_target;
    case 0x868fc8u: goto copy_third_block;
    case 0x868fcbu: goto next_image_target;
    case 0x868fcdu: goto clear_next_image_carry;
    case 0x868fceu: goto offset_next_image_target;
    case 0x868fd1u: goto store_next_image_target;
    case 0x868fd3u: goto narrow_list_counter;
    case 0x868fd5u: goto count_loaded_image;
    case 0x868fd7u: goto read_loaded_count;
    case 0x868fd9u: goto compare_list_count;
    case 0x868fdcu: goto repeat_images;
    case 0x868fdeu: goto upload_source;
    case 0x868fe1u: goto store_upload_source;
    case 0x868fe3u: goto upload_source_bank;
    case 0x868fe5u: goto store_upload_bank;
    case 0x868fe7u: goto upload_video_target;
    case 0x868feau: goto store_video_target;
    case 0x868fecu: goto upload_size;
    case 0x868fefu: goto store_upload_size;
    case 0x868ff1u: goto queue_image_upload;
    case 0x868ff5u: goto return_image_set;
    default: return 0;
    }
initial_image_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f6fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x6000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8f72u);
}
store_image_target: {
    const uint32_t address = (uint16_t)(registers->dp + 0x1cu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f72u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8f74u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_list_index_low: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f74u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8f76u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_list_index_high: {
    const uint32_t address = (uint16_t)(registers->dp + 0x05u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f76u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8f78u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_image_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f78u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8f7au);
}
clear_index_high_source: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f7au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8f7cu);
}
clear_image_index_high: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f7cu, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8f7du);
}
read_list_index: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f7du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8f7fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_image_id: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0a7bu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0a7bu >> 8) != ((0x0a7bu + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f7fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8f82u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
double_image_id: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f82u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8f83u);
}
clear_image_offset_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f83u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8f84u);
}
triple_image_id: {
    const uint32_t address = ((((uint32_t)registers->db << 16) | 0x0a7bu) + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0a7bu >> 8) != ((0x0a7bu + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f84u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8f87u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_image_descriptor: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f87u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8f88u);
}
read_image_bank: {
    const uint32_t address = (0x8ee5c4u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f88u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8f8cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_image_bank: {
    const uint32_t address = (uint16_t)(registers->dp + 0x1bu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f8cu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8f8eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_image_address: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f8eu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8f90u);
}
read_image_base: {
    const uint32_t address = (0x8ee5c2u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f90u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8f94u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_image_base_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f94u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8f95u);
}
offset_image_base: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f95u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0200u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8f98u);
}
store_image_base: {
    const uint32_t address = (uint16_t)(registers->dp + 0x19u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f98u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8f9au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_block_source: {
    const uint32_t address = (uint16_t)(registers->dp + 0x19u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f9au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8f9cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_first_source: {
    const uint32_t address = (uint16_t)(registers->dp + 0x08u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f9cu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8f9eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_block_target: {
    const uint32_t address = (uint16_t)(registers->dp + 0x1cu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868f9eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fa0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_first_target: {
    const uint32_t address = (uint16_t)(registers->dp + 0x0bu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fa0u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fa2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
copy_first_block: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x868fa2u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x868fa2u, 0x20u, 0x868ff6u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
second_block_source: {
    const uint32_t address = (uint16_t)(registers->dp + 0x19u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fa5u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fa7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_second_source_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fa7u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8fa8u);
}
offset_second_source: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fa8u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0400u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8fabu);
}
store_second_source: {
    const uint32_t address = (uint16_t)(registers->dp + 0x08u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fabu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fadu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
second_block_target: {
    const uint32_t address = (uint16_t)(registers->dp + 0x1cu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fadu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fafu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_second_target_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fafu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8fb0u);
}
offset_second_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fb0u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0100u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8fb3u);
}
store_second_target: {
    const uint32_t address = (uint16_t)(registers->dp + 0x0bu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fb3u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fb5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
copy_second_block: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x868fb5u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x868fb5u, 0x20u, 0x868ff6u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
third_block_source: {
    const uint32_t address = (uint16_t)(registers->dp + 0x19u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fb8u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fbau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_third_source_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fbau, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8fbbu);
}
offset_third_source: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fbbu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0a00u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8fbeu);
}
store_third_source: {
    const uint32_t address = (uint16_t)(registers->dp + 0x08u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fbeu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fc0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
third_block_target: {
    const uint32_t address = (uint16_t)(registers->dp + 0x1cu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fc0u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fc2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_third_target_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fc2u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8fc3u);
}
offset_third_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fc3u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0400u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8fc6u);
}
store_third_target: {
    const uint32_t address = (uint16_t)(registers->dp + 0x0bu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fc6u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fc8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
copy_third_block: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x868fc8u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x868fc8u, 0x20u, 0x868ff6u, false, false, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
next_image_target: {
    const uint32_t address = (uint16_t)(registers->dp + 0x1cu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fcbu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fcdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_next_image_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fcdu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8fceu);
}
offset_next_image_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fceu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0800u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8fd1u);
}
store_next_image_target: {
    const uint32_t address = (uint16_t)(registers->dp + 0x1cu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fd1u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fd3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_list_counter: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fd3u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8fd5u);
}
count_loaded_image: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fd5u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fd7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_loaded_count: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fd7u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fd9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_list_count: {
    const uint32_t address = ((((uint32_t)registers->db << 16) | 0x0a7au) + 0u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fd9u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fdcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_images: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fdcu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8f78u : 0x8fdeu);
    if (taken)
        goto finished;
}
upload_source: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fdeu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x6000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8fe1u);
}
store_upload_source: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5du);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fe1u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fe3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
upload_source_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fe3u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x007eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8fe5u);
}
store_upload_bank: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fe5u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fe7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
upload_video_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fe7u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x3000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8feau);
}
store_video_target: {
    const uint32_t address = (uint16_t)(registers->dp + 0x60u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868feau, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8fecu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
upload_size: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fecu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x2000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8fefu);
}
store_upload_size: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x868fefu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8ff1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
queue_image_upload: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x868ff1u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x868ff1u, 0x22u, 0x828044u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_image_set: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x868ff5u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x868ff5u, 0x6bu, true, registers->mf, true);
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
