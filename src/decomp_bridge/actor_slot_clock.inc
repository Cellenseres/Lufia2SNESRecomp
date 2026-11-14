enum { ACTOR_SLOT_PHASE_CLOCK_RESERVE = 512u, ACTOR_SLOT_PHASE_MAX_OPCODES = 15u };

static void ActorBridgeActorSlotPhase(CpuState *cpu, uint32_t entry);

static bool ActorSlotPhaseHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return (
#if defined(LUFIA2_HAVE_NATIVE_ACTOR_SLOT_TRAVERSAL)
        pc == 0x83bba1u ||
#endif
        pc == 0x83bba5u || pc == 0x83bbd1u) &&
        hook == ActorBridgeActorSlotPhase;
}

static unsigned PlanActorSlotPhaseClocksForSlot(const CpuState *cpu, uint32_t entry,
    unsigned slot, FixedSpanOpcode *plan, uint32_t *landing) {
    unsigned count = 0u;
#define SLOT_OPCODE(pc, fetched, cycles, data) \
    (plan[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    if (entry == 0x83bbd1u) {
        SLOT_OPCODE(0xbbd1u, 2u, 3u, 1u);
        SLOT_OPCODE(0xbbd3u, 1u, 2u, 0u);
        SLOT_OPCODE(0xbbd4u, 2u, 3u, 1u);
        SLOT_OPCODE(0xbbd6u, 2u, 2u, 0u);
        SLOT_OPCODE(0xbbd8u, 2u, slot == 39u ? 2u : 3u, 0u);
        *landing = slot == 39u ? 0x83bbdau : 0x83bba1u;
        return count;
    }
    const uint8_t state = cpu->ram[0x0622u + slot];
    SLOT_OPCODE(0xbba5u, 2u, cpu->x_flag ? 3u : 4u, cpu->x_flag ? 1u : 2u);
    SLOT_OPCODE(0xbba7u, 3u, 4u, 1u);
    SLOT_OPCODE(0xbbaau, 2u, 2u, 0u);
    SLOT_OPCODE(0xbbacu, 2u, state & 4u ? 3u : 2u, 0u);
    if (state & 4u) {
        *landing = 0x83bbd1u;
        return count;
    }
    SLOT_OPCODE(0xbbaeu, 2u, 2u, 0u);
    SLOT_OPCODE(0xbbb0u, 2u, state & 0x18u ? 3u : 2u, 0u);
    if (!(state & 0x18u)) {
        SLOT_OPCODE(0xbbb2u, 2u, 2u, 0u);
        SLOT_OPCODE(0xbbb4u, 2u, state & 1u ? 3u : 2u, 0u);
        if (state & 1u) {
            *landing = 0x83bbccu;
            return count;
        }
        SLOT_OPCODE(0xbbb6u, 1u, 2u, 0u);
        SLOT_OPCODE(0xbbb7u, 2u, slot ? 3u : 2u, 0u);
        if (!slot) {
            *landing = 0x83bbb9u;
            return count;
        }
    }
    const bool blocked = (cpu->ram[0x1d0a1u] & 0x2cu) != 0u;
    SLOT_OPCODE(0xbbbeu, 4u, 5u, 1u);
    SLOT_OPCODE(0xbbc2u, 2u, 2u, 0u);
    SLOT_OPCODE(0xbbc4u, 2u, blocked ? 2u : 3u, 0u);
    if (blocked) {
        SLOT_OPCODE(0xbbc6u, 1u, 2u, 0u);
        SLOT_OPCODE(0xbbc7u, 2u, slot ? 3u : 2u, 0u);
    }
#undef SLOT_OPCODE
    *landing = blocked && slot ? 0x83bbccu : 0x83bbc9u;
    return count;
}

static bool ActorSlotPhaseClockSupportedWindow(const CpuState *cpu, uint32_t entry,
    unsigned reserve) {
    if ((
#if defined(LUFIA2_HAVE_NATIVE_ACTOR_SLOT_TRAVERSAL)
        entry != 0x83bba1u &&
#endif
        entry != 0x83bba5u && entry != 0x83bbd1u) || cpu != &g_cpu ||
        !cpu->ram || cpu->emulation || cpu->_flag_D || cpu->D ||
        cpu->PB != 0x83u || cpu->DB != 0x83u || cpu->m_flag != 1u ||
        cpu->x_flag > 1u || (entry == 0x83bba1u && cpu->x_flag != 1u) ||
        cpu->ram[0xa7u] >= 40u ||
        (!cpu->x_flag && cpu->ram[0xa8u]) || g_interp_bridge_pc_hook ||
        (cpu->host_return_valid && cpu->host_return_valid != 2u && cpu->host_return_valid != 3u) ||
        !g_snes || !g_snes->dma || !Lufia2DecompFrameReturnCpuReady() || !g_snes->apu ||
        !g_snes->cart || !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize <= 0x1bbdau || g_snes->vPos >= 262u ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu,
            false, 1363u, reserve) ||
        !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit)
        return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < reserve)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

#if defined(LUFIA2_HAVE_NATIVE_ACTOR_SLOT_TRAVERSAL)
enum { ACTOR_SLOT_TRAVERSAL_MAX_OPCODES = 39u * 20u + 11u + ACTOR_SLOT_PHASE_MAX_OPCODES };

typedef struct ActorSlotTraversalPlan {
    FixedSpanOpcode opcodes[ACTOR_SLOT_TRAVERSAL_MAX_OPCODES];
    unsigned count;
    unsigned slots;
    uint32_t landing;
} ActorSlotTraversalPlan;

static bool PlanActorSlotTraversal(const CpuState *cpu, unsigned budget,
    ActorSlotTraversalPlan *plan) {
    plan->count = 0u;
    plan->slots = 0u;
    plan->landing = 0x83bba1u;
    for (unsigned slot = cpu->ram[0xa7u]; slot < 40u; ++slot) {
        FixedSpanOpcode timing[11u + ACTOR_SLOT_PHASE_MAX_OPCODES + 5u];
        timing[0] = (FixedSpanOpcode){0xbba1u, 4u, 8u, 3u};
        memcpy(timing + 1u, actor_record_offset_timing, sizeof actor_record_offset_timing);
        timing[10] = (FixedSpanOpcode){0xab60u, 1u, 6u, 3u};
        unsigned count = 11u;
        uint32_t landing;
        count += PlanActorSlotPhaseClocksForSlot(cpu, 0x83bba5u, slot,
            timing + count, &landing);
        if (landing == 0x83bbd1u)
            count += PlanActorSlotPhaseClocksForSlot(cpu, landing, slot,
                timing + count, &landing);
        if (count > budget - plan->count ||
            count > ACTOR_SLOT_TRAVERSAL_MAX_OPCODES - plan->count)
            break;
        memcpy(plan->opcodes + plan->count, timing, count * sizeof *timing);
        plan->count += count;
        ++plan->slots;
        plan->landing = landing;
        if (landing != 0x83bba1u)
            break;
    }
    return plan->slots != 0u;
}

static unsigned ActorSlotTraversalReserve(const ActorSlotTraversalPlan *plan) {
    unsigned master = 0u;
    for (unsigned opcode = 0u; opcode < plan->count; ++opcode) {
        const FixedSpanOpcode *timing = plan->opcodes + opcode;
        const unsigned bus = timing->fetched + timing->data_bytes;
        master += 8u * bus + 6u * (timing->cycles - bus);
    }
    return master + 80u * (master / 1284u + 2u);
}

typedef struct ActorSlotTraversalClock {
    FixedSpanClock span;
    const ActorSlotTraversalPlan *plan;
    unsigned offset;
    unsigned last_progress;
} ActorSlotTraversalClock;

static void PrepareActorSlotTraversalClock(ActorSlotTraversalClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index < clock->span.count || !clock->span.count)
            return;
        for (unsigned opcode = 0u, mask = clock->span.progress_mask;
             mask; ++opcode, mask >>= 1u)
            if (mask & 1u)
                clock->last_progress = clock->offset + opcode + 1u;
        clock->offset += clock->span.count;
        const unsigned remaining = clock->plan->count - clock->offset;
        clock->span = (FixedSpanClock){.cpu = clock->span.cpu, .bank = 0x830000u,
            .timing = clock->plan->opcodes + clock->offset,
            .count = remaining < 32u ? remaining : 32u};
    }
}

static uint8_t ActorSlotTraversalRead(void *context, uint32_t address) {
    ActorSlotTraversalClock *clock = context;
    PrepareActorSlotTraversalClock(clock);
    return FixedSpanRead(&clock->span, address);
}

static void ActorSlotTraversalWrite(void *context, uint32_t address, uint8_t value) {
    ActorSlotTraversalClock *clock = context;
    PrepareActorSlotTraversalClock(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static bool PrepareActorSlotTraversal(const CpuState *cpu, ActorSlotTraversalPlan *plan,
    unsigned *reserve) {
    const uint32_t entry = 0x83bba1u;
    if (!ActorSlotPhaseClockSupportedWindow(cpu, entry, 0u) ||
        !Lufia2DecompFrameRangeQuiet(0x83ab4fu, 0x83ab61u))
        return false;
    const unsigned budget = Lufia2DecompFrameOwnedSpanBudget(
        entry, 0x83bbdau, ActorBridgeActorSlotPhase);
    if (!budget || !PlanActorSlotTraversal(cpu, budget, plan))
        return false;
    *reserve = ActorSlotTraversalReserve(plan);
    return ActorSlotPhaseClockSupportedWindow(cpu, entry, *reserve);
}

static void ActorBridgeActorSlotTraversal(CpuState *cpu) {
    ActorSlotTraversalPlan plan;
    unsigned reserve;
    if (!PrepareActorSlotTraversal(cpu, &plan, &reserve))
        return;
    const uint64_t start = cpu->master_cycles;
    ActorSlotTraversalClock clock = {.span = {.cpu = cpu, .bank = 0x830000u,
        .timing = plan.opcodes, .count = plan.count < 32u ? plan.count : 32u},
        .plan = &plan};
    const Lufia2Memory memory = {
        ActorSlotTraversalRead, ActorSlotTraversalWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2ActorResumeSlotTraversal(&memory, &state, plan.slots);
    PrepareActorSlotTraversalClock(&clock);
    assert(clock.offset == plan.count && !clock.span.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == plan.landing);
    ActorBridgeStore(cpu, &state);
    /* The original AB4F dispatch leaves its JSL return mode active. */
    cpu->host_return_valid = 3u;
    assert(cpu->master_cycles - start <= reserve);
    Lufia2DecompFrameReturnCompleteSpan(plan.count, clock.last_progress);
    interp_bridge_pre_opcode_redirect(result.pc);
}

#endif

static void ActorBridgeActorSlotPhase(CpuState *cpu, uint32_t entry) {
#if defined(LUFIA2_HAVE_NATIVE_ACTOR_SLOT_TRAVERSAL)
    if (entry == 0x83bba1u) {
        ActorBridgeActorSlotTraversal(cpu);
        return;
    }
#endif
    if (!ActorSlotPhaseClockSupportedWindow(cpu, entry, ACTOR_SLOT_PHASE_CLOCK_RESERVE))
        return;
    FixedSpanOpcode timing[ACTOR_SLOT_PHASE_MAX_OPCODES];
    uint32_t landing;
    const unsigned count = PlanActorSlotPhaseClocksForSlot(
        cpu, entry, cpu->ram[0xa7u], timing, &landing);
    const uint32_t end = entry == 0x83bba5u ? 0x83bbd1u : 0x83bbdau;
    if (Lufia2DecompFrameSpanBudget(entry, end, ActorBridgeActorSlotPhase) < count)
        return;
    const uint64_t start = cpu->master_cycles;
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x830000u, .timing = timing, .count = count};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = entry == 0x83bba5u
        ? Lufia2ActorSelectSlotUpdate(&memory, &state)
        : Lufia2ActorAdvanceSlot(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == count && !clock.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == landing);
    ActorBridgeStore(cpu, &state);
    assert(cpu->master_cycles - start <= ACTOR_SLOT_PHASE_CLOCK_RESERVE);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
