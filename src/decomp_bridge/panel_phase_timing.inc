typedef struct PanelTimingSegment {
    uint16_t pc;
    unsigned count;
    const FixedSpanOpcode *timing;
} PanelTimingSegment;

static const FixedSpanOpcode panel_timing_ee4b[] = {
    {0xee4bu, 1u, 3u, 1u},
    {0xee4cu, 2u, 3u, 0u},
    {0xee4eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ee51[] = {
    {0xee51u, 3u, 3u, 0u},
    {0xee54u, 3u, 5u, 2u},
    {0xee57u, 1u, 3u, 1u},
    {0xee58u, 3u, 3u, 0u},
    {0xee5bu, 3u, 3u, 0u},
    {0xee5eu, 3u, 3u, 0u},
};

static const FixedSpanOpcode panel_timing_ee64[] = {
    {0xee64u, 1u, 4u, 1u},
    {0xee65u, 2u, 3u, 0u},
    {0xee67u, 2u, 2u, 0u},
    {0xee69u, 2u, 3u, 1u},
    {0xee6bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ee6e[] = {
    {0xee6eu, 2u, 3u, 0u},
    {0xee70u, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ee73[] = {
    {0xee73u, 3u, 3u, 0u},
    {0xee76u, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ee79[] = {
    {0xee79u, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ee7c[] = {
    {0xee7cu, 3u, 3u, 0u},
    {0xee7fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ee82[] = {
    {0xee82u, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ee85[] = {
    {0xee85u, 2u, 3u, 0u},
    {0xee87u, 2u, 2u, 0u},
    {0xee89u, 2u, 3u, 1u},
    {0xee8bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ee8e[] = {
    {0xee8eu, 2u, 2u, 0u},
    {0xee90u, 2u, 3u, 1u},
    {0xee92u, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ee95[] = {
    {0xee95u, 2u, 3u, 0u},
    {0xee97u, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ee9a[] = {
    {0xee9au, 3u, 3u, 0u},
    {0xee9du, 1u, 4u, 2u},
    {0xee9eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ee9d[] = {
    {0xee9du, 1u, 4u, 2u},
    {0xee9eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_eea1[] = {
    {0xeea1u, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_eea4[] = {
    {0xeea4u, 3u, 5u, 2u},
    {0xeea7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_eeaa[] = {
    {0xeeaau, 3u, 5u, 2u},
    {0xeeadu, 1u, 5u, 2u},
    {0xeeaeu, 3u, 3u, 0u},
};

static const FixedSpanOpcode panel_timing_eeb3[] = {
    {0xeeb3u, 3u, 3u, 0u},
};

static const FixedSpanOpcode panel_timing_eeb8[] = {
    {0xeeb8u, 2u, 3u, 0u},
};

static const FixedSpanOpcode panel_timing_eeba[] = {
    {0xeebau, 3u, 3u, 0u},
};

static const FixedSpanOpcode panel_timing_eebf[] = {
    {0xeebfu, 3u, 3u, 0u},
    {0xeec2u, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_eec5[] = {
    {0xeec5u, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_eec8[] = {
    {0xeec8u, 2u, 3u, 0u},
    {0xeecau, 2u, 2u, 0u},
    {0xeeccu, 2u, 3u, 1u},
    {0xeeceu, 2u, 3u, 0u},
    {0xeed0u, 3u, 3u, 0u},
    {0xeed3u, 2u, 3u, 0u},
};

static const FixedSpanOpcode panel_timing_eed5[] = {
    {0xeed5u, 3u, 3u, 0u},
    {0xeed8u, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_eedb[] = {
    {0xeedbu, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_eede[] = {
    {0xeedeu, 2u, 3u, 0u},
    {0xeee0u, 2u, 2u, 0u},
    {0xeee2u, 2u, 3u, 1u},
    {0xeee4u, 2u, 3u, 0u},
    {0xeee6u, 3u, 3u, 0u},
    {0xeee9u, 2u, 3u, 0u},
};

static const FixedSpanOpcode panel_timing_eeeb[] = {
    {0xeeebu, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_eeee[] = {
    {0xeeeeu, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_eef1[] = {
    {0xeef1u, 3u, 3u, 0u},
    {0xeef4u, 3u, 5u, 2u},
    {0xeef7u, 1u, 3u, 1u},
    {0xeef8u, 3u, 3u, 0u},
    {0xeefbu, 3u, 3u, 0u},
    {0xeefeu, 3u, 3u, 0u},
};

static const FixedSpanOpcode panel_timing_ef04[] = {
    {0xef04u, 1u, 4u, 1u},
    {0xef05u, 2u, 3u, 0u},
    {0xef07u, 2u, 2u, 0u},
    {0xef09u, 2u, 3u, 1u},
    {0xef0bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ef0e[] = {
    {0xef0eu, 2u, 3u, 0u},
    {0xef10u, 3u, 3u, 0u},
    {0xef13u, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ef16[] = {
    {0xef16u, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ef19[] = {
    {0xef19u, 3u, 3u, 0u},
    {0xef1cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ef1f[] = {
    {0xef1fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ef22[] = {
    {0xef22u, 3u, 3u, 0u},
    {0xef25u, 3u, 5u, 2u},
    {0xef28u, 3u, 3u, 0u},
    {0xef2bu, 2u, 4u, 2u},
    {0xef2du, 3u, 3u, 0u},
    {0xef30u, 3u, 3u, 0u},
    {0xef33u, 4u, 8u, 3u},
};

static const FixedSpanOpcode panel_timing_ef37[] = {
    {0xef37u, 2u, 3u, 0u},
    {0xef39u, 2u, 2u, 0u},
    {0xef3bu, 2u, 3u, 1u},
    {0xef3du, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ef40[] = {
    {0xef40u, 2u, 2u, 0u},
    {0xef42u, 2u, 3u, 1u},
    {0xef44u, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ef47[] = {
    {0xef47u, 2u, 3u, 0u},
    {0xef49u, 3u, 6u, 2u},
};

static const FixedSpanOpcode panel_timing_ef4c[] = {
    {0xef4cu, 1u, 4u, 1u},
};

static const PanelTimingSegment panel_timing_segments[] = {
    {0xee4bu, 3u, panel_timing_ee4b},
    {0xee51u, 6u, panel_timing_ee51},
    {0xee64u, 5u, panel_timing_ee64},
    {0xee6eu, 2u, panel_timing_ee6e},
    {0xee73u, 2u, panel_timing_ee73},
    {0xee79u, 1u, panel_timing_ee79},
    {0xee7cu, 2u, panel_timing_ee7c},
    {0xee82u, 1u, panel_timing_ee82},
    {0xee85u, 4u, panel_timing_ee85},
    {0xee8eu, 3u, panel_timing_ee8e},
    {0xee95u, 2u, panel_timing_ee95},
    {0xee9au, 3u, panel_timing_ee9a},
    {0xee9du, 2u, panel_timing_ee9d},
    {0xeea1u, 1u, panel_timing_eea1},
    {0xeea4u, 2u, panel_timing_eea4},
    {0xeeaau, 3u, panel_timing_eeaa},
    {0xeeb3u, 1u, panel_timing_eeb3},
    {0xeeb8u, 1u, panel_timing_eeb8},
    {0xeebau, 1u, panel_timing_eeba},
    {0xeebfu, 2u, panel_timing_eebf},
    {0xeec5u, 1u, panel_timing_eec5},
    {0xeec8u, 6u, panel_timing_eec8},
    {0xeed5u, 2u, panel_timing_eed5},
    {0xeedbu, 1u, panel_timing_eedb},
    {0xeedeu, 6u, panel_timing_eede},
    {0xeeebu, 1u, panel_timing_eeeb},
    {0xeeeeu, 1u, panel_timing_eeee},
    {0xeef1u, 6u, panel_timing_eef1},
    {0xef04u, 5u, panel_timing_ef04},
    {0xef0eu, 3u, panel_timing_ef0e},
    {0xef16u, 1u, panel_timing_ef16},
    {0xef19u, 2u, panel_timing_ef19},
    {0xef1fu, 1u, panel_timing_ef1f},
    {0xef22u, 7u, panel_timing_ef22},
    {0xef37u, 4u, panel_timing_ef37},
    {0xef40u, 3u, panel_timing_ef40},
    {0xef47u, 2u, panel_timing_ef47},
    {0xef4cu, 1u, panel_timing_ef4c},
};
