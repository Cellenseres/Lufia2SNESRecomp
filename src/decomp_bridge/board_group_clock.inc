typedef struct BoardGroupPhase {
    uint16_t pc;
    uint16_t branch;
    uint16_t return_pc;
    uint8_t byte_mode;
    unsigned count;
    const FixedSpanOpcode *opcodes;
} BoardGroupPhase;

static const FixedSpanOpcode board_group_d016[] = {
    {0xd016u, 1u, 3u, 1u},
    {0xd017u, 2u, 3u, 0u},
    {0xd019u, 2u, 3u, 1u},
    {0xd01bu, 2u, 3u, 1u},
    {0xd01du, 2u, 2u, 0u},
    {0xd01fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode board_group_d022[] = {
    {0xd022u, 3u, 6u, 2u},
};

static const FixedSpanOpcode board_group_d025[] = {
    {0xd025u, 2u, 2u, 0u},
    {0xd027u, 2u, 3u, 1u},
    {0xd029u, 3u, 3u, 0u},
    {0xd02cu, 2u, 4u, 2u},
    {0xd02eu, 2u, 3u, 0u},
    {0xd030u, 3u, 5u, 2u},
    {0xd033u, 1u, 2u, 0u},
    {0xd034u, 1u, 2u, 0u},
    {0xd035u, 1u, 2u, 0u},
    {0xd036u, 2u, 7u, 5u},
    {0xd038u, 2u, 4u, 2u},
    {0xd03au, 3u, 5u, 2u},
    {0xd03du, 3u, 3u, 0u},
};

static const FixedSpanOpcode board_group_d042[] = {
    {0xd042u, 3u, 3u, 0u},
    {0xd045u, 2u, 4u, 2u},
    {0xd047u, 2u, 4u, 2u},
    {0xd049u, 3u, 3u, 0u},
};

static const FixedSpanOpcode board_group_d04e[] = {
    {0xd04eu, 3u, 3u, 0u},
    {0xd051u, 2u, 4u, 2u},
    {0xd053u, 2u, 3u, 0u},
    {0xd066u, 3u, 3u, 0u},
    {0xd069u, 3u, 3u, 0u},
    {0xd06cu, 2u, 4u, 2u},
    {0xd06eu, 3u, 3u, 0u},
    {0xd071u, 2u, 4u, 2u},
    {0xd073u, 2u, 7u, 5u},
    {0xd075u, 1u, 2u, 0u},
    {0xd076u, 1u, 2u, 0u},
    {0xd077u, 1u, 2u, 0u},
    {0xd078u, 2u, 4u, 2u},
    {0xd07au, 2u, 4u, 2u},
    {0xd07cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode board_group_d055[] = {
    {0xd055u, 3u, 3u, 0u},
    {0xd058u, 2u, 4u, 2u},
    {0xd05au, 2u, 4u, 2u},
    {0xd05cu, 3u, 3u, 0u},
};

static const FixedSpanOpcode board_group_d061[] = {
    {0xd061u, 3u, 3u, 0u},
    {0xd064u, 2u, 4u, 2u},
    {0xd066u, 3u, 3u, 0u},
    {0xd069u, 3u, 3u, 0u},
    {0xd06cu, 2u, 4u, 2u},
    {0xd06eu, 3u, 3u, 0u},
    {0xd071u, 2u, 4u, 2u},
    {0xd073u, 2u, 7u, 5u},
    {0xd075u, 1u, 2u, 0u},
    {0xd076u, 1u, 2u, 0u},
    {0xd077u, 1u, 2u, 0u},
    {0xd078u, 2u, 4u, 2u},
    {0xd07au, 2u, 4u, 2u},
    {0xd07cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode board_group_d066[] = {
    {0xd066u, 3u, 3u, 0u},
    {0xd069u, 3u, 3u, 0u},
    {0xd06cu, 2u, 4u, 2u},
    {0xd06eu, 3u, 3u, 0u},
    {0xd071u, 2u, 4u, 2u},
    {0xd073u, 2u, 7u, 5u},
    {0xd075u, 1u, 2u, 0u},
    {0xd076u, 1u, 2u, 0u},
    {0xd077u, 1u, 2u, 0u},
    {0xd078u, 2u, 4u, 2u},
    {0xd07au, 2u, 4u, 2u},
    {0xd07cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode board_group_d06e[] = {
    {0xd06eu, 3u, 3u, 0u},
    {0xd071u, 2u, 4u, 2u},
    {0xd073u, 2u, 7u, 5u},
    {0xd075u, 1u, 2u, 0u},
    {0xd076u, 1u, 2u, 0u},
    {0xd077u, 1u, 2u, 0u},
    {0xd078u, 2u, 4u, 2u},
    {0xd07au, 2u, 4u, 2u},
    {0xd07cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode board_group_d073[] = {
    {0xd073u, 2u, 7u, 5u},
    {0xd075u, 1u, 2u, 0u},
    {0xd076u, 1u, 2u, 0u},
    {0xd077u, 1u, 2u, 0u},
    {0xd078u, 2u, 4u, 2u},
    {0xd07au, 2u, 4u, 2u},
    {0xd07cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode board_group_d07f[] = {
    {0xd07fu, 2u, 7u, 5u},
    {0xd081u, 1u, 2u, 0u},
    {0xd082u, 1u, 2u, 0u},
    {0xd083u, 1u, 2u, 0u},
    {0xd084u, 2u, 4u, 2u},
    {0xd086u, 3u, 6u, 2u},
};

static const FixedSpanOpcode board_group_d089[] = {
    {0xd089u, 2u, 7u, 4u},
};

static const FixedSpanOpcode board_group_d08d[] = {
    {0xd08du, 3u, 6u, 2u},
};

static const FixedSpanOpcode board_group_d090[] = {
    {0xd090u, 1u, 4u, 2u},
    {0xd091u, 3u, 6u, 2u},
};

static const FixedSpanOpcode board_group_d094[] = {
    {0xd094u, 1u, 5u, 2u},
    {0xd095u, 2u, 7u, 4u},
};

static const FixedSpanOpcode board_group_d099[] = {
    {0xd099u, 1u, 4u, 1u},
};

static const BoardGroupPhase board_group_phases[] = {
    {0xd016u, 0x0000u, 0x0000u, 0u, sizeof board_group_d016 / sizeof board_group_d016[0], board_group_d016},
    {0xd022u, 0x0000u, 0x0000u, 1u, sizeof board_group_d022 / sizeof board_group_d022[0], board_group_d022},
    {0xd025u, 0xd040u, 0x0000u, 1u, sizeof board_group_d025 / sizeof board_group_d025[0], board_group_d025},
    {0xd042u, 0xd04cu, 0x0000u, 0u, sizeof board_group_d042 / sizeof board_group_d042[0], board_group_d042},
    {0xd04eu, 0x0000u, 0x0000u, 0u, sizeof board_group_d04e / sizeof board_group_d04e[0], board_group_d04e},
    {0xd055u, 0xd05fu, 0x0000u, 0u, sizeof board_group_d055 / sizeof board_group_d055[0], board_group_d055},
    {0xd061u, 0x0000u, 0x0000u, 0u, sizeof board_group_d061 / sizeof board_group_d061[0], board_group_d061},
    {0xd066u, 0x0000u, 0x0000u, 0u, sizeof board_group_d066 / sizeof board_group_d066[0], board_group_d066},
    {0xd06eu, 0x0000u, 0x0000u, 0u, sizeof board_group_d06e / sizeof board_group_d06e[0], board_group_d06e},
    {0xd073u, 0x0000u, 0x0000u, 0u, sizeof board_group_d073 / sizeof board_group_d073[0], board_group_d073},
    {0xd07fu, 0x0000u, 0x0000u, 0u, sizeof board_group_d07f / sizeof board_group_d07f[0], board_group_d07f},
    {0xd089u, 0xd08bu, 0x0000u, 0u, sizeof board_group_d089 / sizeof board_group_d089[0], board_group_d089},
    {0xd08du, 0x0000u, 0x0000u, 0u, sizeof board_group_d08d / sizeof board_group_d08d[0], board_group_d08d},
    {0xd090u, 0x0000u, 0x0000u, 0u, sizeof board_group_d090 / sizeof board_group_d090[0], board_group_d090},
    {0xd094u, 0xd097u, 0x0000u, 0u, sizeof board_group_d094 / sizeof board_group_d094[0], board_group_d094},
    {0xd099u, 0x0000u, 0xd09au, 0u, sizeof board_group_d099 / sizeof board_group_d099[0], board_group_d099},
};

enum { BOARD_GROUP_CLOCK_CAPACITY = 256u };

typedef struct BoardGroupClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[BOARD_GROUP_CLOCK_CAPACITY];
    ActorPushedCall call;
    uint32_t next;
    bool returning;
} BoardGroupClock;

static void BoardGroupOpcode(BoardGroupClock *clock, unsigned pc,
                               unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < BOARD_GROUP_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){(uint16_t)pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static const BoardGroupPhase *FindBoardGroupPhase(uint16_t pc) {
    for (unsigned i = 0u; i < sizeof board_group_phases / sizeof board_group_phases[0]; ++i)
        if (board_group_phases[i].pc == pc)
            return board_group_phases + i;
    return NULL;
}

static uint16_t BoardGroupWord(const CpuState *cpu, unsigned address) {
    return cpu->ram[address] | (uint16_t)cpu->ram[address + 1u] << 8;
}

static bool AppendBoardGroupPhase(BoardGroupClock *clock, uint16_t pc) {
    const BoardGroupPhase *phase = FindBoardGroupPhase(pc);
    const CpuState *cpu = clock->call.cpu;
    if (!phase || clock->span.count + phase->count + 1u >= BOARD_GROUP_CLOCK_CAPACITY)
        return false;
    for (unsigned i = 0u; i < phase->count; ++i) {
        const FixedSpanOpcode *op = phase->opcodes + i;
        BoardGroupOpcode(clock, op->pc, op->fetched, op->cycles, op->data_bytes);
    }
    if (phase->return_pc) {
        clock->returning = true;
        clock->next = 0x840000u | phase->return_pc;
    }
    if (!phase->branch)
        return true;
    bool taken;
    uint16_t target;
    switch (phase->branch) {
    case 0xd040u:
        taken = (BoardGroupWord(cpu, 0x71ceu) & 1u) != 0u;
        target = taken ? 0xd055u : 0xd042u;
        break;
    case 0xd04cu:
    case 0xd05fu:
        taken = (BoardGroupWord(cpu, 0x22u) & 5u) != 0u;
        target = taken ? 0xd066u : phase->branch == 0xd04cu ? 0xd04eu : 0xd061u;
        break;
    case 0xd08bu:
        taken = !((uint16_t)(BoardGroupWord(cpu, 2u) - 1u) & 0x8000u);
        target = taken ? 0xd073u : 0xd08du;
        break;
    case 0xd097u:
        taken = !((uint16_t)(BoardGroupWord(cpu, 4u) - 1u) & 0x8000u);
        target = taken ? 0xd06eu : 0xd099u;
        break;
    default: return false;
    }
    BoardGroupOpcode(clock, phase->branch, 2u, taken ? 3u : 2u, 0u);
    return AppendBoardGroupPhase(clock, target);
}

static bool BoardGroupWindowSupported(const BoardGroupClock *clock) {
    CpuState *cpu = clock->call.cpu;
    if (!CardScoreRegistersSupported(cpu) ||
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

static bool PrepareBoardGroupClock(BoardGroupClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->returning = false;
    clock->next = pc;
    if (!g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x140000u ||
        (pc >> 16) != 0x84u || !CardScoreRegistersSupported(cpu))
        return false;
    const BoardGroupPhase *phase = FindBoardGroupPhase((uint16_t)pc);
    if (!phase || (pc != 0x84d016u && cpu->m_flag != phase->byte_mode))
        return false;
    return AppendBoardGroupPhase(clock, (uint16_t)pc) &&
        BoardGroupWindowSupported(clock);
}

static uint8_t BoardGroupChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    BoardGroupClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareBoardGroupClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeBoardGroup(CpuState *cpu, uint32_t entry,
                                         ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    BoardGroupClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || !PrepareBoardGroupClock(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, BoardGroupChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.returning && clock.next == result.pc &&
        clock.span.index == clock.span.count && !clock.span.remaining);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
