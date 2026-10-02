#pragma once

#include <stdbool.h>

struct SaveLoadInfo;

/* MSU-1 music, swapped in on the decomp song load. */
void Lufia2MsuDriverInstall(void);

/* Once per frame, after the guest ran. */
void Lufia2MsuDriverFrame(void);

bool Lufia2MsuDriverPlaying(void);

void Lufia2MsuSaveState(struct SaveLoadInfo *sli);
bool Lufia2MsuLoadState(struct SaveLoadInfo *sli);
void Lufia2MsuApplyLoadedState(void);
