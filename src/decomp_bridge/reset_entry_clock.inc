static const FixedSpanOpcode reset_entry_opcodes[] = {
    {0x8000u, 1u, 2u, 0u}, {0x8001u, 1u, 2u, 0u},
    {0x8002u, 1u, 2u, 0u}, {0x8003u, 1u, 2u, 0u},
    {0x8004u, 2u, 3u, 0u}, {0x8006u, 2u, 2u, 0u},
    {0x8008u, 1u, 3u, 1u}, {0x8009u, 1u, 4u, 1u},
    {0x800au, 2u, 2u, 0u}, {0x800cu, 3u, 4u, 1u},
    {0x800fu, 2u, 3u, 1u}, {0x8011u, 2u, 3u, 1u},
    {0x8013u, 3u, 4u, 1u}, {0x8016u, 3u, 4u, 1u},
    {0x8019u, 3u, 4u, 1u}, {0x801cu, 2u, 3u, 0u},
    {0x801eu, 3u, 3u, 0u}, {0x8021u, 1u, 2u, 0u},
    {0x8022u, 3u, 3u, 0u}, {0x8025u, 1u, 2u, 0u},
    {0x8026u, 2u, 3u, 0u}, {0x8028u, 1u, 2u, 0u},
    {0x8029u, 4u, 5u, 1u}, {0x802du, 2u, 3u, 1u},
    {0x802fu, 2u, 3u, 1u}, {0x8031u, 4u, 4u, 0u}
};

static bool ResetEntryWindow(const CpuState *cpu, uint32_t pc) {
    const unsigned budget = 768u;
    if ((pc != 0x008000u && pc != 0x808000u) || cpu->PB != pc >> 16 ||
        cpu->emulation > 1u || cpu->m_flag != 1u || cpu->x_flag != 1u ||
        cpu->X > 0xffu || cpu->Y > 0xffu || cpu->D || cpu->host_return_valid ||
        cpu->S < 0x100u || cpu->S > 0x1fffu ||
        (cpu->emulation && (cpu->S & 0xff00u) != 0x100u) ||
        !cpu->ram || !g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x35u ||
        g_snes->ram != cpu->ram || !g_snes->dma ||
        g_snes->nmiEnabled || g_snes->inNmi || g_snes->inIrq ||
        g_snes->hIrqEnabled || g_snes->vIrqEnabled || g_snes->vPos ||
        g_snes->hPos > 256u || g_snes->beamMasterLast != cpu->master_cycles ||
        g_snes->dma->dmaBusy || g_snes->dma->dmaTimer || g_snes->dma->hdmaPendingInit ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget || !Lufia2DecompFrameReturnCpuReady())
        return false;
    for (unsigned i = 0u; i < 8u; ++i)
        if (g_snes->dma->channel[i].dmaActive || g_snes->dma->channel[i].hdmaActive)
            return false;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (deadline <= cpu->master_cycles || deadline - cpu->master_cycles < budget)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool RunNativeResetEntry(CpuState *cpu, uint32_t entry, unsigned *progress) {
    if (!ResetEntryWindow(cpu, entry))
        return false;
    const uint8_t emulation = cpu->emulation;
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = entry & 0xff0000u;
    clock.timing = reset_entry_opcodes;
    clock.count = sizeof reset_entry_opcodes / sizeof reset_entry_opcodes[0];
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2ResetSystem(&memory, &state, emulation);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == 0x808035u);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    cpu->emulation = 0u;
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    *progress = clock.progress_mask;
    return true;
}

static void ActorBridgeResetEntry(CpuState *cpu, uint32_t pc) {
    unsigned progress;
    if (!RunNativeResetEntry(cpu, pc, &progress))
        return;
    Lufia2DecompFrameReturnComplete(
        sizeof reset_entry_opcodes / sizeof reset_entry_opcodes[0], progress);
    interp_bridge_pre_opcode_redirect(0x808035u);
}
