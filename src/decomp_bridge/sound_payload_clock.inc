#include "sound_clock_barrier.h"

typedef struct SoundPayloadClock {
    FixedSpanClock *span;
#if defined(LUFIA2_HAVE_SOUND_CLOCK_BARRIERS)
    Lufia2SoundClockBarrier barrier;
#endif
    bool enabled;
} SoundPayloadClock;

static bool SoundPayloadClockPrepare(SoundPayloadClock *clock, unsigned budget) {
#if defined(LUFIA2_HAVE_SOUND_CLOCK_BARRIERS)
    clock->enabled = Lufia2DecompSoundClockBegin(
        clock->span->cpu, budget, &clock->barrier);
#else
    (void)budget;
    clock->enabled = false;
#endif
    return clock->enabled;
}

static void SoundPayloadClockClose(SoundPayloadClock *clock) {
#if defined(LUFIA2_HAVE_SOUND_CLOCK_BARRIERS)
    if (clock->enabled)
        Lufia2DecompSoundClockEnd(&clock->barrier);
#else
    (void)clock;
#endif
}

#if defined(LUFIA2_HAVE_SOUND_CLOCK_BARRIERS)
static void SoundPayloadClockFinish(SoundPayloadClock *clock) {
    FixedSpanClock *span = clock->span;
    const FixedSpanOpcode *timing = span->timing + span->index;
    const unsigned internal = timing->cycles - timing->fetched - timing->data_bytes;
    if (span->progress && span->index < 32u)
        span->progress_mask |= 1u << span->index;
    Lufia2DecompSoundClockAccount(&clock->barrier, timing->cycles,
        span->fetch_master + span->data_master + internal * 6u, span->progress);
    ++span->index;
    span->data_master = 0u;
    span->fetch_master = 0u;
    span->progress = false;
}

static bool SoundPayloadClockPureMemory(uint32_t address) {
    const uint8_t bank = (uint8_t)(address >> 16);
    const uint16_t offset = (uint16_t)address;
    return bank == 0x7eu || bank == 0x7fu ||
        ((bank & 0x7fu) < 0x40u && (offset < 0x2000u || offset >= 0x8000u));
}

static void SoundPayloadClockBeforeAccess(SoundPayloadClock *clock,
    uint32_t address) {
    if (SoundPayloadClockPureMemory(address))
        return;
    const uint8_t bank = (uint8_t)(address >> 16);
    const uint16_t offset = (uint16_t)address;
    if ((bank & 0x7fu) < 0x40u && offset >= 0x2140u && offset <= 0x2143u)
        Lufia2DecompSoundClockBarrier(&clock->barrier);
    else
        Lufia2DecompSoundClockEnd(&clock->barrier);
}

static void SoundPayloadClockFinishAccess(SoundPayloadClock *clock,
    uint32_t address, bool progress) {
    FixedSpanClock *span = clock->span;
    span->data_master += cpu_region_speed(address);
    span->progress |= progress;
    if (!--span->remaining)
        SoundPayloadClockFinish(clock);
}
#endif

static void SoundPayloadClockBegin(SoundPayloadClock *clock) {
#if defined(LUFIA2_HAVE_SOUND_CLOCK_BARRIERS)
    if (clock->enabled) {
        FixedSpanClock *span = clock->span;
        while (!span->remaining && span->index < span->count) {
            FetchFixedSpanOpcode(span);
            if (!span->remaining)
                SoundPayloadClockFinish(clock);
        }
        return;
    }
#endif
    BeginFixedSpanOpcode(clock->span);
}

static uint8_t SoundPayloadClockRead(void *context, uint32_t address) {
    SoundPayloadClock *clock = context;
#if defined(LUFIA2_HAVE_SOUND_CLOCK_BARRIERS)
    if (clock->enabled) {
        SoundPayloadClockBegin(clock);
        assert(clock->span->remaining);
        SoundPayloadClockBeforeAccess(clock, address);
        bool progress = false;
        const uint8_t value = Lufia2DecompFrameReturnReadByte(
            clock->span->cpu, address, &progress);
        SoundPayloadClockFinishAccess(clock, address, progress);
        return value;
    }
#endif
    return FixedSpanRead(clock->span, address);
}

static void SoundPayloadClockWrite(void *context, uint32_t address, uint8_t value) {
    SoundPayloadClock *clock = context;
#if defined(LUFIA2_HAVE_SOUND_CLOCK_BARRIERS)
    if (clock->enabled) {
        SoundPayloadClockBegin(clock);
        assert(clock->span->remaining);
        SoundPayloadClockBeforeAccess(clock, address);
        bool progress = false;
        Lufia2DecompFrameReturnWriteByte(clock->span->cpu, address, value, &progress);
        SoundPayloadClockFinishAccess(clock, address, progress);
        return;
    }
#endif
    FixedSpanWrite(clock->span, address, value);
}
