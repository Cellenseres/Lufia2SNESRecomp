#include "lufia2_room_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crc32.h"
#include "lufia2_log.h"
#include "lufia2_room_inflate.h"

enum {
    LUFIA2_ROOM_VERSION = 6,
    LUFIA2_ROOM_HEADER_SIZE = 64,
    LUFIA2_ROOM_DIRECTORY_SIZE = 48,
    LUFIA2_ROOM_PATCH_SIZE = 8,
    LUFIA2_ROOM_MAX_FILE_SIZE = 16 * 1024 * 1024,
    /* Version 5 map payload. */
    LUFIA2_ROOM_MAP_SIZE = 52,
    /* Versions 1-4: rooms in a MAP0 payload. */
    LUFIA2_ROOM_MAP_HEADER_V2_SIZE = 20,
    LUFIA2_ROOM_MAP_HEADER_V3_SIZE = 24,
    LUFIA2_ROOM_V1_SIZE = 52,
    LUFIA2_ROOM_V2_SIZE = 44,
    LUFIA2_ROOM_V4_SIZE = 52,
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
static void *s_paint_storage;

static struct {
    bool valid;
    uint8_t map_id;
    uint8_t width;
    uint8_t height;
    Lufia2MapPaint paint;
    Lufia2RoomLookupResult result;
    const char *error;
} s_cached_map;

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

static bool TableIsValid(size_t offset, uint32_t count, size_t stride,
                         size_t limit) {
    return offset <= limit && count <= (limit - offset) / stride;
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
    if ((s_version < 1 || s_version > LUFIA2_ROOM_VERSION) ||
        Read16(s_data + 6) != LUFIA2_ROOM_HEADER_SIZE ||
        memcmp(s_data + 8, kLufia2RomSha256, 32) != 0) {
        return false;
    }

    s_map_count = Read32(s_data + 40);
    s_directory_offset = Read32(s_data + 44);
    s_strings_offset = Read32(s_data + 48);
    if (Read32(s_data + 52) != s_data_size ||
        s_directory_offset < LUFIA2_ROOM_HEADER_SIZE ||
        !TableIsValid(s_directory_offset, s_map_count,
                      LUFIA2_ROOM_DIRECTORY_SIZE, s_strings_offset) ||
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
        path = "assets/widescreen/lufia2.l2rooms";

    if (!LoadFile(path)) {
        LUFIA2_LOG(
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
    LUFIA2_LOG(
        "[room-data] loaded %u authored map(s), format version %u%s: %s\n",
        (unsigned)s_map_count, (unsigned)s_version,
        s_version < 5 ? " (rooms merged per map)" : "",
        path);
}

static const uint8_t *FindMapDirectory(uint8_t map_id) {
    for (uint32_t index = 0; index < s_map_count; index++) {
        const uint8_t *entry = s_data + s_directory_offset +
            index * LUFIA2_ROOM_DIRECTORY_SIZE;
        /* Version 5: other kinds are for later readers. */
        if (s_version >= 5 && Read16(entry + 4) != 0)
            continue;
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

static bool ReadVoid(
    const uint8_t *record,
    uint8_t width,
    uint8_t height,
    Lufia2RoomVoid *definition) {
    definition->mode = Read16(record);
    definition->x = Read16(record + 2);
    definition->y = Read16(record + 4);
    definition->block = Read16(record + 6);
    if (definition->mode > LUFIA2_ROOM_VOID_BLOCK)
        return false;
    if (definition->mode == LUFIA2_ROOM_VOID_SOURCE_CELL &&
        (definition->x >= width || definition->y >= height)) {
        return false;
    }
    return true;
}

static bool SameVoid(const Lufia2RoomVoid *a, const Lufia2RoomVoid *b) {
    if (a->mode != b->mode)
        return false;
    if (a->mode == LUFIA2_ROOM_VOID_SOURCE_CELL)
        return a->x == b->x && a->y == b->y;
    if (a->mode == LUFIA2_ROOM_VOID_BLOCK)
        return a->block == b->block;
    return true;
}

/* Most rooms win; a tie goes to the lowest room id. */
static Lufia2RoomVoid MajorityVoid(const Lufia2RoomVoid *voids, unsigned count) {
    unsigned best = 0;
    unsigned best_votes = 0;
    for (unsigned i = 0; i < count; i++) {
        unsigned votes = 0;
        for (unsigned j = 0; j < count; j++)
            votes += SameVoid(&voids[i], &voids[j]);
        if (votes > best_votes) {
            best = i;
            best_votes = votes;
        }
    }
    return voids[best];
}

static Lufia2RoomLookupResult ReadMapV5(
    const uint8_t *payload,
    uint32_t payload_size,
    Lufia2MapPaint *paint) {
    if (payload_size < LUFIA2_ROOM_MAP_SIZE ||
        memcmp(payload, "MAP5", 4) != 0 ||
        Read32(payload + 4) != payload_size ||
        Read16(payload + 8) < LUFIA2_ROOM_MAP_SIZE ||
        Read16(payload + 8) > payload_size)
        return InvalidData("map payload header is invalid");
    paint->band = Read16(payload + 18);
    if (paint->band > LUFIA2_ROOM_MAX_PATCH_BAND)
        return InvalidData("paint band is too wide");
    if (!ReadVoid(payload + 20, paint->map_width, paint->map_height,
                  &paint->bg1) ||
        !ReadVoid(payload + 28, paint->map_width, paint->map_height,
                  &paint->bg2))
        return InvalidData("map fill is invalid");
    const uint32_t patch_offset = Read32(payload + 36);
    const uint32_t patch_count = Read32(payload + 40);
    if (!TableIsValid(patch_offset, patch_count, LUFIA2_ROOM_PATCH_SIZE,
                      payload_size) ||
        !RangeIsValid(Read32(payload + 44), Read32(payload + 48),
                      payload_size))
        return InvalidData("map table is out of bounds");
    if (patch_count) {
        paint->patch_offset[0] = patch_offset;
        paint->patch_count[0] = patch_count;
        paint->table_count = 1;
    }
    return LUFIA2_ROOM_FOUND;
}

static bool PaintValueIsValid(uint16_t value) {
    return value <= 0x3ffu || value >= LUFIA2_ROOM_PATCH_TRANSPARENT;
}

static Lufia2RoomLookupResult ReadMapV6(
    const uint8_t *payload, uint32_t size, Lufia2MapPaint *paint) {
    if (size < 64 || memcmp(payload, "MAP6", 4) || Read32(payload + 4) != size)
        return InvalidData("invalid v6 map header");
    const uint32_t header = Read16(payload + 8);
    const uint32_t offset = Read32(payload + 36);
    const uint32_t count = Read32(payload + 40);
    const uint32_t extra = Read32(payload + 44);
    const uint32_t extra_size = Read32(payload + 48);
    const uint32_t encoded_size = Read32(payload + 52);
    const uint32_t decoded_size = Read32(payload + 56);
    const unsigned codec = payload[60];
    paint->band = Read16(payload + 18);
    paint->extend[1] = payload[61];
    paint->extend[0] = payload[62];
    paint->repeat = payload[63];
    if (header < 64 || header > size || paint->band > LUFIA2_ROOM_MAX_PATCH_BAND ||
        codec > 2 || payload[61] > 2 || payload[62] > 2 ||
        (paint->repeat & (paint->repeat >> 1) & 0x55u))
        return InvalidData("unsupported v6 map settings");
    const uint32_t width = paint->map_width + 2u * paint->band;
    const uint32_t height = paint->map_height + 2u * paint->band;
    const uint32_t area = width * height;
    if (!paint->map_width || !paint->map_height || count > area ||
        offset < header || !RangeIsValid(offset, encoded_size, size) ||
        !RangeIsValid(extra, extra_size, size) ||
        (extra_size && (extra < header ||
         (offset < extra + extra_size && extra < offset + encoded_size))))
        return InvalidData("v6 map sections are invalid");
    if (!ReadVoid(payload + 20, paint->map_width, paint->map_height, &paint->bg1) ||
        !ReadVoid(payload + 28, paint->map_width, paint->map_height, &paint->bg2) ||
        (paint->bg1.mode == LUFIA2_ROOM_VOID_BLOCK && paint->bg1.block > 0x3ffu) ||
        (paint->bg2.mode == LUFIA2_ROOM_VOID_BLOCK && paint->bg2.block > 0x3ffu))
        return InvalidData("v6 fill is invalid");
    if ((!count && (codec || encoded_size || decoded_size)) ||
        (codec == 0 && (encoded_size != count * 8u || decoded_size != encoded_size)) ||
        (codec != 0 && (decoded_size < count * 5u || decoded_size > count * 7u)) ||
        (codec == 1 && encoded_size != decoded_size))
        return InvalidData("v6 paint size is invalid");
    if (!count) {
        free(s_paint_storage);
        s_paint_storage = NULL;
        return LUFIA2_ROOM_FOUND;
    }
    uint8_t *inflated = NULL;
    const uint8_t *stream = payload + offset;
    if (codec == 2) {
        inflated = (uint8_t *)malloc(decoded_size);
        if (!inflated)
            return InvalidData("cannot allocate v6 inflate buffer");
        if (!Lufia2RoomInflate(stream, encoded_size, inflated, decoded_size)) {
            free(inflated);
            return InvalidData("invalid compressed v6 paint");
        }
        stream = inflated;
    }
    const size_t rows_size = (size_t)(height + 1u) * sizeof(uint32_t);
    uint32_t *rows = (uint32_t *)calloc(1, rows_size + (size_t)count * 6u);
    if (!rows) {
        free(inflated);
        return InvalidData("cannot allocate v6 paint cache");
    }
    uint16_t *cells = (uint16_t *)((uint8_t *)rows + rows_size);
    uint32_t cursor = 0, next_position = 0, next_row = 0;
    bool valid = true;
    for (uint32_t i = 0; i < count && valid; i++) {
        uint32_t position, x, y;
        uint16_t bg1, bg2;
        if (codec == 0) {
            const int32_t px = (int16_t)Read16(stream + cursor);
            const int32_t py = (int16_t)Read16(stream + cursor + 2);
            const int32_t band = paint->band;
            if (px < -band || py < -band || px >= (int32_t)paint->map_width + band ||
                py >= (int32_t)paint->map_height + band) {
                valid = false;
                break;
            }
            x = (uint32_t)(px + band);
            y = (uint32_t)(py + band);
            position = y * width + x;
            bg1 = Read16(stream + cursor + 4);
            bg2 = Read16(stream + cursor + 6);
            cursor += 8;
        } else {
            uint32_t gap = 0;
            unsigned shift = 0;
            for (;;) {
                if (cursor >= decoded_size || shift > 14) {
                    valid = false;
                    break;
                }
                const unsigned byte = stream[cursor++];
                gap |= (uint32_t)(byte & 127u) << shift;
                if (!(byte & 128u)) {
                    if (shift && !byte)
                        valid = false;
                    break;
                }
                shift += 7;
            }
            if (!valid || gap >= area || next_position >= area || gap >= area - next_position ||
                decoded_size - cursor < 4) {
                valid = false;
                break;
            }
            position = next_position + gap;
            x = position % width;
            y = position / width;
            bg1 = Read16(stream + cursor);
            bg2 = Read16(stream + cursor + 2);
            cursor += 4;
        }
        if (position < next_position || !PaintValueIsValid(bg1) || !PaintValueIsValid(bg2)) {
            valid = false;
            break;
        }
        next_position = position + 1;
        while (next_row <= y)
            rows[next_row++] = i;
        cells[i * 3u] = (uint16_t)x;
        cells[i * 3u + 1] = bg1;
        cells[i * 3u + 2] = bg2;
    }
    free(inflated);
    if (!valid || cursor != decoded_size) {
        free(rows);
        return InvalidData("invalid v6 paint records");
    }
    while (next_row <= height)
        rows[next_row++] = count;
    free(s_paint_storage);
    s_paint_storage = rows;
    paint->paint_rows = rows;
    paint->paint_cells = cells;
    paint->paint_cell_count = count;
    return LUFIA2_ROOM_FOUND;
}

/* Versions 1-4: merge the rooms like the editor's converter. */
static Lufia2RoomLookupResult ReadMapRooms(
    const uint8_t *directory,
    const uint8_t *payload,
    uint32_t payload_size,
    Lufia2MapPaint *paint) {
    const uint32_t header_size = s_version >= 3
        ? LUFIA2_ROOM_MAP_HEADER_V3_SIZE : LUFIA2_ROOM_MAP_HEADER_V2_SIZE;
    const uint32_t room_size =
        s_version == 1 ? LUFIA2_ROOM_V1_SIZE :
        s_version >= 4 ? LUFIA2_ROOM_V4_SIZE : LUFIA2_ROOM_V2_SIZE;
    const uint16_t room_count = Read16(directory + 4);
    if (payload_size < header_size ||
        memcmp(payload, "MAP0", 4) != 0 ||
        Read32(payload + 4) != payload_size ||
        Read32(payload + 8) != room_size ||
        !TableIsValid(header_size, room_count, room_size, payload_size))
        return InvalidData("map payload header is invalid");
    if (room_count > LUFIA2_ROOM_MAX_TABLES)
        return InvalidData("too many rooms to merge");

    /* Room records by ascending id. */
    const uint8_t *rooms[LUFIA2_ROOM_MAX_TABLES];
    for (uint16_t index = 0; index < room_count; index++) {
        const uint8_t *room = payload + header_size + (size_t)index * room_size;
        uint16_t at = index;
        while (at > 0 && Read16(rooms[at - 1]) > Read16(room)) {
            rooms[at] = rooms[at - 1];
            at--;
        }
        rooms[at] = room;
    }

    Lufia2RoomVoid bg1[LUFIA2_ROOM_MAX_TABLES];
    Lufia2RoomVoid bg2[LUFIA2_ROOM_MAX_TABLES];
    const uint32_t void_at = s_version == 1 ? 36 : 28;
    for (uint16_t index = 0; index < room_count; index++) {
        const uint8_t *room = rooms[index];
        if (!ReadVoid(room + void_at, paint->map_width, paint->map_height,
                      &bg1[index]) ||
            !ReadVoid(room + void_at + 8, paint->map_width,
                      paint->map_height, &bg2[index]))
            return InvalidData("room fill is invalid");
        if (s_version < 4)
            continue;
        const uint32_t patch_offset = Read32(room + 44);
        const uint32_t patch_count = Read32(room + 48);
        if (!TableIsValid(patch_offset, patch_count, LUFIA2_ROOM_PATCH_SIZE,
                          payload_size))
            return InvalidData("patch table is out of bounds");
        if (!patch_count)
            continue;
        paint->patch_offset[paint->table_count] = patch_offset;
        paint->patch_count[paint->table_count] = patch_count;
        paint->table_count++;
    }
    if (room_count) {
        paint->bg1 = MajorityVoid(bg1, room_count);
        paint->bg2 = MajorityVoid(bg2, room_count);
    }
    paint->band = LUFIA2_ROOM_PATCH_BAND;
    return LUFIA2_ROOM_FOUND;
}

static Lufia2RoomLookupResult ReadMap(
    uint8_t map_id,
    uint8_t map_width,
    uint8_t map_height,
    Lufia2MapPaint *paint) {
    s_last_error = NULL;
    memset(paint, 0, sizeof(*paint));
    EnsureLoaded();
    if (!s_load_valid)
        return LUFIA2_ROOM_NOT_AUTHORED;

    const uint8_t *directory = FindMapDirectory(map_id);
    if (!directory)
        return LUFIA2_ROOM_NOT_AUTHORED;
    if (directory[2] != map_width || directory[3] != map_height)
        return MapNotReady("runtime map dimensions are still changing");

    const uint32_t payload_offset = Read32(directory + 40);
    const uint32_t payload_size = Read32(directory + 44);
    const size_t directory_end = s_directory_offset +
        (size_t)s_map_count * LUFIA2_ROOM_DIRECTORY_SIZE;
    if (payload_offset < directory_end ||
        !RangeIsValid(payload_offset, payload_size, s_strings_offset))
        return InvalidData("map payload is out of bounds");

    const uint8_t *payload = s_data + payload_offset;
    paint->map_width = map_width;
    paint->map_height = map_height;
    paint->payload = payload;
    paint->payload_size = payload_size;
    if (s_version == 6)
        return ReadMapV6(payload, payload_size, paint);
    if (s_version == 5)
        return ReadMapV5(payload, payload_size, paint);
    return ReadMapRooms(directory, payload, payload_size, paint);
}

Lufia2RoomLookupResult Lufia2RoomDataFindMap(
    uint8_t map_id, uint8_t map_width, uint8_t map_height,
    Lufia2MapPaint *paint) {
    if (!paint)
        return InvalidData("map paint output is missing");
    if (s_cached_map.valid && s_cached_map.map_id == map_id &&
        s_cached_map.width == map_width && s_cached_map.height == map_height) {
        *paint = s_cached_map.paint;
        s_last_error = s_cached_map.error;
        return s_cached_map.result;
    }

    const Lufia2RoomLookupResult result = ReadMap(
        map_id, map_width, map_height, paint);
    s_cached_map.valid = true;
    s_cached_map.map_id = map_id;
    s_cached_map.paint = *paint;
    s_cached_map.width = map_width;
    s_cached_map.height = map_height;
    s_cached_map.result = result;
    s_cached_map.error = s_last_error;
    return result;
}

static bool FindPatch(
    const uint8_t *table,
    uint32_t count,
    int32_t cell_x,
    int32_t cell_y,
    int logical_layer,
    uint16_t *block) {
    uint32_t low = 0;
    uint32_t high = count;
    while (low < high) {
        const uint32_t middle = low + (high - low) / 2u;
        const uint8_t *entry = table + (size_t)middle * LUFIA2_ROOM_PATCH_SIZE;
        const int32_t x = (int16_t)Read16(entry);
        const int32_t y = (int16_t)Read16(entry + 2);
        if (y < cell_y || (y == cell_y && x < cell_x)) {
            low = middle + 1;
            continue;
        }
        if (y > cell_y || x > cell_x) {
            high = middle;
            continue;
        }
        const uint16_t value =
            Read16(entry + (logical_layer == 1 ? 4 : 6));
        if (value == LUFIA2_ROOM_PATCH_NONE)
            return false;
        *block = value;
        return true;
    }
    return false;
}

bool Lufia2RoomDataPatchBlock(
    const Lufia2MapPaint *paint,
    int32_t cell_x,
    int32_t cell_y,
    int logical_layer,
    uint16_t *block) {
    if (!paint || !block || (logical_layer != 0 && logical_layer != 1))
        return false;
    const int32_t band = paint->band;
    if (cell_x < -band || cell_y < -band ||
        cell_x >= (int32_t)paint->map_width + band ||
        cell_y >= (int32_t)paint->map_height + band) {
        return false;
    }
    if (paint->paint_rows) {
        const uint32_t row = (uint32_t)(cell_y + band);
        const uint16_t x = (uint16_t)(cell_x + band);
        uint32_t low = paint->paint_rows[row], high = paint->paint_rows[row + 1];
        while (low < high) {
            const uint32_t middle = low + (high - low) / 2u;
            const uint16_t *cell = paint->paint_cells + middle * 3u;
            if (cell[0] < x)
                low = middle + 1;
            else if (cell[0] > x)
                high = middle;
            else {
                *block = cell[logical_layer == 1 ? 1 : 2];
                return *block != LUFIA2_ROOM_PATCH_NONE;
            }
        }
        return false;
    }
    if (!paint->table_count || !paint->payload)
        return false;
    for (uint32_t table = 0; table < paint->table_count; table++) {
        if (FindPatch(paint->payload + paint->patch_offset[table],
                      paint->patch_count[table], cell_x, cell_y,
                      logical_layer, block))
            return true;
    }
    return false;
}

const char *Lufia2RoomLookupResultName(Lufia2RoomLookupResult result) {
    switch (result) {
    case LUFIA2_ROOM_NOT_AUTHORED: return "not authored";
    case LUFIA2_ROOM_FOUND: return "found";
    case LUFIA2_ROOM_MAP_NOT_READY: return "map transition pending";
    case LUFIA2_ROOM_INVALID: return "invalid data";
    default: return "unknown";
    }
}

const char *Lufia2RoomDataLastError(void) {
    return s_last_error;
}

unsigned Lufia2RoomDataVersion(void) {
    EnsureLoaded();
    return s_load_valid ? s_version : 0u;
}
