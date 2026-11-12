#include "lufia2/battle_command_tiles.h"

static void ActorBridgeCommandTile(CpuState *cpu, uint32_t entry);
#if defined(LUFIA2_HAVE_NATIVE_SELECTED_COMMAND_TILE)
static void ActorBridgeSelectedCommandTile(CpuState *cpu, uint32_t entry);
#endif

static bool CommandTileHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    if (pc == 0x81cb99u && hook == ActorBridgeCommandTile)
        return true;
#if defined(LUFIA2_HAVE_NATIVE_SELECTED_COMMAND_TILE)
    return pc == 0x81cbb8u && hook == ActorBridgeSelectedCommandTile;
#else
    return false;
#endif
}

static bool CommandTileClockContextSupported(
    const CpuState *cpu, uint32_t entry, unsigned clock_reserve) {
    if (cpu != &g_cpu || cpu->m_flag != 1u ||
        cpu->x_flag || cpu->DB != 0x97u || cpu->D || cpu->_flag_D ||
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
    return phase < 1364u && (!charged || charged == cpu->master_cycles);
}

static bool CommandTileClockSupported(const CpuState *cpu, uint32_t entry) {
    return entry == 0x81cb99u && cpu->X <= 6u && !(cpu->X % 3u) &&
        CommandTileClockContextSupported(cpu, entry, 34u * 8u + 80u) &&
        Lufia2DecompFrameRangeAllows(entry, 0x81cbafu, CommandTileHookOwned);
}

static void RunCommandTilePhase(CpuState *cpu, const FixedSpanOpcode *timing,
    unsigned count, ActorWholeFunction prepare, uint32_t stop) {
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x810000u;
    clock.timing = timing;
    clock.count = count;
    const Lufia2Memory memory = {
        FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = prepare(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == stop);
    (void)stop;
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
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
    RunCommandTilePhase(cpu, timing, sizeof timing / sizeof timing[0],
        Lufia2BattlePrepareCommandTile, 0x81cbafu);
}

#if defined(LUFIA2_HAVE_NATIVE_SELECTED_COMMAND_TILE)
static bool SelectedCommandTileClockSupported(const CpuState *cpu, uint32_t entry) {
    return entry == 0x81cbb8u &&
        CommandTileClockContextSupported(cpu, entry, 53u * 8u + 80u) &&
        Lufia2DecompFrameRangeAllows(entry, 0x81cbddu, CommandTileHookOwned);
}

static void ActorBridgeSelectedCommandTile(CpuState *cpu, uint32_t entry) {
    if (!SelectedCommandTileClockSupported(cpu, entry))
        return;
    static const FixedSpanOpcode timing[] = {
        {0xcbb8u, 2u, 3u, 0u}, {0xcbbau, 2u, 3u, 1u},
        {0xcbbcu, 1u, 2u, 0u}, {0xcbbdu, 1u, 2u, 0u},
        {0xcbbeu, 2u, 2u, 0u}, {0xcbc0u, 1u, 2u, 0u},
        {0xcbc1u, 3u, 4u, 1u}, {0xcbc4u, 2u, 3u, 0u},
        {0xcbc6u, 2u, 4u, 2u}, {0xcbc8u, 4u, 5u, 1u},
        {0xcbccu, 1u, 2u, 0u}, {0xcbcdu, 2u, 2u, 0u},
        {0xcbcfu, 2u, 3u, 1u}, {0xcbd1u, 4u, 5u, 1u},
        {0xcbd5u, 2u, 3u, 1u}, {0xcbd7u, 4u, 5u, 1u},
        {0xcbdbu, 2u, 3u, 1u}
    };
    RunCommandTilePhase(cpu, timing, sizeof timing / sizeof timing[0],
        Lufia2BattlePrepareSelectedCommandTile, 0x81cbddu);
}
#endif
