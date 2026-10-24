#ifndef LUFIA2_MENU_RESUME_H
#define LUFIA2_MENU_RESUME_H

#include "equipment_resume.h"

static inline uint32_t Lufia2MenuContinuationOwner(uint32_t landing) {
    return landing == 0x84c268u ? 0x84c243u : Lufia2EquipmentContinuationOwner(landing);
}

uint32_t Lufia2DecompMenuResumeReturn(CpuState *cpu, uint32_t landing);
RecompReturn Lufia2DecompMenuResume(CpuState *cpu, uint32_t landing);

#endif
