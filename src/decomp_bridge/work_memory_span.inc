#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunSpriteAllocationReset(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                unsigned step_budget, long *step_count) {
    if (pc < 0x83ab61u || pc > 0x83ab7bu || !cpu || !cpu->ram || registers->e || !step_budget ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x83ab61u, 0x83ab7cu))
        return 0;
    if (pc >= 0x83ab65u && pc < 0x83ab79u && !registers->xf)
        return 0;
    if (pc >= 0x83ab65u && pc <= 0x83ab68u && !registers->mf)
        return 0;
    if (pc >= 0x83ab6bu && pc < 0x83ab79u && registers->mf)
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x83ab61u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 27u || memcmp(cart->rom + first,
                                              "\x08\x8b\xe2\x30\xa9\x7e\x48\xab\xc2\x20\xa2\x7c\x9e\x00\xe1"
                                              "\x9e\x02\xe1\xca\xca\xca\xca\x10\xf4\xab\x28\x60",
                                              27u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83ab61u: goto save_status;
    case 0x83ab62u: goto save_bank;
    case 0x83ab63u: goto narrow_registers;
    case 0x83ab65u: goto load_work_bank;
    case 0x83ab67u: goto push_work_bank;
    case 0x83ab68u: goto select_work_bank;
    case 0x83ab69u: goto widen_accumulator;
    case 0x83ab6bu: goto load_last_slot;
    case 0x83ab6du: goto clear_flags;
    case 0x83ab70u: goto clear_tile;
    case 0x83ab73u: goto previous_slot_1;
    case 0x83ab74u: goto previous_slot_2;
    case 0x83ab75u: goto previous_slot_3;
    case 0x83ab76u: goto previous_slot_4;
    case 0x83ab77u: goto repeat_slot;
    case 0x83ab79u: goto restore_bank;
    case 0x83ab7au: goto restore_status;
    case 0x83ab7bu: goto return_short;
    default: return 0;
    }
save_status:
    pc = 0x83ab61u;
    goto push_byte;
save_bank:
    pc = 0x83ab62u;
    goto push_byte;
push_work_bank:
    pc = 0x83ab67u;
push_byte:
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f00u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, pc, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--,
                     pc == 0x83ab61u   ? interp816_getFlags(registers)
                     : pc == 0x83ab62u ? registers->db
                                       : (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 3u, (uint16_t)(pc + 1u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    if (pc == 0x83ab61u)
        goto save_bank;
    if (pc == 0x83ab67u)
        goto select_work_bank;
narrow_registers:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab63u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x30u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xab65u);
load_work_bank:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab65u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x7eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xab67u);
    goto push_work_bank;
select_work_bank:
    pc = 0x83ab68u;
    goto pull_bank;
restore_bank:
    pc = 0x83ab79u;
pull_bank:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, pc, 1u, span.instructions != 0u);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->n = (registers->db & 0x80u) != 0u;
    registers->z = registers->db == 0u;
    NativeSpanStep(&span, registers, cpu, 4u, (uint16_t)(pc + 1u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    if (pc == 0x83ab79u)
        goto restore_status;
widen_accumulator:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab69u, 2u, span.instructions != 0u);
    registers->mf = false;
    NativeSpanStep(&span, registers, cpu, 3u, 0xab6bu);
load_last_slot:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab6bu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x7cu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xab6du);
clear_flags:
    pc = 0x83ab6du;
    goto clear_word;
clear_tile:
    pc = 0x83ab70u;
clear_word: {
    const uint16_t base = pc == 0x83ab6du ? 0xe100u : 0xe102u;
    const uint32_t address = (((uint32_t)registers->db << 16) + base + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + (((base + registers->x) >> 8) != (base >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, pc, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, true);
    NativeSpanStep(&span, registers, cpu, cycles, (uint16_t)(pc + 3u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    if (pc == 0x83ab6du)
        goto clear_tile;
}
previous_slot_1:
    pc = 0x83ab73u;
    goto decrease_slot;
previous_slot_2:
    pc = 0x83ab74u;
    goto decrease_slot;
previous_slot_3:
    pc = 0x83ab75u;
    goto decrease_slot;
previous_slot_4:
    pc = 0x83ab76u;
decrease_slot:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, pc, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint8_t)(registers->x - 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(pc + 1u));
    if (pc == 0x83ab73u)
        goto previous_slot_2;
    if (pc == 0x83ab74u)
        goto previous_slot_3;
    if (pc == 0x83ab75u)
        goto previous_slot_4;
repeat_slot: {
    const unsigned cycles = registers->n ? 2u : 3u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab77u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, registers->n ? 0xab79u : 0xab6du);
    if (!registers->n)
        goto clear_flags;
}
    goto restore_bank;
restore_status:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab7au, 1u, span.instructions != 0u);
    interp816_setFlags(registers, registers->read(registers->mem, ++registers->sp));
    NativeSpanStep(&span, registers, cpu, 4u, 0xab7bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
return_short:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab7bu, 1u, span.instructions != 0u);
    {
        uint16_t address = registers->read(registers->mem, ++registers->sp);
        address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        NativeSpanStep(&span, registers, cpu, 6u, (uint16_t)(address + 1u));
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
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
static int Lufia2DecompRunMenuActionCopy(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                         uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget,
                                         long *step_count) {
    if (pc < 0x85d7a8u || pc > 0x85d7bau || !cpu || !cpu->ram || registers->e || !registers->mf ||
        registers->xf || !step_budget || registers->waiting || registers->stopped || registers->nmiWanted ||
        registers->irqWanted || !Lufia2DecompFrameRangeQuiet(0x85d7a8u, 0x85d7bbu))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x85d7a8u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 19u ||
        memcmp(cart->rom + first,
               "\xa0\x87\x00\xb7\xb5\x97\xb8\x88\x10\xf9\xa5\xca\x8f\x50\xf4\x7f\x20\xce\xd7", 19u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x85d7a8u: goto load_last_byte;
    case 0x85d7abu: goto load_byte;
    case 0x85d7adu: goto store_byte;
    case 0x85d7afu: goto previous_byte;
    case 0x85d7b0u: goto repeat_byte;
    case 0x85d7b2u: goto load_actor_mask;
    case 0x85d7b4u: goto publish_actor_mask;
    case 0x85d7b8u: goto call_target_check;
    default: return 0;
    }
load_last_byte:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x85d7a8u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 135u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xd7abu);
load_byte:
    pc = 0x85d7abu;
    goto indirect_byte;
store_byte:
    pc = 0x85d7adu;
indirect_byte: {
    const uint16_t base = (uint16_t)(registers->dp + (pc == 0x85d7abu ? 0xb5u : 0xb8u));
    const uint16_t high = (uint16_t)(base + 1u);
    const uint16_t bank = (uint16_t)(base + 2u);
    if (!NativeSpanCanStep(&span, cpu, 6u + direct_penalty) || !Lufia2DecompIsWramByte(base) ||
        !Lufia2DecompIsWramByte(high) || !Lufia2DecompIsWramByte(bank))
        goto finished;
    const uint32_t expected =
        cpu->ram[base] | ((uint32_t)cpu->ram[high] << 8) | ((uint32_t)cpu->ram[bank] << 16);
    const uint32_t address = (expected + registers->y) & 0xffffffu;
    if (pc == 0x85d7adu ? !Lufia2DecompIsWramByte(address)
                        : !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, pc, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, base, high, true);
    pointer |= (uint32_t)registers->read(registers->mem, bank) << 16;
    pointer = (pointer + registers->y) & 0xffffffu;
    if (pc == 0x85d7abu)
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, pointer), true);
    else
        registers->write(registers->mem, pointer, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 6u + direct_penalty, (uint16_t)(pc + 2u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    if (pc == 0x85d7abu)
        goto store_byte;
}
previous_byte:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x85d7afu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xd7b0u);
repeat_byte: {
    const unsigned cycles = registers->n ? 2u : 3u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x85d7b0u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, registers->n ? 0xd7b2u : 0xd7abu);
    if (!registers->n)
        goto load_byte;
}
load_actor_mask: {
    const uint16_t address = (uint16_t)(registers->dp + 0xcau);
    if (!NativeSpanCanStep(&span, cpu, 3u + direct_penalty) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x85d7b2u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
    NativeSpanStep(&span, registers, cpu, 3u + direct_penalty, 0xd7b4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
publish_actor_mask:
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x85d7b4u, 4u, span.instructions != 0u);
    registers->write(registers->mem, 0x7ff450u, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 5u, 0xd7b8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
call_target_check:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    {
        if (span.instructions)
            Lufia2SpanProbeOpcode(registers, 0x85d7b8u);
        const int cycles =
            Lufia2DecompRunDirectCallWithWidths(registers, 0x85d7b8u, 0x20u, 0x85d7ceu, false, true, false);
        if (!cycles)
            goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
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
static int Lufia2DecompRunWorkMemorySpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                         uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget,
                                         long *step_count) {
    if (pc < 0x850000u) {
        if (pc < 0x83ab61u)
            return Lufia2DecompRunActorRecordOffsets(registers, cpu, pc, opcode,
                dynamic_epoch, write_epoch, step_budget, step_count);
        return Lufia2DecompRunSpriteAllocationReset(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                    step_budget, step_count);
    }
    if (pc < 0x860000u)
        return Lufia2DecompRunMenuActionCopy(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, step_budget,
                                             step_count);
    return Lufia2DecompRunSpriteSetAnimation(registers, cpu, pc, opcode,
        dynamic_epoch, write_epoch, step_budget, step_count);
}
