#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static bool Lufia2DecompIsInventorySpritePc(uint32_t pc) {
    return (pc >= 0x82bd48u && pc <= 0x82be03u) ||
        (pc >= 0x83a9bau && pc <= 0x83a9cfu) || (pc >= 0x83a9e5u && pc <= 0x83aa2fu);
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunActorLoadSprite(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                          uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget,
                                          long *step_count) {
    const uint32_t phase_end = pc < 0x83a9c3u   ? 0x83a9c3u
                               : pc < 0x83a9c7u ? 0x83a9c7u
                               : pc < 0x83a9cau ? 0x83a9cau
                                                : 0x83a9d0u;
    if (pc < 0x83a9bau || pc >= 0x83a9d0u || !cpu || !cpu->ram || registers->e || !step_budget ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        (pc + 1u < phase_end && !Lufia2DecompFrameRangeQuiet(pc + 1u, phase_end)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x83a9bau;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 22u ||
        memcmp(cart->rom + first,
               "\x48\xda\x5a\x8b\x08\xe2\x30\x4b\xab\x22\xe5\xa9\x83\x20\x50\xaa\x28\xab\x7a\xfa\x68\x6b",
               22u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83a9bau: goto save_accumulator;
    case 0x83a9bbu: goto save_x;
    case 0x83a9bcu: goto save_y;
    case 0x83a9bdu: goto save_bank;
    case 0x83a9beu: goto save_status;
    case 0x83a9bfu: goto narrow_registers;
    case 0x83a9c1u: goto push_program_bank;
    case 0x83a9c2u: goto select_program_bank;
    case 0x83a9c3u: goto read_descriptor;
    case 0x83a9c7u: goto allocate_sprite;
    case 0x83a9cau: goto restore_status;
    case 0x83a9cbu: goto restore_bank;
    case 0x83a9ccu: goto restore_y;
    case 0x83a9cdu: goto restore_x;
    case 0x83a9ceu: goto restore_accumulator;
    case 0x83a9cfu: goto return_long;
    default: return 0;
    }
save_accumulator: {
    const bool wide = !registers->mf;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < (wide ? 0x1f02u : 0x1f00u) ||
        registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9bau, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    if (wide)
        registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa9bbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_x: {
    const bool wide = !registers->xf;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < (wide ? 0x1f02u : 0x1f00u) ||
        registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9bbu, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    if (wide)
        registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa9bcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_y: {
    const bool wide = !registers->xf;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < (wide ? 0x1f02u : 0x1f00u) ||
        registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9bcu, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    if (wide)
        registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa9bdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_bank: {
    const bool wide = false;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < (wide ? 0x1f02u : 0x1f00u) ||
        registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9bdu, 1u, span.instructions != 0u);
    const uint16_t value = registers->db;
    if (wide)
        registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa9beu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_status: {
    const bool wide = false;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < (wide ? 0x1f02u : 0x1f00u) ||
        registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9beu, 1u, span.instructions != 0u);
    const uint16_t value = interp816_getFlags(registers);
    if (wide)
        registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa9bfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_registers: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9bfu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x30u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa9c1u);
}
push_program_bank: {
    const bool wide = false;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < (wide ? 0x1f02u : 0x1f00u) ||
        registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9c1u, 1u, span.instructions != 0u);
    const uint16_t value = registers->k;
    if (wide)
        registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa9c2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_program_bank: {
    const bool wide = false;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u ||
        registers->sp > (wide ? 0x1ffdu : 0x1ffeu))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9c2u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->db = value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, cycles, 0xa9c3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_descriptor: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a9c3u);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83a9c3u, 0x22u, 0x83a9e5u, true,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
allocate_sprite: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a9c7u);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83a9c7u, 0x20u, 0x83aa50u, false,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_status: {
    const bool wide = false;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u ||
        registers->sp > (wide ? 0x1ffdu : 0x1ffeu))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9cau, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    interp816_setFlags(registers, value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa9cbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_bank: {
    const bool wide = false;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u ||
        registers->sp > (wide ? 0x1ffdu : 0x1ffeu))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9cbu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->db = value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, cycles, 0xa9ccu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_y: {
    const bool wide = !registers->xf;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u ||
        registers->sp > (wide ? 0x1ffdu : 0x1ffeu))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9ccu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa9cdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_x: {
    const bool wide = !registers->xf;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u ||
        registers->sp > (wide ? 0x1ffdu : 0x1ffeu))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9cdu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa9ceu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_accumulator: {
    const bool wide = !registers->mf;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u ||
        registers->sp > (wide ? 0x1ffdu : 0x1ffeu))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9ceu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa9u, value, !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa9cfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
return_long: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a9cfu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83a9cfu, 0x6bu, true, registers->mf, true);
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
static int Lufia2DecompRunActorSpriteDescriptor(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                unsigned step_budget, long *step_count) {
    if (pc < 0x83a9e5u || pc >= 0x83aa30u || !cpu || !cpu->ram || registers->e || !step_budget ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x83a9e6u, 0x83aa30u))
        return 0;
    const bool any_width = pc <= 0x83a9e6u || pc == 0x83a9f1u || pc == 0x83a9fbu || pc == 0x83aa2fu;
    const bool byte_index = !(pc >= 0x83a9f3u && pc <= 0x83a9f9u);
    const bool byte_accumulator =
        !((pc >= 0x83a9f3u && pc <= 0x83a9f9u) || (pc >= 0x83aa1eu && pc <= 0x83aa26u));
    const bool changing_accumulator = pc == 0x83aa1cu || pc == 0x83aa27u;
    if (!any_width &&
        (registers->xf != byte_index || (!changing_accumulator && registers->mf != byte_accumulator)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x83a9e5u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 75u ||
        memcmp(cart->rom + first,
               "\x8b\xe2\x30\xa6\xa7\x9d\xd2\x05\xeb\xa9\x00\xeb\xc2\x30\x0a\xaa\xbf\x00\xf0\xcf\x85\x5d\xe2"
               "\x30\xa9\xcf\x85\x5f\xa6\xa7\xa7\x5d\x29\x07\x9f\x16\xe2\x7f\xa7\x5d\x29\xf8\x9d\x91\x12\xa0"
               "\x01\xb7\x5d\x0a\x9f\xce\xe1\x7f\xc8\xc2\x20\xa6\xab\xb7\x5d\x9d\xb9\x12\xc8\xc8\xe2\x20\xb7"
               "\x5d\x9d\xbb\x12\xab\x6b",
               75u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83a9e5u: goto save_bank;
    case 0x83a9e6u: goto narrow_slot;
    case 0x83a9e8u: goto load_actor_slot;
    case 0x83a9eau: goto store_sprite_id;
    case 0x83a9edu: goto exchange_sprite_id;
    case 0x83a9eeu: goto clear_sprite_high;
    case 0x83a9f0u: goto restore_sprite_id;
    case 0x83a9f1u: goto widen_descriptor_index;
    case 0x83a9f3u: goto double_sprite_id;
    case 0x83a9f4u: goto index_descriptor;
    case 0x83a9f5u: goto read_descriptor_offset;
    case 0x83a9f9u: goto store_descriptor_offset;
    case 0x83a9fbu: goto narrow_descriptor;
    case 0x83a9fdu: goto load_descriptor_bank;
    case 0x83a9ffu: goto store_descriptor_bank;
    case 0x83aa01u: goto reload_actor_slot;
    case 0x83aa03u: goto read_type;
    case 0x83aa05u: goto mask_type;
    case 0x83aa07u: goto store_type;
    case 0x83aa0bu: goto read_flags;
    case 0x83aa0du: goto mask_flags;
    case 0x83aa0fu: goto store_flags;
    case 0x83aa12u: goto first_descriptor_byte;
    case 0x83aa14u: goto read_step;
    case 0x83aa16u: goto double_step;
    case 0x83aa17u: goto store_step;
    case 0x83aa1bu: goto next_descriptor_byte;
    case 0x83aa1cu: goto widen_source;
    case 0x83aa1eu: goto load_actor_record;
    case 0x83aa20u: goto read_source_offset;
    case 0x83aa22u: goto store_source_offset;
    case 0x83aa25u: goto source_bank_low;
    case 0x83aa26u: goto source_bank_high;
    case 0x83aa27u: goto narrow_source_bank;
    case 0x83aa29u: goto read_source_bank;
    case 0x83aa2bu: goto store_source_bank;
    case 0x83aa2eu: goto restore_bank;
    case 0x83aa2fu: goto return_long;
    default: return 0;
    }
save_bank: {
    const bool wide = false;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < (wide ? 0x1f02u : 0x1f00u) ||
        registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9e5u, 1u, span.instructions != 0u);
    const uint16_t value = registers->db;
    if (wide)
        registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa9e6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_slot: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9e6u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x30u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa9e8u);
}
load_actor_slot: {
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9e8u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa9eau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_sprite_id: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05d2u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || ((0x05d2u >> 8) != ((0x05d2u + registers->x) >> 8)));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9eau, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa9edu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
exchange_sprite_id: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9edu, 1u, span.instructions != 0u);
    registers->a = (uint16_t)((registers->a >> 8) | (registers->a << 8));
    registers->z = (registers->a & 0xffu) == 0u;
    registers->n = (registers->a & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 3u, 0xa9eeu);
}
clear_sprite_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9eeu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa9f0u);
}
restore_sprite_id: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9f0u, 1u, span.instructions != 0u);
    registers->a = (uint16_t)((registers->a >> 8) | (registers->a << 8));
    registers->z = (registers->a & 0xffu) == 0u;
    registers->n = (registers->a & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 3u, 0xa9f1u);
}
widen_descriptor_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9f1u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x30u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa9f3u);
}
double_sprite_id: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9f3u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa9f4u);
}
index_descriptor: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9f4u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa9f5u);
}
read_descriptor_offset: {
    const uint32_t address = (0xcff000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9f5u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa9f9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_descriptor_offset: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5du);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9f9u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa9fbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_descriptor: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9fbu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x30u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa9fdu);
}
load_descriptor_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9fdu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00cfu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa9ffu);
}
store_descriptor_bank: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a9ffu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa01u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reload_actor_slot: {
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa01u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa03u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_type: {
    const uint16_t pointer_low = (uint16_t)(registers->dp + 0x5du);
    const uint16_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint16_t pointer_bank = (uint16_t)(pointer_low + 2u);
    const unsigned cycles = 6u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(pointer_low) ||
        !Lufia2DecompIsWramByte(pointer_high) || !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t pointer = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                             ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (pointer) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa03u, 2u, span.instructions != 0u);
    uint32_t source = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    source |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    source = (source) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa05u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_type: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa05u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x0007u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaa07u);
}
store_type: {
    const uint32_t address = (0x7fe216u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa07u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa0bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_flags: {
    const uint16_t pointer_low = (uint16_t)(registers->dp + 0x5du);
    const uint16_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint16_t pointer_bank = (uint16_t)(pointer_low + 2u);
    const unsigned cycles = 6u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(pointer_low) ||
        !Lufia2DecompIsWramByte(pointer_high) || !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t pointer = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                             ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (pointer) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa0bu, 2u, span.instructions != 0u);
    uint32_t source = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    source |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    source = (source) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa0du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_flags: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa0du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00f8u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaa0fu);
}
store_flags: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1291u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || ((0x1291u >> 8) != ((0x1291u + registers->x) >> 8)));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa0fu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa12u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_descriptor_byte: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa12u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaa14u);
}
read_step: {
    const uint16_t pointer_low = (uint16_t)(registers->dp + 0x5du);
    const uint16_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint16_t pointer_bank = (uint16_t)(pointer_low + 2u);
    const unsigned cycles = 6u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(pointer_low) ||
        !Lufia2DecompIsWramByte(pointer_high) || !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t pointer = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                             ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (pointer + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa14u, 2u, span.instructions != 0u);
    uint32_t source = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    source |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    source = (source + registers->y) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa16u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
double_step: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa16u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x0080u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a << 1), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaa17u);
}
store_step: {
    const uint32_t address = (0x7fe1ceu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa17u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa1bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_descriptor_byte: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa1bu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint8_t)(registers->y + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaa1cu);
}
widen_source: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa1cu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xaa1eu);
}
load_actor_record: {
    const uint32_t address = (uint16_t)(registers->dp + 0xabu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa1eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa20u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_source_offset: {
    const uint16_t pointer_low = (uint16_t)(registers->dp + 0x5du);
    const uint16_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint16_t pointer_bank = (uint16_t)(pointer_low + 2u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(pointer_low) ||
        !Lufia2DecompIsWramByte(pointer_high) || !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t pointer = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                             ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (pointer + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!Lufia2DecompIsDecompressSourceByte(cart, address) || !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa20u, 2u, span.instructions != 0u);
    uint32_t source = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    source |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    source = (source + registers->y) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa22u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_source_offset: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x12b9u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + (!registers->xf || ((0x12b9u >> 8) != ((0x12b9u + registers->x) >> 8)));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa22u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa25u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
source_bank_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa25u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint8_t)(registers->y + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaa26u);
}
source_bank_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa26u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint8_t)(registers->y + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaa27u);
}
narrow_source_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa27u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xaa29u);
}
read_source_bank: {
    const uint16_t pointer_low = (uint16_t)(registers->dp + 0x5du);
    const uint16_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint16_t pointer_bank = (uint16_t)(pointer_low + 2u);
    const unsigned cycles = 6u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(pointer_low) ||
        !Lufia2DecompIsWramByte(pointer_high) || !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t pointer = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                             ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (pointer + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa29u, 2u, span.instructions != 0u);
    uint32_t source = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    source |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    source = (source + registers->y) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa2bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_source_bank: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x12bbu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + (!registers->xf || ((0x12bbu >> 8) != ((0x12bbu + registers->x) >> 8)));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa2bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa2eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_bank: {
    const bool wide = false;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u ||
        registers->sp > (wide ? 0x1ffdu : 0x1ffeu))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa2eu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->db = value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa2fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
return_long: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83aa2fu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83aa2fu, 0x6bu, true, registers->mf, true);
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
static int Lufia2DecompRunInventorySpriteSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                              uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                              unsigned step_budget, long *step_count) {
    if ((pc >> 16) == 0x82u)
        return Lufia2DecompRunMenuEquipmentSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                step_budget, step_count);
    if (pc < 0x83a9e5u)
        return Lufia2DecompRunActorLoadSprite(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                              step_budget, step_count);
    return Lufia2DecompRunActorSpriteDescriptor(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                step_budget, step_count);
}
