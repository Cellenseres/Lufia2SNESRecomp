static int Lufia2DecompRunBattleSpriteTileRecord(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                    uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const unsigned store_cycles = registers->xf ? 5u : 6u;
    const unsigned total_cycles = 5u * (store_cycles + 5u) + 2u;
    if (pc != 0x81bd74u || opcode != 0xa5u || registers->e || !registers->mf ||
        registers->dp || registers->db != 0x7eu ||
        (registers->xf ? registers->x > 0xffu : registers->x > 0xfffbu) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < total_cycles)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 32u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x17\x9d\x00\x00\xe8\xa5\x16\x9d\x00\x00\xe8"
        "\xa5\x11\x9d\x00\x00\xe8\xa5\x04\x9d\x00\x00\xe8\xa5\x18\x29\x03\x9d\x00\x00\xe8", 31u))
        return 0;

    static const uint8_t fields[] = {0x17u, 0x16u, 0x11u, 0x04u, 0x18u};
    long executed = 0;
    long progress = 0;
    for (unsigned field = 0; field < sizeof(fields); ++field) {
        const uint32_t site = pc + 6u * field;
        Lufia2SpanFetch(registers, site, 2u, field != 0u);
        const uint8_t value = registers->read(registers->mem, fields[field]);
        Lufia2DecompApplyOperand(registers, 0xa9u, value, true);
        Lufia2SpanAccountInstruction(cpu, 3u);
        ++executed;
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = executed;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        const bool attributes_high = field == 4u;
        if (attributes_high) {
            Lufia2SpanFetch(registers, site + 2u, 2u, true);
            Lufia2DecompApplyOperand(registers, 0x29u, 3u, true);
            Lufia2SpanAccountInstruction(cpu, 2u);
            ++executed;
        }

        const uint32_t store_site = site + (attributes_high ? 4u : 2u);
        Lufia2SpanFetch(registers, store_site, 3u, true);
        registers->write(registers->mem, 0x7e0000u | registers->x, (uint8_t)registers->a);
        Lufia2SpanAccountInstruction(cpu, store_cycles);
        ++executed;
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = executed;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, store_site + 3u, 1u, true);
        const uint16_t next = registers->x + 1u;
        Lufia2DecompApplyOperand(registers, 0xa2u, next, registers->xf);
        Lufia2SpanAccountInstruction(cpu, 2u);
        ++executed;
    }
    registers->pc = 0xbd94u;
    registers->cyclesUsed = 2u;
    *tail_steps = progress ? executed - progress : 1 - executed;
    return total_cycles;
}

static int Lufia2DecompRunBattleSpriteTileAdvance(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                     uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x81bd94u || opcode != 0xe6u || registers->e || !registers->mf ||
        registers->dp || registers->d || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 48u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 31u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x15\xc2\x20\xa5\x17\x18\x69\x10\x00\x85\x17"
        "\xe2\x20\xa5\x11\x18\x69\x02\x89\x0f\xd0\x02\x69\x10\x85\x11\xc6\x13\xd0\xc1", 30u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompModifyWram(registers, 0xe6u, 0x15u, 0x16u, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    long progress = (dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch) ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, 0x81bd96u, 2u, true);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, 0x81bd98u, 2u, true);
    const uint16_t component = Lufia2DecompReadData(registers, 0x17u, 0x18u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, component, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, 0x81bd9au, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, 0x81bd9bu, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x10u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, 0x81bd9eu, 2u, true);
    Lufia2DecompWriteData(registers, 0x17u, 0x18u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, 0x81bda0u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, 0x81bda2u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, 0x11u), true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, 0x81bda4u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, 0x81bda5u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 2u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, 0x81bda7u, 2u, true);
    registers->z = (registers->a & 0x0fu) == 0u;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, 0x81bda9u, 2u, true);
    const bool next_tile_row = registers->z;
    Lufia2SpanAccountInstruction(cpu, next_tile_row ? 2u : 3u);
    if (next_tile_row) {
        Lufia2SpanFetch(registers, 0x81bdabu, 2u, true);
        Lufia2DecompApplyOperand(registers, 0x69u, 0x10u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    const long store = next_tile_row ? 14 : 13;
    Lufia2SpanFetch(registers, 0x81bdadu, 2u, true);
    registers->write(registers->mem, 0x11u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = store;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, 0x81bdafu, 2u, true);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x13u, 0x14u, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = store + 1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, 0x81bdb1u, 2u, true);
    registers->pc = registers->z ? 0xbdb3u : 0xbd74u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? store + 2 - progress : -store - 1;
    return 44u + next_tile_row + registers->cyclesUsed;
}
