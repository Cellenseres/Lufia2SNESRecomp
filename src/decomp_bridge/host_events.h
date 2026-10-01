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

typedef enum Lufia2DecompGameFileOperation {
    LUFIA2_DECOMP_GAME_FILE_LOAD,
    LUFIA2_DECOMP_GAME_FILE_SAVE,
    LUFIA2_DECOMP_GAME_FILE_PREVIEW,
    LUFIA2_DECOMP_GAME_FILE_LOAD_HEADER
} Lufia2DecompGameFileOperation;

/* File index in A at 9099/90C9/914B, before their first opcode.
 * LOAD_HEADER identifies the nested 914B call from 9099, not a preview. */
typedef void (*Lufia2DecompGameFileEvent)(
    CpuState *, uint32_t, Lufia2DecompGameFileOperation);
void Lufia2DecompSetGameFileEvent(Lufia2DecompGameFileEvent callback);
void Lufia2DecompGameFile(CpuState *cpu, uint32_t pc);

/* Song ID in A before STA $54 at $80:942E. Return zero to continue,
 * or a guest JSL target. The event runs again at the same PC on return,
 * so the subscriber can restore A after a volume command. */
typedef uint32_t (*Lufia2DecompSongLoadEvent)(CpuState *, uint32_t);
void Lufia2DecompSetSongLoadEvent(Lufia2DecompSongLoadEvent callback);
uint32_t Lufia2DecompSongLoad(CpuState *cpu, uint32_t pc);

/* Before PHP at $80:9692. */
typedef void (*Lufia2DecompMusicFadeOutEvent)(CpuState *, uint32_t);
void Lufia2DecompSetMusicFadeOutEvent(Lufia2DecompMusicFadeOutEvent callback);
void Lufia2DecompMusicFadeOut(CpuState *cpu, uint32_t pc);

#endif
