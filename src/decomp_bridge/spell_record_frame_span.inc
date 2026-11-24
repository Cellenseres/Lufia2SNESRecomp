static int Lufia2DecompRunSpellRecordFrame(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    bool wide;
    switch (pc) {
    case 0x81f414u: case 0x81f415u: case 0x81f417u: case 0x81f418u: case 0x81f419u:
    case 0x81f41cu: case 0x81f41fu: case 0x81f444u: case 0x81f445u: case 0x81f446u: case 0x81f45du:
        wide = false;
        break;
    case 0x81f448u: case 0x81f44bu: case 0x81f44eu: case 0x81f44fu: case 0x81f450u:
    case 0x81f454u: case 0x81f455u: case 0x81f458u: case 0x81f45bu:
        wide = true;
        break;
    default: return 0;
    }
    const bool pointer = pc >= 0x81f446u;
    const bool restore_bank = pc == 0x81f444u || pc == 0x81f445u;
    const bool load_index = pc == 0x81f41cu || pc == 0x81f41fu;
    const uint32_t first_pc = pointer ? 0x81f446u : restore_bank ? 0x81f444u : load_index ? 0x81f41cu : 0x81f414u;
    const uint32_t end_pc = pointer ? 0x81f45eu : restore_bank ? 0x81f446u : load_index ? 0x81f422u : 0x81f41cu;
    if (registers->e || registers->mf == wide || registers->xf || !step_budget ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(first_pc, end_pc))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = (pc & 0xffffu) - 0xf414u;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 0x4au ||
        memcmp(cart->rom + first, "\x8b\xa9\x95\x48\xab\x20\x46\xf4\xac\x0d\x0a\xa2\x00\x00", 14u) ||
        memcmp(cart->rom + first + 0x30u,
            "\xab\x6b\xc2\x20\xad\x0b\x0a\x29\xff\x00\x0a\xaa\xbf\x5b\xfa\x95"
            "\x18\x69\x5b\xfa\x8d\x0d\x0a\xe2\x20\x60", 26u))
        return 0;
    const uint32_t bank = (uint32_t)registers->db << 16;
    if ((pointer || load_index) &&
        (!Lufia2DecompIsWramByte(bank + 0x0a0bu) || !Lufia2DecompIsWramByte(bank + 0x0a0eu)))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x81f414u: goto save_bank;
    case 0x81f415u: goto load_bank;
    case 0x81f417u: goto push_bank;
    case 0x81f418u: case 0x81f444u: goto pull_bank;
    case 0x81f419u: goto call_pointer;
    case 0x81f41cu: goto load_source;
    case 0x81f41fu: goto clear_index;
    case 0x81f445u: goto return_long;
    case 0x81f446u: goto widen;
    case 0x81f448u: goto load_spell;
    case 0x81f44bu: goto mask_spell;
    case 0x81f44eu: goto double_spell;
    case 0x81f44fu: goto select_spell;
    case 0x81f450u: goto load_offset;
    case 0x81f454u: goto clear_carry;
    case 0x81f455u: goto add_base;
    case 0x81f458u: goto store_pointer;
    case 0x81f45bu: goto narrow;
    case 0x81f45du: goto return_short;
    default: return 0;
    }
save_bank:
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81f414u, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, registers->db);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf415u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_bank:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x81f415u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x95u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf417u);
push_bank:
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81f417u, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf418u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
pull_bank:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, restore_bank ? 0x81f444u : 0x81f418u, 1u, span.instructions != 0u);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->n = (registers->db & 0x80u) != 0u;
    registers->z = registers->db == 0u;
    NativeSpanStep(&span, registers, cpu, 4u, restore_bank ? 0xf445u : 0xf419u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    if (restore_bank) goto return_long;
    goto finished;
call_pointer:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    {
        const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, pc, opcode, 0x81f446u, false, true, false);
        if (!cycles) goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
load_source:
    if (!NativeSpanCanStep(&span, cpu, 5u)) goto finished;
    Lufia2SpanFetch(registers, 0x81f41cu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u,
        Lufia2DecompReadData(registers, bank + 0x0a0du, bank + 0x0a0eu, true), false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xf41fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
clear_index:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x81f41fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf422u);
    goto finished;
widen:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x81f446u, 2u, span.instructions != 0u);
    registers->mf = false;
    NativeSpanStep(&span, registers, cpu, 3u, 0xf448u);
load_spell:
    if (!NativeSpanCanStep(&span, cpu, 5u)) goto finished;
    Lufia2SpanFetch(registers, 0x81f448u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u,
        Lufia2DecompReadData(registers, bank + 0x0a0bu, bank + 0x0a0cu, true), false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xf44bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
mask_spell:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x81f44bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf44eu);
double_spell:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x81f44eu, 1u, span.instructions != 0u);
    {
        const uint16_t value = registers->a;
        registers->c = (value & 0x8000u) != 0u;
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(value << 1), false);
    }
    NativeSpanStep(&span, registers, cpu, 2u, 0xf44fu);
select_spell:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x81f44fu, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xaau);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf450u);
load_offset:
    {
        const uint32_t address = 0x95fa5bu + registers->x;
        if (!NativeSpanCanStep(&span, cpu, 6u) ||
            (!Lufia2DecompIsWramByte(address) && !Lufia2DecompIsLoRomDataByte(cart, address)) ||
            (!Lufia2DecompIsWramByte(address + 1u) && !Lufia2DecompIsLoRomDataByte(cart, address + 1u)))
            goto finished;
        Lufia2SpanFetch(registers, 0x81f450u, 4u, span.instructions != 0u);
        Lufia2DecompApplyOperand(registers, 0xa9u,
            Lufia2DecompReadData(registers, address, address + 1u, true), false);
        NativeSpanStep(&span, registers, cpu, 6u, 0xf454u);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
clear_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x81f454u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0x18u);
    NativeSpanStep(&span, registers, cpu, 2u, 0xf455u);
add_base:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x81f455u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0xfa5bu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xf458u);
store_pointer:
    if (!NativeSpanCanStep(&span, cpu, 5u)) goto finished;
    Lufia2SpanFetch(registers, 0x81f458u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, bank + 0x0a0du, bank + 0x0a0eu, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 5u, 0xf45bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
narrow:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x81f45bu, 2u, span.instructions != 0u);
    registers->mf = true;
    NativeSpanStep(&span, registers, cpu, 3u, 0xf45du);
return_short:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81f45du, 1u, span.instructions != 0u);
    {
        uint16_t address = registers->read(registers->mem, ++registers->sp);
        address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        NativeSpanStep(&span, registers, cpu, 6u, (uint16_t)(address + 1u));
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
return_long:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81f445u, 1u, span.instructions != 0u);
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
