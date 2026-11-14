enum { FIXED_TILE_MAX_OPCODES = 256u };

_Static_assert(FIXED_TILE_MAX_OPCODES + 2u <= LUFIA2_BATTLE_FIXED_TILE_OPCODE_BUDGET,
    "review tile block opcode budget");

static const FixedSpanOpcode fixed_tile_timing[] = {
    {0xbe58u, 1u, 3u, 1u},
    {0xbe59u, 2u, 2u, 0u},
    {0xbe5bu, 1u, 3u, 1u},
    {0xbe5cu, 1u, 4u, 1u},
    {0xbe5du, 2u, 3u, 1u},
    {0xbe5fu, 1u, 2u, 0u},
    {0xbe60u, 2u, 3u, 1u},
    {0xbe62u, 2u, 2u, 0u},
    {0xbe64u, 2u, 3u, 1u},
    {0xbe66u, 2u, 3u, 1u},
    {0xbe68u, 2u, 4u, 2u},
    {0xbe6au, 2u, 4u, 2u},
    {0xbe6cu, 2u, 3u, 1u},
    {0xbe6eu, 2u, 3u, 1u},
    {0xbe70u, 2u, 3u, 1u},
    {0xbe72u, 2u, 3u, 1u},
    {0xbe74u, 2u, 3u, 1u},
    {0xbe76u, 1u, 3u, 0u},
    {0xbe77u, 2u, 3u, 1u},
    {0xbe79u, 2u, 3u, 0u},
    {0xbe7bu, 3u, 7u, 2u},
    {0xbe7eu, 1u, 2u, 0u},
    {0xbe7fu, 1u, 2u, 0u},
    {0xbe80u, 3u, 7u, 2u},
    {0xbe83u, 1u, 2u, 0u},
    {0xbe84u, 1u, 2u, 0u},
    {0xbe85u, 3u, 3u, 0u},
    {0xbe88u, 3u, 7u, 2u},
    {0xbe8bu, 1u, 2u, 0u},
    {0xbe8cu, 3u, 7u, 2u},
    {0xbe8fu, 1u, 2u, 0u},
    {0xbe90u, 3u, 3u, 0u},
    {0xbe93u, 3u, 3u, 0u},
    {0xbe96u, 2u, 2u, 0u},
    {0xbe98u, 3u, 3u, 0u},
    {0xbe9bu, 2u, 3u, 0u},
    {0xbe9du, 1u, 2u, 0u},
    {0xbe9eu, 1u, 2u, 0u},
    {0xbe9fu, 1u, 2u, 0u},
    {0xbea0u, 1u, 2u, 0u},
    {0xbea1u, 2u, 5u, 2u},
    {0xbea3u, 2u, 2u, 0u},
    {0xbea5u, 2u, 3u, 1u},
    {0xbea7u, 2u, 5u, 2u},
    {0xbea9u, 2u, 2u, 0u},
    {0xbeabu, 2u, 3u, 0u},
    {0xbeadu, 2u, 4u, 2u},
    {0xbeafu, 1u, 2u, 0u},
    {0xbeb0u, 3u, 3u, 0u},
    {0xbeb3u, 2u, 4u, 2u},
    {0xbeb5u, 1u, 2u, 0u},
    {0xbeb6u, 2u, 3u, 0u},
    {0xbeb8u, 2u, 3u, 0u},
    {0xbebau, 1u, 4u, 1u},
};

enum FixedTileOpcodeIndex {
    TILE_BE58, TILE_BE59, TILE_BE5B, TILE_BE5C,
    TILE_BE5D, TILE_BE5F, TILE_BE60, TILE_BE62,
    TILE_BE64, TILE_BE66, TILE_BE68, TILE_BE6A,
    TILE_BE6C, TILE_BE6E, TILE_BE70, TILE_BE72,
    TILE_BE74, TILE_BE76, TILE_BE77, TILE_BE79,
    TILE_BE7B, TILE_BE7E, TILE_BE7F, TILE_BE80,
    TILE_BE83, TILE_BE84, TILE_BE85, TILE_BE88,
    TILE_BE8B, TILE_BE8C, TILE_BE8F, TILE_BE90,
    TILE_BE93, TILE_BE96, TILE_BE98, TILE_BE9B,
    TILE_BE9D, TILE_BE9E, TILE_BE9F, TILE_BEA0,
    TILE_BEA1, TILE_BEA3, TILE_BEA5, TILE_BEA7,
    TILE_BEA9, TILE_BEAB, TILE_BEAD, TILE_BEAF,
    TILE_BEB0, TILE_BEB3, TILE_BEB5, TILE_BEB6,
    TILE_BEB8, TILE_BEBA
};

typedef struct FixedTilePlan {
    FixedSpanOpcode opcodes[FIXED_TILE_MAX_OPCODES];
    unsigned count;
    BattleLeafCost cost;
} FixedTilePlan;

static void AppendFixedTileRun(FixedTilePlan *plan, unsigned first, unsigned last) {
    assert(first <= last && plan->count + last - first + 1u <= FIXED_TILE_MAX_OPCODES);
    for (unsigned index = first; index <= last; ++index) {
        const FixedSpanOpcode opcode = fixed_tile_timing[index];
        plan->opcodes[plan->count++] = opcode;
        plan->cost.cycles += opcode.cycles;
        plan->cost.rom_bytes += opcode.fetched;
        plan->cost.ram_bytes += opcode.data_bytes;
    }
}

static void AppendFixedTileBranch(FixedTilePlan *plan, unsigned index, bool taken) {
    AppendFixedTileRun(plan, index, index);
    if (taken) {
        ++plan->opcodes[plan->count - 1u].cycles;
        ++plan->cost.cycles;
    }
}

static bool PlanFixedTileBlock(const uint8_t *ram, FixedTilePlan *plan) {
    const unsigned columns = ram[2], rows = ram[3];
    if (!columns || columns > 4u || !rows || rows > 2u)
        return false;
    plan->count = 0u;
    plan->cost = (BattleLeafCost){0};
    const unsigned doubled = (ram[0] << 1u) & 0xffu;
    unsigned tile = (doubled + (doubled & 0xf0u) + (ram[0] >> 7u)) & 0xffu;
    AppendFixedTileRun(plan, TILE_BE58, TILE_BE6E);
    for (unsigned row = 0u; row < rows; ++row) {
        AppendFixedTileRun(plan, TILE_BE70, TILE_BE77);
        unsigned word = ((unsigned)ram[4] << 8u) | tile;
        for (unsigned column = 0u; column < columns; ++column) {
            AppendFixedTileRun(plan, TILE_BE79, TILE_BE93);
            const unsigned upper_carry = word + 0x10u > 0xffffu;
            const unsigned advanced = word + 2u + upper_carry;
            const unsigned carry = advanced > 0xffffu;
            word = advanced & 0xffffu;
            const bool wraps = !(word & 15u);
            AppendFixedTileBranch(plan, TILE_BE96, !wraps);
            if (wraps) {
                AppendFixedTileRun(plan, TILE_BE98, TILE_BE98);
                word = (word + 0x10u + carry) & 0xffffu;
            }
            AppendFixedTileRun(plan, TILE_BE9B, TILE_BEA1);
            AppendFixedTileBranch(plan, TILE_BEA3, column + 1u != columns);
        }
        tile = word & 0xffu;
        AppendFixedTileRun(plan, TILE_BEA5, TILE_BEA7);
        AppendFixedTileBranch(plan, TILE_BEA9, row + 1u == rows);
        if (row + 1u != rows)
            AppendFixedTileRun(plan, TILE_BEAB, TILE_BEB8);
    }
    AppendFixedTileRun(plan, TILE_BEBA, TILE_BEBA);
    return true;
}

static bool FixedTileCallerSupported(uint32_t return_pc) {
    return return_pc == 0x81cbb2u || return_pc == 0x81cbe0u ||
        return_pc == 0x81ccafu || return_pc == 0x81ccdbu;
}

static RecompReturn ActorBridgeTimedFixedTileBlock(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid,
        Lufia2DecompFramePairedReturnPc(cpu, 0x81be58u)};
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return ActorBridgeFallback(cpu, &frame, 0x81be58u);
#endif
    if (rtl_apu_extended_frame_timing() || cpu != &g_cpu || !cpu->ram ||
        cpu->emulation || cpu->m_flag != 1u || cpu->x_flag || cpu->_flag_D || cpu->D ||
        cpu->PB != 0x81u || cpu->S < 0x1f20u || cpu->S > 0x1ffdu ||
        cpu->host_return_valid != 2u ||
        !Lufia2DecompFramePairedCallReady(cpu, 0x81be58u) ||
        !FixedTileCallerSupported(frame.host_return_pc24) ||
        (cpu->ram[cpu->S + 1u] | (unsigned)cpu->ram[cpu->S + 2u] << 8u) !=
            (uint16_t)(frame.host_return_pc24 - 1u) ||
        !g_snes || !g_snes->apu || !g_snes->dma || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize <= 0xbebbu || g_snes->cart->superfx ||
        g_snes->cart->cx4 || g_snes->cart->dsp1 || g_snes->cart->sdd1 || g_snes->cart->sa1 ||
        (g_snes->vPos < 225u && !g_snes->hdmaBeamOff) || g_snes->dma->hdmaPendingInit)
        return ActorBridgeFallback(cpu, &frame, 0x81be58u);
    const unsigned destination = cpu->ram[8] | (unsigned)cpu->ram[9] << 8u;
    FixedTilePlan plan;
    if (destination < 0x2000u ||
        destination + (unsigned)cpu->ram[3] * 128u + (unsigned)cpu->ram[2] * 4u + 0x43u >= 0x10000u ||
        !PlanFixedTileBlock(cpu->ram, &plan) ||
        plan.count + 1u > LUFIA2_BATTLE_FIXED_TILE_OPCODE_BUDGET)
        return ActorBridgeFallback(cpu, &frame, 0x81be58u);
    BattleLeafCost cost = plan.cost;
    cost.cycles += 6u;
    ++cost.rom_bytes;
    cost.ram_bytes += 2u;
    if (!BattleClockAllows(cpu, 0x81be58u, 0x81bebcu, 2u, false,
            BattleLeafClockRoom(0x81be58u, cost), false, NULL))
        return ActorBridgeFallback(cpu, &frame, 0x81be58u);
    PairedSpanClock clock = BeginPairedSpanClock(cpu, 0x81be58u, plan.opcodes, plan.count);
    const Lufia2Memory memory = {PairedSpanRead, PairedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleTileBlock(&memory, &state);
    PreparePairedSpan(&clock);
    assert(clock.offset == plan.count && !clock.span.remaining &&
        result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x81bebbu);
    ActorBridgeStore(cpu, &state);
    bool progress = false;
    const RecompReturn returned = ReturnTimedMenuCallWithProgress(cpu, &frame, result.pc, &progress);
    Lufia2DecompFramePairedAccountSpan(cpu, 0x81be58u, 1u, progress);
    return returned;
}
