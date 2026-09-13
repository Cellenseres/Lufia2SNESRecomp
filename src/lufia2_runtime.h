#pragma once

#include <stddef.h>
#include <stdint.h>

#include "common_cpu_infra.h"
#include "snesrecomp_platform/snes_ppu_capture.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SaveLoadInfo;

extern const RtlGameInfo kLufia2GameInfo;

void Lufia2RunOneFrame(void);
void Lufia2DrawPpuFrame(void);
void Lufia2PrintDiagnostics(void);
void Lufia2SaveExecutionState(struct SaveLoadInfo *sli);
void Lufia2LoadExecutionState(struct SaveLoadInfo *sli, uint32_t version);
void Lufia2ApplyExecutionState(uint32_t version);

enum { LUFIA2_PPU_VISIBLE_LINES = 224 };

/* Authoritative raster state captured immediately before each native visible
 * scanline is drawn. Row y maps to PPU line y+1. */
const uint8_t *Lufia2LineRegisters(unsigned y);
bool Lufia2PpuRasterHistory(const uint8_t **rows, size_t *stride);
uint32_t Lufia2PpuRasterMemoryFlags(void);

/* Guest boundary at which the next frame resumes. This is observation only;
 * it does not redirect or otherwise alter guest execution. */
uint32_t Lufia2ResumePc(void);

bool Lufia2CapturePpuFrame(SnesPpuFrameCapture *out,
                           unsigned canvas_width,
                           unsigned canvas_extra);

#ifdef __cplusplus
}
#endif
