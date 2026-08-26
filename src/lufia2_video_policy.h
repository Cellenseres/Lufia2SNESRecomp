#pragma once

#include <stdbool.h>

#include "snes/ppu.h"

typedef enum Lufia2VideoLayout {
    LUFIA2_VIDEO_NATIVE = 0,
    LUFIA2_VIDEO_CENTERED,
    LUFIA2_VIDEO_PATTERN_MENU,
    LUFIA2_VIDEO_WORLD_MAP,
    LUFIA2_VIDEO_REGULAR_MAP,
    LUFIA2_VIDEO_MAP_LOADING,
    LUFIA2_VIDEO_BLANK,
    LUFIA2_VIDEO_LAYOUT_COUNT,
} Lufia2VideoLayout;

Lufia2VideoLayout Lufia2SelectVideoLayout(
    const Ppu *ppu,
    bool widescreen_requested);

const char *Lufia2VideoLayoutName(Lufia2VideoLayout layout);
