#ifndef LUFIA2_DECOMP_WORLD_FRAME_OWNER_H
#define LUFIA2_DECOMP_WORLD_FRAME_OWNER_H

#include <stdbool.h>
#include <stdint.h>
#include "cpu_state.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline bool Lufia2DecompWorldFrameEntry(uint32_t pc) {
#if defined(LUFIA2_HAVE_NATIVE_WORLD_FRAME_TAIL)
    switch (pc) {
    case 0x86925cu:
    case 0x869276u: case 0x869279u: case 0x86927cu:
    case 0x86927fu: case 0x869282u: case 0x869285u:
    case 0x869290u: case 0x869293u: case 0x869296u:
    case 0x869299u: case 0x86929cu:
        return true;
    default:
        return false;
    }
#else
    return pc == 0x86925cu;
#endif
}

RecompReturn Lufia2DecompWorldFrameResumeFrom(
    CpuState *cpu, uint32_t entry, uint32_t *next_pc);
bool Lufia2DecompWorldFrameCanResumeFrom(CpuState *cpu, uint32_t entry);
bool Lufia2DecompWorldFrameCanResume(CpuState *cpu);
RecompReturn Lufia2DecompWorldFrameResume(CpuState *cpu, uint32_t *next_pc);

#ifdef __cplusplus
}
#endif

#endif
