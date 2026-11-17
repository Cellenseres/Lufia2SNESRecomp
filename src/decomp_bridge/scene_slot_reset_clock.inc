#include "lufia2/actor_scene_reset_steps.h"

enum { SCENE_RESET_SLOT_COUNT = 40u };

static void ActorBridgeSceneSlotReset(CpuState *cpu, uint32_t entry);

static bool SceneSlotResetHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return (pc == 0x83a692u || pc == 0x83a6a6u) && hook == ActorBridgeSceneSlotReset;
}

static void ActorBridgeSceneSlotReset(CpuState *cpu, uint32_t entry) {
    if (!SceneSlotResetHookOwned(entry, ActorBridgeSceneSlotReset) || cpu != &g_cpu ||
        cpu->PB != 0x83u || cpu->m_flag != 1u || cpu->x_flag != 1u || cpu->D ||
        cpu->_flag_D || cpu->X >= SCENE_RESET_SLOT_COUNT ||
        !((cpu->DB & 0x7fu) < 0x40u || cpu->DB == 0x7eu || cpu->DB == 0x7fu) ||
        !Lufia2DecompOpcodeWindowReady() || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return;
    const bool first = entry == 0x83a692u;
    unsigned phases = first ? 2u : 1u;
    unsigned count = 0u, reserve = 0u;
    uint32_t end = 0u;
    for (; phases; --phases) {
        const bool both = first && phases == 2u;
        count = both ? LUFIA2_SCENE_RESET_STATUS_OPCODES + LUFIA2_SCENE_RESET_RUNTIME_OPCODES
            : first ? LUFIA2_SCENE_RESET_STATUS_OPCODES : LUFIA2_SCENE_RESET_RUNTIME_OPCODES;
        reserve = both ? 576u : first ? 296u : 328u;
        end = first && !both ? 0x83a6a6u : 0x83a6c1u;
        if (cpu->master_cycles > UINT64_MAX - reserve ||
            !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1fffu, false) ||
            !Lufia2DecompFrameRangeAllows(entry, end, SceneSlotResetHookOwned))
            continue;
        const unsigned budget = both
            ? Lufia2DecompFrameOwnedSpanBudget(entry, end, ActorBridgeSceneSlotReset)
            : Lufia2DecompFrameSpanBudget(entry, end, ActorBridgeSceneSlotReset);
        if (budget >= count)
            break;
    }
    if (!phases)
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    static const FixedSpanOpcode plan[] = {
        {0xa692u, 2u, 2u, 0u}, {0xa694u, 3u, 5u, 1u},
        {0xa697u, 3u, 5u, 1u}, {0xa69au, 3u, 5u, 1u},
        {0xa69du, 3u, 5u, 1u}, {0xa6a0u, 3u, 5u, 1u},
        {0xa6a3u, 3u, 5u, 1u}, {0xa6a6u, 2u, 2u, 0u},
        {0xa6a8u, 4u, 5u, 1u}, {0xa6acu, 1u, 2u, 0u},
        {0xa6adu, 4u, 5u, 1u}, {0xa6b1u, 4u, 5u, 1u},
        {0xa6b5u, 4u, 5u, 1u}, {0xa6b9u, 3u, 5u, 1u},
        {0xa6bcu, 2u, 2u, 0u}, {0xa6beu, 3u, 5u, 1u}
    };
    FixedSpanOpcode timing[LUFIA2_SCENE_RESET_STATUS_OPCODES + LUFIA2_SCENE_RESET_RUNTIME_OPCODES];
    const unsigned offset = first ? 0u : LUFIA2_SCENE_RESET_STATUS_OPCODES;
    for (unsigned i = 0u; i < count; ++i)
        timing[i] = plan[offset + i];
    if (first && cpu->X >= 6u)
        timing[3].cycles = 6u;
    Lufia2DecompRomOpcodeSpan status_fetch, runtime_fetch;
    const bool status_prepared = first &&
        Lufia2DecompPrepareRomOpcodeSpan(cpu, 0x83a692u, 20u, &status_fetch);
    const bool runtime_prepared = (!first || phases == 2u) &&
        Lufia2DecompPrepareRomOpcodeSpan(cpu, 0x83a6a6u, 27u, &runtime_fetch);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x830000u, .timing = timing,
        .count = count, .opcode_span = first ? (status_prepared ? &status_fetch : NULL)
            : (runtime_prepared ? &runtime_fetch : NULL)};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    Lufia2ExecutionResult result = Lufia2ActorResetSceneSlotStep(&memory, &state, entry);
    if (first && phases == 2u) {
        assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == 0x83a6a6u);
        assert(clock.index == LUFIA2_SCENE_RESET_STATUS_OPCODES && !clock.remaining);
        clock.opcode_span = runtime_prepared ? &runtime_fetch : NULL;
        result = Lufia2ActorResetSceneSlotStep(&memory, &state, result.pc);
    }
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == end);
    assert(clock.index == clock.count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
