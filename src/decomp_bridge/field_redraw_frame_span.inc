static int Lufia2DecompRunFieldDivisionDelay(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    if ((pc != 0x80f81cu && pc != 0x80f81eu && pc != 0x80f820u) || registers->e ||
        !step_budget || (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x80f81cu, 0x80f821u))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x80f81cu;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 5u || memcmp(cart->rom + first, "\xa5\x00\xc2\x20\x60", 5u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    if (pc == 0x80f820u) goto return_short;
    if (pc == 0x80f81eu) goto widen;
    {
        const uint16_t low = registers->dp;
        const uint16_t high = (uint16_t)(low + 1u);
        const bool wide = !registers->mf;
        const unsigned cycles = 3u + wide + ((low & 0xffu) != 0u);
        if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(low) ||
            (wide && !Lufia2DecompIsWramByte(high)))
            goto finished;
        Lufia2SpanFetch(registers, 0x80f81cu, 2u, false);
        Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, low, high, wide), !wide);
        NativeSpanStep(&span, registers, cpu, cycles, 0xf81eu);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
widen:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f81eu, 2u, span.instructions != 0u);
    registers->mf = false;
    NativeSpanStep(&span, registers, cpu, 3u, 0xf820u);
return_short:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80f820u, 1u, span.instructions != 0u);
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

static int Lufia2DecompRunFieldRedrawFrame(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    if (pc >= 0x80f81cu)
        return Lufia2DecompRunFieldDivisionDelay(registers, cpu, pc, opcode,
            dynamic_epoch, write_epoch, step_budget, step_count);
    if (pc < 0x80f4ccu || pc > 0x80f4ecu || registers->e || registers->dp || !step_budget ||
        (pc < 0x80f4e7u && (registers->mf || registers->xf)) ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x80f4ccu, 0x80f4edu))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x80f4ccu;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 33u || memcmp(cart->rom + first,
        "\xa5\x13\xa8\x18\x69\x10\x00\x85\x13\xa6\x15\xa5\x11\x20\x34\xf7"
        "\xa6\x30\xa4\x2d\x20\xc6\xf6\xc6\x17\xd0\xe5\x28\xab\x7a\xfa\x68\x6b", 33u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x80f4ccu: goto load_row;
    case 0x80f4ceu: goto save_row;
    case 0x80f4cfu: goto clear_carry;
    case 0x80f4d0u: goto advance_row;
    case 0x80f4d3u: goto store_row;
    case 0x80f4d5u: goto load_layer;
    case 0x80f4d7u: goto load_column;
    case 0x80f4d9u: goto locate_cell;
    case 0x80f4dcu: goto load_cell;
    case 0x80f4deu: goto load_buffer;
    case 0x80f4e0u: goto copy_row;
    case 0x80f4e3u: goto count_rows;
    case 0x80f4e5u: goto repeat_row;
    case 0x80f4e7u: goto restore_status;
    case 0x80f4e8u: goto restore_bank;
    case 0x80f4e9u: goto restore_y;
    case 0x80f4eau: goto restore_x;
    case 0x80f4ebu: goto restore_a;
    case 0x80f4ecu: goto return_long;
    default: return 0;
    }
load_row:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f4ccu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, 0x13u, 0x14u, true), false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xf4ceu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
save_row:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f4ceu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf4cfu);
clear_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f4cfu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xf4d0u);
advance_row:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f4d0u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x10u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf4d3u);
store_row:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f4d3u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x13u, 0x14u, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u, 0xf4d5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_layer:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f4d5u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, 0x15u, 0x16u, true), false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xf4d7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_column:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f4d7u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, 0x11u, 0x12u, true), false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xf4d9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
locate_cell:
    pc = 0x80f4d9u;
    goto call_child;
load_cell:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f4dcu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, 0x30u, 0x31u, true), false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xf4deu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_buffer:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f4deu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, 0x2du, 0x2eu, true), false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xf4e0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
copy_row:
    pc = 0x80f4e0u;
call_child:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    {
        if (span.instructions) Lufia2SpanProbeOpcode(registers, pc);
        const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, pc, 0x20u,
            pc == 0x80f4d9u ? 0x80f734u : 0x80f6c6u, false, false, false);
        if (!cycles) goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
count_rows:
    if (!NativeSpanCanStep(&span, cpu, 7u)) goto finished;
    Lufia2SpanFetch(registers, 0x80f4e3u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x17u, 0x18u, true);
    NativeSpanStep(&span, registers, cpu, 7u, 0xf4e5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
repeat_row:
    {
        const unsigned cycles = registers->z ? 2u : 3u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x80f4e5u, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, registers->z ? 0xf4e7u : 0xf4ccu);
        if (!registers->z) goto load_row;
    }
restore_status:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80f4e7u, 1u, span.instructions != 0u);
    interp816_setFlags(registers, registers->read(registers->mem, ++registers->sp));
    NativeSpanStep(&span, registers, cpu, 4u, 0xf4e8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
restore_bank:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80f4e8u, 1u, span.instructions != 0u);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0xf4e9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
restore_y:
    pc = 0x80f4e9u;
    goto pull_index;
restore_x:
    pc = 0x80f4eau;
pull_index:
    {
        const unsigned cycles = 4u + !registers->xf;
        if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u ||
            registers->sp > 0x1ffeu - !registers->xf)
            goto finished;
        Lufia2SpanFetch(registers, pc, 1u, span.instructions != 0u);
        uint16_t value = registers->read(registers->mem, ++registers->sp);
        if (!registers->xf) value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        Lufia2DecompApplyOperand(registers, pc == 0x80f4e9u ? 0xa0u : 0xa2u, value, registers->xf);
        NativeSpanStep(&span, registers, cpu, cycles, (uint16_t)(pc + 1u));
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
        if (pc == 0x80f4e9u) goto restore_x;
    }
restore_a:
    {
        const unsigned cycles = 4u + !registers->mf;
        if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u ||
            registers->sp > 0x1ffeu - !registers->mf)
            goto finished;
        Lufia2SpanFetch(registers, 0x80f4ebu, 1u, span.instructions != 0u);
        uint16_t value = registers->read(registers->mem, ++registers->sp);
        if (!registers->mf) value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        Lufia2DecompApplyOperand(registers, 0xa9u, value, registers->mf);
        NativeSpanStep(&span, registers, cpu, cycles, 0xf4ecu);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
return_long:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80f4ecu, 1u, span.instructions != 0u);
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
