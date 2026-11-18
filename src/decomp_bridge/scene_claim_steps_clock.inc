#include "lufia2/actor_scene_claim_steps.h"

_Static_assert(LUFIA2_ACTOR_SCENE_CLAIM_MAX_OPCODES <= 32u,
               "Scene claim progress mask");

static void ActorBridgeSceneClaimStep(CpuState *cpu, uint32_t entry);

static bool SceneClaimStepHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return (pc == 0x83a6c7u || pc == 0x83a6d5u) && hook == ActorBridgeSceneClaimStep;
}

static unsigned PlanSceneClaimStep(uint32_t entry, unsigned slot, FixedSpanOpcode *plan,
                                   uint32_t *next) {
    unsigned count = 0u;
    if (entry == 0x83a6c7u) {
        static const FixedSpanOpcode setup[] = {{0xa6c7u, 2u, 2u, 0u},
                                                {0xa6c9u, 4u, 5u, 1u},
                                                {0xa6cdu, 4u, 5u, 1u},
                                                {0xa6d1u, 2u, 2u, 0u},
                                                {0xa6d3u, 2u, 2u, 0u}};
        memcpy(plan, setup, sizeof setup);
        count = sizeof setup / sizeof setup[0];
        slot = 7u;
    }
    for (unsigned i = 0u; i < 2u; ++i) {
        plan[count++] = (FixedSpanOpcode){0xa6d5u, 4u, 5u, 1u};
        plan[count++] = (FixedSpanOpcode){0xa6d9u, 1u, 2u, 0u};
        plan[count++] = (FixedSpanOpcode){0xa6dau, 2u, slot ? 3u : 2u, 0u};
        if (!slot--) {
            *next = 0x83a6dcu;
            return count;
        }
    }
    *next = 0x83a6d5u;
    return count;
}

static void ActorBridgeSceneClaimStep(CpuState *cpu, uint32_t entry) {
    const unsigned reserve = 384u;
    if (!SceneClaimStepHookOwned(entry, ActorBridgeSceneClaimStep) || cpu != &g_cpu ||
        cpu->PB != 0x83u || !cpu->ram || cpu->D || cpu->_flag_D || cpu->m_flag != 1u ||
        cpu->x_flag != 1u || (entry == 0x83a6d5u && cpu->X > 7u) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1fffu, false) ||
        !Lufia2DecompFrameRangeAllows(entry, 0x83a6dcu, SceneClaimStepHookOwned))
        return;
    FixedSpanOpcode timing[LUFIA2_ACTOR_SCENE_CLAIM_MAX_OPCODES];
    uint32_t next;
    const unsigned count = PlanSceneClaimStep(entry, cpu->X, timing, &next);
    if (Lufia2DecompFrameOwnedSpanBudget(entry, 0x83a6dcu, ActorBridgeSceneClaimStep) <
        count)
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || charged != cpu->master_cycles)
        return;
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared =
        Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, 0x83a6dcu - entry, &fetch);
    FixedSpanClock clock = {.cpu = cpu,
                            .bank = 0x830000u,
                            .timing = timing,
                            .count = count,
                            .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result =
        Lufia2ActorClearSceneClaimStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == next);
    assert(clock.index == count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
