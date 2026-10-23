#include <assert.h>

typedef struct FixedSpanOpcode {
    uint16_t pc;
    uint8_t fetched;
    uint8_t cycles;
    uint8_t data_bytes;
} FixedSpanOpcode;

typedef struct FixedSpanClock {
    CpuState *cpu;
    uint32_t bank;
    const FixedSpanOpcode *timing;
    unsigned count;
    unsigned index;
    unsigned remaining;
    unsigned progress_mask;
    uint64_t data_master;
    uint64_t fetch_master;
    bool progress;
} FixedSpanClock;

static void FinishFixedSpanOpcode(FixedSpanClock *clock) {
    const FixedSpanOpcode *timing = clock->timing + clock->index;
    const unsigned internal = timing->cycles - timing->fetched - timing->data_bytes;
    if (clock->progress)
        clock->progress_mask |= 1u << clock->index;
    Lufia2DecompFrameReturnAccountOpcode(clock->cpu, timing->cycles,
        clock->fetch_master + clock->data_master + internal * 6u, clock->progress);
    ++clock->index;
    clock->data_master = 0u;
    clock->fetch_master = 0u;
    clock->progress = false;
}

static void BeginFixedSpanOpcode(FixedSpanClock *clock) {
    while (!clock->remaining && clock->index < clock->count) {
        const FixedSpanOpcode *timing = clock->timing + clock->index;
        for (unsigned byte = 0u; byte < timing->fetched; ++byte) {
            bool progress = false;
            const uint32_t address = clock->bank | (uint16_t)(timing->pc + byte);
            (void)Lufia2DecompFrameReturnReadByte(clock->cpu, address, &progress);
            clock->fetch_master += cpu_region_speed(address);
            clock->progress |= progress;
        }
        clock->remaining = timing->data_bytes;
        if (!clock->remaining)
            FinishFixedSpanOpcode(clock);
    }
}

static uint8_t FixedSpanRead(void *context, uint32_t address) {
    FixedSpanClock *clock = context;
    BeginFixedSpanOpcode(clock);
    assert(clock->remaining);
    bool progress = false;
    const uint8_t value =
        Lufia2DecompFrameReturnReadByte(clock->cpu, address, &progress);
    clock->data_master += cpu_region_speed(address);
    clock->progress |= progress;
    if (!--clock->remaining)
        FinishFixedSpanOpcode(clock);
    return value;
}

static void FixedSpanWrite(void *context, uint32_t address, uint8_t value) {
    FixedSpanClock *clock = context;
    BeginFixedSpanOpcode(clock);
    assert(clock->remaining);
    bool progress = false;
    Lufia2DecompFrameReturnWriteByte(clock->cpu, address, value, &progress);
    clock->data_master += cpu_region_speed(address);
    clock->progress |= progress;
    if (!--clock->remaining)
        FinishFixedSpanOpcode(clock);
}
