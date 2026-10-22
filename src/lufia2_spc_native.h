#ifndef LUFIA2_SPC_NATIVE_H
#define LUFIA2_SPC_NATIVE_H

#include <stdbool.h>

struct Apu;
struct SaveLoadInfo;
struct SnesRecompApuProgram;

/* Decompiled sound driver replaces the interpreted SPC-700 program. */
/* False when LUFIA2_SPC_INTERPRETED=1 keeps the interpreter. */
bool Lufia2SpcNativeInstall(void);

/* For hosts that register the program themselves. */
const struct SnesRecompApuProgram *Lufia2SpcNativeProgram(void);

/* True once the native driver owns the SPC. */
bool Lufia2SpcNativeActive(void);

/* Driver progress for savestates and rollback snapshots. */
void Lufia2SpcNativeSaveState(struct SaveLoadInfo *sli, const struct Apu *apu);
/* False leaves the post-load fallback in place. */
bool Lufia2SpcNativeLoadState(struct SaveLoadInfo *sli, struct Apu *apu);

#endif
