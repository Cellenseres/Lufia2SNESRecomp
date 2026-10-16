#ifndef LUFIA2_FRAME_RETURN_TIMING_H
#define LUFIA2_FRAME_RETURN_TIMING_H

#include <stdint.h>

typedef struct Lufia2FrameReturnSchedule {
    unsigned count;
    unsigned pulled_bytes[3];
} Lufia2FrameReturnSchedule;

static inline Lufia2FrameReturnSchedule
Lufia2FrameReturnScheduleFor(uint32_t entry, uint8_t saved_status,
                             unsigned accumulator_is_8_bit) {
    Lufia2FrameReturnSchedule schedule = {0};
    switch (entry) {
    case 0x8293ccu:
    case 0x868b52u:
        schedule.count = 2u;
        schedule.pulled_bytes[0] = 1u;
        schedule.pulled_bytes[1] = saved_status & 0x20u ? 1u : 2u;
        break;
    case 0x869756u:
        schedule.count = 1u;
        schedule.pulled_bytes[0] = accumulator_is_8_bit ? 1u : 2u;
        break;
    case 0x868b6fu:
        schedule.count = 3u;
        schedule.pulled_bytes[0] = 1u;
        schedule.pulled_bytes[1] = saved_status & 0x10u ? 1u : 2u;
        schedule.pulled_bytes[2] = schedule.pulled_bytes[1];
        break;
    }
    return schedule;
}

static inline unsigned Lufia2FrameReturnCpuCycles(unsigned pulled_bytes) {
    return 3u + pulled_bytes;
}

static inline unsigned Lufia2FrameReturnMasterClocks(unsigned pulled_bytes,
                                                     unsigned fetch_speed) {
    return fetch_speed + pulled_bytes * 8u + 12u;
}

#endif
