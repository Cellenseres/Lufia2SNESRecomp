#pragma once
#include "snesrecomp_platform/snes_ppu_job.h"

typedef struct Lufia2IntroMarginJob Lufia2IntroMarginJob;

/* Allocate bounded caches once. Asset ownership stays with the consumer. */
Lufia2IntroMarginJob *Lufia2IntroMarginJobCreate(uint16_t max_margin,uint16_t max_height);
void Lufia2IntroMarginJobDestroy(Lufia2IntroMarginJob *context);

/* Exact original fade/alpha, with no destination reads or guest side effects.
 * The caller supplies the actual unscaled top-left PPU pixel as backdrop.
 * Images are borrowed until the next successful prepare or destruction; freeze
 * them into the common owned job before changing this context again. Missing
 * art, zero margin or zero brightness produces zero images/rectangles. Failure
 * preserves previous views and all outputs. Invalid premul stays on the CPU. */
SnesRecompPpuResult Lufia2IntroMarginJobPrepare(Lufia2IntroMarginJob *context,
    const uint32_t *art,uint32_t pitch,uint16_t width,uint16_t height,
    uint16_t margin,unsigned brightness,uint32_t backdrop,
    SnesRecompPpuJobImage images[2],SnesRecompPpuOverlay rectangles[2],uint16_t *count);
/* Consumer facade: borrow the existing RAM asset and resolve the real logical
 * top-left pixel from a captured Mode-1 frame, including qualified sprites.
 * The caller supplies the original whole-art fade and margin policy. Other
 * rendering effects fail cleanly for ordinary CPU postprocessing. */
SnesRecompPpuResult Lufia2IntroMarginJobPrepareFrame(Lufia2IntroMarginJob *context,
    const SnesRecompPpuFrame *frame,uint16_t margin,unsigned brightness,
    SnesRecompPpuJobImage images[2],SnesRecompPpuOverlay rectangles[2],uint16_t *count);
