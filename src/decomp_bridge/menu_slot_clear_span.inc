static int Lufia2DecompRunMenuSlotClear(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    if (pc < 0x829214u || pc > 0x829245u) return 0;
    const uint32_t base = pc < 0x82922du ? 0x829214u : 0x82922du;
    const unsigned phase = pc - base;
    switch (phase) {
    case 0u: case 1u: case 24u: break;
    case 3u: case 4u: case 7u: case 8u: case 10u:
    case 17u: case 18u: case 21u: case 23u:
        if (registers->mf || registers->xf) return 0;
        break;
    case 12u: case 15u:
        if (!registers->mf || registers->xf) return 0;
        break;
    default: return 0;
    }
    if (registers->e || !step_budget ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(base, base + 25u))
        return 0;
    const uint16_t table = base == 0x829214u ? 0x11d8u : 0x11e8u;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    if (!cart || offset < phase) return 0;
    const unsigned first = offset - phase;
    if (cart->romSize < first + 25u ||
        memcmp(cart->rom + first, "\x08\xc2\x30\x8a\xa2\x00\x00\x4a\x90\x07\xe2\x20\x9e", 13u) ||
        cart->rom[first + 13u] != (uint8_t)table ||
        memcmp(cart->rom + first + 14u, "\x11\xc2\x20\xe8\xe0\x10\x00\xd0\xf0\x28\x6b", 11u))
        return 0;
    const uint32_t bank = (uint32_t)registers->db << 16;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (phase) {
    case 0u: goto save_status;
    case 1u: goto widen;
    case 3u: goto load_mask;
    case 4u: goto clear_index;
    case 7u: goto shift_mask;
    case 8u: goto branch_bit;
    case 10u: goto narrow;
    case 12u: goto clear_slot;
    case 15u: goto widen_accumulator;
    case 17u: goto advance_slot;
    case 18u: goto compare_end;
    case 21u: goto repeat_scan;
    case 23u: goto restore_status;
    case 24u: goto return_long;
    default: return 0;
    }
save_status:
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, base, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, interp816_getFlags(registers));
    NativeSpanStep(&span, registers, cpu, 3u, (uint16_t)(base + 1u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
widen:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, base + 1u, 2u, span.instructions != 0u);
    registers->mf = false;
    registers->xf = false;
    NativeSpanStep(&span, registers, cpu, 3u, (uint16_t)(base + 3u));
load_mask:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, base + 3u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0x8au);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(base + 4u));
clear_index:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, base + 4u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0u, false);
    NativeSpanStep(&span, registers, cpu, 3u, (uint16_t)(base + 7u));
shift_mask:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, base + 7u, 1u, span.instructions != 0u);
    {
        const uint16_t value = registers->a;
        registers->c = (value & 1u) != 0u;
        Lufia2DecompApplyOperand(registers, 0xa9u, value >> 1, false);
    }
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(base + 8u));
branch_bit:
    {
        const bool clear = registers->c;
        const unsigned cycles = clear ? 2u : 3u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, base + 8u, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, (uint16_t)(base + (clear ? 10u : 17u)));
        if (!clear) goto advance_slot;
    }
narrow:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, base + 10u, 2u, span.instructions != 0u);
    registers->mf = true;
    NativeSpanStep(&span, registers, cpu, 3u, (uint16_t)(base + 12u));
clear_slot:
    {
        const uint32_t address = (bank + table + registers->x) & 0xffffffu;
        if (!NativeSpanCanStep(&span, cpu, 6u) || !Lufia2DecompIsWramByte(address))
            goto finished;
        Lufia2SpanFetch(registers, base + 12u, 3u, span.instructions != 0u);
        registers->write(registers->mem, address, 0u);
        NativeSpanStep(&span, registers, cpu, 6u, (uint16_t)(base + 15u));
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
widen_accumulator:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, base + 15u, 2u, span.instructions != 0u);
    registers->mf = false;
    NativeSpanStep(&span, registers, cpu, 3u, (uint16_t)(base + 17u));
advance_slot:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, base + 17u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(base + 18u));
compare_end:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, base + 18u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, 0x0010u, false);
    NativeSpanStep(&span, registers, cpu, 3u, (uint16_t)(base + 21u));
repeat_scan:
    {
        const bool repeat = !registers->z;
        const unsigned cycles = repeat ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, base + 21u, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, (uint16_t)(base + (repeat ? 7u : 23u)));
        if (repeat) {
            if (++span.slots < 8u) goto shift_mask;
            goto finished;
        }
    }
restore_status:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, base + 23u, 1u, span.instructions != 0u);
    interp816_setFlags(registers, registers->read(registers->mem, ++registers->sp));
    NativeSpanStep(&span, registers, cpu, 4u, (uint16_t)(base + 24u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
return_long:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    Lufia2SpanFetch(registers, base + 24u, 1u, span.instructions != 0u);
    {
        uint16_t address = registers->read(registers->mem, ++registers->sp);
        address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        registers->k = registers->read(registers->mem, ++registers->sp);
        NativeSpanStep(&span, registers, cpu, 6u, (uint16_t)(address + 1u));
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
finished:
    if (!span.instructions) return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
