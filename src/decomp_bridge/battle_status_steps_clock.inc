#include "lufia2/battle_status_steps.h"

_Static_assert(LUFIA2_BATTLE_STATUS_MAX_SYMBOLS * 9u ==
    LUFIA2_BATTLE_STATUS_MAX_OPCODES, "Status symbol clock bound");
_Static_assert(LUFIA2_BATTLE_STATUS_MAX_OPCODES <= 32u, "Status progress mask");

static void ActorBridgeBattleStatusStep(CpuState *cpu, uint32_t entry) {
    if (!BattleStatusStepHookOwned(entry, ActorBridgeBattleStatusStep) || cpu != &g_cpu ||
        cpu->PB != 0x81u || cpu->DB != 0x7eu || cpu->D || cpu->_flag_D ||
        cpu->m_flag != 1u || cpu->x_flag || !cpu->ram || cpu->S < 0x100u ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return;
    if (entry == 0x81e1eau) {
        if (cpu->S > 0x1ff0u)
            return;
        const unsigned record = cpu->ram[cpu->S + 3u] |
            (unsigned)cpu->ram[cpu->S + 4u] << 8;
        if (record < 0x200u || record > 0xff43u)
            return;
    }
    const bool symbols = entry == 0x81e1dau;
    const uint32_t fence = symbols ? 0x81e1eau
        : entry == 0x81e1eau ? 0x81e1f8u : 0x81e2aeu;
    if (!Lufia2DecompFrameRangeAllows(entry, fence, BattleStatusStepHookOwned))
        return;
    const unsigned budget = Lufia2DecompFrameSpanBudget(entry, fence,
        ActorBridgeBattleStatusStep);
    unsigned steps = 1u;
    unsigned count = entry == 0x81e1eau ? 8u : 2u;
    unsigned reserve = entry == 0x81e1eau ? 288u : 128u;
    if (symbols) {
        const unsigned remaining = cpu->ram[0x1bu] ? cpu->ram[0x1bu] : 256u;
        steps = remaining < LUFIA2_BATTLE_STATUS_MAX_SYMBOLS
            ? remaining : LUFIA2_BATTLE_STATUS_MAX_SYMBOLS;
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
    if (symbols) {
        const unsigned remaining = cpu->ram[0x1bu] ? cpu->ram[0x1bu] : 256u;
        for (unsigned i = 0u; i < steps; ++i)
            memcpy(timing + 9u * i, i + 1u == remaining
                ? status_detail_symbols_last : status_detail_symbols,
                9u * sizeof timing[0]);
    } else {
        memcpy(timing, entry == 0x81e1eau ? status_detail_level_call : status_detail_end,
            count * sizeof timing[0]);
    }
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry,
        (unsigned)(fence - entry), &fetch);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x810000u, .timing = timing,
        .count = count, .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = Lufia2BattleStatusStep(&memory, &state, entry, steps);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY &&
        result.pc == (symbols && !state.zero ? entry : fence));
    assert(clock.index == count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
