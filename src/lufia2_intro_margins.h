#pragma once

#include <stddef.h>
#include <stdint.h>

/* Hand-authored margin art, behind everything the PPU drew.
   assets/img/lufia2_intro_margins.tga, the size of the wide frame;
   its centre columns are ignored. brightness is INIDISP, 0..15. */
void Lufia2IntroMarginsApply(
    uint8_t *frame, size_t width, size_t height, size_t margin_width,
    unsigned brightness);

/* Borrow the same RAM-resident art used by Apply. Loaded at most once; NULL
 * means missing art or a mismatched canvas. The caller must not modify/free it.
 * Native preparation preserves the original source alpha through this view. */
const uint32_t *Lufia2IntroMarginsBorrowArt(size_t width, size_t height);
