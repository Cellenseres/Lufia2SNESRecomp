enum { SOUND_BEGIN_MASTER_BUDGET = 192u };

typedef struct SoundBeginClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[5];
} SoundBeginClock;

static uint8_t SoundBeginRead(void *context, uint32_t address) {
    SoundBeginClock *clock = context;
    const uint8_t value = FixedSpanRead(&clock->span, address);
    assert(address == 0x002142u && clock->span.index == 3u);
    const bool ready = value == 0xffu;
    clock->timing[3].cycles = ready ? 2u : 3u;
    clock->span.count = ready ? 5u : 4u;
    return value;
}

static void SoundBeginWrite(void *context, uint32_t address, uint8_t value) {
    SoundBeginClock *clock = context;
    FixedSpanWrite(&clock->span, address, value);
}

static bool SoundBeginSupported(const CpuState *cpu, uint32_t entry) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    (void)cpu;
    (void)entry;
    return false;
#else
    if (cpu != &g_cpu || entry != 0x809945u || cpu->PB != 0x80u ||
        cpu->m_flag != 1u || cpu->x_flag || cpu->D || cpu->_flag_D ||
        !Lufia2DecompFrameReturnCpuReady() || !g_snes || g_snes->vPos >= 262u ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - SOUND_BEGIN_MASTER_BUDGET ||
        !ActorBridgeSoundWindowSupported(cpu, entry, 0x1fffu, SOUND_BEGIN_MASTER_BUDGET))
        return false;
    const Cart *cart = g_snes->cart;
    if (!cart || cart->type != CART_LOROM || !cart->rom || cart->romSize < 0x1953u ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1)
        return false;
    return Lufia2DecompFrameSpanBudget(entry, entry + 1u,
               ActorBridgeSoundBeginPayload) >= 5u &&
        Lufia2DecompFrameRangeAllows(entry, 0x809953u, ActorBridgeSoundHookOwned);
#endif
}

static void ActorBridgeSoundBeginPayload(CpuState *cpu, uint32_t entry) {
    if (!SoundBeginSupported(cpu, entry))
        return;
    SoundBeginClock clock = {
        .span = {.cpu = cpu, .bank = 0x800000u, .count = 5u},
        .timing = {
            {0x9945u, 2u, 2u, 0u}, {0x9947u, 4u, 5u, 1u},
            {0x994bu, 4u, 5u, 1u}, {0x994fu, 2u, 2u, 0u},
            {0x9951u, 2u, 3u, 1u}
        }
    };
    clock.span.timing = clock.timing;
    const Lufia2Memory memory = {SoundBeginRead, SoundBeginWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2SoundBeginPayloadStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    assert(result.pc == 0x80994bu || result.pc == 0x809953u);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.span.count, clock.span.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
