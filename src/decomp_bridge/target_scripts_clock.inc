#include "target_scripts_plans.inc"

typedef struct TargetScriptsClock {
    FixedSpanClock span;
    ActorBridgeFrame frame;
    RecompReturn unwound;
    unsigned searched;
    bool finding_slot;
} TargetScriptsClock;

static bool TargetScriptsWindow(const CpuState *cpu, uint32_t entry) {
    const bool listed = entry != 0x819d75u;
    const unsigned budget = listed ? 16384u : 2048u;
    if (!cpu->ram || cpu->D || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->host_return_valid != 2u || cpu->S < 0x1f30u ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu, false, 1363u, budget))
        return false;
    if (listed ?
        !Lufia2DecompFrameRangeQuiet(0x819fabu, 0x81a03au) ||
        !Lufia2DecompFrameRangeQuiet(0x818fabu, 0x818fc5u) :
        !Lufia2DecompFrameRangeQuiet(0x819d75u, 0x819dc9u))
        return false;
    const unsigned line = g_snes->vPos;
    if (line < 225u) {
        if (!g_snes->hdmaBeamOff || (225u - line) * 1364u - g_snes->hPos <= budget)
            return false;
    } else if (line >= 262u || (262u - line) * 1364u - g_snes->hPos <= budget) {
        return false;
    }
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static void TargetScriptsPlan(TargetScriptsClock *clock,
    const FixedSpanOpcode *timing, unsigned count) {
    assert(!clock->span.remaining);
    clock->span.timing = timing;
    clock->span.count = count;
    clock->span.index = 0u;
}

#define TARGET_SCRIPTS_PLAN(clock, name) \
    TargetScriptsPlan(clock, name, sizeof name / sizeof name[0])

static uint8_t TargetScriptsRead(void *context, uint32_t address) {
    TargetScriptsClock *clock = context;
    const uint8_t value = FixedSpanRead(&clock->span, address);
    if (clock->finding_slot && clock->span.index == clock->span.count &&
        (clock->span.timing == target_scripts_slot_first ||
         clock->span.timing == target_scripts_slot_next)) {
        if (!value)
            TARGET_SCRIPTS_PLAN(clock, target_scripts_slot_found);
        else if (++clock->searched < 30u)
            TARGET_SCRIPTS_PLAN(clock, target_scripts_slot_next);
        else
            TARGET_SCRIPTS_PLAN(clock, target_scripts_slot_exhausted);
    }
    return value;
}

static void TargetScriptsWrite(void *context, uint32_t address, uint8_t value) {
    TargetScriptsClock *clock = context;
    FixedSpanWrite(&clock->span, address, value);
}

static uint8_t TargetScriptsHandoff(TargetScriptsClock *clock,
    Lufia2CpuState *state, uint32_t pc) {
    ActorBridgeStore(clock->span.cpu, state);
    clock->unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->span.cpu, pc, pc, clock->frame.entry_s, clock->frame.hrv) + 1);
    return 0u;
}

static uint8_t TargetScriptsFindSlot(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    TargetScriptsClock *clock = context;
    assert(target == 0x818fabu && site == 0x819fddu && frame_size == 2u);
    (void)target; (void)site; (void)frame_size;
    BeginFixedSpanOpcode(&clock->span);
    assert(clock->span.index == clock->span.count);
    clock->finding_slot = true;
    clock->searched = 0u;
    TARGET_SCRIPTS_PLAN(clock, target_scripts_slot_first);
    const Lufia2Memory memory = {TargetScriptsRead, TargetScriptsWrite, clock, NULL, NULL};
    const Lufia2ExecutionResult result = Lufia2BattleEffectFindScriptSlot(&memory, state);
    if (result.flow != LUFIA2_EXECUTION_RETURNED) {
        BeginFixedSpanOpcode(&clock->span);
        return TargetScriptsHandoff(clock, state, result.pc);
    }
    assert(result.pc == 0x818fc4u);
    const uint16_t low = TargetScriptsRead(clock, ++state->stack);
    const uint16_t high = TargetScriptsRead(clock, ++state->stack);
    assert((high << 8 | low) == 0x9fdfu);
    (void)low; (void)high;
    BeginFixedSpanOpcode(&clock->span);
    clock->finding_slot = false;
    if (!TargetScriptsWindow(clock->span.cpu, 0x819fe0u))
        return TargetScriptsHandoff(clock, state, 0x819fe0u);
    const uint16_t remaining = (uint16_t)(clock->span.cpu->ram[2] |
        (uint16_t)clock->span.cpu->ram[3] << 8);
    if (remaining == 1u)
        TARGET_SCRIPTS_PLAN(clock, target_scripts_listed_last);
    else
        TARGET_SCRIPTS_PLAN(clock, target_scripts_listed_more);
    return 1u;
}

static RecompReturn ActorBridgeTargetScripts(CpuState *cpu, bool listed) {
    const uint32_t entry = listed ? 0x819fabu : 0x819d75u;
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    if (!TargetScriptsWindow(cpu, entry) || cpu->Y < 0x3000u ||
        (cpu->Y & 0xffu) > (listed ? 0xe6u : 0xdau) ||
        (listed && cpu->ram[0x4f0au] > 8u))
        return ActorBridgeFallback(cpu, &frame, entry);
    TargetScriptsClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = 0x810000u;
    clock.frame = frame;
    if (listed) {
        if (cpu->ram[0x4f0au])
            TARGET_SCRIPTS_PLAN(&clock, target_scripts_listed_start);
        else
            TARGET_SCRIPTS_PLAN(&clock, target_scripts_listed_empty);
    } else {
        const uint8_t target = cpu->ram[cpu->Y + 0x25u];
        if (target & 0x80u)
            TARGET_SCRIPTS_PLAN(&clock, target_scripts_display_enemy);
        else if (target == 4u)
            TARGET_SCRIPTS_PLAN(&clock, target_scripts_display_special);
        else
            TARGET_SCRIPTS_PLAN(&clock, target_scripts_display_other);
    }
    const Lufia2Memory memory = {TargetScriptsRead, TargetScriptsWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = listed ?
        Lufia2BattleEffectSpawnListedTargetScripts(&memory, &state, TargetScriptsFindSlot, &clock) :
        Lufia2BattleEffectSetSpecialTargetDisplayOffsets(&memory, &state);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.unwound - 1);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}

#undef TARGET_SCRIPTS_PLAN
