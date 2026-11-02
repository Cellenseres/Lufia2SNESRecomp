typedef struct MemberChangePlan {
    const FixedSpanOpcode *timing;
    unsigned count;
    uint16_t returned;
    uint8_t frame_size;
} MemberChangePlan;

static const FixedSpanOpcode MemberChange0_enabled[] = {
    {0xf639u, 2u, 3u, 0u},
    {0xf63bu, 1u, 2u, 0u},
    {0xf63cu, 1u, 2u, 0u},
    {0xf63du, 2u, 3u, 0u},
    {0xf63fu, 3u, 6u, 2u},
    {0xf7bdu, 1u, 3u, 1u},
    {0xf7beu, 2u, 3u, 0u},
    {0xf7c0u, 1u, 2u, 0u},
    {0xf7c1u, 1u, 2u, 0u},
    {0xf7c2u, 1u, 2u, 0u},
    {0xf7c3u, 4u, 6u, 2u},
    {0xf7c7u, 1u, 2u, 0u},
    {0xf7c8u, 1u, 4u, 1u},
    {0xf7c9u, 1u, 6u, 2u},
    {0xf642u, 1u, 2u, 0u},
    {0xf643u, 3u, 6u, 2u},
    {0xf655u, 3u, 5u, 1u},
    {0xf658u, 3u, 6u, 1u},
    {0xf65bu, 2u, 2u, 0u},
    {0xf65du, 2u, 3u, 0u},
    {0xf669u, 1u, 6u, 2u},
};
static const MemberChangePlan MemberPlan0_enabled = {
    MemberChange0_enabled, sizeof MemberChange0_enabled / sizeof MemberChange0_enabled[0], 0xf646u, 3u
};

static const FixedSpanOpcode MemberChange0_disabled[] = {
    {0xf639u, 2u, 3u, 0u},
    {0xf63bu, 1u, 2u, 0u},
    {0xf63cu, 1u, 2u, 0u},
    {0xf63du, 2u, 3u, 0u},
    {0xf63fu, 3u, 6u, 2u},
    {0xf7bdu, 1u, 3u, 1u},
    {0xf7beu, 2u, 3u, 0u},
    {0xf7c0u, 1u, 2u, 0u},
    {0xf7c1u, 1u, 2u, 0u},
    {0xf7c2u, 1u, 2u, 0u},
    {0xf7c3u, 4u, 6u, 2u},
    {0xf7c7u, 1u, 2u, 0u},
    {0xf7c8u, 1u, 4u, 1u},
    {0xf7c9u, 1u, 6u, 2u},
    {0xf642u, 1u, 2u, 0u},
    {0xf643u, 3u, 6u, 2u},
    {0xf655u, 3u, 5u, 1u},
    {0xf658u, 3u, 6u, 1u},
    {0xf65bu, 2u, 2u, 0u},
    {0xf65du, 2u, 2u, 0u},
    {0xf65fu, 2u, 3u, 0u},
    {0xf661u, 3u, 10u, 4u},
    {0xf664u, 3u, 7u, 2u},
    {0xf667u, 2u, 3u, 0u},
    {0xf669u, 1u, 6u, 2u},
};
static const MemberChangePlan MemberPlan0_disabled = {
    MemberChange0_disabled, sizeof MemberChange0_disabled / sizeof MemberChange0_disabled[0], 0xf646u, 3u
};

static const FixedSpanOpcode MemberChange1_revive[] = {
    {0xf647u, 2u, 3u, 0u},
    {0xf649u, 1u, 2u, 0u},
    {0xf64au, 1u, 2u, 0u},
    {0xf64bu, 2u, 3u, 0u},
    {0xf64du, 3u, 6u, 2u},
    {0xf7bdu, 1u, 3u, 1u},
    {0xf7beu, 2u, 3u, 0u},
    {0xf7c0u, 1u, 2u, 0u},
    {0xf7c1u, 1u, 2u, 0u},
    {0xf7c2u, 1u, 2u, 0u},
    {0xf7c3u, 4u, 6u, 2u},
    {0xf7c7u, 1u, 2u, 0u},
    {0xf7c8u, 1u, 4u, 1u},
    {0xf7c9u, 1u, 6u, 2u},
    {0xf650u, 1u, 2u, 0u},
    {0xf651u, 3u, 6u, 2u},
    {0xf66au, 2u, 2u, 0u},
    {0xf66cu, 3u, 5u, 1u},
    {0xf66fu, 3u, 6u, 1u},
    {0xf672u, 2u, 2u, 0u},
    {0xf674u, 2u, 2u, 0u},
    {0xf676u, 2u, 3u, 0u},
    {0xf678u, 3u, 6u, 2u},
    {0xf67bu, 2u, 2u, 0u},
    {0xf67du, 3u, 10u, 4u},
    {0xf680u, 2u, 3u, 0u},
    {0xf682u, 1u, 6u, 2u},
};
static const MemberChangePlan MemberPlan1_revive = {
    MemberChange1_revive, sizeof MemberChange1_revive / sizeof MemberChange1_revive[0], 0xf654u, 3u
};

static const FixedSpanOpcode MemberChange1_alive[] = {
    {0xf647u, 2u, 3u, 0u},
    {0xf649u, 1u, 2u, 0u},
    {0xf64au, 1u, 2u, 0u},
    {0xf64bu, 2u, 3u, 0u},
    {0xf64du, 3u, 6u, 2u},
    {0xf7bdu, 1u, 3u, 1u},
    {0xf7beu, 2u, 3u, 0u},
    {0xf7c0u, 1u, 2u, 0u},
    {0xf7c1u, 1u, 2u, 0u},
    {0xf7c2u, 1u, 2u, 0u},
    {0xf7c3u, 4u, 6u, 2u},
    {0xf7c7u, 1u, 2u, 0u},
    {0xf7c8u, 1u, 4u, 1u},
    {0xf7c9u, 1u, 6u, 2u},
    {0xf650u, 1u, 2u, 0u},
    {0xf651u, 3u, 6u, 2u},
    {0xf66au, 2u, 2u, 0u},
    {0xf66cu, 3u, 5u, 1u},
    {0xf66fu, 3u, 6u, 1u},
    {0xf672u, 2u, 2u, 0u},
    {0xf674u, 2u, 2u, 0u},
    {0xf676u, 2u, 3u, 0u},
    {0xf678u, 3u, 6u, 2u},
    {0xf67bu, 2u, 3u, 0u},
    {0xf680u, 2u, 3u, 0u},
    {0xf682u, 1u, 6u, 2u},
};
static const MemberChangePlan MemberPlan1_alive = {
    MemberChange1_alive, sizeof MemberChange1_alive / sizeof MemberChange1_alive[0], 0xf654u, 3u
};

static const FixedSpanOpcode MemberChange1_disabled[] = {
    {0xf647u, 2u, 3u, 0u},
    {0xf649u, 1u, 2u, 0u},
    {0xf64au, 1u, 2u, 0u},
    {0xf64bu, 2u, 3u, 0u},
    {0xf64du, 3u, 6u, 2u},
    {0xf7bdu, 1u, 3u, 1u},
    {0xf7beu, 2u, 3u, 0u},
    {0xf7c0u, 1u, 2u, 0u},
    {0xf7c1u, 1u, 2u, 0u},
    {0xf7c2u, 1u, 2u, 0u},
    {0xf7c3u, 4u, 6u, 2u},
    {0xf7c7u, 1u, 2u, 0u},
    {0xf7c8u, 1u, 4u, 1u},
    {0xf7c9u, 1u, 6u, 2u},
    {0xf650u, 1u, 2u, 0u},
    {0xf651u, 3u, 6u, 2u},
    {0xf66au, 2u, 2u, 0u},
    {0xf66cu, 3u, 5u, 1u},
    {0xf66fu, 3u, 6u, 1u},
    {0xf672u, 2u, 2u, 0u},
    {0xf674u, 2u, 3u, 0u},
    {0xf682u, 1u, 6u, 2u},
};
static const MemberChangePlan MemberPlan1_disabled = {
    MemberChange1_disabled, sizeof MemberChange1_disabled / sizeof MemberChange1_disabled[0], 0xf654u, 3u
};

static const FixedSpanOpcode MemberChange2_enabled[] = {
    {0xf655u, 3u, 5u, 1u},
    {0xf658u, 3u, 6u, 1u},
    {0xf65bu, 2u, 2u, 0u},
    {0xf65du, 2u, 3u, 0u},
};
static const MemberChangePlan MemberPlan2_enabled = {
    MemberChange2_enabled, sizeof MemberChange2_enabled / sizeof MemberChange2_enabled[0], 0xf669u, 2u
};

static const FixedSpanOpcode MemberChange2_disabled[] = {
    {0xf655u, 3u, 5u, 1u},
    {0xf658u, 3u, 6u, 1u},
    {0xf65bu, 2u, 2u, 0u},
    {0xf65du, 2u, 2u, 0u},
    {0xf65fu, 2u, 3u, 0u},
    {0xf661u, 3u, 10u, 4u},
    {0xf664u, 3u, 7u, 2u},
    {0xf667u, 2u, 3u, 0u},
};
static const MemberChangePlan MemberPlan2_disabled = {
    MemberChange2_disabled, sizeof MemberChange2_disabled / sizeof MemberChange2_disabled[0], 0xf669u, 2u
};

static const FixedSpanOpcode MemberChange3_revive[] = {
    {0xf66au, 2u, 2u, 0u},
    {0xf66cu, 3u, 5u, 1u},
    {0xf66fu, 3u, 6u, 1u},
    {0xf672u, 2u, 2u, 0u},
    {0xf674u, 2u, 2u, 0u},
    {0xf676u, 2u, 3u, 0u},
    {0xf678u, 3u, 6u, 2u},
    {0xf67bu, 2u, 2u, 0u},
    {0xf67du, 3u, 10u, 4u},
    {0xf680u, 2u, 3u, 0u},
};
static const MemberChangePlan MemberPlan3_revive = {
    MemberChange3_revive, sizeof MemberChange3_revive / sizeof MemberChange3_revive[0], 0xf682u, 2u
};

static const FixedSpanOpcode MemberChange3_alive[] = {
    {0xf66au, 2u, 2u, 0u},
    {0xf66cu, 3u, 5u, 1u},
    {0xf66fu, 3u, 6u, 1u},
    {0xf672u, 2u, 2u, 0u},
    {0xf674u, 2u, 2u, 0u},
    {0xf676u, 2u, 3u, 0u},
    {0xf678u, 3u, 6u, 2u},
    {0xf67bu, 2u, 3u, 0u},
    {0xf680u, 2u, 3u, 0u},
};
static const MemberChangePlan MemberPlan3_alive = {
    MemberChange3_alive, sizeof MemberChange3_alive / sizeof MemberChange3_alive[0], 0xf682u, 2u
};

static const FixedSpanOpcode MemberChange3_disabled[] = {
    {0xf66au, 2u, 2u, 0u},
    {0xf66cu, 3u, 5u, 1u},
    {0xf66fu, 3u, 6u, 1u},
    {0xf672u, 2u, 2u, 0u},
    {0xf674u, 2u, 3u, 0u},
};
static const MemberChangePlan MemberPlan3_disabled = {
    MemberChange3_disabled, sizeof MemberChange3_disabled / sizeof MemberChange3_disabled[0], 0xf682u, 2u
};

static const FixedSpanOpcode MemberChange4_sum[] = {
    {0xf683u, 2u, 3u, 0u},
    {0xf685u, 1u, 2u, 0u},
    {0xf686u, 2u, 3u, 0u},
    {0xf688u, 3u, 6u, 2u},
    {0xf7bdu, 1u, 3u, 1u},
    {0xf7beu, 2u, 3u, 0u},
    {0xf7c0u, 1u, 2u, 0u},
    {0xf7c1u, 1u, 2u, 0u},
    {0xf7c2u, 1u, 2u, 0u},
    {0xf7c3u, 4u, 6u, 2u},
    {0xf7c7u, 1u, 2u, 0u},
    {0xf7c8u, 1u, 4u, 1u},
    {0xf7c9u, 1u, 6u, 2u},
    {0xf68bu, 2u, 3u, 0u},
    {0xf68du, 1u, 2u, 0u},
    {0xf68eu, 1u, 2u, 0u},
    {0xf68fu, 3u, 6u, 2u},
    {0xf692u, 3u, 6u, 2u},
    {0xf695u, 2u, 3u, 0u},
    {0xf6a2u, 3u, 7u, 2u},
    {0xf6a5u, 2u, 3u, 0u},
};
static const MemberChangePlan MemberPlan4_sum = {
    MemberChange4_sum, sizeof MemberChange4_sum / sizeof MemberChange4_sum[0], 0xf6a7u, 3u
};

static const FixedSpanOpcode MemberChange4_floor[] = {
    {0xf683u, 2u, 3u, 0u},
    {0xf685u, 1u, 2u, 0u},
    {0xf686u, 2u, 3u, 0u},
    {0xf688u, 3u, 6u, 2u},
    {0xf7bdu, 1u, 3u, 1u},
    {0xf7beu, 2u, 3u, 0u},
    {0xf7c0u, 1u, 2u, 0u},
    {0xf7c1u, 1u, 2u, 0u},
    {0xf7c2u, 1u, 2u, 0u},
    {0xf7c3u, 4u, 6u, 2u},
    {0xf7c7u, 1u, 2u, 0u},
    {0xf7c8u, 1u, 4u, 1u},
    {0xf7c9u, 1u, 6u, 2u},
    {0xf68bu, 2u, 3u, 0u},
    {0xf68du, 1u, 2u, 0u},
    {0xf68eu, 1u, 2u, 0u},
    {0xf68fu, 3u, 6u, 2u},
    {0xf692u, 3u, 6u, 2u},
    {0xf695u, 2u, 2u, 0u},
    {0xf697u, 1u, 2u, 0u},
    {0xf698u, 3u, 6u, 2u},
    {0xf69bu, 3u, 6u, 2u},
    {0xf69eu, 1u, 2u, 0u},
    {0xf69fu, 3u, 6u, 2u},
    {0xf6a2u, 3u, 7u, 2u},
    {0xf6a5u, 2u, 3u, 0u},
};
static const MemberChangePlan MemberPlan4_floor = {
    MemberChange4_floor, sizeof MemberChange4_floor / sizeof MemberChange4_floor[0], 0xf6a7u, 3u
};

static const FixedSpanOpcode MemberChange5_zero[] = {
    {0xf6a8u, 2u, 3u, 0u},
    {0xf6aau, 1u, 2u, 0u},
    {0xf6abu, 2u, 3u, 0u},
    {0xf6adu, 3u, 6u, 2u},
    {0xf7bdu, 1u, 3u, 1u},
    {0xf7beu, 2u, 3u, 0u},
    {0xf7c0u, 1u, 2u, 0u},
    {0xf7c1u, 1u, 2u, 0u},
    {0xf7c2u, 1u, 2u, 0u},
    {0xf7c3u, 4u, 6u, 2u},
    {0xf7c7u, 1u, 2u, 0u},
    {0xf7c8u, 1u, 4u, 1u},
    {0xf7c9u, 1u, 6u, 2u},
    {0xf6b0u, 2u, 3u, 0u},
    {0xf6b2u, 1u, 2u, 0u},
    {0xf6b3u, 1u, 2u, 0u},
    {0xf6b4u, 3u, 6u, 2u},
    {0xf6b7u, 2u, 3u, 0u},
    {0xf6c7u, 3u, 6u, 2u},
    {0xf6cau, 3u, 7u, 2u},
    {0xf6cdu, 3u, 3u, 0u},
    {0xf6d0u, 3u, 6u, 2u},
    {0xf655u, 3u, 6u, 2u},
    {0xf658u, 3u, 7u, 2u},
    {0xf65bu, 3u, 3u, 0u},
    {0xf65eu, 1u, 2u, 0u},
    {0xf65fu, 2u, 3u, 0u},
    {0xf661u, 3u, 10u, 4u},
    {0xf664u, 3u, 7u, 2u},
    {0xf667u, 2u, 3u, 0u},
    {0xf669u, 1u, 6u, 2u},
    {0xf6d3u, 2u, 3u, 0u},
    {0xf6d5u, 1u, 2u, 0u},
};
static const MemberChangePlan MemberPlan5_zero = {
    MemberChange5_zero, sizeof MemberChange5_zero / sizeof MemberChange5_zero[0], 0xf6d6u, 3u
};

static const FixedSpanOpcode MemberChange5_borrow[] = {
    {0xf6a8u, 2u, 3u, 0u},
    {0xf6aau, 1u, 2u, 0u},
    {0xf6abu, 2u, 3u, 0u},
    {0xf6adu, 3u, 6u, 2u},
    {0xf7bdu, 1u, 3u, 1u},
    {0xf7beu, 2u, 3u, 0u},
    {0xf7c0u, 1u, 2u, 0u},
    {0xf7c1u, 1u, 2u, 0u},
    {0xf7c2u, 1u, 2u, 0u},
    {0xf7c3u, 4u, 6u, 2u},
    {0xf7c7u, 1u, 2u, 0u},
    {0xf7c8u, 1u, 4u, 1u},
    {0xf7c9u, 1u, 6u, 2u},
    {0xf6b0u, 2u, 3u, 0u},
    {0xf6b2u, 1u, 2u, 0u},
    {0xf6b3u, 1u, 2u, 0u},
    {0xf6b4u, 3u, 6u, 2u},
    {0xf6b7u, 2u, 2u, 0u},
    {0xf6b9u, 3u, 5u, 2u},
    {0xf6bcu, 1u, 2u, 0u},
    {0xf6bdu, 3u, 6u, 2u},
    {0xf6c0u, 3u, 7u, 2u},
    {0xf6c3u, 2u, 3u, 0u},
    {0xf6c5u, 1u, 2u, 0u},
};
static const MemberChangePlan MemberPlan5_borrow = {
    MemberChange5_borrow, sizeof MemberChange5_borrow / sizeof MemberChange5_borrow[0], 0xf6c6u, 2u
};

static const FixedSpanOpcode MemberChange6_sum[] = {
    {0xf6d7u, 2u, 3u, 0u},
    {0xf6d9u, 1u, 2u, 0u},
    {0xf6dau, 2u, 3u, 0u},
    {0xf6dcu, 3u, 6u, 2u},
    {0xf7bdu, 1u, 3u, 1u},
    {0xf7beu, 2u, 3u, 0u},
    {0xf7c0u, 1u, 2u, 0u},
    {0xf7c1u, 1u, 2u, 0u},
    {0xf7c2u, 1u, 2u, 0u},
    {0xf7c3u, 4u, 6u, 2u},
    {0xf7c7u, 1u, 2u, 0u},
    {0xf7c8u, 1u, 4u, 1u},
    {0xf7c9u, 1u, 6u, 2u},
    {0xf6dfu, 2u, 3u, 0u},
    {0xf6e1u, 1u, 2u, 0u},
    {0xf6e2u, 1u, 2u, 0u},
    {0xf6e3u, 3u, 6u, 2u},
    {0xf6e6u, 3u, 6u, 2u},
    {0xf6e9u, 2u, 3u, 0u},
    {0xf6f6u, 3u, 7u, 2u},
    {0xf6f9u, 2u, 3u, 0u},
};
static const MemberChangePlan MemberPlan6_sum = {
    MemberChange6_sum, sizeof MemberChange6_sum / sizeof MemberChange6_sum[0], 0xf6fbu, 3u
};

static const FixedSpanOpcode MemberChange6_floor[] = {
    {0xf6d7u, 2u, 3u, 0u},
    {0xf6d9u, 1u, 2u, 0u},
    {0xf6dau, 2u, 3u, 0u},
    {0xf6dcu, 3u, 6u, 2u},
    {0xf7bdu, 1u, 3u, 1u},
    {0xf7beu, 2u, 3u, 0u},
    {0xf7c0u, 1u, 2u, 0u},
    {0xf7c1u, 1u, 2u, 0u},
    {0xf7c2u, 1u, 2u, 0u},
    {0xf7c3u, 4u, 6u, 2u},
    {0xf7c7u, 1u, 2u, 0u},
    {0xf7c8u, 1u, 4u, 1u},
    {0xf7c9u, 1u, 6u, 2u},
    {0xf6dfu, 2u, 3u, 0u},
    {0xf6e1u, 1u, 2u, 0u},
    {0xf6e2u, 1u, 2u, 0u},
    {0xf6e3u, 3u, 6u, 2u},
    {0xf6e6u, 3u, 6u, 2u},
    {0xf6e9u, 2u, 2u, 0u},
    {0xf6ebu, 1u, 2u, 0u},
    {0xf6ecu, 3u, 6u, 2u},
    {0xf6efu, 3u, 6u, 2u},
    {0xf6f2u, 1u, 2u, 0u},
    {0xf6f3u, 3u, 6u, 2u},
    {0xf6f6u, 3u, 7u, 2u},
    {0xf6f9u, 2u, 3u, 0u},
};
static const MemberChangePlan MemberPlan6_floor = {
    MemberChange6_floor, sizeof MemberChange6_floor / sizeof MemberChange6_floor[0], 0xf6fbu, 3u
};

static const FixedSpanOpcode MemberChange7_zero[] = {
    {0xf6fcu, 2u, 3u, 0u},
    {0xf6feu, 1u, 2u, 0u},
    {0xf6ffu, 2u, 3u, 0u},
    {0xf701u, 3u, 6u, 2u},
    {0xf7bdu, 1u, 3u, 1u},
    {0xf7beu, 2u, 3u, 0u},
    {0xf7c0u, 1u, 2u, 0u},
    {0xf7c1u, 1u, 2u, 0u},
    {0xf7c2u, 1u, 2u, 0u},
    {0xf7c3u, 4u, 6u, 2u},
    {0xf7c7u, 1u, 2u, 0u},
    {0xf7c8u, 1u, 4u, 1u},
    {0xf7c9u, 1u, 6u, 2u},
    {0xf704u, 2u, 3u, 0u},
    {0xf706u, 1u, 2u, 0u},
    {0xf707u, 1u, 2u, 0u},
    {0xf708u, 3u, 6u, 2u},
    {0xf70bu, 2u, 3u, 0u},
    {0xf71au, 3u, 6u, 2u},
    {0xf71du, 3u, 7u, 2u},
    {0xf720u, 2u, 3u, 0u},
};
static const MemberChangePlan MemberPlan7_zero = {
    MemberChange7_zero, sizeof MemberChange7_zero / sizeof MemberChange7_zero[0], 0xf722u, 3u
};

static const FixedSpanOpcode MemberChange7_borrow[] = {
    {0xf6fcu, 2u, 3u, 0u},
    {0xf6feu, 1u, 2u, 0u},
    {0xf6ffu, 2u, 3u, 0u},
    {0xf701u, 3u, 6u, 2u},
    {0xf7bdu, 1u, 3u, 1u},
    {0xf7beu, 2u, 3u, 0u},
    {0xf7c0u, 1u, 2u, 0u},
    {0xf7c1u, 1u, 2u, 0u},
    {0xf7c2u, 1u, 2u, 0u},
    {0xf7c3u, 4u, 6u, 2u},
    {0xf7c7u, 1u, 2u, 0u},
    {0xf7c8u, 1u, 4u, 1u},
    {0xf7c9u, 1u, 6u, 2u},
    {0xf704u, 2u, 3u, 0u},
    {0xf706u, 1u, 2u, 0u},
    {0xf707u, 1u, 2u, 0u},
    {0xf708u, 3u, 6u, 2u},
    {0xf70bu, 2u, 2u, 0u},
    {0xf70du, 3u, 5u, 2u},
    {0xf710u, 1u, 2u, 0u},
    {0xf711u, 3u, 6u, 2u},
    {0xf714u, 3u, 7u, 2u},
    {0xf717u, 2u, 3u, 0u},
};
static const MemberChangePlan MemberPlan7_borrow = {
    MemberChange7_borrow, sizeof MemberChange7_borrow / sizeof MemberChange7_borrow[0], 0xf719u, 2u
};
