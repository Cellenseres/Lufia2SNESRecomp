#ifndef LUFIA2_EFFECT_DISPATCH_CLOCK_H
#define LUFIA2_EFFECT_DISPATCH_CLOCK_H

#include <stdbool.h>
#include <stdint.h>

struct CpuState;

#ifdef __cplusplus
extern "C" {
#endif

bool Lufia2DecompEffectDispatchHookOwned(uint32_t pc,
    void (*hook)(struct CpuState *, uint32_t));

#ifdef __cplusplus
}
#endif

#endif
