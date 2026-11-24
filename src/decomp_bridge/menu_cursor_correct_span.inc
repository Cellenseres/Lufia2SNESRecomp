static int Lufia2DecompRunMenuCursorCorrect(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    if (pc < 0x828ad8u || pc > 0x828af9u) return 0;
    const uint32_t first_pc = pc < 0x828ae9u ? 0x828ad8u : 0x828ae9u;
    const unsigned phase = pc - first_pc;
    const bool vertical = first_pc == 0x828ae9u;
    if (registers->e || (!registers->mf && phase != 16u) || !step_budget ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(first_pc, first_pc + 17u))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    if (!cart || offset < phase) return 0;
    const unsigned first = offset - phase;
    const char *signature = vertical
        ? "\xb9\xe8\x13\x18\x65\xe6\x99\xe8\x13\xa5\xe3\x18\x65\xe1\x85\xe3\x60"
        : "\xb9\x88\x13\x18\x65\xe5\x99\x88\x13\xa5\xe4\x18\x65\xe2\x85\xe4\x60";
    if (cart->romSize < first + 17u || memcmp(cart->rom + first, signature, 17u))
        return 0;
    const uint16_t slot = vertical ? 0x13e8u : 0x1388u;
    const uint16_t direction = (uint16_t)(registers->dp + (vertical ? 0xe6u : 0xe5u));
    const uint16_t error = (uint16_t)(registers->dp + (vertical ? 0xe3u : 0xe4u));
    const uint16_t span_size = (uint16_t)(registers->dp + (vertical ? 0xe1u : 0xe2u));
    const uint32_t coordinate = (((uint32_t)registers->db << 16) + slot + registers->y) & 0xffffffu;
    const bool crossed = (slot >> 8) != ((slot + registers->y) >> 8);
    const unsigned direct_cycles = 3u + ((registers->dp & 0xffu) != 0u);
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (phase) {
    case 0u: goto load_coordinate;
    case 3u: goto clear_move_carry;
    case 4u: goto add_direction;
    case 6u: goto store_coordinate;
    case 9u: goto load_error;
    case 11u: goto clear_error_carry;
    case 12u: goto add_span;
    case 14u: goto store_error;
    case 16u: goto return_short;
    default: return 0;
    }
load_coordinate:
    if (!NativeSpanCanStep(&span, cpu, 4u + crossed) || !Lufia2DecompIsWramByte(coordinate))
        goto finished;
    Lufia2SpanFetch(registers, first_pc, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, coordinate), true);
    NativeSpanStep(&span, registers, cpu, 4u + crossed, (uint16_t)(first_pc + 3u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
clear_move_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, first_pc + 3u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(first_pc + 4u));
add_direction:
    if (!NativeSpanCanStep(&span, cpu, direct_cycles) || !Lufia2DecompIsWramByte(direction))
        goto finished;
    Lufia2SpanFetch(registers, first_pc + 4u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, registers->read(registers->mem, direction), true);
    NativeSpanStep(&span, registers, cpu, direct_cycles, (uint16_t)(first_pc + 6u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
store_coordinate:
    {
        const unsigned cycles = 5u + (!registers->xf || crossed);
        if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(coordinate))
            goto finished;
        Lufia2SpanFetch(registers, first_pc + 6u, 3u, span.instructions != 0u);
        registers->write(registers->mem, coordinate, (uint8_t)registers->a);
        NativeSpanStep(&span, registers, cpu, cycles, (uint16_t)(first_pc + 9u));
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
load_error:
    if (!NativeSpanCanStep(&span, cpu, direct_cycles) || !Lufia2DecompIsWramByte(error))
        goto finished;
    Lufia2SpanFetch(registers, first_pc + 9u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, error), true);
    NativeSpanStep(&span, registers, cpu, direct_cycles, (uint16_t)(first_pc + 11u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
clear_error_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, first_pc + 11u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(first_pc + 12u));
add_span:
    if (!NativeSpanCanStep(&span, cpu, direct_cycles) || !Lufia2DecompIsWramByte(span_size))
        goto finished;
    Lufia2SpanFetch(registers, first_pc + 12u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, registers->read(registers->mem, span_size), true);
    NativeSpanStep(&span, registers, cpu, direct_cycles, (uint16_t)(first_pc + 14u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
store_error:
    if (!NativeSpanCanStep(&span, cpu, direct_cycles) || !Lufia2DecompIsWramByte(error))
        goto finished;
    Lufia2SpanFetch(registers, first_pc + 14u, 2u, span.instructions != 0u);
    registers->write(registers->mem, error, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, direct_cycles, (uint16_t)(first_pc + 16u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
return_short:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, first_pc + 16u, 1u, span.instructions != 0u);
    {
        uint16_t address = registers->read(registers->mem, ++registers->sp);
        address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        NativeSpanStep(&span, registers, cpu, 6u, (uint16_t)(address + 1u));
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
finished:
    if (!span.instructions) return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
