typedef struct GameResultPhase {
    uint16_t pc;
    uint16_t branch;
    uint16_t return_pc;
    uint8_t byte_mode;
    unsigned count;
    const FixedSpanOpcode *opcodes;
} GameResultPhase;

static const FixedSpanOpcode game_result_ba53[] = {
    {0xba53u, 3u, 5u, 2u},
    {0xba56u, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_result_ba5b[] = {
    {0xba5bu, 3u, 3u, 0u},
    {0xba5eu, 3u, 3u, 0u},
    {0xba61u, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_result_ba61[] = {
    {0xba61u, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_result_ba66[] = {
    {0xba66u, 3u, 10u, 4u},
    {0xba69u, 1u, 2u, 0u},
    {0xba6au, 1u, 2u, 0u},
    {0xba6bu, 1u, 2u, 0u},
};

static const FixedSpanOpcode game_result_ba69[] = {
    {0xba69u, 1u, 2u, 0u},
    {0xba6au, 1u, 2u, 0u},
    {0xba6bu, 1u, 2u, 0u},
};

static const FixedSpanOpcode game_result_ba6e[] = {
    {0xba6eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_ba71[] = {
    {0xba71u, 3u, 4u, 0u},
    {0xbaebu, 3u, 3u, 0u},
    {0xbaeeu, 2u, 4u, 2u},
};

static const FixedSpanOpcode game_result_ba74[] = {
    {0xba74u, 2u, 4u, 2u},
    {0xba76u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_ba79[] = {
    {0xba79u, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_result_ba7e[] = {
    {0xba7eu, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_result_ba83[] = {
    {0xba83u, 3u, 5u, 2u},
    {0xba86u, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_result_ba8b[] = {
    {0xba8bu, 3u, 3u, 0u},
    {0xba8eu, 2u, 3u, 0u},
    {0xbaeeu, 2u, 4u, 2u},
};

static const FixedSpanOpcode game_result_ba90[] = {
    {0xba90u, 2u, 4u, 2u},
    {0xba92u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_ba92[] = {
    {0xba92u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_ba95[] = {
    {0xba95u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_ba98[] = {
    {0xba98u, 2u, 7u, 4u},
    {0xba9au, 3u, 3u, 0u},
    {0xba9du, 2u, 4u, 2u},
};

static const FixedSpanOpcode game_result_baa1[] = {
    {0xbaa1u, 3u, 5u, 2u},
    {0xbaa4u, 1u, 2u, 0u},
    {0xbaa5u, 1u, 2u, 0u},
    {0xbaa6u, 1u, 2u, 0u},
    {0xbaa7u, 3u, 3u, 0u},
    {0xbaaau, 3u, 7u, 2u},
    {0xbaadu, 3u, 3u, 0u},
    {0xbab0u, 3u, 3u, 0u},
    {0xbab3u, 2u, 4u, 2u},
    {0xbab5u, 3u, 5u, 2u},
    {0xbab8u, 1u, 2u, 0u},
    {0xbab9u, 1u, 2u, 0u},
    {0xbabau, 1u, 2u, 0u},
    {0xbabbu, 3u, 7u, 2u},
    {0xbabeu, 1u, 2u, 0u},
    {0xbabfu, 1u, 2u, 0u},
    {0xbac0u, 1u, 2u, 0u},
    {0xbac1u, 1u, 2u, 0u},
    {0xbac2u, 2u, 7u, 4u},
};

static const FixedSpanOpcode game_result_bab5[] = {
    {0xbab5u, 3u, 5u, 2u},
    {0xbab8u, 1u, 2u, 0u},
    {0xbab9u, 1u, 2u, 0u},
    {0xbabau, 1u, 2u, 0u},
    {0xbabbu, 3u, 7u, 2u},
    {0xbabeu, 1u, 2u, 0u},
    {0xbabfu, 1u, 2u, 0u},
    {0xbac0u, 1u, 2u, 0u},
    {0xbac1u, 1u, 2u, 0u},
    {0xbac2u, 2u, 7u, 4u},
};

static const FixedSpanOpcode game_result_bac6[] = {
    {0xbac6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_bac9[] = {
    {0xbac9u, 3u, 3u, 0u},
    {0xbaccu, 3u, 3u, 0u},
    {0xbacfu, 2u, 4u, 2u},
    {0xbad1u, 4u, 6u, 2u},
    {0xbad5u, 2u, 4u, 2u},
    {0xbad7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_bad1[] = {
    {0xbad1u, 4u, 6u, 2u},
    {0xbad5u, 2u, 4u, 2u},
    {0xbad7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_bada[] = {
    {0xbadau, 1u, 2u, 0u},
    {0xbadbu, 1u, 2u, 0u},
    {0xbadcu, 2u, 7u, 4u},
};

static const FixedSpanOpcode game_result_bae0[] = {
    {0xbae0u, 2u, 3u, 0u},
    {0xbae2u, 2u, 2u, 0u},
    {0xbae4u, 2u, 3u, 1u},
    {0xbae6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_bae9[] = {
    {0xbae9u, 2u, 3u, 0u},
    {0xbaebu, 3u, 3u, 0u},
    {0xbaeeu, 2u, 4u, 2u},
};

static const FixedSpanOpcode game_result_baee[] = {
    {0xbaeeu, 2u, 4u, 2u},
};

static const FixedSpanOpcode game_result_d456[] = {
    {0xd456u, 1u, 3u, 1u},
    {0xd457u, 2u, 3u, 0u},
    {0xd459u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_d45c[] = {
    {0xd45cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_d45f[] = {
    {0xd45fu, 2u, 4u, 2u},
    {0xd461u, 3u, 3u, 0u},
    {0xd464u, 3u, 3u, 0u},
    {0xd467u, 3u, 5u, 2u},
    {0xd46au, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_result_d467[] = {
    {0xd467u, 3u, 5u, 2u},
    {0xd46au, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_result_d46f[] = {
    {0xd46fu, 3u, 3u, 0u},
    {0xd472u, 2u, 7u, 4u},
    {0xd474u, 2u, 7u, 4u},
    {0xd476u, 3u, 5u, 2u},
    {0xd479u, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_result_d47e[] = {
    {0xd47eu, 2u, 7u, 4u},
    {0xd480u, 2u, 7u, 4u},
    {0xd482u, 1u, 2u, 0u},
    {0xd483u, 1u, 2u, 0u},
    {0xd484u, 1u, 2u, 0u},
};

static const FixedSpanOpcode game_result_d482[] = {
    {0xd482u, 1u, 2u, 0u},
    {0xd483u, 1u, 2u, 0u},
    {0xd484u, 1u, 2u, 0u},
};

static const FixedSpanOpcode game_result_d487[] = {
    {0xd487u, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_result_d48c[] = {
    {0xd48cu, 2u, 4u, 2u},
    {0xd48eu, 2u, 3u, 0u},
    {0xd49du, 3u, 5u, 2u},
    {0xd4a0u, 3u, 5u, 2u},
    {0xd4a3u, 3u, 5u, 2u},
    {0xd4a6u, 1u, 2u, 0u},
};

static const FixedSpanOpcode game_result_d490[] = {
    {0xd490u, 2u, 4u, 2u},
};

static const FixedSpanOpcode game_result_d494[] = {
    {0xd494u, 2u, 7u, 4u},
    {0xd496u, 1u, 2u, 0u},
    {0xd497u, 1u, 2u, 0u},
    {0xd498u, 1u, 2u, 0u},
    {0xd499u, 1u, 2u, 0u},
    {0xd49au, 1u, 2u, 0u},
    {0xd49bu, 2u, 4u, 2u},
    {0xd49du, 3u, 5u, 2u},
    {0xd4a0u, 3u, 5u, 2u},
    {0xd4a3u, 3u, 5u, 2u},
    {0xd4a6u, 1u, 2u, 0u},
};

static const FixedSpanOpcode game_result_d497[] = {
    {0xd497u, 1u, 2u, 0u},
    {0xd498u, 1u, 2u, 0u},
    {0xd499u, 1u, 2u, 0u},
    {0xd49au, 1u, 2u, 0u},
    {0xd49bu, 2u, 4u, 2u},
    {0xd49du, 3u, 5u, 2u},
    {0xd4a0u, 3u, 5u, 2u},
    {0xd4a3u, 3u, 5u, 2u},
    {0xd4a6u, 1u, 2u, 0u},
};

static const FixedSpanOpcode game_result_d4a9[] = {
    {0xd4a9u, 1u, 2u, 0u},
    {0xd4aau, 1u, 2u, 0u},
    {0xd4abu, 2u, 3u, 0u},
    {0xd4a6u, 1u, 2u, 0u},
};

static const FixedSpanOpcode game_result_d4ad[] = {
    {0xd4adu, 2u, 4u, 2u},
    {0xd4afu, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_result_d4b4[] = {
    {0xd4b4u, 1u, 2u, 0u},
    {0xd4b5u, 1u, 2u, 0u},
    {0xd4b6u, 2u, 4u, 2u},
    {0xd4b8u, 1u, 2u, 0u},
};

static const FixedSpanOpcode game_result_d4b5[] = {
    {0xd4b5u, 1u, 2u, 0u},
    {0xd4b6u, 2u, 4u, 2u},
    {0xd4b8u, 1u, 2u, 0u},
};

static const FixedSpanOpcode game_result_d4bb[] = {
    {0xd4bbu, 1u, 2u, 0u},
    {0xd4bcu, 1u, 2u, 0u},
    {0xd4bdu, 2u, 4u, 2u},
    {0xd4bfu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_d4c2[] = {
    {0xd4c2u, 3u, 3u, 0u},
    {0xd4c5u, 3u, 5u, 2u},
    {0xd4c8u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_d4cb[] = {
    {0xd4cbu, 3u, 3u, 0u},
    {0xd4ceu, 3u, 3u, 0u},
    {0xd4d1u, 4u, 8u, 3u},
};

static const FixedSpanOpcode game_result_d4d5[] = {
    {0xd4d5u, 3u, 3u, 0u},
    {0xd4d8u, 3u, 3u, 0u},
    {0xd4dbu, 4u, 8u, 3u},
};

static const FixedSpanOpcode game_result_d4df[] = {
    {0xd4dfu, 2u, 3u, 0u},
    {0xd4e1u, 2u, 2u, 0u},
    {0xd4e3u, 2u, 3u, 1u},
    {0xd4e5u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_d4e5[] = {
    {0xd4e5u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_d4e8[] = {
    {0xd4e8u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_d4eb[] = {
    {0xd4ebu, 2u, 2u, 0u},
};

static const FixedSpanOpcode game_result_d4ef[] = {
    {0xd4efu, 2u, 3u, 0u},
    {0xd4f1u, 3u, 3u, 0u},
    {0xd4f4u, 3u, 5u, 2u},
    {0xd4f7u, 2u, 4u, 2u},
    {0xd4f9u, 1u, 2u, 0u},
    {0xd4fau, 4u, 6u, 2u},
};

static const FixedSpanOpcode game_result_d500[] = {
    {0xd500u, 3u, 5u, 2u},
    {0xd503u, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_result_d508[] = {
    {0xd508u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_d50b[] = {
    {0xd50bu, 2u, 3u, 0u},
    {0xd4f7u, 2u, 4u, 2u},
    {0xd4f9u, 1u, 2u, 0u},
    {0xd4fau, 4u, 6u, 2u},
};

static const FixedSpanOpcode game_result_d50d[] = {
    {0xd50du, 2u, 4u, 2u},
    {0xd50fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_d512[] = {
    {0xd512u, 3u, 3u, 0u},
    {0xd515u, 3u, 3u, 0u},
    {0xd518u, 4u, 8u, 3u},
};

static const FixedSpanOpcode game_result_d51c[] = {
    {0xd51cu, 3u, 3u, 0u},
    {0xd51fu, 3u, 3u, 0u},
    {0xd522u, 4u, 8u, 3u},
};

static const FixedSpanOpcode game_result_d526[] = {
    {0xd526u, 2u, 3u, 0u},
    {0xd528u, 2u, 2u, 0u},
    {0xd52au, 2u, 3u, 1u},
    {0xd52cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_d52f[] = {
    {0xd52fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_result_d532[] = {
    {0xd532u, 2u, 3u, 0u},
    {0xd534u, 2u, 3u, 0u},
    {0xd4f7u, 2u, 4u, 2u},
    {0xd4f9u, 1u, 2u, 0u},
    {0xd4fau, 4u, 6u, 2u},
};

static const FixedSpanOpcode game_result_d536[] = {
    {0xd536u, 1u, 4u, 1u},
};

static const GameResultPhase game_result_phases[] = {
    {0xba53u, 0xba59u, 0x0000u, 0u, sizeof game_result_ba53 / sizeof game_result_ba53[0], game_result_ba53},
    {0xba5bu, 0xba64u, 0x0000u, 0u, sizeof game_result_ba5b / sizeof game_result_ba5b[0], game_result_ba5b},
    {0xba61u, 0xba64u, 0x0000u, 0u, sizeof game_result_ba61 / sizeof game_result_ba61[0], game_result_ba61},
    {0xba66u, 0xba6cu, 0x0000u, 0u, sizeof game_result_ba66 / sizeof game_result_ba66[0], game_result_ba66},
    {0xba69u, 0xba6cu, 0x0000u, 0u, sizeof game_result_ba69 / sizeof game_result_ba69[0], game_result_ba69},
    {0xba6eu, 0x0000u, 0x0000u, 0u, sizeof game_result_ba6e / sizeof game_result_ba6e[0], game_result_ba6e},
    {0xba71u, 0x0000u, 0xbaf0u, 0u, sizeof game_result_ba71 / sizeof game_result_ba71[0], game_result_ba71},
    {0xba74u, 0x0000u, 0x0000u, 0u, sizeof game_result_ba74 / sizeof game_result_ba74[0], game_result_ba74},
    {0xba79u, 0xba7cu, 0x0000u, 0u, sizeof game_result_ba79 / sizeof game_result_ba79[0], game_result_ba79},
    {0xba7eu, 0xba81u, 0x0000u, 0u, sizeof game_result_ba7e / sizeof game_result_ba7e[0], game_result_ba7e},
    {0xba83u, 0xba89u, 0x0000u, 0u, sizeof game_result_ba83 / sizeof game_result_ba83[0], game_result_ba83},
    {0xba8bu, 0x0000u, 0xbaf0u, 0u, sizeof game_result_ba8b / sizeof game_result_ba8b[0], game_result_ba8b},
    {0xba90u, 0x0000u, 0x0000u, 0u, sizeof game_result_ba90 / sizeof game_result_ba90[0], game_result_ba90},
    {0xba92u, 0x0000u, 0x0000u, 0u, sizeof game_result_ba92 / sizeof game_result_ba92[0], game_result_ba92},
    {0xba95u, 0x0000u, 0x0000u, 0u, sizeof game_result_ba95 / sizeof game_result_ba95[0], game_result_ba95},
    {0xba98u, 0xba9fu, 0x0000u, 0u, sizeof game_result_ba98 / sizeof game_result_ba98[0], game_result_ba98},
    {0xbaa1u, 0xbac4u, 0x0000u, 0u, sizeof game_result_baa1 / sizeof game_result_baa1[0], game_result_baa1},
    {0xbab5u, 0xbac4u, 0x0000u, 0u, sizeof game_result_bab5 / sizeof game_result_bab5[0], game_result_bab5},
    {0xbac6u, 0x0000u, 0x0000u, 0u, sizeof game_result_bac6 / sizeof game_result_bac6[0], game_result_bac6},
    {0xbac9u, 0x0000u, 0x0000u, 0u, sizeof game_result_bac9 / sizeof game_result_bac9[0], game_result_bac9},
    {0xbad1u, 0x0000u, 0x0000u, 0u, sizeof game_result_bad1 / sizeof game_result_bad1[0], game_result_bad1},
    {0xbadau, 0xbadeu, 0x0000u, 0u, sizeof game_result_bada / sizeof game_result_bada[0], game_result_bada},
    {0xbae0u, 0x0000u, 0x0000u, 0u, sizeof game_result_bae0 / sizeof game_result_bae0[0], game_result_bae0},
    {0xbae9u, 0x0000u, 0xbaf0u, 1u, sizeof game_result_bae9 / sizeof game_result_bae9[0], game_result_bae9},
    {0xbaeeu, 0x0000u, 0xbaf0u, 0u, sizeof game_result_baee / sizeof game_result_baee[0], game_result_baee},
    {0xd456u, 0x0000u, 0x0000u, 0u, sizeof game_result_d456 / sizeof game_result_d456[0], game_result_d456},
    {0xd45cu, 0x0000u, 0x0000u, 0u, sizeof game_result_d45c / sizeof game_result_d45c[0], game_result_d45c},
    {0xd45fu, 0xd46du, 0x0000u, 0u, sizeof game_result_d45f / sizeof game_result_d45f[0], game_result_d45f},
    {0xd467u, 0xd46du, 0x0000u, 0u, sizeof game_result_d467 / sizeof game_result_d467[0], game_result_d467},
    {0xd46fu, 0xd47cu, 0x0000u, 0u, sizeof game_result_d46f / sizeof game_result_d46f[0], game_result_d46f},
    {0xd47eu, 0xd485u, 0x0000u, 0u, sizeof game_result_d47e / sizeof game_result_d47e[0], game_result_d47e},
    {0xd482u, 0xd485u, 0x0000u, 0u, sizeof game_result_d482 / sizeof game_result_d482[0], game_result_d482},
    {0xd487u, 0xd48au, 0x0000u, 0u, sizeof game_result_d487 / sizeof game_result_d487[0], game_result_d487},
    {0xd48cu, 0xd4a7u, 0x0000u, 0u, sizeof game_result_d48c / sizeof game_result_d48c[0], game_result_d48c},
    {0xd490u, 0xd492u, 0x0000u, 0u, sizeof game_result_d490 / sizeof game_result_d490[0], game_result_d490},
    {0xd494u, 0xd4a7u, 0x0000u, 0u, sizeof game_result_d494 / sizeof game_result_d494[0], game_result_d494},
    {0xd497u, 0xd4a7u, 0x0000u, 0u, sizeof game_result_d497 / sizeof game_result_d497[0], game_result_d497},
    {0xd4a9u, 0xd4a7u, 0x0000u, 0u, sizeof game_result_d4a9 / sizeof game_result_d4a9[0], game_result_d4a9},
    {0xd4adu, 0xd4b2u, 0x0000u, 0u, sizeof game_result_d4ad / sizeof game_result_d4ad[0], game_result_d4ad},
    {0xd4b4u, 0xd4b9u, 0x0000u, 0u, sizeof game_result_d4b4 / sizeof game_result_d4b4[0], game_result_d4b4},
    {0xd4b5u, 0xd4b9u, 0x0000u, 0u, sizeof game_result_d4b5 / sizeof game_result_d4b5[0], game_result_d4b5},
    {0xd4bbu, 0x0000u, 0x0000u, 0u, sizeof game_result_d4bb / sizeof game_result_d4bb[0], game_result_d4bb},
    {0xd4c2u, 0x0000u, 0x0000u, 0u, sizeof game_result_d4c2 / sizeof game_result_d4c2[0], game_result_d4c2},
    {0xd4cbu, 0x0000u, 0x0000u, 0u, sizeof game_result_d4cb / sizeof game_result_d4cb[0], game_result_d4cb},
    {0xd4d5u, 0x0000u, 0x0000u, 0u, sizeof game_result_d4d5 / sizeof game_result_d4d5[0], game_result_d4d5},
    {0xd4dfu, 0x0000u, 0x0000u, 0u, sizeof game_result_d4df / sizeof game_result_d4df[0], game_result_d4df},
    {0xd4e5u, 0x0000u, 0x0000u, 1u, sizeof game_result_d4e5 / sizeof game_result_d4e5[0], game_result_d4e5},
    {0xd4e8u, 0x0000u, 0x0000u, 1u, sizeof game_result_d4e8 / sizeof game_result_d4e8[0], game_result_d4e8},
    {0xd4ebu, 0xd4edu, 0x0000u, 1u, sizeof game_result_d4eb / sizeof game_result_d4eb[0], game_result_d4eb},
    {0xd4efu, 0xd4feu, 0x0000u, 1u, sizeof game_result_d4ef / sizeof game_result_d4ef[0], game_result_d4ef},
    {0xd500u, 0xd506u, 0x0000u, 0u, sizeof game_result_d500 / sizeof game_result_d500[0], game_result_d500},
    {0xd508u, 0x0000u, 0x0000u, 0u, sizeof game_result_d508 / sizeof game_result_d508[0], game_result_d508},
    {0xd50bu, 0xd4feu, 0x0000u, 0u, sizeof game_result_d50b / sizeof game_result_d50b[0], game_result_d50b},
    {0xd50du, 0x0000u, 0x0000u, 0u, sizeof game_result_d50d / sizeof game_result_d50d[0], game_result_d50d},
    {0xd512u, 0x0000u, 0x0000u, 0u, sizeof game_result_d512 / sizeof game_result_d512[0], game_result_d512},
    {0xd51cu, 0x0000u, 0x0000u, 0u, sizeof game_result_d51c / sizeof game_result_d51c[0], game_result_d51c},
    {0xd526u, 0x0000u, 0x0000u, 0u, sizeof game_result_d526 / sizeof game_result_d526[0], game_result_d526},
    {0xd52fu, 0x0000u, 0x0000u, 1u, sizeof game_result_d52f / sizeof game_result_d52f[0], game_result_d52f},
    {0xd532u, 0xd4feu, 0x0000u, 1u, sizeof game_result_d532 / sizeof game_result_d532[0], game_result_d532},
    {0xd536u, 0x0000u, 0xd537u, 0u, sizeof game_result_d536 / sizeof game_result_d536[0], game_result_d536},
};

enum { GAME_RESULT_CLOCK_CAPACITY = 2048u };

typedef struct GameResultClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[GAME_RESULT_CLOCK_CAPACITY];
    ActorPushedCall call;
    uint32_t next;
    bool returning;
} GameResultClock;

typedef struct GameResultProjection {
    uint16_t index;
    uint16_t remaining;
    uint16_t line_bonus;
    uint16_t multiplier;
    uint16_t payout_step;
} GameResultProjection;

static void GameResultOpcode(GameResultClock *clock, unsigned pc,
                               unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < GAME_RESULT_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){(uint16_t)pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static const GameResultPhase *FindGameResultPhase(uint16_t pc) {
    for (unsigned i = 0u; i < sizeof game_result_phases / sizeof game_result_phases[0]; ++i)
        if (game_result_phases[i].pc == pc)
            return game_result_phases + i;
    return NULL;
}

static uint16_t GameResultWord(const CpuState *cpu, unsigned address) {
    return cpu->ram[address] | (uint16_t)cpu->ram[address + 1u] << 8;
}

static bool AppendGameResultPhase(GameResultClock *clock, uint16_t pc,
                                    GameResultProjection projection) {
    const GameResultPhase *phase = FindGameResultPhase(pc);
    const CpuState *cpu = clock->call.cpu;
    if (!phase || clock->span.count + phase->count + 1u >= GAME_RESULT_CLOCK_CAPACITY)
        return false;
    if (pc == 0xbaa1u) {
        for (unsigned seat = 0u; seat < 4u; ++seat)
            if (GameResultWord(cpu, 0x7141u + seat * 2u) > 3u)
                return false;
    }
    if (pc == 0xd45fu) {
        const unsigned completed = GameResultWord(cpu, 0x71cau);
        if (!completed || completed > 12u)
            return false;
    }
    for (unsigned i = 0u; i < phase->count; ++i) {
        const FixedSpanOpcode *op = phase->opcodes + i;
        switch (op->pc) {
        case 0xba5bu: projection.remaining = 4u; break;
        case 0xba5eu:
        case 0xd461u: projection.index = 0u; break;
        case 0xbaadu: projection.index = 2u; break;
        case 0xbab0u: projection.remaining = 3u; break;
        case 0xba69u:
        case 0xba6au:
        case 0xbac0u:
        case 0xbac1u:
        case 0xd482u:
        case 0xd483u: ++projection.index; break;
        case 0xba6bu:
        case 0xbac2u:
        case 0xd484u:
        case 0xd4a6u:
        case 0xd4b4u:
        case 0xd4b8u: --projection.remaining; break;
        case 0xd45fu: projection.line_bonus = 0u; break;
        case 0xd464u: projection.remaining = 12u; break;
        case 0xd472u:
        case 0xd474u:
        case 0xd47eu:
        case 0xd480u:
        case 0xd494u: ++projection.line_bonus; break;
        case 0xd48cu: projection.multiplier = projection.line_bonus; break;
        case 0xd497u: projection.multiplier = (uint16_t)(projection.line_bonus * 5u); break;
        case 0xd4a0u: projection.remaining = GameResultWord(cpu, 0x71cau); break;
        case 0xd4afu: projection.remaining = projection.multiplier; break;
        case 0xd4f1u: projection.payout_step = 10000u; break;
        default: break;
        }
        unsigned cycles = op->cycles;
        unsigned base = 0u;
        switch (op->pc) {
        case 0xba61u: base = 0x7107u; break;
        case 0xbab5u: base = 0x7141u; break;
        case 0xd467u:
        case 0xd476u: base = 0x71b2u; break;
        default: break;
        }
        if (base && (base >> 8) != ((base + projection.index) >> 8))
            ++cycles;
        GameResultOpcode(clock, op->pc, op->fetched, cycles, op->data_bytes);
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
    case 0xba59u:
        taken = GameResultWord(cpu, 0x713fu) != 3u;
        target = taken ? 0xba74u : 0xba5bu;
        break;
    case 0xba64u:
        taken = !GameResultWord(cpu, 0x7107u + projection.index);
        target = taken ? 0xba69u : 0xba66u;
        break;
    case 0xba6cu:
        taken = projection.remaining != 0u;
        target = taken ? 0xba61u : 0xba6eu;
        break;
    case 0xba7cu:
        taken = !GameResultWord(cpu, 0x70e7u);
        target = taken ? 0xbaeeu : 0xba7eu;
        break;
    case 0xba81u:
        taken = GameResultWord(cpu, 0x70e7u) != GameResultWord(cpu, 0x70e9u);
        target = taken ? 0xba90u : 0xba83u;
        break;
    case 0xba89u:
        taken = GameResultWord(cpu, 0x70efu) != GameResultWord(cpu, 0x70f1u);
        target = taken ? 0xba90u : 0xba8bu;
        break;
    case 0xba9fu:
        taken = (uint16_t)(GameResultWord(cpu, 0x24u) + 1u) != 4u;
        target = taken ? 0xba92u : 0xbaa1u;
        break;
    case 0xbac4u:
        taken = projection.remaining != 0u;
        target = taken ? 0xbab5u : 0xbac6u;
        break;
    case 0xbadeu:
        taken = (uint16_t)(GameResultWord(cpu, 0u) - 1u) != 0u;
        target = taken ? 0xbad1u : 0xbae0u;
        break;
    case 0xd46du:
        taken = ((uint16_t)(GameResultWord(cpu, 0x71b2u + projection.index) - 101u) & 0x8000u) != 0u;
        target = taken ? 0xd482u : 0xd46fu;
        break;
    case 0xd47cu:
        taken = ((uint16_t)(GameResultWord(cpu, 0x71b2u + projection.index) - 108u) & 0x8000u) != 0u;
        target = taken ? 0xd482u : 0xd47eu;
        break;
    case 0xd485u:
        taken = projection.remaining != 0u;
        target = taken ? 0xd467u : 0xd487u;
        break;
    case 0xd48au:
        taken = GameResultWord(cpu, 0x69e2u) != 0u;
        target = taken ? 0xd490u : 0xd48cu;
        break;
    case 0xd492u:
        taken = projection.line_bonus != 0u;
        target = taken ? 0xd497u : 0xd494u;
        break;
    case 0xd4a7u:
        taken = projection.remaining == 0u;
        target = taken ? 0xd4adu : 0xd4a9u;
        break;
    case 0xd4b2u:
        taken = !projection.multiplier;
        target = taken ? 0xd4bbu : 0xd4b4u;
        break;
    case 0xd4b9u:
        taken = projection.remaining != 0u;
        target = taken ? 0xd4b5u : 0xd4bbu;
        break;
    case 0xd4edu:
        taken = !(cpu->A & 0xffu);
        target = taken ? 0xd4e5u : 0xd4efu;
        break;
    case 0xd4feu:
        taken = !((uint16_t)(GameResultWord(cpu, 0u) - projection.payout_step) & 0x8000u);
        target = taken ? 0xd50du : 0xd500u;
        break;
    case 0xd506u:
        taken = projection.payout_step == 1u;
        target = taken ? 0xd536u : 0xd508u;
        break;
    default:
        return false;
    }
    GameResultOpcode(clock, phase->branch, 2u, taken ? 3u : 2u, 0u);
    return AppendGameResultPhase(clock, target, projection);
}

static bool GameResultWindowSupported(const GameResultClock *clock) {
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

static bool PrepareGameResultClock(GameResultClock *clock, uint32_t pc) {
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
    const GameResultPhase *phase = FindGameResultPhase((uint16_t)pc);
    if (!phase || (pc != 0x84d456u && cpu->m_flag != phase->byte_mode))
        return false;
    const GameResultProjection projection = {cpu->X, cpu->Y,
        GameResultWord(cpu, 0u), GameResultWord(cpu, 0x69e2u),
        GameResultWord(cpu, 0x69e4u)};
    return AppendGameResultPhase(clock, (uint16_t)pc, projection) &&
        GameResultWindowSupported(clock);
}

static uint8_t GameResultChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    GameResultClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareGameResultClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeGameResult(CpuState *cpu, uint32_t entry,
                                         ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    GameResultClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || !PrepareGameResultClock(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, GameResultChild, &clock);
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
