enum { BOARD_BORDER_CLOCK_CAPACITY = 384u };

typedef struct BoardBorderClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[BOARD_BORDER_CLOCK_CAPACITY];
    ActorPushedCall call;
    uint32_t next;
    bool returning;
} BoardBorderClock;

static void BoardBorderOpcode(BoardBorderClock *clock, uint16_t pc,
                               unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < BOARD_BORDER_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static bool BoardBorderRegistersSupported(const CpuState *cpu) {
    return cpu->ram && cpu->PB == 0x84u && !cpu->emulation && cpu->m_flag <= 1u &&
        !cpu->x_flag && !cpu->D && !cpu->_flag_D && cpu->DB == 0x7eu &&
        cpu->S >= 0x1f20u && cpu->S <= 0x1ffcu;
}

static bool BoardBorderWindowSupported(const BoardBorderClock *clock) {
    CpuState *cpu = clock->call.cpu;
    if (!BoardBorderRegistersSupported(cpu) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return false;
    unsigned budget = clock->returning
        ? cpu_region_speed(clock->next) + 2u * 8u + 3u * 6u : 0u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(0x840000u | op->pc) +
            op->data_bytes * 8u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (!budget || cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x840000u, 0x1ffcu, true,
                                             1363u, budget))
        return false;
    if (g_snes->vPos < 225u) {
        const unsigned until_vblank =
            (225u - g_snes->vPos) * 1364u - g_snes->hPos;
        if (!g_snes->hdmaBeamOff || until_vblank <= budget)
            return false;
    } else if (g_snes->vPos >= 262u ||
               (262u - g_snes->vPos) * 1364u - g_snes->hPos <= budget) {
        return false;
    }
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static void BoardBorderSetup(BoardBorderClock *clock, uint16_t pc, bool vertical) {
    BoardBorderOpcode(clock, pc, 1u, 3u, 1u);
    BoardBorderOpcode(clock, pc + 1u, 2u, 3u, 0u);
    BoardBorderOpcode(clock, pc + 3u, 2u, 2u, 0u);
    BoardBorderOpcode(clock, pc + 5u, 2u, 3u, 1u);
    BoardBorderOpcode(clock, pc + 7u, 3u, 3u, 0u);
    BoardBorderOpcode(clock, pc + 10u, 2u, 4u, 2u);
    BoardBorderOpcode(clock, pc + 12u, 2u, 3u, 0u);
    BoardBorderOpcode(clock, pc + 14u, 3u, 5u, 2u);
    for (unsigned i = 17u; i < 20u; ++i)
        BoardBorderOpcode(clock, pc + i, 1u, 2u, 0u);
    BoardBorderOpcode(clock, pc + 20u, 2u, 7u, 5u);
    BoardBorderOpcode(clock, pc + 22u, 1u, 2u, 0u);
    BoardBorderOpcode(clock, pc + 23u, 3u, 3u, 0u);
    BoardBorderOpcode(clock, pc + 26u, 2u, 4u, 2u);
    BoardBorderOpcode(clock, pc + 28u, 3u, 5u, 2u);
    BoardBorderOpcode(clock, pc + 31u, 3u, 3u, 0u);
    BoardBorderOpcode(clock, pc + 34u, 2u, vertical ? 3u : 2u, 0u);
    const uint16_t dimensions = pc + (vertical ? 58u : 36u);
    for (unsigned i = 0u; i < 4u; ++i) {
        BoardBorderOpcode(clock, dimensions + i * 5u, 3u, 3u, 0u);
        BoardBorderOpcode(clock, dimensions + i * 5u + 3u, 2u, 4u, 2u);
    }
    if (!vertical)
        BoardBorderOpcode(clock, pc + 56u, 2u, 3u, 0u);
    BoardBorderOpcode(clock, pc + 78u, 2u, 4u, 2u);
}

static void BoardBorderHorizontal(BoardBorderClock *clock, uint16_t pc,
                                   unsigned columns, bool clearing) {
    if (clearing) {
        BoardBorderOpcode(clock, pc, 2u, 4u, 2u);
        pc += 2u;
    } else {
        BoardBorderOpcode(clock, pc, 3u, 3u, 0u);
        BoardBorderOpcode(clock, pc + 3u, 3u, 7u, 2u);
        BoardBorderOpcode(clock, pc + 6u, 1u, 2u, 0u);
        BoardBorderOpcode(clock, pc + 7u, 1u, 2u, 0u);
        BoardBorderOpcode(clock, pc + 8u, 2u, 4u, 2u);
        BoardBorderOpcode(clock, pc + 10u, 3u, 3u, 0u);
        pc += 13u;
    }
    for (unsigned column = 0u; column < columns; ++column) {
        BoardBorderOpcode(clock, pc, 3u, 7u, 2u);
        for (unsigned i = 3u; i < 6u; ++i)
            BoardBorderOpcode(clock, pc + i, 1u, 2u, 0u);
        BoardBorderOpcode(clock, pc + 6u, 2u, column + 1u < columns ? 3u : 2u, 0u);
    }
    if (!clearing) {
        BoardBorderOpcode(clock, pc + 8u, 3u, 3u, 0u);
        BoardBorderOpcode(clock, pc + 11u, 3u, 7u, 2u);
    }
}

static void BoardBorderSides(BoardBorderClock *clock, uint16_t pc,
                              unsigned rows, bool clearing) {
    BoardBorderOpcode(clock, pc, 2u, 4u, 2u);
    BoardBorderOpcode(clock, pc + 2u, 1u, 2u, 0u);
    BoardBorderOpcode(clock, pc + 3u, 3u, 3u, 0u);
    BoardBorderOpcode(clock, pc + 6u, 1u, 2u, 0u);
    BoardBorderOpcode(clock, pc + 7u, 2u, 4u, 2u);
    pc += 9u;
    for (unsigned row = 0u; row < rows; ++row) {
        uint16_t part = pc;
        for (unsigned side = 0u; side < 2u; ++side) {
            if (!clearing) {
                BoardBorderOpcode(clock, part, 3u, 3u, 0u);
                part += 3u;
            }
            BoardBorderOpcode(clock, part, 3u, 7u, 2u);
            BoardBorderOpcode(clock, part + 3u, 1u, 2u, 0u);
            BoardBorderOpcode(clock, part + 4u, 1u, 2u, 0u);
            BoardBorderOpcode(clock, part + 5u, 2u, 4u, 2u);
            BoardBorderOpcode(clock, part + 7u, 1u, 2u, 0u);
            part += 8u;
        }
        BoardBorderOpcode(clock, part, 1u, 2u, 0u);
        BoardBorderOpcode(clock, part + 1u, 2u, row + 1u < rows ? 3u : 2u, 0u);
    }
}

static bool PrepareBoardBorderClock(BoardBorderClock *clock, uint32_t pc) {
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->returning = false;
    clock->next = pc;
    if (!BoardBorderRegistersSupported(clock->call.cpu))
        return false;
    if (pc == 0x84cbcdu) {
        BoardBorderOpcode(clock, 0xcbcdu, 1u, 3u, 1u);
        BoardBorderOpcode(clock, 0xcbceu, 1u, 4u, 2u);
        BoardBorderOpcode(clock, 0xcbcfu, 2u, 3u, 0u);
        BoardBorderOpcode(clock, 0xcbd1u, 3u, 6u, 2u);
    } else if (pc == 0x84cbd4u) {
        BoardBorderOpcode(clock, 0xcbd4u, 2u, 3u, 1u);
        BoardBorderOpcode(clock, 0xcbd6u, 2u, 2u, 0u);
        BoardBorderOpcode(clock, 0xcbd8u, 2u, 3u, 1u);
        BoardBorderOpcode(clock, 0xcbdau, 2u, 3u, 1u);
        BoardBorderOpcode(clock, 0xcbdcu, 3u, 6u, 2u);
    } else if (pc == 0x84cbdfu) {
        BoardBorderOpcode(clock, 0xcbdfu, 1u, 5u, 2u);
        BoardBorderOpcode(clock, 0xcbe0u, 1u, 4u, 1u);
        clock->returning = true;
        clock->next = 0x84cbe1u;
    } else if (pc == 0x84cee4u || pc == 0x84cf91u) {
        const uint8_t *ram = clock->call.cpu->ram;
        const unsigned group = ram[0x71ceu] | (unsigned)ram[0x71cfu] << 8u;
        if (group > 3u)
            return false;
        const bool clearing = pc == 0x84cf91u;
        const bool vertical = (group & 1u) != 0u;
        const unsigned columns = (vertical ? 5u : 8u) + (clearing ? 2u : 0u);
        BoardBorderSetup(clock, (uint16_t)pc, vertical);
        BoardBorderHorizontal(clock, clearing ? 0xcfe1u : 0xcf34u, columns, clearing);
        BoardBorderSides(clock, clearing ? 0xcfebu : 0xcf4fu, vertical ? 8u : 5u, clearing);
        BoardBorderHorizontal(clock, clearing ? 0xd007u : 0xcf71u, columns, clearing);
        BoardBorderOpcode(clock, clearing ? 0xd011u : 0xcf8cu, 3u, 6u, 2u);
    } else if (pc == 0x84cf8fu || pc == 0x84d014u) {
        BoardBorderOpcode(clock, (uint16_t)pc, 1u, 4u, 1u);
        clock->returning = true;
        clock->next = pc + 1u;
    } else {
        return false;
    }
    return BoardBorderWindowSupported(clock);
}

static uint8_t BoardBorderChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    BoardBorderClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareBoardBorderClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing,
        clock->call.frame.entry_s, clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeTimedBoardBorder(CpuState *cpu, uint32_t entry,
                                               ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    BoardBorderClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || !PrepareBoardBorderClock(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, BoardBorderChild, &clock);
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
