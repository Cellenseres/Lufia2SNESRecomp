enum { EVENT_REGION_CLOCK_CAPACITY = 512u };

typedef struct EventRegionClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[EVENT_REGION_CLOCK_CAPACITY];
    ActorPushedCall call;
    bool complete;
} EventRegionClock;

static void EventRegionOpcode(EventRegionClock *clock, unsigned pc,
                                unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < EVENT_REGION_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){(uint16_t)pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static bool EventRegionData(const CpuState *cpu, unsigned bank,
                              unsigned address, uint8_t *value) {
    bank = (bank + (address >> 16)) & 0xffu;
    address &= 0xffffu;
    if (bank == 0x7eu || bank == 0x7fu) {
        *value = cpu->ram[((bank & 1u) << 16) | address];
        return true;
    }
    if (!(bank < 0x40u || (bank >= 0x80u && bank < 0xc0u)))
        return false;
    if (address < 0x2000u) {
        *value = cpu->ram[address];
        return true;
    }
    if (address < 0x8000u)
        return false;
    const unsigned offset = (bank & 0x7fu) * 0x8000u + (address & 0x7fffu);
    if (offset >= g_snes->cart->romSize)
        return false;
    *value = g_snes->cart->rom[offset];
    return true;
}

static bool EventRegionWindow(const EventRegionClock *clock) {
    CpuState *cpu = clock->call.cpu;
    if (cpu->coprocessor_master_cycles != cpu->master_cycles)
        return false;
    unsigned budget = 0u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(0x800000u | op->pc) +
            op->data_bytes * 8u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (!budget || cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x800000u, 0x1ffcu, true, 1363u, budget))
        return false;
    if (g_snes->vPos < 225u) {
        if (!g_snes->hdmaBeamOff ||
            (225u - g_snes->vPos) * 1364u - g_snes->hPos <= budget)
            return false;
    } else if (g_snes->vPos >= 262u ||
               (262u - g_snes->vPos) * 1364u - g_snes->hPos <= budget) {
        return false;
    }
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool PrepareEventRegionClock(EventRegionClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x800000u;
    clock->span.timing = clock->timing;
    clock->complete = false;
    const bool root = pc == 0x80d5f2u || pc == 0x80e50bu;
    if (!cpu->ram || !g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || cpu->PB != 0x80u ||
        cpu->emulation || cpu->m_flag != 1u || cpu->x_flag || cpu->D || cpu->_flag_D ||
        cpu->S < (root ? 0x1f20u : 0x1f10u) || cpu->S > 0x1ffcu)
        return false;
    if (root) {
        uint8_t value;
        const EventScriptCursor cursor = {cpu->Y, cpu->DB, cpu->ram[0x1d199u]};
        if (!EventScriptByte(cpu, &cursor, &value))
            return false;
    }
    switch (pc) {
    case 0x80d5f2u:
    case 0x80e50bu:
        EventRegionOpcode(clock, (uint16_t)pc, 3u, 6u, 2u);
        break;
    case 0x80d5f5u: {
            EventRegionOpcode(clock, 0xd5f5u, 2u, 3u, 1u);
            EventRegionOpcode(clock, 0xd5f7u, 1u, 3u, 1u);
            EventRegionOpcode(clock, 0xd5f8u, 1u, 4u, 2u);
            EventRegionOpcode(clock, 0xd5f9u, 2u, 2u, 0u);
            EventRegionOpcode(clock, 0xd5fbu, 1u, 3u, 1u);
            EventRegionOpcode(clock, 0xd5fcu, 1u, 4u, 1u);
            EventRegionOpcode(clock, 0xd5fdu, 4u, 5u, 1u);
            EventRegionOpcode(clock, 0xd601u, 1u, 3u, 0u);
            EventRegionOpcode(clock, 0xd602u, 4u, 5u, 1u);
            EventRegionOpcode(clock, 0xd606u, 1u, 2u, 0u);
        uint16_t position = cpu->ram[0x1d1a0u] | (uint16_t)cpu->ram[0x1d1a1u] << 8;
        bool found = false;
        for (unsigned visited = 0u; visited < 32u; ++visited) {
            const unsigned address = 0xb8b5u + position;
            uint8_t value;
            if (!EventRegionData(cpu, 0x91u, address, &value))
                return false;
                EventRegionOpcode(clock, 0xd607u, 1u, 2u, 0u);
                EventRegionOpcode(clock, 0xd608u, 3u, 4u, 1u);
                EventRegionOpcode(clock, 0xd60bu, 2u, 2u, 0u);
            if ((address & ~0xffu) != 0xb800u)
                ++clock->timing[clock->span.count - 2u].cycles;
                EventRegionOpcode(clock, 0xd60du, 2u, value == 0xffu ? 3u : 2u, 0u);
            if (value == 0xffu) {
                    EventRegionOpcode(clock, 0xd61au, 2u, 3u, 1u);
                    EventRegionOpcode(clock, 0xd61cu, 4u, 5u, 1u);
                    EventRegionOpcode(clock, 0xd620u, 3u, 3u, 0u);
                    EventRegionOpcode(clock, 0xd623u, 4u, 8u, 3u);
                found = true;
                break;
            }
                EventRegionOpcode(clock, 0xd60fu, 2u, 2u, 0u);
                EventRegionOpcode(clock, 0xd611u, 2u, 3u, 1u);
            const bool match = (value & 31u) == (uint8_t)cpu->A;
                EventRegionOpcode(clock, 0xd613u, 2u, match ? 3u : 2u, 0u);
            if (match) {
                    EventRegionOpcode(clock, 0xd629u, 3u, 4u, 1u);
                    EventRegionOpcode(clock, 0xd62cu, 2u, 2u, 0u);
                    EventRegionOpcode(clock, 0xd62eu, 4u, 5u, 1u);
                    EventRegionOpcode(clock, 0xd632u, 4u, 8u, 3u);
                if ((address & ~0xffu) != 0xb800u)
                    ++clock->timing[clock->span.count - 4u].cycles;
                found = true;
                break;
            }
                EventRegionOpcode(clock, 0xd615u, 1u, 2u, 0u);
                EventRegionOpcode(clock, 0xd616u, 1u, 2u, 0u);
                EventRegionOpcode(clock, 0xd617u, 1u, 2u, 0u);
                EventRegionOpcode(clock, 0xd618u, 2u, 3u, 0u);
            position = (uint16_t)(position + 3u);
        }
        if (!found)
            return false;
        break;
    }
    case 0x80d627u:
        EventRegionOpcode(clock, 0xd627u, 2u, 3u, 0u);
        /* fall through */
    case 0x80d681u:
        EventRegionOpcode(clock, 0xd681u, 1u, 5u, 2u);
        EventRegionOpcode(clock, 0xd682u, 1u, 4u, 1u);
        EventRegionOpcode(clock, 0xd683u, 3u, 4u, 0u);
        clock->complete = true;
        break;
    case 0x80d636u:
        EventRegionOpcode(clock, 0xd636u, 4u, 8u, 3u);
        break;
    case 0x80d63au:
        EventRegionOpcode(clock, 0xd63au, 2u, 4u, 2u);
        EventRegionOpcode(clock, 0xd63cu, 1u, 2u, 0u);
        EventRegionOpcode(clock, 0xd63du, 4u, 8u, 3u);
        break;
    case 0x80d641u:
        EventRegionOpcode(clock, 0xd641u, 4u, 8u, 3u);
        break;
    case 0x80d645u: {
        uint8_t value;
        const unsigned address = 0x093bu + cpu->X;
        if (!EventRegionData(cpu, cpu->DB, address, &value))
            return false;
            EventRegionOpcode(clock, 0xd645u, 3u, 4u, 1u);
        if ((address & ~0xffu) != 0x0900u)
            ++clock->timing[clock->span.count - 1u].cycles;
        const bool empty = ((uint8_t)cpu->A & value) == 0u;
            EventRegionOpcode(clock, 0xd648u, 2u, empty ? 3u : 2u, 0u);
        if (empty) {
                EventRegionOpcode(clock, 0xd660u, 2u, 4u, 2u);
                EventRegionOpcode(clock, 0xd662u, 2u, 3u, 0u);
                EventRegionOpcode(clock, 0xd664u, 4u, 6u, 2u);
                EventRegionOpcode(clock, 0xd668u, 4u, 6u, 2u);
                EventRegionOpcode(clock, 0xd66cu, 4u, 6u, 2u);
                EventRegionOpcode(clock, 0xd670u, 4u, 6u, 2u);
        } else {
                EventRegionOpcode(clock, 0xd64au, 2u, 4u, 2u);
                EventRegionOpcode(clock, 0xd64cu, 2u, 3u, 0u);
                EventRegionOpcode(clock, 0xd64eu, 4u, 6u, 2u);
                EventRegionOpcode(clock, 0xd652u, 4u, 6u, 2u);
                EventRegionOpcode(clock, 0xd656u, 4u, 6u, 2u);
                EventRegionOpcode(clock, 0xd65au, 4u, 6u, 2u);
                EventRegionOpcode(clock, 0xd65eu, 2u, 3u, 0u);
        }
            EventRegionOpcode(clock, 0xd674u, 2u, 3u, 0u);
            EventRegionOpcode(clock, 0xd676u, 4u, 5u, 1u);
            EventRegionOpcode(clock, 0xd67au, 4u, 5u, 1u);
            EventRegionOpcode(clock, 0xd67eu, 3u, 6u, 2u);
        break;
    }
    case 0x80e50eu:
        EventRegionOpcode(clock, 0xe50eu, 1u, 3u, 1u);
        EventRegionOpcode(clock, 0xe50fu, 1u, 4u, 2u);
        EventRegionOpcode(clock, 0xe510u, 3u, 6u, 2u);
        break;
    case 0x80e513u:
        EventRegionOpcode(clock, 0xe513u, 1u, 3u, 0u);
        EventRegionOpcode(clock, 0xe514u, 2u, 2u, 0u);
        EventRegionOpcode(clock, 0xe516u, 1u, 3u, 0u);
        EventRegionOpcode(clock, 0xe517u, 3u, 3u, 0u);
        EventRegionOpcode(clock, 0xe51au, 4u, 8u, 3u);
        break;
    case 0x80e51eu:
        EventRegionOpcode(clock, 0xe51eu, 4u, 8u, 3u);
        break;
    case 0x80e522u:
        EventRegionOpcode(clock, 0xe522u, 3u, 6u, 2u);
        break;
    case 0x80e525u:
        EventRegionOpcode(clock, 0xe525u, 1u, 5u, 2u);
        EventRegionOpcode(clock, 0xe526u, 1u, 4u, 1u);
        EventRegionOpcode(clock, 0xe527u, 3u, 4u, 0u);
        clock->complete = true;
        break;
    default:
        return false;
    }
    return EventRegionWindow(clock);
}

static uint8_t EventRegionChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    EventRegionClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareEventRegionClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing,
        clock->call.frame.entry_s, clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeTimedEventRegion(CpuState *cpu, uint32_t entry,
                                              ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    EventRegionClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (!PrepareEventRegionClock(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, EventRegionChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.pc == 0x80cc35u) {
        BeginFixedSpanOpcode(&clock.span);
        assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && clock.complete &&
            clock.span.index == clock.span.count && !clock.span.remaining);
    }
    const uint32_t source = clock.complete
        ? 0x800000u | clock.timing[clock.span.count - 1u].pc : result.pc;
    return interp_tier_dispatch_tail(cpu, result.pc, source, frame.entry_s, frame.hrv);
}
