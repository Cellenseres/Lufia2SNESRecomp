#pragma once

#include <stddef.h>
#include <stdint.h>

#include "snesrecomp_platform/snes_ppu_mode7.h"

typedef enum Lufia2IntroMode7WorldStatus {
    LUFIA2_INTRO_WORLD_READY = 0,
    LUFIA2_INTRO_WORLD_NO_ROM,
    LUFIA2_INTRO_WORLD_INVALID_LAYOUT,
    LUFIA2_INTRO_WORLD_RING_MISMATCH,
    LUFIA2_INTRO_WORLD_INVALID_ARGUMENT,
} Lufia2IntroMode7WorldStatus;

/* Keeps a read-only reference to the already verified, headerless US ROM.
 * The ROM remains owned by the desktop host. */
void Lufia2IntroMode7WorldInit(const uint8_t *rom, size_t rom_size);

/* Reconstructs the game's complete 4096x4096 world as a 512x512 Mode 7
 * tile-number plane, verifies that the currently loaded 1024x1024 PPU ring
 * is an exact window into it, and lifts the captured local affine lines into
 * full-world coordinates. Nothing is changed in guest WRAM or VRAM. */
Lufia2IntroMode7WorldStatus Lufia2IntroMode7WorldPrepare(
    const SnesPpuFrameCapture *capture,
    const SnesRecompMode7Line *local_lines,
    unsigned line_count,
    SnesRecompMode7Line *world_lines,
    unsigned world_line_capacity,
    SnesRecompMode7MapSource *world_source);

const char *Lufia2IntroMode7WorldStatusName(
    Lufia2IntroMode7WorldStatus status);
