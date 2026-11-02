#include "dissolve_mask_plans.inc"
#include "dissolve_counts_plans.inc"

typedef struct DissolveMaskClock {
    FixedSpanClock span;
    const Lufia2CpuState *state;
    uint64_t reserved_until;
    unsigned budget;
    unsigned opcodes;
    unsigned last_write;
    uint8_t advance_low;
} DissolveMaskClock;

#define DISSOLVE_MASK_PLAN(clock, name) do { \
    (clock)->span.timing = dissolve_mask_##name; \
    (clock)->span.count = sizeof dissolve_mask_##name / sizeof dissolve_mask_##name[0]; \
} while (0)

static void FinishDissolveMaskPhase(DissolveMaskClock *clock) {
    BeginFixedSpanOpcode(&clock->span);
    assert(!clock->span.remaining && clock->span.index == clock->span.count);
    clock->opcodes += clock->span.count;
    clock->span.count = clock->span.index = clock->span.progress_mask = 0u;
}

static uint8_t DissolveMaskRead(void *context, uint32_t address) {
    DissolveMaskClock *clock = context;
    BeginFixedSpanOpcode(&clock->span);
    const uint16_t pc = clock->span.timing[clock->span.index].pc;
    const bool indexed = pc == 0xe7d8u || pc == 0xe7dbu ||
        pc == 0xe7deu || pc == 0xe7e1u;
    const unsigned low = pc == 0xe7dbu ? 1u : pc == 0xe7deu ? 16u :
        pc == 0xe7e1u ? 17u : 0u;
    const unsigned remaining = clock->span.remaining;
    const uint8_t value = indexed && clock->state->x + low > 0xffu
        ? ReadDissolveCrossedPage(&clock->span, address)
        : FixedSpanRead(&clock->span, address);
    if (pc == 0xe7fbu) {
        if (remaining == 2u) clock->advance_low = value;
        else if ((uint16_t)(clock->advance_low | ((uint16_t)value << 8)) < 32u)
            DISSOLVE_MASK_PLAN(clock, advance_end);
    }
    return value;
}

static void DissolveMaskWrite(void *context, uint32_t address, uint8_t value) {
    DissolveMaskClock *clock = context;
    BeginFixedSpanOpcode(&clock->span);
    const uint16_t pc = clock->span.timing[clock->span.index].pc;
    FixedSpanWrite(&clock->span, address, value);
    clock->last_write = clock->opcodes + clock->span.index +
        (clock->span.remaining != 0u);
    if (pc == 0xe7eeu && !value)
        DISSOLVE_MASK_PLAN(clock, row_end);
}

static uint64_t DissolveMaskPhaseRoom(uint32_t pc) {
    return pc == 0x85e8e8u ? 1024u : pc == 0x85e7c5u ? 192u : pc == 0x85e7d0u ? 160u :
        pc == 0x85e7d8u ? 400u : 288u;
}

static bool PrepareDissolveMaskClock(DissolveMaskClock *clock, CpuState *cpu,
    uint32_t entry, bool parent) {
    const bool counts = entry == 0x85e8e8u;
    const uint64_t required = DissolveMaskPhaseRoom(entry);
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->D || cpu->_flag_D || (!parent && cpu->DB != 0x7eu) ||
        cpu->S < (parent ? 0x1f10u : counts ? 0x1f14u : 0x1f00u) ||
        (parent ? cpu->host_return_valid != 3u : counts ? cpu->host_return_valid != 2u :
            (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
             cpu->host_return_valid != 3u)) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffbu,
            false, 1363u, required) || g_snes->vPos >= 262u ||
        (parent && !Lufia2DecompFrameRangeQuiet(0x85e7c5u, 0x85e7d0u)) ||
        (counts && !Lufia2DecompFrameRangeQuiet(0x85e8e8u, 0x85e91fu)))
        return false;
    const unsigned budget = counts ? 0u : Lufia2DecompFrameOwnedSpanBudget(
        0x85e7d0u, 0x85e806u, ActorBridgeDissolveMask);
    if (!counts && !budget)
        return false;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u &&
        g_snes->hPos < 1024u ? 1024u : 1364u;
    uint64_t room = fence - g_snes->hPos - 1u;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (room > deadline - cpu->master_cycles)
        room = deadline - cpu->master_cycles;
    if (room < required || room > UINT64_MAX - cpu->master_cycles)
        return false;
    *clock = (DissolveMaskClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x850000u;
    clock->budget = budget;
    clock->reserved_until = cpu->master_cycles + room;
    return true;
}

static uint8_t DissolveMaskBudget(void *context,
    const Lufia2CpuState *state, uint32_t pc) {
    DissolveMaskClock *clock = context;
    FinishDissolveMaskPhase(clock);
    const unsigned count = pc == 0x85e7c5u ? 6u :
        pc == 0x85e7d0u ? 4u : pc == 0x85e7d8u ? 12u : 11u;
    const uint64_t required = DissolveMaskPhaseRoom(pc);
    if (pc == 0x85e806u || clock->opcodes > clock->budget ||
        count > clock->budget - clock->opcodes ||
        clock->span.cpu->master_cycles > clock->reserved_until ||
        required > clock->reserved_until - clock->span.cpu->master_cycles)
        return 0u;
    clock->state = state;
    if (pc == 0x85e7c5u) DISSOLVE_MASK_PLAN(clock, begin);
    else if (pc == 0x85e7d0u) DISSOLVE_MASK_PLAN(clock, tile);
    else if (pc == 0x85e7d8u) DISSOLVE_MASK_PLAN(clock, row);
    else {
        DISSOLVE_MASK_PLAN(clock, advance);
    }
    return 1u;
}

static uint8_t DissolveMaskChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    (void)context; (void)site; (void)frame_size;
    state->program_bank = (uint8_t)(target >> 16);
    state->resume_pc = target;
    return 0u;
}

static void ActorBridgeDissolveMask(CpuState *cpu, uint32_t pc) {
    if (pc != 0x85e7d0u && pc != 0x85e7d8u && pc != 0x85e7f2u)
        return;
    DissolveMaskClock clock;
    if (!PrepareDissolveMaskClock(&clock, cpu, pc, false))
        return;
    const Lufia2Memory memory =
        {DissolveMaskRead, DissolveMaskWrite, &clock, NULL, NULL};
    Lufia2CpuState state = {0};
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleContinueDissolveMask(
        &memory, &state, pc, DissolveMaskBudget, &clock);
    FinishDissolveMaskPhase(&clock);
    if (!clock.opcodes)
        return;
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnCompleteSpan(clock.opcodes, clock.last_write);
    interp_bridge_pre_opcode_redirect(result.pc);
}

static RecompReturn ActorBridgeDissolveSetup(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    DissolveMaskClock clock;
    if (!PrepareDissolveMaskClock(&clock, cpu, 0x85e7c5u, true))
        return ActorBridgeFallback(cpu, &frame, 0x85e7c5u);
    const Lufia2Memory memory =
        {DissolveMaskRead, DissolveMaskWrite, &clock, NULL, NULL};
    Lufia2CpuState state = {0};
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleRunDissolveEffectWithBudget(
        &memory, &state, DissolveMaskChild, &clock, DissolveMaskBudget, &clock);
    FinishDissolveMaskPhase(&clock);
    ActorBridgeStore(cpu, &state);
    return interp_tier_dispatch_tail(cpu, result.pc, result.pc,
        frame.entry_s, frame.hrv);
}

static uint8_t DissolveCountsBudget(void *context,
    const Lufia2CpuState *state, uint32_t pc) {
    DissolveMaskClock *clock = context;
    FinishDissolveMaskPhase(clock);
    if (pc != 0x85e8e8u)
        return 0u;
    clock->state = state;
    clock->span.timing = dissolve_counts_prefix;
    clock->span.count = sizeof dissolve_counts_prefix / sizeof dissolve_counts_prefix[0];
    return 1u;
}

static RecompReturn ActorBridgeDissolveTransition(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    DissolveMaskClock clock;
    if (!cpu->ram || cpu->ram[0x19u] != 0u || cpu->ram[0x1au] != 0xa0u ||
        cpu->ram[0x1fu] != 0u || cpu->ram[0x20u] != 0xdfu ||
        !PrepareDissolveMaskClock(&clock, cpu, 0x85e8e8u, false))
        return ActorBridgeFallback(cpu, &frame, 0x85e8e8u);
    const Lufia2Memory memory =
        {DissolveMaskRead, DissolveMaskWrite, &clock, NULL, NULL};
    Lufia2CpuState state = {0};
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2BattleRunDissolveTransitionWithBudget(&memory, &state,
            DissolveMaskChild, &clock, DissolveCountsBudget, &clock);
    FinishDissolveMaskPhase(&clock);
    ActorBridgeStore(cpu, &state);
    return interp_tier_dispatch_tail(cpu, result.pc, result.pc,
        frame.entry_s, frame.hrv);
}

#undef DISSOLVE_MASK_PLAN
