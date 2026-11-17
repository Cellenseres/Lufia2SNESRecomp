#include "lufia2/battle_fill_steps.h"
#include "battle_output_range.h"

enum {
    BATTLE_FILL_MAX_OPCODES = LUFIA2_BATTLE_FILL_MAX_CELLS * LUFIA2_BATTLE_FILL_CELL_OPCODES
};
_Static_assert(BATTLE_FILL_MAX_OPCODES <= 32u, "Battle fill progress mask");

static void ActorBridgeBattleFillCells(CpuState *cpu, uint32_t entry);

static bool BattleFillCellsHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return pc == 0x81e7ecu && hook == ActorBridgeBattleFillCells;
}

static void ActorBridgeBattleFillCells(CpuState *cpu, uint32_t entry) {
    if (!BattleFillCellsHookOwned(entry, ActorBridgeBattleFillCells) ||
        cpu != &g_cpu || cpu->PB != 0x81u || cpu->DB != 0x7eu ||
        cpu->m_flag || cpu->x_flag || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !Lufia2DecompFrameRangeAllows(entry, 0x81e7f4u, BattleFillCellsHookOwned))
        return;
    unsigned cells = Lufia2DecompFrameSpanBudget(entry, 0x81e7f4u,
        ActorBridgeBattleFillCells) / LUFIA2_BATTLE_FILL_CELL_OPCODES;
    if (cells > LUFIA2_BATTLE_FILL_MAX_CELLS)
        cells = LUFIA2_BATTLE_FILL_MAX_CELLS;
    if (cpu->Y && cells > cpu->Y)
        cells = cpu->Y;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (!cells || deadline <= cpu->master_cycles)
        return;
    uint64_t room = deadline - cpu->master_cycles;
    if (room > 1363u)
        room = 1363u;
    const unsigned iteration = 8u * cpu_region_speed(entry) + 52u;
    const unsigned timed_cells = room > 40u ? (unsigned)((room - 40u) / iteration) : 0u;
    if (cells > timed_cells)
        cells = timed_cells;
    while (cells && !BattleWindowOutputSupported(cpu->X, cpu->X + 2u * cells))
        --cells;
    const unsigned reserve = cells * iteration + 40u;
    if (!cells || cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1ffcu, true))
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    FixedSpanOpcode timing[BATTLE_FILL_MAX_OPCODES];
    for (unsigned index = 0u; index < cells; ++index) {
        const unsigned offset = index * LUFIA2_BATTLE_FILL_CELL_OPCODES;
        timing[offset] = (FixedSpanOpcode){0xe7ecu, 3u, 7u, 2u};
        timing[offset + 1u] = (FixedSpanOpcode){0xe7efu, 1u, 2u, 0u};
        timing[offset + 2u] = (FixedSpanOpcode){0xe7f0u, 1u, 2u, 0u};
        timing[offset + 3u] = (FixedSpanOpcode){0xe7f1u, 1u, 2u, 0u};
        timing[offset + 4u] = (FixedSpanOpcode){0xe7f2u, 2u,
            cpu->Y == index + 1u ? 2u : 3u, 0u};
    }
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, 8u, &fetch);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x810000u, .timing = timing,
        .count = cells * LUFIA2_BATTLE_FILL_CELL_OPCODES,
        .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = Lufia2BattleFillCellsSpan(&memory, &state, cells);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY &&
        (result.pc == entry || result.pc == 0x81e7f4u));
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
