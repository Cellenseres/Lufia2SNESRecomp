static RecompReturn ActorBridgeTimedActionWork(CpuState *cpu, unsigned kind) {
    static const uint16_t offsets[] = {0xccceu, 0xcce3u, 0xccf8u, 0xcd8cu, 0xcd9bu};
    static const uint16_t returns[] = {0xcce2u, 0xccf7u, 0xcd0fu, 0xcd9au, 0xcda9u};
    static const BattleLeafCost costs[] = {
        {896u, 541u, 145u}, {896u, 541u, 145u}, {3344u, 2327u, 343u},
        {2048u, 1500u, 275u}, {2048u, 1500u, 275u}
    };
    const uint32_t entry = (uint32_t)cpu->PB << 16 | offsets[kind];
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    BattleLeafCost cost = costs[kind];
    const bool supported = cpu == &g_cpu && cpu->ram && !cpu->emulation && !cpu->_flag_D &&
        (cpu->PB == 0x85u || cpu->PB == 0x05u) && (kind < 3u || cpu->m_flag) &&
        g_snes && g_snes->cart &&
        g_snes->cart->type == CART_LOROM &&
        interp_bridge_use_absolute_apu_timeline(rtl_apu_frame_timeline_active(),
            cart_has_sa1(g_snes->cart), rtl_apu_extended_frame_timing() &&
            g_snes->apu && g_snes->apu->portTimeValid) &&
        BattleQuietClockSupported(cpu, entry,
            (entry & 0xff0000u) | (uint32_t)(returns[kind] + 1u), 3u, true,
            BattleLeafClockRoom(entry, cost));
    if (!supported)
        return ActorBridgeFallback(cpu, &frame, entry);
    if (frame.hrv)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result;
    switch (kind) {
    case 0u:
        result = Lufia2BattleClearActionWork(&memory, &state);
        break;
    case 1u:
        result = Lufia2BattleClearSavedActionWork(&memory, &state);
        break;
    case 2u:
        result = Lufia2BattleClearActionRecords(&memory, &state);
        break;
    case 3u:
        result = Lufia2BattleSaveActionWork(&memory, &state);
        break;
    default:
        result = Lufia2BattleRestoreActionWork(&memory, &state);
        break;
    }
    assert(result.flow == LUFIA2_EXECUTION_RETURNED &&
        (uint16_t)result.pc == returns[kind]);
    ActorBridgeStore(cpu, &state);
    cost.cycles -= 6u;
    --cost.rom_bytes;
    cost.ram_bytes -= 3u;
    Lufia2DecompFrameReturnAccountOpcode(cpu, cost.cycles,
        BattleLeafMaster(entry, cost), true);
    return ReturnTimedLongCall(cpu, &frame, (entry & 0xff0000u) | returns[kind]);
}
