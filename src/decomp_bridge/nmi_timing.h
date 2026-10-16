#ifndef LUFIA2_NMI_TIMING_H
#define LUFIA2_NMI_TIMING_H

#include <stdint.h>

typedef struct Lufia2NmiCost {
    unsigned fetch;
    unsigned ram;
    unsigned io;
    unsigned internal;
} Lufia2NmiCost;

static inline unsigned Lufia2NmiCpuCycles(Lufia2NmiCost cost) {
    return cost.fetch + cost.ram + cost.io + cost.internal;
}

static inline unsigned Lufia2NmiMasterClocks(Lufia2NmiCost cost, unsigned fetch_speed) {
    return cost.fetch * fetch_speed + cost.ram * 8u + (cost.io + cost.internal) * 6u;
}

typedef struct Lufia2NmiSchedule {
    Lufia2NmiCost steps[32];
    unsigned count;
} Lufia2NmiSchedule;

static inline void Lufia2NmiAppend(Lufia2NmiSchedule *schedule, unsigned fetch,
                                   unsigned ram, unsigned io, unsigned internal) {
    schedule->steps[schedule->count++] = (Lufia2NmiCost){fetch, ram, io, internal};
}

static inline Lufia2NmiSchedule Lufia2NmiPrepareSchedule(const uint8_t *ram) {
    Lufia2NmiSchedule schedule = {0};
    Lufia2NmiAppend(&schedule, 2, 0, 0, 1);
    Lufia2NmiAppend(&schedule, 1, 1, 0, 1);
    Lufia2NmiAppend(&schedule, 1, 1, 0, 2);
    Lufia2NmiAppend(&schedule, 2, 0, 0, 0);
    Lufia2NmiAppend(&schedule, 3, 0, 1, 0);
    Lufia2NmiAppend(&schedule, 3, 1, 0, 0);
    Lufia2NmiAppend(&schedule, 2, 0, 0, ram[0x81] ? 0 : 1);
    if (ram[0x81])
        Lufia2NmiAppend(&schedule, 3, 0, 1, 0);
    return schedule;
}

static inline Lufia2NmiSchedule Lufia2NmiCallbacksSchedule(const uint8_t *ram,
                                                           uint32_t entry) {
    static const unsigned flags[] = {0x6a, 0x6f, 0x71};
    unsigned first = entry == 0x80864c ? 0 : entry == 0x808653 ? 1 : 2;
    Lufia2NmiSchedule schedule = {0};
    for (unsigned i = first; i < 3; ++i) {
        Lufia2NmiAppend(&schedule, 2, 1, 0, 0);
        Lufia2NmiAppend(&schedule, 2, 0, 0, ram[flags[i]] ? 0 : 1);
        if (ram[flags[i]])
            break;
    }
    return schedule;
}

static inline Lufia2NmiSchedule Lufia2NmiClockSchedule(const uint8_t *ram) {
    Lufia2NmiSchedule schedule = {0};
    Lufia2NmiAppend(&schedule, 3, 1, 0, 0);
    Lufia2NmiAppend(&schedule, 3, 0, 1, 0);
    Lufia2NmiAppend(&schedule, 3, 1, 0, 0);
    Lufia2NmiAppend(&schedule, 2, 0, 0, (ram[0xb50] & 0x80) ? 1 : 0);
    if (ram[0xb50] & 0x80)
        return schedule;
    for (unsigned address = 0xb50; address >= 0xb4e; --address) {
        unsigned advance = (uint8_t)(ram[address] + 1) >= 60;
        Lufia2NmiAppend(&schedule, 1, 0, 0, 1);
        Lufia2NmiAppend(&schedule, 3, 1, 0, 0);
        Lufia2NmiAppend(&schedule, 2, 0, 0, 0);
        Lufia2NmiAppend(&schedule, 2, 0, 0, advance ? 0 : 1);
        if (!advance)
            break;
        Lufia2NmiAppend(&schedule, 3, 1, 0, 0);
        Lufia2NmiAppend(&schedule, 3, address == 0xb4e ? 2 : 1, 0,
                        address == 0xb4e ? 1 : 0);
    }
    return schedule;
}

static inline Lufia2NmiSchedule Lufia2NmiRestoreSchedule(const uint8_t *ram) {
    Lufia2NmiSchedule schedule = {0};

    Lufia2NmiAppend(&schedule, 2, 4, 0, 1);
    Lufia2NmiAppend(&schedule, 2, 4, 0, 1);
    Lufia2NmiAppend(&schedule, 4, 2, 0, 0);
    Lufia2NmiAppend(&schedule, 3, 0, 0, 0);
    Lufia2NmiAppend(&schedule, 2, 0, 0, ram[0x520] ? 0 : 1);
    if (ram[0x520]) {
        unsigned reset = ram[0x46] == 0x30 && ram[0x47] == 0x30;
        Lufia2NmiAppend(&schedule, 2, 2, 0, 0);
        Lufia2NmiAppend(&schedule, 3, 0, 0, 0);
        Lufia2NmiAppend(&schedule, 2, 0, 0, 1);
        Lufia2NmiAppend(&schedule, 2, 0, 0, reset ? 0 : 1);
        if (reset)
            return schedule;
    }
    Lufia2NmiAppend(&schedule, 3, 0, ram[0x520] ? 1 : 2, 0);
    Lufia2NmiAppend(&schedule, 2, 0, 0, 1);
    for (unsigned i = 0; i < 3; ++i)
        Lufia2NmiAppend(&schedule, 1, 2, 0, 2);
    for (unsigned i = 0; i < 2; ++i)
        Lufia2NmiAppend(&schedule, 1, 1, 0, 2);
    return schedule;
}

static inline Lufia2NmiSchedule Lufia2NmiEntrySchedule(void) {
    Lufia2NmiSchedule schedule = {0};
    for (unsigned saved_byte = 0; saved_byte < 2; ++saved_byte)
        Lufia2NmiAppend(&schedule, 1, 1, 0, 1);
    Lufia2NmiAppend(&schedule, 2, 0, 0, 1);
    for (unsigned saved_word = 0; saved_word < 3; ++saved_word)
        Lufia2NmiAppend(&schedule, 1, 2, 0, 1);
    Lufia2NmiAppend(&schedule, 4, 0, 0, 0);
    return schedule;
}

#endif
