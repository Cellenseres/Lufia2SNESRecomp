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
enum { LUFIA2_INTRO_TREE_MAX_COLUMNS = 65, LUFIA2_INTRO_TREE_ROWS = 32 };
/* Resolved consumer data, after exactly one Observe. Unknown historical
 * columns remain transparent. This neither advances scroll history nor reads
 * a rendered destination; graphics consumers receive the final column choices. */
typedef struct Lufia2IntroTreeSnapshot {
    int32_t first_column;
    uint16_t width, column_count, fine_x, chars, scroll_y;
    uint32_t palette[32];
    uint16_t entries[LUFIA2_INTRO_TREE_MAX_COLUMNS][LUFIA2_INTRO_TREE_ROWS];
    uint8_t known[LUFIA2_INTRO_TREE_MAX_COLUMNS];
} Lufia2IntroTreeSnapshot;
bool Lufia2IntroTreesCapture(const Ppu *ppu, unsigned layer, size_t width,
    size_t margin_width, const uint16_t *line_scroll, size_t lines,
    Lufia2IntroTreeSnapshot *out);
/* A synchronous palette view; original rounding is checked before admission. */
bool Lufia2IntroTreesCaptureUnshaded(const Ppu *ppu, unsigned layer, size_t width,
    size_t margin_width, const uint16_t *line_scroll, size_t lines,
    Lufia2IntroTreeSnapshot *out, uint8_t *brightness_reduction);
bool Lufia2IntroTreesPaint(
    const Ppu *ppu,
    unsigned layer,
    uint8_t *frame,
    size_t width,
    size_t height,
    size_t margin_width,
    const uint16_t *line_scroll,
    size_t lines);
