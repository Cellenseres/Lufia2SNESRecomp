static int Lufia2DecompRunSpellRecordCopy(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    switch (pc) {
    case 0x81f422u: case 0x81f425u: case 0x81f429u: case 0x81f42au: case 0x81f42bu: case 0x81f42eu:
    case 0x81f430u: case 0x81f431u: case 0x81f435u:
    case 0x81f436u: case 0x81f439u: case 0x81f43du: case 0x81f43eu: case 0x81f43fu: case 0x81f442u:
        break;
    default: return 0;
    }
    const bool name = pc < 0x81f430u;
    const bool terminate = pc >= 0x81f430u && pc < 0x81f436u;
    const uint32_t first_pc = name ? 0x81f422u : terminate ? 0x81f430u : 0x81f436u;
    const uint32_t end_pc = name ? 0x81f430u : terminate ? 0x81f436u : 0x81f444u;
    if (registers->e || !registers->mf || registers->xf || !step_budget ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(first_pc, end_pc))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = (pc & 0xffffu) - 0xf422u;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 34u || memcmp(cart->rom + first,
        "\xb9\x00\x00\x9f\x77\x0b\x00\xc8\xe8\xe0\x08\x00\xd0\xf2"
        "\x7b\x9f\x77\x0b\x00\xe8"
        "\xb9\x00\x00\x9f\x77\x0b\x00\xc8\xe8\xe0\x14\x00\xd0\xf2", 34u))
        return 0;
    const uint32_t bank = (uint32_t)registers->db << 16;
    const uint16_t limit = name ? 8u : 20u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x81f422u: case 0x81f436u: goto load_byte;
    case 0x81f425u: case 0x81f439u: goto store_byte;
    case 0x81f429u: case 0x81f43du: goto advance_source;
    case 0x81f42au: case 0x81f43eu: goto advance_destination;
    case 0x81f42bu: case 0x81f43fu: goto compare_end;
    case 0x81f42eu: case 0x81f442u: goto repeat_copy;
    case 0x81f430u: goto load_terminator;
    case 0x81f431u: goto store_terminator;
    case 0x81f435u: goto skip_terminator;
    default: return 0;
    }
load_byte:
    {
        const uint32_t address = bank + registers->y;
        const unsigned cycles = 4u + (registers->y >= 0x100u);
        if (!NativeSpanCanStep(&span, cpu, cycles) ||
            (!Lufia2DecompIsWramByte(address) && !Lufia2DecompIsLoRomDataByte(cart, address | 0x800000u)))
            goto finished;
        Lufia2SpanFetch(registers, first_pc, 3u, span.instructions != 0u);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
        NativeSpanStep(&span, registers, cpu, cycles, (uint16_t)(first_pc + 3u));
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
store_byte:
    if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsWramByte(0x0b77u + registers->x))
        goto finished;
    Lufia2SpanFetch(registers, first_pc + 3u, 4u, span.instructions != 0u);
    registers->write(registers->mem, 0x0b77u + registers->x, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 5u, (uint16_t)(first_pc + 7u));
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
advance_source:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, first_pc + 7u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(first_pc + 8u));
advance_destination:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, first_pc + 8u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, (uint16_t)(first_pc + 9u));
compare_end:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, first_pc + 9u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, limit, false);
    NativeSpanStep(&span, registers, cpu, 3u, (uint16_t)(first_pc + 12u));
repeat_copy:
    {
        const bool repeat = !registers->z;
        const unsigned cycles = repeat ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, first_pc + 12u, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, (uint16_t)(repeat ? first_pc : end_pc));
        if (repeat && ++span.slots < 8u) goto load_byte;
    }
    goto finished;
load_terminator:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x81f430u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0x7bu);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf431u);
store_terminator:
    if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsWramByte(0x0b77u + registers->x))
        goto finished;
    Lufia2SpanFetch(registers, 0x81f431u, 4u, span.instructions != 0u);
    registers->write(registers->mem, 0x0b77u + registers->x, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 5u, 0xf435u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
skip_terminator:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x81f435u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf436u);
finished:
    if (!span.instructions) return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
