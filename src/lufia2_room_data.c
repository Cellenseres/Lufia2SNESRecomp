#include "lufia2_room_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crc32.h"

enum {
    LUFIA2_ROOM_HEADER_SIZE = 64,
    LUFIA2_ROOM_DIRECTORY_SIZE = 48,
    LUFIA2_ROOM_MAP_HEADER_V2_SIZE = 20,
    LUFIA2_ROOM_MAP_HEADER_SIZE = 24,
    LUFIA2_ROOM_TRANSITION_SIZE = 8,
    LUFIA2_ROOM_V1_SIZE = 52,
    LUFIA2_ROOM_V2_SIZE = 44,
    LUFIA2_ROOM_SPAN_SIZE = 6,
    LUFIA2_ROOM_MAX_FILE_SIZE = 16 * 1024 * 1024,
};

static const uint8_t kLufia2RomSha256[32] = {
    0x7c, 0x34, 0xec, 0xb1, 0x6c, 0x10, 0xf5, 0x51,
    0x12, 0x0e, 0xd7, 0xb8, 0x6c, 0xfb, 0xc9, 0x47,
    0x04, 0x2f, 0x47, 0x9b, 0x52, 0xee, 0x74, 0xbb,
    0x3c, 0x40, 0xe9, 0x2f, 0xdd, 0x19, 0x2b, 0x3a,
};

static uint8_t *s_data;
static size_t s_data_size;
static uint16_t s_version;
static uint32_t s_map_count;
static uint32_t s_directory_offset;
static uint32_t s_strings_offset;
static bool s_load_attempted;
static bool s_load_valid;
static const char *s_last_error;

static uint16_t Read16(const uint8_t *data) {
    return (uint16_t)(data[0] | ((uint16_t)data[1] << 8));
}

static uint32_t Read32(const uint8_t *data) {
    return (uint32_t)data[0] |
        ((uint32_t)data[1] << 8) |
        ((uint32_t)data[2] << 16) |
        ((uint32_t)data[3] << 24);
}

static bool RangeIsValid(size_t offset, size_t size, size_t limit) {
    return offset <= limit && size <= limit - offset;
}

static bool LoadFile(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file)
        return false;
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return false;
    }
    const long length = ftell(file);
    if (length < LUFIA2_ROOM_HEADER_SIZE ||
        length > LUFIA2_ROOM_MAX_FILE_SIZE ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return false;
    }

    uint8_t *data = (uint8_t *)malloc((size_t)length);
    if (!data) {
        fclose(file);
        return false;
    }
    const bool read_ok =
        fread(data, 1, (size_t)length, file) == (size_t)length;
    fclose(file);
    if (!read_ok) {
        free(data);
        return false;
    }

    s_data = data;
    s_data_size = (size_t)length;
    return true;
}

static bool ValidateHeader(void) {
    if (memcmp(s_data, "L2RD", 4) != 0)
        return false;
    s_version = Read16(s_data + 4);
    if ((s_version < 1 || s_version > 3) ||
        Read16(s_data + 6) != LUFIA2_ROOM_HEADER_SIZE ||
        memcmp(s_data + 8, kLufia2RomSha256, 32) != 0) {
        return false;
    }

    s_map_count = Read32(s_data + 40);
    s_directory_offset = Read32(s_data + 44);
    s_strings_offset = Read32(s_data + 48);
    if (Read32(s_data + 52) != s_data_size ||
        !RangeIsValid(
            s_directory_offset,
            (size_t)s_map_count * LUFIA2_ROOM_DIRECTORY_SIZE,
            s_strings_offset) ||
        s_strings_offset > s_data_size) {
        return false;
    }

    return crc32_compute(
        s_data + LUFIA2_ROOM_HEADER_SIZE,
        s_data_size - LUFIA2_ROOM_HEADER_SIZE) == Read32(s_data + 56);
}

static void EnsureLoaded(void) {
    if (s_load_attempted)
        return;
    s_load_attempted = true;

    const char *path = getenv("LUFIA2_ROOM_DATA");
    if (!path || !*path)
        path = "data/widescreen/lufia2_rooms.l2rooms";

    if (!LoadFile(path)) {
        fprintf(stderr,
            "[room-data] no room metadata at %s; using stock map policy\n",
            path);
        return;
    }
    if (!ValidateHeader()) {
        fprintf(stderr, "[room-data] rejected invalid room metadata: %s\n", path);
        free(s_data);
        s_data = NULL;
        s_data_size = 0;
        return;
    }

    s_load_valid = true;
    fprintf(stderr,
        "[room-data] loaded %u authored map(s), format version %u: %s\n",
        (unsigned)s_map_count, (unsigned)s_version, path);
}

static const uint8_t *FindMapDirectory(uint8_t map_id) {
    for (uint32_t index = 0; index < s_map_count; index++) {
        const uint8_t *entry = s_data + s_directory_offset +
            index * LUFIA2_ROOM_DIRECTORY_SIZE;
        if (Read16(entry) == map_id)
            return entry;
    }
    return NULL;
}

static Lufia2RoomLookupResult InvalidData(const char *reason) {
    s_last_error = reason;
    return LUFIA2_ROOM_INVALID;
}

static Lufia2RoomLookupResult MapNotReady(const char *reason) {
    s_last_error = reason;
    return LUFIA2_ROOM_MAP_NOT_READY;
}

static int SpanTableContains(
    const uint8_t *payload,
    size_t payload_size,
    uint32_t offset,
    uint32_t count,
    uint8_t width,
    uint8_t height,
    uint16_t cell_x,
    uint16_t cell_y) {
    if (!RangeIsValid(
            offset, (size_t)count * LUFIA2_ROOM_SPAN_SIZE, payload_size)) {
        return -1;
    }

    bool contains = false;
    for (uint32_t index = 0; index < count; index++) {
        const uint8_t *span = payload + offset + index * LUFIA2_ROOM_SPAN_SIZE;
        const uint16_t y = Read16(span);
        const uint16_t x0 = Read16(span + 2);
        const uint16_t x1 = Read16(span + 4);
        if (y >= height || x0 >= x1 || x1 > width)
            return -1;
        if (y == cell_y && cell_x >= x0 && cell_x < x1)
            contains = true;
    }
    return contains ? 1 : 0;
}

static bool ReadVoid(
    const uint8_t *room,
    size_t offset,
    uint8_t width,
    uint8_t height,
    Lufia2RoomVoid *definition) {
    definition->mode = Read16(room + offset);
    definition->x = Read16(room + offset + 2);
    definition->y = Read16(room + offset + 4);
    definition->block = Read16(room + offset + 6);
    if (definition->mode > LUFIA2_ROOM_VOID_BLOCK)
        return false;
    if (definition->mode == LUFIA2_ROOM_VOID_SOURCE_CELL &&
        (definition->x >= width || definition->y >= height)) {
        return false;
    }
    return true;
}

static bool ReadSelection(
    const uint8_t *payload,
    size_t payload_size,
    const uint8_t *room,
    uint8_t width,
    uint8_t height,
    Lufia2RoomSelection *selection) {
    memset(selection, 0, sizeof(*selection));
    selection->room_id = Read16(room);
    selection->map_width = width;
    selection->map_height = height;
    selection->map_payload = payload;
    selection->map_payload_size = payload_size;

    const uint32_t name_offset = Read32(room + 4);
    if (name_offset >= s_data_size - s_strings_offset)
        return false;
    const char *name = (const char *)(s_data + s_strings_offset + name_offset);
    if (!memchr(name, '\0', s_data_size - s_strings_offset - name_offset))
        return false;
    selection->name = name;

    if (s_version == 1) {
        selection->activation_offset = Read32(room + 12);
        selection->activation_count = Read32(room + 16);
        selection->visibility_offset = Read32(room + 20);
        selection->visibility_count = Read32(room + 24);
        return ReadVoid(room, 36, width, height, &selection->bg1) &&
            ReadVoid(room, 44, width, height, &selection->bg2);
    }

    selection->visibility_offset = Read32(room + 12);
    selection->visibility_count = Read32(room + 16);
    return ReadVoid(room, 28, width, height, &selection->bg1) &&
        ReadVoid(room, 36, width, height, &selection->bg2);
}

/* One map's validated payload: the room record table and, from version 3,
   the transition table. */
typedef struct Lufia2MapPayload {
    const uint8_t *payload;
    uint32_t payload_size;
    uint32_t records_offset;
    uint32_t room_size;
    uint16_t room_count;
    uint32_t transition_offset;
    uint32_t transition_count;
} Lufia2MapPayload;

static Lufia2RoomLookupResult ResolveMapPayload(
    uint8_t map_id,
    uint8_t map_width,
    uint8_t map_height,
    Lufia2MapPayload *out) {
    s_last_error = NULL;
    EnsureLoaded();
    if (!s_load_valid)
        return LUFIA2_ROOM_NOT_AUTHORED;

    const uint8_t *directory = FindMapDirectory(map_id);
    if (!directory)
        return LUFIA2_ROOM_NOT_AUTHORED;
    if (directory[2] != map_width || directory[3] != map_height)
        return MapNotReady("runtime map dimensions are still changing");

    const uint32_t header_size = s_version >= 3
        ? LUFIA2_ROOM_MAP_HEADER_SIZE : LUFIA2_ROOM_MAP_HEADER_V2_SIZE;
    const uint32_t payload_offset = Read32(directory + 40);
    const uint32_t payload_size = Read32(directory + 44);
    const size_t directory_end = s_directory_offset +
        (size_t)s_map_count * LUFIA2_ROOM_DIRECTORY_SIZE;
    if (payload_offset < directory_end ||
        !RangeIsValid(payload_offset, payload_size, s_strings_offset) ||
        payload_size < header_size) {
        return InvalidData("map payload is out of bounds");
    }
    const uint8_t *payload = s_data + payload_offset;
    const uint32_t room_size = Read32(payload + 8);
    const uint32_t expected_room_size =
        s_version == 1 ? LUFIA2_ROOM_V1_SIZE : LUFIA2_ROOM_V2_SIZE;
    const uint16_t room_count = Read16(directory + 4);
    if (memcmp(payload, "MAP0", 4) != 0 ||
        Read32(payload + 4) != payload_size ||
        room_size != expected_room_size ||
        !RangeIsValid(
            header_size, (size_t)room_count * room_size, payload_size)) {
        return InvalidData("map payload header is invalid");
    }

    out->payload = payload;
    out->payload_size = payload_size;
    out->records_offset = header_size;
    out->room_size = room_size;
    out->room_count = room_count;
    out->transition_offset = 0;
    out->transition_count = 0;
    if (s_version >= 3) {
        out->transition_count = Read16(payload + 18);
        out->transition_offset = Read32(payload + 20);
        if (!RangeIsValid(
                out->transition_offset,
                (size_t)out->transition_count * LUFIA2_ROOM_TRANSITION_SIZE,
                payload_size)) {
            return InvalidData("transition table is out of bounds");
        }
    }
    return LUFIA2_ROOM_FOUND;
}

/* A transition cell records which room the player is in after leaving it in
   the given direction. The visibility masks cannot express that on their own:
   a doorway is drawn by the room above it but walked into from the room
   below, so ownership alone always names the wrong room there. */
bool Lufia2RoomDataFindTransition(
    uint8_t map_id,
    uint8_t map_width,
    uint8_t map_height,
    uint16_t cell_x,
    uint16_t cell_y,
    Lufia2RoomDirection direction,
    uint16_t *room_id) {
    Lufia2MapPayload map;
    if (ResolveMapPayload(map_id, map_width, map_height, &map) !=
        LUFIA2_ROOM_FOUND) {
        return false;
    }

    for (uint32_t index = 0; index < map.transition_count; index++) {
        const uint8_t *record = map.payload + map.transition_offset +
            index * LUFIA2_ROOM_TRANSITION_SIZE;
        if (Read16(record) != cell_x || Read16(record + 2) != cell_y ||
            Read16(record + 4) != (uint16_t)direction) {
            continue;
        }
        *room_id = Read16(record + 6);
        return true;
    }
    return false;
}

bool Lufia2RoomDataSelectRoomById(
    uint8_t map_id,
    uint8_t map_width,
    uint8_t map_height,
    uint16_t room_id,
    Lufia2RoomSelection *selection) {
    Lufia2MapPayload map;
    if (ResolveMapPayload(map_id, map_width, map_height, &map) !=
        LUFIA2_ROOM_FOUND) {
        return false;
    }

    for (uint16_t index = 0; index < map.room_count; index++) {
        const uint8_t *room = map.payload + map.records_offset +
            (size_t)index * map.room_size;
        Lufia2RoomSelection candidate;
        if (!ReadSelection(
                map.payload, map.payload_size, room,
                map_width, map_height, &candidate)) {
            return false;
        }
        if (candidate.room_id != room_id)
            continue;
        *selection = candidate;
        return true;
    }
    return false;
}

Lufia2RoomLookupResult Lufia2RoomDataFindOwner(
    uint8_t map_id,
    uint8_t map_width,
    uint8_t map_height,
    uint16_t player_x,
    uint16_t player_y,
    uint16_t preferred_room_id,
    Lufia2RoomSelection *selection) {
    Lufia2MapPayload map;
    const Lufia2RoomLookupResult resolved =
        ResolveMapPayload(map_id, map_width, map_height, &map);
    if (resolved != LUFIA2_ROOM_FOUND)
        return resolved;

    const uint8_t *payload = map.payload;
    const uint32_t payload_size = map.payload_size;
    const uint16_t room_count = map.room_count;
    const uint32_t room_size = map.room_size;

    const uint16_t cell_x = player_x / 16u;
    const uint16_t cell_y = player_y / 16u;
    if (cell_x >= map_width || cell_y >= map_height)
        return LUFIA2_ROOM_NO_OWNER;

    unsigned owners = 0;
    bool preferred_owns = false;
    Lufia2RoomSelection first = {0};
    Lufia2RoomSelection preferred = {0};
    for (uint16_t index = 0; index < room_count; index++) {
        const uint8_t *room = payload + map.records_offset +
            (size_t)index * room_size;
        Lufia2RoomSelection candidate;
        if (!ReadSelection(
                payload, payload_size, room,
                map_width, map_height, &candidate)) {
            return InvalidData("room record is invalid");
        }
        const int visible = SpanTableContains(
            payload, payload_size,
            candidate.visibility_offset, candidate.visibility_count,
            map_width, map_height, cell_x, cell_y);
        const int activation = SpanTableContains(
            payload, payload_size,
            candidate.activation_offset, candidate.activation_count,
            map_width, map_height, cell_x, cell_y);
        if (visible < 0 || activation < 0)
            return InvalidData("room span table is invalid");
        if (!visible && !activation)
            continue;
        if (owners == 0)
            first = candidate;
        if (candidate.room_id == preferred_room_id) {
            preferred = candidate;
            preferred_owns = true;
        }
        owners++;
    }

    if (owners == 0)
        return LUFIA2_ROOM_NO_OWNER;
    *selection = preferred_owns ? preferred : first;
    return LUFIA2_ROOM_FOUND;
}

bool Lufia2RoomDataCellIsVisible(
    const Lufia2RoomSelection *selection,
    uint32_t cell) {
    if (!selection || !selection->map_width ||
        cell >= (uint32_t)selection->map_width * selection->map_height) {
        return false;
    }
    const uint16_t x = (uint16_t)(cell % selection->map_width);
    const uint16_t y = (uint16_t)(cell / selection->map_width);
    const int visible = SpanTableContains(
        selection->map_payload, selection->map_payload_size,
        selection->visibility_offset, selection->visibility_count,
        selection->map_width, selection->map_height, x, y);
    const int activation = SpanTableContains(
        selection->map_payload, selection->map_payload_size,
        selection->activation_offset, selection->activation_count,
        selection->map_width, selection->map_height, x, y);
    return visible > 0 || activation > 0;
}

const char *Lufia2RoomDirectionName(Lufia2RoomDirection direction) {
    switch (direction) {
    case LUFIA2_ROOM_UP: return "up";
    case LUFIA2_ROOM_DOWN: return "down";
    case LUFIA2_ROOM_LEFT: return "left";
    case LUFIA2_ROOM_RIGHT: return "right";
    default: return "unknown";
    }
}

const char *Lufia2RoomLookupResultName(Lufia2RoomLookupResult result) {
    switch (result) {
    case LUFIA2_ROOM_NOT_AUTHORED: return "not authored";
    case LUFIA2_ROOM_FOUND: return "found";
    case LUFIA2_ROOM_NO_OWNER: return "no owner";
    case LUFIA2_ROOM_MAP_NOT_READY: return "map transition pending";
    case LUFIA2_ROOM_INVALID: return "invalid data";
    default: return "unknown";
    }
}

const char *Lufia2RoomDataLastError(void) {
    return s_last_error;
}
