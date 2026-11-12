#ifndef LUFIA2_APU_CLOCK_STATE_H
#define LUFIA2_APU_CLOCK_STATE_H

#include <stdbool.h>

struct SaveLoadInfo;

void Lufia2ApuClockSaveState(struct SaveLoadInfo *sli);
bool Lufia2ApuClockLoadState(struct SaveLoadInfo *sli);
void Lufia2ApuClockApplyLoadedState(void);

#endif
