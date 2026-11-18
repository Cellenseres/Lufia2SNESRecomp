#include "lufia2/actor_scene_value_steps.h"

_Static_assert(LUFIA2_ACTOR_SCENE_VALUE_MAX_OPCODES <= 32u,
               "Scene value progress mask");

static void ActorBridgeSceneValueStep(CpuState *cpu, uint32_t entry);

static bool SceneValueHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return (pc == 0x83a92cu || pc == 0x83a961u) && hook == ActorBridgeSceneValueStep;
}

static unsigned PlanSceneValueStep(const CpuState *cpu, uint32_t entry,
                                   FixedSpanOpcode *plan, uint32_t *next) {
    unsigned count = 0u;
#define SCENE_VALUE_OPCODE(pc, fetched, cycles, data)                                  \
    (plan[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    if (entry == 0x83a92cu) {
        const bool window = (cpu->ram[0x9a7u] & 1u) != 0u;
        const unsigned sprite = cpu->ram[0x5d2u + cpu->X];
        const unsigned attribute = cpu->ram[0x1212u];
        SCENE_VALUE_OPCODE(0xa92cu, 3u, 4u, 1u);
        SCENE_VALUE_OPCODE(0xa92fu, 2u, 2u, 0u);
        SCENE_VALUE_OPCODE(0xa931u, 2u, window ? 2u : 3u, 0u);
        if (window) {
            SCENE_VALUE_OPCODE(0xa933u, 3u, 4u, 1u);
            SCENE_VALUE_OPCODE(0xa936u, 2u, 2u, 0u);
            SCENE_VALUE_OPCODE(0xa938u, 2u, sprite == 0xfdu ? 3u : 2u, 0u);
            if (sprite == 0xfdu) {
                *next = 0x83a96bu;
                return count;
            }
            SCENE_VALUE_OPCODE(0xa93au, 2u, 2u, 0u);
            SCENE_VALUE_OPCODE(0xa93cu, 2u, sprite >= 0x80u ? 3u : 2u, 0u);
            if (sprite < 0x80u) {
                SCENE_VALUE_OPCODE(0xa93eu, 2u, 2u, 0u);
                SCENE_VALUE_OPCODE(0xa940u, 2u, sprite < 0x0au ? 3u : 2u, 0u);
                if (sprite >= 0x0au) {
                    SCENE_VALUE_OPCODE(0xa942u, 2u, 2u, 0u);
                    SCENE_VALUE_OPCODE(0xa944u, 2u, 3u, 0u);
                    goto store;
                }
            }
        }
        SCENE_VALUE_OPCODE(0xa946u, 4u, 5u, 1u);
        SCENE_VALUE_OPCODE(0xa94au, 2u, attribute ? 3u : 2u, 0u);
        if (attribute)
            goto store;
        SCENE_VALUE_OPCODE(0xa94cu, 1u, 2u, 0u);
        SCENE_VALUE_OPCODE(0xa94du, 3u, 4u, 1u);
        SCENE_VALUE_OPCODE(0xa950u, 1u, 2u, 0u);
        SCENE_VALUE_OPCODE(0xa951u, 2u, 2u, 0u);
        SCENE_VALUE_OPCODE(0xa953u, 2u, sprite < 0x80u ? 3u : 2u, 0u);
        if (sprite < 0x80u) {
            *next = 0x83a95eu;
            return count;
        }
        SCENE_VALUE_OPCODE(0xa955u, 1u, 2u, 0u);
        SCENE_VALUE_OPCODE(0xa956u, 4u, 5u, 1u);
        SCENE_VALUE_OPCODE(0xa95au, 2u, cpu->x_flag ? 3u : 4u, cpu->x_flag ? 1u : 2u);
        SCENE_VALUE_OPCODE(0xa95cu, 2u, 3u, 0u);
    } else {
        SCENE_VALUE_OPCODE(0xa961u, 2u, cpu->_flag_C ? 2u : 3u, 0u);
        if (cpu->_flag_C) {
            SCENE_VALUE_OPCODE(0xa963u, 2u, 2u, 0u);
            SCENE_VALUE_OPCODE(0xa965u, 2u, 3u, 0u);
        } else {
            SCENE_VALUE_OPCODE(0xa967u, 1u, 2u, 0u);
        }
    }
store:
    SCENE_VALUE_OPCODE(0xa968u, 3u, cpu->x_flag ? 5u : 6u, 1u);
    *next = 0x83a96bu;
#undef SCENE_VALUE_OPCODE
    return count;
}

static void ActorBridgeSceneValueStep(CpuState *cpu, uint32_t entry) {
    const unsigned reserve = 640u;
    if (!SceneValueHookOwned(entry, ActorBridgeSceneValueStep) || cpu != &g_cpu ||
        !cpu->ram || cpu->PB != 0x83u || cpu->m_flag != 1u || cpu->x_flag > 1u ||
        cpu->D || cpu->_flag_D || cpu->X >= 40u || (cpu->DB & 0x7fu) >= 0x40u ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1fffu, false))
        return;
    uint32_t fence = 0x83a96bu;
    bool store = false;
    if (entry == 0x83a92cu) {
        const bool window = (cpu->ram[0x9a7u] & 1u) != 0u;
        const unsigned sprite = cpu->ram[0x5d2u + cpu->X];
        const unsigned attribute = cpu->ram[0x1212u];
        const bool omit = window && sprite == 0xfdu;
        const bool forced = window && sprite >= 0x0au && sprite < 0x80u;
        const bool table = !omit && !forced && !attribute && sprite >= 0x80u;
        if (table) {
            const unsigned slot =
                cpu->ram[0xa7u] | (cpu->x_flag ? 0u : (unsigned)cpu->ram[0xa8u] << 8);
            if (slot >= 40u || g_snes->cart->romSize <= 0x27f6b5u + sprite - 0x80u)
                return;
        }
        store = !omit && (forced || attribute || table);
        fence = omit        ? 0x83a93au
                : forced    ? 0x83a946u
                : attribute ? 0x83a94cu
                            : 0x83a95eu;
    }
    if (!Lufia2DecompFrameRangeAllows(entry, fence, SceneValueHookOwned) ||
        (store &&
         !Lufia2DecompFrameRangeAllows(0x83a968u, 0x83a96bu, SceneValueHookOwned)))
        return;
    FixedSpanOpcode timing[LUFIA2_ACTOR_SCENE_VALUE_MAX_OPCODES];
    uint32_t next;
    const unsigned count = PlanSceneValueStep(cpu, entry, timing, &next);
    if (Lufia2DecompFrameOwnedSpanBudget(entry, fence, ActorBridgeSceneValueStep) <
        count)
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
        Lufia2ActorSceneValueStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == next);
    assert(clock.index == count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
