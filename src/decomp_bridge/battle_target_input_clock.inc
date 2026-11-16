#include "lufia2/battle_target_name_steps.h"

static void ActorBridgeTargetInputPhase(CpuState *cpu, uint32_t entry);

static uint32_t TargetInputPhaseEnd(uint32_t entry) {
    switch (entry) {
    case 0x81d638u: return 0x81d640u;
    case 0x81d648u: return 0x81d64au;
    case 0x81d64eu: return 0x81d685u;
    default: return 0u;
    }
}

static bool TargetInputHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return TargetInputPhaseEnd(pc) && hook == ActorBridgeTargetInputPhase;
}

static unsigned TargetInputPlan(const CpuState *cpu, uint32_t entry,
    FixedSpanOpcode *timing) {
    unsigned count = 0u;
#define INPUT_OPCODE(pc, fetched, cycles, data) \
    (timing[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    if (entry == 0x81d638u) {
        INPUT_OPCODE(0xd638u, 2u, 2u, 0u);
        INPUT_OPCODE(0xd63au, 4u, 5u, 1u);
        INPUT_OPCODE(0xd63eu, 2u, 3u, 0u);
        return count;
    }
    if (entry == 0x81d648u) {
        INPUT_OPCODE(0xd648u, 2u, 3u, 0u);
        return count;
    }
    const uint8_t pad = cpu->ram[0xddu];
    INPUT_OPCODE(0xd64eu, 2u, 3u, 1u);
    INPUT_OPCODE(0xd650u, 2u, 2u, 0u);
    INPUT_OPCODE(0xd652u, 2u, pad & 0xa0u ? 2u : 3u, 0u);
    if (pad & 0xa0u) {
        INPUT_OPCODE(0xd654u, 3u, 3u, 0u);
        return count;
    }
    INPUT_OPCODE(0xd657u, 2u, 2u, 0u);
    INPUT_OPCODE(0xd659u, 2u, pad & 0x30u ? 2u : 3u, 0u);
    if (pad & 0x30u) {
        const bool all = (cpu->ram[0x24u] & 3u) == 1u;
        INPUT_OPCODE(0xd65bu, 2u, 3u, 1u);
        INPUT_OPCODE(0xd65du, 2u, 2u, 0u);
        INPUT_OPCODE(0xd65fu, 2u, 2u, 0u);
        INPUT_OPCODE(0xd661u, 2u, all ? 2u : 3u, 0u);
        if (all) {
            INPUT_OPCODE(0xd663u, 3u, 3u, 0u);
            return count;
        }
    }
    const uint8_t high = cpu->ram[0xdeu];
    INPUT_OPCODE(0xd666u, 2u, 3u, 1u);
    INPUT_OPCODE(0xd668u, 2u, high & 0x80u ? 2u : 3u, 0u);
    if (high & 0x80u) {
        INPUT_OPCODE(0xd66au, 3u, 3u, 0u);
        return count;
    }
    const uint8_t direction = g_snes->cart->rom[0xbb5aau + (high & 15u)];
    INPUT_OPCODE(0xd66du, 2u, 3u, 0u);
    INPUT_OPCODE(0xd66fu, 3u, 3u, 0u);
    INPUT_OPCODE(0xd672u, 1u, 2u, 0u);
    INPUT_OPCODE(0xd673u, 2u, 3u, 0u);
    INPUT_OPCODE(0xd675u, 4u, 5u, 1u);
    INPUT_OPCODE(0xd679u, 2u, direction ? 3u : 2u, 0u);
    if (!direction) {
        INPUT_OPCODE(0xd67bu, 3u, 3u, 0u);
        return count;
    }
    INPUT_OPCODE(0xd67eu, 2u, 2u, 0u);
    INPUT_OPCODE(0xd680u, 2u, direction & 1u ? 3u : 2u, 0u);
    if (!(direction & 1u))
        INPUT_OPCODE(0xd682u, 3u, 3u, 0u);
#undef INPUT_OPCODE
    return count;
}

static void ActorBridgeTargetInputPhase(CpuState *cpu, uint32_t entry) {
    const uint32_t end = TargetInputPhaseEnd(entry);
    const bool wide = entry == 0x81d648u;
    const unsigned reserve = entry == 0x81d64eu ? 768u : 256u;
    if (!end || cpu != &g_cpu || cpu->PB != 0x81u || cpu->DB != 0x7eu ||
        cpu->D || cpu->_flag_D || cpu->m_flag != !wide || cpu->x_flag ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1ffcu, wide) ||
        g_snes->cart->romSize <= 0xbb5b9u ||
        !Lufia2DecompFrameRangeAllows(entry, end, TargetInputHookOwned))
        return;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || (current && current != cpu->master_cycles))
        return;
    FixedSpanOpcode timing[24];
    const unsigned count = TargetInputPlan(cpu, entry, timing);
    if (Lufia2DecompFrameOwnedSpanBudget(entry, end, ActorBridgeTargetInputPhase) < count)
        return;
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x810000u, .timing = timing, .count = count};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleTargetInputStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    assert(clock.index == count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
