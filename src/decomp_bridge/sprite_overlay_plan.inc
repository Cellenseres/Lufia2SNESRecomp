enum { SPRITE_OVERLAY_MAX_COUNT = 6u };

typedef struct SpriteOverlayPlan {
    unsigned count;
    unsigned opcode_count;
    unsigned cycles;
    uint8_t sprite_counts[SPRITE_OVERLAY_MAX_COUNT];
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

static const FixedSpanOpcode sprite_overlay_active_setup[] = {
    {0xb6aau, 3u, 4u, 1u},
    {0xb6adu, 2u, 3u, 1u},
    {0xb6afu, 3u, 4u, 1u},
    {0xb6b2u, 2u, 3u, 1u},
    {0xb6b4u, 3u, 4u, 1u},
    {0xb6b7u, 2u, 3u, 1u},
    {0xb6b9u, 3u, 4u, 1u},
    {0xb6bcu, 1u, 4u, 2u},
    {0xb6bdu, 2u, 3u, 0u},
    {0xb6bfu, 3u, 3u, 0u},
    {0xb6c2u, 1u, 2u, 0u},
    {0xb6c3u, 1u, 2u, 0u},
    {0xb6c4u, 1u, 2u, 0u},
    {0xb6c5u, 3u, 5u, 2u},
    {0xb6c8u, 3u, 3u, 0u},
    {0xb6cbu, 2u, 4u, 2u},
    {0xb6cdu, 1u, 2u, 0u},
    {0xb6ceu, 1u, 2u, 0u},
    {0xb6cfu, 2u, 4u, 2u},
    {0xb6d1u, 1u, 2u, 0u},
    {0xb6d2u, 2u, 3u, 0u},
};

static const FixedSpanOpcode sprite_overlay_active_sprite[] = {
    {0xb6d4u, 2u, 3u, 1u},
    {0xb6d6u, 1u, 2u, 0u},
    {0xb6d7u, 4u, 5u, 1u},
    {0xb6dbu, 3u, 6u, 1u},
    {0xb6deu, 1u, 2u, 0u},
    {0xb6dfu, 1u, 2u, 0u},
    {0xb6e0u, 2u, 3u, 1u},
    {0xb6e2u, 1u, 2u, 0u},
    {0xb6e3u, 4u, 5u, 1u},
    {0xb6e7u, 3u, 6u, 1u},
    {0xb6eau, 1u, 2u, 0u},
    {0xb6ebu, 1u, 2u, 0u},
    {0xb6ecu, 1u, 2u, 0u},
    {0xb6edu, 1u, 2u, 0u},
    {0xb6eeu, 1u, 2u, 0u},
    {0xb6efu, 1u, 2u, 0u},
    {0xb6f0u, 1u, 2u, 0u},
    {0xb6f1u, 2u, 5u, 2u},
    {0xb6f3u, 2u, 2u, 0u},
};

static bool PlanSpriteOverlays(const uint8_t *ram, SpriteOverlayPlan *plan) {
    const unsigned count = ram[0x154eu];
    if (count > SPRITE_OVERLAY_MAX_COUNT)
        return false;
    *plan = (SpriteOverlayPlan){.count = count,
        .opcode_count = count ? 9u + 13u * count : 2u,
        .cycles = count ? 27u + 42u * count : 7u};
    for (unsigned overlay = 0u; overlay < count; ++overlay) {
        if (!ram[0x1534u + overlay])
            continue;
        const unsigned sprites = ram[0x157fu + overlay];
        if (!sprites || sprites > 128u)
            return false;
        plan->sprite_counts[overlay] = (uint8_t)sprites;
        plan->opcode_count += 22u + 19u * sprites;
        plan->cycles += 69u + 58u * sprites;
    }
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
    if (plan->opcode_count == 9u + 13u * plan->count) {
        FixedSpanOpcode opcode = sprite_overlay_disabled[index % 7u];
        if (index + 2u == 7u * plan->count)
            ++opcode.cycles;
        return opcode;
    }
    for (unsigned overlay = 0u; overlay < plan->count; ++overlay) {
        const unsigned sprites = plan->sprite_counts[overlay];
        const unsigned active_length = sprites ? 22u + 19u * sprites : 0u;
        const unsigned length = 7u + active_length - (overlay + 1u == plan->count);
        if (index >= length) {
            index -= length;
            continue;
        }
        if (index < 2u) {
            FixedSpanOpcode opcode = sprite_overlay_disabled[index];
            if (sprites && index == 1u)
                --opcode.cycles;
            return opcode;
        }
        index -= 2u;
        if (sprites) {
            if (index < 21u)
                return sprite_overlay_active_setup[index];
            index -= 21u;
            if (index < 19u * sprites) {
                FixedSpanOpcode opcode = sprite_overlay_active_sprite[index % 19u];
                if (index % 19u == 18u && index + 1u < 19u * sprites)
                    ++opcode.cycles;
                return opcode;
            }
            index -= 19u * sprites;
            if (!index)
                return (FixedSpanOpcode){0xb6f5u, 1u, 5u, 2u};
            --index;
        }
        FixedSpanOpcode opcode = sprite_overlay_disabled[index + 2u];
        if (overlay + 1u == plan->count && index == 3u)
            ++opcode.cycles;
        return opcode;
    }
    assert(false);
    return (FixedSpanOpcode){0};
}
