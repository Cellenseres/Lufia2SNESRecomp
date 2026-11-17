#include "lufia2/battle_effect_slot_steps.h"

static unsigned EffectClearSpanSize(CpuState *cpu, uint32_t entry) {
    if (cpu != &g_cpu || entry != 0x818bfbu || cpu->PB != 0x81u || cpu->m_flag != 1u ||
        cpu->x_flag || cpu->DB != 0x97u || cpu->D || cpu->_flag_D || cpu->emulation || !cpu->X ||
        cpu->X > 0x800u ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        !Lufia2DecompFrameReturnCpuReady() || !g_snes || g_snes->vPos >= 262u ||
        g_snes->ramAdr != 0x3800u - cpu->X ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu, false, 1363u, 1u) ||
        !Lufia2DecompFrameRangeAllows(entry, entry + 6u, Lufia2DecompEffectClearHookOwned))
        return 0u;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || (current && current != cpu->master_cycles))
        return 0u;
    const unsigned opcodes =
        Lufia2DecompFrameSpanBudget(entry, entry + 6u, ActorBridgeEffectClearBytes);
    unsigned limit = opcodes / 3u;
    if (opcodes < 3u)
        return 0u;
    if (limit > LUFIA2_BATTLE_EFFECT_CLEAR_SPAN_MAX_BYTES)
        limit = LUFIA2_BATTLE_EFFECT_CLEAR_SPAN_MAX_BYTES;
    if (limit > cpu->X)
        limit = cpu->X;
    const unsigned fence =
        !g_snes->hdmaBeamOff && g_snes->vPos < 225u && g_snes->hPos < 1024u ? 1024u : 1364u;
    uint64_t room = fence - g_snes->hPos - 1u;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (deadline) {
        if (deadline <= cpu->master_cycles)
            return 0u;
        if (room > deadline - cpu->master_cycles)
            room = deadline - cpu->master_cycles;
    }
    if (room > UINT64_MAX - cpu->master_cycles)
        room = UINT64_MAX - cpu->master_cycles;
    const unsigned iteration = 6u * cpu_region_speed(entry) + 20u;
    unsigned bytes = room > 40u ? (unsigned)((room - 40u) / iteration) : 0u;
    if (bytes > limit)
        bytes = limit;
    if (limit == cpu->X && bytes < limit && limit * iteration + 34u <= room)
        bytes = limit;
#if defined(LUFIA2_HAVE_NATIVE_EFFECT_CLEAR_WINDOW)
    uint64_t window_room = 1363u;
    if (deadline && window_room > deadline - cpu->master_cycles)
        window_room = deadline - cpu->master_cycles;
    if (window_room > UINT64_MAX - cpu->master_cycles)
        window_room = UINT64_MAX - cpu->master_cycles;
    unsigned window_bytes = window_room > 40u ? (unsigned)((window_room - 40u) / iteration) : 0u;
    if (window_bytes > limit)
        window_bytes = limit;
    if (window_bytes > bytes && NativeOpcodeWindowSupported(cpu, entry,
            window_bytes * iteration + 40u, 0x1fffu, false))
        return window_bytes;
#endif
    return bytes;
}

static void ActorBridgeEffectClearBytes(CpuState *cpu, uint32_t entry) {
    const unsigned bytes = EffectClearSpanSize(cpu, entry);
    if (!bytes)
        return;
    unsigned retired = 0u;
    uint32_t next = entry;
    for (unsigned cleared = 0u; cleared < bytes;) {
        const unsigned chunk = bytes - cleared < 4u ? bytes - cleared : 4u;
        FixedSpanOpcode timing[12];
        for (unsigned index = 0u; index < chunk; ++index) {
            timing[3u * index] = (FixedSpanOpcode){0x8bfbu, 3u, 4u, 1u};
            timing[3u * index + 1u] = (FixedSpanOpcode){0x8bfeu, 1u, 2u, 0u};
            timing[3u * index + 2u] =
                (FixedSpanOpcode){0x8bffu, 2u, (uint8_t)(cpu->X == index + 1u ? 2u : 3u), 0u};
        }
        Lufia2DecompRomOpcodeSpan fetch;
        const bool owned_fetch = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, 6u, &fetch);
        EffectClearClock effect = {0};
        effect.code_speed = cpu_region_speed(entry);
        FixedSpanClock *clock = &effect.span;
        clock->opcode_span = owned_fetch ? &fetch : NULL;
        const unsigned budget =
            chunk * (6u * effect.code_speed + cpu_region_speed(0x972180u) + 12u) -
            (cpu->X == chunk ? 6u : 0u);
        if (owned_fetch)
            Lufia2DecompPrepareClockSpan(cpu, budget, &effect.clocks);
        clock->cpu = cpu;
        clock->bank = 0x810000u;
        clock->timing = timing;
        clock->count = 3u * chunk;
        const Lufia2Memory memory = {EffectClearRead, EffectClearWrite, &effect, NULL, NULL};
        Lufia2CpuState state;
        ActorBridgeLoad(cpu, &state);
        const Lufia2ExecutionResult result =
            Lufia2BattleEffectClearByteSpan(&memory, &state, chunk);
        BeginEffectClearOpcode(&effect);
        assert(clock->index == clock->count && !clock->remaining);
        assert(result.pc == 0x818bfbu || result.pc == 0x818c01u);
        ActorBridgeStore(cpu, &state);
        retired += clock->count;
        cleared += chunk;
        next = result.pc;
    }
    /* Every original port write advances the bridge epoch. */
    Lufia2DecompFrameReturnCompleteSpan(retired, retired - 2u);
    interp_bridge_pre_opcode_redirect(next);
}
