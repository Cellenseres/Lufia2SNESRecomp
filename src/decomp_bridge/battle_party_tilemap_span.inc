typedef struct BattleTilemapSlotLoop {
    uint16_t check_pc;
    uint16_t advance_pc;
    uint16_t status_offset;
    uint8_t stride;
    uint8_t counter;
} BattleTilemapSlotLoop;

static int Lufia2DecompRunBattlePartyTilemapSlot(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                       uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    static const BattleTilemapSlotLoop loops[] = {
        {0x8b67u, 0x8bb0u, 0x139au, 13u, 0x0du},
        {0x8c3bu, 0x8c82u, 0x1435u, 13u, 0x0du},
        {0x8d5bu, 0x8db5u, 0x13dbu, 15u, 0x05u},
    };
    if (pc < 0x858b00u || pc >= 0x858e00u)
        return 0;
    const BattleTilemapSlotLoop *loop = &loops[(pc - 0x858b00u) >> 8];
    const bool check_slot = (uint16_t)pc == loop->check_pc;
    const uint16_t status_offset = loop->status_offset;
    const uint32_t status_address = ((uint32_t)registers->db << 16) + status_offset + registers->y;
    if ((!check_slot && (uint16_t)pc != loop->advance_pc) || opcode != (check_slot ? 0xb9u : 0xc2u) ||
        registers->e || !registers->mf || registers->xf || registers->dp ||
        (check_slot && (registers->y > 0xffffu - status_offset || !Lufia2DecompIsWramByte(status_address))) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (check_slot ? 8u : 23u))
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, check_slot ? 5u : 14u, &offset);
    const uint8_t stride = loop->stride;
    const uint8_t counter = loop->counter;
    const uint8_t check_bytes[] = {
        (uint8_t)status_offset, (uint8_t)(status_offset >> 8), 0x10u, (uint8_t)(loop->advance_pc - loop->check_pc - 5),
    };
    const uint8_t advance_bytes[] = {
        0x20u, 0x98u, 0x18u, 0x69u, stride, 0x00u, 0xa8u, 0xe2u,
        0x20u, 0xc6u, counter, 0xd0u, (uint8_t)(loop->check_pc - loop->advance_pc - 14),
    };
    if (!cart || (check_slot ? memcmp(cart->rom + offset + 1u, check_bytes, sizeof(check_bytes)) :
        memcmp(cart->rom + offset + 1u, advance_bytes, sizeof(advance_bytes))))
        return 0;

    if (check_slot) {
        Lufia2SpanFetch(registers, pc, 3u, false);
        const uint8_t status = registers->read(registers->mem, status_address);
        Lufia2DecompApplyOperand(registers, 0xa9u, status, true);
        const unsigned read_cycles = 4u + ((status_offset & 0xffu) + registers->y >= 0x100u);
        Lufia2SpanAccountInstruction(cpu, read_cycles);
        const bool status_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 3u, 2u, true);
        registers->pc = registers->n ? (uint16_t)(pc + 5u) : loop->advance_pc;
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
    Lufia2DecompApplyOperand(registers, 0x69u, stride, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 7u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    Lufia2DecompModifyWram(registers, 0xc6u, counter, counter + 1u, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    const bool counter_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 12u, 2u, true);
    registers->pc = registers->z ? (uint16_t)(pc + 14u) : loop->check_pc;
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

static int Lufia2DecompRunBattlePartyTilemapAttributes(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                         uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x858d9cu || opcode != 0xb9u || registers->e || !registers->mf || registers->xf ||
        registers->dp || registers->db != 0x7eu || registers->y > 0xec21u ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 30u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 20u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\xdd\x13\x18\x6d\xb7\x15\x29\x07\x0a\x0a"
        "\x09\x23\x85\x04\xb9\xde\x13\x85\x00", 19u))
        return 0;

    const unsigned first_read_cycles = 4u + (registers->y >= 0x23u);
    const unsigned second_read_cycles = 4u + (registers->y >= 0x22u);
    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint8_t palette = registers->read(registers->mem, 0x7e13ddu + registers->y);
    Lufia2DecompApplyOperand(registers, 0xa9u, palette, true);
    Lufia2SpanAccountInstruction(cpu, first_read_cycles);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 4u, 3u, true);
    const uint8_t palette_offset = registers->read(registers->mem, 0x7e15b7u);
    Lufia2DecompApplyOperand(registers, 0x69u, palette_offset, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 7u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    for (unsigned shift = 0; shift < 2u; ++shift) {
        Lufia2SpanFetch(registers, pc + 9u + shift, 1u, true);
        registers->c = (registers->a & 0x80u) != 0u;
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a << 1), true);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }
    Lufia2SpanFetch(registers, pc + 11u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x09u, 0x23u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 13u, 2u, true);
    registers->write(registers->mem, 0x04u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 15u, 3u, true);
    const uint8_t tile = registers->read(registers->mem, 0x7e13deu + registers->y);
    Lufia2DecompApplyOperand(registers, 0xa9u, tile, true);
    Lufia2SpanAccountInstruction(cpu, second_read_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 9;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 18u, 2u, true);
    registers->write(registers->mem, 0x00u, tile);
    registers->pc = 0x8db0u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 10;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 10 - progress : -9;
    return 20u + first_read_cycles + second_read_cycles;
}

static int Lufia2DecompRunBattleTilemapLayout(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x858b6cu || opcode != 0x5au || registers->e || !registers->mf || registers->xf ||
        registers->dp || registers->y > 0xec5bu ||
        !Lufia2DecompIsWramByte(bank + 0x13a4u + registers->y) ||
        registers->sp < 0x1f02u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 57u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 33u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x7b\xb9\xa4\x13\x18\x79\xa0\x13\x1a\x85\x05\xa2\x02\x02\x86\x02"
        "\xc2\x20\xb9\xa2\x13\x18\x79\x9e\x13\x29\xff\x01\x85\x06\xe2\x20", 32u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->write(registers->mem, registers->sp--, (uint8_t)(registers->y >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->y);
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->dp, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    const uint32_t record = bank + 0x139au + registers->y;
    const unsigned size_cycles = 4u + (registers->y >= 0x5cu);
    Lufia2SpanFetch(registers, pc + 2u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, record + 10u), true);
    Lufia2SpanAccountInstruction(cpu, size_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 5u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    const unsigned size_add_cycles = 4u + (registers->y >= 0x60u);
    Lufia2SpanFetch(registers, pc + 6u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, registers->read(registers->mem, record + 6u), true);
    Lufia2SpanAccountInstruction(cpu, size_add_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    registers->write(registers->mem, 0x05u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 7;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 12u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0202u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 15u, 2u, true);
    Lufia2DecompWriteData(registers, 0x02u, 0x03u, registers->x, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 9;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 17u, 2u, true);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    const unsigned position_cycles = 5u + (registers->y >= 0x5eu);
    Lufia2SpanFetch(registers, pc + 19u, 3u, true);
    const uint16_t position = Lufia2DecompReadData(registers, record + 8u, record + 9u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, position, false);
    Lufia2SpanAccountInstruction(cpu, position_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 11;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 22u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    const unsigned position_add_cycles = 5u + (registers->y >= 0x62u);
    Lufia2SpanFetch(registers, pc + 23u, 3u, true);
    const uint16_t displacement = Lufia2DecompReadData(registers, record + 4u, record + 5u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, displacement, false);
    Lufia2SpanAccountInstruction(cpu, position_add_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 13;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 26u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x01ffu, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 29u, 2u, true);
    Lufia2DecompWriteData(registers, 0x06u, 0x07u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 15;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 31u, 2u, true);
    registers->mf = true;
    registers->pc = 0x8b8du;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 16 - progress : -15;
    return 35u + size_cycles + size_add_cycles + position_cycles + position_add_cycles;
}

static int Lufia2DecompRunBattleTilemapLayoutStyle(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                       uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x858b8du || opcode != 0xadu || registers->e || !registers->mf || registers->xf ||
        registers->dp || registers->y > 0xec62u ||
        !Lufia2DecompIsWramByte(bank + 0x139du + registers->y) ||
        !Lufia2DecompIsWramByte(bank + 0x15b5u) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 35u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 23u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\xb5\x15\x18\x79\x9c\x13\x29\x07\x0a\x0d\x75\x14\x19\x9b\x13"
        "\x85\x04\xb9\x9d\x13\x85\x00", 22u))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, bank + 0x15b5u), true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    const unsigned palette_cycles = 4u + (registers->y >= 0x64u);
    const uint32_t record = bank + 0x139au + registers->y;
    Lufia2SpanFetch(registers, pc + 4u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, registers->read(registers->mem, record + 2u), true);
    Lufia2SpanAccountInstruction(cpu, palette_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 7u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 9u, 1u, true);
    registers->c = false;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a << 1), true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 10u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x09u, registers->read(registers->mem, bank + 0x1475u), true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    const unsigned flags_cycles = 4u + (registers->y >= 0x65u);
    Lufia2SpanFetch(registers, pc + 13u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x09u, registers->read(registers->mem, record + 1u), true);
    Lufia2SpanAccountInstruction(cpu, flags_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 7;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 16u, 2u, true);
    registers->write(registers->mem, 0x04u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    const unsigned tile_cycles = 4u + (registers->y >= 0x63u);
    Lufia2SpanFetch(registers, pc + 18u, 3u, true);
    const uint8_t tile = registers->read(registers->mem, record + 3u);
    Lufia2DecompApplyOperand(registers, 0xa9u, tile, true);
    Lufia2SpanAccountInstruction(cpu, tile_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 9;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 21u, 2u, true);
    registers->write(registers->mem, 0x00u, tile);
    registers->pc = 0x8ba4u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 10;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 10 - progress : -9;
    return 20u + palette_cycles + flags_cycles + tile_cycles;
}
