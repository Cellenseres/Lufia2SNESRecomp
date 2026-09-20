#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "snes/ppu.h"

typedef enum Lufia2VideoLayout {
    LUFIA2_VIDEO_NATIVE = 0,
    LUFIA2_VIDEO_CENTERED,
    LUFIA2_VIDEO_PATTERN_MENU,
    LUFIA2_VIDEO_WORLD_MAP,
    LUFIA2_VIDEO_INTRO_MODE7,
    LUFIA2_VIDEO_BATTLE,
    LUFIA2_VIDEO_REGULAR_MAP,
    LUFIA2_VIDEO_MAP_LOADING,
    LUFIA2_VIDEO_BLANK,
    LUFIA2_VIDEO_LAYOUT_COUNT,
} Lufia2VideoLayout;

typedef enum Lufia2IntroMode7Raster {
    LUFIA2_INTRO_RASTER_UNKNOWN = 0,
    LUFIA2_INTRO_RASTER_BLANK,
    LUFIA2_INTRO_RASTER_MODE7,
    LUFIA2_INTRO_RASTER_REJECTED,
} Lufia2IntroMode7Raster;

typedef enum Lufia2IntroMode7RejectReason {
    LUFIA2_INTRO_REJECT_NONE = 0,
    LUFIA2_INTRO_REJECT_INVALID_HISTORY,
    LUFIA2_INTRO_REJECT_MIXED_MODE,
    LUFIA2_INTRO_REJECT_FOREIGN_MAIN_LAYER,
    LUFIA2_INTRO_REJECT_SUBSCREEN,
    LUFIA2_INTRO_REJECT_EXTBG,
    LUFIA2_INTRO_REJECT_NO_BG1,
} Lufia2IntroMode7RejectReason;

typedef struct Lufia2IntroMode7RasterDetail {
    Lufia2IntroMode7Raster classification;
    Lufia2IntroMode7RejectReason reason;
    unsigned line;
    uint8_t inidisp;
    uint8_t bgmode;
    uint8_t setini;
    uint8_t main_enable;
    uint8_t sub_enable;
} Lufia2IntroMode7RasterDetail;

typedef struct Lufia2VideoObservation {
    uint8_t runtime_map;
    uint32_t resume_pc;
    Lufia2IntroMode7Raster intro_raster;
    bool battle_layout;
} Lufia2VideoObservation;

bool Lufia2IntroMode7Candidate(const Lufia2VideoObservation *observation);

/* Classifies the completed visible raster history. Each row is the raw PPU
 * register block captured immediately before its scanline was rendered. */
Lufia2IntroMode7Raster Lufia2ClassifyIntroMode7Raster(
    const uint8_t *rows, size_t stride, unsigned line_count,
    bool history_valid);

Lufia2IntroMode7RasterDetail Lufia2InspectIntroMode7Raster(
    const uint8_t *rows, size_t stride, unsigned line_count,
    bool history_valid);

const char *Lufia2IntroMode7RejectReasonName(
    Lufia2IntroMode7RejectReason reason);

Lufia2VideoLayout Lufia2SelectVideoLayout(
    const Ppu *ppu,
    bool widescreen_requested);

Lufia2VideoLayout Lufia2SelectVideoLayoutObserved(
    const Ppu *ppu,
    bool widescreen_requested,
    const Lufia2VideoObservation *observation);

const char *Lufia2VideoLayoutName(Lufia2VideoLayout layout);
