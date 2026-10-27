enum { MEMBER_SCREEN_TIMING_CAPACITY = 64u };

typedef struct MemberScreenClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[MEMBER_SCREEN_TIMING_CAPACITY];
    uint64_t master_bound;
    uint32_t next;
} MemberScreenClock;

#include "member_screen_timing.inc"

static bool MemberScreenOpcode(MemberScreenClock *clock, FixedSpanOpcode timing) {
    if (clock->span.count == MEMBER_SCREEN_TIMING_CAPACITY)
        return false;
    clock->timing[clock->span.count++] = timing;
    clock->master_bound += timing.fetched *
        cpu_region_speed(0x820000u | timing.pc) + timing.data_bytes * 8u +
        (timing.cycles - timing.fetched - timing.data_bytes) * 6u;
    return true;
}

static bool MemberScreenSegment(MemberScreenClock *clock, uint16_t pc) {
    unsigned first = 0u, end = sizeof member_timing_segments /
        sizeof member_timing_segments[0];
    while (first < end) {
        const unsigned middle = first + (end - first) / 2u;
        const MemberTimingSegment *segment = member_timing_segments + middle;
        if (segment->pc < pc) first = middle + 1u;
        else if (segment->pc > pc) end = middle;
        else {
            for (unsigned index = 0u; index < segment->count; ++index)
                if (!MemberScreenOpcode(clock, segment->timing[index]))
                    return false;
            return true;
        }
    }
    return false;
}

static bool MemberScreenBranch(MemberScreenClock *clock, uint16_t pc, bool taken) {
    return MemberScreenOpcode(clock,
        (FixedSpanOpcode){pc, 2u, taken ? 3u : 2u, 0u});
}

static unsigned MemberScreenRamBase(const CpuState *cpu) {
    return cpu->DB == 0x7fu ? 0x10000u : 0u;
}

static unsigned MemberScreenSelected(const CpuState *cpu) {
    const unsigned base = MemberScreenRamBase(cpu);
    return cpu->ram[base + 0x14b3u] | (unsigned)cpu->ram[base + 0x14b4u] << 8;
}

static bool PlanSpellMemberChoice(MemberScreenClock *clock) {
    const CpuState *cpu = clock->span.cpu;
    if (!MemberScreenBranch(clock, 0xb37bu, cpu->_flag_C != 0u))
        return false;
    if (cpu->_flag_C) {
        clock->next = 0x82b3acu;
        return true;
    }
    const unsigned selected = MemberScreenSelected(cpu);
    const unsigned base = MemberScreenRamBase(cpu);
    if (selected >= 4u)
        return false;
    const unsigned address = 0x0a80u + selected * 2u;
    const unsigned record = cpu->ram[base + address] |
        (unsigned)cpu->ram[base + address + 1u] << 8;
    if (record > 0x1ff0u)
        return false;
    const bool available = !(cpu->ram[base + record + 15u] & 4u);
    if (!MemberScreenSegment(clock, 0xb37du) ||
        !MemberScreenBranch(clock, 0xb38fu, available))
        return false;
    clock->next = available ? 0x82b3a6u : 0x82b394u;
    return MemberScreenSegment(clock, available ? 0xb396u : 0xb391u);
}

static bool PlanStatusInput(MemberScreenClock *clock) {
    const unsigned button = clock->span.cpu->A & 0xffu;
    if (!MemberScreenSegment(clock, 0xb31au) ||
        !MemberScreenBranch(clock, 0xb31cu, button == 2u))
        return false;
    if (button == 2u) {
        clock->next = 0x82b343u;
        return MemberScreenSegment(clock, 0xb33du);
    }
    if (!MemberScreenSegment(clock, 0xb31eu) ||
        !MemberScreenBranch(clock, 0xb320u, button == 3u))
        return false;
    if (button == 3u) {
        clock->next = 0x82b336u;
        return MemberScreenSegment(clock, 0xb332u);
    }
    if (!MemberScreenSegment(clock, 0xb322u) ||
        !MemberScreenBranch(clock, 0xb324u, button == 6u))
        return false;
    if (button != 6u && (!MemberScreenSegment(clock, 0xb326u) ||
        !MemberScreenBranch(clock, 0xb328u, button == 7u)))
        return false;
    if (button == 6u || button == 7u) {
        clock->next = 0x82b364u;
        return MemberScreenSegment(clock, 0xb35eu);
    }
    clock->next = 0x82b31au;
    return MemberScreenSegment(clock, 0xb32au) &&
        MemberScreenSegment(clock, 0xb317u);
}

static bool PlanMemberScreen(MemberScreenClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->span.cpu;
    if (pc >= 0xb2feu && cpu->m_flag != 1u)
        return false;
    if (pc == 0xb37bu)
        return PlanSpellMemberChoice(clock);
    if (pc == 0xb31au)
        return PlanStatusInput(clock);
    if (pc == 0xb309u) {
        clock->next = cpu->_flag_C ? 0x82b33cu : 0x82b30fu;
        return MemberScreenBranch(clock, pc, cpu->_flag_C != 0u) &&
            (cpu->_flag_C || MemberScreenSegment(clock, 0xb30bu));
    }
    if (pc == 0xb34fu) {
        const bool finished = cpu->ram[MemberScreenRamBase(cpu) + 0x14b3u] != 0u;
        clock->next = finished ? 0x82b336u : 0x82b35cu;
        return MemberScreenSegment(clock, pc) &&
            MemberScreenBranch(clock, 0xb352u, finished) &&
            MemberScreenSegment(clock, finished ? 0xb332u : 0xb354u);
    }
    if (pc == 0xb315u || pc == 0xb369u) {
        clock->next = 0x82b31au;
        return MemberScreenSegment(clock, pc) &&
            MemberScreenSegment(clock, pc == 0xb315u ? 0xb32au : 0xb32eu) &&
            MemberScreenSegment(clock, 0xb317u);
    }
    if (pc == 0xb394u) {
        clock->next = 0x82b37bu;
        return MemberScreenSegment(clock, pc) && MemberScreenSegment(clock, 0xb373u);
    }
    typedef struct MemberPhase { uint16_t pc, next; uint8_t byte_mode; } MemberPhase;
    static const MemberPhase phases[] = {
        {0x9e67u,0x9e74u,1u}, {0x9e74u,0x9e7fu,1u},
        {0x9e7fu,0x9e88u,0u}, {0x9e88u,0x9e92u,0u},
        {0x9e92u,0x9e97u,1u}, {0x9e97u,0x9ed6u,1u},
        {0x9ed6u,0x9ee9u,1u}, {0x9ee9u,0x9eeeu,1u},
        {0x9eeeu,0x9ef1u,1u}, {0x9ef1u,0x9ef1u,1u},
        {0xb2feu,0xb301u,1u}, {0xb301u,0xb309u,1u},
        {0xb30fu,0xb312u,1u}, {0xb312u,0xb315u,1u},
        {0xb336u,0xb339u,1u}, {0xb339u,0xb33cu,1u},
        {0xb33cu,0xb33cu,1u}, {0xb343u,0xb34bu,1u},
        {0xb34bu,0xb34fu,1u}, {0xb35cu,0xb364u,1u},
        {0xb364u,0xb369u,1u}, {0xb370u,0xb373u,1u},
        {0xb373u,0xb37bu,1u}, {0xb3a6u,0xb3a9u,1u},
        {0xb3a9u,0xb3acu,1u}, {0xb3acu,0xb3acu,1u}
    };
    unsigned first = 0u, end = sizeof phases / sizeof phases[0];
    while (first < end) {
        const unsigned middle = first + (end - first) / 2u;
        const MemberPhase *phase = phases + middle;
        if (phase->pc < pc) first = middle + 1u;
        else if (phase->pc > pc) end = middle;
        else {
            if (cpu->m_flag != phase->byte_mode ||
                (pc == 0x9e97u && MemberScreenSelected(cpu) >= 4u))
                return false;
            clock->next = 0x820000u | phase->next;
            return pc == phase->next || MemberScreenSegment(clock, pc);
        }
    }
    return false;
}

static bool PrepareMemberScreenClockMode(MemberScreenClock *clock, CpuState *cpu,
                                          uint32_t entry, bool prefix) {
    const bool status_builder = entry >= 0x829e67u && entry < 0x829ef2u;
    if (!status_builder && (entry < 0x82b2feu || entry >= 0x82b3adu))
        return false;
    if (cpu->PB != 0x82u || cpu->D || cpu->_flag_D || cpu->x_flag ||
        cpu->m_flag > 1u ||
        (cpu->host_return_valid != 2u && cpu->host_return_valid != 3u &&
         (!prefix || cpu->host_return_valid != 0u)) ||
        (cpu->DB != 0x83u && cpu->DB != 0x7eu && cpu->DB != 0x7fu) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ff7u, true, 1363u, 64u) ||
        !g_snes->cart || g_snes->cart->type != CART_LOROM ||
        !g_snes->cart->rom || g_snes->cart->romSize < 0x280000u ||
        g_snes->vPos >= 262u)
        return false;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x820000u;
    clock->span.timing = clock->timing;
    clock->master_bound = 0u;
    if (!PlanMemberScreen(clock, (uint16_t)entry))
        return false;
    const bool returning = clock->next == 0x829ef1u ||
        clock->next == 0x82b33cu || clock->next == 0x82b3acu;
    const FixedSpanOpcode *last = clock->span.count ?
        &clock->timing[clock->span.count - 1u] : NULL;
    const bool child_call = last &&
        ((last->fetched == 3u && last->cycles == 6u && last->data_bytes == 2u) ||
         (last->fetched == 4u && last->cycles == 8u && last->data_bytes == 3u));
    if (prefix && child_call) {
        const FixedSpanOpcode *call = &clock->timing[--clock->span.count];
        clock->master_bound -= call->fetched * cpu_region_speed(0x820000u | call->pc) +
            call->data_bytes * 8u + (call->cycles - call->fetched - call->data_bytes) * 6u;
        clock->next = 0x820000u | call->pc;
    }
    if (prefix && !clock->span.count)
        return false;
    for (unsigned index = 0u; index < clock->span.count; ++index) {
        const FixedSpanOpcode *opcode = &clock->timing[index];
        const uint32_t pc = 0x820000u | opcode->pc;
        if (!Lufia2DecompFrameRangeQuiet(pc, pc + opcode->fetched) &&
            (pc != entry || Lufia2DecompFrameSpanBudget(pc,
                pc + opcode->fetched, ActorBridgeMemberScreenPhase) < clock->span.count))
            return false;
    }
    if (prefix && Lufia2DecompFrameSpanBudget(entry,
            entry + clock->timing[0].fetched, ActorBridgeMemberScreenPhase) < clock->span.count)
        return false;
    const uint64_t work = clock->master_bound + (!prefix && returning ? 64u : 0u);
    const uint64_t bound = work + 40u * (work / 1324u + 1u);
    if (cpu->master_cycles > UINT64_MAX - bound ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ff7u, true, 1363u, bound))
        return false;
    if (!g_snes->hdmaBeamOff && g_snes->vPos < 225u) {
        const unsigned fence = g_snes->hPos < 1024u ? 1024u : 1364u;
        if (bound >= fence - g_snes->hPos)
            return false;
    }
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if ((uint64_t)lines * 1364u - g_snes->hPos <= bound)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool PrepareMemberScreenClock(MemberScreenClock *clock, CpuState *cpu,
                                      uint32_t entry) {
    return PrepareMemberScreenClockMode(clock, cpu, entry, false);
}
