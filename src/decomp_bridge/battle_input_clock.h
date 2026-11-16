#ifndef LUFIA2_BATTLE_INPUT_CLOCK_H
#define LUFIA2_BATTLE_INPUT_CLOCK_H

#include <stdbool.h>
#include <stdint.h>

struct CpuState;

#ifdef __cplusplus
extern "C" {
#endif

bool Lufia2DecompBattleInputHookOwned(uint32_t pc,
    void (*hook)(struct CpuState *, uint32_t));

#ifdef __cplusplus
}
#endif

#endif
