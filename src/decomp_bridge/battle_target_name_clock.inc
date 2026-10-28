#include "lufia2/battle_target_name_steps.h"

static unsigned BattleTargetNameSpanSize(const CpuState *cpu, uint32_t entry) {
    if ((entry != 0x81d58du && entry != 0x81d7c5u && entry != 0x81d8ccu) ||
        cpu->DB != 0x7eu || cpu->D || cpu->_flag_D ||
        cpu->m_flag != 1u || cpu->x_flag || cpu->Y > 127u ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->master_cycles != cpu->coprocessor_master_cycles ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu, false, 1363u, 1u) ||
        g_snes->vPos >= 262u ||
        !Lufia2DecompFrameRangeQuiet(entry + 1u, entry + 6u))
        return 0u;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || (current && current != cpu->master_cycles))
        return 0u;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u &&
        g_snes->hPos < 1024u ? 1024u : 1364u;
    const unsigned iteration = 6u * cpu_region_speed(entry) + 32u;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    unsigned bytes = cpu->Y < 8u ? cpu->Y + 1u : 8u;
    while (bytes) {
        const unsigned budget = bytes * iteration + 40u -
            (bytes == cpu->Y + 1u ? 6u : 0u);
        if (budget < fence - g_snes->hPos &&
            budget <= UINT64_MAX - cpu->master_cycles &&
            deadline - cpu->master_cycles >= budget)
            return bytes;
        --bytes;
    }
    return 0u;
}

static void ActorBridgeBattleTargetName(CpuState *cpu, uint32_t entry) {
    const unsigned bytes = BattleTargetNameSpanSize(cpu, entry);
    if (!bytes)
        return;
    FixedSpanOpcode timing[24];
    for (unsigned index = 0u; index < bytes; ++index) {
        const uint16_t pc = (uint16_t)entry;
        timing[3u * index] = (FixedSpanOpcode){pc, 3u, 6u, 1u};
        timing[3u * index + 1u] = (FixedSpanOpcode){(uint16_t)(pc + 3u), 1u, 2u, 0u};
        timing[3u * index + 2u] = (FixedSpanOpcode){(uint16_t)(pc + 4u), 2u,
            (uint8_t)(cpu->Y == index ? 2u : 3u), 0u};
    }
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x810000u;
    clock.timing = timing;
    clock.count = 3u * bytes;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2BattleClearTargetNameSpan(&memory, &state, entry, bytes);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
