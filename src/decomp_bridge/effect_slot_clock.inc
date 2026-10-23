#include "lufia2/battle_effect_slot_steps.h"

enum {
    EFFECT_SLOT_ADVANCE_BUDGET = 218u,
    EFFECT_SLOT_IDLE_BUDGET = 278u,
    EFFECT_SLOT_EXPIRED_BUDGET = 206u,
    EFFECT_SLOT_RUNNING_BUDGET = 390u,
    EFFECT_SLOT_COUNTER_ADDRESS = 0x15ebu
};

static unsigned EffectSlotClockBudget(const CpuState *cpu, bool advance) {
    if (advance)
        return EFFECT_SLOT_ADVANCE_BUDGET;
    if (!cpu->ram[cpu->Y])
        return EFFECT_SLOT_IDLE_BUDGET;
    return cpu->ram[cpu->Y + 1u] == 1u
        ? EFFECT_SLOT_EXPIRED_BUDGET : EFFECT_SLOT_RUNNING_BUDGET;
}

static bool EffectSlotClockSupported(const CpuState *cpu, uint32_t entry,
                                     bool actors, bool advance) {
    const uint32_t expected = actors
        ? (advance ? 0x818b1bu : 0x818ac1u)
        : (advance ? 0x818aaau : 0x818a7eu);
    if (entry != expected || cpu->m_flag != 1u || cpu->x_flag ||
        cpu->DB != 0x7eu || cpu->D || cpu->_flag_D || !cpu->ram ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !g_snes || g_snes->ram != cpu->ram)
        return false;
    const unsigned base = actors ? 0x54b7u : 0x4f8fu;
    const unsigned stride = actors ? 0x2du : 0x2cu;
    const unsigned slots = actors ? 64u : 30u;
    if (cpu->Y < base || (cpu->Y - base) % stride ||
        (cpu->Y - base) / stride >= slots)
        return false;
    const unsigned budget = EffectSlotClockBudget(cpu, advance);
    if (cpu->master_cycles > UINT64_MAX - budget)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    if (!ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu, false,
            fence - budget - 1u, budget))
        return false;
    uint64_t phase, clock;
    snes_refresh_state_get(&phase, &clock);
    return phase < 1364u && (!clock || clock == cpu->master_cycles);
}

static unsigned EffectSlotAdvanceTiming(
    FixedSpanOpcode *timing, unsigned count, bool actors, bool repeats) {
    const uint16_t base = actors ? 0x8b1bu : 0x8aaau;
    timing[count++] = (FixedSpanOpcode){base, 2u, 3u, 0u};
    timing[count++] = (FixedSpanOpcode){(uint16_t)(base + 2u), 1u, 2u, 0u};
    timing[count++] = (FixedSpanOpcode){(uint16_t)(base + 3u), 1u, 2u, 0u};
    timing[count++] = (FixedSpanOpcode){(uint16_t)(base + 4u), 3u, 3u, 0u};
    timing[count++] = (FixedSpanOpcode){(uint16_t)(base + 7u), 1u, 2u, 0u};
    timing[count++] = (FixedSpanOpcode){(uint16_t)(base + 8u), 2u, 3u, 0u};
    timing[count++] = (FixedSpanOpcode){(uint16_t)(base + 10u), 3u, 6u, 2u};
    timing[count++] = (FixedSpanOpcode){(uint16_t)(base + 13u), 2u,
        (uint8_t)(repeats ? 3u : 2u), 0u};
    return count;
}

static void ActorBridgeEffectSlotStep(CpuState *cpu, uint32_t entry) {
    const bool actors = entry == 0x818ac1u || entry == 0x818b1bu;
    const bool advance = entry == 0x818aaau || entry == 0x818b1bu;
    if (!EffectSlotClockSupported(cpu, entry, actors, advance))
        return;
    FixedSpanOpcode timing[14];
    unsigned count = 0u;
    bool expired = false;
    if (!advance) {
        const uint16_t base = actors ? 0x8ac1u : 0x8a7eu;
        const bool active = cpu->ram[cpu->Y] != 0u;
        timing[count++] = (FixedSpanOpcode){base, 3u, 5u, 1u};
        timing[count++] = (FixedSpanOpcode){(uint16_t)(base + 3u), 2u,
            (uint8_t)(active ? 2u : 3u), 0u};
        if (active) {
            expired = cpu->ram[cpu->Y + 1u] == 1u;
            timing[count++] = (FixedSpanOpcode){(uint16_t)(base + 5u), 3u, 5u, 1u};
            timing[count++] = (FixedSpanOpcode){(uint16_t)(base + 8u), 1u, 2u, 0u};
            timing[count++] = (FixedSpanOpcode){(uint16_t)(base + 9u), 3u, 6u, 1u};
            timing[count++] = (FixedSpanOpcode){(uint16_t)(base + 12u), 2u,
                (uint8_t)(expired ? 2u : 3u), 0u};
        }
    }
    if (!expired) {
        const bool repeats = cpu->ram[EFFECT_SLOT_COUNTER_ADDRESS] != 1u;
        count = EffectSlotAdvanceTiming(timing, count, actors, repeats);
    }
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x810000u;
    clock.timing = timing;
    clock.count = count;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result;
    if (actors)
        result = advance ? Lufia2BattleEffectActorAdvanceStep(&memory, &state)
                         : Lufia2BattleEffectActorSlotStep(&memory, &state);
    else
        result = advance ? Lufia2BattleEffectScriptAdvanceStep(&memory, &state)
                         : Lufia2BattleEffectScriptSlotStep(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
