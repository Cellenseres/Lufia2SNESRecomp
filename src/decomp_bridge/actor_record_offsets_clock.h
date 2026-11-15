#ifndef LUFIA2_ACTOR_RECORD_OFFSETS_CLOCK_H
#define LUFIA2_ACTOR_RECORD_OFFSETS_CLOCK_H

#include <stdbool.h>
#include <stdint.h>

struct CpuState;

#ifdef __cplusplus
extern "C" {
#endif

bool Lufia2DecompActorRecordOffsetsHookOwned(uint32_t pc,
    void (*hook)(struct CpuState *, uint32_t));

#ifdef __cplusplus
}
#endif

#endif
