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

static int Lufia2DecompRunBattleSpriteTileFar(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool call = pc == 0x81bd47u;
    if ((!call && pc != 0x81bd4au) || opcode != (call ? 0x20u : 0x6bu) ||
        registers->e || registers->sp < 0x1f02u || registers->sp > 0x1ffcu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 6u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, call ? 3u : 1u, &offset);
    if (!cart || (call && memcmp(cart->rom + offset + 1u, "\x4b\xbd", 2u)))
        return 0;

    Lufia2SpanFetch(registers, pc, call ? 3u : 1u, false);
    if (call) {
        registers->write(registers->mem, registers->sp--, 0xbdu);
        registers->write(registers->mem, registers->sp--, 0x49u);
        registers->pc = 0xbd4bu;
    } else {
        uint16_t address = registers->read(registers->mem, ++registers->sp);
        address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        registers->k = registers->read(registers->mem, ++registers->sp);
        registers->pc = (uint16_t)(address + 1u);
    }
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    *tail_steps = (dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch) ? 0 : -1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 6;
}

static int Lufia2DecompRunBattleSpriteTileSetup(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                    uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool enter = pc == 0x81bd4bu;
    if ((!enter && pc != 0x81bd70u) || opcode != (enter ? 0x8bu : 0xa5u) ||
        registers->e || !registers->mf || registers->xf || registers->dp || (enter && registers->d) ||
        (enter && (registers->sp < 0x1f02u || registers->sp > 0x1ffdu)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (enter ? 72u : 6u))
        return 0;
    unsigned offset;
    const unsigned length = enter ? 41u : 4u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = enter ? "\xa9\x7e\x48\xab\x64\x15\x7b\x38\xe5\x07\xeb\xa5\x06\xa8"
        "\x84\x17\x84\x1c\xa5\x00\x0a\x85\x11\x29\xf0\x65\x11\x85\x11\xa5\x05\x3a"
        "\x85\x16\xa6\x08\xa5\x02\x85\x13" : "\x02\x85\x13";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    long progress = 0;
    if (enter) {
        Lufia2SpanFetch(registers, pc, 1u, false);
        registers->write(registers->mem, registers->sp--, registers->db);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, 0x81bd4cu, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, 0x7eu, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, 0x81bd4eu, 1u, true);
        registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 3;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, 0x81bd4fu, 1u, true);
        registers->db = registers->read(registers->mem, ++registers->sp);
        registers->n = (registers->db & 0x80u) != 0u;
        registers->z = registers->db == 0u;
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 4;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, 0x81bd50u, 2u, true);
        registers->write(registers->mem, 0x15u, 0u);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 5;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, 0x81bd52u, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, 0u, false);
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, 0x81bd53u, 1u, true);
        registers->c = true;
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, 0x81bd54u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xe9u, registers->read(registers->mem, 0x07u), true);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 8;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, 0x81bd56u, 1u, true);
        registers->a = (uint16_t)((registers->a >> 8) | (registers->a << 8));
        registers->n = (registers->a & 0x80u) != 0u;
        registers->z = (uint8_t)registers->a == 0u;
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, 0x81bd57u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, 0x06u), true);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 10;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, 0x81bd59u, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
        Lufia2SpanAccountInstruction(cpu, 2u);
        for (unsigned copy = 0; copy < 2u; ++copy) {
            const uint32_t field = copy ? 0x1cu : 0x17u;
            Lufia2SpanFetch(registers, 0x81bd5au + 2u * copy, 2u, true);
            Lufia2DecompWriteData(registers, field, field + 1u, registers->y, true);
            Lufia2SpanAccountInstruction(cpu, 4u);
            if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
                progress = 12u + copy;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        }

        Lufia2SpanFetch(registers, 0x81bd5eu, 2u, true);
        const uint8_t tile = registers->read(registers->mem, 0u);
        Lufia2DecompApplyOperand(registers, 0xa9u, tile, true);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 14;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, 0x81bd60u, 1u, true);
        registers->c = (tile & 0x80u) != 0u;
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(tile << 1), true);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, 0x81bd61u, 2u, true);
        registers->write(registers->mem, 0x11u, (uint8_t)registers->a);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 16;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, 0x81bd63u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0x29u, 0xf0u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, 0x81bd65u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0x69u, registers->read(registers->mem, 0x11u), true);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 18;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, 0x81bd67u, 2u, true);
        registers->write(registers->mem, 0x11u, (uint8_t)registers->a);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 19;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, 0x81bd69u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, 0x05u), true);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 20;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, 0x81bd6bu, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a - 1u), true);
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, 0x81bd6cu, 2u, true);
        registers->write(registers->mem, 0x16u, (uint8_t)registers->a);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 22;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, 0x81bd6eu, 2u, true);
        const uint16_t output = Lufia2DecompReadData(registers, 0x08u, 0x09u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, output, false);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 23;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    const long total = enter ? 25 : 2;
    Lufia2SpanFetch(registers, 0x81bd70u, 2u, enter);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, 0x02u), true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = total - 1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, 0x81bd72u, 2u, true);
    registers->write(registers->mem, 0x13u, (uint8_t)registers->a);
    registers->pc = 0xbd74u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = total;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? total - progress : 1 - total;
    return enter ? 72 : 6;
}

static int Lufia2DecompRunBattleSpriteTileFinish(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool next_row = pc == 0x81bdb3u;
    if ((!next_row && pc != 0x81bdc2u) || opcode != (next_row ? 0xa5u : 0x86u) ||
        registers->e || !registers->mf || registers->xf || registers->dp ||
        (next_row && registers->d) ||
        (!next_row && (registers->sp < 0x1f00u || registers->sp > 0x1ffcu)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (next_row ? 26u : 17u))
        return 0;

    unsigned offset;
    const unsigned length = next_row ? 15u : 6u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = next_row ? "\x16\x18\x69\x10\x85\x16\xa4\x1c\x84\x17\xc6\x03\xd0\xae" :
        "\x08\xa5\x15\xab\x60";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    long progress = 0;
    if (!next_row) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        Lufia2DecompWriteData(registers, 0x08u, 0x09u, registers->x, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 2u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, 0x15u), true);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 2;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 4u, 1u, true);
        registers->db = registers->read(registers->mem, ++registers->sp);
        registers->n = (registers->db & 0x80u) != 0u;
        registers->z = registers->db == 0u;
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 3;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanNearReturn(registers, cpu, pc + 5u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 4;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        *tail_steps = progress ? 4 - progress : -3;
        return 17;
    }

    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, 0x16u), true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 16u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    registers->write(registers->mem, 0x16u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    const uint16_t component = Lufia2DecompReadData(registers, 0x1cu, 0x1du, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, component, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 2u, true);
    Lufia2DecompWriteData(registers, 0x17u, 0x18u, registers->y, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 11u, 2u, true);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x03u, 0x04u, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 7;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 13u, 2u, true);
    registers->pc = registers->z ? 0xbdc2u : 0xbd70u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 8 - progress : -7;
    return 23u + registers->cyclesUsed;
}
