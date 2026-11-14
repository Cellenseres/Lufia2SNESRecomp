#include "hdma_builder_plan.inc"

/* Reverse source indices can wrap into bank $86. */
enum { HDMA_BUILDER_ROM_END = 0x323b3u };

_Static_assert(LUFIA2_NMI_HDMA_BUILDER_OPCODES >= 395u,
    "reserve the complete builder before its original RTS");

static void ActorBridgeHdmaBuilder(CpuState *cpu, uint32_t entry);

static bool HdmaBuilderHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    if (hook != ActorBridgeHdmaBuilder)
        return false;
    for (unsigned mode = 0u; mode < 6u; ++mode)
        if (pc == (0x850000u | hdma_builder_profiles[mode].entry))
            return true;
    return false;
}

typedef struct HdmaBuilderClock {
    FixedSpanClock span;
    const HdmaBuilderPlan *plan;
    FixedSpanOpcode timing[32];
    unsigned completed, last_progress;
} HdmaBuilderClock;

static void PrepareHdmaBuilderClock(HdmaBuilderClock *clock) {
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
            clock->timing[index] = HdmaBuilderOpcode(clock->plan, clock->completed + index);
        clock->span = (FixedSpanClock){.cpu = clock->span.cpu, .bank = 0x850000u,
            .timing = clock->timing, .count = count};
        if (!count)
            return;
    }
}

static uint8_t HdmaBuilderRead(void *context, uint32_t address) {
    HdmaBuilderClock *clock = context;
    PrepareHdmaBuilderClock(clock);
    return FixedSpanRead(&clock->span, address);
}

static void HdmaBuilderWrite(void *context, uint32_t address, uint8_t value) {
    HdmaBuilderClock *clock = context;
    PrepareHdmaBuilderClock(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static bool HdmaBuilderWindowSupported(const CpuState *cpu, const HdmaBuilderPlan *plan) {
#if defined(LUFIA2_HAVE_NATIVE_NMI_RESTORE_WIDTH)
    if (cpu != &g_cpu || !cpu->ram || cpu->PB != 0x85u || cpu->DB != 0x85u ||
        cpu->D || cpu->emulation || cpu->_flag_D || cpu->m_flag != 1u || cpu->x_flag ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) || cpu->S < 0x1f20u || cpu->S > 0x1ff8u ||
        !Lufia2DecompFrameReturnCpuReady() || !Lufia2DecompNmiHasInterruptOwner() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !g_snes || !g_snes->dma || !g_snes->apu || !g_snes->cart ||
        g_snes->ram != cpu->ram || !g_snes->inNmi || g_snes->inIrq ||
        g_snes->hIrqEnabled || g_snes->vIrqEnabled || g_snes->hPos >= 1364u ||
        g_snes->vPos >= 262u || g_snes->beamMasterLast != cpu->master_cycles ||
        g_snes->dma->dmaBusy || g_snes->dma->dmaTimer || g_snes->dma->hdmaPendingInit ||
        g_snes->cart->type != CART_LOROM || !g_snes->cart->rom ||
        g_snes->cart->romSize < HDMA_BUILDER_ROM_END ||
        (cpu->ram[cpu->S + 1u] | (unsigned)cpu->ram[cpu->S + 2u] << 8) != 0x8f3bu)
        return false;
    for (unsigned channel = 0u; channel < 8u; ++channel)
        if (g_snes->dma->channel[channel].dmaActive)
            return false;
    if ((plan->mode < 2u && plan->phase >= 0x20u) ||
        ((plan->mode == 3u || plan->mode == 4u) && plan->phase > 0x180u))
        return false;
    uint64_t budget = 0u;
    for (unsigned index = 0u; index < plan->opcode_count; ++index) {
        const FixedSpanOpcode opcode = HdmaBuilderOpcode(plan, index);
        budget += opcode.fetched * cpu_region_speed(0x850000u | opcode.pc) +
            opcode.data_bytes * 8u + (opcode.cycles - opcode.fetched - opcode.data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (cpu->master_cycles > UINT64_MAX - budget)
        return false;
    uint64_t phase, refresh_clock;
    snes_refresh_state_get(&phase, &refresh_clock);
    if (phase >= 1364u || refresh_clock != cpu->master_cycles)
        return false;
    const unsigned edge = g_snes->vPos < 225u ? 225u : 262u;
    const uint32_t end = (0x850000u | (plan->profile->early_terminal
        ? plan->profile->early_terminal : plan->profile->terminal)) + 1u;
    return budget < (uint64_t)(edge - g_snes->vPos) * 1364u - g_snes->hPos &&
        NmiSpanAvoidsHdma(g_snes, budget, true) &&
        Lufia2DecompFrameRangeAllows(0x850000u | plan->profile->entry,
            end, HdmaBuilderHookOwned);
#else
    (void)cpu;
    (void)plan;
    return false;
#endif
}

static void ActorBridgeHdmaBuilder(CpuState *cpu, uint32_t entry) {
    if (!cpu->ram)
        return;
    const unsigned phase = cpu->ram[0x1b22u] | (unsigned)cpu->ram[0x1b23u] << 8;
    HdmaBuilderPlan plan;
    if (!PlanHdmaBuilder(entry, phase, &plan) || !HdmaBuilderWindowSupported(cpu, &plan))
        return;
    static const ActorWholeFunction builders[] = {
        Lufia2BattleHdmaBuildMode5, Lufia2BattleHdmaBuildMode6, Lufia2BattleHdmaBuildMode7,
        Lufia2BattleHdmaBuildMode8, Lufia2BattleHdmaBuildMode9, Lufia2BattleHdmaBuildMode10
    };
    HdmaBuilderClock clock = {.span = {.cpu = cpu}, .plan = &plan};
    const Lufia2Memory memory = {HdmaBuilderRead, HdmaBuilderWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = builders[plan.mode](&memory, &state);
    PrepareHdmaBuilderClock(&clock);
    assert(clock.completed == plan.opcode_count && !clock.span.remaining &&
        result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == plan.landing);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnCompleteSpan(clock.completed, clock.last_progress);
    interp_bridge_pre_opcode_redirect(result.pc);
}
