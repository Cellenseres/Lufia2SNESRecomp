#ifndef LUFIA2_DECOMP_WORLD_FRAME_OWNER_H
#define LUFIA2_DECOMP_WORLD_FRAME_OWNER_H

#include <stdbool.h>
#include <stdint.h>
#include "cpu_state.h"

#ifdef __cplusplus
extern "C" {
#endif

bool Lufia2DecompWorldFrameCanResume(CpuState *cpu);
RecompReturn Lufia2DecompWorldFrameResume(CpuState *cpu, uint32_t *next_pc);

#ifdef __cplusplus
}
#endif

#endif
