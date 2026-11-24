static int Lufia2DecompRunMenuStringPointer(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    if (pc < 0x808a99u || pc > 0x808aacu || !cpu || !cpu->ram || registers->e ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || !step_budget ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x808a99u, 0x808aadu))
        return 0;
    if (pc != 0x808a99u && pc != 0x808aa1u && pc != 0x808aaau && registers->mf != (pc >= 0x808aa3u))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x808a99u;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 20u || memcmp(cart->rom + first,
        "\xc2\x20\xb7\x5d\x85\x60\xc8\xc8\xe2\x20\xa5\x5f\x85\x62\x20\xca\x8b\x82\x00\xfe", 20u))
        return 0;
    const uint16_t source_low = (uint16_t)(registers->dp + 0x5du);
    const uint16_t source_high = (uint16_t)(registers->dp + 0x5eu);
    const uint16_t source_bank = (uint16_t)(registers->dp + 0x5fu);
    const uint16_t target_low = (uint16_t)(registers->dp + 0x60u);
    const uint16_t target_high = (uint16_t)(registers->dp + 0x61u);
    const uint16_t target_bank = (uint16_t)(registers->dp + 0x62u);
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x808a99u: goto widen;
    case 0x808a9bu: goto load_pointer;
    case 0x808a9du: goto store_pointer;
    case 0x808a9fu: goto advance_low;
    case 0x808aa0u: goto advance_high;
    case 0x808aa1u: goto narrow;
    case 0x808aa3u: goto load_bank;
    case 0x808aa5u: goto store_bank;
    case 0x808aa7u: goto call_string;
    case 0x808aaau: goto resume_parser;
    default: return 0;
    }
widen:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x808a99u, 2u, span.instructions != 0u);
    registers->mf = false;
    NativeSpanStep(&span, registers, cpu, 3u, 0x8a9bu);
load_pointer:
    {
        if (!NativeSpanCanStep(&span, cpu, 7u + direct_penalty) ||
            !Lufia2DecompIsWramByte(source_low) || !Lufia2DecompIsWramByte(source_high) ||
            !Lufia2DecompIsWramByte(source_bank)) goto finished;
        const uint32_t expected = cpu->ram[source_low] | ((uint32_t)cpu->ram[source_high] << 8) |
            ((uint32_t)cpu->ram[source_bank] << 16);
        const uint32_t address = (expected + registers->y) & 0xffffffu;
        const uint32_t high = (address + 1u) & 0xffffffu;
        if ((!Lufia2DecompIsWramByte(address) && !Lufia2DecompIsLoRomDataByte(cart, address | 0x800000u)) ||
            (!Lufia2DecompIsWramByte(high) && !Lufia2DecompIsLoRomDataByte(cart, high | 0x800000u))) goto finished;
        Lufia2SpanFetch(registers, 0x808a9bu, 2u, span.instructions != 0u);
        uint32_t pointer = Lufia2DecompReadData(registers, source_low, source_high, true);
        pointer |= (uint32_t)registers->read(registers->mem, source_bank) << 16;
        pointer = (pointer + registers->y) & 0xffffffu;
        Lufia2DecompApplyOperand(registers, 0xa9u,
            Lufia2DecompReadData(registers, pointer, (pointer + 1u) & 0xffffffu, true), false);
        NativeSpanStep(&span, registers, cpu, 7u + direct_penalty, 0x8a9du);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
store_pointer:
    if (!NativeSpanCanStep(&span, cpu, 4u + direct_penalty) ||
        !Lufia2DecompIsWramByte(target_low) || !Lufia2DecompIsWramByte(target_high)) goto finished;
    Lufia2SpanFetch(registers, 0x808a9du, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, target_low, target_high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u + direct_penalty, 0x8a9fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
advance_low:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x808a9fu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)((registers->y + 1u) & (registers->xf ? 0xffu : 0xffffu)), registers->xf);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8aa0u);
advance_high:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x808aa0u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)((registers->y + 1u) & (registers->xf ? 0xffu : 0xffffu)), registers->xf);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8aa1u);
narrow:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x808aa1u, 2u, span.instructions != 0u);
    registers->mf = true;
    NativeSpanStep(&span, registers, cpu, 3u, 0x8aa3u);
load_bank:
    if (!NativeSpanCanStep(&span, cpu, 3u + direct_penalty) || !Lufia2DecompIsWramByte(source_bank)) goto finished;
    Lufia2SpanFetch(registers, 0x808aa3u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, source_bank), true);
    NativeSpanStep(&span, registers, cpu, 3u + direct_penalty, 0x8aa5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
store_bank:
    if (!NativeSpanCanStep(&span, cpu, 3u + direct_penalty) || !Lufia2DecompIsWramByte(target_bank)) goto finished;
    Lufia2SpanFetch(registers, 0x808aa5u, 2u, span.instructions != 0u);
    registers->write(registers->mem, target_bank, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 3u + direct_penalty, 0x8aa7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
call_string:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu) goto finished;
    {
        if (span.instructions) Lufia2SpanProbeOpcode(registers, 0x808aa7u);
        const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x808aa7u, 0x20u,
            0x808bcau, false, true, registers->xf);
        if (!cycles) goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
resume_parser:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x808aaau, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 4u, 0x88adu);
finished:
    if (!span.instructions) return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
