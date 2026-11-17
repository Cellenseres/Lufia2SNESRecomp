#ifndef LUFIA2_BATTLE_TARGET_MARKER_CLOCK_H
#define LUFIA2_BATTLE_TARGET_MARKER_CLOCK_H

#include <stdbool.h>
#include <stdint.h>

struct CpuState;

enum {
    LUFIA2_TARGET_MARKER_MAX_RECORDS = 3u,
    LUFIA2_TARGET_MARKER_MAX_OPCODES = 10u * LUFIA2_TARGET_MARKER_MAX_RECORDS
};

#ifdef __cplusplus
extern "C" {
#endif

bool Lufia2DecompTargetMarkerHookOwned(uint32_t pc,
    void (*hook)(struct CpuState *, uint32_t));

#ifdef __cplusplus
}
#endif

#endif
