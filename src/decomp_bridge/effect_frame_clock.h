#ifndef LUFIA2_EFFECT_FRAME_CLOCK_H
#define LUFIA2_EFFECT_FRAME_CLOCK_H

#include <stdint.h>

struct CpuState;

#ifdef __cplusplus
extern "C" {
#endif

uint32_t Lufia2DecompEffectFrameHookEnd(uint32_t pc,
    void (*hook)(struct CpuState *, uint32_t));

#ifdef __cplusplus
}
#endif

#endif
