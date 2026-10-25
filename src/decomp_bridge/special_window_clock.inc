typedef struct SpecialWindowOpcode {
    FixedSpanOpcode timing;
    uint16_t operand;
    uint8_t index;
} SpecialWindowOpcode;

static const SpecialWindowOpcode special_window_opcodes[] = {
    {{0xa2b3u, 1u, 4u, 2u}, 0x0u, 0u},
    {{0xa2b4u, 1u, 4u, 2u}, 0x0u, 0u},
    {{0xa2b5u, 1u, 3u, 1u}, 0x0u, 0u},
    {{0xa2b6u, 2u, 3u, 0u}, 0x0u, 0u},
    {{0xa2b8u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa2bbu, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa2bcu, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa2bdu, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa2beu, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa2c1u, 3u, 7u, 2u}, 0x0u, 0u},
    {{0xa2c4u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa2c7u, 3u, 7u, 2u}, 0x0u, 0u},
    {{0xa2cau, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa2cdu, 3u, 7u, 2u}, 0x0u, 0u},
    {{0xa2d0u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa2d3u, 3u, 7u, 2u}, 0x0u, 0u},
    {{0xa2d6u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa2d9u, 3u, 7u, 2u}, 0x0u, 0u},
    {{0xa2dcu, 3u, 5u, 2u}, 0x6034u, 1u},
    {{0xa2dfu, 1u, 3u, 0u}, 0x0u, 0u},
    {{0xa2e0u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa2e1u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa2e2u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa2e3u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa2e4u, 4u, 6u, 2u}, 0x0u, 0u},
    {{0xa2e8u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa2e9u, 4u, 6u, 2u}, 0x0u, 0u},
    {{0xa2edu, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa2f0u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa2f3u, 3u, 5u, 2u}, 0x603eu, 1u},
    {{0xa2f6u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa2f9u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa2fcu, 3u, 5u, 2u}, 0x6048u, 1u},
    {{0xa2ffu, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa302u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa303u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa304u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa307u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa30au, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa30du, 3u, 5u, 2u}, 0x2000u, 1u},
    {{0xa310u, 3u, 7u, 2u}, 0x0u, 0u},
    {{0xa313u, 3u, 7u, 2u}, 0x0u, 0u},
    {{0xa316u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa317u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa318u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa319u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa31au, 3u, 8u, 4u}, 0x0u, 0u},
    {{0xa31du, 2u, 2u, 0u}, 0x0u, 0u},
    {{0xa31fu, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa322u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa325u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa328u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa329u, 3u, 3u, 0u}, 0x0u, 0u},
    {{0xa32cu, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa32fu, 3u, 8u, 4u}, 0x0u, 0u},
    {{0xa332u, 2u, 2u, 0u}, 0x0u, 0u},
    {{0xa334u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa337u, 3u, 3u, 0u}, 0x0u, 0u},
    {{0xa33au, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa33du, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa33eu, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa33fu, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa342u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa343u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa344u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa345u, 4u, 6u, 2u}, 0x0u, 0u},
    {{0xa349u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa34au, 3u, 3u, 0u}, 0x0u, 0u},
    {{0xa34du, 3u, 6u, 2u}, 0x0u, 0u},
    {{0xa350u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa351u, 3u, 3u, 0u}, 0x0u, 0u},
    {{0xa354u, 3u, 6u, 2u}, 0x0u, 0u},
    {{0xa357u, 3u, 8u, 4u}, 0x0u, 0u},
    {{0xa35au, 2u, 2u, 0u}, 0x0u, 0u},
    {{0xa35cu, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa35du, 3u, 3u, 0u}, 0x0u, 0u},
    {{0xa360u, 3u, 6u, 2u}, 0x0u, 0u},
    {{0xa363u, 3u, 8u, 4u}, 0x0u, 0u},
    {{0xa366u, 1u, 4u, 1u}, 0x0u, 0u},
    {{0xa367u, 1u, 5u, 2u}, 0x0u, 0u},
    {{0xa368u, 1u, 5u, 2u}, 0x0u, 0u},
    {{0xa369u, 1u, 6u, 2u}, 0x0u, 0u},
    {{0xa36au, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa36du, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa370u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa373u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa374u, 3u, 7u, 2u}, 0x0u, 0u},
    {{0xa377u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa378u, 3u, 8u, 4u}, 0x0u, 0u},
    {{0xa37bu, 3u, 8u, 4u}, 0x0u, 0u},
    {{0xa37eu, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa37fu, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa380u, 3u, 7u, 2u}, 0x0u, 0u},
    {{0xa383u, 3u, 8u, 4u}, 0x0u, 0u},
    {{0xa386u, 2u, 2u, 0u}, 0x0u, 0u},
    {{0xa388u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa389u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa38au, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa38bu, 3u, 7u, 2u}, 0x0u, 0u},
    {{0xa38eu, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa38fu, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa392u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa393u, 3u, 3u, 0u}, 0x0u, 0u},
    {{0xa396u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa399u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa39au, 1u, 6u, 2u}, 0x0u, 0u},
    {{0xa39bu, 1u, 3u, 1u}, 0x0u, 0u},
    {{0xa39cu, 2u, 3u, 0u}, 0x0u, 0u},
    {{0xa39eu, 3u, 8u, 4u}, 0x0u, 0u},
    {{0xa3a1u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa3a4u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa3a5u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa3a6u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa3a7u, 3u, 5u, 2u}, 0x6034u, 1u},
    {{0xa3aau, 1u, 3u, 0u}, 0x0u, 0u},
    {{0xa3abu, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa3acu, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa3adu, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa3aeu, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa3afu, 4u, 6u, 2u}, 0x0u, 0u},
    {{0xa3b3u, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa3b4u, 4u, 6u, 2u}, 0x0u, 0u},
    {{0xa3b8u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa3bbu, 3u, 5u, 2u}, 0x603eu, 1u},
    {{0xa3beu, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa3c1u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa3c4u, 3u, 5u, 2u}, 0x6048u, 1u},
    {{0xa3c7u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa3cau, 3u, 5u, 2u}, 0x6020u, 1u},
    {{0xa3cdu, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa3d0u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa3d3u, 3u, 5u, 2u}, 0x605eu, 2u},
    {{0xa3d6u, 3u, 7u, 2u}, 0x0u, 0u},
    {{0xa3d9u, 3u, 7u, 2u}, 0x0u, 0u},
    {{0xa3dcu, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa3ddu, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa3deu, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa3dfu, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa3e0u, 3u, 8u, 4u}, 0x0u, 0u},
    {{0xa3e3u, 2u, 2u, 0u}, 0x0u, 0u},
    {{0xa3e5u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa3e8u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa3ebu, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa3eeu, 1u, 2u, 0u}, 0x0u, 0u},
    {{0xa3efu, 3u, 3u, 0u}, 0x0u, 0u},
    {{0xa3f2u, 3u, 5u, 2u}, 0x0u, 0u},
    {{0xa3f5u, 3u, 8u, 4u}, 0x0u, 0u},
    {{0xa3f8u, 2u, 2u, 0u}, 0x0u, 0u},
    {{0xa3fau, 1u, 4u, 1u}, 0x0u, 0u},
    {{0xa3fbu, 1u, 6u, 2u}, 0x0u, 0u},
};

enum { SPECIAL_WINDOW_OPCODE_CAPACITY = 12000u };

typedef struct SpecialWindowClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[SPECIAL_WINDOW_OPCODE_CAPACITY];
    ActorPushedCall call;
} SpecialWindowClock;

static unsigned SpecialWindowWord(const CpuState *cpu, unsigned address) {
    return cpu->ram[address] | (unsigned)cpu->ram[address + 1u] << 8;
}

static void SpecialWindowRange(SpecialWindowClock *clock, unsigned start,
    unsigned end, unsigned x, unsigned y, bool branch) {
    for (unsigned i = 0u; i < sizeof special_window_opcodes / sizeof special_window_opcodes[0]; ++i) {
        const SpecialWindowOpcode *op = special_window_opcodes + i;
        if (op->timing.pc < start || op->timing.pc >= end)
            continue;
        assert(clock->span.count < SPECIAL_WINDOW_OPCODE_CAPACITY);
        FixedSpanOpcode timing = op->timing;
        if (op->index) {
            const unsigned index = op->index == 1u ? x : y;
            if ((unsigned)op->operand >> 8 != (op->operand + index) >> 8)
                ++timing.cycles;
        }
        if (branch && (unsigned)timing.pc + timing.fetched == end)
            ++timing.cycles;
        clock->timing[clock->span.count++] = timing;
    }
}

static bool SpecialWindowRectangle(unsigned left, unsigned top,
    unsigned width, unsigned height, unsigned buffer) {
    return width >= 3u && height >= 3u && left < 32u && top < 32u &&
        width <= 32u - left && height <= 32u - top && !(buffer & 1u) &&
        buffer <= 0x800u && width * height * 2u <= 0x800u - buffer;
}

static bool PrepareSpecialWindowClock(SpecialWindowClock *clock, uint32_t pc) {
    CpuState *cpu = clock->call.cpu;
    if (!cpu->ram || !g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x24000u ||
        cpu->PB != 0x84u || cpu->DB != 0x7eu || cpu->emulation ||
        cpu->m_flag > 1u || cpu->x_flag || cpu->_flag_D ||
        cpu->host_return_valid > 3u || cpu->host_return_valid < 2u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return false;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    unsigned depth = SpecialWindowWord(cpu, 0x6010u);
    if (pc == 0x84a2b3u || pc == 0x84a39bu) {
        if (pc == 0x84a39bu) {
            if (!depth || depth > 5u) return false;
            --depth;
        } else if (depth >= 5u) return false;
        const unsigned record = depth * 2u;
        const bool push = pc == 0x84a2b3u;
        const unsigned left = SpecialWindowWord(cpu, push ? 0x6016u : 0x602au + record);
        const unsigned top = SpecialWindowWord(cpu, push ? 0x6018u : 0x6034u + record);
        const unsigned width = SpecialWindowWord(cpu, push ? 0x601au : 0x603eu + record);
        const unsigned height = SpecialWindowWord(cpu, push ? 0x601cu : 0x6048u + record);
        const unsigned buffer = SpecialWindowWord(cpu, push ? 0x601eu : 0x6020u + record);
        if (!SpecialWindowRectangle(left, top, width, height, buffer)) return false;
        SpecialWindowRange(clock, push ? 0xa2b3u : 0xa39bu,
            push ? 0xa30au : 0xa3d0u, record, 0u, false);
        for (unsigned row = 0u; row < height; ++row) {
            SpecialWindowRange(clock, push ? 0xa30au : 0xa3d0u,
                push ? 0xa30du : 0xa3d3u, 0u, 0u, false);
            for (unsigned column = 0u; column < width; ++column)
                SpecialWindowRange(clock, push ? 0xa30du : 0xa3d3u,
                    push ? 0xa31fu : 0xa3e5u,
                    (top + row) * 64u + (left + column) * 2u,
                    buffer + (row * width + column) * 2u, column + 1u < width);
            SpecialWindowRange(clock, push ? 0xa31fu : 0xa3e5u,
                push ? 0xa334u : 0xa3fau, 0u, 0u, row + 1u < height);
        }
        SpecialWindowRange(clock, push ? 0xa334u : 0xa3fau,
            push ? 0xa350u : 0xa3fbu, 0u, 0u, false);
    } else if (pc == 0x84a36au) {
        const unsigned width = SpecialWindowWord(cpu, 0x6056u);
        const unsigned row = SpecialWindowWord(cpu, 0x6052u);
        if (cpu->m_flag || width < 3u || width > 32u || row >= 0x800u ||
            (row & 1u) || (row & 63u) / 2u + width > 32u) return false;
        SpecialWindowRange(clock, 0xa36au, 0xa37eu, 0u, 0u, false);
        for (unsigned column = 0u; column < width - 2u; ++column)
            SpecialWindowRange(clock, 0xa37eu, 0xa388u, 0u, 0u, column + 1u < width - 2u);
        SpecialWindowRange(clock, 0xa388u, 0xa39au, 0u, 0u, false);
    } else {
        if (cpu->m_flag) return false;
        if (pc == 0x84a350u) {
            SpecialWindowRange(clock, 0xa350u, 0xa357u, 0u, 0u, false);
        } else if (pc == 0x84a357u) {
            const unsigned interior = SpecialWindowWord(cpu, 0x605cu);
            SpecialWindowRange(clock, 0xa357u, 0xa35cu, 0u, 0u, interior != 1u);
            SpecialWindowRange(clock, interior != 1u ? 0xa354u : 0xa35cu,
                interior != 1u ? 0xa357u : 0xa363u, 0u, 0u, false);
        } else if (pc == 0x84a363u) {
            if (cpu->ram[cpu->S + 1u] & 0x10u) return false;
            SpecialWindowRange(clock, 0xa363u, 0xa369u, 0u, 0u, false);
        } else return false;
    }
    unsigned budget = 64u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(clock->span.bank | op->pc) +
            op->data_bytes * 8u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x840000u, 0x1ffcu, true, 1363u, budget) ||
        !g_snes->hdmaBeamOff || g_snes->vPos >= 262u) return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if (lines * 1364u - g_snes->hPos <= budget) return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static uint8_t SpecialWindowChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame) {
    SpecialWindowClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame)) return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame);
    if (PrepareSpecialWindowClock(clock, landing)) return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeSpecialWindow(CpuState *cpu, uint32_t entry) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    SpecialWindowClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (!PrepareSpecialWindowClock(&clock, entry)) return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = entry == 0x84a2b3u
        ? Lufia2SpecialMenuPushWindow(&memory, &state, SpecialWindowChild, &clock)
        : entry == 0x84a36au ? Lufia2SpecialMenuDrawWindowRow(&memory, &state)
        : Lufia2SpecialMenuPopWindow(&memory, &state);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED &&
        clock.span.index == clock.span.count && !clock.span.remaining);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
