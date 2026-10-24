#pragma once
#include "lufia2_intro_trees.h"
#include "snesrecomp_platform/snes_ppu_job.h"

typedef struct Lufia2IntroTreeJob Lufia2IntroTreeJob;
enum { LUFIA2_INTRO_TREE_JOB_IMAGES = 6, LUFIA2_INTRO_TREE_JOB_RECTS = 7 };
typedef struct Lufia2IntroTreeJobStats {
    uint32_t decoded_columns, reused_columns, decoded_pixels;
} Lufia2IntroTreeJobStats;

/* A bounded ring of complete columns, split into six persistent texture strips.
 * Scroll changes only rectangles. An arriving column invalidates its strip
 * instead of forcing a complete tree image upload. */
Lufia2IntroTreeJob *Lufia2IntroTreeJobCreate(uint16_t max_height);
void Lufia2IntroTreeJobDestroy(Lufia2IntroTreeJob *context);
/* Source VRAM is owned by the immutable frame. Palette and history decisions
 * are already resolved by the consumer. No guest, sprite or history changes.
 * Views are borrowed until the next successful prepare or destruction.
 * Failure preserves the previous views and all output parameters. */
SnesRecompPpuResult Lufia2IntroTreeJobPrepare(Lufia2IntroTreeJob *context,
    const SnesRecompPpuFrame *frame, const Lufia2IntroTreeSnapshot *snapshot,
    SnesRecompPpuJobImage images[LUFIA2_INTRO_TREE_JOB_IMAGES],
    SnesRecompPpuOverlay rectangles[LUFIA2_INTRO_TREE_JOB_RECTS], uint16_t *count);
void Lufia2IntroTreeJobGetStats(const Lufia2IntroTreeJob *context,
    Lufia2IntroTreeJobStats *stats);
