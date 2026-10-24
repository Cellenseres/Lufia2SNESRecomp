typedef struct PokerStakePhase {
    uint16_t pc;
    uint16_t branch;
    uint16_t return_pc;
    uint8_t byte_mode;
    unsigned count;
    const FixedSpanOpcode *opcodes;
} PokerStakePhase;

static const FixedSpanOpcode poker_stake_bc78[] = {
    {0xbc78u, 2u, 4u, 2u},
    {0xbc7au, 1u, 2u, 0u},
    {0xbc7bu, 1u, 2u, 0u},
    {0xbc7cu, 1u, 2u, 0u},
    {0xbc7du, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_stake_bc82[] = {
    {0xbc82u, 3u, 4u, 0u},
    {0xbd73u, 2u, 3u, 0u},
    {0xbd75u, 2u, 2u, 0u},
    {0xbd77u, 2u, 3u, 1u},
    {0xbd79u, 2u, 2u, 0u},
    {0xbd7bu, 2u, 3u, 1u},
    {0xbd7du, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bc85[] = {
    {0xbc85u, 4u, 6u, 2u},
    {0xbc89u, 3u, 3u, 0u},
    {0xbc8cu, 1u, 2u, 0u},
    {0xbc8du, 3u, 3u, 0u},
    {0xbc90u, 3u, 5u, 2u},
    {0xbc93u, 4u, 6u, 2u},
    {0xbc97u, 1u, 3u, 0u},
    {0xbc98u, 3u, 3u, 0u},
    {0xbc9bu, 3u, 5u, 2u},
    {0xbc9eu, 3u, 3u, 0u},
    {0xbca1u, 3u, 5u, 2u},
    {0xbca4u, 3u, 3u, 0u},
    {0xbca7u, 3u, 5u, 2u},
    {0xbcaau, 2u, 4u, 2u},
    {0xbcacu, 1u, 2u, 0u},
    {0xbcadu, 1u, 2u, 0u},
    {0xbcaeu, 1u, 2u, 0u},
    {0xbcafu, 3u, 5u, 2u},
    {0xbcb2u, 3u, 5u, 2u},
    {0xbcb5u, 3u, 5u, 2u},
    {0xbcb8u, 3u, 5u, 2u},
    {0xbcbbu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bcbe[] = {
    {0xbcbeu, 2u, 4u, 2u},
};

static const FixedSpanOpcode poker_stake_bcc2[] = {
    {0xbcc2u, 3u, 8u, 4u},
    {0xbcc5u, 3u, 3u, 0u},
};

static const FixedSpanOpcode poker_stake_bcca[] = {
    {0xbccau, 3u, 8u, 4u},
    {0xbccdu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bcd0[] = {
    {0xbcd0u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bcd3[] = {
    {0xbcd3u, 2u, 4u, 2u},
    {0xbcd5u, 2u, 4u, 2u},
    {0xbcd7u, 3u, 5u, 2u},
    {0xbcdau, 2u, 4u, 2u},
    {0xbcdcu, 2u, 7u, 4u},
    {0xbcdeu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bcdc[] = {
    {0xbcdcu, 2u, 7u, 4u},
    {0xbcdeu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bce1[] = {
    {0xbce1u, 3u, 3u, 0u},
    {0xbce4u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bce7[] = {
    {0xbce7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bcea[] = {
    {0xbceau, 3u, 3u, 0u},
    {0xbcedu, 4u, 8u, 3u},
};

static const FixedSpanOpcode poker_stake_bcf1[] = {
    {0xbcf1u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bcf4[] = {
    {0xbcf4u, 3u, 3u, 0u},
    {0xbcf7u, 4u, 8u, 3u},
};

static const FixedSpanOpcode poker_stake_bcfb[] = {
    {0xbcfbu, 2u, 3u, 0u},
    {0xbcfdu, 2u, 2u, 0u},
    {0xbcffu, 2u, 3u, 1u},
    {0xbd01u, 2u, 2u, 0u},
    {0xbd03u, 2u, 3u, 1u},
    {0xbd05u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd08[] = {
    {0xbd08u, 2u, 3u, 0u},
    {0xbd0au, 2u, 7u, 4u},
};

static const FixedSpanOpcode poker_stake_bd0e[] = {
    {0xbd0eu, 2u, 4u, 2u},
    {0xbd10u, 3u, 5u, 2u},
    {0xbd13u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd16[] = {
    {0xbd16u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd19[] = {
    {0xbd19u, 2u, 3u, 0u},
    {0xbd1bu, 2u, 2u, 0u},
    {0xbd1du, 2u, 3u, 1u},
    {0xbd1fu, 2u, 3u, 0u},
    {0xbd21u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd24[] = {
    {0xbd24u, 1u, 2u, 0u},
    {0xbd25u, 3u, 3u, 0u},
    {0xbd28u, 1u, 2u, 0u},
    {0xbd29u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd2c[] = {
    {0xbd2cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd2f[] = {
    {0xbd2fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd32[] = {
    {0xbd32u, 2u, 3u, 0u},
    {0xbd73u, 2u, 3u, 0u},
    {0xbd75u, 2u, 2u, 0u},
    {0xbd77u, 2u, 3u, 1u},
    {0xbd79u, 2u, 2u, 0u},
    {0xbd7bu, 2u, 3u, 1u},
    {0xbd7du, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd34[] = {
    {0xbd34u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd37[] = {
    {0xbd37u, 2u, 3u, 0u},
    {0xbd44u, 3u, 3u, 0u},
    {0xbd47u, 3u, 5u, 2u},
    {0xbd4au, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd39[] = {
    {0xbd39u, 2u, 4u, 2u},
    {0xbd3bu, 1u, 2u, 0u},
    {0xbd3cu, 1u, 2u, 0u},
    {0xbd3du, 1u, 2u, 0u},
    {0xbd3eu, 3u, 7u, 2u},
    {0xbd41u, 3u, 8u, 4u},
    {0xbd44u, 3u, 3u, 0u},
    {0xbd47u, 3u, 5u, 2u},
    {0xbd4au, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd4d[] = {
    {0xbd4du, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd50[] = {
    {0xbd50u, 3u, 3u, 0u},
    {0xbd53u, 4u, 8u, 3u},
};

static const FixedSpanOpcode poker_stake_bd57[] = {
    {0xbd57u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd5a[] = {
    {0xbd5au, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd5d[] = {
    {0xbd5du, 2u, 3u, 0u},
    {0xbd5fu, 2u, 2u, 0u},
    {0xbd61u, 2u, 3u, 1u},
    {0xbd63u, 2u, 3u, 0u},
    {0xbd65u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd68[] = {
    {0xbd68u, 1u, 2u, 0u},
    {0xbd69u, 3u, 3u, 0u},
    {0xbd6cu, 1u, 2u, 0u},
    {0xbd6du, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd70[] = {
    {0xbd70u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd73[] = {
    {0xbd73u, 2u, 3u, 0u},
    {0xbd75u, 2u, 2u, 0u},
    {0xbd77u, 2u, 3u, 1u},
    {0xbd79u, 2u, 2u, 0u},
    {0xbd7bu, 2u, 3u, 1u},
    {0xbd7du, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bd80[] = {
    {0xbd80u, 2u, 3u, 0u},
};

static const FixedSpanOpcode poker_stake_bf0b[] = {
    {0xbf0bu, 3u, 3u, 0u},
    {0xbf0eu, 3u, 5u, 2u},
    {0xbf11u, 3u, 3u, 0u},
    {0xbf14u, 3u, 5u, 2u},
    {0xbf17u, 2u, 4u, 2u},
    {0xbf19u, 2u, 7u, 4u},
    {0xbf1bu, 2u, 4u, 2u},
    {0xbf1du, 1u, 2u, 0u},
    {0xbf1eu, 1u, 2u, 0u},
    {0xbf1fu, 1u, 2u, 0u},
    {0xbf20u, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_stake_bf1b[] = {
    {0xbf1bu, 2u, 4u, 2u},
    {0xbf1du, 1u, 2u, 0u},
    {0xbf1eu, 1u, 2u, 0u},
    {0xbf1fu, 1u, 2u, 0u},
    {0xbf20u, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_stake_bf25[] = {
    {0xbf25u, 3u, 5u, 2u},
    {0xbf28u, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_stake_bf2d[] = {
    {0xbf2du, 3u, 5u, 2u},
    {0xbf30u, 3u, 5u, 2u},
    {0xbf33u, 3u, 5u, 2u},
    {0xbf36u, 3u, 5u, 2u},
    {0xbf39u, 4u, 6u, 2u},
    {0xbf3du, 3u, 3u, 0u},
    {0xbf40u, 1u, 2u, 0u},
    {0xbf41u, 3u, 3u, 0u},
    {0xbf44u, 3u, 5u, 2u},
    {0xbf47u, 4u, 6u, 2u},
    {0xbf4bu, 1u, 3u, 0u},
    {0xbf4cu, 3u, 3u, 0u},
    {0xbf4fu, 3u, 5u, 2u},
    {0xbf52u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bf55[] = {
    {0xbf55u, 2u, 7u, 4u},
    {0xbf57u, 3u, 3u, 0u},
    {0xbf5au, 2u, 4u, 2u},
};

static const FixedSpanOpcode poker_stake_bf5e[] = {
    {0xbf5eu, 2u, 3u, 0u},
    {0xbf60u, 2u, 2u, 0u},
    {0xbf62u, 2u, 3u, 1u},
    {0xbf64u, 2u, 3u, 0u},
};

static const FixedSpanOpcode poker_stake_bf67[] = {
    {0xbf67u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bf6a[] = {
    {0xbf6au, 3u, 3u, 0u},
    {0xbf6du, 3u, 5u, 2u},
    {0xbf70u, 2u, 4u, 2u},
};

static const FixedSpanOpcode poker_stake_bf6d[] = {
    {0xbf6du, 3u, 5u, 2u},
    {0xbf70u, 2u, 4u, 2u},
};

static const FixedSpanOpcode poker_stake_bf74[] = {
    {0xbf74u, 1u, 2u, 0u},
    {0xbf75u, 1u, 2u, 0u},
    {0xbf76u, 3u, 3u, 0u},
};

static const FixedSpanOpcode poker_stake_bf7b[] = {
    {0xbf7bu, 3u, 3u, 0u},
};

static const FixedSpanOpcode poker_stake_bf80[] = {
    {0xbf80u, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_stake_bf85[] = {
    {0xbf85u, 3u, 3u, 0u},
    {0xbf88u, 4u, 8u, 3u},
};

static const FixedSpanOpcode poker_stake_bf8c[] = {
    {0xbf8cu, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_stake_bf91[] = {
    {0xbf91u, 3u, 3u, 0u},
    {0xbf94u, 2u, 4u, 2u},
    {0xbf96u, 1u, 2u, 0u},
    {0xbf97u, 1u, 2u, 0u},
    {0xbf98u, 2u, 4u, 2u},
    {0xbf9au, 1u, 2u, 0u},
    {0xbf9bu, 1u, 2u, 0u},
    {0xbf9cu, 2u, 4u, 2u},
    {0xbf9eu, 3u, 5u, 2u},
    {0xbfa1u, 1u, 2u, 0u},
    {0xbfa2u, 2u, 4u, 2u},
    {0xbfa4u, 1u, 2u, 0u},
    {0xbfa5u, 1u, 2u, 0u},
    {0xbfa6u, 2u, 4u, 2u},
    {0xbfa8u, 3u, 5u, 2u},
    {0xbfabu, 4u, 8u, 3u},
};

static const FixedSpanOpcode poker_stake_bfaf[] = {
    {0xbfafu, 1u, 2u, 0u},
    {0xbfb0u, 2u, 4u, 2u},
    {0xbfb2u, 2u, 4u, 2u},
    {0xbfb4u, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_stake_bfb9[] = {
    {0xbfb9u, 2u, 4u, 2u},
    {0xbfbbu, 1u, 2u, 0u},
    {0xbfbcu, 3u, 5u, 2u},
    {0xbfbfu, 2u, 4u, 2u},
    {0xbfc1u, 2u, 4u, 2u},
    {0xbfc3u, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_stake_bfc1[] = {
    {0xbfc1u, 2u, 4u, 2u},
    {0xbfc3u, 3u, 5u, 2u},
};

static const FixedSpanOpcode poker_stake_bfc8[] = {
    {0xbfc8u, 2u, 4u, 2u},
    {0xbfcau, 1u, 2u, 0u},
    {0xbfcbu, 1u, 2u, 0u},
    {0xbfccu, 1u, 2u, 0u},
    {0xbfcdu, 3u, 5u, 2u},
    {0xbfd0u, 1u, 2u, 0u},
    {0xbfd1u, 3u, 5u, 2u},
    {0xbfd4u, 3u, 7u, 2u},
    {0xbfd7u, 3u, 3u, 0u},
    {0xbfdau, 3u, 4u, 0u},
    {0xbfebu, 2u, 4u, 2u},
    {0xbfedu, 3u, 3u, 0u},
    {0xbff0u, 3u, 5u, 2u},
    {0xbff3u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bfdd[] = {
    {0xbfddu, 2u, 4u, 2u},
    {0xbfdfu, 1u, 2u, 0u},
    {0xbfe0u, 1u, 2u, 0u},
    {0xbfe1u, 1u, 2u, 0u},
    {0xbfe2u, 3u, 7u, 2u},
    {0xbfe5u, 3u, 8u, 4u},
    {0xbfe8u, 3u, 3u, 0u},
    {0xbfebu, 2u, 4u, 2u},
    {0xbfedu, 3u, 3u, 0u},
    {0xbff0u, 3u, 5u, 2u},
    {0xbff3u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bff6[] = {
    {0xbff6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_bff9[] = {
    {0xbff9u, 3u, 3u, 0u},
    {0xbffcu, 4u, 8u, 3u},
};

static const FixedSpanOpcode poker_stake_c000[] = {
    {0xc000u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_c003[] = {
    {0xc003u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_c006[] = {
    {0xc006u, 2u, 3u, 0u},
    {0xc008u, 2u, 2u, 0u},
    {0xc00au, 2u, 3u, 1u},
    {0xc00cu, 2u, 3u, 0u},
    {0xc00eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_c011[] = {
    {0xc011u, 1u, 2u, 0u},
    {0xc012u, 3u, 3u, 0u},
    {0xc015u, 1u, 2u, 0u},
    {0xc016u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_c019[] = {
    {0xc019u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_c01c[] = {
    {0xc01cu, 2u, 3u, 0u},
    {0xc01eu, 2u, 2u, 0u},
    {0xc020u, 2u, 3u, 1u},
    {0xc022u, 3u, 6u, 2u},
};

static const FixedSpanOpcode poker_stake_c025[] = {
    {0xc025u, 2u, 3u, 0u},
};

static const PokerStakePhase poker_stake_phases[] = {
    {0xbc78u, 0xbc80u, 0x0000u, 0u, sizeof poker_stake_bc78 / sizeof poker_stake_bc78[0], poker_stake_bc78},
    {0xbc82u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bc82 / sizeof poker_stake_bc82[0], poker_stake_bc82},
    {0xbc85u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bc85 / sizeof poker_stake_bc85[0], poker_stake_bc85},
    {0xbcbeu, 0xbcc0u, 0x0000u, 0u, sizeof poker_stake_bcbe / sizeof poker_stake_bcbe[0], poker_stake_bcbe},
    {0xbcc2u, 0xbcc8u, 0x0000u, 0u, sizeof poker_stake_bcc2 / sizeof poker_stake_bcc2[0], poker_stake_bcc2},
    {0xbccau, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bcca / sizeof poker_stake_bcca[0], poker_stake_bcca},
    {0xbcd0u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bcd0 / sizeof poker_stake_bcd0[0], poker_stake_bcd0},
    {0xbcd3u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bcd3 / sizeof poker_stake_bcd3[0], poker_stake_bcd3},
    {0xbcdcu, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bcdc / sizeof poker_stake_bcdc[0], poker_stake_bcdc},
    {0xbce1u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bce1 / sizeof poker_stake_bce1[0], poker_stake_bce1},
    {0xbce7u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bce7 / sizeof poker_stake_bce7[0], poker_stake_bce7},
    {0xbceau, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bcea / sizeof poker_stake_bcea[0], poker_stake_bcea},
    {0xbcf1u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bcf1 / sizeof poker_stake_bcf1[0], poker_stake_bcf1},
    {0xbcf4u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bcf4 / sizeof poker_stake_bcf4[0], poker_stake_bcf4},
    {0xbcfbu, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bcfb / sizeof poker_stake_bcfb[0], poker_stake_bcfb},
    {0xbd08u, 0xbd0cu, 0x0000u, 1u, sizeof poker_stake_bd08 / sizeof poker_stake_bd08[0], poker_stake_bd08},
    {0xbd0eu, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd0e / sizeof poker_stake_bd0e[0], poker_stake_bd0e},
    {0xbd16u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd16 / sizeof poker_stake_bd16[0], poker_stake_bd16},
    {0xbd19u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd19 / sizeof poker_stake_bd19[0], poker_stake_bd19},
    {0xbd24u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd24 / sizeof poker_stake_bd24[0], poker_stake_bd24},
    {0xbd2cu, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd2c / sizeof poker_stake_bd2c[0], poker_stake_bd2c},
    {0xbd2fu, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd2f / sizeof poker_stake_bd2f[0], poker_stake_bd2f},
    {0xbd32u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd32 / sizeof poker_stake_bd32[0], poker_stake_bd32},
    {0xbd34u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd34 / sizeof poker_stake_bd34[0], poker_stake_bd34},
    {0xbd37u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd37 / sizeof poker_stake_bd37[0], poker_stake_bd37},
    {0xbd39u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd39 / sizeof poker_stake_bd39[0], poker_stake_bd39},
    {0xbd4du, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd4d / sizeof poker_stake_bd4d[0], poker_stake_bd4d},
    {0xbd50u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd50 / sizeof poker_stake_bd50[0], poker_stake_bd50},
    {0xbd57u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd57 / sizeof poker_stake_bd57[0], poker_stake_bd57},
    {0xbd5au, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd5a / sizeof poker_stake_bd5a[0], poker_stake_bd5a},
    {0xbd5du, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd5d / sizeof poker_stake_bd5d[0], poker_stake_bd5d},
    {0xbd68u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd68 / sizeof poker_stake_bd68[0], poker_stake_bd68},
    {0xbd70u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd70 / sizeof poker_stake_bd70[0], poker_stake_bd70},
    {0xbd73u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bd73 / sizeof poker_stake_bd73[0], poker_stake_bd73},
    {0xbd80u, 0x0000u, 0xbd82u, 1u, sizeof poker_stake_bd80 / sizeof poker_stake_bd80[0], poker_stake_bd80},
    {0xbf0bu, 0xbf23u, 0x0000u, 0u, sizeof poker_stake_bf0b / sizeof poker_stake_bf0b[0], poker_stake_bf0b},
    {0xbf1bu, 0xbf23u, 0x0000u, 0u, sizeof poker_stake_bf1b / sizeof poker_stake_bf1b[0], poker_stake_bf1b},
    {0xbf25u, 0xbf2bu, 0x0000u, 0u, sizeof poker_stake_bf25 / sizeof poker_stake_bf25[0], poker_stake_bf25},
    {0xbf2du, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bf2d / sizeof poker_stake_bf2d[0], poker_stake_bf2d},
    {0xbf55u, 0xbf5cu, 0x0000u, 0u, sizeof poker_stake_bf55 / sizeof poker_stake_bf55[0], poker_stake_bf55},
    {0xbf5eu, 0x0000u, 0xbf66u, 0u, sizeof poker_stake_bf5e / sizeof poker_stake_bf5e[0], poker_stake_bf5e},
    {0xbf67u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bf67 / sizeof poker_stake_bf67[0], poker_stake_bf67},
    {0xbf6au, 0xbf72u, 0x0000u, 0u, sizeof poker_stake_bf6a / sizeof poker_stake_bf6a[0], poker_stake_bf6a},
    {0xbf6du, 0xbf72u, 0x0000u, 0u, sizeof poker_stake_bf6d / sizeof poker_stake_bf6d[0], poker_stake_bf6d},
    {0xbf74u, 0xbf79u, 0x0000u, 0u, sizeof poker_stake_bf74 / sizeof poker_stake_bf74[0], poker_stake_bf74},
    {0xbf7bu, 0xbf7eu, 0x0000u, 0u, sizeof poker_stake_bf7b / sizeof poker_stake_bf7b[0], poker_stake_bf7b},
    {0xbf80u, 0xbf83u, 0x0000u, 0u, sizeof poker_stake_bf80 / sizeof poker_stake_bf80[0], poker_stake_bf80},
    {0xbf85u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bf85 / sizeof poker_stake_bf85[0], poker_stake_bf85},
    {0xbf8cu, 0xbf8fu, 0x0000u, 0u, sizeof poker_stake_bf8c / sizeof poker_stake_bf8c[0], poker_stake_bf8c},
    {0xbf91u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bf91 / sizeof poker_stake_bf91[0], poker_stake_bf91},
    {0xbfafu, 0xbfb7u, 0x0000u, 0u, sizeof poker_stake_bfaf / sizeof poker_stake_bfaf[0], poker_stake_bfaf},
    {0xbfb9u, 0xbfc6u, 0x0000u, 0u, sizeof poker_stake_bfb9 / sizeof poker_stake_bfb9[0], poker_stake_bfb9},
    {0xbfc1u, 0xbfc6u, 0x0000u, 0u, sizeof poker_stake_bfc1 / sizeof poker_stake_bfc1[0], poker_stake_bfc1},
    {0xbfc8u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bfc8 / sizeof poker_stake_bfc8[0], poker_stake_bfc8},
    {0xbfddu, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bfdd / sizeof poker_stake_bfdd[0], poker_stake_bfdd},
    {0xbff6u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bff6 / sizeof poker_stake_bff6[0], poker_stake_bff6},
    {0xbff9u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_bff9 / sizeof poker_stake_bff9[0], poker_stake_bff9},
    {0xc000u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_c000 / sizeof poker_stake_c000[0], poker_stake_c000},
    {0xc003u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_c003 / sizeof poker_stake_c003[0], poker_stake_c003},
    {0xc006u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_c006 / sizeof poker_stake_c006[0], poker_stake_c006},
    {0xc011u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_c011 / sizeof poker_stake_c011[0], poker_stake_c011},
    {0xc019u, 0x0000u, 0x0000u, 0u, sizeof poker_stake_c019 / sizeof poker_stake_c019[0], poker_stake_c019},
    {0xc01cu, 0x0000u, 0x0000u, 0u, sizeof poker_stake_c01c / sizeof poker_stake_c01c[0], poker_stake_c01c},
    {0xc025u, 0x0000u, 0xc027u, 1u, sizeof poker_stake_c025 / sizeof poker_stake_c025[0], poker_stake_c025},
};

typedef struct PokerStakeProjection {
    uint16_t seat;
    uint16_t rank_index;
    uint16_t budget;
} PokerStakeProjection;

static const PokerStakePhase *FindPokerStakePhase(uint16_t pc) {
    for (unsigned i = 0u; i < sizeof poker_stake_phases / sizeof poker_stake_phases[0]; ++i)
        if (poker_stake_phases[i].pc == pc)
            return poker_stake_phases + i;
    return NULL;
}

static uint16_t PokerStakeWord(const CpuState *cpu, unsigned address) {
    return cpu->ram[address] | (uint16_t)cpu->ram[address + 1u] << 8;
}

static bool AppendPokerStakePhase(CardScoreClock *clock, uint16_t pc,
                                    PokerStakeProjection projection) {
    const PokerStakePhase *phase = FindPokerStakePhase(pc);
    const CpuState *cpu = clock->call.cpu;
    if (!phase)
        return false;
    if (pc == 0xbf0bu)
        projection.seat = 1u;
    if (pc == 0xbf55u) {
        if (projection.seat >= 4u)
            return false;
        ++projection.seat;
    }
    if (pc == 0xbf6au)
        projection.rank_index = 0u;
    if (pc == 0xbf74u)
        projection.rank_index += 2u;
    if (pc == 0xbfafu)
        projection.budget = (uint16_t)(cpu->A + PokerStakeWord(cpu, 0u));
    if (pc == 0xbfb9u)
        projection.budget = (uint16_t)(projection.budget + PokerStakeWord(cpu, 0x711fu));
    for (unsigned i = 0u; i < phase->count; ++i) {
        const FixedSpanOpcode *op = phase->opcodes + i;
        unsigned cycles = op->cycles;
        unsigned base = 0u, index = 0u;
        switch (op->pc) {
        case 0xbc7du:
        case 0xbf20u: base = 0x7107u; index = (uint16_t)(projection.seat * 2u); break;
        case 0xbf25u: base = 0x7127u; index = (uint16_t)(projection.seat * 2u); break;
        case 0xbcb5u:
        case 0xbf2du: base = 0x711fu; index = (uint16_t)(projection.seat * 2u); break;
        case 0xbcafu:
        case 0xbf33u: base = 0x7117u; index = (uint16_t)(projection.seat * 2u); break;
        case 0xbf6du: base = 0x7141u; index = projection.rank_index; break;
        case 0xbf80u:
        case 0xbfb4u: base = 0x70e7u; index = projection.rank_index; break;
        default: break;
        }
        if (base && (base >> 8) != ((base + index) >> 8))
            ++cycles;
        CardScoreOpcode(clock, op->pc, op->fetched, cycles, op->data_bytes);
    }
    if (phase->return_pc) {
        clock->returning = true;
        clock->next = 0x840000u | phase->return_pc;
    }
    if (!phase->branch)
        return true;
    const unsigned seat_index = (uint16_t)(projection.seat * 2u);
    bool taken;
    uint16_t target;
    switch (phase->branch) {
    case 0xbc80u:
        taken = PokerStakeWord(cpu, 0x7107u + seat_index) != 0u;
        target = taken ? 0xbc85u : 0xbc82u;
        break;
    case 0xbcc0u:
        taken = !PokerStakeWord(cpu, 4u);
        target = taken ? 0xbd39u : 0xbcc2u;
        break;
    case 0xbcc8u:
        taken = PokerStakeWord(cpu, 4u) == 1u;
        target = taken ? 0xbd34u : 0xbccau;
        break;
    case 0xbd0cu:
        taken = (uint16_t)(PokerStakeWord(cpu, 0x22u) - 1u) != 0u;
        target = taken ? 0xbcdcu : 0xbd0eu;
        break;
    case 0xbf23u:
        taken = !PokerStakeWord(cpu, 0x7107u + seat_index);
        target = taken ? 0xbf55u : 0xbf25u;
        break;
    case 0xbf2bu:
        taken = PokerStakeWord(cpu, 0x7127u + seat_index) == PokerStakeWord(cpu, 0x7127u);
        target = taken ? 0xbf55u : 0xbf2du;
        break;
    case 0xbf5cu:
        taken = projection.seat != 4u;
        target = taken ? 0xbf1bu : 0xbf5eu;
        break;
    case 0xbf72u:
        taken = PokerStakeWord(cpu, 0x7141u + projection.rank_index) == projection.seat;
        target = taken ? 0xbf7bu : 0xbf74u;
        break;
    case 0xbf79u:
        taken = projection.rank_index != 8u;
        target = taken ? 0xbf6du : 0xbf7bu;
        break;
    case 0xbf7eu:
        taken = !projection.rank_index;
        target = taken ? 0xbfc8u : 0xbf80u;
        break;
    case 0xbf83u:
        taken = PokerStakeWord(cpu, 0x70e7u + projection.rank_index) != 0u;
        target = taken ? 0xbf91u : 0xbf85u;
        break;
    case 0xbf8fu:
        taken = ((uint16_t)(cpu->A - PokerStakeWord(cpu, 0x711fu)) & 0x8000u) != 0u;
        target = taken ? 0xbfc8u : 0xbf91u;
        break;
    case 0xbfb7u:
        taken = !PokerStakeWord(cpu, 0x70e7u + projection.rank_index);
        target = taken ? 0xbfc1u : 0xbfb9u;
        break;
    case 0xbfc6u:
        taken = ((uint16_t)(projection.budget - PokerStakeWord(cpu, 0x7127u)) & 0x8000u) != 0u;
        target = taken ? 0xbfddu : 0xbfc8u;
        break;
    default:
        return false;
    }
    CardScoreOpcode(clock, phase->branch, 2u, taken ? 3u : 2u, 0u);
    return AppendPokerStakePhase(clock, target, projection);
}

static bool PreparePokerStake(CardScoreClock *clock, uint32_t pc) {
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
    const PokerStakePhase *phase = FindPokerStakePhase((uint16_t)pc);
    if (!phase || cpu->m_flag != phase->byte_mode)
        return false;
    const PokerStakeProjection projection = {
        PokerStakeWord(cpu, 0x11u), cpu->X, PokerStakeWord(cpu, 0u)};
    return AppendPokerStakePhase(clock, (uint16_t)pc, projection) &&
        CardScoreWindowSupported(clock);
}

static uint8_t PokerStakeChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    CardScoreClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PreparePokerStake(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgePokerStake(CpuState *cpu, uint32_t entry,
                                         ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    CardScoreClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || !PreparePokerStake(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, PokerStakeChild, &clock);
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
