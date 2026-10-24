#pragma once
#include "lufia2_intro_widescreen.h"
#include "snesrecomp_platform/snes_ppu_job.h"

typedef struct Lufia2IntroPpuJob Lufia2IntroPpuJob;
enum { LUFIA2_INTRO_JOB_IMAGES = 8, LUFIA2_INTRO_JOB_RECTS = 9 };
Lufia2IntroPpuJob *Lufia2IntroPpuJobCreate(void);
void Lufia2IntroPpuJobDestroy(Lufia2IntroPpuJob *context);
/* Prepare after the authoritative Observe and capture. Exact painting then
 * trees, as on the CPU. No destination readback or duplicate guest/observer
 * work. Freeze the borrowed views before another call. Unsupported effects,
 * capacity or invalid art decline for ordinary CPU postprocessing. */
SnesRecompPpuResult Lufia2IntroPpuJobPrepare(Lufia2IntroPpuJob *context,
    const Ppu *ppu, const SnesRecompPpuFrame *frame, uint16_t margin,
    SnesRecompPpuJobImage images[LUFIA2_INTRO_JOB_IMAGES], uint16_t *image_count,
    SnesRecompPpuOverlay rectangles[LUFIA2_INTRO_JOB_RECTS], uint16_t *rectangle_count);
/* Requires POST_RGB5_BRIGHTNESS. Tree images retain full-brightness RGB5,
 * with exact dimming in the overlay material; authored margins keep their
 * existing conditional, pre-shaded pixels. Failure selects ordinary fallback. */
SnesRecompPpuResult Lufia2IntroPpuJobPrepareUnshadedTrees(Lufia2IntroPpuJob *context,
    const Ppu *ppu, const SnesRecompPpuFrame *frame, uint16_t margin,
    SnesRecompPpuJobImage images[LUFIA2_INTRO_JOB_IMAGES], uint16_t *image_count,
    SnesRecompPpuOverlay rectangles[LUFIA2_INTRO_JOB_RECTS], uint16_t *rectangle_count);
