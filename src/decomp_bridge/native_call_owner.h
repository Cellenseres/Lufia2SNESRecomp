#ifndef LUFIA2_DECOMP_NATIVE_CALL_OWNER_H
#define LUFIA2_DECOMP_NATIVE_CALL_OWNER_H

#include <stdbool.h>
#include <stdint.h>
#include "cpu_state.h"
#include "lufia2/execution.h"

#ifdef __cplusplus
extern "C" {
#endif

bool Lufia2DecompCallOwnerHasBudget(const CpuState *cpu, unsigned count);
void Lufia2DecompCallOwnerAccountPrefix(
    const CpuState *cpu, unsigned count, unsigned progress_mask);
uint8_t Lufia2DecompCallOwnerInvokeChild(CpuState *cpu, void *context,
    Lufia2CpuState *state, uint32_t target, uint32_t site, uint8_t frame_size,
    Lufia2PushedChildCall child);

#ifdef __cplusplus
}
#endif
#endif
