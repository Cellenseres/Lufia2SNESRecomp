typedef struct PokerPlayerPhase {
    uint16_t pc;
    uint16_t branch;
    uint16_t return_pc;
    uint8_t byte_mode;
    unsigned count;
    const FixedSpanOpcode *opcodes;
} PokerPlayerPhase;

static const FixedSpanOpcode poker_player_a607[] = {
    {0xa607u, 4u, 8u, 3u},
};

static const FixedSpanOpcode poker_player_c028[] = {
    {0xc028u, 3u, 5u, 2u},
    {0xc02bu, 3u, 3u, 0u},
    {0xc02eu, 4u, 6u, 2u},
    {0xc032u, 1u, 2u, 0u},
    {0xc033u, 1u, 2u, 0u},
    {0xc034u, 1u, 2u, 0u},
    {0xc035u, 1u, 2u, 0u},
    {0xc036u, 4u, 6u, 2u},
    {0xc03au, 2u, 4u, 2u},
    {0xc03cu, 3u, 5u, 2u},
    {0xc03fu, 1u, 2u, 0u},
    {0xc040u, 2u, 4u, 2u},
};

static const FixedSpanOpcode poker_player_c044[] = {
    {0xc044u, 3u, 8u, 4u},
};

static const FixedSpanOpcode poker_player_c049[] = {
    {0xc049u, 3u, 5u, 2u},
    {0xc04cu, 1u, 2u, 0u},
    {0xc04du, 2u, 4u, 2u},
    {0xc04fu, 3u, 5u, 2u},
    {0xc052u, 2u, 3u, 0u},
    {0xc08du, 3u, 3u, 0u},
    {0xc090u, 3u, 5u, 2u},
    {0xc093u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c04f[] = {
    {0xc04fu, 3u, 5u, 2u},
    {0xc052u, 2u, 3u, 0u},
    {0xc08du, 3u, 3u, 0u},
    {0xc090u, 3u, 5u, 2u},
    {0xc093u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c054[] = {
    {0xc054u, 3u, 5u, 2u},
    {0xc057u, 3u, 3u, 0u},
    {0xc05au, 3u, 5u, 2u},
    {0xc05du, 3u, 3u, 0u},
    {0xc060u, 3u, 5u, 2u},
    {0xc063u, 3u, 3u, 0u},
    {0xc066u, 3u, 5u, 2u},
    {0xc069u, 3u, 3u, 0u},
    {0xc06cu, 3u, 5u, 2u},
    {0xc06fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c072[] = {
    {0xc072u, 3u, 3u, 0u},
    {0xc075u, 3u, 3u, 0u},
    {0xc078u, 4u, 8u, 3u},
};

static const FixedSpanOpcode poker_player_c07c[] = {
    {0xc07cu, 2u, 3u, 0u},
    {0xc07eu, 2u, 2u, 0u},
    {0xc080u, 2u, 3u, 1u},
    {0xc082u, 2u, 3u, 0u},
    {0xc084u, 3u, 3u, 0u},
    {0xc087u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c08a[] = {
    {0xc08au, 3u, 4u, 0u},
    {0xc1aeu, 3u, 5u, 2u},
    {0xc1b1u, 3u, 8u, 4u},
    {0xc1b4u, 2u, 3u, 0u},
    {0xc1c9u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c096[] = {
    {0xc096u, 3u, 5u, 2u},
    {0xc099u, 2u, 4u, 2u},
    {0xc09bu, 2u, 4u, 2u},
    {0xc09du, 3u, 5u, 2u},
    {0xc0a0u, 3u, 3u, 0u},
    {0xc0a3u, 3u, 5u, 2u},
    {0xc0a6u, 3u, 3u, 0u},
    {0xc0a9u, 3u, 5u, 2u},
    {0xc0acu, 3u, 3u, 0u},
    {0xc0afu, 3u, 5u, 2u},
    {0xc0b2u, 3u, 3u, 0u},
    {0xc0b5u, 3u, 5u, 2u},
    {0xc0b8u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c0bb[] = {
    {0xc0bbu, 3u, 3u, 0u},
    {0xc0beu, 3u, 3u, 0u},
    {0xc0c1u, 4u, 8u, 3u},
};

static const FixedSpanOpcode poker_player_c0c5[] = {
    {0xc0c5u, 2u, 3u, 0u},
    {0xc0c7u, 2u, 4u, 2u},
    {0xc0c9u, 1u, 2u, 0u},
    {0xc0cau, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_player_c0cf[] = {
    {0xc0cfu, 3u, 5u, 2u},
    {0xc0d2u, 3u, 3u, 0u},
    {0xc0d5u, 3u, 5u, 2u},
    {0xc0d8u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c0db[] = {
    {0xc0dbu, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_player_c0e0[] = {
    {0xc0e0u, 3u, 5u, 2u},
    {0xc0e3u, 1u, 2u, 0u},
    {0xc0e4u, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_player_c0e9[] = {
    {0xc0e9u, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_player_c0ee[] = {
    {0xc0eeu, 4u, 6u, 2u},
    {0xc0f2u, 2u, 3u, 0u},
    {0xc0f4u, 2u, 2u, 0u},
    {0xc0f6u, 4u, 5u, 1u},
    {0xc0fau, 1u, 2u, 0u},
    {0xc0fbu, 1u, 2u, 0u},
    {0xc0fcu, 1u, 2u, 0u},
    {0xc0fdu, 1u, 2u, 0u},
    {0xc0feu, 1u, 2u, 0u},
    {0xc0ffu, 1u, 2u, 0u},
    {0xc100u, 1u, 2u, 0u},
    {0xc101u, 1u, 2u, 0u},
    {0xc102u, 2u, 3u, 0u},
    {0xc104u, 4u, 6u, 2u},
    {0xc108u, 2u, 4u, 2u},
    {0xc10au, 2u, 3u, 0u},
    {0xc111u, 3u, 5u, 2u},
    {0xc114u, 1u, 2u, 0u},
    {0xc115u, 2u, 4u, 2u},
    {0xc117u, 3u, 3u, 0u},
};

static const FixedSpanOpcode poker_player_c10c[] = {
    {0xc10cu, 3u, 5u, 2u},
    {0xc10fu, 2u, 4u, 2u},
    {0xc111u, 3u, 5u, 2u},
    {0xc114u, 1u, 2u, 0u},
    {0xc115u, 2u, 4u, 2u},
    {0xc117u, 3u, 3u, 0u},
};

static const FixedSpanOpcode poker_player_c111[] = {
    {0xc111u, 3u, 5u, 2u},
    {0xc114u, 1u, 2u, 0u},
    {0xc115u, 2u, 4u, 2u},
    {0xc117u, 3u, 3u, 0u},
};

static const FixedSpanOpcode poker_player_c11c[] = {
    {0xc11cu, 3u, 3u, 0u},
    {0xc11fu, 1u, 2u, 0u},
    {0xc120u, 3u, 5u, 2u},
    {0xc123u, 2u, 4u, 2u},
    {0xc125u, 3u, 3u, 0u},
    {0xc128u, 2u, 4u, 2u},
    {0xc12au, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c128[] = {
    {0xc128u, 2u, 4u, 2u},
    {0xc12au, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c12d[] = {
    {0xc12du, 2u, 4u, 2u},
    {0xc12fu, 1u, 2u, 0u},
    {0xc130u, 2u, 4u, 2u},
    {0xc132u, 2u, 4u, 2u},
    {0xc134u, 2u, 4u, 2u},
    {0xc136u, 2u, 4u, 2u},
    {0xc138u, 3u, 3u, 0u},
    {0xc13bu, 3u, 3u, 0u},
    {0xc13eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c141[] = {
    {0xc141u, 2u, 4u, 2u},
    {0xc143u, 2u, 4u, 2u},
    {0xc145u, 3u, 3u, 0u},
    {0xc148u, 3u, 3u, 0u},
    {0xc14bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c14e[] = {
    {0xc14eu, 2u, 4u, 2u},
    {0xc150u, 1u, 2u, 0u},
    {0xc151u, 3u, 5u, 2u},
    {0xc154u, 2u, 4u, 2u},
    {0xc156u, 3u, 3u, 0u},
    {0xc159u, 3u, 3u, 0u},
    {0xc15cu, 4u, 8u, 3u},
};

static const FixedSpanOpcode poker_player_c160[] = {
    {0xc160u, 3u, 5u, 2u},
    {0xc163u, 2u, 4u, 2u},
};

static const FixedSpanOpcode poker_player_c167[] = {
    {0xc167u, 1u, 3u, 1u},
    {0xc168u, 2u, 3u, 0u},
    {0xc16au, 2u, 2u, 0u},
    {0xc16cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c16f[] = {
    {0xc16fu, 1u, 4u, 1u},
};

static const FixedSpanOpcode poker_player_c172[] = {
    {0xc172u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c175[] = {
    {0xc175u, 2u, 3u, 0u},
    {0xc17au, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c177[] = {
    {0xc177u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c17a[] = {
    {0xc17au, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c17d[] = {
    {0xc17du, 2u, 3u, 0u},
    {0xc17fu, 2u, 2u, 0u},
    {0xc181u, 2u, 3u, 1u},
    {0xc183u, 2u, 2u, 0u},
    {0xc185u, 2u, 3u, 1u},
    {0xc187u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c18a[] = {
    {0xc18au, 2u, 3u, 0u},
    {0xc18cu, 2u, 4u, 2u},
    {0xc18eu, 3u, 5u, 2u},
    {0xc191u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c194[] = {
    {0xc194u, 3u, 3u, 0u},
};

static const FixedSpanOpcode poker_player_c199[] = {
    {0xc199u, 3u, 4u, 0u},
    {0xc0c5u, 2u, 3u, 0u},
    {0xc0c7u, 2u, 4u, 2u},
    {0xc0c9u, 1u, 2u, 0u},
    {0xc0cau, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_player_c19c[] = {
    {0xc19cu, 2u, 4u, 2u},
    {0xc19eu, 2u, 3u, 0u},
    {0xc1a0u, 2u, 2u, 0u},
    {0xc1a2u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c1a5[] = {
    {0xc1a5u, 2u, 3u, 0u},
    {0xc1a7u, 2u, 4u, 2u},
    {0xc1a9u, 3u, 3u, 0u},
};

static const FixedSpanOpcode poker_player_c1ae[] = {
    {0xc1aeu, 3u, 5u, 2u},
    {0xc1b1u, 3u, 8u, 4u},
    {0xc1b4u, 2u, 3u, 0u},
    {0xc1c9u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c1b6[] = {
    {0xc1b6u, 2u, 4u, 2u},
    {0xc1b8u, 1u, 2u, 0u},
    {0xc1b9u, 3u, 5u, 2u},
    {0xc1bcu, 3u, 5u, 2u},
    {0xc1bfu, 1u, 2u, 0u},
    {0xc1c0u, 3u, 5u, 2u},
    {0xc1c3u, 3u, 5u, 2u},
    {0xc1c6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c1c9[] = {
    {0xc1c9u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c1cc[] = {
    {0xc1ccu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c1cf[] = {
    {0xc1cfu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c1d2[] = {
    {0xc1d2u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c1d5[] = {
    {0xc1d5u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c1d8[] = {
    {0xc1d8u, 2u, 3u, 0u},
    {0xc1dau, 2u, 2u, 0u},
    {0xc1dcu, 2u, 3u, 1u},
    {0xc1deu, 2u, 2u, 0u},
    {0xc1e0u, 2u, 3u, 1u},
    {0xc1e2u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_player_c1e5[] = {
    {0xc1e5u, 2u, 3u, 0u},
};

static const PokerPlayerPhase poker_player_phases[] = {
    {0xa607u, 0x0000u, 0x0000u, 0u, sizeof poker_player_a607 / sizeof poker_player_a607[0], poker_player_a607},
    {0xa60bu, 0x0000u, 0xa60bu, 0u, 0u, NULL},
    {0xc028u, 0xc042u, 0x0000u, 0u, sizeof poker_player_c028 / sizeof poker_player_c028[0], poker_player_c028},
    {0xc044u, 0xc047u, 0x0000u, 0u, sizeof poker_player_c044 / sizeof poker_player_c044[0], poker_player_c044},
    {0xc049u, 0x0000u, 0x0000u, 0u, sizeof poker_player_c049 / sizeof poker_player_c049[0], poker_player_c049},
    {0xc04fu, 0x0000u, 0x0000u, 0u, sizeof poker_player_c04f / sizeof poker_player_c04f[0], poker_player_c04f},
    {0xc054u, 0x0000u, 0x0000u, 0u, sizeof poker_player_c054 / sizeof poker_player_c054[0], poker_player_c054},
    {0xc072u, 0x0000u, 0x0000u, 0u, sizeof poker_player_c072 / sizeof poker_player_c072[0], poker_player_c072},
    {0xc07cu, 0x0000u, 0x0000u, 0u, sizeof poker_player_c07c / sizeof poker_player_c07c[0], poker_player_c07c},
    {0xc08au, 0x0000u, 0x0000u, 0u, sizeof poker_player_c08a / sizeof poker_player_c08a[0], poker_player_c08a},
    {0xc096u, 0x0000u, 0x0000u, 0u, sizeof poker_player_c096 / sizeof poker_player_c096[0], poker_player_c096},
    {0xc0bbu, 0x0000u, 0x0000u, 0u, sizeof poker_player_c0bb / sizeof poker_player_c0bb[0], poker_player_c0bb},
    {0xc0c5u, 0xc0cdu, 0x0000u, 0u, sizeof poker_player_c0c5 / sizeof poker_player_c0c5[0], poker_player_c0c5},
    {0xc0cfu, 0x0000u, 0x0000u, 0u, sizeof poker_player_c0cf / sizeof poker_player_c0cf[0], poker_player_c0cf},
    {0xc0dbu, 0xc0deu, 0x0000u, 0u, sizeof poker_player_c0db / sizeof poker_player_c0db[0], poker_player_c0db},
    {0xc0e0u, 0xc0e7u, 0x0000u, 0u, sizeof poker_player_c0e0 / sizeof poker_player_c0e0[0], poker_player_c0e0},
    {0xc0e9u, 0xc0ecu, 0x0000u, 0u, sizeof poker_player_c0e9 / sizeof poker_player_c0e9[0], poker_player_c0e9},
    {0xc0eeu, 0xc11au, 0x0000u, 0u, sizeof poker_player_c0ee / sizeof poker_player_c0ee[0], poker_player_c0ee},
    {0xc10cu, 0xc11au, 0x0000u, 0u, sizeof poker_player_c10c / sizeof poker_player_c10c[0], poker_player_c10c},
    {0xc111u, 0xc11au, 0x0000u, 0u, sizeof poker_player_c111 / sizeof poker_player_c111[0], poker_player_c111},
    {0xc11cu, 0x0000u, 0x0000u, 0u, sizeof poker_player_c11c / sizeof poker_player_c11c[0], poker_player_c11c},
    {0xc128u, 0x0000u, 0x0000u, 0u, sizeof poker_player_c128 / sizeof poker_player_c128[0], poker_player_c128},
    {0xc12du, 0x0000u, 0x0000u, 0u, sizeof poker_player_c12d / sizeof poker_player_c12d[0], poker_player_c12d},
    {0xc141u, 0x0000u, 0x0000u, 0u, sizeof poker_player_c141 / sizeof poker_player_c141[0], poker_player_c141},
    {0xc14eu, 0x0000u, 0x0000u, 0u, sizeof poker_player_c14e / sizeof poker_player_c14e[0], poker_player_c14e},
    {0xc160u, 0xc165u, 0x0000u, 0u, sizeof poker_player_c160 / sizeof poker_player_c160[0], poker_player_c160},
    {0xc167u, 0x0000u, 0x0000u, 0u, sizeof poker_player_c167 / sizeof poker_player_c167[0], poker_player_c167},
    {0xc16fu, 0xc170u, 0x0000u, 1u, sizeof poker_player_c16f / sizeof poker_player_c16f[0], poker_player_c16f},
    {0xc172u, 0x0000u, 0x0000u, 0u, sizeof poker_player_c172 / sizeof poker_player_c172[0], poker_player_c172},
    {0xc175u, 0x0000u, 0x0000u, 0u, sizeof poker_player_c175 / sizeof poker_player_c175[0], poker_player_c175},
    {0xc177u, 0x0000u, 0x0000u, 0u, sizeof poker_player_c177 / sizeof poker_player_c177[0], poker_player_c177},
    {0xc17au, 0x0000u, 0x0000u, 0u, sizeof poker_player_c17a / sizeof poker_player_c17a[0], poker_player_c17a},
    {0xc17du, 0x0000u, 0x0000u, 0u, sizeof poker_player_c17d / sizeof poker_player_c17d[0], poker_player_c17d},
    {0xc18au, 0x0000u, 0x0000u, 1u, sizeof poker_player_c18a / sizeof poker_player_c18a[0], poker_player_c18a},
    {0xc194u, 0xc197u, 0x0000u, 0u, sizeof poker_player_c194 / sizeof poker_player_c194[0], poker_player_c194},
    {0xc199u, 0xc0cdu, 0x0000u, 0u, sizeof poker_player_c199 / sizeof poker_player_c199[0], poker_player_c199},
    {0xc19cu, 0x0000u, 0x0000u, 0u, sizeof poker_player_c19c / sizeof poker_player_c19c[0], poker_player_c19c},
    {0xc1a5u, 0xc1acu, 0x0000u, 1u, sizeof poker_player_c1a5 / sizeof poker_player_c1a5[0], poker_player_c1a5},
    {0xc1aeu, 0x0000u, 0x0000u, 0u, sizeof poker_player_c1ae / sizeof poker_player_c1ae[0], poker_player_c1ae},
    {0xc1b6u, 0x0000u, 0x0000u, 0u, sizeof poker_player_c1b6 / sizeof poker_player_c1b6[0], poker_player_c1b6},
    {0xc1c9u, 0x0000u, 0x0000u, 0u, sizeof poker_player_c1c9 / sizeof poker_player_c1c9[0], poker_player_c1c9},
    {0xc1ccu, 0x0000u, 0x0000u, 0u, sizeof poker_player_c1cc / sizeof poker_player_c1cc[0], poker_player_c1cc},
    {0xc1cfu, 0x0000u, 0x0000u, 0u, sizeof poker_player_c1cf / sizeof poker_player_c1cf[0], poker_player_c1cf},
    {0xc1d2u, 0x0000u, 0x0000u, 0u, sizeof poker_player_c1d2 / sizeof poker_player_c1d2[0], poker_player_c1d2},
    {0xc1d5u, 0x0000u, 0x0000u, 0u, sizeof poker_player_c1d5 / sizeof poker_player_c1d5[0], poker_player_c1d5},
    {0xc1d8u, 0x0000u, 0x0000u, 0u, sizeof poker_player_c1d8 / sizeof poker_player_c1d8[0], poker_player_c1d8},
    {0xc1e5u, 0x0000u, 0xc1e7u, 1u, sizeof poker_player_c1e5 / sizeof poker_player_c1e5[0], poker_player_c1e5},
};

enum { POKER_PLAYER_CLOCK_CAPACITY = 128u };

typedef struct PokerPlayerClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[POKER_PLAYER_CLOCK_CAPACITY];
    ActorPushedCall call;
    uint32_t next;
    bool returning;
} PokerPlayerClock;

typedef struct PokerPlayerProjection {
    uint16_t balance;
    uint16_t cost;
    uint16_t raise;
} PokerPlayerProjection;

static uint16_t PokerPlayerWord(const CpuState *cpu, unsigned address) {
    return cpu->ram[address] | (uint16_t)cpu->ram[address + 1u] << 8;
}

static const PokerPlayerPhase *FindPokerPlayerPhase(uint16_t pc) {
    for (unsigned i = 0u; i < sizeof poker_player_phases / sizeof poker_player_phases[0]; ++i)
        if (poker_player_phases[i].pc == pc) return poker_player_phases + i;
    return NULL;
}

static bool AppendPokerPlayerPhase(PokerPlayerClock *clock, uint16_t pc,
                                   PokerPlayerProjection projection) {
    const PokerPlayerPhase *phase = FindPokerPlayerPhase(pc);
    const CpuState *cpu = clock->call.cpu;
    if (!phase || clock->span.count + phase->count + 1u >= POKER_PLAYER_CLOCK_CAPACITY)
        return false;
    for (unsigned i = 0u; i < phase->count; ++i)
        clock->timing[clock->span.count++] = phase->opcodes[i];
    if (phase->return_pc) {
        clock->returning = true;
        clock->next = 0x840000u | phase->return_pc;
    }
    if (!phase->branch) return true;
    bool taken;
    uint16_t target;
    switch (phase->branch) {
    case 0xc042u:
        taken = projection.balance >= projection.cost; target = 0xc04fu; break;
    case 0xc047u:
        taken = ((uint16_t)(PokerPlayerWord(cpu, 0xb57u) - 1u) & 0x8000u) != 0u;
        target = 0xc054u; break;
    case 0xc0cdu:
        taken = projection.raise < PokerPlayerWord(cpu, 0x69e0u); target = 0xc111u; break;
    case 0xc0deu:
        taken = PokerPlayerWord(cpu, 0xb57u) != 0u; target = 0xc111u; break;
    case 0xc0e7u:
        taken = projection.balance >= PokerPlayerWord(cpu, 0x600cu); target = 0xc111u; break;
    case 0xc0ecu:
        taken = !projection.balance; target = 0xc10cu;
        projection.raise = taken ? PokerPlayerWord(cpu, 0x69e0u) : projection.balance / 10u;
        break;
    case 0xc11au:
        taken = ((uint16_t)(PokerPlayerWord(cpu, 0x7127u) + projection.raise - 99u) & 0x8000u) != 0u;
        target = 0xc128u; break;
    case 0xc165u:
        taken = PokerPlayerWord(cpu, 0x69e0u) == projection.raise; target = 0xc17au; break;
    case 0xc170u: {
        const uint8_t saved = cpu->ram[(uint16_t)(cpu->S + 1u)];
        if (saved & 0x38u) return false;
        taken = !(saved & 0x80u); target = 0xc177u; break;
    }
    case 0xc197u:
        taken = cpu->A != 3u; target = 0xc19cu; break;
    case 0xc1acu:
        taken = PokerPlayerWord(cpu, 0u) != 2u; target = 0xc1b6u; break;
    default: return false;
    }
    clock->timing[clock->span.count++] = (FixedSpanOpcode){phase->branch, 2u,
        (uint8_t)(taken ? 3u : 2u), 0u};
    return AppendPokerPlayerPhase(clock, taken ? target : (uint16_t)(phase->branch + 2u), projection);
}

static bool PokerPlayerWindowSupported(const PokerPlayerClock *clock) {
    CpuState *cpu = clock->call.cpu;
    unsigned budget = 64u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(clock->span.bank | op->pc) +
            op->data_bytes * 12u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x84c028u, 0x1ffcu, true, 1363u, budget) ||
        !g_snes->hdmaBeamOff || g_snes->vPos >= 262u) return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if (lines * 1364u - g_snes->hPos <= budget) return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool PreparePokerPlayerClock(PokerPlayerClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->next = pc;
    clock->returning = false;
    if (!cpu->ram || !g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x28000u ||
        pc >> 16 != 0x84u || cpu->PB != 0x84u || cpu->emulation ||
        cpu->m_flag > 1u || cpu->x_flag > 1u || cpu->host_return_valid < 2u ||
        cpu->host_return_valid > 3u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu)
        return false;
    const PokerPlayerPhase *phase = FindPokerPlayerPhase((uint16_t)pc);
    if (!phase) return false;
    if (pc != 0x84a607u && pc != 0x84a60bu) {
        if (cpu->x_flag || cpu->D || cpu->_flag_D || cpu->DB != 0x7eu) return false;
        const bool normalizes = pc == 0x84c07cu || pc == 0x84c0c5u ||
            pc == 0x84c16fu || pc == 0x84c18au || pc == 0x84c1a5u || pc == 0x84c1e5u;
        if (!normalizes && cpu->m_flag != phase->byte_mode) return false;
    }
    const uint16_t stake = PokerPlayerWord(cpu, 0x7127u) | 0x0a00u;
    const PokerPlayerProjection projection = {PokerPlayerWord(cpu, 0xb55u),
        (uint16_t)((stake & 255u) * (stake >> 8)), PokerPlayerWord(cpu, 0xbu)};
    return AppendPokerPlayerPhase(clock, (uint16_t)pc, projection) && PokerPlayerWindowSupported(clock);
}

static uint8_t PokerPlayerChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    PokerPlayerClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size)) return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PreparePokerPlayerClock(clock, landing)) return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgePokerPlayer(CpuState *cpu, uint32_t entry,
                                           ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    PokerPlayerClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || !PreparePokerPlayerClock(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span,
        SpecialArithmeticWait, &clock.span};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, PokerPlayerChild, &clock);
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
