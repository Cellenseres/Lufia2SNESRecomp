#pragma once
#include "snesrecomp_platform/snes_ppu_obj_run.h"
struct Ppu;
/* Caller-owned bounded recording, bound only to the primary timeline PPU.
 * Failure invalidates the capture without altering selection, pixels or flags.
 * Reset/bind never selects OBJ and never reads guest memory. */
typedef struct Lufia2PpuObjCapture {
    SnesRecompPpuObjRun *runs;
    uint32_t capacity,count;
    uint32_t lines[8];
    uint32_t completed_lines[8];
    uint32_t line_first[256],line_end[256];
    bool valid;
    const struct Ppu *ppu;
} Lufia2PpuObjCapture;
extern Lufia2PpuObjCapture *lufia2_ppu_obj_capture;
void Lufia2PpuObjCaptureBind(Lufia2PpuObjCapture *capture,const struct Ppu *ppu);
void Lufia2PpuObjCaptureLine(const struct Ppu *ppu,int line);
void Lufia2PpuObjCaptureEndLine(const struct Ppu *ppu,int line,bool unrecorded_margin);
void Lufia2PpuObjCaptureRun(const struct Ppu *ppu,int line,int x,int tile,int row,
    int palette,int priority,int flip,unsigned slot,int begin,int end,
    int overlay_begin,int overlay_end,unsigned flags,unsigned owner);
