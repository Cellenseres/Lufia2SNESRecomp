static int Lufia2DecompRunFieldVisibleActorScan(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    switch (pc) {
    case 0x80ef34u: case 0x80ef37u: case 0x80ef39u: case 0x80ef8au: case 0x80ef8bu:
        break;
    default: return 0;
    }
    if (registers->e || !registers->mf || registers->xf || !step_budget ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x80ef34u, 0x80ef3bu) ||
        !Lufia2DecompFrameRangeQuiet(0x80ef8au, 0x80ef8du))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = (pc & 0xffffu) - 0xef34u;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 0x59u ||
        memcmp(cart->rom + first, "\xb9\x22\x06\x89\x06\xd0\x4f", 7u) ||
        memcmp(cart->rom + first + 0x56u, "\x88\x10\xa7", 3u))
        return 0;
    const uint32_t bank = (uint32_t)registers->db << 16;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x80ef34u: goto load_status;
    case 0x80ef37u: goto test_status;
    case 0x80ef39u: goto branch_status;
    case 0x80ef8au: goto previous_actor;
    case 0x80ef8bu: goto repeat_scan;
    default: return 0;
    }
load_status:
    {
        const uint32_t address = (bank + 0x0622u + registers->y) & 0xffffffu;
        const unsigned cycles = 4u + (registers->y >= 0xdeu);
        if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
            goto finished;
        Lufia2SpanFetch(registers, 0x80ef34u, 3u, span.instructions != 0u);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
        NativeSpanStep(&span, registers, cpu, cycles, 0xef37u);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
test_status:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x80ef37u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x06u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xef39u);
branch_status:
    {
        const bool skip = !registers->z;
        const unsigned cycles = skip ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x80ef39u, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, skip ? 0xef8au : 0xef3bu);
        if (!skip) goto finished;
    }
previous_actor:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x80ef8au, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xef8bu);
repeat_scan:
    {
        const bool repeat = !registers->n;
        const unsigned cycles = repeat ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x80ef8bu, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, repeat ? 0xef34u : 0xef8du);
        if (repeat && ++span.slots < 8u) goto load_status;
    }
finished:
    if (!span.instructions) return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
