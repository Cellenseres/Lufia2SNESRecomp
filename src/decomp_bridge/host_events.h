#ifndef LUFIA2_DECOMP_HOST_EVENTS_H
#define LUFIA2_DECOMP_HOST_EVENTS_H

#include <stdint.h>

typedef struct CpuState CpuState;

/* Before LDY #$C37D at $82:A3B4 or $82:A3F0; no subscriber is a no-op. */
typedef void (*Lufia2DecompEquipmentListDrawEvent)(CpuState *, uint32_t);
void Lufia2DecompSetEquipmentListDrawEvent(
    Lufia2DecompEquipmentListDrawEvent callback);
void Lufia2DecompEquipmentListDraw(CpuState *cpu, uint32_t pc);

/* After the original clock tick and before REP #$20 at $80:8699. */
typedef void (*Lufia2DecompPlayTimeTickEvent)(CpuState *, uint32_t);
void Lufia2DecompSetPlayTimeTickEvent(Lufia2DecompPlayTimeTickEvent callback);
void Lufia2DecompPlayTimeTick(CpuState *cpu, uint32_t pc);

/* Resolved value pointer and digit start, before PHY at $80:8922. */
typedef void (*Lufia2DecompMenuNumberEvent)(CpuState *, uint32_t);
void Lufia2DecompSetMenuNumberEvent(Lufia2DecompMenuNumberEvent callback);
void Lufia2DecompMenuNumber(CpuState *cpu, uint32_t pc);

/* Map ID in A at $83:B548; commit at $83:B580 only after a real load. */
typedef void (*Lufia2DecompMapLoadEvent)(CpuState *, uint32_t);
void Lufia2DecompSetMapLoadBeginEvent(Lufia2DecompMapLoadEvent callback);
void Lufia2DecompSetMapLoadCommittedEvent(Lufia2DecompMapLoadEvent callback);
void Lufia2DecompMapLoadBegin(CpuState *cpu, uint32_t pc);
void Lufia2DecompMapLoadCommitted(CpuState *cpu, uint32_t pc);

#endif
