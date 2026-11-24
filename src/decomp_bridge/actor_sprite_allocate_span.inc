#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunActorAllocateSprite(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                              uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                              unsigned step_budget, long *step_count) {
    const uint32_t phase_end = pc < 0x83aa5fu   ? 0x83aa5fu
                               : pc < 0x83aa6eu ? 0x83aa6eu
                               : pc < 0x83aa77u ? 0x83aa77u
                               : pc < 0x83aa7bu ? 0x83aa7bu
                                                : 0x83aa7du;
    if (pc < 0x83aa50u || pc >= 0x83aa7du || !cpu || !cpu->ram || registers->e || !step_budget ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        (pc + 1u < phase_end && !Lufia2DecompFrameRangeQuiet(pc + 1u, phase_end)))
        return 0;
    const bool any_width = pc <= 0x83aa51u || pc >= 0x83aa7bu;
    const bool byte_accumulator = pc < 0x83aa72u;
    if (!any_width && (!registers->xf || registers->mf != byte_accumulator))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x83aa50u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 45u ||
        memcmp(cart->rom + first,
               "\x08\xe2\x30\xa6\xa7\x7b\xbf\x16\xe2\x7f\xaa\xbf\xf4\xab\x83\x22\x7c\xab\x83\xa6\xa7\x9f\x5e"
               "\xe2\x7f\xeb\x9f\xa6\xe2\x7f\x22\xe9\xab\x83\xa4\xa9\x99\x31\x13\x22\x7d\xaa\x83\x28\x60",
               45u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83aa50u: goto save_status;
    case 0x83aa51u: goto narrow_registers;
    case 0x83aa53u: goto load_actor_slot;
    case 0x83aa55u: goto clear_accumulator_from_page;
    case 0x83aa56u: goto read_type;
    case 0x83aa5au: goto index_type;
    case 0x83aa5bu: goto read_slot_count;
    case 0x83aa5fu: goto allocate_slots;
    case 0x83aa63u: goto reload_actor_slot;
    case 0x83aa65u: goto store_slot_low;
    case 0x83aa69u: goto exchange_slot_bytes;
    case 0x83aa6au: goto store_slot_high;
    case 0x83aa6eu: goto compute_vram_base;
    case 0x83aa72u: goto load_actor_word_offset;
    case 0x83aa74u: goto store_vram_base;
    case 0x83aa77u: goto select_tables;
    case 0x83aa7bu: goto restore_status;
    case 0x83aa7cu: goto return_short;
    default: return 0;
    }
save_status: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f00u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa50u, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, (uint8_t)interp816_getFlags(registers));
    NativeSpanStep(&span, registers, cpu, 3u, 0xaa51u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_registers: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa51u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x30u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xaa53u);
}
load_actor_slot: {
    const bool wide = !registers->xf;
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa53u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa55u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_accumulator_from_page: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa55u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->dp, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaa56u);
}
read_type: {
    const bool wide = false;
    const uint32_t address = (0x7fe216u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa56u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa5au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
index_type: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa5au, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->xf ? (uint8_t)registers->a : registers->a,
                             registers->xf);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaa5bu);
}
read_slot_count: {
    const bool wide = false;
    const uint32_t address = (0x83abf4u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa5bu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa5fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
allocate_slots: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83aa5fu);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83aa5fu, 0x22u, 0x83ab7cu, true,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
reload_actor_slot: {
    const bool wide = !registers->xf;
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa63u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa65u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_slot_low: {
    const bool wide = false;
    const uint32_t address = (0x7fe25eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa65u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa69u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
exchange_slot_bytes: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa69u, 1u, span.instructions != 0u);
    registers->a = (uint16_t)((registers->a >> 8) | (registers->a << 8));
    registers->z = (registers->a & 0xffu) == 0u;
    registers->n = (registers->a & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 3u, 0xaa6au);
}
store_slot_high: {
    const bool wide = false;
    const uint32_t address = (0x7fe2a6u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa6au, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa6eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
compute_vram_base: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83aa6eu);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83aa6eu, 0x22u, 0x83abe9u, true,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
load_actor_word_offset: {
    const bool wide = !registers->xf;
    const uint32_t address = (uint16_t)(registers->dp + 0xa9u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa72u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa74u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_vram_base: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1331u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const bool crossed = (0x1331u >> 8) != ((0x1331u + registers->y) >> 8);
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa74u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa77u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
select_tables: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83aa77u);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83aa77u, 0x22u, 0x83aa7du, true,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_status: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa7bu, 1u, span.instructions != 0u);
    interp816_setFlags(registers, registers->read(registers->mem, ++registers->sp));
    NativeSpanStep(&span, registers, cpu, 4u, 0xaa7cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
return_short: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83aa7cu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83aa7cu, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunActorSpriteTables(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                            uint64_t dynamic_epoch, uint64_t write_epoch,
                                            unsigned step_budget, long *step_count) {
    if (pc < 0x83aa7du || pc >= 0x83aaafu || !cpu || !cpu->ram || registers->e || !step_budget ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x83aa7eu, 0x83aaafu))
        return 0;
    const bool any_width =
        pc <= 0x83aa7eu || pc >= 0x83aaadu || pc == 0x83aa88u || pc == 0x83aa91u || pc == 0x83aaa2u;
    const bool byte_accumulator = pc < 0x83aa8au || (pc >= 0x83aa93u && pc <= 0x83aaa2u);
    if (!any_width && registers->mf != byte_accumulator)
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x83aa7du;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 50u ||
        memcmp(cart->rom + first,
               "\x08\xe2\x20\xa6\xa7\xbf\x16\xe2\x7f\x0a\xaa\xc2\x20\xbf\xfc\xab\x83\x99\x81\x13\xe2\x20\xa4"
               "\xa7\xa9\xff\x99\x71\x14\xb9\x91\x12\x29\x18\x4a\x4a\xaa\xc2\x20\xbf\x14\xac\x83\xa4\xa9\x99"
               "\xd1\x13\x28\x6b",
               50u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83aa7du: goto save_status;
    case 0x83aa7eu: goto narrow_type;
    case 0x83aa80u: goto load_actor_slot;
    case 0x83aa82u: goto read_type;
    case 0x83aa86u: goto double_type;
    case 0x83aa87u: goto index_animation;
    case 0x83aa88u: goto widen_animation;
    case 0x83aa8au: goto read_animation_table;
    case 0x83aa8eu: goto store_animation_table;
    case 0x83aa91u: goto narrow_flags;
    case 0x83aa93u: goto reload_actor_slot;
    case 0x83aa95u: goto load_frame_sentinel;
    case 0x83aa97u: goto store_frame_sentinel;
    case 0x83aa9au: goto read_flags;
    case 0x83aa9du: goto mask_motion;
    case 0x83aa9fu: goto halve_motion_once;
    case 0x83aaa0u: goto halve_motion_twice;
    case 0x83aaa1u: goto index_motion;
    case 0x83aaa2u: goto widen_motion;
    case 0x83aaa4u: goto read_motion_table;
    case 0x83aaa8u: goto load_actor_word_offset;
    case 0x83aaaau: goto store_motion_table;
    case 0x83aaadu: goto restore_status;
    case 0x83aaaeu: goto return_long;
    default: return 0;
    }
save_status: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f00u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa7du, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, (uint8_t)interp816_getFlags(registers));
    NativeSpanStep(&span, registers, cpu, 3u, 0xaa7eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_type: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa7eu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xaa80u);
}
load_actor_slot: {
    const bool wide = !registers->xf;
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa80u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa82u);
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
    Lufia2SpanFetch(registers, 0x83aa82u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa86u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
double_type: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa86u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x0080u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a << 1), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaa87u);
}
index_animation: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa87u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->xf ? (uint8_t)registers->a : registers->a,
                             registers->xf);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaa88u);
}
widen_animation: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa88u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xaa8au);
}
read_animation_table: {
    const bool wide = true;
    const uint32_t address = (0x83abfcu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa8au, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa8eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_animation_table: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1381u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const bool crossed = (0x1381u >> 8) != ((0x1381u + registers->y) >> 8);
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa8eu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa91u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_flags: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa91u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xaa93u);
}
reload_actor_slot: {
    const bool wide = !registers->xf;
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa93u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa95u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_frame_sentinel: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa95u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaa97u);
}
store_frame_sentinel: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1471u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const bool crossed = (0x1471u >> 8) != ((0x1471u + registers->y) >> 8);
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa97u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa9au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_flags: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1291u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const bool crossed = (0x1291u >> 8) != ((0x1291u + registers->y) >> 8);
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa9au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaa9du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_motion: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa9du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x0018u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaa9fu);
}
halve_motion_once: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aa9fu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)registers->a >> 1, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaaa0u);
}
halve_motion_twice: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aaa0u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)registers->a >> 1, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaaa1u);
}
index_motion: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aaa1u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->xf ? (uint8_t)registers->a : registers->a,
                             registers->xf);
    NativeSpanStep(&span, registers, cpu, 2u, 0xaaa2u);
}
widen_motion: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aaa2u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xaaa4u);
}
read_motion_table: {
    const bool wide = true;
    const uint32_t address = (0x83ac14u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aaa4u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaaa8u);
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
    Lufia2SpanFetch(registers, 0x83aaa8u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaaaau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_motion_table: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13d1u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const bool crossed = (0x13d1u >> 8) != ((0x13d1u + registers->y) >> 8);
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83aaaau, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xaaadu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_status: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83aaadu, 1u, span.instructions != 0u);
    interp816_setFlags(registers, registers->read(registers->mem, ++registers->sp));
    NativeSpanStep(&span, registers, cpu, 4u, 0xaaaeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
return_long: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83aaaeu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83aaaeu, 0x6bu, true, registers->mf, true);
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
static int Lufia2DecompRunSpriteVramBase(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                         uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget,
                                         long *step_count) {
    if (pc < 0x83abe9u || pc >= 0x83abf4u || !cpu || !cpu->ram || registers->e || !step_budget ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x83abeau, 0x83abf4u))
        return 0;
    if (pc >= 0x83abecu && pc < 0x83abf3u && registers->mf)
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x83abe9u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 11u ||
        memcmp(cart->rom + first, "\xeb\xc2\x20\x0a\x0a\x0a\x0a\x69\x00\x20\x6b", 11u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83abe9u: goto exchange_slot_bytes;
    case 0x83abeau: goto widen_base;
    case 0x83abecu: goto scale_slot_once;
    case 0x83abedu: goto scale_slot_twice;
    case 0x83abeeu: goto scale_slot_thrice;
    case 0x83abefu: goto scale_slot_fourfold;
    case 0x83abf0u: goto add_vram_origin;
    case 0x83abf3u: goto return_long;
    default: return 0;
    }
exchange_slot_bytes: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83abe9u, 1u, span.instructions != 0u);
    registers->a = (uint16_t)((registers->a >> 8) | (registers->a << 8));
    registers->z = (registers->a & 0xffu) == 0u;
    registers->n = (registers->a & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 3u, 0xabeau);
}
widen_base: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83abeau, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xabecu);
}
scale_slot_once: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83abecu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xabedu);
}
scale_slot_twice: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83abedu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xabeeu);
}
scale_slot_thrice: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83abeeu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xabefu);
}
scale_slot_fourfold: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83abefu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xabf0u);
}
add_vram_origin: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83abf0u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x2000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xabf3u);
}
return_long: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83abf3u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83abf3u, 0x6bu, true, registers->mf, true);
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
static int Lufia2DecompRunActorSpriteAllocateSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                  uint8_t opcode, uint64_t dynamic_epoch,
                                                  uint64_t write_epoch, unsigned step_budget,
                                                  long *step_count) {
    if (pc >= 0x83abe9u)
        return Lufia2DecompRunSpriteVramBase(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                             step_budget, step_count);
    if (pc < 0x83aa7du)
        return Lufia2DecompRunActorAllocateSprite(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                  step_budget, step_count);
    return Lufia2DecompRunActorSpriteTables(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                            step_budget, step_count);
}
