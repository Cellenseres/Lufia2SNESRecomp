#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
    LUFIA2_ROOM_PATCH_FILL = 0xfffe,
    LUFIA2_ROOM_PATCH_TRANSPARENT = 0xfffd,
    LUFIA2_ROOM_EXTEND_FILL = 0,
    LUFIA2_ROOM_EXTEND_MAP = 1,
    LUFIA2_ROOM_EXTEND_VISIBLE = 2,
    /* Old files: one paint table per room. */
    LUFIA2_ROOM_MAX_TABLES = 32,
};

typedef struct Lufia2RoomVoid {
    uint16_t mode;
    uint16_t x;
    uint16_t y;
    uint16_t block;
} Lufia2RoomVoid;

typedef struct Lufia2MapPaint {
    uint8_t map_width;
    uint8_t map_height;
    uint16_t band;
    const uint8_t *payload;
    size_t payload_size;
    /* Fill past the map edges, per logical layer. */
    Lufia2RoomVoid bg1;
    Lufia2RoomVoid bg2;
    /* Index by logical layer: BG2=0, BG1=1. */
    uint8_t extend[2];
    /* Two bits per direction (left, right, up, down): 0/1/2 = 1/2/3 cells. */
    uint8_t repeat;
    /* v6 cache: row starts and sorted x/BG1/BG2 triples. */
    const uint32_t *paint_rows;
    const uint16_t *paint_cells;
    uint32_t paint_cell_count;
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

/* Painted cells may lie inside or beyond map edges. */
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
