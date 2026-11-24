#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunSpriteDmaSetupSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                             uint64_t dynamic_epoch, uint64_t write_epoch,
                                             unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x848193u && pc < 0x84819au)
        end_pc = 0x84819au;
    else if (pc >= 0x84819fu && pc < 0x8481cfu)
        end_pc = 0x8481cfu;
    else if (pc >= 0x8481cfu && pc < 0x8481f6u)
        end_pc = 0x8481f6u;
    else if (pc >= 0x8481fdu && pc < 0x848204u)
        end_pc = 0x848204u;
    else
        return 0;
    if (!cpu || !cpu->ram || registers->e || !step_budget || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    const bool changing_width = pc == 0x848193u || pc == 0x8481cfu || pc == 0x8481e5u;
    const bool byte_accumulator = pc < 0x8481d1u || pc > 0x8481e5u;
    if ((pc > 0x848195u && registers->xf) ||
        (!changing_width && pc != 0x848203u && registers->mf != byte_accumulator))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x848193u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 113u ||
        memcmp(cart->rom + first,
               "\xe2\x20\xc2\x10\xa2\x00\x00\xbc\xc2\x05\xf0\x57\x8c\x02\x43\x84\x54\x9e\xc2\x05\x9e\xc3\x05"
               "\xbd\xd9\x11\x8d\x04\x43\xbc\xe9\x11\x84\x56\x8c\x16\x21\xbc\xf9\x11\x84\x58\x8c\x05\x43\xa9"
               "\x01\x8d\x00\x43\xa9\x18\x8d\x01\x43\xa9\x01\x8d\x0b\x42\xc2\x20\xa5\x56\x18\x69\x00\x01\x8d"
               "\x16\x21\xa5\x58\x8d\x05\x43\x18\x65\x54\x8d\x02\x43\xe2\x20\xa9\x01\x8d\x00\x43\xa9\x18\x8d"
               "\x01\x43\xa9\x01\x8d\x0b\x42\xe8\xe8\xe0\x10\x00\xd0\x9d\x9c\x32\x07\x9c\x33\x07\x6b",
               113u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x848193u: goto set_byte_accumulator;
    case 0x848195u: goto widen_slot_index;
    case 0x848197u: goto first_slot;
    case 0x84819fu: goto dma_source;
    case 0x8481a2u: goto save_source;
    case 0x8481a4u: goto clear_source_low;
    case 0x8481a7u: goto clear_source_high;
    case 0x8481aau: goto load_source_bank;
    case 0x8481adu: goto dma_source_bank;
    case 0x8481b0u: goto load_vram_target;
    case 0x8481b3u: goto save_vram_target;
    case 0x8481b5u: goto dma_vram_target;
    case 0x8481b8u: goto load_transfer_size;
    case 0x8481bbu: goto save_transfer_size;
    case 0x8481bdu: goto dma_transfer_size;
    case 0x8481c0u: goto load_dma_mode;
    case 0x8481c2u: goto set_dma_mode;
    case 0x8481c5u: goto load_vram_port;
    case 0x8481c7u: goto set_dma_port;
    case 0x8481cau: goto first_dma_mask;
    case 0x8481ccu: goto start_first_dma;
    case 0x8481cfu: goto widen_second_transfer;
    case 0x8481d1u: goto reload_vram_target;
    case 0x8481d3u: goto clear_vram_carry;
    case 0x8481d4u: goto advance_vram_target;
    case 0x8481d7u: goto set_second_vram_target;
    case 0x8481dau: goto reload_transfer_size;
    case 0x8481dcu: goto set_second_size;
    case 0x8481dfu: goto clear_source_carry;
    case 0x8481e0u: goto advance_source;
    case 0x8481e2u: goto set_second_source;
    case 0x8481e5u: goto narrow_second_transfer;
    case 0x8481e7u: goto load_second_mode;
    case 0x8481e9u: goto set_second_mode;
    case 0x8481ecu: goto load_second_port;
    case 0x8481eeu: goto set_second_port;
    case 0x8481f1u: goto second_dma_mask;
    case 0x8481f3u: goto start_second_dma;
    case 0x8481fdu: goto clear_cursor_low;
    case 0x848200u: goto clear_cursor_high;
    case 0x848203u: goto return_long;
    default: return 0;
    }
set_byte_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x848193u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8195u);
}
widen_slot_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x848195u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x10u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8197u);
}
first_slot: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x848197u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x819au);
    goto finished;
}
dma_source: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4302u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    const bool mapped_port = (((address >> 16) & 0x7fu) < 0x40u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || (!Lufia2DecompIsWramByte(address) && !mapped_port) ||
        (wide && !Lufia2DecompIsWramByte(high) && !mapped_port))
        goto finished;
    Lufia2SpanFetch(registers, 0x84819fu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->y, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81a2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_source: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481a2u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->y, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81a4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_source_low: {
    const bool wide = false;
    const bool crossed = (0x05c2u >> 8) != ((0x05c2u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05c2u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481a4u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81a7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_source_high: {
    const bool wide = false;
    const bool crossed = (0x05c3u >> 8) != ((0x05c3u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05c3u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481a7u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81aau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_source_bank: {
    const bool wide = false;
    const bool crossed = (0x11d9u >> 8) != ((0x11d9u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x11d9u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481aau, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81adu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
dma_source_bank: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4304u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    const bool mapped_port = (((address >> 16) & 0x7fu) < 0x40u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || (!Lufia2DecompIsWramByte(address) && !mapped_port) ||
        (wide && !Lufia2DecompIsWramByte(high) && !mapped_port))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481adu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81b0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_vram_target: {
    const bool wide = true;
    const bool crossed = (0x11e9u >> 8) != ((0x11e9u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x11e9u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481b0u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81b3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_vram_target: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481b3u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->y, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81b5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
dma_vram_target: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x2116u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    const bool mapped_port = (((address >> 16) & 0x7fu) < 0x40u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || (!Lufia2DecompIsWramByte(address) && !mapped_port) ||
        (wide && !Lufia2DecompIsWramByte(high) && !mapped_port))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481b5u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->y, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81b8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_transfer_size: {
    const bool wide = true;
    const bool crossed = (0x11f9u >> 8) != ((0x11f9u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x11f9u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481b8u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81bbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_transfer_size: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481bbu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->y, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81bdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
dma_transfer_size: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4305u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    const bool mapped_port = (((address >> 16) & 0x7fu) < 0x40u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || (!Lufia2DecompIsWramByte(address) && !mapped_port) ||
        (wide && !Lufia2DecompIsWramByte(high) && !mapped_port))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481bdu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->y, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81c0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_dma_mode: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481c0u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x81c2u);
}
set_dma_mode: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4300u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    const bool mapped_port = (((address >> 16) & 0x7fu) < 0x40u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || (!Lufia2DecompIsWramByte(address) && !mapped_port) ||
        (wide && !Lufia2DecompIsWramByte(high) && !mapped_port))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481c2u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81c5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_vram_port: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481c5u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0018u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x81c7u);
}
set_dma_port: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4301u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    const bool mapped_port = (((address >> 16) & 0x7fu) < 0x40u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || (!Lufia2DecompIsWramByte(address) && !mapped_port) ||
        (wide && !Lufia2DecompIsWramByte(high) && !mapped_port))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481c7u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81cau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_dma_mask: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481cau, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x81ccu);
}
start_first_dma: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x420bu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    const bool mapped_port = (((address >> 16) & 0x7fu) < 0x40u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || (!Lufia2DecompIsWramByte(address) && !mapped_port) ||
        (wide && !Lufia2DecompIsWramByte(high) && !mapped_port))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481ccu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81cfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
widen_second_transfer: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481cfu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x81d1u);
}
reload_vram_target: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481d1u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81d3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_vram_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481d3u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x81d4u);
}
advance_vram_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481d4u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0100u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x81d7u);
}
set_second_vram_target: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x2116u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    const bool mapped_port = (((address >> 16) & 0x7fu) < 0x40u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || (!Lufia2DecompIsWramByte(address) && !mapped_port) ||
        (wide && !Lufia2DecompIsWramByte(high) && !mapped_port))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481d7u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81dau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reload_transfer_size: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481dau, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81dcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_second_size: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4305u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    const bool mapped_port = (((address >> 16) & 0x7fu) < 0x40u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || (!Lufia2DecompIsWramByte(address) && !mapped_port) ||
        (wide && !Lufia2DecompIsWramByte(high) && !mapped_port))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481dcu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81dfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_source_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481dfu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x81e0u);
}
advance_source: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481e0u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81e2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_second_source: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4302u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    const bool mapped_port = (((address >> 16) & 0x7fu) < 0x40u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || (!Lufia2DecompIsWramByte(address) && !mapped_port) ||
        (wide && !Lufia2DecompIsWramByte(high) && !mapped_port))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481e2u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81e5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_second_transfer: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481e5u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x81e7u);
}
load_second_mode: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481e7u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x81e9u);
}
set_second_mode: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4300u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    const bool mapped_port = (((address >> 16) & 0x7fu) < 0x40u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || (!Lufia2DecompIsWramByte(address) && !mapped_port) ||
        (wide && !Lufia2DecompIsWramByte(high) && !mapped_port))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481e9u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81ecu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_second_port: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481ecu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0018u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x81eeu);
}
set_second_port: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4301u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    const bool mapped_port = (((address >> 16) & 0x7fu) < 0x40u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || (!Lufia2DecompIsWramByte(address) && !mapped_port) ||
        (wide && !Lufia2DecompIsWramByte(high) && !mapped_port))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481eeu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81f1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
second_dma_mask: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481f1u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x81f3u);
}
start_second_dma: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x420bu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    const bool mapped_port = (((address >> 16) & 0x7fu) < 0x40u);
    if (!NativeSpanCanStep(&span, cpu, cycles) || (!Lufia2DecompIsWramByte(address) && !mapped_port) ||
        (wide && !Lufia2DecompIsWramByte(high) && !mapped_port))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481f3u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x81f6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
clear_cursor_low: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0732u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x8481fdu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8200u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_cursor_high: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0733u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x848200u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8203u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
return_long: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x848203u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x848203u, 0x6bu, true, registers->mf, false);
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
