#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunBuildEquipmentGroups(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                               uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                               unsigned step_budget, long *step_count) {
    const uint32_t phase_end = pc < 0x82bd70u   ? 0x82bd70u
                               : pc < 0x82bd78u ? 0x82bd78u
                               : pc < 0x82bd7eu ? 0x82bd7eu
                               : pc < 0x82bd82u ? 0x82bd82u
                               : pc < 0x82bd87u ? 0x82bd87u
                               : pc < 0x82bd8bu ? 0x82bd8bu
                               : pc < 0x82bd8eu ? 0x82bd8eu
                               : pc < 0x82bd96u ? 0x82bd96u
                               : pc < 0x82bd99u ? 0x82bd99u
                                                : 0x82bd9bu;
    if (pc < 0x82bd48u || pc >= 0x82bd9bu || !cpu || !cpu->ram || registers->e || !step_budget ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        (pc + 1u < phase_end && !Lufia2DecompFrameRangeQuiet(pc + 1u, phase_end)))
        return 0;
    const bool any_width = pc <= 0x82bd49u || pc >= 0x82bd99u;
    const bool changing_width = pc == 0x82bd4cu || pc == 0x82bd52u;
    const bool byte_entry = pc >= 0x82bd4eu && pc <= 0x82bd51u;
    if (!any_width && (registers->xf || (!changing_width && registers->mf != byte_entry)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x82bd48u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 83u ||
        memcmp(cart->rom + first,
               "\x08\xc2\x30\x8b\xe2\x20\xa9\x7e\x48\xab\xc2\x20\x9c\xe2\x9c\x9c\xe4\x9c\x9c\xe6\x9c\x9c\xe8"
               "\x9c\x9c\xea\x9c\x9c\xec\x9c\x9c\xee\x9c\xab\x9c\xcd\x09\xa2\x00\x00\xda\x86\x08\xbd\x8d\x0a"
               "\xf0\x16\xee\xcd\x09\x8d\x06\x0a\x22\x94\xf1\x81\x29\x02\x00\xf0\x07\x22\xab\xf1\x81\x20\x9b"
               "\xbd\xfa\xe8\xe8\xe0\xc0\x00\xd0\xda\x20\xdb\xbd\x28\x60",
               83u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x82bd48u: goto save_status;
    case 0x82bd49u: goto widen_registers;
    case 0x82bd4bu: goto save_bank;
    case 0x82bd4cu: goto byte_work_bank;
    case 0x82bd4eu: goto load_work_bank;
    case 0x82bd50u: goto push_work_bank;
    case 0x82bd51u: goto select_work_bank;
    case 0x82bd52u: goto widen_records;
    case 0x82bd54u: goto clear_category_0;
    case 0x82bd57u: goto clear_category_1;
    case 0x82bd5au: goto clear_category_2;
    case 0x82bd5du: goto clear_category_3;
    case 0x82bd60u: goto clear_category_4;
    case 0x82bd63u: goto clear_category_5;
    case 0x82bd66u: goto clear_category_6;
    case 0x82bd69u: goto restore_inventory_bank;
    case 0x82bd6au: goto clear_item_count;
    case 0x82bd6du: goto first_item;
    case 0x82bd70u: goto save_item_index;
    case 0x82bd71u: goto store_item_offset;
    case 0x82bd73u: goto read_item;
    case 0x82bd76u: goto skip_empty;
    case 0x82bd78u: goto count_item;
    case 0x82bd7bu: goto select_item;
    case 0x82bd7eu: goto read_item_record;
    case 0x82bd82u: goto test_equippable;
    case 0x82bd85u: goto skip_ineligible;
    case 0x82bd87u: goto update_item_stats;
    case 0x82bd8bu: goto append_item_group;
    case 0x82bd8eu: goto restore_item_index;
    case 0x82bd8fu: goto next_item_low;
    case 0x82bd90u: goto next_item_high;
    case 0x82bd91u: goto test_item_end;
    case 0x82bd94u: goto repeat_items;
    case 0x82bd96u: goto terminate_groups;
    case 0x82bd99u: goto restore_status;
    case 0x82bd9au: goto return_short;
    default: return 0;
    }
save_status:
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f00u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd48u, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, (uint8_t)interp816_getFlags(registers));
    NativeSpanStep(&span, registers, cpu, 3u, 0xbd49u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
widen_registers:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd49u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x30u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbd4bu);
save_bank:
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f00u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd4bu, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->db);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbd4cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
byte_work_bank:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd4cu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbd4eu);
load_work_bank:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd4eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x007eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbd50u);
push_work_bank:
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f00u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd50u, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbd51u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
select_work_bank:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd51u, 1u, span.instructions != 0u);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0xbd52u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
widen_records:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd52u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbd54u);
clear_category_0: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x9ce2u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd54u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbd57u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_category_1: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x9ce4u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd57u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbd5au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_category_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x9ce6u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd5au, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbd5du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_category_3: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x9ce8u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd5du, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbd60u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_category_4: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x9ceau) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd60u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbd63u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_category_5: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x9cecu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd63u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbd66u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_category_6: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x9ceeu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd66u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbd69u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_inventory_bank:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd69u, 1u, span.instructions != 0u);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0xbd6au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
clear_item_count: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09cdu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd6au, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbd6du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_item:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd6du, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbd70u);
    goto finished;
save_item_index:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd70u, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, (uint8_t)(registers->x >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->x);
    NativeSpanStep(&span, registers, cpu, 4u, 0xbd71u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
store_item_offset: {
    const uint32_t address = (uint16_t)(registers->dp + 0x08u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd71u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbd73u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_item: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0a8du + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0a8du >> 8) != ((0x0a8du + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd73u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbd76u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_empty: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd76u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xbd8eu : 0xbd78u);
    goto finished;
}
count_item: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09cdu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 8u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd78u, 3u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbd7bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_item: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0a06u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd7bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbd7eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_item_record:
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    {
        if (span.instructions)
            Lufia2SpanProbeOpcode(registers, 0x82bd7eu);
        const int cycles =
            Lufia2DecompRunDirectCallWithWidths(registers, 0x82bd7eu, 0x22u, 0x81f194u, true, false, false);
        if (!cycles)
            goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
test_equippable:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd82u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x0002u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbd85u);
skip_ineligible: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd85u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xbd8eu : 0xbd87u);
    goto finished;
}
update_item_stats:
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    {
        if (span.instructions)
            Lufia2SpanProbeOpcode(registers, 0x82bd87u);
        const int cycles =
            Lufia2DecompRunDirectCallWithWidths(registers, 0x82bd87u, 0x22u, 0x81f1abu, true, false, false);
        if (!cycles)
            goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
append_item_group:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    {
        if (span.instructions)
            Lufia2SpanProbeOpcode(registers, 0x82bd8bu);
        const int cycles =
            Lufia2DecompRunDirectCallWithWidths(registers, 0x82bd8bu, 0x20u, 0x82bd9bu, false, false, false);
        if (!cycles)
            goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
restore_item_index:
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd8eu, 1u, span.instructions != 0u);
    {
        uint16_t value = registers->read(registers->mem, ++registers->sp);
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        Lufia2DecompApplyOperand(registers, 0xa2u, value, false);
    }
    NativeSpanStep(&span, registers, cpu, 5u, 0xbd8fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
next_item_low:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd8fu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbd90u);
next_item_high:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd90u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbd91u);
test_item_end:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd91u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, 0x00c0u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbd94u);
repeat_items: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd94u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xbd70u : 0xbd96u);
    goto finished;
}
terminate_groups:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    {
        if (span.instructions)
            Lufia2SpanProbeOpcode(registers, 0x82bd96u);
        const int cycles =
            Lufia2DecompRunDirectCallWithWidths(registers, 0x82bd96u, 0x20u, 0x82bddbu, false, false, false);
        if (!cycles)
            goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
restore_status:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd99u, 1u, span.instructions != 0u);
    interp816_setFlags(registers, registers->read(registers->mem, ++registers->sp));
    NativeSpanStep(&span, registers, cpu, 4u, 0xbd9au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
return_short:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    {
        if (span.instructions)
            Lufia2SpanProbeOpcode(registers, 0x82bd9au);
        const int cycles = Lufia2DecompRunReturn(registers, 0x82bd9au, 0x60u, false, registers->mf, true);
        if (!cycles)
            goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunGroupEquipmentItem(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                             uint64_t dynamic_epoch, uint64_t write_epoch,
                                             unsigned step_budget, long *step_count) {
    if (pc < 0x82bd9bu || pc >= 0x82bddbu || !cpu || !cpu->ram || registers->e || !step_budget ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x82bd9cu, 0x82bddbu))
        return 0;
    const bool changing_width = pc == 0x82bda4u || pc == 0x82bdaau;
    const bool byte_entry = pc >= 0x82bda6u && pc <= 0x82bda9u;
    if (registers->xf || (pc != 0x82bddau && !changing_width && registers->mf != byte_entry))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x82bd9bu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 64u ||
        memcmp(cart->rom + first,
               "\xad\x8b\x0b\x29\xff\x00\x85\x00\x8b\xe2\x20\xa9\x7e\x48\xab\xc2\x20\xa2\x00\x00\xbf\x68\xe5"
               "\x8e\xf0\x08\x24\x00\xd0\x04\xe8\xe8\x80\xf2\xbf\x50\xe5\x8e\xa8\xb9\x00\x00\x85\x00\x1a\x1a"
               "\x99\x00\x00\xbf\x38\xe5\x8e\x18\x65\x00\xa8\xa5\x08\x99\x00\x00\xab\x60",
               64u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x82bd9bu: goto read_category_flags;
    case 0x82bd9eu: goto mask_category;
    case 0x82bda1u: goto save_category;
    case 0x82bda3u: goto save_bank;
    case 0x82bda4u: goto byte_work_bank;
    case 0x82bda6u: goto load_work_bank;
    case 0x82bda8u: goto push_work_bank;
    case 0x82bda9u: goto select_work_bank;
    case 0x82bdaau: goto widen_records;
    case 0x82bdacu: goto first_category;
    case 0x82bdafu: goto read_mask;
    case 0x82bdb3u: goto catch_sentinel;
    case 0x82bdb5u: goto test_mask;
    case 0x82bdb7u: goto match_category;
    case 0x82bdb9u: goto next_mask_low;
    case 0x82bdbau: goto next_mask_high;
    case 0x82bdbbu: goto repeat_masks;
    case 0x82bdbdu: goto read_length_pointer;
    case 0x82bdc1u: goto index_length;
    case 0x82bdc2u: goto read_length;
    case 0x82bdc5u: goto save_length;
    case 0x82bdc7u: goto increment_length_low;
    case 0x82bdc8u: goto increment_length_high;
    case 0x82bdc9u: goto publish_length;
    case 0x82bdccu: goto read_group_pointer;
    case 0x82bdd0u: goto clear_group_carry;
    case 0x82bdd1u: goto add_length;
    case 0x82bdd3u: goto index_output;
    case 0x82bdd4u: goto read_item_offset;
    case 0x82bdd6u: goto append_item;
    case 0x82bdd9u: goto restore_bank;
    case 0x82bddau: goto return_short;
    default: return 0;
    }
read_category_flags: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0b8bu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd9bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbd9eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_category:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bd9eu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbda1u);
save_category: {
    const uint32_t address = (uint16_t)(registers->dp + 0x00u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bda1u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbda3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_bank:
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f00u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82bda3u, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->db);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbda4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
byte_work_bank:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bda4u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbda6u);
load_work_bank:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bda6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x007eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbda8u);
push_work_bank:
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f00u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82bda8u, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbda9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
select_work_bank:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82bda9u, 1u, span.instructions != 0u);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0xbdaau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
widen_records:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdaau, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbdacu);
first_category:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdacu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbdafu);
read_mask: {
    const uint32_t address = (0x8ee568u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdafu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbdb3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
catch_sentinel: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdb3u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xbdbdu : 0xbdb5u);
    if (taken)
        goto read_length_pointer;
}
test_mask: {
    const uint32_t address = (uint16_t)(registers->dp + 0x00u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdb5u, 2u, span.instructions != 0u);
    const uint16_t value = Lufia2DecompReadData(registers, address, high, true);
    registers->z = (registers->a & value) == 0u;
    registers->n = (value & 0x8000u) != 0u;
    registers->v = (value & 0x4000u) != 0u;
    NativeSpanStep(&span, registers, cpu, cycles, 0xbdb7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
match_category: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdb7u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xbdbdu : 0xbdb9u);
    if (taken)
        goto read_length_pointer;
}
next_mask_low:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdb9u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbdbau);
next_mask_high:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdbau, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbdbbu);
repeat_masks:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdbbu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbdafu);
    goto read_mask;
read_length_pointer: {
    const uint32_t address = (0x8ee550u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdbdu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbdc1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
index_length:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdc1u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbdc2u);
read_length: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0000u >> 8) != ((0x0000u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdc2u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbdc5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_length: {
    const uint32_t address = (uint16_t)(registers->dp + 0x00u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdc5u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbdc7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
increment_length_low:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdc7u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbdc8u);
increment_length_high:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdc8u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbdc9u);
publish_length: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdc9u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbdccu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_group_pointer: {
    const uint32_t address = (0x8ee538u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdccu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbdd0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_group_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdd0u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xbdd1u);
add_length: {
    const uint32_t address = (uint16_t)(registers->dp + 0x00u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdd1u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbdd3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
index_output:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdd3u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbdd4u);
read_item_offset: {
    const uint32_t address = (uint16_t)(registers->dp + 0x08u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdd4u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbdd6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
append_item: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdd6u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbdd9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_bank:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdd9u, 1u, span.instructions != 0u);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0xbddau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
return_short:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    {
        if (span.instructions)
            Lufia2SpanProbeOpcode(registers, 0x82bddau);
        const int cycles = Lufia2DecompRunReturn(registers, 0x82bddau, 0x60u, false, registers->mf, true);
        if (!cycles)
            goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunFinishEquipmentGroups(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                unsigned step_budget, long *step_count) {
    if (pc < 0x82bddbu || pc >= 0x82be04u || !cpu || !cpu->ram || registers->e || !step_budget ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x82bddcu, 0x82be04u))
        return 0;
    const bool changing_width = pc == 0x82bddcu || pc == 0x82bde2u;
    const bool byte_entry = pc >= 0x82bddeu && pc <= 0x82bde1u;
    if (registers->xf ||
        (pc != 0x82bddbu && pc != 0x82be03u && !changing_width && registers->mf != byte_entry))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x82bddbu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 41u ||
        memcmp(cart->rom + first,
               "\x8b\xe2\x20\xa9\x7e\x48\xab\xc2\x20\xa2\x00\x00\xbf\x50\xe5\x8e\xa8\xb9\x00\x00\x18\x7f\x38"
               "\xe5\x8e\xa8\xa9\xff\xff\x99\x00\x00\xe8\xe8\xe0\x0c\x00\xd0\xe5\xab\x60",
               41u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x82bddbu: goto save_bank;
    case 0x82bddcu: goto byte_work_bank;
    case 0x82bddeu: goto load_work_bank;
    case 0x82bde0u: goto push_work_bank;
    case 0x82bde1u: goto select_work_bank;
    case 0x82bde2u: goto widen_records;
    case 0x82bde4u: goto first_group;
    case 0x82bde7u: goto read_length_pointer;
    case 0x82bdebu: goto index_length;
    case 0x82bdecu: goto read_length;
    case 0x82bdefu: goto clear_end_carry;
    case 0x82bdf0u: goto add_group_pointer;
    case 0x82bdf4u: goto index_end;
    case 0x82bdf5u: goto load_terminator;
    case 0x82bdf8u: goto store_terminator;
    case 0x82bdfbu: goto next_group_low;
    case 0x82bdfcu: goto next_group_high;
    case 0x82bdfdu: goto test_group_end;
    case 0x82be00u: goto repeat_groups;
    case 0x82be02u: goto restore_bank;
    case 0x82be03u: goto return_short;
    default: return 0;
    }
save_bank:
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f00u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82bddbu, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->db);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbddcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
byte_work_bank:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bddcu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbddeu);
load_work_bank:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bddeu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x007eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbde0u);
push_work_bank:
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f00u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82bde0u, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbde1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
select_work_bank:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82bde1u, 1u, span.instructions != 0u);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0xbde2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
widen_records:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bde2u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbde4u);
first_group:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bde4u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbde7u);
read_length_pointer: {
    const uint32_t address = (0x8ee550u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bde7u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbdebu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
index_length:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdebu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbdecu);
read_length: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0000u >> 8) != ((0x0000u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdecu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbdefu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_end_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdefu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xbdf0u);
add_group_pointer: {
    const uint32_t address = (0x8ee538u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdf0u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbdf4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
index_end:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdf4u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbdf5u);
load_terminator:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdf5u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0xffffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbdf8u);
store_terminator: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdf8u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbdfbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_group_low:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdfbu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbdfcu);
next_group_high:
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdfcu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbdfdu);
test_group_end:
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82bdfdu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, 0x000cu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbe00u);
repeat_groups: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82be00u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xbde7u : 0xbe02u);
    if (taken)
        goto read_length_pointer;
}
restore_bank:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82be02u, 1u, span.instructions != 0u);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0xbe03u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
return_short:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    {
        if (span.instructions)
            Lufia2SpanProbeOpcode(registers, 0x82be03u);
        const int cycles = Lufia2DecompRunReturn(registers, 0x82be03u, 0x60u, false, registers->mf, true);
        if (!cycles)
            goto finished;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
    goto finished;
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuEquipmentSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                            uint64_t dynamic_epoch, uint64_t write_epoch,
                                            unsigned step_budget, long *step_count) {
    if (pc < 0x82bd9bu)
        return Lufia2DecompRunBuildEquipmentGroups(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                   step_budget, step_count);
    if (pc < 0x82bddbu)
        return Lufia2DecompRunGroupEquipmentItem(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                 step_budget, step_count);
    return Lufia2DecompRunFinishEquipmentGroups(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                step_budget, step_count);
}
