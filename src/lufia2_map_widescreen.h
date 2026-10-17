#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "snes/ppu.h"
#include "lufia2_map_visibility.h"

enum {
    LUFIA2_WS_SHADOW_TILE_BIAS = 128,
    /* Lufia hides these native edge columns while streaming its tilemap. */
    LUFIA2_MAP_STREAM_GUARD_PIXELS = 16,
};

/* Who decides which map cells the margins may show. */
typedef enum Lufia2RoomVisibility {
    LUFIA2_ROOM_VISIBILITY_L2ROOMS = 0,
    /* Room data draws; disagreements with the game are reported. */
    LUFIA2_ROOM_VISIBILITY_COMPARE = 1,
    LUFIA2_ROOM_VISIBILITY_GAME = 2,
} Lufia2RoomVisibility;

typedef enum Lufia2MapWidescreenResult {
    LUFIA2_MAP_WIDESCREEN_DISABLED = 0,
    LUFIA2_MAP_WIDESCREEN_ACTIVE,
    LUFIA2_MAP_WIDESCREEN_LOADING,
} Lufia2MapWidescreenResult;

Lufia2MapWidescreenResult Lufia2PrepareMapWidescreen(
    Ppu *ppu,
    int margin_pixels);
void Lufia2FinalizeMapWidescreen(Ppu *ppu, int margin_pixels);
void Lufia2BeginMapRenderOverlay(Ppu *ppu);
void Lufia2EndMapRenderOverlay(Ppu *ppu);
void Lufia2DeactivateMapWidescreen(void);
void Lufia2MapWidescreenStateChanged(void);
/* The configured choice; LUFIA2_ROOM_VISIBILITY overrides it. */
void Lufia2MapWidescreenSetRoomVisibility(const char *configured);
