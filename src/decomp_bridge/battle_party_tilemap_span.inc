static int Lufia2DecompRunBattlePartyTilemapSlot(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                       uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool check_slot = pc == 0x858d5bu;
    if ((!check_slot && pc != 0x858db5u) || opcode != (check_slot ? 0xb9u : 0xc2u) ||
        registers->e || !registers->mf || registers->xf || registers->dp ||
        (check_slot && (registers->db != 0x7eu || registers->y > 0xec24u)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (check_slot ? 8u : 23u))
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, check_slot ? 5u : 14u, &offset);
    if (!cart || (check_slot ? memcmp(cart->rom + offset + 1u, "\xdb\x13\x10\x55", 4u) :
        memcmp(cart->rom + offset + 1u, "\x20\x98\x18\x69\x0f\x00\xa8\xe2\x20\xc6\x05\xd0\x98", 13u)))
        return 0;

    if (check_slot) {
        Lufia2SpanFetch(registers, pc, 3u, false);
        const uint8_t status = registers->read(registers->mem, 0x7e13dbu + registers->y);
        Lufia2DecompApplyOperand(registers, 0xa9u, status, true);
        const unsigned read_cycles = 4u + (registers->y >= 0x25u);
        Lufia2SpanAccountInstruction(cpu, read_cycles);
        const bool status_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 3u, 2u, true);
        registers->pc = registers->n ? 0x8d60u : 0x8db5u;
        registers->cyclesUsed = registers->n ? 2u : 3u;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        *tail_steps = status_progress ? 1 : -1;
        return read_cycles + registers->cyclesUsed;
    }

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 4u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 15u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 7u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x05u, 0x06u, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    const bool counter_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 12u, 2u, true);
    registers->pc = registers->z ? 0x8dc3u : 0x8d5bu;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = counter_progress ? 1 : -7;
    return 20u + registers->cyclesUsed;
}

static int Lufia2DecompRunBattlePartyTilemapRecord(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                         uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x858d60u || opcode != 0x5au || registers->e || !registers->mf || registers->xf ||
        registers->db != 0x7eu || registers->dp || registers->y > 0xec17u ||
        registers->sp < 0x1f02u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 56u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 32u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\xc2\x20\xb9\xe7\x13\x85\x02\xe2\x20\xb9\xe1\x13"
        "\xeb\xb9\xdf\x13\xc2\x20\x18\x69\x00\x01\x4a\x4a\x4a\x29\x1f\x1f\xe2\x20\xeb", 31u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->write(registers->mem, registers->sp--, (uint8_t)(registers->y >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->y);
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 2u, true);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    const uint32_t record = 0x7e13dbu + registers->y;
    Lufia2SpanFetch(registers, pc + 3u, 3u, true);
    const uint16_t size = Lufia2DecompReadData(registers, record + 12u, record + 13u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, size, false);
    const unsigned size_extra = registers->y >= 0x19u;
    Lufia2SpanAccountInstruction(cpu, 5u + size_extra);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    Lufia2DecompWriteData(registers, 0x02u, 0x03u, size, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 10u, 3u, true);
    const uint8_t row = registers->read(registers->mem, record + 6u);
    Lufia2DecompApplyOperand(registers, 0xa9u, row, true);
    const unsigned row_extra = registers->y >= 0x1fu;
    Lufia2SpanAccountInstruction(cpu, 4u + row_extra);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 13u, 1u, true);
    MenuGlyphExchangeBytes(registers);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 14u, 3u, true);
    const uint8_t column = registers->read(registers->mem, record + 4u);
    Lufia2DecompApplyOperand(registers, 0xa9u, column, true);
    const unsigned column_extra = registers->y >= 0x21u;
    Lufia2SpanAccountInstruction(cpu, 4u + column_extra);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 17u, 2u, true);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 19u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 20u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0100u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    for (unsigned shift = 0; shift < 3u; ++shift) {
        Lufia2SpanFetch(registers, pc + 23u + shift, 1u, true);
        registers->c = (registers->a & 1u) != 0u;
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->a >> 1, false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }
    Lufia2SpanFetch(registers, pc + 26u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x1f1fu, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 29u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 31u, 1u, true);
    MenuGlyphExchangeBytes(registers);
    registers->pc = 0x8d80u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 17 - progress : -16;
    return 53u + size_extra + row_extra + column_extra;
}

static int Lufia2DecompRunBattlePartyTilemapMultiply(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                            uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x858d80u || opcode != 0x8fu || registers->e || !registers->mf || registers->xf ||
        registers->dp || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 40u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 28u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x02\x42\x00\xa9\x40\x8f\x03\x42\x00\xa9\x00"
        "\xeb\x0a\xc2\x20\x18\x6f\x16\x42\x00\x69\x00\x28\x85\x08\xe2\x20", 27u))
        return 0;

    Lufia2SpanFetch(registers, pc, 4u, false);
    registers->write(registers->mem, 0x4202u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 5u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x40u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 6u, 4u, true);
    registers->write(registers->mem, 0x4203u, 0x40u);
    Lufia2SpanAccountInstruction(cpu, 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 12u, 1u, true);
    MenuGlyphExchangeBytes(registers);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 13u, 1u, true);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a << 1), true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 14u, 2u, true);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 16u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 17u, 4u, true);
    const uint16_t product = Lufia2DecompReadData(registers, 0x4216u, 0x4217u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, product, false);
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 9;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 21u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x2800u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 24u, 2u, true);
    Lufia2DecompWriteData(registers, 0x08u, 0x09u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 11;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 26u, 2u, true);
    registers->mf = true;
    registers->pc = 0x8d9cu;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 12 - progress : -11;
    return 40;
}
