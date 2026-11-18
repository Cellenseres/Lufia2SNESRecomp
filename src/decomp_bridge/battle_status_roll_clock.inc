#include "lufia2/battle_status_roll_steps.h"

_Static_assert(LUFIA2_BATTLE_STATUS_ROLL_MAX_OPCODES <= 32u,
               "Status roll progress mask");

static const FixedSpanOpcode status_roll_chance[] = {
    {0xd40du, 1u, 2u, 0u}, {0xd40eu, 2u, 6u, 4u}, {0xd410u, 1u, 2u, 0u},
    {0xd411u, 1u, 2u, 0u}, {0xd412u, 1u, 2u, 0u}, {0xd413u, 1u, 2u, 0u},
    {0xd414u, 2u, 5u, 2u}, {0xd416u, 2u, 3u, 0u},
};
static const FixedSpanOpcode status_roll_remove[] = {
    {0xd418u, 2u, 3u, 1u}, {0xd41au, 2u, 2u, 0u}, {0xd41cu, 3u, 5u, 1u},
    {0xd41fu, 3u, 4u, 1u}, {0xd422u, 3u, 5u, 1u}, {0xd425u, 3u, 6u, 1u},
};
static const FixedSpanOpcode status_roll_add[] = {
    {0xd44au, 2u, 3u, 1u}, {0xd44cu, 3u, 5u, 1u}, {0xd44fu, 2u, 2u, 0u},
    {0xd451u, 3u, 4u, 1u}, {0xd454u, 3u, 5u, 1u}, {0xd457u, 3u, 6u, 1u},
};

static bool StatusRollAddressIsRam(uint32_t address) {
    const unsigned bank = address >> 16;
    return bank == 0x7eu || bank == 0x7fu ||
           ((bank & 0x7fu) < 0x40u && (address & 0xffffu) < 0x2000u);
}

static void ActorBridgeBattleStatusRoll(CpuState *cpu, uint32_t entry) {
    if (!BattleStatusRollHookOwned(entry, ActorBridgeBattleStatusRoll) ||
        cpu != &g_cpu || cpu->PB != 0x85u || cpu->D || cpu->_flag_D ||
        cpu->m_flag != 1u || cpu->x_flag || !cpu->ram ||
        !(cpu->DB < 0x40u || (cpu->DB >= 0x7eu && cpu->DB < 0xc0u)) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return;
    const uint32_t pointer = cpu->ram[0xb8u] | (uint32_t)cpu->ram[0xb9u] << 8 |
                             (uint32_t)cpu->ram[0xbau] << 16;
    if (!StatusRollAddressIsRam((pointer + cpu->Y) & 0xffffffu))
        return;
    const bool adding = entry == 0x85d43fu;
    const bool finished = (cpu->ram[0x54u] & 0x80u) != 0u;
    if (finished && (cpu->X < 0x200u ||
                     !StatusRollAddressIsRam(
                         (((uint32_t)cpu->DB << 16) + cpu->X + 0xfu) & 0xffffffu) ||
                     !StatusRollAddressIsRam(((uint32_t)cpu->DB << 16) |
                                             (adding ? 0xa35u : 0xa36u))))
        return;
    const uint32_t fence = adding ? (finished ? 0x85d45au : 0x85d44au)
                                  : (finished ? 0x85d428u : 0x85d418u);
    const unsigned count = finished ? 14u : 8u;
    const unsigned reserve = finished ? 448u : 256u;
    // Every data access is WRAM; this span cannot enable HDMA.
    if (cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowWithDeferredHdmaSupported(cpu, entry, reserve, 0x1ffdu,
                                                     false, true) ||
        !Lufia2DecompFrameRangeAllows(entry, fence, BattleStatusRollHookOwned) ||
        Lufia2DecompFrameSpanBudget(entry, fence, ActorBridgeBattleStatusRoll) < count)
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    FixedSpanOpcode timing[LUFIA2_BATTLE_STATUS_ROLL_MAX_OPCODES];
    memcpy(timing, status_roll_chance, sizeof status_roll_chance);
    if (adding)
        for (unsigned i = 0u; i < 8u; ++i)
            timing[i].pc += 0x32u;
    if (finished) {
        timing[7].cycles = 2u;
        memcpy(timing + 8u, adding ? status_roll_add : status_roll_remove,
               sizeof status_roll_remove);
    }
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared =
        Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, (unsigned)(fence - entry), &fetch);
    FixedSpanClock clock = {.cpu = cpu,
                            .bank = 0x850000u,
                            .timing = timing,
                            .count = count,
                            .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result =
        Lufia2BattleStatusRollStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY &&
           result.pc == (finished ? fence
                         : adding ? 0x85d439u
                                  : 0x85d407u));
    assert(clock.index == count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
