#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "snes/ppu.h"

/* The guest holds only the visible 256 pixels of the tree line.
   Observe keeps passing columns, Paint draws the wide frame. */
void Lufia2IntroTreesReset(void);
/* line_scroll is what the frame was drawn with; the register is not. */
void Lufia2IntroTreesObserve(
    const Ppu *ppu, unsigned layer, const uint16_t *line_scroll, size_t lines);
bool Lufia2IntroTreesPaint(
    const Ppu *ppu,
    unsigned layer,
    uint8_t *frame,
    size_t width,
    size_t height,
    size_t margin_width,
    const uint16_t *line_scroll,
    size_t lines);
