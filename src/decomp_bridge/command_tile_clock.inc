#include "lufia2/battle_command_tiles.h"

static void ActorBridgeCommandTile(CpuState *cpu, uint32_t entry);

static bool CommandTileHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return pc == 0x81cb99u && hook == ActorBridgeCommandTile;
}

static bool CommandTileClockSupported(const CpuState *cpu, uint32_t entry) {
    enum { clock_reserve = 34u * 8u + 80u };
    if (entry != 0x81cb99u || cpu != &g_cpu || cpu->m_flag != 1u ||
        cpu->x_flag || cpu->DB != 0x97u || cpu->D || cpu->_flag_D ||
        cpu->X > 6u || cpu->X % 3u ||
        (cpu->host_return_valid && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        cpu->S < 0x1f02u || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - clock_reserve ||
        !g_snes || !g_snes->dma || g_snes->dma->hdmaPendingInit)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    if (!ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu, false,
            fence - clock_reserve - 1u, clock_reserve))
        return false;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    return phase < 1364u && (!charged || charged == cpu->master_cycles) &&
        Lufia2DecompFrameRangeAllows(entry, 0x81cbafu, CommandTileHookOwned);
}

static void ActorBridgeCommandTile(CpuState *cpu, uint32_t entry) {
    if (!CommandTileClockSupported(cpu, entry))
        return;
    static const FixedSpanOpcode timing[] = {
        {0xcb99u, 4u, 5u, 1u}, {0xcb9du, 2u, 3u, 1u},
        {0xcb9fu, 4u, 5u, 1u}, {0xcba3u, 2u, 3u, 1u},
        {0xcba5u, 4u, 5u, 1u}, {0xcba9u, 2u, 3u, 1u},
        {0xcbabu, 1u, 2u, 0u}, {0xcbacu, 1u, 2u, 0u},
        {0xcbadu, 1u, 2u, 0u}, {0xcbaeu, 1u, 4u, 2u}
    };
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x810000u;
    clock.timing = timing;
    clock.count = sizeof timing / sizeof timing[0];
    const Lufia2Memory memory = {
        FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2BattlePrepareCommandTile(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == 0x81cbafu);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
