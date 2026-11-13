enum { PAIRED_SPAN_MAX_OPCODES = 32u };

typedef struct PairedSpanClock {
    FixedSpanClock span;
    const FixedSpanOpcode *plan;
    unsigned count;
    unsigned offset;
    uint32_t entry;
} PairedSpanClock;

static void PreparePairedSpan(PairedSpanClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index < clock->span.count || !clock->span.count)
            return;
        Lufia2DecompFramePairedAccountSpan(clock->span.cpu, clock->entry,
            clock->span.count, clock->span.progress_mask);
        clock->offset += clock->span.count;
        const unsigned remaining = clock->count - clock->offset;
        clock->span = (FixedSpanClock){.cpu = clock->span.cpu, .bank = clock->entry & 0xff0000u,
            .timing = clock->plan + clock->offset,
            .count = remaining < PAIRED_SPAN_MAX_OPCODES ? remaining : PAIRED_SPAN_MAX_OPCODES};
    }
}

static uint8_t PairedSpanRead(void *context, uint32_t address) {
    PairedSpanClock *clock = context;
    PreparePairedSpan(clock);
    return FixedSpanRead(&clock->span, address);
}

static void PairedSpanWrite(void *context, uint32_t address, uint8_t value) {
    PairedSpanClock *clock = context;
    PreparePairedSpan(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static PairedSpanClock BeginPairedSpanClock(CpuState *cpu, uint32_t entry,
    const FixedSpanOpcode *plan, unsigned count) {
    return (PairedSpanClock){.span = {.cpu = cpu, .bank = entry & 0xff0000u,
        .timing = plan, .count = count < PAIRED_SPAN_MAX_OPCODES ? count : PAIRED_SPAN_MAX_OPCODES},
        .plan = plan, .count = count, .entry = entry};
}
