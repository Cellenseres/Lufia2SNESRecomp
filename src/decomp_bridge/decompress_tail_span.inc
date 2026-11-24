static bool Lufia2DecompIsDecompressSourceByte(const Cart *cart, uint32_t address) {
    if (Lufia2DecompIsWramByte(address)) return true;
    const unsigned offset = ((address >> 16) & 0x7fu) * 0x8000u + (address & 0x7fffu);
    return (address & 0x8000u) != 0u && offset < cart->romSize;
}

static int Lufia2DecompRunDecompressTail(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    const bool symbol = pc == 0x808efau || pc == 0x808efcu;
    if ((!symbol && pc != 0x808f40u && pc != 0x808f42u) || !cpu || !cpu->ram ||
        registers->e || (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) ||
        !step_budget || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted)
        return 0;
    const uint32_t first_pc = symbol ? 0x808efau : 0x808f40u;
    // Entry hooks are handled by the dispatcher; guard the combined continuation.
    if (!Lufia2DecompFrameRangeQuiet(pc + 1u, first_pc + 4u)) return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - first_pc;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 4u || memcmp(cart->rom + first,
        symbol ? "\xb7\x5d\x30\x42" : "\x06\x65\x90\xcc", 4u))
        return 0;
    const bool wide = !registers->mf;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    if (pc != first_pc) goto branch;
    if (symbol) {
        const uint16_t low = (uint16_t)(registers->dp + 0x5du);
        const uint16_t high = (uint16_t)(registers->dp + 0x5eu);
        const uint16_t bank = (uint16_t)(registers->dp + 0x5fu);
        const unsigned cycles = 6u + wide + direct_penalty;
        if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(low) ||
            !Lufia2DecompIsWramByte(high) || !Lufia2DecompIsWramByte(bank)) goto finished;
        const uint32_t pointer = cpu->ram[low] | ((uint32_t)cpu->ram[high] << 8) |
            ((uint32_t)cpu->ram[bank] << 16);
        const uint32_t address = (pointer + registers->y) & 0xffffffu;
        const uint32_t next = (address + 1u) & 0xffffffu;
        if (!Lufia2DecompIsDecompressSourceByte(cart, address) ||
            (wide && !Lufia2DecompIsDecompressSourceByte(cart, next)))
            goto finished;
        Lufia2SpanFetch(registers, first_pc, 2u, false);
        uint32_t source = Lufia2DecompReadData(registers, low, high, true);
        source |= (uint32_t)registers->read(registers->mem, bank) << 16;
        source = (source + registers->y) & 0xffffffu;
        Lufia2DecompApplyOperand(registers, 0xa9u,
            Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, wide), !wide);
        NativeSpanStep(&span, registers, cpu, cycles, 0x8efcu);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    } else {
        const uint16_t low = (uint16_t)(registers->dp + 0x65u);
        const uint16_t high = (uint16_t)(registers->dp + 0x66u);
        const unsigned cycles = 5u + 2u * wide + direct_penalty;
        if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(low) ||
            (wide && !Lufia2DecompIsWramByte(high))) goto finished;
        Lufia2SpanFetch(registers, first_pc, 2u, false);
        Lufia2DecompModifyWram(registers, 0x06u, low, high, wide);
        NativeSpanStep(&span, registers, cpu, cycles, 0x8f42u);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
branch:
    {
        const bool taken = symbol ? registers->n : !registers->c;
        const unsigned cycles = taken ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, first_pc + 2u, 2u, span.instructions != 0u);
        const uint16_t next = symbol ? (taken ? 0x8f40u : 0x8efeu) : (taken ? 0x8f10u : 0x8f44u);
        NativeSpanStep(&span, registers, cpu, cycles, next);
    }
finished:
    if (!span.instructions) return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
