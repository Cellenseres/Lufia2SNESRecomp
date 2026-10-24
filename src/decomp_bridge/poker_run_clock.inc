typedef struct PokerRunPhase {
    uint16_t pc;
    uint16_t branch;
    uint16_t return_pc;
    uint8_t byte_mode;
    unsigned count;
    const FixedSpanOpcode *opcodes;
} PokerRunPhase;

static const FixedSpanOpcode poker_run_b827[] = {
    {0xb827u, 1u, 3u, 1u},
    {0xb828u, 2u, 3u, 0u},
    {0xb82au, 2u, 2u, 0u},
    {0xb82cu, 3u, 4u, 1u},
    {0xb82fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b832[] = {
    {0xb832u, 1u, 3u, 1u},
    {0xb833u, 2u, 2u, 0u},
    {0xb835u, 1u, 3u, 1u},
    {0xb836u, 1u, 4u, 1u},
    {0xb837u, 3u, 3u, 0u},
    {0xb83au, 3u, 5u, 2u},
    {0xb83du, 2u, 3u, 0u},
    {0xb83fu, 3u, 5u, 2u},
    {0xb842u, 3u, 3u, 0u},
    {0xb845u, 3u, 5u, 2u},
    {0xb848u, 1u, 2u, 0u},
    {0xb849u, 3u, 5u, 2u},
    {0xb84cu, 1u, 2u, 0u},
    {0xb84du, 3u, 5u, 2u},
    {0xb850u, 3u, 3u, 0u},
    {0xb853u, 3u, 5u, 2u},
    {0xb856u, 3u, 3u, 0u},
    {0xb859u, 3u, 5u, 2u},
    {0xb85cu, 3u, 3u, 0u},
    {0xb85fu, 3u, 5u, 2u},
    {0xb862u, 2u, 3u, 0u},
    {0xb864u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b867[] = {
    {0xb867u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b86a[] = {
    {0xb86au, 3u, 5u, 2u},
    {0xb86du, 1u, 2u, 0u},
    {0xb86eu, 3u, 3u, 0u},
};

static const FixedSpanOpcode poker_run_b873[] = {
    {0xb873u, 3u, 5u, 2u},
    {0xb876u, 1u, 2u, 0u},
};

static const FixedSpanOpcode poker_run_b879[] = {
    {0xb879u, 3u, 4u, 0u},
    {0xb9e6u, 3u, 3u, 0u},
    {0xb9e9u, 3u, 5u, 2u},
    {0xb9ecu, 3u, 3u, 0u},
    {0xb9efu, 3u, 5u, 2u},
    {0xb9f2u, 3u, 3u, 0u},
    {0xb9f5u, 3u, 5u, 2u},
    {0xb9f8u, 3u, 3u, 0u},
    {0xb9fbu, 3u, 5u, 2u},
    {0xb9feu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b87c[] = {
    {0xb87cu, 3u, 5u, 2u},
    {0xb87fu, 3u, 5u, 2u},
    {0xb882u, 1u, 2u, 0u},
    {0xb883u, 3u, 3u, 0u},
    {0xb886u, 3u, 5u, 2u},
    {0xb889u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b886[] = {
    {0xb886u, 3u, 5u, 2u},
    {0xb889u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b88c[] = {
    {0xb88cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b88f[] = {
    {0xb88fu, 2u, 3u, 0u},
    {0xb891u, 2u, 2u, 0u},
    {0xb893u, 2u, 3u, 1u},
    {0xb895u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b898[] = {
    {0xb898u, 2u, 3u, 0u},
    {0xb89au, 2u, 4u, 2u},
    {0xb89cu, 2u, 4u, 2u},
    {0xb89eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b89e[] = {
    {0xb89eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b8a1[] = {
    {0xb8a1u, 2u, 7u, 4u},
    {0xb8a3u, 3u, 3u, 0u},
    {0xb8a6u, 2u, 4u, 2u},
};

static const FixedSpanOpcode poker_run_b8aa[] = {
    {0xb8aau, 2u, 7u, 4u},
    {0xb8acu, 2u, 4u, 2u},
    {0xb8aeu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b8ae[] = {
    {0xb8aeu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b8b1[] = {
    {0xb8b1u, 2u, 7u, 4u},
    {0xb8b3u, 3u, 3u, 0u},
    {0xb8b6u, 2u, 4u, 2u},
};

static const FixedSpanOpcode poker_run_b8ba[] = {
    {0xb8bau, 3u, 5u, 2u},
    {0xb8bdu, 3u, 5u, 2u},
    {0xb8c0u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b8c3[] = {
    {0xb8c3u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b8c6[] = {
    {0xb8c6u, 2u, 3u, 0u},
    {0xb8c8u, 2u, 2u, 0u},
    {0xb8cau, 2u, 3u, 1u},
    {0xb8ccu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b8cf[] = {
    {0xb8cfu, 2u, 3u, 0u},
    {0xb8d1u, 3u, 3u, 0u},
    {0xb8d4u, 2u, 4u, 2u},
    {0xb8d6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b8d6[] = {
    {0xb8d6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b8d9[] = {
    {0xb8d9u, 2u, 7u, 4u},
    {0xb8dbu, 3u, 3u, 0u},
    {0xb8deu, 2u, 4u, 2u},
};

static const FixedSpanOpcode poker_run_b8e2[] = {
    {0xb8e2u, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_run_b8e7[] = {
    {0xb8e7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b8ea[] = {
    {0xb8eau, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b8ed[] = {
    {0xb8edu, 3u, 5u, 2u},
    {0xb8f0u, 1u, 2u, 0u},
    {0xb8f1u, 3u, 5u, 2u},
    {0xb8f4u, 3u, 5u, 2u},
    {0xb8f7u, 3u, 5u, 2u},
    {0xb8fau, 3u, 3u, 0u},
};

static const FixedSpanOpcode poker_run_b8ff[] = {
    {0xb8ffu, 2u, 4u, 2u},
    {0xb901u, 2u, 4u, 2u},
    {0xb903u, 1u, 2u, 0u},
    {0xb904u, 1u, 2u, 0u},
    {0xb905u, 1u, 2u, 0u},
    {0xb906u, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_run_b901[] = {
    {0xb901u, 2u, 4u, 2u},
    {0xb903u, 1u, 2u, 0u},
    {0xb904u, 1u, 2u, 0u},
    {0xb905u, 1u, 2u, 0u},
    {0xb906u, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_run_b90b[] = {
    {0xb90bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b90e[] = {
    {0xb90eu, 2u, 7u, 4u},
    {0xb910u, 3u, 3u, 0u},
    {0xb913u, 2u, 4u, 2u},
};

static const FixedSpanOpcode poker_run_b917[] = {
    {0xb917u, 3u, 8u, 4u},
};

static const FixedSpanOpcode poker_run_b91c[] = {
    {0xb91cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b91f[] = {
    {0xb91fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b922[] = {
    {0xb922u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b925[] = {
    {0xb925u, 3u, 3u, 0u},
    {0xb928u, 3u, 5u, 2u},
    {0xb92bu, 3u, 3u, 0u},
    {0xb92eu, 3u, 5u, 2u},
    {0xb931u, 3u, 3u, 0u},
    {0xb934u, 3u, 5u, 2u},
    {0xb937u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b93a[] = {
    {0xb93au, 2u, 4u, 2u},
};

static const FixedSpanOpcode poker_run_b93e[] = {
    {0xb93eu, 3u, 3u, 0u},
    {0xb941u, 3u, 5u, 2u},
    {0xb944u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b947[] = {
    {0xb947u, 3u, 3u, 0u},
    {0xb94au, 3u, 3u, 0u},
    {0xb94du, 4u, 8u, 3u},
};

static const FixedSpanOpcode poker_run_b951[] = {
    {0xb951u, 2u, 3u, 0u},
    {0xb953u, 2u, 2u, 0u},
    {0xb955u, 2u, 3u, 1u},
    {0xb957u, 2u, 3u, 0u},
    {0xb959u, 3u, 3u, 0u},
    {0xb95cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b95f[] = {
    {0xb95fu, 3u, 5u, 2u},
    {0xb962u, 1u, 2u, 0u},
    {0xb963u, 3u, 5u, 2u},
    {0xb966u, 3u, 3u, 0u},
};

static const FixedSpanOpcode poker_run_b96b[] = {
    {0xb96bu, 3u, 3u, 0u},
    {0xb96eu, 3u, 5u, 2u},
    {0xb971u, 2u, 3u, 0u},
    {0xb9e0u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b96e[] = {
    {0xb96eu, 3u, 5u, 2u},
    {0xb971u, 2u, 3u, 0u},
    {0xb9e0u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b973[] = {
    {0xb973u, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_run_b978[] = {
    {0xb978u, 3u, 3u, 0u},
    {0xb97bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b97e[] = {
    {0xb97eu, 2u, 3u, 0u},
    {0xb9acu, 3u, 3u, 0u},
    {0xb9afu, 3u, 5u, 2u},
    {0xb9b2u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b980[] = {
    {0xb980u, 3u, 3u, 0u},
    {0xb983u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b986[] = {
    {0xb986u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b989[] = {
    {0xb989u, 3u, 5u, 2u},
    {0xb98cu, 1u, 2u, 0u},
    {0xb98du, 3u, 5u, 2u},
    {0xb990u, 3u, 3u, 0u},
};

static const FixedSpanOpcode poker_run_b995[] = {
    {0xb995u, 3u, 3u, 0u},
    {0xb998u, 3u, 5u, 2u},
    {0xb99bu, 3u, 3u, 0u},
    {0xb99eu, 3u, 5u, 2u},
    {0xb9a1u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b998[] = {
    {0xb998u, 3u, 5u, 2u},
    {0xb99bu, 3u, 3u, 0u},
    {0xb99eu, 3u, 5u, 2u},
    {0xb9a1u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b9a4[] = {
    {0xb9a4u, 3u, 5u, 2u},
    {0xb9a7u, 2u, 4u, 2u},
    {0xb9a9u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b9ac[] = {
    {0xb9acu, 3u, 3u, 0u},
    {0xb9afu, 3u, 5u, 2u},
    {0xb9b2u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b9b5[] = {
    {0xb9b5u, 3u, 3u, 0u},
    {0xb9b8u, 3u, 3u, 0u},
    {0xb9bbu, 4u, 8u, 3u},
};

static const FixedSpanOpcode poker_run_b9bf[] = {
    {0xb9bfu, 2u, 3u, 0u},
    {0xb9c1u, 2u, 2u, 0u},
    {0xb9c3u, 2u, 3u, 1u},
    {0xb9c5u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b9c8[] = {
    {0xb9c8u, 2u, 3u, 0u},
    {0xb9cau, 3u, 3u, 0u},
    {0xb9cdu, 3u, 5u, 2u},
    {0xb9d0u, 3u, 3u, 0u},
    {0xb9d3u, 3u, 5u, 2u},
    {0xb9d6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b9d9[] = {
    {0xb9d9u, 2u, 4u, 2u},
    {0xb9dbu, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_run_b9e0[] = {
    {0xb9e0u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_b9e3[] = {
    {0xb9e3u, 3u, 4u, 0u},
    {0xb862u, 2u, 3u, 0u},
    {0xb864u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_ba01[] = {
    {0xba01u, 3u, 3u, 0u},
    {0xba04u, 3u, 3u, 0u},
    {0xba07u, 4u, 8u, 3u},
};

static const FixedSpanOpcode poker_run_ba0b[] = {
    {0xba0bu, 2u, 3u, 0u},
    {0xba0du, 2u, 2u, 0u},
    {0xba0fu, 2u, 3u, 1u},
    {0xba11u, 2u, 3u, 0u},
    {0xba13u, 3u, 3u, 0u},
    {0xba16u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_ba19[] = {
    {0xba19u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_run_ba1c[] = {
    {0xba1cu, 1u, 4u, 1u},
    {0xba1du, 1u, 4u, 1u},
};

static const PokerRunPhase poker_run_phases[] = {
    {0xb827u, 0x0000u, 0x0000u, 1u, sizeof poker_run_b827 / sizeof poker_run_b827[0], poker_run_b827},
    {0xb832u, 0x0000u, 0x0000u, 1u, sizeof poker_run_b832 / sizeof poker_run_b832[0], poker_run_b832},
    {0xb867u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b867 / sizeof poker_run_b867[0], poker_run_b867},
    {0xb86au, 0xb871u, 0x0000u, 0u, sizeof poker_run_b86a / sizeof poker_run_b86a[0], poker_run_b86a},
    {0xb873u, 0xb877u, 0x0000u, 0u, sizeof poker_run_b873 / sizeof poker_run_b873[0], poker_run_b873},
    {0xb879u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b879 / sizeof poker_run_b879[0], poker_run_b879},
    {0xb87cu, 0x0000u, 0x0000u, 0u, sizeof poker_run_b87c / sizeof poker_run_b87c[0], poker_run_b87c},
    {0xb886u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b886 / sizeof poker_run_b886[0], poker_run_b886},
    {0xb88cu, 0x0000u, 0x0000u, 0u, sizeof poker_run_b88c / sizeof poker_run_b88c[0], poker_run_b88c},
    {0xb88fu, 0x0000u, 0x0000u, 0u, sizeof poker_run_b88f / sizeof poker_run_b88f[0], poker_run_b88f},
    {0xb898u, 0x0000u, 0x0000u, 1u, sizeof poker_run_b898 / sizeof poker_run_b898[0], poker_run_b898},
    {0xb89eu, 0x0000u, 0x0000u, 0u, sizeof poker_run_b89e / sizeof poker_run_b89e[0], poker_run_b89e},
    {0xb8a1u, 0xb8a8u, 0x0000u, 0u, sizeof poker_run_b8a1 / sizeof poker_run_b8a1[0], poker_run_b8a1},
    {0xb8aau, 0x0000u, 0x0000u, 0u, sizeof poker_run_b8aa / sizeof poker_run_b8aa[0], poker_run_b8aa},
    {0xb8aeu, 0x0000u, 0x0000u, 0u, sizeof poker_run_b8ae / sizeof poker_run_b8ae[0], poker_run_b8ae},
    {0xb8b1u, 0xb8b8u, 0x0000u, 0u, sizeof poker_run_b8b1 / sizeof poker_run_b8b1[0], poker_run_b8b1},
    {0xb8bau, 0x0000u, 0x0000u, 0u, sizeof poker_run_b8ba / sizeof poker_run_b8ba[0], poker_run_b8ba},
    {0xb8c3u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b8c3 / sizeof poker_run_b8c3[0], poker_run_b8c3},
    {0xb8c6u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b8c6 / sizeof poker_run_b8c6[0], poker_run_b8c6},
    {0xb8cfu, 0x0000u, 0x0000u, 1u, sizeof poker_run_b8cf / sizeof poker_run_b8cf[0], poker_run_b8cf},
    {0xb8d6u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b8d6 / sizeof poker_run_b8d6[0], poker_run_b8d6},
    {0xb8d9u, 0xb8e0u, 0x0000u, 0u, sizeof poker_run_b8d9 / sizeof poker_run_b8d9[0], poker_run_b8d9},
    {0xb8e2u, 0xb8e5u, 0x0000u, 0u, sizeof poker_run_b8e2 / sizeof poker_run_b8e2[0], poker_run_b8e2},
    {0xb8e7u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b8e7 / sizeof poker_run_b8e7[0], poker_run_b8e7},
    {0xb8eau, 0x0000u, 0x0000u, 0u, sizeof poker_run_b8ea / sizeof poker_run_b8ea[0], poker_run_b8ea},
    {0xb8edu, 0xb8fdu, 0x0000u, 0u, sizeof poker_run_b8ed / sizeof poker_run_b8ed[0], poker_run_b8ed},
    {0xb8ffu, 0xb909u, 0x0000u, 0u, sizeof poker_run_b8ff / sizeof poker_run_b8ff[0], poker_run_b8ff},
    {0xb901u, 0xb909u, 0x0000u, 0u, sizeof poker_run_b901 / sizeof poker_run_b901[0], poker_run_b901},
    {0xb90bu, 0x0000u, 0x0000u, 0u, sizeof poker_run_b90b / sizeof poker_run_b90b[0], poker_run_b90b},
    {0xb90eu, 0xb915u, 0x0000u, 0u, sizeof poker_run_b90e / sizeof poker_run_b90e[0], poker_run_b90e},
    {0xb917u, 0xb91au, 0x0000u, 0u, sizeof poker_run_b917 / sizeof poker_run_b917[0], poker_run_b917},
    {0xb91cu, 0x0000u, 0x0000u, 0u, sizeof poker_run_b91c / sizeof poker_run_b91c[0], poker_run_b91c},
    {0xb91fu, 0x0000u, 0x0000u, 0u, sizeof poker_run_b91f / sizeof poker_run_b91f[0], poker_run_b91f},
    {0xb922u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b922 / sizeof poker_run_b922[0], poker_run_b922},
    {0xb925u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b925 / sizeof poker_run_b925[0], poker_run_b925},
    {0xb93au, 0xb93cu, 0x0000u, 0u, sizeof poker_run_b93a / sizeof poker_run_b93a[0], poker_run_b93a},
    {0xb93eu, 0x0000u, 0x0000u, 0u, sizeof poker_run_b93e / sizeof poker_run_b93e[0], poker_run_b93e},
    {0xb947u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b947 / sizeof poker_run_b947[0], poker_run_b947},
    {0xb951u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b951 / sizeof poker_run_b951[0], poker_run_b951},
    {0xb95fu, 0xb969u, 0x0000u, 0u, sizeof poker_run_b95f / sizeof poker_run_b95f[0], poker_run_b95f},
    {0xb96bu, 0x0000u, 0x0000u, 0u, sizeof poker_run_b96b / sizeof poker_run_b96b[0], poker_run_b96b},
    {0xb96eu, 0x0000u, 0x0000u, 0u, sizeof poker_run_b96e / sizeof poker_run_b96e[0], poker_run_b96e},
    {0xb973u, 0xb976u, 0x0000u, 0u, sizeof poker_run_b973 / sizeof poker_run_b973[0], poker_run_b973},
    {0xb978u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b978 / sizeof poker_run_b978[0], poker_run_b978},
    {0xb97eu, 0x0000u, 0x0000u, 0u, sizeof poker_run_b97e / sizeof poker_run_b97e[0], poker_run_b97e},
    {0xb980u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b980 / sizeof poker_run_b980[0], poker_run_b980},
    {0xb986u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b986 / sizeof poker_run_b986[0], poker_run_b986},
    {0xb989u, 0xb993u, 0x0000u, 0u, sizeof poker_run_b989 / sizeof poker_run_b989[0], poker_run_b989},
    {0xb995u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b995 / sizeof poker_run_b995[0], poker_run_b995},
    {0xb998u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b998 / sizeof poker_run_b998[0], poker_run_b998},
    {0xb9a4u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b9a4 / sizeof poker_run_b9a4[0], poker_run_b9a4},
    {0xb9acu, 0x0000u, 0x0000u, 0u, sizeof poker_run_b9ac / sizeof poker_run_b9ac[0], poker_run_b9ac},
    {0xb9b5u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b9b5 / sizeof poker_run_b9b5[0], poker_run_b9b5},
    {0xb9bfu, 0x0000u, 0x0000u, 0u, sizeof poker_run_b9bf / sizeof poker_run_b9bf[0], poker_run_b9bf},
    {0xb9c8u, 0x0000u, 0x0000u, 1u, sizeof poker_run_b9c8 / sizeof poker_run_b9c8[0], poker_run_b9c8},
    {0xb9d9u, 0xb9deu, 0x0000u, 0u, sizeof poker_run_b9d9 / sizeof poker_run_b9d9[0], poker_run_b9d9},
    {0xb9e0u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b9e0 / sizeof poker_run_b9e0[0], poker_run_b9e0},
    {0xb9e3u, 0x0000u, 0x0000u, 0u, sizeof poker_run_b9e3 / sizeof poker_run_b9e3[0], poker_run_b9e3},
    {0xba01u, 0x0000u, 0x0000u, 0u, sizeof poker_run_ba01 / sizeof poker_run_ba01[0], poker_run_ba01},
    {0xba0bu, 0x0000u, 0x0000u, 0u, sizeof poker_run_ba0b / sizeof poker_run_ba0b[0], poker_run_ba0b},
    {0xba19u, 0x0000u, 0x0000u, 0u, sizeof poker_run_ba19 / sizeof poker_run_ba19[0], poker_run_ba19},
    {0xba1cu, 0x0000u, 0xba1eu, 0u, sizeof poker_run_ba1c / sizeof poker_run_ba1c[0], poker_run_ba1c},
};

enum { POKER_RUN_CLOCK_CAPACITY = 512u };

typedef struct PokerRunClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[POKER_RUN_CLOCK_CAPACITY];
    ActorPushedCall call;
    uint32_t next;
    bool returning;
} PokerRunClock;

typedef struct PokerRunProjection {
    uint16_t hand;
    uint16_t opponent;
} PokerRunProjection;

static bool PokerRunRegistersSupported(const CpuState *cpu) {
    return cpu->ram && cpu->PB == 0x84u && !cpu->emulation && cpu->m_flag <= 1u &&
        !cpu->x_flag && !cpu->D && !cpu->_flag_D &&
        (cpu->DB == 0x7eu || cpu->DB == 0u || cpu->DB == 0x80u) &&
        cpu->S >= 0x1f20u && cpu->S <= 0x1ffcu;
}

static void PokerRunOpcode(PokerRunClock *clock, unsigned pc,
                               unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < POKER_RUN_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){(uint16_t)pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static const PokerRunPhase *FindPokerRunPhase(uint16_t pc) {
    for (unsigned i = 0u; i < sizeof poker_run_phases / sizeof poker_run_phases[0]; ++i)
        if (poker_run_phases[i].pc == pc)
            return poker_run_phases + i;
    return NULL;
}

static uint16_t PokerRunWord(const CpuState *cpu, unsigned address) {
    return cpu->ram[address] | (uint16_t)cpu->ram[address + 1u] << 8;
}

static bool AppendPokerRunPhase(PokerRunClock *clock, uint16_t pc,
                                PokerRunProjection projection) {
    const PokerRunPhase *phase = FindPokerRunPhase(pc);
    const CpuState *cpu = clock->call.cpu;
    if (!phase || clock->span.count + phase->count + 1u >= POKER_RUN_CLOCK_CAPACITY)
        return false;
    for (unsigned i = 0u; i < phase->count; ++i) {
        const FixedSpanOpcode *op = phase->opcodes + i;
        switch (op->pc) {
        case 0xb89au:
        case 0xb8acu:
        case 0xb8ffu: projection.hand = 0u; break;
        case 0xb8a1u:
        case 0xb8b1u:
        case 0xb90eu: ++projection.hand; break;
        case 0xb8d4u: projection.opponent = 1u; break;
        case 0xb8d9u: ++projection.opponent; break;
        default: break;
        }
        unsigned cycles = op->cycles;
        if (op->pc == 0xb906u) {
            if (projection.hand > 3u)
                return false;
            const unsigned index = projection.hand * 2u;
            if ((0x7107u >> 8) != ((0x7107u + index) >> 8))
                ++cycles;
        }
        PokerRunOpcode(clock, op->pc, op->fetched, cycles, op->data_bytes);
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
    case 0xb871u:
        taken = PokerRunWord(cpu, 0xb55u) >= 10u;
        target = taken ? 0xb886u : 0xb873u;
        break;
    case 0xb877u:
        taken = !((uint16_t)(PokerRunWord(cpu, 0xb57u) - 1u) & 0x8000u);
        target = taken ? 0xb87cu : 0xb879u;
        break;
    case 0xb8a8u:
        taken = projection.hand != 4u;
        target = taken ? 0xb89eu : 0xb8aau;
        break;
    case 0xb8b8u:
        taken = projection.hand != 4u;
        target = taken ? 0xb8aeu : 0xb8bau;
        break;
    case 0xb8e0u:
        taken = projection.opponent != 4u;
        target = taken ? 0xb8d6u : 0xb8e2u;
        break;
    case 0xb8e5u:
        taken = !PokerRunWord(cpu, 0x7107u);
        target = taken ? 0xb8eau : 0xb8e7u;
        break;
    case 0xb8fdu:
        taken = PokerRunWord(cpu, 0x713fu) == 3u;
        target = taken ? 0xb922u : 0xb8ffu;
        break;
    case 0xb909u:
        taken = !PokerRunWord(cpu, 0x7107u + projection.hand * 2u);
        target = taken ? 0xb90eu : 0xb90bu;
        break;
    case 0xb915u:
        taken = projection.hand != 4u;
        target = taken ? 0xb901u : 0xb917u;
        break;
    case 0xb91au:
        taken = (uint16_t)(PokerRunWord(cpu, 0x713du) - 1u) != 0u;
        target = taken ? 0xb8bau : 0xb91cu;
        break;
    case 0xb93cu:
        taken = PokerRunWord(cpu, 0u) != 0u;
        target = taken ? 0xb973u : 0xb93eu;
        break;
    case 0xb969u:
        taken = (uint16_t)(PokerRunWord(cpu, 0x69dcu) + PokerRunWord(cpu, 0x714bu)) < 60000u;
        target = taken ? 0xb96eu : 0xb96bu;
        break;
    case 0xb976u:
        taken = PokerRunWord(cpu, 0x7107u) != 0u;
        target = taken ? 0xb980u : 0xb978u;
        break;
    case 0xb993u:
        taken = ((uint16_t)(PokerRunWord(cpu, 0x69dcu) + PokerRunWord(cpu, 0x714bu) - 9999u) & 0x8000u) != 0u;
        target = taken ? 0xb998u : 0xb995u;
        break;
    case 0xb9deu:
        taken = PokerRunWord(cpu, 0u) != 0u;
        target = taken ? 0xba19u : 0xb9e0u;
        break;
    default: return false;
    }
    PokerRunOpcode(clock, phase->branch, 2u, taken ? 3u : 2u, 0u);
    return AppendPokerRunPhase(clock, target, projection);
}

static bool PokerRunWindowSupported(const PokerRunClock *clock) {
    CpuState *cpu = clock->call.cpu;
    if (!PokerRunRegistersSupported(cpu) ||
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

static bool PreparePokerRunClock(PokerRunClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->returning = false;
    clock->next = pc;
    if (!g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x140000u ||
        (pc >> 16) != 0x84u || !PokerRunRegistersSupported(cpu))
        return false;
    const PokerRunPhase *phase = FindPokerRunPhase((uint16_t)pc);
    if (!phase || cpu->m_flag != phase->byte_mode ||
        (cpu->DB != 0x7eu && pc != 0x84b827u && pc != 0x84b832u))
        return false;
    const PokerRunProjection projection = {PokerRunWord(cpu, 0x17u), PokerRunWord(cpu, 0x11u)};
    return AppendPokerRunPhase(clock, (uint16_t)pc, projection) &&
        PokerRunWindowSupported(clock);
}

static uint8_t PokerRunChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    PokerRunClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PreparePokerRunClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgePokerRun(CpuState *cpu, uint32_t entry,
                                         ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    PokerRunClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || !PreparePokerRunClock(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, PokerRunChild, &clock);
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
