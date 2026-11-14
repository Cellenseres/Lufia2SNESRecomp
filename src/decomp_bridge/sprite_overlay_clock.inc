#include "sprite_overlay_plan.inc"

static void ActorBridgeSpriteOverlays(CpuState *cpu, uint32_t entry);

static bool SpriteOverlayHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return pc == 0x81b680u && hook == ActorBridgeSpriteOverlays;
}

typedef struct SpriteOverlayClock {
    FixedSpanClock span;
    const SpriteOverlayPlan *plan;
    FixedSpanOpcode timing[32];
    unsigned completed, last_progress;
} SpriteOverlayClock;

static void PrepareSpriteOverlayClock(SpriteOverlayClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index < clock->span.count)
            return;
        for (unsigned opcode = 0u, mask = clock->span.progress_mask;
             mask; ++opcode, mask >>= 1u)
            if (mask & 1u)
                clock->last_progress = clock->completed + opcode + 1u;
        clock->completed += clock->span.count;
        const unsigned remaining = clock->plan->opcode_count - clock->completed;
        const unsigned count = remaining < 32u ? remaining : 32u;
        for (unsigned index = 0u; index < count; ++index)
            clock->timing[index] = SpriteOverlayOpcode(clock->plan, clock->completed + index);
        clock->span = (FixedSpanClock){.cpu = clock->span.cpu, .bank = 0x810000u,
            .timing = clock->timing, .count = count};
        if (!count)
            return;
    }
}

static uint8_t SpriteOverlayRead(void *context, uint32_t address) {
    SpriteOverlayClock *clock = context;
    PrepareSpriteOverlayClock(clock);
    return FixedSpanRead(&clock->span, address);
}

static void SpriteOverlayWrite(void *context, uint32_t address, uint8_t value) {
    SpriteOverlayClock *clock = context;
    PrepareSpriteOverlayClock(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static unsigned SpriteOverlayReserve(const SpriteOverlayPlan *plan) {
    const unsigned master = 8u * (plan->count ? 27u + 42u * plan->count : 7u);
    return master + 80u * (master / 1284u + 2u);
}

static bool SpriteOverlayWindowSupported(const CpuState *cpu, unsigned reserve) {
#if defined(SNES_COSIM) || (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return false;
#endif
    if (cpu != &g_cpu || !cpu->ram || cpu->PB != 0x81u || cpu->emulation ||
        cpu->m_flag != 1u || cpu->x_flag || cpu->D || cpu->_flag_D ||
        !((cpu->DB & 0x7fu) < 0x40u || cpu->DB == 0x7eu) ||
        (cpu->host_return_valid && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0xb700u ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x81b680u, 0x1ffcu,
            false, 1363u, reserve) || g_snes->vPos >= 262u ||
        (!g_snes->hdmaBeamOff && g_snes->vPos < 225u) ||
        g_snes->dma->hdmaPendingInit)
        return false;
    const unsigned edge = g_snes->vPos < 225u ? 225u : 262u;
    if ((edge - g_snes->vPos) * 1364u - g_snes->hPos < reserve)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static void ActorBridgeSpriteOverlays(CpuState *cpu, uint32_t entry) {
    if (entry != 0x81b680u || !SpriteOverlayWindowSupported(cpu, 0u))
        return;
    SpriteOverlayPlan plan;
    if (!PlanDisabledSpriteOverlays(cpu->ram, &plan) ||
        !Lufia2DecompFrameRangeAllows(entry, 0x81b700u, SpriteOverlayHookOwned) ||
        Lufia2DecompFrameSpanBudget(entry, 0x81b700u,
            ActorBridgeSpriteOverlays) < plan.opcode_count)
        return;
    const unsigned reserve = SpriteOverlayReserve(&plan);
    if (!SpriteOverlayWindowSupported(cpu, reserve))
        return;
    const uint64_t start = cpu->master_cycles;
    SpriteOverlayClock clock = {.span = {.cpu = cpu}, .plan = &plan};
    const Lufia2Memory memory = {SpriteOverlayRead, SpriteOverlayWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleApplySpritePositionOverlays(&memory, &state);
    PrepareSpriteOverlayClock(&clock);
    assert(clock.completed == plan.opcode_count && !clock.span.remaining &&
        result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == 0x81b700u);
    ActorBridgeStore(cpu, &state);
    assert(cpu->master_cycles - start <= reserve);
    Lufia2DecompFrameReturnCompleteSpan(clock.completed, clock.last_progress);
    interp_bridge_pre_opcode_redirect(result.pc);
}
