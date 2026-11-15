#include "effect_frame_clock.h"
#include "lufia2/battle_effect_slot_steps.h"

enum { EFFECT_FRAME_ACTIVE_ADDRESS = 0x1becu };

static void ActorBridgeEffectFrameStep(CpuState *cpu, uint32_t entry);

uint32_t Lufia2DecompEffectFrameHookEnd(uint32_t pc, InterpPreOpcodeHook hook) {
    if (hook != ActorBridgeEffectFrameStep)
        return 0u;
    switch (pc) {
    case 0x818a76u:
    case 0x818ab9u:
        return pc + 8u;
    case 0x818b2du:
    case 0x818b3cu:
    case 0x818b4bu:
    case 0x818b5au:
        return pc + 7u;
    case 0x818bc1u:
        return 0x818bc7u;
    case 0x818bcbu:
        return 0x818bd6u;
    default:
        return 0u;
    }
}

static bool EffectFrameHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return Lufia2DecompEffectFrameHookEnd(pc, hook) != 0u;
}

static bool EffectFrameClockSupported(const CpuState *cpu, uint32_t entry,
    uint32_t end, unsigned count, unsigned budget) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return false;
#endif
    if (cpu != &g_cpu || cpu->PB != 0x81u || cpu->DB != 0x7eu ||
        cpu->m_flag != 1u || cpu->x_flag || cpu->D || cpu->_flag_D ||
        !cpu->ram || !g_snes || g_snes->ram != cpu->ram ||
        g_snes->vPos >= 262u || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !Lufia2DecompFrameRangeAllows(entry, end, EffectFrameHookOwned) ||
        Lufia2DecompFrameSpanBudget(entry, end, ActorBridgeEffectFrameStep) < count)
        return false;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || charged != cpu->master_cycles)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    return (budget < fence && ActorBridgeTimedStepSupportedWindow(cpu, entry,
                0x1fffu, false, fence - budget - 1u, budget)) ||
        NativeOpcodeWindowSupported(cpu, entry, budget, 0x1fffu, false);
}

static void ActorBridgeEffectFrameStep(CpuState *cpu, uint32_t entry) {
    if (!cpu->ram)
        return;
    FixedSpanOpcode timing[4];
    unsigned count, bytes, data_and_refresh;
    switch (entry) {
    case 0x818a76u:
    case 0x818ab9u:
        timing[0] = (FixedSpanOpcode){(uint16_t)entry, 2u, 2u, 0u};
        timing[1] = (FixedSpanOpcode){(uint16_t)(entry + 2u), 3u, 4u, 1u};
        timing[2] = (FixedSpanOpcode){(uint16_t)(entry + 5u), 3u, 3u, 0u};
        count = 3u;
        bytes = 8u;
        data_and_refresh = 48u;
        break;
    case 0x818b2du:
    case 0x818b3cu:
    case 0x818b4bu:
    case 0x818b5au: {
        static const uint16_t requests[] = {0x15b0u, 0x15aeu, 0x15adu, 0x15afu};
        const unsigned layer = (entry - 0x818b2du) / 15u;
        const bool empty = cpu->ram[requests[layer]] == 0u;
        timing[0] = (FixedSpanOpcode){(uint16_t)entry, 2u, 2u, 0u};
        timing[1] = (FixedSpanOpcode){(uint16_t)(entry + 2u), 3u, 6u, 2u};
        timing[2] = (FixedSpanOpcode){(uint16_t)(entry + 5u), 2u,
            (uint8_t)(empty ? 3u : 2u), 0u};
        count = 3u;
        bytes = 7u;
        data_and_refresh = 68u;
        break;
    }
    case 0x818bc1u:
        timing[0] = (FixedSpanOpcode){0x8bc1u, 2u, 2u, 0u};
        timing[1] = (FixedSpanOpcode){0x8bc3u, 4u, 5u, 1u};
        count = 2u;
        bytes = 6u;
        data_and_refresh = 48u;
        break;
    case 0x818bcbu: {
        const bool finished = cpu->ram[EFFECT_FRAME_ACTIVE_ADDRESS] == 0u;
        timing[0] = (FixedSpanOpcode){0x8bcbu, 3u, 6u, 2u};
        timing[1] = (FixedSpanOpcode){0x8bceu, 3u, 4u, 1u};
        timing[2] = (FixedSpanOpcode){0x8bd1u, 2u,
            (uint8_t)(finished ? 3u : 2u), 0u};
        timing[3] = (FixedSpanOpcode){0x8bd3u, 3u, 3u, 0u};
        count = finished ? 3u : 4u;
        bytes = 11u;
        data_and_refresh = 76u;
        break;
    }
    default:
        return;
    }
    const uint32_t end = Lufia2DecompEffectFrameHookEnd(entry, ActorBridgeEffectFrameStep);
    const unsigned budget = bytes * cpu_region_speed(entry) + data_and_refresh;
    if (!EffectFrameClockSupported(cpu, entry, end, count, budget))
        return;
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x810000u;
    clock.timing = timing;
    clock.count = count;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleEffectFrameStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
