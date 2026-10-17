#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Per-map widescreen paint; old room files merge on load. */

typedef enum Lufia2RoomLookupResult {
    LUFIA2_ROOM_NOT_AUTHORED = 0,
    LUFIA2_ROOM_FOUND,
    LUFIA2_ROOM_MAP_NOT_READY,
    LUFIA2_ROOM_INVALID,
} Lufia2RoomLookupResult;

typedef enum Lufia2RoomVoidMode {
    LUFIA2_ROOM_VOID_TRANSPARENT = 0,
    LUFIA2_ROOM_VOID_SOURCE_CELL = 1,
    LUFIA2_ROOM_VOID_BLOCK = 2,
} Lufia2RoomVoidMode;

enum {
    LUFIA2_ROOM_PATCH_BAND = 8,
    LUFIA2_ROOM_MAX_PATCH_BAND = 64,
    LUFIA2_ROOM_PATCH_NONE = 0xffff,
    /* Old files: one paint table per room. */
    LUFIA2_ROOM_MAX_TABLES = 32,
};

typedef struct Lufia2RoomVoid {
    uint16_t mode;
    uint16_t x;
    uint16_t y;
    uint16_t block;
} Lufia2RoomVoid;

/* What one map draws in the margins besides the game's cells. */
typedef struct Lufia2MapPaint {
    uint8_t map_width;
    uint8_t map_height;
    uint16_t band;
    const uint8_t *payload;
    size_t payload_size;
    /* Fill past the map edges, per logical layer. */
    Lufia2RoomVoid bg1;
    Lufia2RoomVoid bg2;
    /* Searched in order; the first painted layer wins. */
    uint32_t table_count;
    uint32_t patch_offset[LUFIA2_ROOM_MAX_TABLES];
    uint32_t patch_count[LUFIA2_ROOM_MAX_TABLES];
} Lufia2MapPaint;

Lufia2RoomLookupResult Lufia2RoomDataFindMap(
    uint8_t map_id,
    uint8_t map_width,
    uint8_t map_height,
    Lufia2MapPaint *paint);

/* A painted block for this cell and layer, inside or outside the map. */
bool Lufia2RoomDataPatchBlock(
    const Lufia2MapPaint *paint,
    int32_t cell_x,
    int32_t cell_y,
    int logical_layer,
    uint16_t *block);

const char *Lufia2RoomLookupResultName(Lufia2RoomLookupResult result);
const char *Lufia2RoomDataLastError(void);
/* Format version of the loaded file, or 0. */
unsigned Lufia2RoomDataVersion(void);
