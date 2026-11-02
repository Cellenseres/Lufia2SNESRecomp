#include "staged_gold_plans.inc"

static bool StagedGoldClockSupported(const CpuState *cpu, uint32_t entry) {
    const unsigned budget = 2048u;
    if (cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->host_return_valid != 3u || cpu->S < 0x1f20u ||
        !(cpu->DB == 0x7eu || cpu->DB == 0x7fu || (cpu->DB & 0x7fu) < 0x40u) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu, false, 1363u, budget) ||
        !Lufia2DecompFrameRangeQuiet(entry, entry == 0x81f577u ? 0x81f59bu : 0x81f5bdu))
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


static RecompReturn ActorBridgeStagedGold(CpuState *cpu, bool add) {
    const uint32_t entry = add ? 0x81f577u : 0x81f59bu;
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    if (!StagedGoldClockSupported(cpu, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const uint8_t *ram = cpu->ram + (cpu->DB == 0x7fu ? 0x10000u : 0u);
    const uint32_t gold = ram[0x0a8a] | (uint32_t)ram[0x0a8b] << 8 |
        (uint32_t)ram[0x0a8c] << 16;
    const uint32_t amount = ram[0x09f2] | (uint32_t)ram[0x09f3] << 8 |
        (uint32_t)ram[0x09f4] << 16;
    const bool clamp = add ? gold + amount > 0xffffffu : gold < amount;
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x810000u;
#define STAGED_GOLD_PLAN(name) do { \
    clock.timing = staged_gold_##name; \
    clock.count = sizeof staged_gold_##name / sizeof staged_gold_##name[0]; \
} while (0)
    if (add) {
        if (clamp) STAGED_GOLD_PLAN(add_clamp);
        else STAGED_GOLD_PLAN(add);
    } else {
        if (clamp) STAGED_GOLD_PLAN(subtract_clamp);
        else STAGED_GOLD_PLAN(subtract);
    }
#undef STAGED_GOLD_PLAN
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = add
        ? Lufia2GoldAddStagedAmount(&memory, &state)
        : Lufia2GoldSubtractStagedAmount(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedLongCall(cpu, &frame, result.pc);
}
