#include "lufia2/battle_swirl_steps.h"

static unsigned SwirlPartBudget(uint32_t entry) {
    return entry == 0x848cddu ? 144u : entry == 0x848ce7u ? 208u : 272u;
}

static bool SwirlPartClockSupported(const CpuState *cpu, uint32_t entry) {
    const bool prepare = entry == 0x848cddu;
    const bool first_half = cpu->Y < 0xe0u && !(cpu->Y & 1u);
    const bool second_half = cpu->Y >= 0xe1u && cpu->Y < 0x1c1u && (cpu->Y & 1u);
    if ((!prepare && entry != 0x848ce7u && entry != 0x848cf3u) ||
        cpu->PB != 0x84u || cpu->m_flag != (unsigned)prepare || cpu->x_flag ||
        cpu->DB != 0x7eu || cpu->D || cpu->_flag_D || cpu->X > 0xffu ||
        (!first_half && !second_half) ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        !Lufia2DecompFrameReturnCpuReady() || !g_snes || g_snes->vPos >= 262u ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - SwirlPartBudget(entry))
        return false;
    const unsigned budget = SwirlPartBudget(entry);
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    if (!ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu,
            !prepare, fence - budget - 1u, budget))
        return false;
    uint64_t phase, clock;
    snes_refresh_state_get(&phase, &clock);
    return phase < 1364u && (!clock || clock == cpu->master_cycles);
}

static void ActorBridgeSwirlPartClock(CpuState *cpu, uint32_t entry) {
    if (!SwirlPartClockSupported(cpu, entry))
        return;
    static const FixedSpanOpcode prepare[] = {
        {0x8cddu, 4u, 5u, 1u}, {0x8ce1u, 4u, 5u, 1u},
        {0x8ce5u, 2u, 3u, 0u}
    };
    static const FixedSpanOpcode store[] = {
        {0x8ce7u, 4u, 6u, 2u}, {0x8cebu, 1u, 2u, 0u},
        {0x8cecu, 4u, 6u, 2u}, {0x8cf0u, 3u, 7u, 2u}
    };
    FixedSpanOpcode advance[12] = {
        {0x8cf3u, 1u, 2u, 0u}, {0x8cf4u, 1u, 2u, 0u},
        {0x8cf5u, 3u, 3u, 0u}, {0x8cf8u, 1u, 2u, 0u},
        {0x8cf9u, 2u, 3u, 0u}, {0x8cfbu, 1u, 2u, 0u},
        {0x8cfcu, 1u, 2u, 0u}, {0x8cfdu, 3u, 3u, 0u},
        {0x8d00u, 2u, 3u, 0u}
    };
    unsigned count = 9u;
    if (cpu->Y == 0xdeu) {
        advance[8].cycles = 2u;
        advance[count++] = (FixedSpanOpcode){0x8d02u, 1u, 2u, 0u};
    }
    advance[count++] = (FixedSpanOpcode){0x8d03u, 3u, 3u, 0u};
    advance[count++] = (FixedSpanOpcode){0x8d06u, 2u,
        (uint8_t)(cpu->Y == 0x1bfu ? 2u : 3u), 0u};
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x840000u;
    clock.timing = entry == 0x848cddu ? prepare
        : entry == 0x848ce7u ? store : advance;
    clock.count = entry == 0x848cddu ? 3u : entry == 0x848ce7u ? 4u : count;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = entry == 0x848cddu
        ? Lufia2BattleSwirlHdmaPrepareStep(&memory, &state)
        : entry == 0x848ce7u ? Lufia2BattleSwirlHdmaStoreStep(&memory, &state)
        : Lufia2BattleSwirlHdmaAdvanceStep(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}

static void ActorBridgeSwirlHdmaDispatch(CpuState *cpu, uint32_t entry) {
    if (SwirlHdmaClockSupported(cpu, entry))
        ActorBridgeSwirlHdmaRow(cpu, entry);
    else
        ActorBridgeSwirlPartClock(cpu, entry);
}
