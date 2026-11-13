static BattleLeafCost BattleActionRecordCost(uint8_t mask, unsigned kind) {
    static const BattleLeafCost bases[] = {
        {58u, 39u, 7u}, {381u, 220u, 90u}, {1285u, 765u, 238u}
    };
    BattleLeafCost cost = bases[kind];
    if (mask & 0x80u) {
        ++cost.cycles;
        cost.rom_bytes += 2u;
    }
    while (!(mask & 1u)) {
        cost.cycles += 11u;
        cost.rom_bytes += 6u;
        cost.ram_bytes += 2u;
        mask >>= 1u;
    }
    return cost;
}

static uint8_t BattleActionRecordChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    const Lufia2Memory *memory = context;
    assert((target == 0x85ccceu && frame_size == 3u) ||
        (target == 0x85cdfau && frame_size == 2u));
    const Lufia2ExecutionResult result = target == 0x85ccceu
        ? Lufia2BattleClearActionWork(memory, state)
        : Lufia2BattleActionRecordPointer(memory, state);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED &&
        result.pc == (target == 0x85ccceu ? 0x85cce2u : 0x85ce20u));
    (void)result;
    const uint16_t low = memory->read_byte(memory->context, ++state->stack);
    const uint16_t high = memory->read_byte(memory->context, ++state->stack);
    if (frame_size == 3u)
        state->program_bank = memory->read_byte(memory->context, ++state->stack);
    assert(((uint32_t)state->program_bank << 16 |
        (uint16_t)((high << 8 | low) + 1u)) == site + frame_size + 1u);
    (void)low;
    (void)high;
    (void)site;
    return 1u;
}

static RecompReturn ActorBridgeTimedActionRecord(CpuState *cpu, unsigned kind) {
    static const uint16_t offsets[] = {0xcdfau, 0xcdaau, 0xcdd0u};
    static const uint16_t returns[] = {0xce20u, 0xcdcfu, 0xcdf9u};
    const uint32_t entry = (uint32_t)cpu->PB << 16 | offsets[kind];
    const unsigned frame_size = kind ? 3u : 2u;
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    if (cpu != &g_cpu || !cpu->ram || cpu->emulation || cpu->_flag_D ||
        cpu->PB != 0x85u || !(cpu->A & 0xffu) || !g_snes || !g_snes->cart ||
        g_snes->cart->type != CART_LOROM ||
        (kind ? cpu->D || cpu->S < 0x1f10u :
            !cpu->m_flag || !((cpu->DB & 0x7fu) < 0x40u || cpu->DB == 0x7eu)))
        return ActorBridgeFallback(cpu, &frame, entry);
    BattleLeafCost cost = BattleActionRecordCost((uint8_t)cpu->A, kind);
    if (!interp_bridge_use_absolute_apu_timeline(rtl_apu_frame_timeline_active(),
            cart_has_sa1(g_snes->cart), rtl_apu_extended_frame_timing() &&
            g_snes->apu && g_snes->apu->portTimeValid) ||
        !BattleQuietClockSupported(cpu, entry, 0x850000u | (returns[kind] + 1u),
            frame_size, true, BattleLeafClockRoom(entry, cost)) ||
        (kind && !Lufia2DecompFrameRangeQuiet(0x85cdfau, 0x85ce21u)) ||
        (kind == 2u && !Lufia2DecompFrameRangeQuiet(0x85ccceu, 0x85cce3u)))
        return ActorBridgeFallback(cpu, &frame, entry);
    if (kind) {
        const unsigned pointer = kind == 1u ? 0xb5u : 0xb8u;
        const unsigned address = cpu->ram[pointer] | (unsigned)cpu->ram[pointer + 1u] << 8;
        if (cpu->ram[pointer + 2u] != 0x7fu || address < 0x4000u || address > 0xfff4u)
            return ActorBridgeFallback(cpu, &frame, entry);
    }
    if (frame.hrv)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = !kind
        ? Lufia2BattleActionRecordPointer(&memory, &state)
        : kind == 1u ? Lufia2BattleLoadTurnRecord(&memory, &state,
            BattleActionRecordChild, &memory)
        : Lufia2BattleLoadActionRecord(&memory, &state,
            BattleActionRecordChild, &memory);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc ==
        (0x850000u | returns[kind]));
    ActorBridgeStore(cpu, &state);
    cost.cycles -= 6u;
    --cost.rom_bytes;
    cost.ram_bytes -= frame_size;
    Lufia2DecompFrameReturnAccountOpcode(cpu, cost.cycles,
        BattleLeafMaster(entry, cost), true);
    return kind ? ReturnTimedLongCall(cpu, &frame, result.pc)
                : ReturnTimedMenuCall(cpu, &frame, result.pc);
}
