#include "lufia2/actor_scene_presentation_steps.h"

_Static_assert(LUFIA2_ACTOR_SCENE_PRESENTATION_MAX_OPCODES <= 32u,
               "Scene presentation progress mask");

static void ActorBridgeScenePresentationStep(CpuState *cpu, uint32_t entry);

static bool ScenePresentationHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return hook == ActorBridgeScenePresentationStep &&
           (pc == 0x83a87cu || pc == 0x83a89au || pc == 0x83a8deu || pc == 0x83a90au ||
            pc == 0x83a96fu);
}

static unsigned PlanScenePresentationStep(const CpuState *cpu, uint32_t entry,
                                          FixedSpanOpcode *plan, uint32_t *next) {
    unsigned count = 0u, slot = cpu->ram[0xa7u];
#define SCENE_PRESENTATION_OPCODE(pc, fetched, cycles, data)                           \
    (plan[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    if (entry == 0x83a87cu) {
        SCENE_PRESENTATION_OPCODE(0xa87cu, 2u, 3u, 1u);
        SCENE_PRESENTATION_OPCODE(0xa87eu, 2u, 2u, 0u);
        SCENE_PRESENTATION_OPCODE(0xa880u, 2u, slot >= 5u ? 3u : 2u, 0u);
        if (slot < 5u) {
            SCENE_PRESENTATION_OPCODE(0xa882u, 1u, 2u, 0u);
            SCENE_PRESENTATION_OPCODE(0xa883u, 4u, 5u, 1u);
            SCENE_PRESENTATION_OPCODE(0xa887u, 1u, 2u, 0u);
            SCENE_PRESENTATION_OPCODE(0xa888u, 2u, 3u, 0u);
        } else {
            const bool random = !(cpu->ram[0x9a7u] & 1u);
            SCENE_PRESENTATION_OPCODE(0xa88au, 3u, 4u, 1u);
            SCENE_PRESENTATION_OPCODE(0xa88du, 2u, 2u, 0u);
            SCENE_PRESENTATION_OPCODE(0xa88fu, 2u, random ? 3u : 2u, 0u);
            if (random) {
                SCENE_PRESENTATION_OPCODE(0xa894u, 2u, 2u, 0u);
                *next = 0x83a896u;
                return count;
            }
            SCENE_PRESENTATION_OPCODE(0xa891u, 1u, 2u, 0u);
            SCENE_PRESENTATION_OPCODE(0xa892u, 2u, 3u, 0u);
        }
    }
    if (entry == 0x83a87cu || entry == 0x83a89au) {
        SCENE_PRESENTATION_OPCODE(0xa89au, 1u, 2u, 0u);
        SCENE_PRESENTATION_OPCODE(0xa89bu, 4u, 5u, 1u);
        *next = 0x83a89fu;
        return count;
    }
    if (entry == 0x83a8deu) {
        const unsigned record = cpu->ram[0x5fau + slot];
        SCENE_PRESENTATION_OPCODE(0xa8deu, 2u, 3u, 1u);
        SCENE_PRESENTATION_OPCODE(0xa8e0u, 1u, 2u, 0u);
        SCENE_PRESENTATION_OPCODE(0xa8e1u, 3u, slot >= 6u ? 5u : 4u, 1u);
        SCENE_PRESENTATION_OPCODE(0xa8e4u, 2u, 2u, 0u);
        SCENE_PRESENTATION_OPCODE(0xa8e6u, 2u, record >= 0x70u ? 3u : 2u, 0u);
        if (record >= 0x70u) {
            *next = 0x83a92cu;
            return count;
        }
        SCENE_PRESENTATION_OPCODE(0xa8e8u, 2u, 2u, 0u);
        SCENE_PRESENTATION_OPCODE(0xa8eau, 2u, record < 0x10u ? 3u : 2u, 0u);
        if (record < 0x10u) {
            *next = 0x83a96bu;
            return count;
        }
        SCENE_PRESENTATION_OPCODE(0xa8ecu, 2u, 2u, 0u);
        SCENE_PRESENTATION_OPCODE(0xa8eeu, 2u, record >= 0x50u ? 3u : 2u, 0u);
        if (record < 0x50u) {
            const unsigned index = record - 0x10u;
            const bool matches = cpu->ram[0x85eu + index] == cpu->ram[0x5acu];
            SCENE_PRESENTATION_OPCODE(0xa8f0u, 1u, 2u, 0u);
            SCENE_PRESENTATION_OPCODE(0xa8f1u, 2u, 2u, 0u);
            SCENE_PRESENTATION_OPCODE(0xa8f3u, 1u, 2u, 0u);
            SCENE_PRESENTATION_OPCODE(0xa8f4u, 3u, 4u, 1u);
            SCENE_PRESENTATION_OPCODE(0xa8f7u, 3u, 4u, 1u);
            SCENE_PRESENTATION_OPCODE(0xa8fau, 2u, matches ? 2u : 3u, 0u);
            if (!matches) {
                *next = 0x83a96bu;
                return count;
            }
            SCENE_PRESENTATION_OPCODE(0xa8fcu, 3u, 4u, 1u);
            SCENE_PRESENTATION_OPCODE(0xa8ffu, 2u, 3u, 0u);
        } else {
            SCENE_PRESENTATION_OPCODE(0xa901u, 1u, 2u, 0u);
            SCENE_PRESENTATION_OPCODE(0xa902u, 2u, 2u, 0u);
        }
        SCENE_PRESENTATION_OPCODE(0xa904u, 2u, 3u, 0u);
        *next = 0x83a906u;
        return count;
    }
    if (entry == 0x83a90au) {
        SCENE_PRESENTATION_OPCODE(0xa90au, 2u, 4u, 2u);
        for (unsigned field = 0u; field < 4u; ++field) {
            SCENE_PRESENTATION_OPCODE(0xa90cu + field * 8u, 4u, 5u, 1u);
            SCENE_PRESENTATION_OPCODE(0xa910u + field * 8u, 4u, 5u, 1u);
        }
        *next = 0x83a92cu;
        return count;
    }
    SCENE_PRESENTATION_OPCODE(0xa96fu, 2u, 3u, 0u);
    SCENE_PRESENTATION_OPCODE(0xa971u, 2u, 3u, 1u);
    SCENE_PRESENTATION_OPCODE(0xa973u, 1u, 2u, 0u);
    SCENE_PRESENTATION_OPCODE(0xa974u, 2u, slot ? 2u : 3u, 0u);
    if (slot)
        SCENE_PRESENTATION_OPCODE(0xa976u, 3u, 4u, 0u);
    *next = slot ? 0x83a83bu : 0x83a979u;
#undef SCENE_PRESENTATION_OPCODE
    return count;
}

static void ActorBridgeScenePresentationStep(CpuState *cpu, uint32_t entry) {
    const unsigned reserve = 512u;
    const bool copying = entry == 0x83a90au;
    const bool restoring = entry == 0x83a96fu;
    if (!ScenePresentationHookOwned(entry, ActorBridgeScenePresentationStep) ||
        cpu != &g_cpu || !cpu->ram || cpu->PB != 0x83u || cpu->m_flag != 1u ||
        cpu->x_flag > 1u || (!restoring && cpu->x_flag != !copying) || cpu->D ||
        cpu->_flag_D || (cpu->DB & 0x7fu) >= 0x40u ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1fffu, false))
        return;
    const unsigned slot = copying ? cpu->ram[0xa7u] | (unsigned)cpu->ram[0xa8u] << 8
                          : entry == 0x83a89au ? cpu->X
                                               : cpu->ram[0xa7u];
    if (slot >= 40u)
        return;
    uint32_t fence = entry == 0x83a89au   ? 0x83a89fu
                     : entry == 0x83a8deu ? 0x83a906u
                     : copying            ? 0x83a92cu
                                          : 0x83a979u;
    bool store_delay = false;
    if (entry == 0x83a87cu) {
        store_delay = slot < 5u || (cpu->ram[0x9a7u] & 1u);
        fence = slot < 5u ? 0x83a88au : store_delay ? 0x83a894u : 0x83a896u;
    }
    if (!Lufia2DecompFrameRangeAllows(entry, fence, ScenePresentationHookOwned) ||
        (store_delay && !Lufia2DecompFrameRangeAllows(0x83a89au, 0x83a89fu,
                                                      ScenePresentationHookOwned)))
        return;
    FixedSpanOpcode timing[LUFIA2_ACTOR_SCENE_PRESENTATION_MAX_OPCODES];
    uint32_t next;
    const unsigned count = PlanScenePresentationStep(cpu, entry, timing, &next);
    if (Lufia2DecompFrameOwnedSpanBudget(entry, fence,
                                         ActorBridgeScenePresentationStep) < count)
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || charged != cpu->master_cycles)
        return;
    Lufia2DecompRomOpcodeSpan fetch;
    const unsigned fetched = fence - entry < 32u ? fence - entry : 32u;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, fetched, &fetch);
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
        Lufia2ActorScenePresentationStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == next);
    assert(clock.index == count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
