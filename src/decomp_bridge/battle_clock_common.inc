#ifndef LUFIA2_BATTLE_CLOCK_COMMON
#define LUFIA2_BATTLE_CLOCK_COMMON

static bool BattleClockAllows(const CpuState *cpu, uint32_t entry, uint32_t end,
    unsigned frame_size, bool wide, unsigned room, bool vblank_only,
    bool (*owned)(uint32_t, void (*)(CpuState *, uint32_t))) {
    if (cpu->x_flag ||
        (cpu->host_return_valid && cpu->host_return_valid != frame_size) ||
        (!Lufia2DecompFrameReturnCpuReady() &&
         !Lufia2DecompFramePairedCallReady(cpu, entry)) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - room ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu - frame_size, wide,
                                             1363u, room) ||
        (vblank_only && g_snes->vPos < 225u) || g_snes->vPos >= 262u ||
        ((g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos) * 1364u -
            g_snes->hPos < room ||
        !Lufia2DecompFrameRangeAllows(entry, end, owned))
        return false;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    return phase < 1364u && (!charged || charged == cpu->master_cycles);
}

static bool BattleQuietClockAllows(const CpuState *cpu, uint32_t entry, uint32_t end,
    unsigned frame_size, bool wide, unsigned room,
    bool (*owned)(uint32_t, void (*)(CpuState *, uint32_t))) {
    return BattleClockAllows(cpu, entry, end, frame_size, wide, room, true, owned);
}

static bool BattleQuietClockSupported(const CpuState *cpu, uint32_t entry, uint32_t end,
                                      unsigned frame_size, bool wide, unsigned room) {
    return BattleQuietClockAllows(cpu, entry, end, frame_size, wide, room, NULL);
}

#endif
