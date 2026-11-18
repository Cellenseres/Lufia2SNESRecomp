#include "lufia2/field_actor_rebuild_steps.h"

_Static_assert(LUFIA2_FIELD_ACTOR_REBUILD_MAX_OPCODES <= 32u,
               "Field actor progress mask");

static void ActorBridgeFieldActorStep(CpuState *cpu, uint32_t entry);

static bool FieldActorStepHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return hook == ActorBridgeFieldActorStep &&
           (pc == 0x83a776u || pc == 0x83a787u || pc == 0x83a792u || pc == 0x83a7c2u);
}

static unsigned PlanFieldActorStep(const CpuState *cpu, uint32_t entry,
                                   FixedSpanOpcode *plan, uint32_t *next) {
    const unsigned base = cpu->DB == 0x7fu ? 0x10000u : 0u;
    unsigned slot = cpu->X, count = 0u;
#define FIELD_ACTOR_OPCODE(pc, fetched, cycles, data)                                  \
    (plan[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    if (entry == 0x83a787u) {
        const bool object = cpu->ram[base + 0x5d2u + slot] == 0xfeu;
        FIELD_ACTOR_OPCODE(0xa787u, 3u, 4u, 1u);
        FIELD_ACTOR_OPCODE(0xa78au, 2u, 2u, 0u);
        FIELD_ACTOR_OPCODE(0xa78cu, 2u, object ? 3u : 2u, 0u);
        if (object)
            FIELD_ACTOR_OPCODE(0xa7b9u, 2u, 3u, 0u);
        *next = object ? 0x83a7bbu : 0x83a78eu;
        return count;
    }
    if (entry == 0x83a792u) {
        const bool window = (cpu->ram[base + 0x9a7u] & 1u) != 0u;
        FIELD_ACTOR_OPCODE(0xa792u, 3u, 4u, 1u);
        FIELD_ACTOR_OPCODE(0xa795u, 2u, 2u, 0u);
        FIELD_ACTOR_OPCODE(0xa797u, 2u, window ? 3u : 2u, 0u);
        if (!window) {
            slot = cpu->ram[0xa7u];
            const bool fixed = (cpu->ram[base + 0x622u + slot] & 0x20u) != 0u;
            FIELD_ACTOR_OPCODE(0xa799u, 2u, 3u, 1u);
            FIELD_ACTOR_OPCODE(0xa79bu, 3u, 4u, 1u);
            FIELD_ACTOR_OPCODE(0xa79eu, 2u, 2u, 0u);
            FIELD_ACTOR_OPCODE(0xa7a0u, 2u, fixed ? 3u : 2u, 0u);
            if (!fixed) {
                FIELD_ACTOR_OPCODE(0xa7a2u, 2u, 2u, 0u);
                FIELD_ACTOR_OPCODE(0xa7a4u, 4u, 5u, 1u);
            }
        }
        FIELD_ACTOR_OPCODE(0xa7a8u, 3u, 4u, 1u);
        FIELD_ACTOR_OPCODE(0xa7abu, 3u, 5u, 1u);
        *next = 0x83a7aeu;
        return count;
    }
    if (entry == 0x83a7c2u) {
        FIELD_ACTOR_OPCODE(0xa7c2u, 1u, 2u, 0u);
        FIELD_ACTOR_OPCODE(0xa7c3u, 2u, slot ? 3u : 2u, 0u);
        if (!slot--) {
            *next = 0x83a7c5u;
            return count;
        }
    }
    for (unsigned visited = 0u; visited < 2u; ++visited) {
        const unsigned sprite = cpu->ram[base + 0x5d2u + slot];
        FIELD_ACTOR_OPCODE(0xa776u, 3u, 4u, 1u);
        FIELD_ACTOR_OPCODE(0xa779u, 2u, 2u, 0u);
        FIELD_ACTOR_OPCODE(0xa77bu, 2u, sprite == 0xfeu ? 3u : 2u, 0u);
        if (sprite != 0xfeu) {
            FIELD_ACTOR_OPCODE(0xa77du, 2u, 2u, 0u);
            FIELD_ACTOR_OPCODE(0xa77fu, 2u, sprite == 0xffu ? 3u : 2u, 0u);
        }
        if (sprite != 0xffu) {
            FIELD_ACTOR_OPCODE(0xa781u, 2u, 3u, 1u);
            *next = 0x83a783u;
            return count;
        }
        FIELD_ACTOR_OPCODE(0xa7c2u, 1u, 2u, 0u);
        FIELD_ACTOR_OPCODE(0xa7c3u, 2u, slot ? 3u : 2u, 0u);
        if (!slot--) {
            *next = 0x83a7c5u;
            return count;
        }
    }
    *next = 0x83a776u;
#undef FIELD_ACTOR_OPCODE
    return count;
}

static void ActorBridgeFieldActorStep(CpuState *cpu, uint32_t entry) {
    const unsigned reserve = 512u;
    if (!FieldActorStepHookOwned(entry, ActorBridgeFieldActorStep) || cpu != &g_cpu ||
        !cpu->ram || cpu->PB != 0x83u || cpu->m_flag != 1u || cpu->x_flag != 1u ||
        cpu->D || cpu->_flag_D || cpu->X >= 40u ||
        !((cpu->DB & 0x7fu) < 0x40u || cpu->DB == 0x7eu || cpu->DB == 0x7fu) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1fffu, false))
        return;
    const unsigned base = cpu->DB == 0x7fu ? 0x10000u : 0u;
    if (entry == 0x83a792u && !(cpu->ram[base + 0x9a7u] & 1u) && cpu->ram[0xa7u] >= 40u)
        return;
    if (entry == 0x83a787u && cpu->ram[base + 0x5d2u + cpu->X] == 0xfeu &&
        !Lufia2DecompFrameRangeQuiet(0x83a7b9u, 0x83a7bbu))
        return;
    const bool loop = entry == 0x83a776u || entry == 0x83a7c2u;
    const uint32_t fence = entry == 0x83a776u   ? 0x83a783u
                           : entry == 0x83a787u ? 0x83a78eu
                           : entry == 0x83a792u ? 0x83a7aeu
                                                : 0x83a7c5u;
    if (!Lufia2DecompFrameRangeAllows(entry, fence, FieldActorStepHookOwned) ||
        (loop && !Lufia2DecompFrameRangeAllows(0x83a7c2u, 0x83a7c5u,
                                               FieldActorStepHookOwned)) ||
        (entry == 0x83a7c2u && cpu->X &&
         !Lufia2DecompFrameRangeAllows(0x83a776u, 0x83a783u, FieldActorStepHookOwned)))
        return;
    FixedSpanOpcode timing[LUFIA2_FIELD_ACTOR_REBUILD_MAX_OPCODES];
    uint32_t next;
    const unsigned count = PlanFieldActorStep(cpu, entry, timing, &next);
    if (Lufia2DecompFrameOwnedSpanBudget(entry, fence, ActorBridgeFieldActorStep) <
        count)
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || charged != cpu->master_cycles)
        return;
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared =
        Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, fence - entry, &fetch);
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
        Lufia2FieldRebuildActorStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == next);
    assert(clock.index == count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
