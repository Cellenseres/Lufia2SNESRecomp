#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "snesrecomp_platform/snes_ppu_capture.h"
#include "snesrecomp_platform/snes_ppu_mode7.h"

/* The world-map camera at full guest precision. M7X/M7Y and BG1HOFS/BG1VOFS
 * are integer registers and the matrix is built from an integer table index,
 * so none of this reaches the PPU. */
typedef struct Lufia2Mode7Sample {
    uint16_t center_x; /* $11F8, uploaded to M7X */
    uint16_t center_y; /* $11FA, uploaded to M7Y */
    uint8_t frac_x;    /* $11EC, 1/256 pixel */
    uint8_t frac_y;    /* $11EF */
    uint16_t angle;    /* $1200, 8.8 steps of 1/256 turn */
} Lufia2Mode7Sample;

/* LUFIA2_MODE7_SUBSTEP=0/off restores the captured geometry. */
bool Lufia2Mode7SubstepEnabled(void);

/* Rewrites `lines` in place at the guest's precision, reading WRAM itself.
 * False leaves them untouched. */
bool Lufia2Mode7SubstepRefine(const SnesPpuFrameCapture *capture,
                              SnesRecompMode7Line *lines,
                              unsigned line_count);

bool Lufia2Mode7SubstepRefineWithSample(const SnesPpuFrameCapture *capture,
                                        const Lufia2Mode7Sample *sample,
                                        SnesRecompMode7Line *lines,
                                        unsigned line_count);

int Lufia2Mode7SubstepSelfTest(void);
