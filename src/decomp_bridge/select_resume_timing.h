#ifndef LUFIA2_SELECT_RESUME_TIMING_H
#define LUFIA2_SELECT_RESUME_TIMING_H

#include <stdint.h>

typedef struct Lufia2SelectResumeOpcode {
    unsigned cycles;
    unsigned fetched;
    unsigned data_bytes;
    unsigned internal;
} Lufia2SelectResumeOpcode;

typedef struct Lufia2SelectResumeSchedule {
    unsigned count;
    Lufia2SelectResumeOpcode opcodes[4];
} Lufia2SelectResumeSchedule;

static inline Lufia2SelectResumeSchedule Lufia2SelectResumeScheduleFor(uint32_t entry,
                                                                       uint32_t next) {
    Lufia2SelectResumeSchedule schedule = {0};
    if (entry == 0x868333u) {
        schedule.count = 2u;
        schedule.opcodes[0] = (Lufia2SelectResumeOpcode){4u, 3u, 1u, 0u};
        schedule.opcodes[1] = next == 0x868338u
                                  ? (Lufia2SelectResumeOpcode){2u, 2u, 0u, 0u}
                                  : (Lufia2SelectResumeOpcode){3u, 2u, 0u, 1u};
    } else if (entry == 0x868352u) {
        schedule.count = 4u;
        schedule.opcodes[0] = (Lufia2SelectResumeOpcode){5u, 1u, 2u, 2u};
        schedule.opcodes[1] = (Lufia2SelectResumeOpcode){2u, 1u, 0u, 1u};
        if (next == 0x868357u) {
            schedule.opcodes[2] = (Lufia2SelectResumeOpcode){2u, 2u, 0u, 0u};
            schedule.opcodes[3] = (Lufia2SelectResumeOpcode){2u, 1u, 0u, 1u};
        } else {
            schedule.opcodes[2] = (Lufia2SelectResumeOpcode){3u, 2u, 0u, 1u};
            schedule.opcodes[3] = (Lufia2SelectResumeOpcode){4u, 1u, 2u, 1u};
        }
    } else if (entry == 0x8682fcu) {
        schedule.count = 1u;
        schedule.opcodes[0] = (Lufia2SelectResumeOpcode){3u, 2u, 0u, 1u};
    }
    return schedule;
}

static inline unsigned Lufia2SelectResumeMasterClocks(Lufia2SelectResumeOpcode opcode,
                                                      unsigned fetch_speed) {
    return opcode.fetched * fetch_speed + opcode.data_bytes * 8u + opcode.internal * 6u;
}

#endif
