static int Lufia2DecompRunBattleActionResultFill(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    uint32_t first;
    switch (pc) {
    case 0x85ce34u: case 0x85ce38u: case 0x85ce39u: first = 0x85ce34u; break;
    case 0x85d718u: case 0x85d71cu: case 0x85d71du: first = 0x85d718u; break;
    default: return 0;
    }
    if (registers->e || !registers->mf || registers->xf || !step_budget ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(first, first + 7u))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - first;
    if (!cart || offset < delta) return 0;
    const unsigned start = offset - delta;
    if (cart->romSize < start + 7u ||
        memcmp(cart->rom + start, "\x9f\x0c\xf6\x7f\xca\x10\xf9", 7u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    if (delta == 4u) goto previous_byte;
    if (delta == 5u) goto repeat_fill;
store_byte:
    {
        const uint32_t address = 0x7ff60cu + registers->x;
        if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsWramByte(address))
            goto finished;
        Lufia2SpanFetch(registers, first, 4u, span.instructions != 0u);
        registers->write(registers->mem, address, (uint8_t)registers->a);
        NativeSpanStep(&span, registers, cpu, 5u, (uint16_t)(first + 4u));
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
previous_byte:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, first + 4u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(first + 5u));
repeat_fill:
    {
        const bool repeat = !registers->n;
        const unsigned cycles = repeat ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, first + 5u, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, (uint16_t)(first + (repeat ? 0u : 7u)));
        if (repeat && ++span.slots < 8u) goto store_byte;
    }
finished:
    if (!span.instructions) return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
