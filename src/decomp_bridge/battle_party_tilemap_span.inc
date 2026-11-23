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
    const bool fixed_record = pc == 0x858bd7u;
    const uint32_t bank = (uint32_t)registers->db << 16;
    if ((!fixed_record && pc != 0x858b6cu) || opcode != (fixed_record ? 0x7bu : 0x5au) || registers->e || !registers->mf || registers->xf ||
        registers->dp ||
        (!fixed_record && (registers->y > 0xec5bu || registers->sp < 0x1f02u || registers->sp > 0x1fffu)) ||
        !Lufia2DecompIsWramByte(bank + (fixed_record ? 0x13d8u : 0x13a4u + registers->y)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (fixed_record ? 49u : 57u))
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, fixed_record ? 32u : 33u, &offset);
    if (!cart || (fixed_record ? memcmp(cart->rom + offset + 1u,
        "\xad\xd8\x13\x18\x6d\xd4\x13\x1a\x85\x05\xa2\x03\x03\x86\x02"
        "\xc2\x20\xad\xd6\x13\x18\x6d\xd2\x13\x29\xff\x01\x85\x06\xe2\x20", 31u) :
        memcmp(cart->rom + offset + 1u,
        "\x7b\xb9\xa4\x13\x18\x79\xa0\x13\x1a\x85\x05\xa2\x02\x02\x86\x02"
        "\xc2\x20\xb9\xa2\x13\x18\x79\x9e\x13\x29\xff\x01\x85\x06\xe2\x20", 32u)))
        return 0;

    long progress = 0;
    if (!fixed_record) {
        Lufia2SpanFetch(registers, pc, 1u, false);
        registers->write(registers->mem, registers->sp--, (uint8_t)(registers->y >> 8));
        registers->write(registers->mem, registers->sp--, (uint8_t)registers->y);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }
    const uint32_t body_pc = pc + !fixed_record;
    Lufia2SpanFetch(registers, body_pc, 1u, !fixed_record);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->dp, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    const uint32_t record = bank + (fixed_record ? 0x13ceu : 0x139au + registers->y);
    const unsigned size_cycles = 4u + (!fixed_record && registers->y >= 0x5cu);
    Lufia2SpanFetch(registers, body_pc + 1u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, record + 10u), true);
    Lufia2SpanAccountInstruction(cpu, size_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, body_pc + 4u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    const unsigned size_add_cycles = 4u + (!fixed_record && registers->y >= 0x60u);
    Lufia2SpanFetch(registers, body_pc + 5u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, registers->read(registers->mem, record + 6u), true);
    Lufia2SpanAccountInstruction(cpu, size_add_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, body_pc + 8u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a + 1u), true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, body_pc + 9u, 2u, true);
    registers->write(registers->mem, 0x05u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 7;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, body_pc + 11u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, fixed_record ? 0x0303u : 0x0202u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, body_pc + 14u, 2u, true);
    Lufia2DecompWriteData(registers, 0x02u, 0x03u, registers->x, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 9;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, body_pc + 16u, 2u, true);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    const unsigned position_cycles = 5u + (!fixed_record && registers->y >= 0x5eu);
    Lufia2SpanFetch(registers, body_pc + 18u, 3u, true);
    const uint16_t position = Lufia2DecompReadData(registers, record + 8u, record + 9u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, position, false);
    Lufia2SpanAccountInstruction(cpu, position_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 11;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, body_pc + 21u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    const unsigned position_add_cycles = 5u + (!fixed_record && registers->y >= 0x62u);
    Lufia2SpanFetch(registers, body_pc + 22u, 3u, true);
    const uint16_t displacement = Lufia2DecompReadData(registers, record + 4u, record + 5u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, displacement, false);
    Lufia2SpanAccountInstruction(cpu, position_add_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 13;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, body_pc + 25u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x01ffu, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, body_pc + 28u, 2u, true);
    Lufia2DecompWriteData(registers, 0x06u, 0x07u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 15;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, body_pc + 30u, 2u, true);
    registers->mf = true;
    registers->pc = (uint16_t)(body_pc + 32u);
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 16 - progress : fixed_record ? -14 : -15;
    return (fixed_record ? 31u : 35u) + size_cycles + size_add_cycles + position_cycles + position_add_cycles;
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

static int Lufia2DecompRunBattleTilemapCount(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                 uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t counter = ((uint32_t)registers->db << 16) | 0x15cau;
    if (pc != 0x858ba8u || opcode != 0x18u || registers->e || !registers->mf || registers->xf ||
        !Lufia2DecompIsWramByte(counter) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 15u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 8u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x6d\xca\x15\x8d\xca\x15\x7a", 7u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 1u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, registers->read(registers->mem, counter), true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 2 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 3u, true);
    registers->write(registers->mem, counter, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 7u, 1u, true);
    uint16_t index = registers->read(registers->mem, ++registers->sp);
    index |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, index, false);
    registers->pc = 0x8bb0u;
    registers->cyclesUsed = 5u;
    Lufia2SpanAccountInstruction(cpu, 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 4 - progress : -3;
    return 15;
}

static int Lufia2DecompRunBattleTilemapReturn(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool publish_count = pc == 0x858c22u;
    const bool copy_count = pc == 0x858c90u;
    const uint32_t bank = (uint32_t)registers->db << 16;
    const unsigned length = copy_count ? 8u : publish_count ? 5u : 2u;
    if ((!copy_count && !publish_count && pc != 0x858bbeu && pc != 0x858c96u && pc != 0x858dc3u && pc != 0x858c25u && pc != 0x8588f0u) ||
        opcode != (copy_count ? 0xadu : publish_count ? 0x8du : 0xabu) || registers->e ||
        ((copy_count || publish_count) && (!registers->mf || !Lufia2DecompIsWramByte(bank + 0x15d2u))) ||
        registers->sp < 0x1f00u || registers->sp > 0x1ffbu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        !Lufia2DecompFrameRangeQuiet(pc, pc + length) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (copy_count ? 18u : publish_count ? 14u : 10u))
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    if (!cart || (copy_count ? memcmp(cart->rom + offset + 1u, "\xd2\x15\x8d\xcf\x15\xab\x6b", 7u) :
        publish_count ? memcmp(cart->rom + offset + 1u, "\xce\x15\xab\x6b", 4u) :
        cart->rom[offset + 1u] != 0x6bu))
        return 0;

    long progress = 0;
    if (copy_count) {
        Lufia2SpanFetch(registers, pc, 3u, false);
        const uint8_t count = registers->read(registers->mem, bank + 0x15d2u);
        Lufia2DecompApplyOperand(registers, 0xa9u, count, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        Lufia2SpanFetch(registers, pc + 3u, 3u, true);
        registers->write(registers->mem, bank + 0x15cfu, count);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 2;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    if (publish_count) {
        Lufia2SpanFetch(registers, pc, 3u, false);
        registers->write(registers->mem, bank + 0x15ceu, (uint8_t)registers->a);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }
    const uint32_t pull_pc = pc + (copy_count ? 6u : publish_count ? 3u : 0u);
    Lufia2SpanFetch(registers, pull_pc, 1u, copy_count || publish_count);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = copy_count ? 3 : publish_count ? 2 : 1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pull_pc + 1u, 1u, true);
    uint16_t return_pc = registers->read(registers->mem, ++registers->sp);
    return_pc |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->k = registers->read(registers->mem, ++registers->sp);
    registers->pc = (uint16_t)(return_pc + 1u);
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    const long instructions = copy_count ? 4 : publish_count ? 3 : 2;
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = instructions;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? instructions - progress : 1 - instructions;
    return copy_count ? 18 : publish_count ? 14 : 10;
}

static int Lufia2DecompRunBattleTilemapSetup(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                 uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool roster = pc == 0x858c27u;
    const bool records = pc == 0x858b4bu;
    const unsigned length = records ? 18u : roster ? 20u : 13u;
    if ((!records && !roster && pc != 0x858bc0u) || opcode != 0x8bu || registers->e ||
        !registers->mf || registers->xf || registers->dp ||
        registers->sp < 0x1f02u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        !Lufia2DecompFrameRangeQuiet(pc, pc + length) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (records || roster ? 31u : 25u))
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    if (!cart || (records ? memcmp(cart->rom + offset + 1u,
        "\xa9\x00\x48\xab\xa9\xff\x8d\xc7\x15\xae\xc8\x15\x86\x08\x9c\xca\x15", 17u) : roster ? memcmp(cart->rom + offset + 1u,
        "\xa9\x00\x48\xab\xa2\x3d\x49\x86\x08\x9c\xd2\x15\xa9\x05\x85\x0d\xa0\x00\x00", 19u) :
        memcmp(cart->rom + offset + 1u, "\xa9\x00\x48\xab\xae\xcc\x15\x86\x08\x9c\xcb\x15", 12u)))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->write(registers->mem, registers->sp--, registers->db);
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 4u, 1u, true);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    if (records) {
        Lufia2SpanFetch(registers, pc + 5u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, 0xffu, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, pc + 7u, 3u, true);
        registers->write(registers->mem, 0x15c7u, 0xffu);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 6;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }
    const uint32_t fields_pc = pc + (records ? 10u : 5u);
    const long extra_steps = records ? 2 : 0;
    Lufia2SpanFetch(registers, fields_pc, 3u, true);
    const uint16_t destination = roster ? 0x493du : Lufia2DecompReadData(registers, records ? 0x15c8u : 0x15ccu, records ? 0x15c9u : 0x15cdu, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, destination, false);
    Lufia2SpanAccountInstruction(cpu, roster ? 3u : 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 5 + extra_steps;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, fields_pc + 3u, 2u, true);
    Lufia2DecompWriteData(registers, 0x08u, 0x09u, registers->x, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 6 + extra_steps;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, fields_pc + 5u, 3u, true);
    registers->write(registers->mem, records ? 0x15cau : roster ? 0x15d2u : 0x15cbu, 0u);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 7 + extra_steps;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    if (roster) {
        Lufia2SpanFetch(registers, pc + 13u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, 5u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, pc + 15u, 2u, true);
        registers->write(registers->mem, 0x0du, 5u);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 9;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        Lufia2SpanFetch(registers, pc + 17u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, 0u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);
    }
    registers->pc = (uint16_t)(pc + length);
    registers->cyclesUsed = roster ? 3u : 4u;
    const long instructions = records ? 9 : roster ? 10 : 7;
    *tail_steps = progress ? instructions - progress : 1 - instructions;
    return records || roster ? 31 : 25;
}

static int Lufia2DecompRunBattleTilemapFixedStyle(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                      uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x858bf7u || opcode != 0xadu || registers->e || !registers->mf || registers->dp ||
        !Lufia2DecompIsWramByte(bank + 0x15b6u) || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 38u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 27u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\xb6\x15\x18\x6d\xd0\x13\x29\x07\x0d\xce\x13\x38\x2a\x0d\x76\x14\x0d\xcf\x13\x85\x04\xad\xd1\x13\x85\x00", 26u))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, bank + 0x15b6u), true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 4u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, registers->read(registers->mem, bank + 0x13d0u), true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 7u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 9u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x09u, registers->read(registers->mem, bank + 0x13ceu), true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 12u, 1u, true);
    registers->c = true;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 13u, 1u, true);
    const uint8_t rotated = (uint8_t)((registers->a << 1) | 1u);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, rotated, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    static const uint16_t addresses[] = {0x1476u, 0x13cfu};
    for (unsigned index = 0; index < 2u; ++index) {
        Lufia2SpanFetch(registers, pc + 14u + index * 3u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0x09u, registers->read(registers->mem, bank + addresses[index]), true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 8u + index;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    Lufia2SpanFetch(registers, pc + 20u, 2u, true);
    registers->write(registers->mem, 0x04u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 10;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 22u, 3u, true);
    const uint8_t tile = registers->read(registers->mem, bank + 0x13d1u);
    Lufia2DecompApplyOperand(registers, 0xa9u, tile, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 11;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 25u, 2u, true);
    registers->write(registers->mem, 0x00u, tile);
    registers->pc = 0x8c12u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 12;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 12 - progress : -11;
    return 38;
}

static int Lufia2DecompRunBattleTilemapSelect(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                            uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool after_call = pc == 0x858c1cu;
    if ((!after_call && pc != 0x858c12u) || opcode != (after_call ? 0x80u : 0xa5u) ||
        registers->e || (!after_call && (!registers->mf || registers->dp)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (after_call ? 3u : 8u))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, after_call ? 2u : 6u, &offset);
    if (!cart || (after_call ? cart->rom[offset + 1u] != 4u :
        memcmp(cart->rom + offset + 1u, "\x04\x89\x40\xd0\x06", 5u)))
        return 0;

    if (after_call) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        registers->pc = 0x8c22u;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = -1;
        return 3;
    }

    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, 0x04u), true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x40u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    registers->pc = registers->z ? 0x8c18u : 0x8c1eu;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 2 : -2;
    return 5u + registers->cyclesUsed;
}

static int Lufia2DecompRunSpriteOverlayAssign(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                             uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool setup = pc == 0x81b685u;
    const uint32_t bank = (uint32_t)registers->db << 16;
    if ((!setup && pc != 0x81b68bu) || opcode != (setup ? 0xe2u : 0x9du) ||
        registers->e || !registers->mf || registers->dp ||
        (!setup && (!registers->xf || registers->x > 5u ||
                    !Lufia2DecompIsWramByte(bank + 0x15c0u))) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (setup ? 8u : 20u))
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, setup ? 6u : 13u, &offset);
    if (!cart || (setup ? memcmp(cart->rom + offset + 1u, "\x10\xa2\x00\xa5\x5f", 5u) :
        memcmp(cart->rom + offset + 1u, "\xbb\x15\x18\x7d\x7f\x15\xe8\xec\x4e\x15\xd0\xf3", 12u)))
        return 0;

    if (setup) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        registers->xf = true;
        registers->x &= 0xffu;
        registers->y &= 0xffu;
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanFetch(registers, pc + 2u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, 0u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, pc + 4u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, 0x5fu), true);
        registers->pc = 0xb68bu;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : -2;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        return 8;
    }

    Lufia2SpanFetch(registers, pc, 3u, false);
    registers->write(registers->mem, bank + 0x15bbu + registers->x, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 5u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 4u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u,
        registers->read(registers->mem, bank + 0x157fu + registers->x), true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 7u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint8_t)(registers->x + 1u), true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 8u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xe0u, registers->read(registers->mem, bank + 0x154eu), true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 11u, 2u, true);
    registers->pc = registers->z ? 0xb698u : 0xb68bu;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 6 - progress : -5;
    return 17u + registers->cyclesUsed;
}

static int Lufia2DecompRunSpriteOverlayVisit(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                            uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool setup = pc == 0x81b698u;
    const bool check = pc == 0x81b6a5u;
    const uint32_t bank = (uint32_t)registers->db << 16;
    if ((!setup && !check && pc != 0x81b6f6u) ||
        opcode != (setup ? 0xc2u : check ? 0xbdu : 0xe8u) ||
        registers->e || !registers->mf || registers->dp || (!setup && registers->xf) ||
        (setup && !Lufia2DecompIsWramByte(bank + 0x154eu)) ||
        (check && (registers->x > 5u || !Lufia2DecompIsWramByte(bank + 0x1539u))) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (setup ? 16u : check ? 7u : 15u))
        return 0;
    unsigned offset;
    const unsigned length = setup ? 13u : check ? 5u : 10u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = setup ? "\x10\xad\x4e\x15\xd0\x01\x60\x85\x55\xa2\x00\x00" :
        check ? "\x34\x15\xf0\x4c" : "\xe0\x06\x00\xc6\x55\xf0\x02\x80\xa5";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    if (setup) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        registers->xf = false;
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanFetch(registers, pc + 2u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, bank + 0x154eu), true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 2 : 0;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        Lufia2SpanFetch(registers, pc + 5u, 2u, true);
        if (registers->z) {
            registers->pc = 0xb69fu;
            registers->cyclesUsed = 2u;
            Lufia2SpanAccountInstruction(cpu, 2u);
            *tail_steps = progress ? 3 - progress : -2;
            return 9;
        }
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanFetch(registers, pc + 8u, 2u, true);
        registers->write(registers->mem, 0x55u, (uint8_t)registers->a);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 4;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        Lufia2SpanFetch(registers, pc + 10u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, 0u, false);
        registers->pc = 0xb6a5u;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 5 - progress : -4;
        return 16;
    }

    if (check) {
        Lufia2SpanFetch(registers, pc, 3u, false);
        Lufia2DecompApplyOperand(registers, 0xa9u,
            registers->read(registers->mem, bank + 0x1534u + registers->x), true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        Lufia2SpanFetch(registers, pc + 3u, 2u, true);
        registers->pc = registers->z ? 0xb6f6u : 0xb6aau;
        registers->cyclesUsed = registers->z ? 3u : 2u;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        *tail_steps = progress ? 1 : -1;
        return 4u + registers->cyclesUsed;
    }

    Lufia2SpanFetch(registers, pc, 1u, false);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 1u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xe0u, 6u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x55u, 0x56u, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    if (registers->z) {
        registers->pc = 0xb700u;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 1 : -3;
        return 13;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    registers->pc = 0xb6a5u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 2 : -4;
    return 15;
}

static int Lufia2DecompRunBattleSpriteSelection(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                              uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool skip = pc == 0x858a81u;
    const bool frame_load = pc == 0x858a78u;
    const bool frame_branch = pc == 0x858a7bu;
    const bool records_load = pc == 0x858b5du;
    const bool records_branch = pc == 0x858b60u;
    const bool records_start = pc == 0x858b62u;
    const bool records = records_load || records_branch || records_start;
    const bool load = frame_load || records_load;
    const unsigned length = skip || frame_branch ? 2u : frame_load || records_start ? 5u : records_load ? 10u : 7u;
    const unsigned budget = skip ? 3u : frame_load ? 7u : frame_branch ? 3u : records_load ? 12u : records_branch ? 8u : 6u;
    const uint32_t source = ((uint32_t)registers->db << 16) + (frame_load ? 0x11deu : 0x153cu);
    if ((!skip && !frame_load && !frame_branch && !records) ||
        opcode != (skip ? 0x80u : load ? 0xadu : frame_branch ? 0xd0u : records_branch ? 0xf0u : 0x85u) ||
        registers->e || !registers->mf || registers->xf || (records && registers->dp) ||
        (load && !Lufia2DecompIsWramByte(source)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        !Lufia2DecompFrameRangeQuiet(pc, pc + length) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < budget)
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = skip ? "\x12" : frame_load ? "\xde\x11\xd0\x06" : frame_branch ? "\x06" :
        records_load ? "\x3c\x15\xf0\x5c\x85\x0d\xa0\x00\x00" : records_branch ?
        "\x5c\x85\x0d\xa0\x00\x00" : "\x0d\xa0\x00\x00";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;
    long progress = 0;
    long instructions = 0;
    unsigned cycles = 0;
    uint32_t next = pc;
    if (skip) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        registers->pc = 0x8a95u;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = -1;
        return 3;
    }
    if (load) {
        Lufia2SpanFetch(registers, pc, 3u, false);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, source), true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        instructions = 1;
        cycles = 4u;
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = instructions;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        next += 3u;
    }
    if (!records_start) {
        Lufia2SpanFetch(registers, next, 2u, load);
        const bool taken = records ? registers->z : !registers->z;
        registers->cyclesUsed = taken ? 3u : 2u;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        cycles += registers->cyclesUsed;
        ++instructions;
        registers->pc = records ? (taken ? 0x8bbeu : 0x8b62u) : (taken ? 0x8a83u : 0x8a7du);
        if (!records || taken) {
            *tail_steps = progress ? instructions - progress : 1 - instructions;
            return (int)cycles;
        }
        next += 2u;
    }
    Lufia2SpanFetch(registers, next, 2u, !records_start);
    registers->write(registers->mem, 0x0du, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    ++instructions;
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = instructions;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, next + 2u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0u, false);
    registers->pc = 0x8b67u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    ++instructions;
    *tail_steps = progress ? instructions - progress : 1 - instructions;
    return (int)(cycles + 6u);
}

static int Lufia2DecompRunBattleSingleSpriteStart(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool load = pc == 0x858bcdu;
    const bool branch = pc == 0x858bd0u;
    const bool initialize = pc == 0x858bd2u;
    const bool store = pc == 0x858bd4u;
    const unsigned length = load ? 10u : branch ? 7u : initialize ? 5u : 3u;
    const unsigned budget = load ? 12u : branch ? 8u : initialize ? 6u : 4u;
    const uint32_t bank = (uint32_t)registers->db << 16;
    if ((!load && !branch && !initialize && !store) ||
        opcode != (load ? 0xadu : branch ? 0x10u : initialize ? 0xa9u : 0x8du) ||
        registers->e || !registers->mf || !Lufia2DecompIsWramByte(bank + 0x15cbu) ||
        (load && !Lufia2DecompIsWramByte(bank + 0x13ceu)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        !Lufia2DecompFrameRangeQuiet(pc, pc + length) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < budget)
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = load ? "\xce\x13\x10\x53\xa9\xff\x8d\xcb\x15" : branch ?
        "\x53\xa9\xff\x8d\xcb\x15" : initialize ? "\xff\x8d\xcb\x15" : "\xcb\x15";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;
    long progress = 0;
    long instructions = 0;
    unsigned cycles = 0;
    uint32_t next = pc;
    if (load) {
        Lufia2SpanFetch(registers, next, 3u, false);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, bank + 0x13ceu), true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        instructions = 1;
        cycles = 4u;
        next += 3u;
    }
    if (load || branch) {
        Lufia2SpanFetch(registers, next, 2u, load);
        const bool hidden = !registers->n;
        registers->cyclesUsed = hidden ? 3u : 2u;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        cycles += registers->cyclesUsed;
        ++instructions;
        if (hidden) {
            registers->pc = 0x8c25u;
            *tail_steps = progress ? instructions - progress : 1 - instructions;
            return (int)cycles;
        }
        next += 2u;
    }
    if (!store) {
        Lufia2SpanFetch(registers, next, 2u, !initialize);
        Lufia2DecompApplyOperand(registers, 0xa9u, 0xffu, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
        ++instructions;
        cycles += 2u;
        next += 2u;
    }
    Lufia2SpanFetch(registers, next, 3u, !store);
    registers->write(registers->mem, bank + 0x15cbu, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 4u);
    ++instructions;
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = instructions;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    registers->pc = 0x8bd7u;
    registers->cyclesUsed = 4u;
    *tail_steps = progress ? instructions - progress : 1 - instructions;
    return (int)(cycles + 4u);
}

static int Lufia2DecompRunBattlePartyGridSetup(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                             uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool begin = pc == 0x858d2eu;
    const bool load = pc == 0x858d4bu;
    const bool branch = pc == 0x858d4eu;
    const bool prepare = pc == 0x858d50u;
    const unsigned length = begin ? 20u : load ? 16u : branch ? 13u : 11u;
    const unsigned budget = begin ? 31u : load ? 23u : branch ? 19u : 17u;
    const uint32_t bank = (uint32_t)registers->db << 16;
    if ((!begin && !load && !branch && !prepare) ||
        opcode != (begin ? 0x8bu : load ? 0xadu : branch ? 0xf0u : 0xa9u) ||
        registers->e || !registers->mf || registers->xf || registers->dp ||
        registers->sp < 0x1f02u || registers->sp > 0x1fffu ||
        (load && !Lufia2DecompIsWramByte(bank + 0x154eu)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        !Lufia2DecompFrameRangeQuiet(pc, pc + length) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < budget)
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = begin ? "\x4b\xab\x9c\xd3\x15\xa2\x00\x28\xa0\x00\x02\x8e\x81\x21\x9c\x83\x21\xa9\x01" :
        load ? "\x4e\x15\xf0\x73\xa9\x06\x85\x05\xa9\x7e\x48\xab\xa0\x00\x00" :
        branch ? "\x73\xa9\x06\x85\x05\xa9\x7e\x48\xab\xa0\x00\x00" :
        "\x06\x85\x05\xa9\x7e\x48\xab\xa0\x00\x00";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;
    long progress = 0;
    if (begin) {
        for (unsigned push = 0; push < 2u; ++push) {
            Lufia2SpanFetch(registers, pc + push, 1u, push != 0u);
            registers->write(registers->mem, registers->sp--, push ? registers->k : registers->db);
            Lufia2SpanAccountInstruction(cpu, 3u);
            if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
                progress = push + 1u;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        }
        Lufia2SpanFetch(registers, pc + 2u, 1u, true);
        registers->db = registers->read(registers->mem, ++registers->sp);
        registers->n = (registers->db & 0x80u) != 0u;
        registers->z = registers->db == 0u;
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 3;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        Lufia2SpanFetch(registers, pc + 3u, 3u, true);
        registers->write(registers->mem, 0x8515d3u, 0u);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 4;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        Lufia2SpanFetch(registers, pc + 6u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, 0x2800u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanFetch(registers, pc + 9u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, 0x0200u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);
        Lufia2SpanFetch(registers, pc + 12u, 3u, true);
        Lufia2DecompWriteData(registers, 0x852181u, 0x852182u, registers->x, true);
        Lufia2SpanAccountInstruction(cpu, 5u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 7;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        Lufia2SpanFetch(registers, pc + 15u, 3u, true);
        registers->write(registers->mem, 0x852183u, 0u);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 8;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        Lufia2SpanFetch(registers, pc + 18u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, 1u, true);
        registers->pc = 0x8d42u;
        registers->cyclesUsed = 2u;
        Lufia2SpanAccountInstruction(cpu, 2u);
        *tail_steps = progress ? 9 - progress : -8;
        return 31;
    }
    uint32_t next = pc;
    long instructions = 0;
    unsigned cycles = 0;
    if (load) {
        Lufia2SpanFetch(registers, next, 3u, false);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, bank + 0x154eu), true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        instructions = 1;
        cycles = 4u;
        next += 3u;
    }
    if (!prepare) {
        Lufia2SpanFetch(registers, next, 2u, load);
        const bool empty = registers->z;
        registers->cyclesUsed = empty ? 3u : 2u;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        ++instructions;
        cycles += registers->cyclesUsed;
        if (empty) {
            registers->pc = 0x8dc3u;
            *tail_steps = progress ? instructions - progress : 1 - instructions;
            return (int)cycles;
        }
        next += 2u;
    }
    Lufia2SpanFetch(registers, next, 2u, !prepare);
    Lufia2DecompApplyOperand(registers, 0xa9u, 6u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    ++instructions;
    Lufia2SpanFetch(registers, next + 2u, 2u, true);
    registers->write(registers->mem, 0x05u, 6u);
    Lufia2SpanAccountInstruction(cpu, 3u);
    ++instructions;
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = instructions;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, next + 4u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x7eu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    ++instructions;
    Lufia2SpanFetch(registers, next + 6u, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    ++instructions;
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = instructions;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, next + 7u, 1u, true);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    ++instructions;
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = instructions;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, next + 8u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0u, false);
    registers->pc = 0x8d5bu;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    ++instructions;
    *tail_steps = progress ? instructions - progress : 1 - instructions;
    return (int)(cycles + 17u);
}
