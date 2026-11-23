static void SpriteAllocationFinishShift(Interp816 *registers, bool rotate) {
    const uint8_t value = (uint8_t)registers->a;
    const uint8_t result = (uint8_t)((value << 1) | (rotate && registers->c));
    registers->c = (value & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, result, true);
}

static int Lufia2DecompRunSpriteAllocationFinish(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    if (registers->e || !registers->mf || !registers->xf || registers->x > 0xffu ||
        registers->y > 0xffu || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted || !step_budget ||
        !Lufia2DecompFrameRangeQuiet(0x83aba5u, 0x83abccu))
        return 0;
    switch (pc) {
    case 0x83aba5u: case 0x83aba6u: case 0x83aba7u: case 0x83aba8u: case 0x83abaau:
    case 0x83abacu: case 0x83abaeu: case 0x83abb1u: case 0x83abb2u: case 0x83abb4u:
    case 0x83abb6u: case 0x83abb7u: case 0x83abb9u: case 0x83abbau: case 0x83abbcu:
    case 0x83abbeu: case 0x83abc0u: case 0x83abc2u: case 0x83abc3u: case 0x83abc4u:
    case 0x83abc6u: case 0x83abc7u: case 0x83abc8u: case 0x83abcau: case 0x83abcbu:
        break;
    default: return 0;
    }
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = (pc & 0xffffu) - 0xaba5u;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 39u || memcmp(cart->rom + first,
        "\xab\x38\x6b\xa6\x55\xa5\xa7\x09\x80\x9d\x00\xe1\xe8\xc6\x54\xd0\xf8"
        "\xab\xa5\x55\x0a\x85\x55\xa5\x55\x29\xf0\x14\x55\x0a\xeb\xa9\x00"
        "\x2a\xeb\x65\x55\x18\x6b", 39u))
        return 0;
    const bool failure = pc <= 0x83aba7u;
    const bool painting = pc >= 0x83aba8u && pc <= 0x83abb4u;
    const uint32_t bank = (uint32_t)registers->db << 16;
    const uint16_t count = (uint16_t)(registers->dp + 0x54u);
    const uint16_t start = (uint16_t)(registers->dp + 0x55u);
    const uint16_t actor = (uint16_t)(registers->dp + 0xa7u);
    if ((!failure && !Lufia2DecompIsWramByte(start)) ||
        (painting && (!Lufia2DecompIsWramByte(count) || !Lufia2DecompIsWramByte(actor) ||
         !Lufia2DecompIsWramByte(bank + 0xe100u) || !Lufia2DecompIsWramByte(bank + 0xe1ffu))))
        return 0;
    const unsigned direct_cycles = 3u + ((registers->dp & 0xffu) != 0u);
    SpriteScanSpan span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83aba5u: case 0x83abb6u: goto pull_bank;
    case 0x83aba6u: goto fail;
    case 0x83aba7u: case 0x83abcbu: goto return_long;
    case 0x83aba8u: goto load_start;
    case 0x83abaau: goto load_actor;
    case 0x83abacu: goto mark_actor;
    case 0x83abaeu: goto write_slot;
    case 0x83abb1u: goto advance_slot;
    case 0x83abb2u: goto consume_slot;
    case 0x83abb4u: goto repeat_slots;
    case 0x83abb7u: goto load_offset;
    case 0x83abb9u: goto double_offset;
    case 0x83abbau: goto store_offset;
    case 0x83abbcu: goto reload_offset;
    case 0x83abbeu: goto mask_high;
    case 0x83abc0u: goto clear_high;
    case 0x83abc2u: goto shift_high;
    case 0x83abc3u: goto exchange_high;
    case 0x83abc4u: goto clear_low;
    case 0x83abc6u: goto rotate_low;
    case 0x83abc7u: goto exchange_low;
    case 0x83abc8u: goto add_low;
    case 0x83abcau: goto succeed;
    default: return 0;
    }
load_start:
    if (!SpriteScanCanStep(&span, cpu, direct_cycles)) goto finished;
    Lufia2SpanFetch(registers, 0x83aba8u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->read(registers->mem, start), true);
    SpriteScanStep(&span, registers, cpu, direct_cycles, 0xabaau);
    SpriteAllocationScanProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_actor:
    if (!SpriteScanCanStep(&span, cpu, direct_cycles)) goto finished;
    Lufia2SpanFetch(registers, 0x83abaau, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, actor), true);
    SpriteScanStep(&span, registers, cpu, direct_cycles, 0xabacu);
    SpriteAllocationScanProgress(&span, cpu, &dynamic_epoch, &write_epoch);
mark_actor:
    if (!SpriteScanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83abacu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0x80u, true);
    SpriteScanStep(&span, registers, cpu, 2u, 0xabaeu);
write_slot:
    if (!SpriteScanCanStep(&span, cpu, 5u)) goto finished;
    Lufia2SpanFetch(registers, 0x83abaeu, 3u, span.instructions != 0u);
    registers->write(registers->mem, bank + 0xe100u + registers->x, (uint8_t)registers->a);
    SpriteScanStep(&span, registers, cpu, 5u, 0xabb1u);
    SpriteAllocationScanProgress(&span, cpu, &dynamic_epoch, &write_epoch);
advance_slot:
    if (!SpriteScanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83abb1u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint8_t)(registers->x + 1u), true);
    SpriteScanStep(&span, registers, cpu, 2u, 0xabb2u);
consume_slot:
    if (!SpriteScanCanStep(&span, cpu, direct_cycles + 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83abb2u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, count, (uint16_t)(count + 1u), false);
    SpriteScanStep(&span, registers, cpu, direct_cycles + 2u, 0xabb4u);
    SpriteAllocationScanProgress(&span, cpu, &dynamic_epoch, &write_epoch);
repeat_slots:
    {
        const bool repeat = !registers->z;
        const unsigned cycles = repeat ? 3u : 2u;
        if (!SpriteScanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x83abb4u, 2u, span.instructions != 0u);
        SpriteScanStep(&span, registers, cpu, cycles, repeat ? 0xabaeu : 0xabb6u);
        if (repeat) {
            if (++span.slots < 8u) goto write_slot;
            goto finished;
        }
    }
pull_bank:
    if (!SpriteScanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    {
        const uint32_t pull_pc = failure ? 0x83aba5u : 0x83abb6u;
        Lufia2SpanFetch(registers, pull_pc, 1u, span.instructions != 0u);
        registers->db = registers->read(registers->mem, ++registers->sp);
        registers->z = registers->db == 0u;
        registers->n = (registers->db & 0x80u) != 0u;
        SpriteScanStep(&span, registers, cpu, 4u, failure ? 0xaba6u : 0xabb7u);
        SpriteAllocationScanProgress(&span, cpu, &dynamic_epoch, &write_epoch);
        if (failure) goto fail;
    }
load_offset:
    if (!SpriteScanCanStep(&span, cpu, direct_cycles)) goto finished;
    Lufia2SpanFetch(registers, 0x83abb7u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, start), true);
    SpriteScanStep(&span, registers, cpu, direct_cycles, 0xabb9u);
    SpriteAllocationScanProgress(&span, cpu, &dynamic_epoch, &write_epoch);
double_offset:
    if (!SpriteScanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83abb9u, 1u, span.instructions != 0u);
    SpriteAllocationFinishShift(registers, false);
    SpriteScanStep(&span, registers, cpu, 2u, 0xabbau);
store_offset:
    if (!SpriteScanCanStep(&span, cpu, direct_cycles)) goto finished;
    Lufia2SpanFetch(registers, 0x83abbau, 2u, span.instructions != 0u);
    registers->write(registers->mem, start, (uint8_t)registers->a);
    SpriteScanStep(&span, registers, cpu, direct_cycles, 0xabbcu);
    SpriteAllocationScanProgress(&span, cpu, &dynamic_epoch, &write_epoch);
reload_offset:
    if (!SpriteScanCanStep(&span, cpu, direct_cycles)) goto finished;
    Lufia2SpanFetch(registers, 0x83abbcu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, start), true);
    SpriteScanStep(&span, registers, cpu, direct_cycles, 0xabbeu);
    SpriteAllocationScanProgress(&span, cpu, &dynamic_epoch, &write_epoch);
mask_high:
    if (!SpriteScanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83abbeu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0xf0u, true);
    SpriteScanStep(&span, registers, cpu, 2u, 0xabc0u);
clear_high:
    if (!SpriteScanCanStep(&span, cpu, direct_cycles + 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83abc0u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0x14u, start, (uint16_t)(start + 1u), false);
    SpriteScanStep(&span, registers, cpu, direct_cycles + 2u, 0xabc2u);
    SpriteAllocationScanProgress(&span, cpu, &dynamic_epoch, &write_epoch);
shift_high:
    if (!SpriteScanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83abc2u, 1u, span.instructions != 0u);
    SpriteAllocationFinishShift(registers, false);
    SpriteScanStep(&span, registers, cpu, 2u, 0xabc3u);
exchange_high:
    if (!SpriteScanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x83abc3u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    SpriteScanStep(&span, registers, cpu, 3u, 0xabc4u);
clear_low:
    if (!SpriteScanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83abc4u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0u, true);
    SpriteScanStep(&span, registers, cpu, 2u, 0xabc6u);
rotate_low:
    if (!SpriteScanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83abc6u, 1u, span.instructions != 0u);
    SpriteAllocationFinishShift(registers, true);
    SpriteScanStep(&span, registers, cpu, 2u, 0xabc7u);
exchange_low:
    if (!SpriteScanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x83abc7u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    SpriteScanStep(&span, registers, cpu, 3u, 0xabc8u);
add_low:
    if (!SpriteScanCanStep(&span, cpu, direct_cycles)) goto finished;
    Lufia2SpanFetch(registers, 0x83abc8u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, registers->read(registers->mem, start), true);
    SpriteScanStep(&span, registers, cpu, direct_cycles, 0xabcau);
    SpriteAllocationScanProgress(&span, cpu, &dynamic_epoch, &write_epoch);
succeed:
    if (!SpriteScanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83abcau, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0x18u);
    SpriteScanStep(&span, registers, cpu, 2u, 0xabcbu);
    goto return_long;
fail:
    if (!SpriteScanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83aba6u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0x38u);
    SpriteScanStep(&span, registers, cpu, 2u, 0xaba7u);
return_long:
    if (!SpriteScanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    Lufia2SpanFetch(registers, failure ? 0x83aba7u : 0x83abcbu, 1u, span.instructions != 0u);
    {
        uint16_t address = registers->read(registers->mem, ++registers->sp);
        address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        registers->k = registers->read(registers->mem, ++registers->sp);
        SpriteScanStep(&span, registers, cpu, 6u, (uint16_t)(address + 1u));
        SpriteAllocationScanProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
finished:
    if (!span.instructions) return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
