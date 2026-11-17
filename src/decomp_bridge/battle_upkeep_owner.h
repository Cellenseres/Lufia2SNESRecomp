#ifndef LUFIA2_DECOMP_BATTLE_UPKEEP_OWNER_H
#define LUFIA2_DECOMP_BATTLE_UPKEEP_OWNER_H

#include "native_call_owner.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline bool Lufia2DecompBattleUpkeepEntry(uint32_t pc) {
    return pc == 0x85ec81u || pc == 0x85ec8bu;
}

bool Lufia2DecompBattleUpkeepCanResume(CpuState *cpu, uint32_t entry);
RecompReturn Lufia2DecompBattleUpkeepResume(
    CpuState *cpu, uint32_t entry, uint32_t *next_pc);

#ifdef __cplusplus
}
#endif
#endif
