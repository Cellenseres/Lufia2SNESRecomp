#ifndef LUFIA2_FRAME_DECISION_CLOCK_H
#define LUFIA2_FRAME_DECISION_CLOCK_H

#include <stdbool.h>
#include <stdint.h>

struct CpuState;

#ifdef __cplusplus
extern "C" {
#endif

bool Lufia2DecompFrameDecisionHookOwned(uint32_t pc,
    void (*hook)(struct CpuState *, uint32_t));

#ifdef __cplusplus
}
#endif

#endif
