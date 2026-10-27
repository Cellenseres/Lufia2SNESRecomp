enum { PANEL_INPUT_TIMING_CAPACITY = 512u };

typedef struct PanelInputClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[PANEL_INPUT_TIMING_CAPACITY];
    uint64_t master_bound;
    uint32_t next;
    bool stop_before_input;
} PanelInputClock;

static bool PanelInputOpcode(PanelInputClock *clock, FixedSpanOpcode timing) {
    if (clock->span.count == PANEL_INPUT_TIMING_CAPACITY)
        return false;
    clock->timing[clock->span.count++] = timing;
    clock->master_bound += timing.fetched *
        cpu_region_speed(0x840000u | timing.pc) + timing.data_bytes * 12u +
        (timing.cycles - timing.fetched - timing.data_bytes) * 6u;
    return true;
}

static bool PanelInputSequence(PanelInputClock *clock,
    const FixedSpanOpcode *timing, unsigned count) {
    for (unsigned index = 0u; index < count; ++index)
        if (!PanelInputOpcode(clock, timing[index]))
            return false;
    return true;
}

static bool PanelInputBranch(PanelInputClock *clock, uint16_t pc, bool taken) {
    return PanelInputOpcode(clock, (FixedSpanOpcode){pc, 2u, taken ? 3u : 2u, 0u});
}

static bool PlanPanelCommandAppend(PanelInputClock *clock) {
    static const FixedSpanOpcode begin[] = {
        {0xef88u, 2u, 3u, 0u}, {0xef8au, 3u, 3u, 0u}
    };
    static const FixedSpanOpcode reset[] = {
        {0xef94u, 3u, 5u, 2u}, {0xef97u, 2u, 3u, 0u}
    };
    static const FixedSpanOpcode count[] = {
        {0xef99u, 3u, 5u, 2u}, {0xef9cu, 3u, 3u, 0u}
    };
    static const FixedSpanOpcode grow[] = {
        {0xefa1u, 1u, 2u, 0u}, {0xefa2u, 1u, 2u, 0u},
        {0xefa3u, 3u, 5u, 2u}, {0xefa6u, 2u, 3u, 0u}
    };
    static const FixedSpanOpcode initialize[] = {
        {0xefa8u, 3u, 3u, 0u}, {0xefabu, 3u, 3u, 0u}
    };
    static const FixedSpanOpcode shift[] = {
        {0xefaeu, 3u, 5u, 2u}, {0xefb1u, 3u, 7u, 2u},
        {0xefb4u, 1u, 2u, 0u}, {0xefb5u, 1u, 2u, 0u},
        {0xefb6u, 1u, 2u, 0u}, {0xefb7u, 1u, 2u, 0u},
        {0xefb8u, 3u, 3u, 0u}
    };
    static const FixedSpanOpcode append[] = {
        {0xefbdu, 3u, 5u, 2u}, {0xefc0u, 3u, 7u, 2u}
    };
    const CpuState *cpu = clock->span.cpu;
    const unsigned command = cpu->A;
    clock->next = 0x84efc3u;
    if (!PanelInputSequence(clock, begin, 2u) ||
        !PanelInputBranch(clock, 0xef8du, command == 0u))
        return false;
    if (!command)
        return true;
    if (!PanelInputOpcode(clock, (FixedSpanOpcode){0xef8fu, 3u, 3u, 0u}) ||
        !PanelInputBranch(clock, 0xef92u, command != 8u))
        return false;
    if (command == 8u)
        return PanelInputSequence(clock, reset, 2u);
    const unsigned base = ((unsigned)cpu->DB & 1u) << 16;
    const unsigned used = cpu->ram[base + 0x71c0u] |
        (unsigned)cpu->ram[base + 0x71c1u] << 8;
    if (used > 28u || (used & 1u) ||
        !PanelInputSequence(clock, count, 2u) ||
        !PanelInputBranch(clock, 0xef9fu, used == 28u))
        return false;
    if (used != 28u) {
        if (!PanelInputSequence(clock, grow, 4u))
            return false;
    } else {
        if (!PanelInputSequence(clock, initialize, 2u))
            return false;
        for (unsigned word = 0u; word < 14u; ++word)
            if (!PanelInputSequence(clock, shift, 7u) ||
                !PanelInputBranch(clock, 0xefbbu, word != 13u))
                return false;
    }
    return PanelInputSequence(clock, append, 2u);
}

static bool PlanPanelResource(PanelInputClock *clock) {
    static const FixedSpanOpcode setup[] = {
        {0xef4eu, 2u, 3u, 0u}, {0xef50u, 2u, 4u, 2u},
        {0xef52u, 3u, 3u, 0u}, {0xef55u, 2u, 4u, 2u},
        {0xef57u, 2u, 2u, 0u}, {0xef59u, 2u, 3u, 1u},
        {0xef5bu, 4u, 8u, 3u}
    };
    clock->next = 0x84ef5fu;
    return PanelInputSequence(clock, setup, 7u);
}

static bool PanelSequenceWord(unsigned offset, unsigned *word) {
    if (offset > 0x72u || (offset & 1u))
        return false;
    const unsigned source = 0x27308u + offset;
    if (source + 1u >= g_snes->cart->romSize)
        return false;
    *word = g_snes->cart->rom[source] |
        (unsigned)g_snes->cart->rom[source + 1u] << 8;
    return true;
}

static bool PlanPanelSequenceResult(PanelInputClock *clock, unsigned sequence) {
    static const FixedSpanOpcode inspect[] = {
        {0xeff2u, 2u, 4u, 2u}, {0xeff4u, 3u, 3u, 0u}
    };
    static const FixedSpanOpcode flag[] = {
        {0xeff9u, 3u, 8u, 4u}, {0xeffcu, 2u, 3u, 0u}
    };
    static const FixedSpanOpcode result[] = {
        {0xeffeu, 1u, 2u, 0u}, {0xefffu, 1u, 2u, 0u},
        {0xf000u, 1u, 2u, 0u}, {0xf001u, 1u, 2u, 0u},
        {0xf002u, 4u, 6u, 2u}, {0xf006u, 3u, 5u, 2u},
        {0xf009u, 4u, 6u, 2u}, {0xf00du, 3u, 5u, 2u}
    };
    static const FixedSpanOpcode sound[] = {
        {0xf010u, 3u, 3u, 0u}, {0xf013u, 3u, 6u, 2u}
    };
    if (!PanelInputSequence(clock, inspect, 2u) ||
        !PanelInputBranch(clock, 0xeff7u, sequence != 4u) ||
        !PanelInputSequence(clock, sequence == 4u ? flag : result,
            sequence == 4u ? 2u : 8u))
        return false;
    clock->next = 0x84f016u;
    return PanelInputSequence(clock, sound, 2u);
}

static bool PlanPanelSequence(PanelInputClock *clock) {
    static const FixedSpanOpcode begin[] = {
        {0xefc4u, 2u, 3u, 0u}, {0xefc6u, 2u, 4u, 2u},
        {0xefc8u, 3u, 3u, 0u}, {0xefcbu, 3u, 3u, 0u}
    };
    static const FixedSpanOpcode next[] = {
        {0xefceu, 1u, 2u, 0u}, {0xefcfu, 1u, 2u, 0u},
        {0xefd0u, 1u, 2u, 0u}, {0xefd1u, 1u, 2u, 0u},
        {0xefd2u, 4u, 6u, 2u}
    };
    static const FixedSpanOpcode skip[] = {
        {0xefe0u, 1u, 2u, 0u}, {0xefe1u, 1u, 2u, 0u},
        {0xefe2u, 4u, 6u, 2u}
    };
    static const FixedSpanOpcode retry[] = {
        {0xefeeu, 2u, 7u, 4u}, {0xeff0u, 2u, 3u, 0u}
    };
    if (!PanelInputSequence(clock, begin, 4u))
        return false;
    const CpuState *cpu = clock->span.cpu;
    const unsigned base = ((unsigned)cpu->DB & 1u) << 16;
    unsigned offset = 0xfffeu, command = 0u, sequence = 0u;
    for (;;) {
        unsigned symbol;
        offset = (offset + 2u) & 0xffffu;
        command += 2u;
        if (command > 32u || !PanelSequenceWord(offset, &symbol) ||
            !PanelInputSequence(clock, next, 5u) ||
            !PanelInputBranch(clock, 0xefd6u, symbol == 0u))
            return false;
        if (!symbol)
            return PlanPanelSequenceResult(clock, sequence);
        const unsigned address = base + 0x71c0u + command;
        const unsigned value = cpu->ram[address] | (unsigned)cpu->ram[address + 1u] << 8;
        if (!PanelInputOpcode(clock, (FixedSpanOpcode){0xefd8u, 3u, 5u, 2u}) ||
            !PanelInputBranch(clock, 0xefdbu, symbol == value))
            return false;
        if (symbol == value)
            continue;
        command = 0u;
        if (!PanelInputOpcode(clock, (FixedSpanOpcode){0xefddu, 3u, 3u, 0u}))
            return false;
        do {
            offset += 2u;
            if (!PanelSequenceWord(offset, &symbol) ||
                !PanelInputSequence(clock, skip, 3u) ||
                !PanelInputBranch(clock, 0xefe6u, symbol != 0u))
                return false;
        } while (symbol);
        if (!PanelSequenceWord(offset + 2u, &symbol) ||
            !PanelInputOpcode(clock, (FixedSpanOpcode){0xefe8u, 4u, 6u, 2u}) ||
            !PanelInputBranch(clock, 0xefecu, (symbol & 0x8000u) != 0u))
            return false;
        if (symbol & 0x8000u) {
            clock->next = 0x84f019u;
            return true;
        }
        if (!PanelInputSequence(clock, retry, 2u))
            return false;
        ++sequence;
    }
}

#include "panel_phase_timing.inc"

static bool PanelInputSegment(PanelInputClock *clock, uint16_t pc) {
    unsigned first = 0u;
    unsigned end = sizeof panel_timing_segments / sizeof panel_timing_segments[0];
    while (first < end) {
        const unsigned middle = first + (end - first) / 2u;
        const PanelTimingSegment *segment = panel_timing_segments + middle;
        if (segment->pc < pc) first = middle + 1u;
        else if (segment->pc > pc) end = middle;
        else return PanelInputSequence(clock, segment->timing, segment->count);
    }
    return false;
}

static bool PlanPanelPalette(PanelInputClock *clock, bool restoring) {
    if (!PanelInputSegment(clock, restoring ? 0xeef1u : 0xee51u))
        return false;
    const uint16_t move = restoring ? 0xef01u : 0xee61u;
    for (unsigned byte = 0u; byte < 256u; ++byte)
        if (!PanelInputOpcode(clock, (FixedSpanOpcode){move, 3u, 7u, 2u}))
            return false;
    clock->next = restoring ? 0x84ef0eu : 0x84ee6eu;
    return PanelInputSegment(clock, restoring ? 0xef04u : 0xee64u);
}

static bool PlanPanelInputDecision(PanelInputClock *clock) {
    const CpuState *cpu = clock->span.cpu;
    const unsigned base = ((unsigned)cpu->DB & 1u) << 16;
    const unsigned command = cpu->ram[base + 0x71e0u] |
        (unsigned)cpu->ram[base + 0x71e1u] << 8;
    if (!PanelInputSegment(clock, 0xeeaau) ||
        !PanelInputBranch(clock, 0xeeb1u, command == 1u))
        return false;
    if (command == 1u) {
        const unsigned saved_x = cpu->ram[cpu->S + 1u] |
            (unsigned)cpu->ram[cpu->S + 2u] << 8;
        if (!PanelInputSegment(clock, 0xeebau) ||
            !PanelInputBranch(clock, 0xeebdu, saved_x != 0u))
            return false;
        clock->next = saved_x ? 0x84eedbu : 0x84eec5u;
        return PanelInputSegment(clock, saved_x ? 0xeed5u : 0xeebfu);
    }
    if (!PanelInputSegment(clock, 0xeeb3u) ||
        !PanelInputBranch(clock, 0xeeb6u, command == 2u))
        return false;
    if (command == 2u) {
        clock->next = 0x84eeeeu;
        return PanelInputSegment(clock, 0xeeebu);
    }
    clock->next = clock->stop_before_input ? 0x84ee9du : 0x84eea1u;
    return PanelInputSegment(clock, 0xeeb8u) &&
        (clock->stop_before_input || PanelInputSegment(clock, 0xee9du));
}

static bool PlanPanelParent(PanelInputClock *clock, uint16_t pc) {
    typedef struct PanelPhase {
        uint16_t pc, next;
        uint8_t byte_mode;
    } PanelPhase;
    static const PanelPhase phases[] = {
        {0xee4bu, 0xee51u, 2u}, {0xee51u, 0xee6eu, 0u},
        {0xee6eu, 0xee73u, 1u}, {0xee73u, 0xee79u, 0u},
        {0xee79u, 0xee7cu, 2u}, {0xee7cu, 0xee82u, 0u},
        {0xee82u, 0xee85u, 2u}, {0xee85u, 0xee8eu, 0u},
        {0xee8eu, 0xee95u, 1u}, {0xee95u, 0xee9au, 1u},
        {0xee9au, 0xeea1u, 0u}, {0xeea1u, 0xeea4u, 0u},
        {0xeea4u, 0xeeaau, 0u}, {0xeeaau, 0u, 0u},
        {0xeec5u, 0xeec8u, 2u}, {0xeec8u, 0xeea1u, 0u},
        {0xeedbu, 0xeedeu, 2u}, {0xeedeu, 0xeea1u, 0u},
        {0xeeeeu, 0xeef1u, 0u}, {0xeef1u, 0xef0eu, 0u},
        {0xef0eu, 0xef16u, 1u}, {0xef16u, 0xef19u, 2u},
        {0xef19u, 0xef1fu, 0u}, {0xef1fu, 0xef22u, 2u},
        {0xef22u, 0xef37u, 0u}, {0xef37u, 0xef40u, 2u},
        {0xef40u, 0xef47u, 1u}, {0xef47u, 0xef4cu, 1u},
        {0xef4cu, 0xef4du, 0u}
    };
    unsigned first = 0u, end = sizeof phases / sizeof phases[0];
    while (first < end) {
        const unsigned middle = first + (end - first) / 2u;
        const PanelPhase *phase = phases + middle;
        if (phase->pc < pc) first = middle + 1u;
        else if (phase->pc > pc) end = middle;
        else {
            if (phase->byte_mode < 2u &&
                phase->byte_mode != clock->span.cpu->m_flag)
                return false;
            if (pc == 0xee51u || pc == 0xeef1u)
                return PlanPanelPalette(clock, pc == 0xeef1u);
            if (pc == 0xeeaau)
                return PlanPanelInputDecision(clock);
            clock->next = 0x840000u | phase->next;
            if ((pc == 0xeec8u || pc == 0xeedeu) && clock->stop_before_input)
                clock->next = 0x84ee9du;
            return PanelInputSegment(clock, pc) &&
                ((pc != 0xeec8u && pc != 0xeedeu) ||
                    clock->stop_before_input || PanelInputSegment(clock, 0xee9du));
        }
    }
    return false;
}

static bool PreparePanelInputClock(PanelInputClock *clock, CpuState *cpu,
    uint32_t entry) {
    uint32_t end;
    switch (entry) {
    case 0x84ef4eu: end = 0x84ef60u; break;
    case 0x84ef5fu: end = 0x84ef60u; break;
    case 0x84ef88u: end = 0x84efc3u; break;
    case 0x84efc4u: end = 0x84f019u; break;
    case 0x84f016u: end = 0x84f019u; break;
    default:
        if (entry < 0x84ee4bu || entry >= 0x84ef4eu ||
            !Lufia2DecompFrameRangeQuiet(0x84ee4bu, 0x84ef00u))
            return false;
        end = 0x84ef4eu;
        break;
    }
    if (cpu->PB != 0x84u || cpu->D || cpu->_flag_D || cpu->x_flag ||
        cpu->m_flag > 1u || cpu->host_return_valid != 2u ||
        (cpu->DB != 0x7eu && cpu->DB != 0x7fu) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ff7u, true, 1363u, 64u) ||
        !g_snes->cart || g_snes->cart->type != CART_LOROM ||
        !g_snes->cart->rom || g_snes->cart->romSize < 0x280000u ||
        !g_snes->hdmaBeamOff || g_snes->vPos >= 262u ||
        !Lufia2DecompFrameRangeQuiet(
            entry < 0x84ef4eu ? 0x84ef00u : entry, end))
        return false;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->master_bound = 64u;
    bool planned;
    if (entry < 0x84ef4eu) planned = PlanPanelParent(clock, (uint16_t)entry);
    else if (entry == 0x84ef4eu) planned = PlanPanelResource(clock);
    else if (entry == 0x84ef88u) planned = PlanPanelCommandAppend(clock);
    else if (entry == 0x84efc4u) planned = PlanPanelSequence(clock);
    else {
        clock->next = entry == 0x84ef5fu ? entry : 0x84f019u;
        planned = entry == 0x84ef5fu || (!cpu->m_flag &&
            PanelInputOpcode(clock, (FixedSpanOpcode){0xf016u, 3u, 5u, 2u}));
    }
    if (!planned)
        return false;
    const uint64_t bound = clock->master_bound +
        40u * (clock->master_bound / 1324u + 1u);
    if (cpu->master_cycles > UINT64_MAX - bound ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ff7u, true, 1363u, bound))
        return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if ((uint64_t)lines * 1364u - g_snes->hPos <= bound)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}
