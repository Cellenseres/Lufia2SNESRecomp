enum { EQUIPMENT_CLOCK_CAPACITY = 48u };

typedef struct EquipmentCallClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[EQUIPMENT_CLOCK_CAPACITY];
    ActorPushedCall call;
    uint32_t entry;
    uint32_t next;
    bool returning;
} EquipmentCallClock;

static void EquipmentClockOpcode(EquipmentCallClock *clock, uint16_t pc,
                                 unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < EQUIPMENT_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static void EquipmentClockNearCall(EquipmentCallClock *clock, uint16_t pc) {
    EquipmentClockOpcode(clock, pc, 3u, 6u, 2u);
}

static void EquipmentClockLongCall(EquipmentCallClock *clock, uint16_t pc) {
    EquipmentClockOpcode(clock, pc, 4u, 8u, 3u);
}

static void EquipmentClockJump(EquipmentCallClock *clock, uint16_t pc) {
    EquipmentClockOpcode(clock, pc, 3u, 3u, 0u);
}

static bool EquipmentClockRegistersSupported(const CpuState *cpu) {
    return cpu->ram && cpu->PB == 0x82u && !cpu->emulation && cpu->m_flag <= 1u &&
        !cpu->x_flag && !cpu->D && !cpu->_flag_D &&
        (cpu->DB == 0x7eu || (cpu->DB & 0x7fu) < 0x40u) &&
        cpu->S >= 0x1f20u && cpu->S <= 0x1ffcu;
}

static bool EquipmentClockWindowSupported(EquipmentCallClock *clock) {
    CpuState *cpu = clock->call.cpu;
    if (!EquipmentClockRegistersSupported(cpu) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return false;
    unsigned budget = clock->returning
        ? cpu_region_speed(clock->next) + 2u * 8u + 3u * 6u : 0u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(0x820000u | op->pc) +
            op->data_bytes * 8u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (!budget || cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x820000u, 0x1ffcu, true,
                                             1363u, budget))
        return false;
    if (g_snes->vPos < 225u) {
        const unsigned until_vblank =
            (225u - g_snes->vPos) * 1364u - g_snes->hPos;
        if (!g_snes->hdmaBeamOff || until_vblank < budget)
            return false;
    } else if (g_snes->vPos >= 262u ||
               (262u - g_snes->vPos) * 1364u - g_snes->hPos <= budget) {
        return false;
    }
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool PrepareEquipmentScrollClock(EquipmentCallClock *clock, uint16_t pc) {
    CpuState *cpu = clock->call.cpu;
    switch (pc) {
    case 0xbd02u: {
        const bool scrolling =
            ((uint8_t)(cpu->ram[0x14b5u] - cpu->ram[0x14b7u]) & 0x80u) != 0u;
        EquipmentClockOpcode(clock, 0xbd02u, 3u, 4u, 1u);
        EquipmentClockOpcode(clock, 0xbd05u, 3u, 4u, 1u);
        EquipmentClockOpcode(clock, 0xbd08u, 2u, scrolling ? 3u : 2u, 0u);
        if (scrolling) {
            EquipmentClockOpcode(clock, 0xbd0du, 3u, 4u, 1u);
            EquipmentClockOpcode(clock, 0xbd10u, 1u, 2u, 0u);
            EquipmentClockOpcode(clock, 0xbd11u, 2u, 2u, 0u);
            EquipmentClockOpcode(clock, 0xbd13u, 2u, 3u, 1u);
            EquipmentClockOpcode(clock, 0xbd15u, 2u, 3u, 1u);
            EquipmentClockNearCall(clock, 0xbd17u);
        } else {
            EquipmentClockJump(clock, 0xbd0au);
            EquipmentClockOpcode(clock, 0xbd45u, 2u, 3u, 0u);
            clock->returning = true;
            clock->next = 0x82bd47u;
        }
        break;
    }
    case 0xbd1au:
        EquipmentClockOpcode(clock, 0xbd1au, 3u, 6u, 2u);
        EquipmentClockOpcode(clock, 0xbd1du, 2u, 2u, 0u);
        EquipmentClockOpcode(clock, 0xbd1fu, 3u, 4u, 1u);
        EquipmentClockLongCall(clock, 0xbd22u);
        break;
    case 0xbd26u:
    case 0xbd2fu:
        EquipmentClockOpcode(clock, pc, 2u, 2u, 0u);
        EquipmentClockOpcode(clock, pc + 2u, 3u, 4u, 1u);
        EquipmentClockLongCall(clock, pc + 5u);
        break;
    case 0xbd38u:
        EquipmentClockNearCall(clock, 0xbd38u);
        break;
    case 0xbd3bu:
        EquipmentClockOpcode(clock, 0xbd3bu, 2u, 2u, 0u);
        EquipmentClockOpcode(clock, 0xbd3du, 3u, 4u, 1u);
        EquipmentClockOpcode(clock, 0xbd40u, 2u, 3u, 0u);
        EquipmentClockOpcode(clock, 0xbd42u, 3u, 8u, 4u);
        EquipmentClockOpcode(clock, 0xbd45u, 2u, 3u, 0u);
        clock->returning = true;
        clock->next = 0x82bd47u;
        break;
    default:
        return false;
    }
    return true;
}

#include "equipment_picker_clock.inc"

static bool PrepareEquipmentCallClock(EquipmentCallClock *clock, uint32_t pc) {
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x820000u;
    clock->span.timing = clock->timing;
    clock->next = pc;
    clock->returning = false;
    if (!EquipmentClockRegistersSupported(clock->call.cpu))
        return false;
    const bool planned = clock->entry == 0x82bd02u
        ? PrepareEquipmentScrollClock(clock, (uint16_t)pc)
        : PrepareEquipmentPickerClock(clock, (uint16_t)pc);
    return planned && EquipmentClockWindowSupported(clock);
}

static uint8_t EquipmentClockChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    EquipmentCallClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    assert(clock->span.count &&
        clock->timing[clock->span.count - 1u].pc == (uint16_t)site);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareEquipmentCallClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing,
        clock->call.frame.entry_s, clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeTimedEquipmentCallAt(
    CpuState *cpu, uint32_t entry, uint32_t landing, ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    EquipmentCallClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    clock.entry = entry;
    if (!cpu->m_flag || cpu->host_return_valid != 2u ||
        !PrepareEquipmentCallClock(&clock, landing))
        return ActorBridgeFallback(cpu, &frame, landing);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result;
    if (run)
        result = run(&memory, &state, EquipmentClockChild, &clock);
    else if (entry == 0x82bd02u)
        result = Lufia2MenuResumeEquipmentScrollDown(
            &memory, &state, landing, EquipmentClockChild, &clock);
    else
        result = Lufia2MenuResumeEquipmentChoice(
            &memory, &state, landing, EquipmentClockChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.returning && clock.next == result.pc &&
        clock.span.index == clock.span.count && !clock.span.remaining);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}

static RecompReturn ActorBridgeTimedEquipmentCall(CpuState *cpu, uint32_t entry,
                                                  ActorSceneOwnersFunction run) {
    return ActorBridgeTimedEquipmentCallAt(cpu, entry, entry, run);
}


uint32_t Lufia2DecompEquipmentResumeReturn(CpuState *cpu, uint32_t landing) {
    const uint32_t owner = Lufia2EquipmentContinuationOwner(landing);
    if (!owner || !cpu->m_flag || !EquipmentClockRegistersSupported(cpu) ||
        !cpu_dispatch_has_entry(cpu, owner))
        return 0u;
    EquipmentCallClock clock = {0};
    clock.call.cpu = cpu;
    clock.entry = owner;
    if (!PrepareEquipmentCallClock(&clock, landing))
        return 0u;
    const uint16_t target = (uint16_t)((cpu->ram[cpu->S + 1u] |
        (uint16_t)cpu->ram[cpu->S + 2u] << 8) + 1u);
    return target >= 0x8000u ? 0x820000u | target : 0u;
}

RecompReturn Lufia2DecompEquipmentResume(CpuState *cpu, uint32_t landing) {
    cpu->host_return_valid = 2u;
    return ActorBridgeTimedEquipmentCallAt(cpu, Lufia2EquipmentContinuationOwner(landing), landing, NULL);
}
