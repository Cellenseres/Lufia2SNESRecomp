enum { VEHICLE_TERRAIN_MAX_OPCODES = 96u };

static const FixedSpanOpcode vehicle_terrain_timing[] = {
    {0xbbafu, 3u, 4u, 1u}, {0xbbb2u, 2u, 3u, 1u},
    {0xbbb4u, 3u, 4u, 1u}, {0xbbb7u, 2u, 3u, 1u},
    {0xbbb9u, 4u, 8u, 3u}, {0xbbbdu, 2u, 2u, 0u},
    {0xbbbfu, 2u, 2u, 0u}, {0xbbc1u, 2u, 3u, 1u},
    {0xbbc3u, 4u, 8u, 3u}, {0xbbc7u, 4u, 8u, 3u},
    {0xbbcbu, 2u, 2u, 0u}, {0xbbcdu, 2u, 2u, 0u},
    {0xbbcfu, 1u, 2u, 0u}
};

static const FixedSpanOpcode vehicle_direction_timing[] = {
    {0xfb12u, 1u, 3u, 0u}, {0xfb13u, 2u, 2u, 0u},
    {0xfb15u, 1u, 3u, 0u}, {0xfb16u, 1u, 2u, 0u},
    {0xfb17u, 3u, 6u, 2u}
};

_Static_assert(2u * (sizeof field_map_value_timing / sizeof *field_map_value_timing) +
    sizeof vehicle_terrain_timing / sizeof *vehicle_terrain_timing +
    sizeof vehicle_direction_timing / sizeof *vehicle_direction_timing + 4u <=
    LUFIA2_FIELD_VEHICLE_OPCODE_BUDGET - 2u, "reserve the original call and return");

typedef struct VehicleTerrainPlan {
    FixedSpanOpcode opcodes[VEHICLE_TERRAIN_MAX_OPCODES];
    uint8_t banks[VEHICLE_TERRAIN_MAX_OPCODES];
    unsigned count;
    uint32_t landing;
} VehicleTerrainPlan;

static void AppendVehicleOpcode(VehicleTerrainPlan *plan, unsigned bank,
    FixedSpanOpcode opcode) {
    assert(plan->count < VEHICLE_TERRAIN_MAX_OPCODES);
    plan->banks[plan->count] = (uint8_t)bank;
    plan->opcodes[plan->count++] = opcode;
}

static void AppendVehicleParent(VehicleTerrainPlan *plan, unsigned index,
    bool branch_taken) {
    FixedSpanOpcode opcode = vehicle_terrain_timing[index];
    opcode.cycles += branch_taken;
    AppendVehicleOpcode(plan, 0x8eu, opcode);
}

static void AppendVehicleMapProbe(VehicleTerrainPlan *plan) {
    for (unsigned i = 0u; i < sizeof field_map_value_timing / sizeof *field_map_value_timing; ++i)
        AppendVehicleOpcode(plan, 0x83u, field_map_value_timing[i]);
    AppendVehicleOpcode(plan, 0x83u, (FixedSpanOpcode){0xfb8au, 1u, 6u, 3u});
}

static bool VehicleTerrainValue(const uint8_t *ram, unsigned x, unsigned y,
    unsigned layer, unsigned *value) {
    const unsigned base = ram[0x1d008u + layer] |
        (unsigned)ram[0x1d009u + layer] << 8;
    const uint16_t cell = (uint16_t)(base + 2u * (x + y * ram[0x5b9u]));
    if (cell == 0xffffu)
        return false;
    const unsigned tile = (ram[0x10000u + cell] |
        (unsigned)ram[0x10001u + cell] << 8) & 0x3ffu;
    const unsigned attributes = ram[0x1d03eu] | (unsigned)ram[0x1d03fu] << 8;
    *value = ram[0x10000u + (uint16_t)(attributes + tile)];
    return true;
}

static bool PlanVehicleTerrain(const uint8_t *ram, VehicleTerrainPlan *plan) {
    *plan = (VehicleTerrainPlan){0};
    const unsigned layer = ram[0x5aau] | (unsigned)ram[0x5abu] << 8;
    const unsigned direction = ram[0x23u];
    if (layer > 6u || (layer & 1u) || direction > 6u || (direction & 1u))
        return false;
    unsigned x = ram[0x6bau], y = ram[0x6e2u], value;
    if (!VehicleTerrainValue(ram, x, y, layer, &value))
        return false;
    for (unsigned i = 0u; i < 5u; ++i)
        AppendVehicleParent(plan, i, false);
    AppendVehicleMapProbe(plan);
    AppendVehicleParent(plan, 5u, false);
    AppendVehicleParent(plan, 6u, value != 6u);
    if (value == 6u) {
        AppendVehicleParent(plan, 7u, false);
        AppendVehicleParent(plan, 8u, false);
        for (unsigned i = 0u; i < sizeof vehicle_direction_timing / sizeof *vehicle_direction_timing; ++i)
            AppendVehicleOpcode(plan, 0x83u, vehicle_direction_timing[i]);
        const unsigned modify = 0xfb22u + 3u * (direction / 2u);
        AppendVehicleOpcode(plan, 0x83u, (FixedSpanOpcode){modify, 2u, 5u, 2u});
        AppendVehicleOpcode(plan, 0x83u, (FixedSpanOpcode){modify + 2u, 1u, 6u, 3u});
        if (direction == 0u)
            y = (uint8_t)(y + 1u);
        else if (direction == 2u)
            x = (uint8_t)(x - 1u);
        else if (direction == 4u)
            y = (uint8_t)(y - 1u);
        else
            x = (uint8_t)(x + 1u);
        if (!VehicleTerrainValue(ram, x, y, layer, &value))
            return false;
        AppendVehicleParent(plan, 9u, false);
        AppendVehicleMapProbe(plan);
        AppendVehicleParent(plan, 10u, false);
        AppendVehicleParent(plan, 11u, value == 6u);
        if (value == 6u) {
            plan->landing = 0x8ebbd1u;
            return true;
        }
    }
    AppendVehicleParent(plan, 12u, false);
    plan->landing = 0x8ebbd0u;
    return true;
}

typedef struct VehicleTerrainClock {
    FixedSpanClock span;
    const VehicleTerrainPlan *plan;
    unsigned offset;
    unsigned last_progress;
    bool paired;
    uint16_t product;
} VehicleTerrainClock;

static void PrepareVehicleTerrainClock(VehicleTerrainClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index < clock->span.count)
            return;
        for (unsigned opcode = 0u, mask = clock->span.progress_mask;
             mask; ++opcode, mask >>= 1u)
            if (mask & 1u)
                clock->last_progress = clock->offset + opcode + 1u;
        if (clock->paired && clock->span.count)
            Lufia2DecompFramePairedAccountSpan(clock->span.cpu, 0x8ebbafu,
                clock->span.count, clock->span.progress_mask);
        clock->offset += clock->span.count;
        if (clock->offset == clock->plan->count)
            return;
        unsigned count = 1u;
        const unsigned bank = clock->plan->banks[clock->offset];
        while (count < 32u && clock->offset + count < clock->plan->count &&
            clock->plan->banks[clock->offset + count] == bank)
            ++count;
        clock->span = (FixedSpanClock){.cpu = clock->span.cpu, .bank = bank << 16,
            .timing = clock->plan->opcodes + clock->offset, .count = count};
    }
}

static uint8_t VehicleTerrainRead(void *context, uint32_t address) {
    VehicleTerrainClock *clock = context;
    PrepareVehicleTerrainClock(clock);
    FixedSpanClock *span = &clock->span;
    assert(clock->offset < clock->plan->count && span->index < span->count);
    if (span->bank != 0x830000u || span->timing[span->index].pc != 0xfa09u)
        return FixedSpanRead(span, address);
    bool progress = false;
    uint8_t value;
    /* The original multiplier result is a paired hardware bus read. */
    if (span->remaining == 2u) {
        assert(address == 0x4216u);
        clock->product = Lufia2DecompFrameReturnReadWord(
            span->cpu, address, address + 1u, &progress);
        value = (uint8_t)clock->product;
    } else {
        assert(span->remaining == 1u && address == 0x4217u);
        value = (uint8_t)(clock->product >> 8);
    }
    span->data_master += cpu_region_speed(address);
    span->progress |= progress;
    if (!--span->remaining)
        FinishFixedSpanOpcode(span);
    return value;
}

static void VehicleTerrainWrite(void *context, uint32_t address, uint8_t value) {
    VehicleTerrainClock *clock = context;
    PrepareVehicleTerrainClock(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static uint8_t VehicleTerrainChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    const Lufia2Memory *memory = context;
    assert(frame_size == 3u);
    state->program_bank = 0x83u;
    if (target == 0x83fb71u)
        Lufia2ActorReadMapCellValue(memory, state);
    else {
        assert(target == 0x83fb12u);
        const uint32_t returned = Lufia2ActorMovementStep(memory, state);
        assert(returned != 0u);
        (void)returned;
    }
    const uint8_t low = memory->read_byte(memory->context, ++state->stack);
    const uint8_t high = memory->read_byte(memory->context, ++state->stack);
    state->program_bank = memory->read_byte(memory->context, ++state->stack);
    assert((((uint32_t)state->program_bank << 16) |
        (uint16_t)((low | (unsigned)high << 8) + 1u)) == site + 4u);
    return 1u;
}

static bool VehicleTerrainHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return pc == 0x8ebbafu && hook == ActorBridgeVehicleTerrain;
}

static bool VehicleTerrainWindowSupported(const CpuState *cpu, unsigned reserve, bool paired) {
    if (cpu != &g_cpu || !cpu->ram || cpu->emulation || cpu->_flag_D || cpu->D ||
        cpu->PB != 0x8eu || cpu->DB != 0x83u || cpu->m_flag != 1u || cpu->x_flag ||
        cpu->host_return_valid != 3u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu ||
        (paired ? !Lufia2DecompFramePairedCallReady(cpu, 0x8ebbafu)
                : !Lufia2DecompFrameReturnCpuReady()) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x8ebbafu, 0x1ffcu, false, 1363u, reserve) ||
        !g_snes->apu || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize <= 0x73bd1u ||
        !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit || g_snes->vPos >= 262u)
        return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < reserve)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool PrepareVehicleTerrain(const CpuState *cpu, VehicleTerrainPlan *plan,
    unsigned *reserve) {
    if (!VehicleTerrainWindowSupported(cpu, 0u, false) ||
        Lufia2DecompFrameSchedulerBounces())
        return false;
    const unsigned budget = Lufia2DecompFrameOwnedSpanBudget(
        0x8ebbafu, 0x8ebbd1u, ActorBridgeVehicleTerrain);
    if (!budget || !Lufia2DecompFrameRangeQuiet(0x83f9d4u, 0x83fa12u) ||
        !Lufia2DecompFrameRangeQuiet(0x83fb12u, 0x83fb2eu) ||
        !Lufia2DecompFrameRangeQuiet(0x83fb71u, 0x83fb8bu) ||
        !PlanVehicleTerrain(cpu->ram, plan) || plan->count > budget)
        return false;
    *reserve = FieldMapClockReserve(plan->opcodes, plan->count);
    return VehicleTerrainWindowSupported(cpu, *reserve, false);
}

static void ActorBridgeVehicleTerrain(CpuState *cpu, uint32_t entry) {
    if (entry != 0x8ebbafu)
        return;
    VehicleTerrainPlan plan;
    unsigned reserve;
    if (!PrepareVehicleTerrain(cpu, &plan, &reserve))
        return;
    const uint64_t start = cpu->master_cycles;
    VehicleTerrainClock clock = {.span = {.cpu = cpu}, .plan = &plan};
    const Lufia2Memory memory = {VehicleTerrainRead, VehicleTerrainWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2FieldProbeVehicleTerrain(
        &memory, &state, VehicleTerrainChild, (void *)&memory);
    PrepareVehicleTerrainClock(&clock);
    assert(clock.offset == plan.count && !clock.span.remaining);
    assert(result.flow == (plan.landing == 0x8ebbd0u ? LUFIA2_EXECUTION_RETURNED :
        LUFIA2_EXECUTION_BOUNDARY) && result.pc == plan.landing);
    ActorBridgeStore(cpu, &state);
    assert(cpu->master_cycles - start <= reserve);
    Lufia2DecompFrameReturnCompleteSpan(plan.count, clock.last_progress);
    interp_bridge_pre_opcode_redirect(result.pc);
}


static RecompReturn ActorBridgeTimedVehicleTerrain(CpuState *cpu) {
    VehicleTerrainPlan plan;
    if (!VehicleTerrainWindowSupported(cpu, 0u, true) ||
        Lufia2DecompFramePairedReturnPc(cpu, 0x8ebbafu) != 0x83c202u ||
        (cpu->ram[cpu->S + 1u] | (unsigned)cpu->ram[cpu->S + 2u] << 8) != 0xc201u ||
        cpu->ram[cpu->S + 3u] != 0x83u ||
        !Lufia2DecompFrameRangeAllows(0x8ebbafu, 0x8ebbd1u, VehicleTerrainHookOwned) ||
        !Lufia2DecompFrameRangeQuiet(0x83f9d4u, 0x83fa12u) ||
        !Lufia2DecompFrameRangeQuiet(0x83fb12u, 0x83fb2eu) ||
        !Lufia2DecompFrameRangeQuiet(0x83fb71u, 0x83fb8bu) ||
        !PlanVehicleTerrain(cpu->ram, &plan) ||
        plan.count + 2u > LUFIA2_FIELD_VEHICLE_OPCODE_BUDGET ||
        !VehicleTerrainWindowSupported(cpu, FieldMapClockReserve(plan.opcodes, plan.count), true)) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
        return ActorBridgeFallback(cpu, &frame, 0x8ebbafu);
    }
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0x83c202u};
    VehicleTerrainClock clock = {.span = {.cpu = cpu}, .plan = &plan, .paired = true};
    const Lufia2Memory memory = {VehicleTerrainRead, VehicleTerrainWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2FieldProbeVehicleTerrain(
        &memory, &state, VehicleTerrainChild, (void *)&memory);
    PrepareVehicleTerrainClock(&clock);
    assert(clock.offset == plan.count && !clock.span.remaining && result.pc == plan.landing);
    ActorBridgeStore(cpu, &state);
    /* The original map children finish through their paired JSR ABI. */
    cpu->host_return_valid = 2u;
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc,
            frame.entry_s, frame.hrv);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x8ebbd0u);
    bool progress = false;
    const RecompReturn returned = ReturnTimedLongCallWithProgress(
        cpu, &frame, result.pc, &progress);
    Lufia2DecompFramePairedAccountSpan(cpu, 0x8ebbafu, 1u, progress);
    return returned;
}
