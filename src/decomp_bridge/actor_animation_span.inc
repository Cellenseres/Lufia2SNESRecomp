#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunActorRecordOffsets(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                             uint64_t dynamic_epoch, uint64_t write_epoch,
                                             unsigned step_budget, long *step_count) {
    if (pc < 0x83ab4fu || pc >= 0x83ab61u || !cpu || !cpu->ram || registers->e || !step_budget ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        (pc + 1u < 0x83ab61u && !Lufia2DecompFrameRangeQuiet(pc + 1u, 0x83ab61u)))
        return 0;
    if (pc != 0x83ab60u && !registers->mf)
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x83ab4fu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 18u ||
        memcmp(cart->rom + first, "\x64\xa8\xa5\xa7\x0a\x85\xa9\x64\xaa\x65\xa7\x85\xab\x64\xac\x64\xad\x6b",
               18u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    uint8_t displacement;
    unsigned operation;
    switch (pc) {
    case 0x83ab4fu: goto clear_actor_high;
    case 0x83ab51u: goto load_actor_id;
    case 0x83ab53u: goto double_actor_id;
    case 0x83ab54u: goto store_word_offset;
    case 0x83ab56u: goto clear_word_high;
    case 0x83ab58u: goto add_actor_id;
    case 0x83ab5au: goto store_record_offset;
    case 0x83ab5cu: goto clear_record_high;
    case 0x83ab5eu: goto clear_record_bank;
    case 0x83ab60u: goto return_long;
    default: return 0;
    }
clear_actor_high:
    pc = 0x83ab4fu;
    displacement = 0xa8u;
    operation = 0u;
    goto direct_byte;
load_actor_id:
    pc = 0x83ab51u;
    displacement = 0xa7u;
    operation = 1u;
    goto direct_byte;
double_actor_id:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab53u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xab54u);
    goto store_word_offset;
store_word_offset:
    pc = 0x83ab54u;
    displacement = 0xa9u;
    operation = 2u;
    goto direct_byte;
clear_word_high:
    pc = 0x83ab56u;
    displacement = 0xaau;
    operation = 0u;
    goto direct_byte;
add_actor_id:
    pc = 0x83ab58u;
    displacement = 0xa7u;
    operation = 3u;
    goto direct_byte;
store_record_offset:
    pc = 0x83ab5au;
    displacement = 0xabu;
    operation = 2u;
    goto direct_byte;
clear_record_high:
    pc = 0x83ab5cu;
    displacement = 0xacu;
    operation = 0u;
    goto direct_byte;
clear_record_bank:
    pc = 0x83ab5eu;
    displacement = 0xadu;
    operation = 0u;
    goto direct_byte;
return_long:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    {
        if (span.instructions)
            Lufia2SpanProbeOpcode(registers, 0x83ab60u);
        const int cycles = Lufia2DecompRunReturn(registers, 0x83ab60u, 0x6bu, true, registers->mf, true);
        if (!cycles)
            goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
direct_byte: {
    const uint16_t address = (uint16_t)(registers->dp + displacement);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, pc, 2u, span.instructions != 0u);
    if (operation == 1u || operation == 3u)
        Lufia2DecompApplyOperand(registers, operation == 1u ? 0xa9u : 0x69u,
                                 registers->read(registers->mem, address), true);
    else
        registers->write(registers->mem, address, operation == 0u ? 0u : (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, (uint16_t)(pc + 2u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    switch (pc) {
    case 0x83ab4fu: goto load_actor_id;
    case 0x83ab51u: goto double_actor_id;
    case 0x83ab54u: goto clear_word_high;
    case 0x83ab56u: goto add_actor_id;
    case 0x83ab58u: goto store_record_offset;
    case 0x83ab5au: goto clear_record_high;
    case 0x83ab5cu: goto clear_record_bank;
    case 0x83ab5eu: goto return_long;
    default: goto finished;
    }
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
static int Lufia2DecompRunSpriteSetAnimation(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                             uint64_t dynamic_epoch, uint64_t write_epoch,
                                             unsigned step_budget, long *step_count) {
    if (pc < 0x868cf5u || pc >= 0x868d47u || !cpu || !cpu->ram || registers->e || !step_budget ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        (pc + 1u < 0x868d47u && !Lufia2DecompFrameRangeQuiet(pc + 1u, 0x868d47u)))
        return 0;
    const bool changing_width = pc == 0x868cf9u || pc == 0x868d00u;
    const bool byte_entry = pc < 0x868cfbu || pc > 0x868d00u;
    if (registers->xf || (!changing_width && registers->mf != byte_entry))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x868cf5u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 82u ||
        memcmp(cart->rom + first,
               "\x5a\x9d\x08\x12\xc2\x20\x29\xff\x00\x0a\xa8\xe2\x20\xbd\x68\x12\x85\x5d\xbd\x98\x12\x85\x5e"
               "\xbd\x38\x12\x85\x5f\xb7\x5d\x9d\xc8\x12\xc8\xb7\x5d\x9d\xf8\x12\xbd\xc8\x12\x85\x5d\xbd\xf8"
               "\x12\x85\x5e\xbd\x48\x14\x0a\xa8\xb7\x5d\x9d\x28\x13\xc8\xb7\x5d\x9d\x58\x13\xbd\x28\x13\x85"
               "\x5d\xbd\x58\x13\x85\x5e\xa7\x5d\x9d\x78\x14\x7a\x6b",
               82u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    uint16_t absolute_base;
    uint8_t displacement;
    bool store_absolute;
    switch (pc) {
    case 0x868cf5u: goto save_caller_y;
    case 0x868cf6u: goto store_animation;
    case 0x868cf9u: goto widen_animation;
    case 0x868cfbu: goto mask_animation;
    case 0x868cfeu: goto double_animation;
    case 0x868cffu: goto index_animation;
    case 0x868d00u: goto narrow_animation;
    case 0x868d02u: goto load_list_low;
    case 0x868d05u: goto store_list_low;
    case 0x868d07u: goto load_list_high;
    case 0x868d0au: goto store_list_high;
    case 0x868d0cu: goto load_script_bank;
    case 0x868d0fu: goto store_script_bank;
    case 0x868d11u: goto load_script_low;
    case 0x868d13u: goto store_script_low;
    case 0x868d16u: goto next_script_byte;
    case 0x868d17u: goto load_script_high;
    case 0x868d19u: goto store_script_high;
    case 0x868d1cu: goto reload_script_low;
    case 0x868d1fu: goto store_frame_list_low;
    case 0x868d21u: goto reload_script_high;
    case 0x868d24u: goto store_frame_list_high;
    case 0x868d26u: goto load_frame_index;
    case 0x868d29u: goto double_frame_index;
    case 0x868d2au: goto index_frame;
    case 0x868d2bu: goto load_frame_low;
    case 0x868d2du: goto store_frame_low;
    case 0x868d30u: goto next_frame_byte;
    case 0x868d31u: goto load_frame_high;
    case 0x868d33u: goto store_frame_high;
    case 0x868d36u: goto reload_frame_low;
    case 0x868d39u: goto store_timer_low;
    case 0x868d3bu: goto reload_frame_high;
    case 0x868d3eu: goto store_timer_high;
    case 0x868d40u: goto load_timer;
    case 0x868d42u: goto store_timer;
    case 0x868d45u: goto restore_caller_y;
    case 0x868d46u: goto return_long;
    default: return 0;
    }
save_caller_y:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x868cf5u, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, (uint8_t)(registers->y >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->y);
    NativeSpanStep(&span, registers, cpu, 4u, 0x8cf6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
store_animation:
    pc = 0x868cf6u;
    absolute_base = 0x1208u;
    store_absolute = true;
    goto absolute_byte;
widen_animation:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868cf9u, 2u, span.instructions != 0u);
    registers->mf = false;
    NativeSpanStep(&span, registers, cpu, 3u, 0x8cfbu);
mask_animation:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868cfbu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8cfeu);
double_animation:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868cfeu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8cffu);
index_animation:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868cffu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d00u);
narrow_animation:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868d00u, 2u, span.instructions != 0u);
    registers->mf = true;
    NativeSpanStep(&span, registers, cpu, 3u, 0x8d02u);
load_list_low:
    pc = 0x868d02u;
    absolute_base = 0x1268u;
    store_absolute = false;
    goto absolute_byte;
store_list_low:
    pc = 0x868d05u;
    displacement = 0x5du;
    goto store_pointer_byte;
load_list_high:
    pc = 0x868d07u;
    absolute_base = 0x1298u;
    store_absolute = false;
    goto absolute_byte;
store_list_high:
    pc = 0x868d0au;
    displacement = 0x5eu;
    goto store_pointer_byte;
load_script_bank:
    pc = 0x868d0cu;
    absolute_base = 0x1238u;
    store_absolute = false;
    goto absolute_byte;
store_script_bank:
    pc = 0x868d0fu;
    displacement = 0x5fu;
    goto store_pointer_byte;
load_script_low:
    pc = 0x868d11u;
    goto indirect_byte;
store_script_low:
    pc = 0x868d13u;
    absolute_base = 0x12c8u;
    store_absolute = true;
    goto absolute_byte;
next_script_byte:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868d16u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d17u);
load_script_high:
    pc = 0x868d17u;
    goto indirect_byte;
store_script_high:
    pc = 0x868d19u;
    absolute_base = 0x12f8u;
    store_absolute = true;
    goto absolute_byte;
reload_script_low:
    pc = 0x868d1cu;
    absolute_base = 0x12c8u;
    store_absolute = false;
    goto absolute_byte;
store_frame_list_low:
    pc = 0x868d1fu;
    displacement = 0x5du;
    goto store_pointer_byte;
reload_script_high:
    pc = 0x868d21u;
    absolute_base = 0x12f8u;
    store_absolute = false;
    goto absolute_byte;
store_frame_list_high:
    pc = 0x868d24u;
    displacement = 0x5eu;
    goto store_pointer_byte;
load_frame_index:
    pc = 0x868d26u;
    absolute_base = 0x1448u;
    store_absolute = false;
    goto absolute_byte;
double_frame_index:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868d29u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d2au);
index_frame:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868d2au, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d2bu);
load_frame_low:
    pc = 0x868d2bu;
    goto indirect_byte;
store_frame_low:
    pc = 0x868d2du;
    absolute_base = 0x1328u;
    store_absolute = true;
    goto absolute_byte;
next_frame_byte:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868d30u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8d31u);
load_frame_high:
    pc = 0x868d31u;
    goto indirect_byte;
store_frame_high:
    pc = 0x868d33u;
    absolute_base = 0x1358u;
    store_absolute = true;
    goto absolute_byte;
reload_frame_low:
    pc = 0x868d36u;
    absolute_base = 0x1328u;
    store_absolute = false;
    goto absolute_byte;
store_timer_low:
    pc = 0x868d39u;
    displacement = 0x5du;
    goto store_pointer_byte;
reload_frame_high:
    pc = 0x868d3bu;
    absolute_base = 0x1358u;
    store_absolute = false;
    goto absolute_byte;
store_timer_high:
    pc = 0x868d3eu;
    displacement = 0x5eu;
    goto store_pointer_byte;
load_timer:
    pc = 0x868d40u;
    goto indirect_byte;
store_timer:
    pc = 0x868d42u;
    absolute_base = 0x1478u;
    store_absolute = true;
    goto absolute_byte;
restore_caller_y:
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x868d45u, 1u, span.instructions != 0u);
    {
        uint16_t value = registers->read(registers->mem, ++registers->sp);
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    }
    NativeSpanStep(&span, registers, cpu, 5u, 0x8d46u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
return_long:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    {
        if (span.instructions)
            Lufia2SpanProbeOpcode(registers, 0x868d46u);
        const int cycles = Lufia2DecompRunReturn(registers, 0x868d46u, 0x6bu, true, registers->mf, true);
        if (!cycles)
            goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
absolute_byte: {
    const uint32_t address = (((uint32_t)registers->db << 16) + absolute_base + registers->x) & 0xffffffu;
    const bool crossed = (absolute_base >> 8) != ((absolute_base + registers->x) >> 8);
    const unsigned cycles = (store_absolute ? 5u : 4u) + (store_absolute || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, pc, 3u, span.instructions != 0u);
    if (store_absolute)
        registers->write(registers->mem, address, (uint8_t)registers->a);
    else
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, (uint16_t)(pc + 3u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    switch (pc) {
    case 0x868cf6u: goto widen_animation;
    case 0x868d02u: goto store_list_low;
    case 0x868d07u: goto store_list_high;
    case 0x868d0cu: goto store_script_bank;
    case 0x868d13u: goto next_script_byte;
    case 0x868d19u: goto reload_script_low;
    case 0x868d1cu: goto store_frame_list_low;
    case 0x868d21u: goto store_frame_list_high;
    case 0x868d26u: goto double_frame_index;
    case 0x868d2du: goto next_frame_byte;
    case 0x868d33u: goto reload_frame_low;
    case 0x868d36u: goto store_timer_low;
    case 0x868d3bu: goto store_timer_high;
    case 0x868d42u: goto restore_caller_y;
    default: goto finished;
    }
}
store_pointer_byte: {
    const uint16_t address = (uint16_t)(registers->dp + displacement);
    if (!NativeSpanCanStep(&span, cpu, 3u + direct_penalty) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, pc, 2u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 3u + direct_penalty, (uint16_t)(pc + 2u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    switch (pc) {
    case 0x868d05u: goto load_list_high;
    case 0x868d0au: goto load_script_bank;
    case 0x868d0fu: goto load_script_low;
    case 0x868d1fu: goto reload_script_high;
    case 0x868d24u: goto load_frame_index;
    case 0x868d39u: goto reload_frame_high;
    case 0x868d3eu: goto load_timer;
    default: goto finished;
    }
}
indirect_byte: {
    const uint16_t low = (uint16_t)(registers->dp + 0x5du);
    const uint16_t high = (uint16_t)(low + 1u);
    const uint16_t bank = (uint16_t)(low + 2u);
    const unsigned cycles = 6u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high) || !Lufia2DecompIsWramByte(bank))
        goto finished;
    const uint16_t index = pc == 0x868d40u ? 0u : registers->y;
    const uint32_t pointer =
        cpu->ram[low] | ((uint32_t)cpu->ram[high] << 8) | ((uint32_t)cpu->ram[bank] << 16);
    if (!Lufia2DecompIsDecompressSourceByte(cart, (pointer + index) & 0xffffffu))
        goto finished;
    Lufia2SpanFetch(registers, pc, 2u, span.instructions != 0u);
    uint32_t address = Lufia2DecompReadData(registers, low, high, true);
    address |= (uint32_t)registers->read(registers->mem, bank) << 16;
    address = (address + index) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, cycles, (uint16_t)(pc + 2u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    switch (pc) {
    case 0x868d11u: goto store_script_low;
    case 0x868d17u: goto store_script_high;
    case 0x868d2bu: goto store_frame_low;
    case 0x868d31u: goto store_frame_high;
    case 0x868d40u: goto store_timer;
    default: goto finished;
    }
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
