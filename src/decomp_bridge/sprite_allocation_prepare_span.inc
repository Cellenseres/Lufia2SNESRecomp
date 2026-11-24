static int Lufia2DecompRunSpriteAllocationPrepare(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    if (registers->e || !registers->mf || !registers->xf || registers->x > 0xffu ||
        registers->y > 0xffu || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted || !step_budget ||
        !Lufia2DecompFrameRangeQuiet(0x83ab7cu, 0x83ab8au))
        return 0;
    switch (pc) {
    case 0x83ab7cu: case 0x83ab7du: case 0x83ab7fu: case 0x83ab81u: case 0x83ab82u:
    case 0x83ab83u: case 0x83ab85u: case 0x83ab86u: case 0x83ab88u:
        break;
    default: return 0;
    }
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = (pc & 0xffffu) - 0xab7cu;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 14u || memcmp(cart->rom + first,
        "\x8b\x85\x54\xa9\x7e\x48\xab\xa2\x00\x9b\x64\x55\x64\x56", 14u))
        return 0;
    const uint16_t request = (uint16_t)(registers->dp + 0x54u);
    const uint16_t start = (uint16_t)(registers->dp + 0x55u);
    const uint16_t cursor = (uint16_t)(registers->dp + 0x56u);
    if (!Lufia2DecompIsWramByte(request) || !Lufia2DecompIsWramByte(start) ||
        !Lufia2DecompIsWramByte(cursor))
        return 0;
    const unsigned direct_cycles = 3u + ((registers->dp & 0xffu) != 0u);
    SpriteScanSpan span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83ab7cu: goto save_bank;
    case 0x83ab7du: goto store_request;
    case 0x83ab7fu: goto load_bank;
    case 0x83ab81u: goto push_bank;
    case 0x83ab82u: goto select_bank;
    case 0x83ab83u: goto clear_index;
    case 0x83ab85u: goto clear_run;
    case 0x83ab86u: goto clear_start;
    case 0x83ab88u: goto clear_cursor;
    default: return 0;
    }
save_bank:
    if (!SpriteScanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab7cu, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, registers->db);
    SpriteScanStep(&span, registers, cpu, 3u, 0xab7du);
    SpriteAllocationScanProgress(&span, cpu, &dynamic_epoch, &write_epoch);
store_request:
    if (!SpriteScanCanStep(&span, cpu, direct_cycles)) goto finished;
    Lufia2SpanFetch(registers, 0x83ab7du, 2u, span.instructions != 0u);
    registers->write(registers->mem, request, (uint8_t)registers->a);
    SpriteScanStep(&span, registers, cpu, direct_cycles, 0xab7fu);
    SpriteAllocationScanProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_bank:
    if (!SpriteScanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83ab7fu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x7eu, true);
    SpriteScanStep(&span, registers, cpu, 2u, 0xab81u);
push_bank:
    if (!SpriteScanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab81u, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    SpriteScanStep(&span, registers, cpu, 3u, 0xab82u);
    SpriteAllocationScanProgress(&span, cpu, &dynamic_epoch, &write_epoch);
select_bank:
    if (!SpriteScanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83ab82u, 1u, span.instructions != 0u);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    SpriteScanStep(&span, registers, cpu, 4u, 0xab83u);
    SpriteAllocationScanProgress(&span, cpu, &dynamic_epoch, &write_epoch);
clear_index:
    if (!SpriteScanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83ab83u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0u, true);
    SpriteScanStep(&span, registers, cpu, 2u, 0xab85u);
clear_run:
    if (!SpriteScanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83ab85u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0x9bu);
    SpriteScanStep(&span, registers, cpu, 2u, 0xab86u);
clear_start:
    if (!SpriteScanCanStep(&span, cpu, direct_cycles)) goto finished;
    Lufia2SpanFetch(registers, 0x83ab86u, 2u, span.instructions != 0u);
    registers->write(registers->mem, start, 0u);
    SpriteScanStep(&span, registers, cpu, direct_cycles, 0xab88u);
    SpriteAllocationScanProgress(&span, cpu, &dynamic_epoch, &write_epoch);
clear_cursor:
    if (!SpriteScanCanStep(&span, cpu, direct_cycles)) goto finished;
    Lufia2SpanFetch(registers, 0x83ab88u, 2u, span.instructions != 0u);
    registers->write(registers->mem, cursor, 0u);
    SpriteScanStep(&span, registers, cpu, direct_cycles, 0xab8au);
    SpriteAllocationScanProgress(&span, cpu, &dynamic_epoch, &write_epoch);
finished:
    if (!span.instructions) return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
