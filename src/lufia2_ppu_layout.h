#pragma once
#include "lufia2_video_policy.h"

/* Provisional admission. Offload still rejects live overlay capture and every
 * unresolved pixel policy before omitting software pixels. */
static inline bool Lufia2PpuLayoutRecorded(Lufia2VideoLayout layout,
    uint8_t runtime_map, bool intro_post, bool battle_active, bool ui_active) {
    /* Retain the existing consumer call contract. Resolved rendering policy,
     * rather than the room number, decides whether pixels can be deferred. */
    (void)runtime_map;
    (void)intro_post;
    if (battle_active || ui_active) return false;
    switch (layout) {
    case LUFIA2_VIDEO_REGULAR_MAP:
    case LUFIA2_VIDEO_WORLD_MAP:
    case LUFIA2_VIDEO_INTRO_MODE7:
    case LUFIA2_VIDEO_NATIVE:
    case LUFIA2_VIDEO_CENTERED:
    case LUFIA2_VIDEO_PATTERN_MENU:
        return true;
    default:
        return false;
    }
}
