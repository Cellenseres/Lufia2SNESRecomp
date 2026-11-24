#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunSpriteSetTableSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                             uint64_t dynamic_epoch, uint64_t write_epoch,
                                             unsigned step_budget, long *step_count) {
    const bool byte_accumulator = true;
    const bool dynamic_accumulator_widths = false;
    if (pc < 0x868cdau || pc >= 0x868cf5u || !cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram ||
        registers->e || !step_budget || registers->waiting || registers->stopped || registers->nmiWanted ||
        registers->irqWanted || (!dynamic_accumulator_widths && registers->mf != byte_accumulator) ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) ||
        (pc + 1u < 0x868cf5u && !Lufia2DecompFrameRangeQuiet(pc + 1u, 0x868cf5u)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x868cdau;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 27u || memcmp(cart->rom + first,
                                              "\x8b\xa9\x8e\x48\xab\xb9\xa9\xd9\x9f\x68\x12\x00\xb9\xaa\xd9"
                                              "\x9f\x98\x12\x00\xa9\x8e\x9f\x38\x12\x00\xab\x6b",
                                              27u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x868cdau: goto save_data_bank;
    case 0x868cdbu: goto table_bank;
    case 0x868cddu: goto save_table_bank;
    case 0x868cdeu: goto select_table_bank;
    case 0x868cdfu: goto read_list_low;
    case 0x868ce2u: goto store_list_low;
    case 0x868ce6u: goto read_list_high;
    case 0x868ce9u: goto store_list_high;
    case 0x868cedu: goto list_bank;
    case 0x868cefu: goto store_list_bank;
    case 0x868cf3u: goto restore_data_bank;
    case 0x868cf4u: goto return_table;
    default: return 0;
    }
save_data_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x868cdau, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp, (uint8_t)registers->db);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, 3u, 0x8cdbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
table_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868cdbu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x008eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8cddu);
}
save_table_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x868cddu, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp, (uint8_t)registers->a);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, 3u, 0x8cdeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_table_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x868cdeu, 1u, span.instructions != 0u);
    const uint8_t bank = registers->read(registers->mem, ++registers->sp);
    registers->db = bank;
    registers->z = bank == 0u;
    registers->n = (bank & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x8cdfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_list_low: {
    const bool crossed = (0xd9a9u >> 8) != ((0xd9a9u + registers->y) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xd9a9u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x868cdfu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8ce2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_list_low: {
    const uint32_t address = (0x001268u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsWramByte(address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x868ce2u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8ce6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_list_high: {
    const bool crossed = (0xd9aau >> 8) != ((0xd9aau + registers->y) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xd9aau + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x868ce6u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8ce9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_list_high: {
    const uint32_t address = (0x001298u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsWramByte(address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x868ce9u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8cedu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
list_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x868cedu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x008eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8cefu);
}
store_list_bank: {
    const uint32_t address = (0x001238u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsWramByte(address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x868cefu, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8cf3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_data_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x868cf3u, 1u, span.instructions != 0u);
    const uint8_t bank = registers->read(registers->mem, ++registers->sp);
    registers->db = bank;
    registers->z = bank == 0u;
    registers->n = (bank & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x8cf4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
return_table: {
    if (!NativeSpanCanStep(&span, cpu, 6u))
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x868cf4u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x868cf4u, 0x6bu, true, true, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
