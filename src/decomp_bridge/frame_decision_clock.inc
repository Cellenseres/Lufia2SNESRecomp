#include "frame_decision_clock.h"
#include "lufia2/battle_input_steps.h"
#include "lufia2/world_scene_returns.h"
#if defined(LUFIA2_HAVE_NATIVE_WORLD_FRAME_LOOP)
#include "world_frame_owner.h"
#endif

static void ActorBridgeFrameDecision(CpuState *cpu, uint32_t entry);

static uint32_t FrameDecisionHookEnd(uint32_t pc, InterpPreOpcodeHook hook) {
    if (hook != ActorBridgeFrameDecision)
        return 0u;
    switch (pc & 0x7fffffu) {
    case 0x05ec85u: return (pc & 0xff0000u) | 0xec8bu;
    case 0x05ec8fu: return (pc & 0xff0000u) | 0xec94u;
    case 0x06925cu: return (pc & 0xff0000u) | 0x9261u;
    case 0x069264u: return (pc & 0xff0000u) | 0x9276u;
    case 0x069288u: return (pc & 0xff0000u) | 0x9290u;
    default: return 0u;
    }
}

bool Lufia2DecompFrameDecisionHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return FrameDecisionHookEnd(pc, hook) != 0u;
}

static unsigned PlanFrameDecision(const CpuState *cpu, uint32_t entry,
    FixedSpanOpcode *timing) {
    switch (entry) {
    case 0x85ec85u:
        timing[0] = (FixedSpanOpcode){0xec85u, 4u, 5u, 1u};
        timing[1] = (FixedSpanOpcode){0xec89u, 2u, cpu->ram[0x12f3u] ? 2u : 3u, 0u};
        return 2u;
    case 0x85ec8fu:
        timing[0] = (FixedSpanOpcode){0xec8fu, 1u, 2u, 0u};
        timing[1] = (FixedSpanOpcode){0xec90u, 4u, 5u, 1u};
        return 2u;
    case 0x86925cu:
        timing[0] = (FixedSpanOpcode){0x925cu, 3u, 4u, 1u};
        timing[1] = (FixedSpanOpcode){0x925fu, 2u, cpu->ram[0x057cu] ? 2u : 3u, 0u};
        return 2u;
    case 0x869264u: {
        const bool column_centered = (cpu->ram[0x11e8u] & 15u) == 8u;
        const bool row_centered = (cpu->ram[0x11eau] & 15u) == 8u;
        timing[0] = (FixedSpanOpcode){0x9264u, 3u, 4u, 1u};
        timing[1] = (FixedSpanOpcode){0x9267u, 2u, 2u, 0u};
        timing[2] = (FixedSpanOpcode){0x9269u, 2u, 2u, 0u};
        timing[3] = (FixedSpanOpcode){0x926bu, 2u, column_centered ? 2u : 3u, 0u};
        if (!column_centered)
            return 4u;
        timing[4] = (FixedSpanOpcode){0x926du, 3u, 4u, 1u};
        timing[5] = (FixedSpanOpcode){0x9270u, 2u, 2u, 0u};
        timing[6] = (FixedSpanOpcode){0x9272u, 2u, 2u, 0u};
        timing[7] = (FixedSpanOpcode){0x9274u, 2u, row_centered ? 2u : 3u, 0u};
        return 8u;
    }
    case 0x869288u:
        timing[0] = (FixedSpanOpcode){0x9288u, 3u, 4u, 1u};
        timing[1] = (FixedSpanOpcode){0x928bu, 3u, 4u, 1u};
        timing[2] = (FixedSpanOpcode){0x928eu, 2u,
            (cpu->ram[0x1710u] | cpu->ram[0x1711u]) ? 3u : 2u, 0u};
        return 3u;
    default:
        return 0u;
    }
}

static void ActorBridgeFrameDecision(CpuState *cpu, uint32_t entry) {
    const uint32_t end = FrameDecisionHookEnd(entry, ActorBridgeFrameDecision);
    if (!end || entry < 0x800000u || cpu->D || cpu->_flag_D ||
        cpu->m_flag != 1u || cpu->x_flag ||
        ((entry >> 16) == 0x86u && cpu->DB != 0x86u) ||
        !g_snes || g_snes->vPos >= 262u ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->master_cycles != cpu->coprocessor_master_cycles ||
        cpu->master_cycles > UINT64_MAX - 256u ||
        Lufia2DecompFrameSpanBudget(entry, end, ActorBridgeFrameDecision) < 8u ||
        !NativeOpcodeWindowSupported(cpu, entry, 256u, 0x1ffcu, false))
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
#if defined(LUFIA2_HAVE_NATIVE_WORLD_FRAME_LOOP)
    if (entry == 0x86925cu && Lufia2DecompWorldFrameCanResume(cpu))
        return;
#endif
    FixedSpanOpcode timing[8];
    const unsigned count = PlanFrameDecision(cpu, entry, timing);
    if (!count)
        return;
    FixedSpanClock clock = {.cpu = cpu, .bank = entry & 0xff0000u,
        .timing = timing, .count = count};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result;
    switch (entry) {
    case 0x85ec85u: result = Lufia2BattleCheckSpriteRequest(&memory, &state); break;
    case 0x85ec8fu: result = Lufia2BattleClearSpriteRequest(&memory, &state); break;
    case 0x86925cu: result = Lufia2WorldMapCheckFrameMovement(&memory, &state); break;
    case 0x869264u: result = Lufia2WorldMapCheckFramePosition(&memory, &state); break;
    default: result = Lufia2WorldMapCheckFramePlaneState(&memory, &state); break;
    }
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    assert(clock.index == count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
