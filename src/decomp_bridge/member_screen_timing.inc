typedef struct MemberTimingSegment {
    uint16_t pc;
    unsigned count;
    const FixedSpanOpcode *timing;
} MemberTimingSegment;

static const FixedSpanOpcode member_timing_9e67[] = {
    {0x9e67u, 3u, 4u, 1u},
    {0x9e6au, 3u, 4u, 1u},
    {0x9e6du, 3u, 3u, 0u},
    {0x9e70u, 4u, 8u, 3u},
};

static const FixedSpanOpcode member_timing_9e74[] = {
    {0x9e74u, 2u, 3u, 0u},
    {0x9e76u, 3u, 3u, 0u},
    {0x9e79u, 3u, 3u, 0u},
    {0x9e7cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_9e7f[] = {
    {0x9e7fu, 3u, 3u, 0u},
    {0x9e82u, 3u, 3u, 0u},
    {0x9e85u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_9e88[] = {
    {0x9e88u, 2u, 3u, 0u},
    {0x9e8au, 3u, 4u, 1u},
    {0x9e8du, 2u, 3u, 1u},
    {0x9e8fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_9e92[] = {
    {0x9e92u, 2u, 2u, 0u},
    {0x9e94u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_9e97[] = {
    {0x9e97u, 3u, 3u, 0u},
    {0x9e9au, 3u, 5u, 2u},
    {0x9e9du, 3u, 4u, 1u},
    {0x9ea0u, 3u, 6u, 1u},
    {0x9ea3u, 3u, 4u, 1u},
    {0x9ea6u, 3u, 6u, 1u},
    {0x9ea9u, 3u, 4u, 1u},
    {0x9eacu, 3u, 6u, 1u},
    {0x9eafu, 3u, 4u, 1u},
    {0x9eb2u, 3u, 6u, 1u},
    {0x9eb5u, 3u, 4u, 1u},
    {0x9eb8u, 3u, 6u, 1u},
    {0x9ebbu, 3u, 4u, 1u},
    {0x9ebeu, 3u, 6u, 1u},
    {0x9ec1u, 3u, 4u, 1u},
    {0x9ec4u, 3u, 6u, 1u},
    {0x9ec7u, 2u, 2u, 0u},
    {0x9ec9u, 3u, 6u, 1u},
    {0x9eccu, 3u, 6u, 1u},
    {0x9ecfu, 3u, 4u, 1u},
    {0x9ed2u, 4u, 8u, 3u},
};

static const FixedSpanOpcode member_timing_9ed6[] = {
    {0x9ed6u, 3u, 3u, 0u},
    {0x9ed9u, 2u, 4u, 2u},
    {0x9edbu, 3u, 3u, 0u},
    {0x9edeu, 2u, 4u, 2u},
    {0x9ee0u, 3u, 3u, 0u},
    {0x9ee3u, 3u, 3u, 0u},
    {0x9ee6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_9ee9[] = {
    {0x9ee9u, 2u, 2u, 0u},
    {0x9eebu, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_9eee[] = {
    {0x9eeeu, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b2fe[] = {
    {0xb2feu, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b301[] = {
    {0xb301u, 2u, 2u, 0u},
    {0xb303u, 3u, 4u, 1u},
    {0xb306u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b30b[] = {
    {0xb30bu, 4u, 8u, 3u},
};

static const FixedSpanOpcode member_timing_b30f[] = {
    {0xb30fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b312[] = {
    {0xb312u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b315[] = {
    {0xb315u, 2u, 3u, 0u},
};

static const FixedSpanOpcode member_timing_b317[] = {
    {0xb317u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b31a[] = {
    {0xb31au, 2u, 2u, 0u},
};

static const FixedSpanOpcode member_timing_b31e[] = {
    {0xb31eu, 2u, 2u, 0u},
};

static const FixedSpanOpcode member_timing_b322[] = {
    {0xb322u, 2u, 2u, 0u},
};

static const FixedSpanOpcode member_timing_b326[] = {
    {0xb326u, 2u, 2u, 0u},
};

static const FixedSpanOpcode member_timing_b32a[] = {
    {0xb32au, 2u, 2u, 0u},
    {0xb32cu, 2u, 3u, 0u},
};

static const FixedSpanOpcode member_timing_b32e[] = {
    {0xb32eu, 2u, 2u, 0u},
    {0xb330u, 2u, 3u, 0u},
};

static const FixedSpanOpcode member_timing_b332[] = {
    {0xb332u, 4u, 8u, 3u},
};

static const FixedSpanOpcode member_timing_b336[] = {
    {0xb336u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b339[] = {
    {0xb339u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b33d[] = {
    {0xb33du, 2u, 2u, 0u},
    {0xb33fu, 4u, 8u, 3u},
};

static const FixedSpanOpcode member_timing_b343[] = {
    {0xb343u, 2u, 2u, 0u},
    {0xb345u, 3u, 3u, 0u},
    {0xb348u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b34b[] = {
    {0xb34bu, 4u, 8u, 3u},
};

static const FixedSpanOpcode member_timing_b34f[] = {
    {0xb34fu, 3u, 4u, 1u},
};

static const FixedSpanOpcode member_timing_b354[] = {
    {0xb354u, 2u, 2u, 0u},
    {0xb356u, 3u, 3u, 0u},
    {0xb359u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b35c[] = {
    {0xb35cu, 2u, 2u, 0u},
    {0xb35eu, 2u, 2u, 0u},
    {0xb360u, 1u, 2u, 0u},
    {0xb361u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b35e[] = {
    {0xb35eu, 2u, 2u, 0u},
    {0xb360u, 1u, 2u, 0u},
    {0xb361u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b364[] = {
    {0xb364u, 2u, 2u, 0u},
    {0xb366u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b369[] = {
    {0xb369u, 2u, 2u, 0u},
    {0xb36bu, 2u, 3u, 1u},
    {0xb36du, 3u, 3u, 0u},
};

static const FixedSpanOpcode member_timing_b370[] = {
    {0xb370u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b373[] = {
    {0xb373u, 2u, 2u, 0u},
    {0xb375u, 3u, 4u, 1u},
    {0xb378u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b37d[] = {
    {0xb37du, 2u, 3u, 0u},
    {0xb37fu, 3u, 5u, 2u},
    {0xb382u, 1u, 2u, 0u},
    {0xb383u, 1u, 2u, 0u},
    {0xb384u, 3u, 5u, 2u},
    {0xb387u, 1u, 2u, 0u},
    {0xb388u, 2u, 3u, 0u},
    {0xb38au, 3u, 5u, 1u},
    {0xb38du, 2u, 2u, 0u},
};

static const FixedSpanOpcode member_timing_b391[] = {
    {0xb391u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b394[] = {
    {0xb394u, 2u, 3u, 0u},
};

static const FixedSpanOpcode member_timing_b396[] = {
    {0xb396u, 3u, 5u, 2u},
    {0xb399u, 3u, 5u, 2u},
    {0xb39cu, 4u, 5u, 1u},
    {0xb3a0u, 3u, 4u, 1u},
    {0xb3a3u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b3a6[] = {
    {0xb3a6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode member_timing_b3a9[] = {
    {0xb3a9u, 3u, 6u, 2u},
};

static const MemberTimingSegment member_timing_segments[] = {
    {0x9e67u, 4u, member_timing_9e67},
    {0x9e74u, 4u, member_timing_9e74},
    {0x9e7fu, 3u, member_timing_9e7f},
    {0x9e88u, 4u, member_timing_9e88},
    {0x9e92u, 2u, member_timing_9e92},
    {0x9e97u, 21u, member_timing_9e97},
    {0x9ed6u, 7u, member_timing_9ed6},
    {0x9ee9u, 2u, member_timing_9ee9},
    {0x9eeeu, 1u, member_timing_9eee},
    {0xb2feu, 1u, member_timing_b2fe},
    {0xb301u, 3u, member_timing_b301},
    {0xb30bu, 1u, member_timing_b30b},
    {0xb30fu, 1u, member_timing_b30f},
    {0xb312u, 1u, member_timing_b312},
    {0xb315u, 1u, member_timing_b315},
    {0xb317u, 1u, member_timing_b317},
    {0xb31au, 1u, member_timing_b31a},
    {0xb31eu, 1u, member_timing_b31e},
    {0xb322u, 1u, member_timing_b322},
    {0xb326u, 1u, member_timing_b326},
    {0xb32au, 2u, member_timing_b32a},
    {0xb32eu, 2u, member_timing_b32e},
    {0xb332u, 1u, member_timing_b332},
    {0xb336u, 1u, member_timing_b336},
    {0xb339u, 1u, member_timing_b339},
    {0xb33du, 2u, member_timing_b33d},
    {0xb343u, 3u, member_timing_b343},
    {0xb34bu, 1u, member_timing_b34b},
    {0xb34fu, 1u, member_timing_b34f},
    {0xb354u, 3u, member_timing_b354},
    {0xb35cu, 4u, member_timing_b35c},
    {0xb35eu, 3u, member_timing_b35e},
    {0xb364u, 2u, member_timing_b364},
    {0xb369u, 3u, member_timing_b369},
    {0xb370u, 1u, member_timing_b370},
    {0xb373u, 3u, member_timing_b373},
    {0xb37du, 9u, member_timing_b37d},
    {0xb391u, 1u, member_timing_b391},
    {0xb394u, 1u, member_timing_b394},
    {0xb396u, 5u, member_timing_b396},
    {0xb3a6u, 1u, member_timing_b3a6},
    {0xb3a9u, 1u, member_timing_b3a9},
};
