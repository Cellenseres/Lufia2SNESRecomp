#include "lufia2/battle_status_steps.h"

_Static_assert(LUFIA2_BATTLE_STATUS_MAX_SYMBOLS * 9u <=
                   LUFIA2_BATTLE_STATUS_MAX_OPCODES,
               "Status symbol clock bound");
_Static_assert(LUFIA2_BATTLE_STATUS_MAX_OPCODES <= 32u, "Status progress mask");

static unsigned StatusStackWord(const CpuState *cpu, unsigned offset) {
    return cpu->ram[cpu->S + offset] | (unsigned)cpu->ram[cpu->S + offset + 1u] << 8;
}

static bool StatusStepInputsFit(const CpuState *cpu, uint32_t entry) {
    if (entry == 0x81e1b5u)
        return cpu->X <= 6u && !(cpu->X & 1u) &&
               (cpu->DB == 0x7eu || cpu->DB == 0x7fu || (cpu->DB & 0x7fu) < 0x40u);
    if (cpu->DB != 0x7eu)
        return false;
    if (entry == 0x81e290u) {
        const unsigned slot = StatusStackWord(cpu, 1u);
        return slot <= 6u && !(slot & 1u) && cpu->Y >= 0x2000u && cpu->Y <= 0xfffbu;
    }
    if (entry == 0x81e1eau || entry == 0x81e1fbu || entry == 0x81e23bu ||
        entry == 0x81e261u) {
        const unsigned record = StatusStackWord(cpu, entry == 0x81e1fbu ? 1u : 3u);
        if (record < 0x200u || record > 0xff43u)
            return false;
    }
    if (entry == 0x81e1fbu)
        return cpu->Y >= 0x2000u && cpu->Y <= 0xffefu;
    if (entry == 0x81e261u) {
        const unsigned cursor = (StatusStackWord(cpu, 1u) + 0x40u) & 0xffffu;
        return cursor >= 0x2000u && cursor <= 0xfff7u;
    }
    return true;
}

static void AppendStatusStepTiming(FixedSpanOpcode *timing, unsigned *count,
                                   const FixedSpanOpcode *source, unsigned size) {
    memcpy(timing + *count, source, size * sizeof timing[0]);
    *count += size;
}

static unsigned PrepareStatusStepClock(const CpuState *cpu, uint32_t entry,
                                       FixedSpanOpcode *timing, unsigned symbols) {
    unsigned count = 0u;
    switch (entry) {
    case 0x81e1b5u:
        AppendStatusStepTiming(timing, &count, status_detail_begin, 22u);
        break;
    case 0x81e1dau: {
        const unsigned remaining = cpu->ram[0x1bu] ? cpu->ram[0x1bu] : 256u;
        for (unsigned i = 0u; i < symbols; ++i)
            AppendStatusStepTiming(timing, &count,
                                   i + 1u == remaining ? status_detail_symbols_last
                                                       : status_detail_symbols,
                                   9u);
        break;
    }
    case 0x81e1eau:
        AppendStatusStepTiming(timing, &count, status_detail_level_call, 8u);
        break;
    case 0x81e1fbu:
        AppendStatusStepTiming(timing, &count, status_detail_level_prefix, 5u);
        AppendStatusStepTiming(timing, &count,
                               cpu->ram[0xb3u] ? status_detail_level_double
                                               : status_detail_level_single,
                               cpu->ram[0xb3u] ? 4u : 1u);
        AppendStatusStepTiming(timing, &count, status_detail_level_end, 21u);
        break;
    case 0x81e23bu:
        AppendStatusStepTiming(timing, &count, status_detail_mp_digits, 18u);
        break;
    case 0x81e261u:
        AppendStatusStepTiming(timing, &count, status_detail_ip_gauge, 24u);
        break;
    case 0x81e290u:
        AppendStatusStepTiming(timing, &count, status_detail_ip_gauge + 24u, 11u);
        break;
    default:
        AppendStatusStepTiming(timing, &count, status_detail_end, 2u);
        break;
    }
    return count;
}

static void ActorBridgeBattleStatusStep(CpuState *cpu, uint32_t entry) {
    if (!BattleStatusStepHookOwned(entry, ActorBridgeBattleStatusStep) ||
        cpu != &g_cpu || cpu->PB != 0x81u || cpu->D || cpu->_flag_D ||
        cpu->m_flag != 1u || cpu->x_flag || !cpu->ram || cpu->S < 0x1f00u ||
        cpu->S > 0x1ff0u || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !StatusStepInputsFit(cpu, entry))
        return;
    const bool symbols = entry == 0x81e1dau;
    uint32_t fence;
    unsigned count, reserve;
    switch (entry) {
    case 0x81e1b5u:
        fence = 0x81e1dau;
        count = 22u;
        reserve = 672u;
        break;
    case 0x81e1dau:
        fence = 0x81e1eau;
        count = 9u;
        reserve = 296u;
        break;
    case 0x81e1eau:
        fence = 0x81e1f8u;
        count = 8u;
        reserve = 288u;
        break;
    case 0x81e1fbu:
        fence = 0x81e235u;
        count = cpu->ram[0xb3u] ? 30u : 27u;
        reserve = 864u;
        break;
    case 0x81e23bu:
        fence = 0x81e25bu;
        count = 18u;
        reserve = 576u;
        break;
    case 0x81e261u:
        fence = 0x81e290u;
        count = 24u;
        reserve = 704u;
        break;
    case 0x81e290u:
        fence = 0x81e2a9u;
        count = 11u;
        reserve = 416u;
        break;
    default:
        fence = 0x81e2aeu;
        count = 2u;
        reserve = 128u;
        break;
    }
    if (!Lufia2DecompFrameRangeAllows(entry, fence, BattleStatusStepHookOwned))
        return;
    const unsigned budget =
        Lufia2DecompFrameSpanBudget(entry, fence, ActorBridgeBattleStatusStep);
    unsigned steps = 1u;
    if (symbols) {
        const unsigned remaining = cpu->ram[0x1bu] ? cpu->ram[0x1bu] : 256u;
        steps = remaining < LUFIA2_BATTLE_STATUS_MAX_SYMBOLS
                    ? remaining
                    : LUFIA2_BATTLE_STATUS_MAX_SYMBOLS;
        if (steps > budget / 9u)
            steps = budget / 9u;
        for (; steps; --steps) {
            reserve = steps * 256u + 40u;
            if (cpu->Y >= 0x2000u && cpu->Y <= 0xffffu - 2u * steps &&
                cpu->master_cycles <= UINT64_MAX - reserve &&
                NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1ff0u, false))
                break;
        }
        count = steps * 9u;
    }
    if (!steps || budget < count || cpu->master_cycles > UINT64_MAX - reserve ||
        (!symbols && !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1ff0u, false)))
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    FixedSpanOpcode timing[LUFIA2_BATTLE_STATUS_MAX_OPCODES];
    const unsigned prepared_count = PrepareStatusStepClock(cpu, entry, timing, steps);
    assert(prepared_count == count);
    (void)prepared_count;
    Lufia2DecompRomOpcodeSpan fetch;
    const unsigned bytes = (unsigned)(fence - entry);
    const bool prepared =
        Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, bytes < 32u ? bytes : 32u, &fetch);
    FixedSpanClock clock = {.cpu = cpu,
                            .bank = 0x810000u,
                            .timing = timing,
                            .count = count,
                            .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result =
        Lufia2BattleStatusStep(&memory, &state, entry, steps);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY &&
           result.pc == (symbols && !state.zero ? entry : fence));
    assert(clock.index == count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
