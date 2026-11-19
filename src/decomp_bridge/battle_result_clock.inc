#include "lufia2/battle_result_steps.h"

static void ActorBridgeResultPhase(CpuState *cpu, uint32_t entry);

static uint32_t ResultPhaseEnd(uint32_t entry) {
    switch (entry) {
    case 0x81de8eu: return 0x81de8fu;
    case 0x81de88u:
    case 0x81de93u:
    case 0x81de95u: return 0x81de9du;
    case 0x81dea2u: return 0x81dea8u;
    default: return 0u;
    }
}

static bool ResultPhaseHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return ResultPhaseEnd(pc) && hook == ActorBridgeResultPhase;
}

static unsigned ResultPhasePlan(const CpuState *cpu, uint32_t entry,
    FixedSpanOpcode *timing) {
    unsigned count = 0u;
#define RESULT_OPCODE(pc, fetched, cycles, data) \
    (timing[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    if (entry == 0x81de8eu) {
        RESULT_OPCODE(0xde8eu, 1u, 3u, 1u);
        return count;
    }
    if (entry == 0x81dea2u) {
        const bool confirm = (cpu->ram[0xddu] & 0xa0u) != 0u;
        RESULT_OPCODE(0xdea2u, 2u, 3u, 1u);
        RESULT_OPCODE(0xdea4u, 2u, 2u, 0u);
        RESULT_OPCODE(0xdea6u, 2u, confirm ? 2u : 3u, 0u);
        return count;
    }
    bool finished = cpu->_flag_Z != 0u;
    if (entry == 0x81de88u) {
        RESULT_OPCODE(0xde88u, 2u, 3u, 0u);
        RESULT_OPCODE(0xde8au, 2u, 2u, 0u);
        RESULT_OPCODE(0xde8cu, 2u, 3u, 0u);
        finished = false;
    } else if (entry == 0x81de93u) {
        RESULT_OPCODE(0xde93u, 1u, 4u, 1u);
        RESULT_OPCODE(0xde94u, 1u, 2u, 0u);
        finished = cpu->ram[cpu->S + 1u] == 0xffu;
    }
    RESULT_OPCODE(0xde95u, 3u, 4u, 1u);
    RESULT_OPCODE(0xde98u, 3u, 4u, 1u);
    RESULT_OPCODE(0xde9bu, 2u, finished ? 2u : 3u, 0u);
#undef RESULT_OPCODE
    return count;
}

static void ActorBridgeResultPhase(CpuState *cpu, uint32_t entry) {
    const uint32_t end = ResultPhaseEnd(entry);
    const bool wide = entry == 0x81de88u;
    const bool push = entry == 0x81de8eu;
    const unsigned reserve = push ? 64u : 256u;
    if (!end || cpu != &g_cpu || cpu->PB != 0x81u ||
        (cpu->DB != 0x7eu && (entry != 0x81dea2u || cpu->DB != 0x81u)) ||
        cpu->D || cpu->_flag_D || cpu->m_flag != !wide || cpu->x_flag ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !Lufia2DecompFrameRangeAllows(entry, end, ResultPhaseHookOwned))
        return;
    if (push) {
        if (!NativeOpcodeWindowTimingSupported(cpu, entry, reserve, 0x1ffcu, false) ||
            g_snes->dma->hdmaPendingInit || g_snes->cart->romSize <= 0xde8eu ||
            g_snes->cart->rom[0xde8eu] != 0x48u)
            return;
        const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u &&
            g_snes->hPos < 1024u ? 1024u : 1364u;
        if (reserve >= fence - g_snes->hPos)
            return;
    } else if (!NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1ffcu, wide)) {
        return;
    }
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || (current && current != cpu->master_cycles))
        return;
    FixedSpanOpcode timing[6];
    const unsigned count = ResultPhasePlan(cpu, entry, timing);
    if (Lufia2DecompFrameOwnedSpanBudget(entry, end, ActorBridgeResultPhase) < count)
        return;
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x810000u, .timing = timing, .count = count};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    uint32_t next;
    if (push) {
        FixedSpanWrite(&clock, state.stack, (uint8_t)state.accumulator);
        --state.stack;
        next = end;
    } else {
        const Lufia2ExecutionResult result = Lufia2BattleResultStep(&memory, &state, entry);
        assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
        next = result.pc;
    }
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(next);
}
