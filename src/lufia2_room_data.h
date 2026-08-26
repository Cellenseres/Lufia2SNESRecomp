#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum { LUFIA2_ROOM_ID_NONE = 0xffff };

typedef enum Lufia2RoomDirection {
    LUFIA2_ROOM_UP = 0,
    LUFIA2_ROOM_DOWN = 1,
    LUFIA2_ROOM_LEFT = 2,
    LUFIA2_ROOM_RIGHT = 3,
} Lufia2RoomDirection;

typedef enum Lufia2RoomLookupResult {
    LUFIA2_ROOM_NOT_AUTHORED = 0,
    LUFIA2_ROOM_FOUND,
    LUFIA2_ROOM_NO_OWNER,
    LUFIA2_ROOM_MAP_NOT_READY,
    LUFIA2_ROOM_INVALID,
} Lufia2RoomLookupResult;

typedef enum Lufia2RoomVoidMode {
    LUFIA2_ROOM_VOID_TRANSPARENT = 0,
    LUFIA2_ROOM_VOID_SOURCE_CELL = 1,
    LUFIA2_ROOM_VOID_BLOCK = 2,
} Lufia2RoomVoidMode;

typedef struct Lufia2RoomVoid {
    uint16_t mode;
    uint16_t x;
    uint16_t y;
    uint16_t block;
} Lufia2RoomVoid;

typedef struct Lufia2RoomSelection {
    uint16_t room_id;
    const char *name;
    uint8_t map_width;
    uint8_t map_height;
    const uint8_t *map_payload;
    size_t map_payload_size;
    uint32_t activation_offset;
    uint32_t activation_count;
    uint32_t visibility_offset;
    uint32_t visibility_count;
    Lufia2RoomVoid bg1;
    Lufia2RoomVoid bg2;
} Lufia2RoomSelection;

/* Visible Areas may share cells along a shared wall or doorway. When more
   than one owns the player's cell, `preferred_room_id` wins if it is among
   them, so the player keeps the room already entered; pass
   LUFIA2_ROOM_ID_NONE to take the first authored record instead. */
Lufia2RoomLookupResult Lufia2RoomDataFindOwner(
    uint8_t map_id,
    uint8_t map_width,
    uint8_t map_height,
    uint16_t player_x,
    uint16_t player_y,
    uint16_t preferred_room_id,
    Lufia2RoomSelection *selection);

/* Authored transition cell: which room the player is in after leaving this
   cell in that direction. Ownership cannot express this, because a doorway is
   drawn by one room and walked into from another. */
bool Lufia2RoomDataFindTransition(
    uint8_t map_id,
    uint8_t map_width,
    uint8_t map_height,
    uint16_t cell_x,
    uint16_t cell_y,
    Lufia2RoomDirection direction,
    uint16_t *room_id);

bool Lufia2RoomDataSelectRoomById(
    uint8_t map_id,
    uint8_t map_width,
    uint8_t map_height,
    uint16_t room_id,
    Lufia2RoomSelection *selection);

bool Lufia2RoomDataCellIsVisible(
    const Lufia2RoomSelection *selection,
    uint32_t cell);

const char *Lufia2RoomDirectionName(Lufia2RoomDirection direction);
const char *Lufia2RoomLookupResultName(Lufia2RoomLookupResult result);
const char *Lufia2RoomDataLastError(void);
