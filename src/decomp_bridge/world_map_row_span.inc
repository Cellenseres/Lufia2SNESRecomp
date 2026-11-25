enum {
    WORLD_ROW_CELL_INSTRUCTIONS = 49u,
    WORLD_ROW_CELL_CYCLES = 185u,
    WORLD_ROW_BOTTOM_LEFT = 0x80u,
    WORLD_ROW_BOTTOM_RIGHT = 0x81u
};

typedef struct WorldMapRow {
    NativeSpanState span;
    Interp816 *registers;
    CpuState *cpu;
    const Cart *cart;
    uint64_t dynamic_epoch;
    uint64_t write_epoch;
    unsigned fetch_speed;
} WorldMapRow;

/* The cell never writes MEMSEL, so one region speed covers every fetch. */
static void WorldRowFetch(WorldMapRow *row, unsigned length) {
    const uint16_t address = row->registers->pc;
    if (row->span.instructions)
        (void)cpu_read8(row->cpu, 0x86u, address);
    (void)cpu_read8(row->cpu, 0x86u, address);
    g_interp816_cur_pc = 0x860000u | address;
    for (unsigned byte = 1u; byte < length; ++byte)
        (void)cpu_read8(row->cpu, 0x86u, (uint16_t)(address + byte));
    s_interp_bus_cycles += length;
    s_interp_bus_master += length * row->fetch_speed;
}

static void WorldRowFinish(WorldMapRow *row, int length, unsigned cycles) {
    NativeSpanStep(&row->span, row->registers, row->cpu, cycles, (uint16_t)(row->registers->pc + length));
}

static void WorldRowFinishAccess(WorldMapRow *row, unsigned length, unsigned cycles) {
    WorldRowFinish(row, length, cycles);
    NativeSpanSyncProgress(&row->span, row->cpu, &row->dynamic_epoch, &row->write_epoch);
}

static void WorldRowImmediate(WorldMapRow *row, uint8_t operation, uint16_t value) {
    WorldRowFetch(row, 3u);
    Lufia2DecompApplyOperand(row->registers, operation, value, false);
    WorldRowFinish(row, 3, 3u);
}

static void WorldRowTransfer(WorldMapRow *row, uint8_t operation, uint16_t value, unsigned cycles) {
    WorldRowFetch(row, 1u);
    Lufia2DecompApplyOperand(row->registers, operation, value, false);
    WorldRowFinish(row, 1, cycles);
}

static void WorldRowShiftLeft(WorldMapRow *row) {
    const uint16_t value = row->registers->a;
    row->registers->c = (value & 0x8000u) != 0u;
    WorldRowTransfer(row, 0xa9u, (uint16_t)(value << 1), 2u);
}

static void WorldRowSetByteAccumulator(WorldMapRow *row, bool byte) {
    WorldRowFetch(row, 2u);
    row->registers->mf = byte;
    WorldRowFinish(row, 2, 3u);
}

static bool WorldRowBranch(WorldMapRow *row, bool taken, int distance) {
    WorldRowFetch(row, 2u);
    WorldRowFinish(row, taken ? 2 + distance : 2, taken ? 3u : 2u);
    return taken;
}

static void WorldRowLoad(WorldMapRow *row, uint8_t operation, uint8_t address) {
    WorldRowFetch(row, 2u);
    const uint16_t value = Lufia2DecompReadData(row->registers, address, address + 1u, true);
    Lufia2DecompApplyOperand(row->registers, operation, value, false);
    WorldRowFinishAccess(row, 2u, 4u);
}

static void WorldRowStore(WorldMapRow *row, uint8_t address) {
    WorldRowFetch(row, 2u);
    bridge_bus_write(row->cpu, address, (uint8_t)row->registers->a);
    bridge_bus_write(row->cpu, address + 1u, (uint8_t)(row->registers->a >> 8));
    WorldRowFinishAccess(row, 2u, 4u);
}

static void WorldRowStep(WorldMapRow *row, uint8_t opcode, uint8_t address) {
    const bool wide = !row->registers->mf;
    WorldRowFetch(row, 2u);
    Lufia2DecompModifyWram(row->registers, opcode, address, address + 1u, wide);
    WorldRowFinishAccess(row, 2u, wide ? 7u : 5u);
}

static bool WorldRowLoadTable(WorldMapRow *row, uint8_t pointer) {
    Interp816 *registers = row->registers;
    const bool wide = !registers->mf;
    const uint8_t *ram = row->cpu->ram + pointer;
    const uint32_t expected = ((ram[0] | ((uint32_t)ram[1] << 8) | ((uint32_t)ram[2] << 16)) +
        registers->y) & 0xffffffu;
    if (!Lufia2DecompIsDecompressSourceByte(row->cart, expected) ||
        !Lufia2DecompIsDecompressSourceByte(row->cart, (expected + 1u) & 0xffffffu))
        return false;

    WorldRowFetch(row, 2u);
    uint32_t source = Lufia2DecompReadData(registers, pointer, pointer + 1u, true);
    source |= (uint32_t)bridge_bus_read(row->cpu, pointer + 2u) << 16;
    source = (source + registers->y) & 0xffffffu;
    const uint16_t value = Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, wide);
    Lufia2DecompApplyOperand(registers, 0xa9u, value, !wide);
    WorldRowFinishAccess(row, 2u, wide ? 7u : 6u);
    return true;
}

static void WorldRowStoreTile(WorldMapRow *row, unsigned offset) {
    WorldRowFetch(row, 3u);
    bridge_bus_write(row->cpu, 0x7f0000u + row->registers->x + offset, (uint8_t)row->registers->a);
    WorldRowFinishAccess(row, 3u, 6u);
}

static bool WorldRowSelectMetatile(WorldMapRow *row) {
    Interp816 *registers = row->registers;
    WorldRowLoad(row, 0xa9u, 0x58u);
    WorldRowImmediate(row, 0x29u, 1u);
    WorldRowShiftLeft(row);
    WorldRowLoad(row, 0x69u, 0x13u);
    WorldRowStore(row, 0x00u);

    WorldRowFetch(row, 2u);
    const uint32_t source = 0x7f0000u | Lufia2DecompReadData(registers, 0x08u, 0x09u, true);
    const uint16_t metatile = Lufia2DecompReadData(registers, source, source + 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, metatile, false);
    WorldRowFinishAccess(row, 2u, 6u);

    for (unsigned shift = 0u; shift < 3u; ++shift)
        WorldRowShiftLeft(row);
    WorldRowLoad(row, 0x69u, 0x00u);
    WorldRowTransfer(row, 0xa0u, registers->a, 2u);
    if (WorldRowBranch(row, !registers->n, 4)) {
        if (!WorldRowLoadTable(row, 0xdfu))
            return false;
    } else {
        if (!WorldRowLoadTable(row, 0xe3u))
            return false;
        (void)WorldRowBranch(row, true, 2);
    }
    WorldRowStore(row, 0x00u);
    WorldRowShiftLeft(row);
    WorldRowShiftLeft(row);
    WorldRowLoad(row, 0x69u, 0x00u);
    WorldRowTransfer(row, 0xa0u, registers->a, 2u);
    WorldRowLoad(row, 0xa2u, 0x0bu);
    return true;
}

static void WorldRowExchangeBytes(WorldMapRow *row) {
    WorldRowFetch(row, 1u);
    MenuGlyphExchangeBytes(row->registers);
    WorldRowFinish(row, 1, 3u);
}

static bool WorldRowWriteTiles(WorldMapRow *row) {
    if (!WorldRowLoadTable(row, 0xe7u))
        return false;
    WorldRowSetByteAccumulator(row, true);
    WorldRowStoreTile(row, 0u);
    WorldRowExchangeBytes(row);
    WorldRowStoreTile(row, WORLD_ROW_BOTTOM_LEFT);
    if (!WorldRowLoadTable(row, 0xeau))
        return false;
    WorldRowStoreTile(row, 1u);
    if (!WorldRowLoadTable(row, 0xedu))
        return false;
    WorldRowStoreTile(row, WORLD_ROW_BOTTOM_RIGHT);
    WorldRowSetByteAccumulator(row, false);
    return true;
}

static void WorldRowAdvanceCell(WorldMapRow *row) {
    Interp816 *registers = row->registers;
    WorldRowLoad(row, 0xa9u, 0x0bu);
    WorldRowTransfer(row, 0xa9u, (uint16_t)(registers->a + 1u), 2u);
    WorldRowTransfer(row, 0xa9u, (uint16_t)(registers->a + 1u), 2u);
    WorldRowImmediate(row, 0x89u, 0x7fu);
    if (!WorldRowBranch(row, !registers->z, 4)) {
        WorldRowFetch(row, 1u);
        registers->c = true;
        WorldRowFinish(row, 1, 2u);
        WorldRowImmediate(row, 0xe9u, 0x80u);
    }
    WorldRowStore(row, 0x0bu);
}

/* The map edge leaves the span at the call that rebuilds the block pointer. */
static bool WorldRowAdvanceBlock(WorldMapRow *row) {
    Interp816 *registers = row->registers;
    WorldRowSetByteAccumulator(row, true);
    WorldRowStep(row, 0xe6u, 0x58u);
    WorldRowSetByteAccumulator(row, false);
    if (!WorldRowBranch(row, !registers->z, 6))
        return false;
    WorldRowLoad(row, 0xa9u, 0x58u);
    const uint16_t column = registers->a;
    registers->c = (column & 1u) != 0u;
    WorldRowTransfer(row, 0xa9u, column >> 1, 2u);
    if (!WorldRowBranch(row, registers->c, 4)) {
        WorldRowStep(row, 0xe6u, 0x08u);
        WorldRowStep(row, 0xe6u, 0x08u);
    }
    WorldRowStep(row, 0xc6u, 0x26u);
    return true;
}

static int Lufia2DecompRunWorldMapRowSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                          uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget,
                                          long *step_count) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) ||                                             \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    if (pc != 0x86ac93u || opcode != 0xa5u || !cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram ||
        registers->read != bridge_bus_read || registers->write != bridge_bus_write || registers->e ||
        registers->mf || registers->xf || registers->d || registers->dp || registers->db != 0x7fu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        !Lufia2DecompFrameRangeQuiet(pc + 1u, pc + 0x64u))
        return 0;

    static const uint8_t code[] = {
        0xa5, 0x58, 0x29, 0x01, 0x00, 0x0a, 0x65, 0x13, 0x85, 0x00, 0xb2, 0x08, 0x0a, 0x0a, 0x0a, 0x65, 0x00,
        0xa8, 0x10, 0x04, 0xb7, 0xe3, 0x80, 0x02, 0xb7, 0xdf, 0x85, 0x00, 0x0a, 0x0a, 0x65, 0x00, 0xa8, 0xa6,
        0x0b, 0xb7, 0xe7, 0xe2, 0x20, 0x9d, 0x00, 0x00, 0xeb, 0x9d, 0x80, 0x00, 0xb7, 0xea, 0x9d, 0x01, 0x00,
        0xb7, 0xed, 0x9d, 0x81, 0x00, 0xc2, 0x20, 0xa5, 0x0b, 0x1a, 0x1a, 0x89, 0x7f, 0x00, 0xd0, 0x04, 0x38,
        0xe9, 0x80, 0x00, 0x85, 0x0b, 0xe2, 0x20, 0xe6, 0x58, 0xc2, 0x20, 0xd0, 0x06, 0x20, 0x05, 0xae, 0x82,
        0x09, 0x00, 0xa5, 0x58, 0x4a, 0xb0, 0x04, 0xe6, 0x08, 0xe6, 0x08, 0xc6, 0x26, 0xd0, 0x9c
    };
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, sizeof code, &offset);
    if (!cart || memcmp(cart->rom + offset, code, sizeof code))
        return 0;

    const bool repeats = Lufia2DecompFrameRangeQuiet(pc, pc + 1u);
    WorldMapRow row = {{BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0u, 0u, 0u, 0u},
                       registers, cpu, cart, dynamic_epoch, write_epoch, bridge_region_speed(pc)};
    do {
        if (row.span.instructions + WORLD_ROW_CELL_INSTRUCTIONS > row.span.steps ||
            row.span.cycles + WORLD_ROW_CELL_CYCLES > row.span.budget ||
            (s_lle_master_deadline && cpu->master_cycles >= s_lle_master_deadline) ||
            !WorldRowSelectMetatile(&row) || !WorldRowWriteTiles(&row))
            break;
        WorldRowAdvanceCell(&row);
        if (!WorldRowAdvanceBlock(&row))
            break;
        if (!WorldRowBranch(&row, !registers->z, -0x64))
            break;
    } while (repeats);

    if (!row.span.instructions)
        return 0;
    *step_count = row.span.progress ? row.span.instructions - row.span.progress
                                    : *step_count + row.span.instructions - 1;
    return (int)row.span.cycles;
}
