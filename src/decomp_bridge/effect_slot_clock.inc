#include "lufia2/battle_effect_slot_steps.h"

enum { EFFECT_SLOT_COUNTER_ADDRESS = 0x15ebu };

static void ActorBridgeEffectSlotStep(CpuState *cpu, uint32_t entry);

static bool EffectSlotHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return hook == ActorBridgeEffectSlotStep &&
        (pc == 0x818a7eu || pc == 0x818ac1u ||
         pc == 0x818aaau || pc == 0x818b1bu
#if defined(LUFIA2_BATTLE_EFFECT_SLOT_COUNTER_API)
         || pc == 0x818ab4u || pc == 0x818b25u
#endif
        );
}

static unsigned EffectSlotHeaderBudget(const CpuState *cpu, unsigned fetch) {
    if (!cpu->ram[cpu->Y])
        return 5u * fetch + 20u;
    return 14u * fetch + (cpu->ram[cpu->Y + 1u] == 1u ? 54u : 60u);
}

static unsigned EffectSlotClockBudget(const CpuState *cpu, uint32_t entry,
                                     bool advance) {
    const unsigned fetch = cpu_region_speed(entry);
    unsigned budget = advance ? 0u : EffectSlotHeaderBudget(cpu, fetch);
    if (!advance && cpu->ram[cpu->Y] && cpu->ram[cpu->Y + 1u] == 1u)
        return budget;
    budget += 15u * fetch + 58u;
    if (cpu->ram[EFFECT_SLOT_COUNTER_ADDRESS] == 1u)
        budget -= 6u;
    return budget;
}

static bool EffectSlotClockStateSupported(const CpuState *cpu) {
    if (cpu->m_flag != 1u || cpu->x_flag ||
        cpu->DB != 0x7eu || cpu->D || cpu->_flag_D || !cpu->ram ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !g_snes || g_snes->ram != cpu->ram)
        return false;
    return true;
}

static bool EffectSlotGridSupported(unsigned y, bool actors) {
    const unsigned base = actors ? 0x54b7u : 0x4f8fu;
    const unsigned stride = actors ? 0x2du : 0x2cu;
    const unsigned slots = actors ? 64u : 30u;
    if (y < base || (y - base) % stride || (y - base) / stride >= slots)
        return false;
    return true;
}

static bool EffectSlotClockContextSupported(const CpuState *cpu, uint32_t entry,
                                     bool actors, bool advance) {
    const uint32_t expected = actors
        ? (advance ? 0x818b1bu : 0x818ac1u)
        : (advance ? 0x818aaau : 0x818a7eu);
    return entry == expected && EffectSlotClockStateSupported(cpu) &&
        EffectSlotGridSupported(cpu->Y, actors);
}

static bool EffectSlotClockWindowSupported(const CpuState *cpu,
    uint32_t entry, unsigned budget) {
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return false;
    if (!charged || phase + budget >= 1364u)
        budget += 40u;
    if (cpu->master_cycles > UINT64_MAX - budget)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u &&
        g_snes->hPos < 1024u ? 1024u : 1364u;
    return ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu, false,
        fence - budget - 1u, budget);
}

static bool EffectSlotClockSupported(const CpuState *cpu, uint32_t entry,
                                     bool actors, bool advance) {
    if (!EffectSlotClockContextSupported(cpu, entry, actors, advance) ||
        !EffectSlotClockWindowSupported(cpu, entry,
            EffectSlotClockBudget(cpu, entry, advance)))
        return false;
    if (!Lufia2DecompFrameRangeAllows(entry, entry + (advance ? 15u : 14u),
                                     EffectSlotHookOwned))
        return false;
    if (!advance && (!cpu->ram[cpu->Y] || cpu->ram[cpu->Y + 1u] != 1u)) {
        const uint32_t cursor = actors ? 0x818b1bu : 0x818aaau;
        if (!Lufia2DecompFrameRangeAllows(cursor, cursor + 15u, EffectSlotHookOwned))
            return false;
    }
    return true;
}

#if defined(LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_SLOT_CURSORS)
static bool EffectSlotCursorClockSupported(const CpuState *cpu, uint32_t entry,
                                           bool actors) {
    return EffectSlotClockContextSupported(cpu, entry, actors, true) &&
        EffectSlotClockWindowSupported(cpu, entry,
            10u * cpu_region_speed(entry) + 30u) &&
        Lufia2DecompFrameRangeAllows(entry, entry + 10u, EffectSlotHookOwned);
}
#endif

#if defined(LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_SLOT_HEADERS)
static bool EffectSlotHeaderClockSupported(const CpuState *cpu, uint32_t entry,
                                           bool actors) {
    return EffectSlotClockContextSupported(cpu, entry, actors, false) &&
        EffectSlotClockWindowSupported(cpu, entry,
            EffectSlotHeaderBudget(cpu, cpu_region_speed(entry))) &&
        Lufia2DecompFrameRangeAllows(entry, entry + 14u, EffectSlotHookOwned);
}
#endif

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

#if defined(LUFIA2_BATTLE_EFFECT_SLOT_COUNTER_API)
static void EffectSlotCounterStep(CpuState *cpu, uint32_t entry) {
    const bool actors = entry == 0x818b25u;
    const unsigned stride = actors ? 0x2du : 0x2cu;
    if (!EffectSlotClockStateSupported(cpu) || cpu->Y < stride ||
        !EffectSlotGridSupported(cpu->Y - stride, actors))
        return;
    const bool repeats = cpu->ram[EFFECT_SLOT_COUNTER_ADDRESS] != 1u;
    const unsigned budget = 5u * cpu_region_speed(entry) + (repeats ? 28u : 22u);
    if (!EffectSlotClockWindowSupported(cpu, entry, budget) ||
        !Lufia2DecompFrameRangeAllows(entry, entry + 5u, EffectSlotHookOwned) ||
        Lufia2DecompFrameSpanBudget(entry, entry + 5u, ActorBridgeEffectSlotStep) < 2u)
        return;
    static const FixedSpanOpcode timing[2][2][2] = {
        {{{0x8ab4u, 3u, 6u, 2u}, {0x8ab7u, 2u, 2u, 0u}},
         {{0x8ab4u, 3u, 6u, 2u}, {0x8ab7u, 2u, 3u, 0u}}},
        {{{0x8b25u, 3u, 6u, 2u}, {0x8b28u, 2u, 2u, 0u}},
         {{0x8b25u, 3u, 6u, 2u}, {0x8b28u, 2u, 3u, 0u}}}
    };
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x810000u;
    clock.timing = timing[actors][repeats];
    clock.count = 2u;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state = {0};
    state.program_bank = 0x81u;
    state.data_bank = cpu->DB;
    state.accumulator_is_8_bit = 1u;
    const Lufia2ExecutionResult result =
        Lufia2BattleEffectSlotCounterStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    cpu->_flag_N = state.negative;
    cpu->_flag_Z = state.zero;
    cpu_mirrors_to_p(cpu);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
#endif

static void ActorBridgeEffectSlotStep(CpuState *cpu, uint32_t entry) {
#if defined(LUFIA2_BATTLE_EFFECT_SLOT_COUNTER_API)
    if (entry == 0x818ab4u || entry == 0x818b25u) {
        EffectSlotCounterStep(cpu, entry);
        return;
    }
#endif
    const bool actors = entry == 0x818ac1u || entry == 0x818b1bu;
    const bool advance = entry == 0x818aaau || entry == 0x818b1bu;
    bool header_only = false;
    bool cursor_only = false;
    if (!EffectSlotClockSupported(cpu, entry, actors, advance)) {
        if (advance) {
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_SLOT_CURSORS)
            if (!EffectSlotCursorClockSupported(cpu, entry, actors))
                return;
            cursor_only = true;
#else
            return;
#endif
        } else {
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_SLOT_HEADERS)
            if (!EffectSlotHeaderClockSupported(cpu, entry, actors))
                return;
            header_only = true;
#else
            return;
#endif
        }
    }
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
    if (!expired && !header_only) {
        const bool repeats = cpu->ram[EFFECT_SLOT_COUNTER_ADDRESS] != 1u;
        count = EffectSlotAdvanceTiming(timing, count, actors, repeats);
        if (cursor_only)
            count -= 2u;
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
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_SLOT_CURSORS)
    if (cursor_only)
        result = Lufia2BattleEffectAdvanceCursorStep(&memory, &state, entry);
    else
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_SLOT_HEADERS)
    if (header_only)
        result = Lufia2BattleEffectSlotHeaderStep(&memory, &state, entry);
    else
#endif
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
