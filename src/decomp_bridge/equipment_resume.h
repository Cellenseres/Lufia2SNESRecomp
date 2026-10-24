#ifndef LUFIA2_EQUIPMENT_RESUME_H
#define LUFIA2_EQUIPMENT_RESUME_H

#include "cpu_state.h"

static inline uint32_t Lufia2EquipmentContinuationOwner(uint32_t landing) {
    switch (landing) {
    case 0x82b910u:
    case 0x82b919u:
        return 0x82b907u;
    case 0x82bd1au:
    case 0x82bd26u:
    case 0x82bd2fu:
    case 0x82bd38u:
    case 0x82bd3bu:
        return 0x82bd02u;
    default:
        return 0u;
    }
}

uint32_t Lufia2DecompEquipmentResumeReturn(CpuState *cpu, uint32_t landing);
RecompReturn Lufia2DecompEquipmentResume(CpuState *cpu, uint32_t landing);

#endif
