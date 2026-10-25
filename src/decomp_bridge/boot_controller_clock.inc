typedef struct BootControllerClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[8];
    ActorPushedCall call;
} BootControllerClock;

static const FixedSpanOpcode boot_controller_8035[] = {
    {0x8035u, 2u, 3u, 0u},
    {0x8037u, 2u, 2u, 0u},
    {0x8039u, 1u, 3u, 1u},
    {0x803au, 1u, 4u, 1u},
    {0x803bu, 2u, 2u, 0u},
    {0x803du, 3u, 4u, 1u},
    {0x8040u, 3u, 6u, 2u},
};

static const FixedSpanOpcode boot_controller_8043[] = {
    {0x8043u, 3u, 6u, 2u},
};

static const FixedSpanOpcode boot_controller_8046[] = {
    {0x8046u, 4u, 8u, 3u},
};

static const FixedSpanOpcode boot_controller_804a[] = {
    {0x804au, 4u, 8u, 3u},
};

static const FixedSpanOpcode boot_controller_8053[] = {
    {0x8053u, 3u, 6u, 2u},
};

static const FixedSpanOpcode boot_controller_8056[] = {
    {0x8056u, 2u, 2u, 0u},
    {0x8058u, 3u, 4u, 1u},
    {0x805bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode boot_controller_805e[] = {
    {0x805eu, 4u, 8u, 3u},
};

static const FixedSpanOpcode boot_controller_8062[] = {
    {0x8062u, 2u, 2u, 0u},
    {0x8064u, 3u, 4u, 1u},
    {0x8067u, 4u, 8u, 3u},
};

static const FixedSpanOpcode boot_controller_8070[] = {
    {0x8070u, 2u, 2u, 0u},
    {0x8072u, 3u, 4u, 1u},
    {0x8075u, 3u, 6u, 2u},
};

static const FixedSpanOpcode boot_controller_8078[] = {
    {0x8078u, 4u, 8u, 3u},
};

static const FixedSpanOpcode boot_controller_807c[] = {
    {0x807cu, 4u, 8u, 3u},
};

static const FixedSpanOpcode boot_controller_8080[] = {
    {0x8080u, 4u, 8u, 3u},
};

static bool PrepareBootController(BootControllerClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x800000u;
    clock->span.timing = clock->timing;
    if (!cpu->ram || !g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x10000u ||
        cpu->PB != 0x80u || cpu->emulation || cpu->m_flag != 1u ||
        cpu->x_flag != 1u || cpu->X > 0xffu || cpu->Y > 0xffu || cpu->D ||
        cpu->_flag_D || cpu->S < 0x1f20u || cpu->S > 0x1fffu ||
        (pc != 0x808035u && cpu->DB != 0x80u) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return false;
    const FixedSpanOpcode *opcodes = NULL;
    unsigned count = 0u;
    switch (pc) {
    case 0x808035u: opcodes = boot_controller_8035; count = 7u; break;
    case 0x808043u: opcodes = boot_controller_8043; count = 1u; break;
    case 0x808046u: opcodes = boot_controller_8046; count = 1u; break;
    case 0x80804au: opcodes = boot_controller_804a; count = 1u; break;
    case 0x808053u: opcodes = boot_controller_8053; count = 1u; break;
    case 0x808056u: opcodes = boot_controller_8056; count = 3u; break;
    case 0x80805eu: opcodes = boot_controller_805e; count = 1u; break;
    case 0x808062u: opcodes = boot_controller_8062; count = 3u; break;
    case 0x808070u: opcodes = boot_controller_8070; count = 3u; break;
    case 0x808078u: opcodes = boot_controller_8078; count = 1u; break;
    case 0x80807cu: opcodes = boot_controller_807c; count = 1u; break;
    case 0x808080u: opcodes = boot_controller_8080; count = 1u; break;
    case 0x80804eu:
        clock->timing[0] = (FixedSpanOpcode){0x804eu, 2u, (uint8_t)(cpu->_flag_C ? 3u : 2u), 0u};
        clock->timing[1] = (FixedSpanOpcode){(uint16_t)(cpu->_flag_C ? 0x806du : 0x8050u), 3u, 6u, 2u};
        count = 2u;
        break;
    default: return false;
    }
    if (opcodes)
        memcpy(clock->timing, opcodes, count * sizeof *opcodes);
    clock->span.count = count;
    const uint64_t budget = 384u;
    if (cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x808035u, 0x1fffu, false, 1363u, budget) ||
        (!g_snes->hdmaBeamOff && g_snes->nmiEnabled) || g_snes->dma->hdmaPendingInit || g_snes->vPos >= 262u)
        return false;
    for (unsigned channel = 0u; channel < 8u; ++channel)
        if (g_snes->dma->channel[channel].hdmaActive)
            return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if (lines * 1364u - g_snes->hPos <= budget)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static uint8_t BootControllerChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    BootControllerClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareBootController(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeBootController(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    BootControllerClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid || !PrepareBootController(&clock, 0x808035u))
        return ActorBridgeFallback(cpu, &frame, 0x808035u);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2RunBootSequence(&memory, &state,
        BootControllerChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
}
