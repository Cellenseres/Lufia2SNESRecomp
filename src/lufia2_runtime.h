#pragma once

#include "common_cpu_infra.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const RtlGameInfo kLufia2GameInfo;

void Lufia2RunOneFrame(void);
void Lufia2DrawPpuFrame(void);
void Lufia2PrintDiagnostics(void);

#ifdef __cplusplus
}
#endif
