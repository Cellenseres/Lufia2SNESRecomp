#ifndef LUFIA2_MENU_RESUME_H
#define LUFIA2_MENU_RESUME_H

#include "equipment_resume.h"

static inline uint32_t Lufia2CardAnimationContinuationOwner(uint32_t landing) {
    switch (landing) {
    case 0x84b161u:
        return 0x84b13du;
    case 0x84b241u: case 0x84b259u: case 0x84b25cu:
    case 0x84b25fu: case 0x84b26eu: case 0x84b271u:
        return 0x84b22bu;
    default:
        return 0u;
    }
}

static inline uint32_t Lufia2MenuContinuationOwner(uint32_t landing) {
    const uint32_t card = Lufia2CardAnimationContinuationOwner(landing);
    if (card)
        return card;
    return landing == 0x84c268u ? 0x84c243u : Lufia2EquipmentContinuationOwner(landing);
}

uint32_t Lufia2DecompMenuResumeReturn(CpuState *cpu, uint32_t landing);
RecompReturn Lufia2DecompMenuResume(CpuState *cpu, uint32_t landing);

#endif
