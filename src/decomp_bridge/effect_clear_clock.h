#ifndef LUFIA2_EFFECT_CLEAR_CLOCK_H
#define LUFIA2_EFFECT_CLEAR_CLOCK_H

#include <stdbool.h>
#include <stdint.h>

struct CpuState;

enum { LUFIA2_EFFECT_CLEAR_STEP_OPCODES = 12u };

#ifdef __cplusplus
extern "C" {
#endif

bool Lufia2DecompEffectClearHookOwned(uint32_t pc,
    void (*hook)(struct CpuState *, uint32_t));

#ifdef __cplusplus
}
#endif
#endif
