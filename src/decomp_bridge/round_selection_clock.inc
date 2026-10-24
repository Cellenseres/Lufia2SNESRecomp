typedef struct RoundSelectionPhase {
    uint16_t pc;
    uint16_t branch;
    uint16_t return_pc;
    uint8_t byte_mode;
    unsigned count;
    const FixedSpanOpcode *opcodes;
} RoundSelectionPhase;

static const FixedSpanOpcode round_selection_b027[] = {
    {0xb027u, 2u, 3u, 0u},
    {0xb029u, 3u, 3u, 0u},
    {0xb02cu, 3u, 5u, 2u},
    {0xb02fu, 3u, 5u, 2u},
    {0xb032u, 2u, 2u, 0u},
    {0xb034u, 2u, 3u, 1u},
    {0xb036u, 3u, 3u, 0u},
    {0xb039u, 2u, 4u, 2u},
    {0xb03bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_b03e[] = {
    {0xb03eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_b041[] = {
    {0xb041u, 2u, 2u, 0u},
    {0xb043u, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_b046[] = {
    {0xb046u, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_b049[] = {
    {0xb049u, 2u, 2u, 0u},
    {0xb04bu, 2u, 3u, 1u},
    {0xb04du, 3u, 3u, 0u},
    {0xb050u, 2u, 4u, 2u},
    {0xb052u, 2u, 3u, 0u},
    {0xb054u, 3u, 3u, 0u},
    {0xb057u, 3u, 5u, 2u},
    {0xb05au, 3u, 5u, 2u},
    {0xb05du, 3u, 5u, 2u},
    {0xb060u, 3u, 5u, 2u},
    {0xb063u, 1u, 2u, 0u},
    {0xb064u, 2u, 4u, 2u},
    {0xb066u, 2u, 4u, 2u},
    {0xb068u, 1u, 2u, 0u},
    {0xb069u, 1u, 2u, 0u},
    {0xb06au, 1u, 2u, 0u},
    {0xb06bu, 1u, 2u, 0u},
    {0xb06cu, 3u, 8u, 4u},
    {0xb06fu, 3u, 5u, 2u},
    {0xb072u, 3u, 5u, 2u},
    {0xb075u, 3u, 7u, 2u},
    {0xb078u, 1u, 2u, 0u},
    {0xb079u, 1u, 2u, 0u},
    {0xb07au, 1u, 2u, 0u},
    {0xb07bu, 2u, 7u, 5u},
    {0xb07du, 2u, 4u, 2u},
    {0xb07fu, 1u, 2u, 0u},
    {0xb080u, 1u, 2u, 0u},
    {0xb081u, 2u, 7u, 5u},
    {0xb083u, 2u, 4u, 2u},
    {0xb085u, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_b088[] = {
    {0xb088u, 2u, 4u, 2u},
    {0xb08au, 2u, 4u, 2u},
    {0xb08cu, 3u, 5u, 2u},
    {0xb08fu, 2u, 4u, 2u},
    {0xb091u, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_b094[] = {
    {0xb094u, 3u, 3u, 0u},
    {0xb097u, 4u, 8u, 3u},
};

static const FixedSpanOpcode round_selection_b09b[] = {
    {0xb09bu, 3u, 3u, 0u},
    {0xb09eu, 2u, 4u, 2u},
    {0xb0a0u, 3u, 3u, 0u},
    {0xb0a3u, 1u, 2u, 0u},
    {0xb0a4u, 1u, 2u, 0u},
    {0xb0a5u, 1u, 2u, 0u},
    {0xb0a6u, 3u, 5u, 2u},
    {0xb0a9u, 1u, 2u, 0u},
};

static const FixedSpanOpcode round_selection_b0a4[] = {
    {0xb0a4u, 1u, 2u, 0u},
    {0xb0a5u, 1u, 2u, 0u},
    {0xb0a6u, 3u, 5u, 2u},
    {0xb0a9u, 1u, 2u, 0u},
};

static const FixedSpanOpcode round_selection_b0ac[] = {
    {0xb0acu, 1u, 2u, 0u},
};

static const FixedSpanOpcode round_selection_b0af[] = {
    {0xb0afu, 3u, 8u, 4u},
    {0xb0b2u, 3u, 7u, 2u},
    {0xb0b5u, 3u, 5u, 2u},
    {0xb0b8u, 3u, 5u, 2u},
    {0xb0bbu, 1u, 2u, 0u},
    {0xb0bcu, 1u, 2u, 0u},
    {0xb0bdu, 1u, 2u, 0u},
    {0xb0beu, 1u, 2u, 0u},
    {0xb0bfu, 2u, 4u, 2u},
    {0xb0c1u, 2u, 4u, 2u},
    {0xb0c3u, 1u, 2u, 0u},
    {0xb0c4u, 1u, 2u, 0u},
    {0xb0c5u, 1u, 2u, 0u},
    {0xb0c6u, 1u, 2u, 0u},
    {0xb0c7u, 2u, 7u, 5u},
    {0xb0c9u, 2u, 4u, 2u},
    {0xb0cbu, 1u, 2u, 0u},
    {0xb0ccu, 1u, 2u, 0u},
    {0xb0cdu, 2u, 7u, 5u},
    {0xb0cfu, 2u, 4u, 2u},
    {0xb0d1u, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_b0d4[] = {
    {0xb0d4u, 2u, 4u, 2u},
    {0xb0d6u, 2u, 4u, 2u},
    {0xb0d8u, 3u, 5u, 2u},
    {0xb0dbu, 2u, 4u, 2u},
    {0xb0ddu, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_b0e0[] = {
    {0xb0e0u, 3u, 3u, 0u},
    {0xb0e3u, 2u, 4u, 2u},
    {0xb0e5u, 3u, 3u, 0u},
    {0xb0e8u, 1u, 2u, 0u},
    {0xb0e9u, 1u, 2u, 0u},
    {0xb0eau, 3u, 10u, 4u},
};

static const FixedSpanOpcode round_selection_b0e8[] = {
    {0xb0e8u, 1u, 2u, 0u},
    {0xb0e9u, 1u, 2u, 0u},
    {0xb0eau, 3u, 10u, 4u},
};

static const FixedSpanOpcode round_selection_b0ef[] = {
    {0xb0efu, 3u, 8u, 4u},
    {0xb0f2u, 3u, 5u, 2u},
    {0xb0f5u, 3u, 5u, 2u},
    {0xb0f8u, 1u, 2u, 0u},
    {0xb0f9u, 1u, 2u, 0u},
    {0xb0fau, 1u, 2u, 0u},
    {0xb0fbu, 1u, 2u, 0u},
    {0xb0fcu, 2u, 4u, 2u},
    {0xb0feu, 2u, 4u, 2u},
    {0xb100u, 1u, 2u, 0u},
    {0xb101u, 1u, 2u, 0u},
    {0xb102u, 1u, 2u, 0u},
    {0xb103u, 1u, 2u, 0u},
    {0xb104u, 2u, 7u, 5u},
    {0xb106u, 2u, 4u, 2u},
    {0xb108u, 1u, 2u, 0u},
    {0xb109u, 1u, 2u, 0u},
    {0xb10au, 2u, 7u, 5u},
    {0xb10cu, 2u, 4u, 2u},
    {0xb10eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_b111[] = {
    {0xb111u, 2u, 4u, 2u},
    {0xb113u, 2u, 4u, 2u},
    {0xb115u, 3u, 5u, 2u},
    {0xb118u, 2u, 4u, 2u},
    {0xb11au, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_cb64[] = {
    {0xcb64u, 1u, 3u, 1u},
    {0xcb65u, 2u, 3u, 0u},
    {0xcb67u, 3u, 3u, 0u},
    {0xcb6au, 3u, 5u, 2u},
    {0xcb6du, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_cb70[] = {
    {0xcb70u, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_cb73[] = {
    {0xcb73u, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_cb76[] = {
    {0xcb76u, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_cb79[] = {
    {0xcb79u, 2u, 4u, 2u},
};

static const FixedSpanOpcode round_selection_cb7d[] = {
    {0xcb7du, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_cb80[] = {
    {0xcb80u, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_cb83[] = {
    {0xcb83u, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_cb86[] = {
    {0xcb86u, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_cb89[] = {
    {0xcb89u, 3u, 5u, 2u},
    {0xcb8cu, 2u, 4u, 2u},
    {0xcb8eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_cb91[] = {
    {0xcb91u, 2u, 4u, 2u},
    {0xcb93u, 3u, 5u, 2u},
};

static const FixedSpanOpcode round_selection_cb98[] = {
    {0xcb98u, 3u, 5u, 2u},
    {0xcb9bu, 2u, 3u, 0u},
    {0xcbcbu, 1u, 4u, 1u},
};

static const FixedSpanOpcode round_selection_cb9d[] = {
    {0xcb9du, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_cba0[] = {
    {0xcba0u, 3u, 3u, 0u},
    {0xcba3u, 3u, 5u, 2u},
    {0xcba6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_cba9[] = {
    {0xcba9u, 3u, 3u, 0u},
    {0xcbacu, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_cbaf[] = {
    {0xcbafu, 3u, 5u, 2u},
    {0xcbb2u, 2u, 4u, 2u},
    {0xcbb4u, 3u, 3u, 0u},
    {0xcbb7u, 4u, 8u, 3u},
};

static const FixedSpanOpcode round_selection_cbbb[] = {
    {0xcbbbu, 3u, 3u, 0u},
    {0xcbbeu, 2u, 4u, 2u},
    {0xcbc0u, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_cbc0[] = {
    {0xcbc0u, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_cbc3[] = {
    {0xcbc3u, 3u, 6u, 2u},
};

static const FixedSpanOpcode round_selection_cbc6[] = {
    {0xcbc6u, 3u, 3u, 0u},
};

static const FixedSpanOpcode round_selection_cbcb[] = {
    {0xcbcbu, 1u, 4u, 1u},
};

static const RoundSelectionPhase round_selection_phases[] = {
    {0xb027u, 0x0000u, 0x0000u, 0u, sizeof round_selection_b027 / sizeof round_selection_b027[0], round_selection_b027},
    {0xb03eu, 0x0000u, 0x0000u, 1u, sizeof round_selection_b03e / sizeof round_selection_b03e[0], round_selection_b03e},
    {0xb041u, 0x0000u, 0x0000u, 1u, sizeof round_selection_b041 / sizeof round_selection_b041[0], round_selection_b041},
    {0xb046u, 0x0000u, 0x0000u, 1u, sizeof round_selection_b046 / sizeof round_selection_b046[0], round_selection_b046},
    {0xb049u, 0x0000u, 0x0000u, 1u, sizeof round_selection_b049 / sizeof round_selection_b049[0], round_selection_b049},
    {0xb088u, 0x0000u, 0x0000u, 0u, sizeof round_selection_b088 / sizeof round_selection_b088[0], round_selection_b088},
    {0xb094u, 0x0000u, 0x0000u, 0u, sizeof round_selection_b094 / sizeof round_selection_b094[0], round_selection_b094},
    {0xb09bu, 0xb0aau, 0x0000u, 0u, sizeof round_selection_b09b / sizeof round_selection_b09b[0], round_selection_b09b},
    {0xb0a4u, 0xb0aau, 0x0000u, 0u, sizeof round_selection_b0a4 / sizeof round_selection_b0a4[0], round_selection_b0a4},
    {0xb0acu, 0xb0adu, 0x0000u, 0u, sizeof round_selection_b0ac / sizeof round_selection_b0ac[0], round_selection_b0ac},
    {0xb0afu, 0x0000u, 0x0000u, 0u, sizeof round_selection_b0af / sizeof round_selection_b0af[0], round_selection_b0af},
    {0xb0d4u, 0x0000u, 0x0000u, 0u, sizeof round_selection_b0d4 / sizeof round_selection_b0d4[0], round_selection_b0d4},
    {0xb0e0u, 0xb0edu, 0x0000u, 0u, sizeof round_selection_b0e0 / sizeof round_selection_b0e0[0], round_selection_b0e0},
    {0xb0e8u, 0xb0edu, 0x0000u, 0u, sizeof round_selection_b0e8 / sizeof round_selection_b0e8[0], round_selection_b0e8},
    {0xb0efu, 0x0000u, 0x0000u, 0u, sizeof round_selection_b0ef / sizeof round_selection_b0ef[0], round_selection_b0ef},
    {0xb111u, 0x0000u, 0x0000u, 0u, sizeof round_selection_b111 / sizeof round_selection_b111[0], round_selection_b111},
    {0xb11du, 0x0000u, 0xb11du, 0u, 0u, NULL},
    {0xcb64u, 0x0000u, 0x0000u, 0u, sizeof round_selection_cb64 / sizeof round_selection_cb64[0], round_selection_cb64},
    {0xcb70u, 0x0000u, 0x0000u, 0u, sizeof round_selection_cb70 / sizeof round_selection_cb70[0], round_selection_cb70},
    {0xcb73u, 0x0000u, 0x0000u, 0u, sizeof round_selection_cb73 / sizeof round_selection_cb73[0], round_selection_cb73},
    {0xcb76u, 0x0000u, 0x0000u, 0u, sizeof round_selection_cb76 / sizeof round_selection_cb76[0], round_selection_cb76},
    {0xcb79u, 0xcb7bu, 0x0000u, 0u, sizeof round_selection_cb79 / sizeof round_selection_cb79[0], round_selection_cb79},
    {0xcb7du, 0x0000u, 0x0000u, 0u, sizeof round_selection_cb7d / sizeof round_selection_cb7d[0], round_selection_cb7d},
    {0xcb80u, 0x0000u, 0x0000u, 0u, sizeof round_selection_cb80 / sizeof round_selection_cb80[0], round_selection_cb80},
    {0xcb83u, 0x0000u, 0x0000u, 0u, sizeof round_selection_cb83 / sizeof round_selection_cb83[0], round_selection_cb83},
    {0xcb86u, 0x0000u, 0x0000u, 0u, sizeof round_selection_cb86 / sizeof round_selection_cb86[0], round_selection_cb86},
    {0xcb89u, 0x0000u, 0x0000u, 0u, sizeof round_selection_cb89 / sizeof round_selection_cb89[0], round_selection_cb89},
    {0xcb91u, 0xcb96u, 0x0000u, 0u, sizeof round_selection_cb91 / sizeof round_selection_cb91[0], round_selection_cb91},
    {0xcb98u, 0x0000u, 0xcbccu, 0u, sizeof round_selection_cb98 / sizeof round_selection_cb98[0], round_selection_cb98},
    {0xcb9du, 0x0000u, 0x0000u, 0u, sizeof round_selection_cb9d / sizeof round_selection_cb9d[0], round_selection_cb9d},
    {0xcba0u, 0x0000u, 0x0000u, 0u, sizeof round_selection_cba0 / sizeof round_selection_cba0[0], round_selection_cba0},
    {0xcba9u, 0x0000u, 0x0000u, 0u, sizeof round_selection_cba9 / sizeof round_selection_cba9[0], round_selection_cba9},
    {0xcbafu, 0x0000u, 0x0000u, 0u, sizeof round_selection_cbaf / sizeof round_selection_cbaf[0], round_selection_cbaf},
    {0xcbbbu, 0x0000u, 0x0000u, 0u, sizeof round_selection_cbbb / sizeof round_selection_cbbb[0], round_selection_cbbb},
    {0xcbc0u, 0x0000u, 0x0000u, 0u, sizeof round_selection_cbc0 / sizeof round_selection_cbc0[0], round_selection_cbc0},
    {0xcbc3u, 0x0000u, 0x0000u, 0u, sizeof round_selection_cbc3 / sizeof round_selection_cbc3[0], round_selection_cbc3},
    {0xcbc6u, 0xcbc9u, 0x0000u, 0u, sizeof round_selection_cbc6 / sizeof round_selection_cbc6[0], round_selection_cbc6},
    {0xcbcbu, 0x0000u, 0xcbccu, 0u, sizeof round_selection_cbcb / sizeof round_selection_cbcb[0], round_selection_cbcb},
};

typedef struct RoundSelectionProjection {
    uint16_t index;
    uint16_t remaining;
    unsigned scanned;
} RoundSelectionProjection;

static const RoundSelectionPhase *FindRoundSelectionPhase(uint16_t pc) {
    for (unsigned i = 0u; i < sizeof round_selection_phases / sizeof round_selection_phases[0]; ++i)
        if (round_selection_phases[i].pc == pc)
            return round_selection_phases + i;
    return NULL;
}

static uint16_t RoundSelectionWord(const CpuState *cpu, unsigned address) {
    return cpu->ram[address] | (uint16_t)cpu->ram[address + 1u] << 8;
}

static bool AppendRoundSelectionPhase(CardScoreClock *clock, uint16_t pc,
                                        RoundSelectionProjection projection) {
    const RoundSelectionPhase *phase = FindRoundSelectionPhase(pc);
    const CpuState *cpu = clock->call.cpu;
    if (!phase || clock->span.count + phase->count + 1u >= CARD_SCORE_CLOCK_CAPACITY)
        return false;
    if (pc == 0xb049u)
        projection.index = (uint16_t)(RoundSelectionWord(cpu, 0x689cu) * 2u);
    if (pc == 0xb09bu || pc == 0xb0e0u) {
        projection.index = 0u;
        projection.remaining = cpu->A;
        projection.scanned = 1u;
    }
    if (pc == 0xb0a4u || pc == 0xb0e8u) {
        if (++projection.scanned > 16u)
            return false;
        projection.index = (uint16_t)(projection.index + 2u);
    }
    if (pc == 0xb0acu)
        --projection.remaining;
    for (unsigned i = 0u; i < phase->count; ++i) {
        const FixedSpanOpcode *op = phase->opcodes + i;
        unsigned cycles = op->cycles;
        if (op->pc == 0xb06fu || op->pc == 0xb0b5u || op->pc == 0xb0f2u)
            cycles += (0x7036u >> 8) != ((0x7036u + projection.index) >> 8);
        if (op->pc == 0xb0a6u)
            cycles += (0x7000u >> 8) != ((0x7000u + projection.index) >> 8);
        CardScoreOpcode(clock, op->pc, op->fetched, cycles, op->data_bytes);
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
    case 0xb0aau:
        taken = ((uint16_t)(RoundSelectionWord(cpu, 0x7000u + projection.index) - 1u) & 0x8000u) != 0u;
        target = taken ? 0xb0a4u : 0xb0acu;
        break;
    case 0xb0adu:
        taken = !(projection.remaining & 0x8000u);
        target = taken ? 0xb0a4u : 0xb0afu;
        break;
    case 0xb0edu:
        taken = ((uint16_t)(RoundSelectionWord(cpu, 0x7000u + projection.index) - 1u) & 0x8000u) != 0u;
        target = taken ? 0xb0e8u : 0xb0efu;
        break;
    case 0xcb7bu:
        taken = RoundSelectionWord(cpu, 0x30u) != 0u;
        target = taken ? 0xcb73u : 0xcb7du;
        break;
    case 0xcb96u:
        taken = ((uint16_t)(RoundSelectionWord(cpu, 0x22u) - RoundSelectionWord(cpu, 0x71cau)) & 0x8000u) != 0u;
        target = taken ? 0xcb9du : 0xcb98u;
        break;
    case 0xcbc9u:
        taken = cpu->A == 0u;
        target = taken ? 0xcbc0u : 0xcbcbu;
        break;
    default:
        return false;
    }
    CardScoreOpcode(clock, phase->branch, 2u, taken ? 3u : 2u, 0u);
    return AppendRoundSelectionPhase(clock, target, projection);
}

static bool PrepareRoundSelection(CardScoreClock *clock, uint32_t pc) {
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
    const RoundSelectionPhase *phase = FindRoundSelectionPhase((uint16_t)pc);
    if (!phase || (pc != 0x84cb64u && cpu->m_flag != phase->byte_mode))
        return false;
    const RoundSelectionProjection projection = {cpu->X, cpu->Y, 0u};
    return AppendRoundSelectionPhase(clock, (uint16_t)pc, projection) &&
        CardScoreWindowSupported(clock);
}

static uint8_t RoundSelectionChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    CardScoreClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareRoundSelection(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeRoundSelection(CpuState *cpu, uint32_t entry,
                                             ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    CardScoreClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || !PrepareRoundSelection(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, RoundSelectionChild, &clock);
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
