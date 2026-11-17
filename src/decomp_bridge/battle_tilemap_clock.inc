#include "lufia2/battle_tilemap_steps.h"

enum {
    BATTLE_TILEMAP_CELLS = 0x280u,
    BATTLE_TILEMAP_MAX_OPCODES = 4u * LUFIA2_BATTLE_TILEMAP_SPAN_MAX_CELLS
};
_Static_assert(BATTLE_TILEMAP_MAX_OPCODES <= 32u, "Battle tilemap progress mask");

static void ActorBridgeBattleTilemapSpan(CpuState *cpu, uint32_t entry);

static bool BattleTilemapHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return pc == 0x8188e9u && hook == ActorBridgeBattleTilemapSpan;
}

static void ActorBridgeBattleTilemapSpan(CpuState *cpu, uint32_t entry) {
    if (!BattleTilemapHookOwned(entry, ActorBridgeBattleTilemapSpan) || cpu != &g_cpu ||
        cpu->PB != 0x81u || cpu->DB != 0x81u || cpu->m_flag != 1u || cpu->x_flag ||
        !cpu->X || cpu->X > BATTLE_TILEMAP_CELLS || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !Lufia2DecompFrameRangeAllows(entry, entry + 9u, BattleTilemapHookOwned))
        return;
    const unsigned opcodes =
        Lufia2DecompFrameSpanBudget(entry, entry + 9u, ActorBridgeBattleTilemapSpan);
    unsigned cells = opcodes / 4u;
    if (cells > LUFIA2_BATTLE_TILEMAP_SPAN_MAX_CELLS)
        cells = LUFIA2_BATTLE_TILEMAP_SPAN_MAX_CELLS;
    if (cells > cpu->X)
        cells = cpu->X;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (!cells || deadline <= cpu->master_cycles)
        return;
    uint64_t room = deadline - cpu->master_cycles;
    if (room > 1363u)
        room = 1363u;
    const unsigned iteration = 9u * cpu_region_speed(entry) + 28u;
    const unsigned timed_cells = room > 40u ? (unsigned)((room - 40u) / iteration) : 0u;
    if (cells > timed_cells)
        cells = timed_cells;
    const unsigned reserve = cells * iteration + 40u;
    if (!cells || cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1fffu, false))
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    FixedSpanOpcode timing[BATTLE_TILEMAP_MAX_OPCODES];
    const uint16_t pc = (uint16_t)entry;
    for (unsigned index = 0u; index < cells; ++index) {
        timing[4u * index] = (FixedSpanOpcode){pc, 3u, 4u, 1u};
        timing[4u * index + 1u] = (FixedSpanOpcode){(uint16_t)(pc + 3u), 3u, 4u, 1u};
        timing[4u * index + 2u] = (FixedSpanOpcode){(uint16_t)(pc + 6u), 1u, 2u, 0u};
        timing[4u * index + 3u] =
            (FixedSpanOpcode){(uint16_t)(pc + 7u), 2u, cpu->X == index + 1u ? 2u : 3u, 0u};
    }
    FixedSpanClock clock = {
        .cpu = cpu, .bank = 0x810000u, .timing = timing, .count = 4u * cells
    };
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2BattleClearTilemapCells(&memory, &state, cells);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
