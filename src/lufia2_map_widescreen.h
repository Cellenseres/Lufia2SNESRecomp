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

typedef enum Lufia2MapWidescreenResult {
    LUFIA2_MAP_WIDESCREEN_DISABLED = 0,
    LUFIA2_MAP_WIDESCREEN_ACTIVE,
    LUFIA2_MAP_WIDESCREEN_LOADING,
} Lufia2MapWidescreenResult;

Lufia2MapWidescreenResult Lufia2PrepareMapWidescreen(
    Ppu *ppu,
    int margin_pixels);

/* The field map this frame shows, for a presentation that wants more of it
   than the PPU's one-screen tilemaps hold. Map pixels count from the map's
   top left; the picture's row `y` shows map row `camera_y + 1 + y`. Cells
   are 16 pixels, tiles 8. */
typedef struct Lufia2FieldMap {
    int32_t camera_x, camera_y;   /* BG1 scroll, unwrapped to the map */
    uint8_t width, height;        /* in cells */
} Lufia2FieldMap;

/* False while no processed map is in bank $7F, or while it does not explain
   what the PPU shows. On success the readers below answer for this frame's
   map. Past its edges the game draws the map again, and tiles, blocks and
   room marks are answered the same way; nobody can step there. */
bool Lufia2FieldMapRead(const Ppu *ppu, Lufia2FieldMap *field);
/* Tilemap word the game itself would draw for `bg` (0 or 1) at a map tile:
   a cell of another room comes out as the filler it draws for one. */
uint16_t Lufia2FieldMapTile(int bg, int32_t map_tile_x, int32_t map_tile_y);
/* The cell's block in `bg`'s layer. */
uint16_t Lufia2FieldMapBlock(int bg, int32_t cell_x, int32_t cell_y);
/* The cell belongs to another room than the one shown. */
bool Lufia2FieldMapHidden(int32_t cell_x, int32_t cell_y);
/* An actor stands in the cell. */
bool Lufia2FieldMapOccupied(int32_t cell_x, int32_t cell_y);
/* Can anyone step between this cell and the one to its right, or below it?
   Actors in the way do not count. */
bool Lufia2FieldMapOpenRight(int32_t cell_x, int32_t cell_y);
bool Lufia2FieldMapOpenDown(int32_t cell_x, int32_t cell_y);
void Lufia2FinalizeMapWidescreen(Ppu *ppu, int margin_pixels);
void Lufia2BeginMapRenderOverlay(Ppu *ppu);
void Lufia2EndMapRenderOverlay(Ppu *ppu);
void Lufia2DeactivateMapWidescreen(void);
void Lufia2MapWidescreenStateChanged(void);
