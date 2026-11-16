#ifndef LUFIA2_BATTLE_TARGET_NAME_CLOCK_H
#define LUFIA2_BATTLE_TARGET_NAME_CLOCK_H

#include <stdbool.h>
#include <stdint.h>

#define LUFIA2_TARGET_NAME_MAX_OPCODES 24u
#define LUFIA2_TARGET_NAME_SPAN_BYTES 6u

struct CpuState;

#ifdef __cplusplus
extern "C" {
#endif

bool Lufia2DecompTargetNameHookOwned(uint32_t pc,
    void (*hook)(struct CpuState *, uint32_t));

#ifdef __cplusplus
}
#endif

#endif
