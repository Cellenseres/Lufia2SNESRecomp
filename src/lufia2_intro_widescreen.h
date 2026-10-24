#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "snes/ppu.h"

/* Widens the intro scenes while widescreen is on.
   LUFIA2_INTRO_WIDE=off, =on (waves only), =trees overrides.
   Per frame: Prepare, draw, Observe, Paint. */

/* False leaves the layout to the caller. */
bool Lufia2IntroWidescreenPrepare(
    Ppu *ppu, uint8_t runtime_map, unsigned margin);

/* After the draw. */
void Lufia2IntroWidescreenObserve(const Ppu *ppu);
/* A GPU consumer may omit Observe/Paint only when neither has pending work. */
bool Lufia2IntroWidescreenNeedsPostprocessing(void);
typedef struct Lufia2IntroPostState {
    bool active, trees_active;
    const uint16_t *tree_scroll;
    size_t tree_lines;
} Lufia2IntroPostState;
/* Read-only consumer decisions after Observe, valid until the next frame. */
Lufia2IntroPostState Lufia2IntroWidescreenGetPostState(void);

void Lufia2IntroWidescreenPaint(
    const Ppu *ppu, uint8_t *frame, size_t width, size_t height,
    unsigned margin);

/* Gives back what Prepare borrowed. Idempotent. */
void Lufia2IntroWidescreenRelease(Ppu *ppu);
