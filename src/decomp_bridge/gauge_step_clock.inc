#include "lufia2/battle_gauge_steps.h"

static bool GaugeOutputRangeSupported(unsigned start, unsigned end) {
    return end <= 0x10000u && (end <= 0x11u || start >= 0x17u) &&
        (end <= 0x4eu || start >= 0x54u) && (end <= 0x1ef8u || start >= 0x2000u);
}

static void ActorBridgeGaugeStep(CpuState *cpu, uint32_t entry) {
    if (!GaugeStepHookOwned(entry, ActorBridgeGaugeStep) || cpu != &g_cpu ||
        cpu->PB != 0x81u || cpu->D || cpu->_flag_D || cpu->x_flag ||
        !Lufia2DecompOpcodeWindowReady() || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return;
    FixedSpanOpcode timing[LUFIA2_GAUGE_PART_MAX_OPCODES];
    unsigned count, reserve, part = 0u;
    uint32_t end;
    switch (entry) {
    case 0x81e308u:
        for (unsigned i = 0u; i < 3u; ++i)
            timing[i] = battle_gauge_begin[i];
        timing[3] = (FixedSpanOpcode){0xe30eu, 2u, 2u, 0u};
        count = LUFIA2_GAUGE_BEGIN_OPCODES;
        reserve = 160u;
        end = 0x81e310u;
        break;
    case 0x81e339u:
        for (unsigned i = 0u; i < LUFIA2_GAUGE_READ_OPCODES; ++i)
            timing[i] = battle_gauge_part_read[i];
        count = LUFIA2_GAUGE_READ_OPCODES;
        reserve = 96u;
        end = 0x81e33du;
        break;
    case 0x81e310u:
        for (unsigned i = 0u; i < LUFIA2_GAUGE_SCALE_OPCODES; ++i)
            timing[i] = battle_gauge_multiply_call[i + 1u];
        count = LUFIA2_GAUGE_SCALE_OPCODES;
        reserve = 160u;
        end = 0x81e318u;
        break;
    default: {
        static const uint32_t entries[] = {0x81e33du, 0x81e352u, 0x81e369u, 0x81e380u};
        static const uint32_t ends[] = {0x81e352u, 0x81e369u, 0x81e380u, 0x81e38eu};
        static const unsigned reserves[] = {288u, 304u, 304u, 192u};
        while (entries[part] != entry)
            ++part;
        const unsigned start = cpu->Y + 10u + part * 2u;
        if (cpu->m_flag != 1u || cpu->DB != 0x7eu ||
            !GaugeOutputRangeSupported(start, start + 1u))
            return;
        BattleGaugeClock plan;
        plan.part = part;
        plan.remaining = (uint8_t)cpu->A;
        SetBattleGaugePhase(&plan, GAUGE_PART_CAP);
        count = plan.span.count;
        for (unsigned i = 0u; i < count; ++i)
            timing[i] = plan.span.timing[i];
        SetBattleGaugePhase(&plan, GAUGE_PART_BODY);
        assert(count + plan.span.count <= LUFIA2_GAUGE_PART_MAX_OPCODES);
        for (unsigned i = 0u; i < plan.span.count; ++i)
            timing[count++] = plan.span.timing[i];
        reserve = reserves[part];
        end = ends[part];
        break;
    }
    }
    if (cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1fffu, true) ||
        !Lufia2DecompFrameRangeAllows(entry, end, GaugeStepHookOwned) ||
        Lufia2DecompFrameSpanBudget(entry, end, ActorBridgeGaugeStep) < count)
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, end - entry, &fetch);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x810000u, .timing = timing,
        .count = count, .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = entry == 0x81e310u
        ? Lufia2BattlePrepareGaugeScaleStep(&memory, &state)
        : (entry == 0x81e308u || entry == 0x81e339u)
            ? Lufia2BattleBeginGaugeStep(&memory, &state, entry)
            : Lufia2BattleGaugePartStep(&memory, &state, entry);
    if (entry == 0x81e308u)
        timing[3].cycles = result.pc == 0x81e339u ? 3u : 2u;
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    assert(clock.index == clock.count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
