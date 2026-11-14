/* Only this original STZ/DEX/BNE slice owns a quiet clock certificate.
 * Ordinary FixedSpanClock consumers retain their original handler and layout. */
typedef struct EffectClearClock {
    FixedSpanClock span;
    Lufia2DecompClockSpan clocks;
    unsigned code_speed;
} EffectClearClock;

/* DEX/BNE have no data/progress access. Keep all original instruction
 * fetches and relative APU flush boundaries, but publish the two eventless
 * host clock updates once when the first cannot expose an APU boundary. */
static bool JoinEffectClearBranch(EffectClearClock *effect) {
    FixedSpanClock *clock = &effect->span;
    if (!clock->opcode_span || clock->progress || clock->data_master ||
        clock->index + 1u >= clock->count)
        return false;
    const FixedSpanOpcode *dex = clock->timing + clock->index;
    const FixedSpanOpcode *branch = dex + 1;
    if (dex->pc != 0x8bfeu || branch->pc != 0x8bffu || dex->fetched != 1u || dex->cycles != 2u ||
        dex->data_bytes || branch->fetched != 2u || branch->data_bytes ||
        (branch->cycles != 2u && branch->cycles != 3u))
        return false;
    const uint64_t first = clock->fetch_master + 6u;
    const uint64_t second = 2u * effect->code_speed + (branch->cycles - 2u) * 6u;
    if (!Lufia2DecompClockSpanCanJoin(clock->cpu, &effect->clocks, first, first + second))
        return false;
    ++clock->index;
    clock->fetch_master = 0u;
    FetchFixedSpanOpcode(clock);
    assert(!clock->remaining && !clock->progress && !clock->data_master);
    const uint64_t actual = first + clock->fetch_master + (branch->cycles - 2u) * 6u;
    assert(actual == first + second);
    Lufia2DecompAccountClockSpan(clock->cpu, &effect->clocks, dex->cycles + branch->cycles, actual,
                                 false);
    ++clock->index;
    clock->fetch_master = 0u;
    return true;
}

static void FinishEffectClearOpcode(EffectClearClock *effect) {
    FixedSpanClock *clock = &effect->span;
    const FixedSpanOpcode *timing = clock->timing + clock->index;
    const unsigned internal = timing->cycles - timing->fetched - timing->data_bytes;
    if (clock->progress && clock->index < 32u)
        clock->progress_mask |= 1u << clock->index;
    Lufia2DecompAccountClockSpan(clock->cpu, &effect->clocks, timing->cycles,
                                 clock->fetch_master + clock->data_master + internal * 6u,
                                 clock->progress);
    ++clock->index;
    clock->data_master = clock->fetch_master = 0u;
    clock->progress = false;
}
static void BeginEffectClearOpcode(EffectClearClock *effect) {
    FixedSpanClock *clock = &effect->span;
    while (!clock->remaining && clock->index < clock->count) {
        FetchFixedSpanOpcode(clock);
        if (!clock->remaining && !JoinEffectClearBranch(effect))
            FinishEffectClearOpcode(effect);
    }
}
/* Unexpected semantic accesses discard the optional certificate before using
 * the generic bus/clock adapter. No already published clock is deferred. */
static uint8_t EffectClearRead(void *context, uint32_t address) {
    EffectClearClock *effect = context;
    effect->clocks.remaining = 0;
    return FixedSpanRead(&effect->span, address);
}
static void EffectClearWrite(void *context, uint32_t address, uint8_t value) {
    EffectClearClock *effect = context;
    FixedSpanClock *clock = &effect->span;
    if (address != 0x972180u || value) {
        effect->clocks.remaining = 0;
        FixedSpanWrite(clock, address, value);
        return;
    }
    BeginEffectClearOpcode(effect);
    assert(clock->remaining);
    bool progress = false;
    Lufia2DecompFrameReturnWriteByte(clock->cpu, 0x972180u, 0u, &progress);
    clock->data_master += cpu_region_speed(0x972180u);
    clock->progress |= progress;
    if (!--clock->remaining)
        FinishEffectClearOpcode(effect);
}
