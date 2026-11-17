#ifndef LUFIA2_DECOMP_FIELD_REGION_OWNER_H
#define LUFIA2_DECOMP_FIELD_REGION_OWNER_H

#include <stdbool.h>
#include <stdint.h>
#include "cpu_state.h"
#include "lufia2/execution.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline bool Lufia2DecompFieldRegionEntry(uint32_t pc) {
    switch (pc) {
    case 0x838265u: case 0x838268u: case 0x83826bu:
    case 0x83827au: case 0x83827du: case 0x83827eu:
    case 0x838281u: case 0x838285u: case 0x838289u:
    case 0x838292u: case 0x83829bu:
        return true;
    default:
        return false;
    }
}

bool Lufia2DecompFieldRegionHasBudget(const CpuState *cpu, unsigned count);
void Lufia2DecompFieldRegionAccountPrefix(
    const CpuState *cpu, unsigned count, unsigned progress_mask);
uint8_t Lufia2DecompFieldRegionInvokeChild(CpuState *cpu, void *context,
    Lufia2CpuState *state, uint32_t target, uint32_t site, uint8_t frame_size,
    Lufia2PushedChildCall child);

bool Lufia2DecompFieldRegionCanResume(CpuState *cpu, uint32_t entry);
RecompReturn Lufia2DecompFieldRegionResume(
    CpuState *cpu, uint32_t entry, uint32_t *next_pc);

#ifdef __cplusplus
}
#endif
#endif
