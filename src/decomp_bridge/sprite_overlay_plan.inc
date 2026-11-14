enum { SPRITE_OVERLAY_MAX_COUNT = 6u };

typedef struct SpriteOverlayPlan {
    unsigned count;
    unsigned opcode_count;
} SpriteOverlayPlan;

static const FixedSpanOpcode sprite_overlay_setup[] = {
    {0xb680u, 3u, 4u, 1u}, {0xb683u, 2u, 2u, 0u},
    {0xb685u, 2u, 3u, 0u}, {0xb687u, 2u, 2u, 0u},
    {0xb689u, 2u, 3u, 1u}
};
static const FixedSpanOpcode sprite_overlay_assign[] = {
    {0xb68bu, 3u, 5u, 1u}, {0xb68eu, 1u, 2u, 0u},
    {0xb68fu, 3u, 4u, 1u}, {0xb692u, 1u, 2u, 0u},
    {0xb693u, 3u, 4u, 1u}, {0xb696u, 2u, 3u, 0u}
};
static const FixedSpanOpcode sprite_overlay_visit_setup[] = {
    {0xb698u, 2u, 3u, 0u}, {0xb69au, 3u, 4u, 1u},
    {0xb69du, 2u, 3u, 0u}, {0xb6a0u, 2u, 3u, 1u},
    {0xb6a2u, 3u, 3u, 0u}
};
static const FixedSpanOpcode sprite_overlay_disabled[] = {
    {0xb6a5u, 3u, 4u, 1u}, {0xb6a8u, 2u, 3u, 0u},
    {0xb6f6u, 1u, 2u, 0u}, {0xb6f7u, 3u, 3u, 0u},
    {0xb6fau, 2u, 5u, 2u}, {0xb6fcu, 2u, 2u, 0u},
    {0xb6feu, 2u, 3u, 0u}
};

static bool PlanDisabledSpriteOverlays(const uint8_t *ram, SpriteOverlayPlan *plan) {
    const unsigned count = ram[0x154eu];
    if (count > SPRITE_OVERLAY_MAX_COUNT)
        return false;
    for (unsigned overlay = 0u; overlay < count; ++overlay)
        if (ram[0x1534u + overlay])
            return false;
    *plan = (SpriteOverlayPlan){count, count ? 9u + 13u * count : 2u};
    return true;
}

static FixedSpanOpcode SpriteOverlayOpcode(const SpriteOverlayPlan *plan, unsigned index) {
    assert(index < plan->opcode_count);
    if (!plan->count) {
        FixedSpanOpcode opcode = sprite_overlay_setup[index];
        if (index == 1u)
            ++opcode.cycles;
        return opcode;
    }
    if (index < 5u)
        return sprite_overlay_setup[index];
    index -= 5u;
    if (index < 6u * plan->count) {
        FixedSpanOpcode opcode = sprite_overlay_assign[index % 6u];
        if (index + 1u == 6u * plan->count)
            --opcode.cycles;
        return opcode;
    }
    index -= 6u * plan->count;
    if (index < 5u)
        return sprite_overlay_visit_setup[index];
    index -= 5u;
    FixedSpanOpcode opcode = sprite_overlay_disabled[index % 7u];
    if (index + 2u == 7u * plan->count)
        ++opcode.cycles;
    return opcode;
}
