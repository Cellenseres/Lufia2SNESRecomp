#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunDecompressLiteral(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                            uint64_t dynamic_epoch, uint64_t write_epoch,
                                            unsigned step_budget, long *step_count) {
    uint32_t first_pc, end_pc;
    if (pc >= 0x808ef4u && pc < 0x808efau) {
        first_pc = 0x808ef4u;
        end_pc = 0x808efau;
    } else if (pc >= 0x808efeu && pc < 0x808f10u) {
        first_pc = 0x808efeu;
        end_pc = 0x808f10u;
    } else if (pc >= 0x808f10u && pc < 0x808f14u) {
        first_pc = 0x808f10u;
        end_pc = 0x808f14u;
    } else if (pc >= 0x808f14u && pc < 0x808f21u) {
        first_pc = 0x808f14u;
        end_pc = 0x808f21u;
    } else if (pc >= 0x808f21u && pc < 0x808f24u) {
        first_pc = 0x808f21u;
        end_pc = 0x808f24u;
    } else if (pc >= 0x808f24u && pc < 0x808f2bu) {
        first_pc = 0x808f24u;
        end_pc = 0x808f2bu;
    } else if (pc >= 0x808f2bu && pc < 0x808f32u) {
        first_pc = 0x808f2bu;
        end_pc = 0x808f32u;
    } else
        return 0;
    const bool restoring = first_pc == 0x808f21u;
    if (!cpu || !cpu->ram || registers->e || !step_budget ||
        (!restoring && (!registers->mf || registers->xf)) ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - first_pc;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + end_pc - first_pc)
        return 0;
    switch (first_pc) {
    case 0x808ef4u:
        if (memcmp(cart->rom + first, "\x85\x65\xa9\x08\x85\x66", 6u))
            return 0;
        break;
    case 0x808efeu:
        if (memcmp(cart->rom + first,
                   "\x9d\x00\x00\xe8\xe4\x63\xf0\x1b\xc8\xd0\xf1\xa0\x00\x80\xe6\x5f\x80\xea", 18u))
            return 0;
        break;
    case 0x808f10u:
        if (memcmp(cart->rom + first, "\x9d\x00\x00\xe8", 4u))
            return 0;
        break;
    case 0x808f14u:
        if (memcmp(cart->rom + first, "\xe4\x63\xf0\x09\xc8\xf0\x10\xc6\x66\xd0\xdb\x80\xce", 13u))
            return 0;
        break;
    case 0x808f21u:
        if (memcmp(cart->rom + first, "\xab\x28\x6b", 3u))
            return 0;
        break;
    case 0x808f24u:
        if (memcmp(cart->rom + first, "\xa0\x00\x80\xe6\x5f\x80\xc9", 7u))
            return 0;
        break;
    case 0x808f2bu:
        if (memcmp(cart->rom + first, "\xa0\x00\x80\xe6\x5f\x80\xe9", 7u))
            return 0;
        break;
    default: return 0;
    }
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x808ef4u: goto store_flags;
    case 0x808ef6u: goto load_flag_count;
    case 0x808ef8u: goto store_flag_count;
    case 0x808efeu: goto store_symbol;
    case 0x808f01u: goto advance_symbol_destination;
    case 0x808f02u: goto compare_symbol_end;
    case 0x808f04u: goto finish_symbol;
    case 0x808f06u: goto advance_symbol_source;
    case 0x808f07u: goto repeat_symbol;
    case 0x808f09u: goto reset_symbol_source;
    case 0x808f0cu: goto advance_symbol_bank;
    case 0x808f0eu: goto resume_symbol;
    case 0x808f10u: goto store_flag_literal;
    case 0x808f13u: goto advance_flag_destination;
    case 0x808f14u: goto compare_flag_end;
    case 0x808f16u: goto finish_flag;
    case 0x808f18u: goto advance_flag_source;
    case 0x808f19u: goto wrap_flag_source;
    case 0x808f1bu: goto count_flags;
    case 0x808f1du: goto repeat_flag;
    case 0x808f1fu: goto resume_control;
    case 0x808f21u: goto restore_bank;
    case 0x808f22u: goto restore_status;
    case 0x808f23u: goto return_long;
    case 0x808f24u: goto reset_control_source;
    case 0x808f27u: goto advance_control_bank;
    case 0x808f29u: goto resume_control_suffix;
    case 0x808f2bu: goto reset_flag_source;
    case 0x808f2eu: goto advance_flag_bank;
    case 0x808f30u: goto resume_flag_counter;
    default: return 0;
    }
store_flags: {
    const uint16_t low = (uint16_t)(registers->dp + 0x65u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 3u + direct_penalty) || !Lufia2DecompIsWramByte(low))
        goto finished;
    Lufia2SpanFetch(registers, 0x808ef4u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 3u + direct_penalty, 0x8ef6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_flag_count:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808ef6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0008u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8ef8u);
store_flag_count: {
    const uint16_t low = (uint16_t)(registers->dp + 0x66u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 3u + direct_penalty) || !Lufia2DecompIsWramByte(low))
        goto finished;
    Lufia2SpanFetch(registers, 0x808ef8u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, low, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 3u + direct_penalty, 0x8efau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
    goto finished;
store_symbol: {
    const uint32_t address = (((uint32_t)registers->db << 16) + registers->x) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 6u) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x808efeu, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 6u, 0x8f01u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
advance_symbol_destination:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f01u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8f02u);
compare_symbol_end: {
    const uint16_t low = (uint16_t)(registers->dp + 0x63u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f02u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, Lufia2DecompReadData(registers, low, high, true), false);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0x8f04u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
finish_symbol: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f04u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8f21u : 0x8f06u);
    if (taken)
        goto finished;
}
advance_symbol_source:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f06u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8f07u);
repeat_symbol: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f07u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8efau : 0x8f09u);
    if (taken)
        goto finished;
}
reset_symbol_source:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f09u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x8000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8f0cu);
advance_symbol_bank: {
    const uint16_t low = (uint16_t)(registers->dp + 0x5fu);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 5u + direct_penalty) || !Lufia2DecompIsWramByte(low))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f0cu, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, low, high, false);
    NativeSpanStep(&span, registers, cpu, 5u + direct_penalty, 0x8f0eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
resume_symbol:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f0eu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8efau);
    goto finished;
store_flag_literal: {
    const uint32_t address = (((uint32_t)registers->db << 16) + registers->x) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 6u) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f10u, 3u, span.instructions != 0u);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 6u, 0x8f13u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
advance_flag_destination:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f13u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8f14u);
    goto finished;
compare_flag_end: {
    const uint16_t low = (uint16_t)(registers->dp + 0x63u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) || !Lufia2DecompIsWramByte(low) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f14u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, Lufia2DecompReadData(registers, low, high, true), false);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0x8f16u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
finish_flag: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f16u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8f21u : 0x8f18u);
    if (taken)
        goto finished;
}
advance_flag_source:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f18u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8f19u);
wrap_flag_source: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f19u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8f2bu : 0x8f1bu);
    if (taken)
        goto finished;
}
count_flags: {
    const uint16_t low = (uint16_t)(registers->dp + 0x66u);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 5u + direct_penalty) || !Lufia2DecompIsWramByte(low))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f1bu, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, low, high, false);
    NativeSpanStep(&span, registers, cpu, 5u + direct_penalty, 0x8f1du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_flag: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f1du, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8efau : 0x8f1fu);
    if (taken)
        goto finished;
}
resume_control:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f1fu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8eefu);
    goto finished;
restore_bank:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x808f21u, 1u, span.instructions != 0u);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x8f22u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
restore_status:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x808f22u, 1u, span.instructions != 0u);
    interp816_setFlags(registers, registers->read(registers->mem, ++registers->sp));
    NativeSpanStep(&span, registers, cpu, 4u, 0x8f23u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
return_long:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    Lufia2SpanFetch(registers, 0x808f23u, 1u, span.instructions != 0u);
    {
        uint16_t address = registers->read(registers->mem, ++registers->sp);
        address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        registers->k = registers->read(registers->mem, ++registers->sp);
        NativeSpanStep(&span, registers, cpu, 6u, (uint16_t)(address + 1u));
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
reset_control_source:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f24u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x8000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8f27u);
advance_control_bank: {
    const uint16_t low = (uint16_t)(registers->dp + 0x5fu);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 5u + direct_penalty) || !Lufia2DecompIsWramByte(low))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f27u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, low, high, false);
    NativeSpanStep(&span, registers, cpu, 5u + direct_penalty, 0x8f29u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
resume_control_suffix:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f29u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8ef4u);
    goto finished;
reset_flag_source:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f2bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x8000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8f2eu);
advance_flag_bank: {
    const uint16_t low = (uint16_t)(registers->dp + 0x5fu);
    const uint16_t high = (uint16_t)(low + 1u);
    if (!NativeSpanCanStep(&span, cpu, 5u + direct_penalty) || !Lufia2DecompIsWramByte(low))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f2eu, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, low, high, false);
    NativeSpanStep(&span, registers, cpu, 5u + direct_penalty, 0x8f30u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
resume_flag_counter:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808f30u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8f1bu);
    goto finished;
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
