typedef struct PokerAnimationPhase {
    uint16_t pc;
    uint16_t branch;
    uint16_t return_pc;
    uint8_t byte_mode;
    unsigned count;
    const FixedSpanOpcode *opcodes;
} PokerAnimationPhase;

static const FixedSpanOpcode poker_animation_c5af[] = {
    {0xc5afu, 1u, 3u, 1u},
    {0xc5b0u, 2u, 3u, 0u},
    {0xc5b2u, 2u, 2u, 0u},
    {0xc5b4u, 2u, 3u, 1u},
    {0xc5b6u, 3u, 3u, 0u},
    {0xc5b9u, 2u, 4u, 2u},
    {0xc5bbu, 2u, 3u, 0u},
    {0xc5bdu, 2u, 4u, 2u},
    {0xc5bfu, 1u, 2u, 0u},
    {0xc5c0u, 1u, 2u, 0u},
    {0xc5c1u, 1u, 2u, 0u},
    {0xc5c2u, 2u, 4u, 2u},
    {0xc5c4u, 1u, 2u, 0u},
    {0xc5c5u, 2u, 4u, 2u},
    {0xc5c7u, 2u, 4u, 2u},
    {0xc5c9u, 3u, 5u, 2u},
    {0xc5ccu, 1u, 2u, 0u},
    {0xc5cdu, 1u, 2u, 0u},
    {0xc5ceu, 1u, 2u, 0u},
    {0xc5cfu, 1u, 2u, 0u},
    {0xc5d0u, 2u, 4u, 2u},
    {0xc5d2u, 1u, 2u, 0u},
    {0xc5d3u, 2u, 4u, 2u},
    {0xc5d5u, 1u, 2u, 0u},
    {0xc5d6u, 2u, 4u, 2u},
    {0xc5d8u, 1u, 2u, 0u},
    {0xc5d9u, 2u, 4u, 2u},
    {0xc5dbu, 2u, 7u, 5u},
    {0xc5ddu, 2u, 4u, 2u},
    {0xc5dfu, 1u, 2u, 0u},
    {0xc5e0u, 1u, 2u, 0u},
    {0xc5e1u, 2u, 7u, 5u},
    {0xc5e3u, 2u, 4u, 2u},
    {0xc5e5u, 1u, 2u, 0u},
    {0xc5e6u, 1u, 2u, 0u},
    {0xc5e7u, 2u, 7u, 5u},
    {0xc5e9u, 2u, 4u, 2u},
    {0xc5ebu, 2u, 4u, 2u},
    {0xc5edu, 2u, 3u, 0u},
    {0xc5efu, 2u, 2u, 0u},
    {0xc5f1u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c5f4[] = {
    {0xc5f4u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c5f7[] = {
    {0xc5f7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c5fa[] = {
    {0xc5fau, 2u, 2u, 0u},
    {0xc5fcu, 2u, 3u, 1u},
    {0xc5feu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c601[] = {
    {0xc601u, 2u, 4u, 2u},
    {0xc603u, 2u, 4u, 2u},
    {0xc605u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c605[] = {
    {0xc605u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c608[] = {
    {0xc608u, 2u, 2u, 0u},
    {0xc60au, 2u, 3u, 1u},
    {0xc60cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c60f[] = {
    {0xc60fu, 2u, 5u, 2u},
};

static const FixedSpanOpcode poker_animation_c613[] = {
    {0xc613u, 2u, 3u, 0u},
    {0xc615u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c618[] = {
    {0xc618u, 2u, 4u, 2u},
    {0xc61au, 2u, 4u, 2u},
    {0xc61cu, 2u, 4u, 2u},
    {0xc61eu, 2u, 4u, 2u},
};

static const FixedSpanOpcode poker_animation_c622[] = {
    {0xc622u, 2u, 4u, 2u},
};

static const FixedSpanOpcode poker_animation_c626[] = {
    {0xc626u, 3u, 4u, 0u},
    {0xc62eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c629[] = {
    {0xc629u, 2u, 7u, 4u},
    {0xc62bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c62e[] = {
    {0xc62eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c631[] = {
    {0xc631u, 2u, 4u, 2u},
    {0xc633u, 2u, 4u, 2u},
    {0xc635u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c638[] = {
    {0xc638u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c63b[] = {
    {0xc63bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c63e[] = {
    {0xc63eu, 2u, 3u, 0u},
    {0xc640u, 2u, 2u, 0u},
    {0xc642u, 2u, 3u, 1u},
    {0xc644u, 2u, 3u, 1u},
    {0xc646u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c649[] = {
    {0xc649u, 1u, 4u, 1u},
};

static const FixedSpanOpcode poker_animation_c64b[] = {
    {0xc64bu, 3u, 5u, 2u},
    {0xc64eu, 3u, 3u, 0u},
    {0xc651u, 3u, 5u, 2u},
    {0xc654u, 3u, 3u, 0u},
    {0xc657u, 3u, 5u, 2u},
    {0xc65au, 3u, 8u, 4u},
};

static const FixedSpanOpcode poker_animation_c65a[] = {
    {0xc65au, 3u, 8u, 4u},
};

static const FixedSpanOpcode poker_animation_c65f[] = {
    {0xc65fu, 3u, 3u, 0u},
    {0xc662u, 3u, 5u, 2u},
    {0xc665u, 3u, 5u, 2u},
    {0xc668u, 1u, 2u, 0u},
    {0xc669u, 3u, 3u, 0u},
};

static const FixedSpanOpcode poker_animation_c66e[] = {
    {0xc66eu, 3u, 3u, 0u},
    {0xc671u, 3u, 5u, 2u},
    {0xc674u, 2u, 3u, 0u},
    {0xc676u, 2u, 2u, 0u},
    {0xc678u, 2u, 3u, 1u},
    {0xc67au, 2u, 3u, 0u},
    {0xc67cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c671[] = {
    {0xc671u, 3u, 5u, 2u},
    {0xc674u, 2u, 3u, 0u},
    {0xc676u, 2u, 2u, 0u},
    {0xc678u, 2u, 3u, 1u},
    {0xc67au, 2u, 3u, 0u},
    {0xc67cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c67c[] = {
    {0xc67cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c67f[] = {
    {0xc67fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c682[] = {
    {0xc682u, 3u, 3u, 0u},
};

static const FixedSpanOpcode poker_animation_c687[] = {
    {0xc687u, 3u, 3u, 0u},
    {0xc68au, 3u, 5u, 2u},
    {0xc68du, 2u, 3u, 0u},
    {0xc68fu, 2u, 2u, 0u},
    {0xc691u, 2u, 3u, 1u},
    {0xc693u, 2u, 3u, 0u},
    {0xc695u, 3u, 3u, 0u},
    {0xc698u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c73a[] = {
    {0xc73au, 3u, 3u, 0u},
    {0xc73du, 2u, 4u, 2u},
    {0xc73fu, 2u, 4u, 2u},
    {0xc741u, 1u, 2u, 0u},
    {0xc742u, 1u, 2u, 0u},
    {0xc743u, 2u, 4u, 2u},
    {0xc745u, 1u, 2u, 0u},
    {0xc746u, 4u, 6u, 2u},
    {0xc74au, 2u, 4u, 2u},
    {0xc74cu, 3u, 5u, 2u},
    {0xc74fu, 1u, 2u, 0u},
    {0xc750u, 2u, 4u, 2u},
    {0xc752u, 1u, 2u, 0u},
    {0xc753u, 1u, 2u, 0u},
    {0xc754u, 1u, 2u, 0u},
    {0xc755u, 1u, 2u, 0u},
    {0xc756u, 1u, 2u, 0u},
    {0xc757u, 2u, 4u, 2u},
    {0xc759u, 1u, 2u, 0u},
    {0xc75au, 2u, 4u, 2u},
    {0xc75cu, 3u, 3u, 0u},
    {0xc75fu, 2u, 4u, 2u},
    {0xc761u, 3u, 5u, 2u},
    {0xc764u, 2u, 4u, 2u},
    {0xc766u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c73f[] = {
    {0xc73fu, 2u, 4u, 2u},
    {0xc741u, 1u, 2u, 0u},
    {0xc742u, 1u, 2u, 0u},
    {0xc743u, 2u, 4u, 2u},
    {0xc745u, 1u, 2u, 0u},
    {0xc746u, 4u, 6u, 2u},
    {0xc74au, 2u, 4u, 2u},
    {0xc74cu, 3u, 5u, 2u},
    {0xc74fu, 1u, 2u, 0u},
    {0xc750u, 2u, 4u, 2u},
    {0xc752u, 1u, 2u, 0u},
    {0xc753u, 1u, 2u, 0u},
    {0xc754u, 1u, 2u, 0u},
    {0xc755u, 1u, 2u, 0u},
    {0xc756u, 1u, 2u, 0u},
    {0xc757u, 2u, 4u, 2u},
    {0xc759u, 1u, 2u, 0u},
    {0xc75au, 2u, 4u, 2u},
    {0xc75cu, 3u, 3u, 0u},
    {0xc75fu, 2u, 4u, 2u},
    {0xc761u, 3u, 5u, 2u},
    {0xc764u, 2u, 4u, 2u},
    {0xc766u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c75c[] = {
    {0xc75cu, 3u, 3u, 0u},
    {0xc75fu, 2u, 4u, 2u},
    {0xc761u, 3u, 5u, 2u},
    {0xc764u, 2u, 4u, 2u},
    {0xc766u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c769[] = {
    {0xc769u, 3u, 3u, 0u},
    {0xc76cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c76f[] = {
    {0xc76fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c772[] = {
    {0xc772u, 2u, 4u, 2u},
    {0xc774u, 1u, 2u, 0u},
    {0xc775u, 3u, 3u, 0u},
    {0xc778u, 2u, 4u, 2u},
    {0xc77au, 2u, 4u, 2u},
    {0xc77cu, 1u, 2u, 0u},
    {0xc77du, 1u, 2u, 0u},
    {0xc77eu, 2u, 4u, 2u},
    {0xc780u, 2u, 7u, 4u},
};

static const FixedSpanOpcode poker_animation_c784[] = {
    {0xc784u, 2u, 3u, 0u},
    {0xc786u, 2u, 2u, 0u},
    {0xc788u, 2u, 3u, 1u},
    {0xc78au, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c78d[] = {
    {0xc78du, 2u, 3u, 0u},
    {0xc78fu, 2u, 7u, 4u},
};

static const FixedSpanOpcode poker_animation_c794[] = {
    {0xc794u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c797[] = {
    {0xc797u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_animation_c79a[] = {
    {0xc79au, 3u, 3u, 0u},
};

static const FixedSpanOpcode poker_animation_c79f[] = {
    {0xc79fu, 3u, 3u, 0u},
    {0xc7a2u, 3u, 6u, 2u},
};

static const PokerAnimationPhase poker_animation_phases[] = {
    {0xc5afu, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c5af / sizeof poker_animation_c5af[0], poker_animation_c5af},
    {0xc5f4u, 0x0000u, 0x0000u, 1u, sizeof poker_animation_c5f4 / sizeof poker_animation_c5f4[0], poker_animation_c5f4},
    {0xc5f7u, 0x0000u, 0x0000u, 1u, sizeof poker_animation_c5f7 / sizeof poker_animation_c5f7[0], poker_animation_c5f7},
    {0xc5fau, 0x0000u, 0x0000u, 1u, sizeof poker_animation_c5fa / sizeof poker_animation_c5fa[0], poker_animation_c5fa},
    {0xc601u, 0x0000u, 0x0000u, 1u, sizeof poker_animation_c601 / sizeof poker_animation_c601[0], poker_animation_c601},
    {0xc605u, 0x0000u, 0x0000u, 1u, sizeof poker_animation_c605 / sizeof poker_animation_c605[0], poker_animation_c605},
    {0xc608u, 0x0000u, 0x0000u, 1u, sizeof poker_animation_c608 / sizeof poker_animation_c608[0], poker_animation_c608},
    {0xc60fu, 0xc611u, 0x0000u, 1u, sizeof poker_animation_c60f / sizeof poker_animation_c60f[0], poker_animation_c60f},
    {0xc613u, 0x0000u, 0x0000u, 1u, sizeof poker_animation_c613 / sizeof poker_animation_c613[0], poker_animation_c613},
    {0xc618u, 0xc620u, 0x0000u, 0u, sizeof poker_animation_c618 / sizeof poker_animation_c618[0], poker_animation_c618},
    {0xc622u, 0xc624u, 0x0000u, 0u, sizeof poker_animation_c622 / sizeof poker_animation_c622[0], poker_animation_c622},
    {0xc626u, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c626 / sizeof poker_animation_c626[0], poker_animation_c626},
    {0xc629u, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c629 / sizeof poker_animation_c629[0], poker_animation_c629},
    {0xc62eu, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c62e / sizeof poker_animation_c62e[0], poker_animation_c62e},
    {0xc631u, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c631 / sizeof poker_animation_c631[0], poker_animation_c631},
    {0xc638u, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c638 / sizeof poker_animation_c638[0], poker_animation_c638},
    {0xc63bu, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c63b / sizeof poker_animation_c63b[0], poker_animation_c63b},
    {0xc63eu, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c63e / sizeof poker_animation_c63e[0], poker_animation_c63e},
    {0xc649u, 0x0000u, 0xc64au, 1u, sizeof poker_animation_c649 / sizeof poker_animation_c649[0], poker_animation_c649},
    {0xc64bu, 0xc65du, 0x0000u, 0u, sizeof poker_animation_c64b / sizeof poker_animation_c64b[0], poker_animation_c64b},
    {0xc65au, 0xc65du, 0x0000u, 0u, sizeof poker_animation_c65a / sizeof poker_animation_c65a[0], poker_animation_c65a},
    {0xc65fu, 0xc66cu, 0x0000u, 0u, sizeof poker_animation_c65f / sizeof poker_animation_c65f[0], poker_animation_c65f},
    {0xc66eu, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c66e / sizeof poker_animation_c66e[0], poker_animation_c66e},
    {0xc671u, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c671 / sizeof poker_animation_c671[0], poker_animation_c671},
    {0xc67cu, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c67c / sizeof poker_animation_c67c[0], poker_animation_c67c},
    {0xc67fu, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c67f / sizeof poker_animation_c67f[0], poker_animation_c67f},
    {0xc682u, 0xc685u, 0x0000u, 0u, sizeof poker_animation_c682 / sizeof poker_animation_c682[0], poker_animation_c682},
    {0xc687u, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c687 / sizeof poker_animation_c687[0], poker_animation_c687},
    {0xc69bu, 0x0000u, 0xc69bu, 0u, 0u, NULL},
    {0xc73au, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c73a / sizeof poker_animation_c73a[0], poker_animation_c73a},
    {0xc73fu, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c73f / sizeof poker_animation_c73f[0], poker_animation_c73f},
    {0xc75cu, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c75c / sizeof poker_animation_c75c[0], poker_animation_c75c},
    {0xc769u, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c769 / sizeof poker_animation_c769[0], poker_animation_c769},
    {0xc76fu, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c76f / sizeof poker_animation_c76f[0], poker_animation_c76f},
    {0xc772u, 0xc782u, 0x0000u, 0u, sizeof poker_animation_c772 / sizeof poker_animation_c772[0], poker_animation_c772},
    {0xc784u, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c784 / sizeof poker_animation_c784[0], poker_animation_c784},
    {0xc78du, 0xc791u, 0x0000u, 1u, sizeof poker_animation_c78d / sizeof poker_animation_c78d[0], poker_animation_c78d},
    {0xc793u, 0x0000u, 0xc793u, 0u, 0u, NULL},
    {0xc794u, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c794 / sizeof poker_animation_c794[0], poker_animation_c794},
    {0xc797u, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c797 / sizeof poker_animation_c797[0], poker_animation_c797},
    {0xc79au, 0xc79du, 0x0000u, 0u, sizeof poker_animation_c79a / sizeof poker_animation_c79a[0], poker_animation_c79a},
    {0xc79fu, 0x0000u, 0x0000u, 0u, sizeof poker_animation_c79f / sizeof poker_animation_c79f[0], poker_animation_c79f},
    {0xc7a5u, 0x0000u, 0xc7a5u, 0u, 0u, NULL},
};

static const PokerAnimationPhase *FindPokerAnimationPhase(uint16_t pc) {
    for (unsigned i = 0u; i < sizeof poker_animation_phases / sizeof poker_animation_phases[0]; ++i)
        if (poker_animation_phases[i].pc == pc)
            return poker_animation_phases + i;
    return NULL;
}

static uint16_t PokerAnimationWord(const CpuState *cpu, unsigned address) {
    return cpu->ram[address] | (uint16_t)cpu->ram[address + 1u] << 8;
}

static void AppendPokerAnimationPhase(CardScoreClock *clock, uint16_t pc,
                                      unsigned card_index) {
    const PokerAnimationPhase *phase = FindPokerAnimationPhase(pc);
    const CpuState *cpu = clock->call.cpu;
    assert(phase);
    if (pc == 0xc73au || pc == 0xc73fu) {
        const unsigned hand = pc == 0xc73au ? 3u : card_index / 10u;
        card_index = (uint16_t)(hand * 10u);
    }
    for (unsigned i = 0u; i < phase->count; ++i) {
        const FixedSpanOpcode *op = phase->opcodes + i;
        unsigned cycles = op->cycles;
        if (op->pc == 0xc5c9u || op->pc == 0xc74cu || op->pc == 0xc761u) {
            const unsigned base = op->pc == 0xc761u ? 0x709eu : 0x70c6u;
            const unsigned hand = op->pc == 0xc5c9u ? PokerAnimationWord(cpu, 0x17u)
                : pc == 0xc73au ? 3u : card_index / 10u;
            const unsigned index = op->pc == 0xc761u ? card_index : (uint16_t)(hand * 2u);
            cycles += (base >> 8) != ((base + index) >> 8);
        }
        CardScoreOpcode(clock, op->pc, op->fetched, cycles, op->data_bytes);
    }
    if (phase->return_pc) {
        clock->returning = true;
        clock->next = 0x840000u | phase->return_pc;
    }
    if (!phase->branch)
        return;
    bool taken;
    uint16_t target;
    switch (phase->branch) {
    case 0xc611u:
        taken = cpu->ram[4u] != 1u;
        target = taken ? 0xc605u : 0xc613u;
        break;
    case 0xc620u:
        taken = PokerAnimationWord(cpu, 0x30u) != 0u;
        target = taken ? 0xc629u : 0xc622u;
        break;
    case 0xc624u:
        taken = PokerAnimationWord(cpu, 0x17u) == 0u;
        target = taken ? 0xc629u : 0xc626u;
        break;
    case 0xc65du:
        taken = pc == 0xc64bu ||
            !((uint16_t)(PokerAnimationWord(cpu, 0x7131u) - 1u) & 0x8000u);
        target = taken ? 0xc67cu : 0xc65fu;
        break;
    case 0xc66cu:
        taken = (uint16_t)(PokerAnimationWord(cpu, 0x2feu) + 1u) != 0x3508u;
        target = taken ? 0xc671u : 0xc66eu;
        break;
    case 0xc685u:
        taken = cpu->A == 0u;
        target = taken ? 0xc65au : 0xc687u;
        break;
    case 0xc782u:
        taken = !((uint16_t)(PokerAnimationWord(cpu, 0x13u) - 1u) & 0x8000u);
        target = taken ? 0xc75cu : 0xc784u;
        card_index = (uint16_t)(PokerAnimationWord(cpu, 0x11u) + 2u);
        break;
    case 0xc791u:
        taken = PokerAnimationWord(cpu, 2u) != 1u;
        target = taken ? 0xc73fu : 0xc793u;
        card_index = (uint16_t)((PokerAnimationWord(cpu, 2u) - 1u) * 10u);
        break;
    case 0xc79du:
        taken = cpu->A == 0u;
        target = taken ? 0xc794u : 0xc79fu;
        break;
    default:
        assert(false);
        return;
    }
    CardScoreOpcode(clock, phase->branch, 2u, taken ? 3u : 2u, 0u);
    AppendPokerAnimationPhase(clock, target, card_index);
}

static bool PreparePokerAnimation(CardScoreClock *clock, uint32_t pc) {
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
    const PokerAnimationPhase *phase = FindPokerAnimationPhase((uint16_t)pc);
    if (!phase || (pc != 0x84c5afu && cpu->m_flag != phase->byte_mode))
        return false;
    AppendPokerAnimationPhase(clock, (uint16_t)pc, cpu->X);
    return CardScoreWindowSupported(clock);
}

static uint8_t PokerAnimationChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    CardScoreClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PreparePokerAnimation(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgePokerAnimation(CpuState *cpu, uint32_t entry,
                                             ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    CardScoreClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || !PreparePokerAnimation(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, PokerAnimationChild, &clock);
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
