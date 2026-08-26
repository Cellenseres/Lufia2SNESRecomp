#include "lufia2_map_widescreen.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lufia2_map_load.h"
#include "lufia2_room_data.h"
#include "snes/ws_shadow.h"

enum {
    LUFIA2_BANK_SIZE = 0x10000,
    LUFIA2_MAP_HEADER_SIZE = 10,
    LUFIA2_MAP_LAYER_HEADER_SIZE = 4,
    LUFIA2_BLOCKSET_HEADER_SIZE = 16,
    LUFIA2_TILE_SIZE = 8,
    LUFIA2_BLOCK_SIZE = 16,
    LUFIA2_NATIVE_WIDTH = 256,
    LUFIA2_NATIVE_SAMPLE_X = 16,
    LUFIA2_NATIVE_SAMPLE_Y = 8,
    LUFIA2_NATIVE_MIN_SAMPLES = 64,
    LUFIA2_NATIVE_MATCH_PERCENT = 90,
    LUFIA2_VISIBLE_HEIGHT = 224,
    LUFIA2_SCROLL_TILES = 128,
    LUFIA2_SCROLL_PIXELS = LUFIA2_SCROLL_TILES * LUFIA2_TILE_SIZE,
    LUFIA2_MAX_MAP_CELLS = 16384,
    LUFIA2_MAX_VRAM_BACKUPS = 256,
    LUFIA2_COLLISION_TOP = 0x04,
    LUFIA2_COLLISION_LEFT = 0x08,
    LUFIA2_ROOM_BORDER = 0x10,
    LUFIA2_PLAYER_X = 0x1ddae,
    LUFIA2_PLAYER_Y = 0x1de3e,
    /* The guest streams the native tilemaps in the NMI after the load
       commits, so the first frames of a new map cannot match yet. */
    LUFIA2_SETTLE_FRAMES = 30,
    /* A transition waits for the guest's fade. Only when the screen
       never darkens at all does it apply this many frames after the
       player enters the cell instead. */
    LUFIA2_TRANSITION_GRACE_FRAMES = 10,
    LUFIA2_FULL_BRIGHTNESS = 15,
};

typedef enum Lufia2Direction {
    LUFIA2_DIRECTION_LEFT,
    LUFIA2_DIRECTION_RIGHT,
    LUFIA2_DIRECTION_UP,
    LUFIA2_DIRECTION_DOWN,
} Lufia2Direction;

typedef struct Lufia2RuntimeMap {
    const uint8_t *data;
    const uint8_t *blockset;
    uint32_t cell_count;
    uint16_t block_count;
    uint8_t width;
    uint8_t height;
} Lufia2RuntimeMap;

typedef struct Lufia2VramBackup {
    uint16_t address;
    uint16_t value;
} Lufia2VramBackup;

extern uint8_t g_ram[0x20000];

/* The source this frame draws its margins from. */
static bool s_active;
static uint64_t s_map_signature;
static uint64_t s_rejected_map_signature;
static Lufia2RuntimeMap s_map;
static bool s_room_active;
static Lufia2RoomSelection s_room;

/* Last reported room lookup, so an unchanged result stays quiet. */
static Lufia2RoomLookupResult s_last_room_lookup = LUFIA2_ROOM_NOT_AUTHORED;
static uint8_t s_last_room_lookup_map;
static uint16_t s_last_room_lookup_id = UINT16_MAX;

/* Map readiness: the observed load generation and the frames a new map
   is still allowed to settle in. */
static uint32_t s_map_generation;
static unsigned s_settle_frames;

/* Player movement, and the authored transition it may arm. */
static uint16_t s_player_cell_x;
static uint16_t s_player_cell_y;
static bool s_player_cell_valid;
static Lufia2RoomDirection s_step_direction;
static bool s_step_valid;
static bool s_transition_armed;
static uint16_t s_transition_room_id;
static unsigned s_transition_frames;

/* A room selection that outranks ownership: applied by a transition, or
   restored after a battle or menu tore the state down. */
static bool s_forced_room;
static uint16_t s_forced_room_id;
static bool s_remembered_room;
static uint16_t s_remembered_room_id;
static uint64_t s_remembered_signature;
static uint16_t s_remembered_cell_x;
static uint16_t s_remembered_cell_y;

/* Scratch for the enclosed-area test used by maps without room data. */
static uint8_t s_collision_component[LUFIA2_MAX_MAP_CELLS];
static uint16_t s_component_queue[LUFIA2_MAX_MAP_CELLS];
static uint16_t s_last_collision_cell;
static bool s_last_collision_cell_valid;

/* PPU state borrowed for the duration of one draw and restored after it.
   Every field here has a matching restore in
   Lufia2EndMapRenderOverlay(). */
static bool s_guard_band_opened;
static uint8_t s_guard_band_left;
static uint8_t s_guard_band_right;
static bool s_mosaic_suppressed;
static uint8_t s_saved_mosaic;
static bool s_wrong_mode_reported;
static Lufia2VramBackup s_vram_backups[LUFIA2_MAX_VRAM_BACKUPS];
static unsigned s_vram_backup_count;
static uint8_t s_vram_backed_up[0x8000 / 8];

static uint16_t Read16(const uint8_t *data) {
    return (uint16_t)(data[0] | ((uint16_t)data[1] << 8));
}

static bool ReadRuntimeMap(Lufia2RuntimeMap *map) {
    const uint8_t *bank = g_ram + LUFIA2_BANK_SIZE;
    if (bank[0] != 0x02)
        return false;

    const uint8_t width = bank[8];
    const uint8_t height = bank[9];
    if (!width || !height)
        return false;

    const uint32_t cell_count = (uint32_t)width * height;
    const uint32_t map_size =
        LUFIA2_MAP_HEADER_SIZE + LUFIA2_MAP_LAYER_HEADER_SIZE +
        cell_count * 4;
    if (cell_count > LUFIA2_MAX_MAP_CELLS ||
        map_size + LUFIA2_BLOCKSET_HEADER_SIZE > LUFIA2_BANK_SIZE) {
        return false;
    }

    const uint8_t *blockset = bank + map_size;
    if (blockset[0] != 'M' || blockset[1] != 'C')
        return false;

    const uint16_t block_count = Read16(blockset + 2);
    const uint32_t blockset_size =
        LUFIA2_BLOCKSET_HEADER_SIZE + (uint32_t)block_count * 8;
    if (!block_count || map_size + blockset_size > LUFIA2_BANK_SIZE)
        return false;

    map->data = bank;
    map->blockset = blockset;
    map->cell_count = cell_count;
    map->block_count = block_count;
    map->width = width;
    map->height = height;
    return true;
}

static uint64_t MapSignature(const Lufia2RuntimeMap *map) {
    return (uint64_t)g_ram[0x05ac] |
        ((uint64_t)map->width << 8) |
        ((uint64_t)map->height << 16) |
        ((uint64_t)map->block_count << 24);
}

static const uint8_t *MapEntry(
    const Lufia2RuntimeMap *map,
    int logical_layer,
    uint32_t cell) {
    uint32_t offset = LUFIA2_MAP_HEADER_SIZE;
    if (logical_layer != 0)
        offset += map->cell_count * 2 + LUFIA2_MAP_LAYER_HEADER_SIZE;
    return map->data + offset + cell * 2;
}

static uint16_t MapBlockIndex(
    const Lufia2RuntimeMap *map,
    int logical_layer,
    uint32_t cell) {
    const uint8_t *entry = MapEntry(map, logical_layer, cell);
    return (uint16_t)(entry[0] | ((entry[1] & 3u) << 8));
}

static bool ResolveCellTile(
    const Lufia2RuntimeMap *map,
    int logical_layer,
    uint32_t cell,
    uint32_t sub_tile_x,
    uint32_t sub_tile_y,
    uint16_t *entry) {
    if (cell >= map->cell_count)
        return false;

    const uint16_t block = MapBlockIndex(map, logical_layer, cell);
    if (block >= map->block_count)
        return false;

    /* Block tiles are stored in column-major order. */
    const uint32_t sub_tile = (sub_tile_x << 1) | sub_tile_y;
    const uint8_t *tile = map->blockset + LUFIA2_BLOCKSET_HEADER_SIZE +
        block * 8 + sub_tile * 2;
    *entry = Read16(tile);
    return true;
}

static bool ResolveBlockTile(
    const Lufia2RuntimeMap *map,
    uint16_t block,
    uint32_t sub_tile_x,
    uint32_t sub_tile_y,
    uint16_t *entry) {
    if (block >= map->block_count)
        return false;
    const uint32_t sub_tile = (sub_tile_x << 1) | sub_tile_y;
    const uint8_t *tile = map->blockset + LUFIA2_BLOCKSET_HEADER_SIZE +
        block * 8 + sub_tile * 2;
    *entry = Read16(tile);
    return true;
}

static bool ResolveMapTile(
    const Lufia2RuntimeMap *map,
    int logical_layer,
    uint32_t world_tile_x,
    uint32_t world_tile_y,
    uint16_t *entry) {
    const uint32_t tile_x = world_tile_x & (LUFIA2_SCROLL_TILES - 1u);
    const uint32_t tile_y = world_tile_y & (LUFIA2_SCROLL_TILES - 1u);
    const uint32_t block_x = tile_x >> 1;
    const uint32_t block_y = tile_y >> 1;
    if (block_x >= map->width || block_y >= map->height)
        return false;

    return ResolveCellTile(
        map, logical_layer, block_y * map->width + block_x,
        tile_x & 1u, tile_y & 1u, entry);
}

static bool RoomOwnsWorldTile(
    const Lufia2RuntimeMap *map,
    const Lufia2RoomSelection *room,
    uint32_t world_tile_x,
    uint32_t world_tile_y) {
    const uint32_t tile_x = world_tile_x & (LUFIA2_SCROLL_TILES - 1u);
    const uint32_t tile_y = world_tile_y & (LUFIA2_SCROLL_TILES - 1u);
    const uint32_t block_x = tile_x >> 1;
    const uint32_t block_y = tile_y >> 1;
    if (block_x >= map->width || block_y >= map->height)
        return false;
    return Lufia2RoomDataCellIsVisible(
        room, block_y * map->width + block_x);
}

static uint16_t ResolveWideTile(
    const Lufia2RuntimeMap *map,
    const Lufia2RoomSelection *room,
    int logical_layer,
    uint32_t world_tile_x,
    uint32_t world_tile_y) {
    uint16_t entry = 0;
    const uint32_t tile_x = world_tile_x & (LUFIA2_SCROLL_TILES - 1u);
    const uint32_t tile_y = world_tile_y & (LUFIA2_SCROLL_TILES - 1u);
    if (!room || RoomOwnsWorldTile(map, room, world_tile_x, world_tile_y)) {
        (void)ResolveMapTile(
            map, logical_layer, world_tile_x, world_tile_y, &entry);
        return entry;
    }

    const Lufia2RoomVoid *definition =
        logical_layer == 1 ? &room->bg1 : &room->bg2;
    if (definition->mode == LUFIA2_ROOM_VOID_SOURCE_CELL) {
        const uint32_t cell =
            (uint32_t)definition->y * map->width + definition->x;
        (void)ResolveCellTile(
            map, logical_layer, cell, tile_x & 1u, tile_y & 1u, &entry);
    } else if (definition->mode == LUFIA2_ROOM_VOID_BLOCK) {
        (void)ResolveBlockTile(
            map, definition->block, tile_x & 1u, tile_y & 1u, &entry);
    }
    return entry;
}

/* Sample the streamed tilemaps against the processed map. This is the gate
   for maps whose widescreen extent is inferred rather than authored: without
   room metadata the wider view is only safe once the map in bank $7F
   demonstrably explains what the PPU is displaying. */
static bool NativeViewportMatches(
    const Ppu *ppu,
    const Lufia2RuntimeMap *map) {
    const uint32_t world_tile_x0 = (uint32_t)ppu->hScroll[0] >> 3;
    const uint32_t world_tile_y0 = (uint32_t)ppu->vScroll[0] >> 3;
    unsigned checked = 0;
    unsigned matched = 0;

    for (int bg = 0; bg < 2; bg++) {
        const int logical_layer = bg == 0 ? 1 : 0;
        const uint16_t map_base = (uint16_t)PPU_bgTilemapAdr(ppu, bg);

        for (uint32_t dy = 0; dy < LUFIA2_NATIVE_SAMPLE_Y; dy++) {
            for (uint32_t dx = 0; dx < LUFIA2_NATIVE_SAMPLE_X; dx++) {
                const uint32_t tile_x = world_tile_x0 + dx;
                const uint32_t tile_y = world_tile_y0 + dy;
                uint16_t expected = 0;
                if (!ResolveMapTile(
                        map, logical_layer, tile_x, tile_y, &expected)) {
                    continue;
                }

                const uint16_t address = (uint16_t)(
                    map_base + ((tile_y & 31u) << 5) + (tile_x & 31u));
                checked++;
                if (ppu->vram[address & 0x7fffu] == expected)
                    matched++;
            }
        }
    }

    return checked >= LUFIA2_NATIVE_MIN_SAMPLES &&
        matched * 100u >= checked * LUFIA2_NATIVE_MATCH_PERCENT;
}

static bool HasRoomBorders(const Lufia2RuntimeMap *map) {
    for (uint32_t cell = 0; cell < map->cell_count; cell++) {
        if (MapEntry(map, 1, cell)[1] & LUFIA2_ROOM_BORDER)
            return true;
    }
    return false;
}

static bool CollisionAllowsStep(
    const Lufia2RuntimeMap *map,
    uint16_t cell,
    uint16_t neighbor,
    Lufia2Direction direction) {
    const uint8_t cell_flags = MapEntry(map, 1, cell)[1];
    const uint8_t neighbor_flags = MapEntry(map, 1, neighbor)[1];

    switch (direction) {
    case LUFIA2_DIRECTION_LEFT:
        return (cell_flags & LUFIA2_COLLISION_LEFT) == 0;
    case LUFIA2_DIRECTION_RIGHT:
        return (neighbor_flags & LUFIA2_COLLISION_LEFT) == 0;
    case LUFIA2_DIRECTION_UP:
        return (cell_flags & LUFIA2_COLLISION_TOP) == 0;
    case LUFIA2_DIRECTION_DOWN:
        return (neighbor_flags & LUFIA2_COLLISION_TOP) == 0;
    default:
        return false;
    }
}

static bool CurrentAreaIsEnclosed(
    const Ppu *ppu,
    const Lufia2RuntimeMap *map) {
    static const int kNeighborX[] = {-1, 1, 0, 0};
    static const int kNeighborY[] = {0, 0, -1, 1};

    const uint32_t center_x =
        (((uint32_t)ppu->hScroll[0] + LUFIA2_NATIVE_WIDTH / 2u) %
         LUFIA2_SCROLL_PIXELS) / LUFIA2_BLOCK_SIZE;
    const uint32_t center_y =
        (((uint32_t)ppu->vScroll[0] + LUFIA2_VISIBLE_HEIGHT / 2u) %
         LUFIA2_SCROLL_PIXELS) / LUFIA2_BLOCK_SIZE;
    if (center_x >= map->width || center_y >= map->height)
        return false;

    const uint32_t center = center_y * map->width + center_x;
    if (s_last_collision_cell_valid && center == s_last_collision_cell)
        return false;

    memset(s_collision_component, 0, map->cell_count);
    s_last_collision_cell = (uint16_t)center;
    s_last_collision_cell_valid = true;

    unsigned head = 0;
    unsigned tail = 0;
    bool touches_edge = false;
    int min_x = (int)center_x;
    int max_x = (int)center_x;
    int min_y = (int)center_y;
    int max_y = (int)center_y;

    s_collision_component[center] = 1;
    s_component_queue[tail++] = (uint16_t)center;

    while (head < tail) {
        const uint16_t cell = s_component_queue[head++];
        const int x = cell % map->width;
        const int y = cell / map->width;

        if (x < min_x) min_x = x;
        if (x > max_x) max_x = x;
        if (y < min_y) min_y = y;
        if (y > max_y) max_y = y;
        if (x == 0 || y == 0 ||
            x == map->width - 1 || y == map->height - 1) {
            touches_edge = true;
        }

        for (int direction = LUFIA2_DIRECTION_LEFT;
             direction <= LUFIA2_DIRECTION_DOWN;
             direction++) {
            const int next_x = x + kNeighborX[direction];
            const int next_y = y + kNeighborY[direction];
            if (next_x < 0 || next_y < 0 ||
                next_x >= map->width || next_y >= map->height) {
                continue;
            }

            const uint16_t neighbor =
                (uint16_t)(next_y * map->width + next_x);
            if (s_collision_component[neighbor] ||
                !CollisionAllowsStep(
                    map, cell, neighbor, (Lufia2Direction)direction)) {
                continue;
            }

            s_collision_component[neighbor] = 1;
            s_component_queue[tail++] = neighbor;
        }
    }

    const bool enclosed =
        !touches_edge && tail >= 4 && tail * 2u < map->cell_count;
    if (enclosed) {
        fprintf(stderr,
            "[video] enclosed map area: %d,%d..%d,%d (%u blocks)\n",
            min_x, min_y, max_x, max_y, tail);
    }
    return enclosed;
}

static void FillWideViewport(
    const Lufia2RuntimeMap *map,
    const Lufia2RoomSelection *room,
    int margin_pixels,
    uint32_t world_x,
    uint32_t world_y) {
    const uint32_t reach = (uint32_t)margin_pixels + LUFIA2_TILE_SIZE;
    const uint32_t left = world_x > reach ? world_x - reach : 0;
    const uint32_t right = world_x + LUFIA2_NATIVE_WIDTH + reach;
    const uint32_t bottom = world_y + LUFIA2_VISIBLE_HEIGHT + LUFIA2_TILE_SIZE;
    const uint32_t tile_x0 = left >> 3;
    const uint32_t tile_x1 = (right + 7u) >> 3;
    const uint32_t tile_y0 = world_y >> 3;
    const uint32_t tile_y1 = (bottom + 7u) >> 3;

    for (int bg = 0; bg < 2; bg++) {
        const int logical_layer = bg == 0 ? 1 : 0;
        for (uint32_t tile_y = tile_y0; tile_y < tile_y1; tile_y++) {
            for (uint32_t tile_x = tile_x0; tile_x < tile_x1; tile_x++) {
                WsShadowForceTile(
                    bg, tile_x, tile_y,
                    ResolveWideTile(
                        map, room, logical_layer, tile_x, tile_y));
            }
        }
    }
}

/* Report one more frame of the load while the new map settles. */
static Lufia2MapWidescreenResult NotReadyYet(void) {
    if (!s_settle_frames)
        return LUFIA2_MAP_WIDESCREEN_DISABLED;
    s_settle_frames--;
    return LUFIA2_MAP_WIDESCREEN_LOADING;
}

void Lufia2DeactivateMapWidescreen(void) {
    /* Battles, menus and scene changes tear this state down. Record what was
       selected so returning to the same map cell restores the same room:
       ownership alone may name a different one at a doorway or shared cell,
       and the player cannot have moved in the meantime. */
    if (s_active && s_room_active) {
        s_remembered_room = true;
        s_remembered_room_id = s_room.room_id;
        s_remembered_signature = s_map_signature;
        s_remembered_cell_x = s_player_cell_x;
        s_remembered_cell_y = s_player_cell_y;
    }
    if (s_active)
        WsShadowReset();
    s_active = false;
    s_map_signature = 0;
    s_map.data = NULL;
    s_room_active = false;
    s_last_collision_cell_valid = false;
}

static bool VramIsBackedUp(uint16_t address) {
    return (s_vram_backed_up[address >> 3] >> (address & 7)) & 1u;
}

static void ForgetVramBackups(void) {
    for (unsigned i = 0; i < s_vram_backup_count; i++) {
        const uint16_t address = s_vram_backups[i].address;
        s_vram_backed_up[address >> 3] &= (uint8_t)~(1u << (address & 7));
    }
    s_vram_backup_count = 0;
}

static void OverlayTile(Ppu *ppu, uint16_t address, uint16_t value) {
    address &= 0x7fff;
    if (!VramIsBackedUp(address)) {
        if (s_vram_backup_count >= LUFIA2_MAX_VRAM_BACKUPS)
            return;
        s_vram_backups[s_vram_backup_count].address = address;
        s_vram_backups[s_vram_backup_count].value = ppu->vram[address];
        s_vram_backup_count++;
        s_vram_backed_up[address >> 3] |= (uint8_t)(1u << (address & 7));
    }
    ppu->vram[address] = value;
}

static void OverlayScreenBand(
    Ppu *ppu,
    const Lufia2RuntimeMap *map,
    int bg,
    int x0,
    int x1) {
    if (x0 >= x1)
        return;

    const int logical_layer = bg == 0 ? 1 : 0;
    const uint32_t world_x0 = (uint32_t)ppu->hScroll[bg] + (uint32_t)x0;
    const uint32_t world_x1 =
        (uint32_t)ppu->hScroll[bg] + (uint32_t)(x1 - 1);
    const uint32_t tile_x0 = world_x0 >> 3;
    const uint32_t tile_x1 = world_x1 >> 3;
    const uint32_t tile_y0 = (uint32_t)ppu->vScroll[bg] >> 3;
    const uint32_t tile_y1 =
        ((uint32_t)ppu->vScroll[bg] + LUFIA2_VISIBLE_HEIGHT - 1u) >> 3;
    const uint16_t map_base = (uint16_t)PPU_bgTilemapAdr(ppu, bg);

    for (uint32_t tile_y = tile_y0; tile_y <= tile_y1; tile_y++) {
        for (uint32_t tile_x = tile_x0; tile_x <= tile_x1; tile_x++) {
            const uint16_t address = (uint16_t)(
                map_base + ((tile_y & 31u) << 5) + (tile_x & 31u));
            OverlayTile(
                ppu, address,
                ResolveWideTile(
                    map, s_room_active ? &s_room : NULL,
                    logical_layer, tile_x, tile_y));
        }
    }
}

/* The renderer aligns mosaic blocks from a table indexed by screen column and
   clamps a widescreen margin column to entry 0, so the first block in a margin
   becomes as wide as the margin itself - one flat colour per scanline. At size
   1 the mosaic is a no-op on the authentic 256 columns, yet the enable bits
   alone still take that path, and Lufia leaves them set after a battle. Drop a
   size-1 mosaic for the draw; a real mosaic is left untouched. */
static void SuppressNoOpMosaic(Ppu *ppu) {
    s_mosaic_suppressed = false;
    if (!ppu || !ppu->mosaic || PPU_mosaicSize(ppu) != 1)
        return;

    s_saved_mosaic = ppu->mosaic;
    ppu->mosaic = 0;
    s_mosaic_suppressed = true;
}

void Lufia2BeginMapRenderOverlay(Ppu *ppu) {
    ForgetVramBackups();
    s_guard_band_opened = false;
    SuppressNoOpMosaic(ppu);
    if (!s_active || !ppu || !s_map.data)
        return;

    /* This writes tilemap entries straight into VRAM. Mode 7 keeps its map
       and its tile data in that same VRAM, so an overlay there is not a
       tilemap edit but picture corruption - one flat colour band per
       scanline. Only a Mode 1 regular map may be overlaid. */
    if (PPU_mode(ppu) != 1) {
        if (!s_wrong_mode_reported) {
            s_wrong_mode_reported = true;
            fprintf(stderr,
                "[video] map overlay suppressed: widescreen is active while "
                "the PPU is in mode %u\n",
                (unsigned)PPU_mode(ppu));
        }
        return;
    }

    bool guard_band = false;
    for (int bg = 0; bg < 2; bg++) {
        const uint32_t flags = ppu->windowsel >> (bg * 4);
        if ((flags & 3u) != 3u ||
            ppu->window1left > 16 || ppu->window1right < 239 ||
            ppu->window1left > ppu->window1right) {
            continue;
        }

        /* Restore the map under Lufia's streaming guard bands for this draw. */
        OverlayScreenBand(ppu, &s_map, bg, 0, ppu->window1left);
        OverlayScreenBand(
            ppu, &s_map, bg, (int)ppu->window1right + 1,
            LUFIA2_NATIVE_WIDTH);
        guard_band = true;
    }

    if (!guard_band)
        return;

    /* The widescreen window expansion shifts a game-authored edge outward by
       the margin instead of opening it to the frame edge, so a guard band at
       column 16/239 still clips the outermost 16 margin pixels to black. The
       band's content has just been restored above, so the band can be opened
       for this draw and put back in Lufia2EndMapRenderOverlay(). */
    s_guard_band_left = ppu->window1left;
    s_guard_band_right = ppu->window1right;
    ppu->window1left = 0;
    ppu->window1right = LUFIA2_NATIVE_WIDTH - 1;
    s_guard_band_opened = true;
}

void Lufia2EndMapRenderOverlay(Ppu *ppu) {
    if (ppu && s_mosaic_suppressed)
        ppu->mosaic = s_saved_mosaic;
    s_mosaic_suppressed = false;
    if (ppu && s_guard_band_opened) {
        ppu->window1left = s_guard_band_left;
        ppu->window1right = s_guard_band_right;
    }
    s_guard_band_opened = false;
    if (ppu) {
        for (unsigned i = 0; i < s_vram_backup_count; i++) {
            ppu->vram[s_vram_backups[i].address] =
                s_vram_backups[i].value;
        }
    }
    ForgetVramBackups();
}

static Lufia2MapWidescreenResult RejectMap(
    uint64_t signature,
    const char *reason) {
    fprintf(stderr,
        "[video] widescreen disabled for indoor map $%02X (%s)\n",
        (unsigned)g_ram[0x05ac], reason);
    Lufia2DeactivateMapWidescreen();
    s_rejected_map_signature = signature;
    return LUFIA2_MAP_WIDESCREEN_DISABLED;
}

static void CurrentPlayerPosition(
    const Ppu *ppu,
    const Lufia2RuntimeMap *map,
    uint16_t *player_x,
    uint16_t *player_y) {
    uint16_t x = Read16(g_ram + LUFIA2_PLAYER_X);
    uint16_t y = Read16(g_ram + LUFIA2_PLAYER_Y);
    if (x / LUFIA2_BLOCK_SIZE >= map->width ||
        y / LUFIA2_BLOCK_SIZE >= map->height) {
        x = (uint16_t)(ppu->hScroll[0] + LUFIA2_NATIVE_WIDTH / 2u);
        y = (uint16_t)(ppu->vScroll[0] + LUFIA2_VISIBLE_HEIGHT / 2u);
    }
    *player_x = x;
    *player_y = y;
}

static void ReportRoomLookup(
    uint8_t map_id,
    uint8_t map_width,
    uint32_t player_cell,
    Lufia2RoomLookupResult lookup,
    const Lufia2RoomSelection *room) {
    const uint16_t room_id = room ? room->room_id : UINT16_MAX;
    if (lookup == s_last_room_lookup &&
        map_id == s_last_room_lookup_map &&
        room_id == s_last_room_lookup_id) {
        return;
    }
    s_last_room_lookup = lookup;
    s_last_room_lookup_map = map_id;
    s_last_room_lookup_id = room_id;

    if (lookup == LUFIA2_ROOM_FOUND) {
        fprintf(stderr,
            "[room-data] map $%02X cell %u,%u -> room %u (%s)\n",
            (unsigned)map_id,
            (unsigned)(player_cell % room->map_width),
            (unsigned)(player_cell / room->map_width),
            (unsigned)room->room_id,
            room->name && *room->name ? room->name : "unnamed");
    } else if (lookup != LUFIA2_ROOM_NOT_AUTHORED) {
        const char *detail =
            (lookup == LUFIA2_ROOM_INVALID ||
             lookup == LUFIA2_ROOM_MAP_NOT_READY)
            ? Lufia2RoomDataLastError() : NULL;
        const char *action = "using centered 4:3";
        if (detail && *detail) {
            fprintf(stderr,
                "[room-data] map $%02X cell %u,%u -> %s (%s); "
                "%s\n",
                (unsigned)map_id,
                (unsigned)(player_cell % map_width),
                (unsigned)(player_cell / map_width),
                Lufia2RoomLookupResultName(lookup), detail, action);
        } else {
            fprintf(stderr,
                "[room-data] map $%02X cell %u,%u -> %s; "
                "%s\n",
                (unsigned)map_id,
                (unsigned)(player_cell % map_width),
                (unsigned)(player_cell / map_width),
                Lufia2RoomLookupResultName(lookup), action);
        }
    }
}

/* Only a single-cell walk names a direction. A door teleports the player, so
   the direction that matters is the one they were walking before it. */
static void TrackPlayerStep(uint16_t cell_x, uint16_t cell_y) {
    if (s_player_cell_valid &&
        (cell_x != s_player_cell_x || cell_y != s_player_cell_y)) {
        const int dx = (int)cell_x - (int)s_player_cell_x;
        const int dy = (int)cell_y - (int)s_player_cell_y;
        if (dx == 0 && (dy == 1 || dy == -1)) {
            s_step_direction = dy > 0 ? LUFIA2_ROOM_DOWN : LUFIA2_ROOM_UP;
            s_step_valid = true;
        } else if (dy == 0 && (dx == 1 || dx == -1)) {
            s_step_direction = dx > 0 ? LUFIA2_ROOM_RIGHT : LUFIA2_ROOM_LEFT;
            s_step_valid = true;
        }
    }
    s_player_cell_x = cell_x;
    s_player_cell_y = cell_y;
    s_player_cell_valid = true;
}

/* Lufia fades a room change through the INIDISP master brightness rather
   than forced blank, so the instant between fade-out and fade-in is the one
   where nothing reaches the screen. */
static bool ScreenIsBlack(const Ppu *ppu) {
    return PPU_forcedBlank(ppu) || PPU_brightness(ppu) == 0;
}

static bool ScreenIsFading(const Ppu *ppu) {
    return !PPU_forcedBlank(ppu) &&
        PPU_brightness(ppu) < LUFIA2_FULL_BRIGHTNESS;
}

/* An authored transition arms while the player stands on its cell moving in
   its direction, and applies once the screen is black. A fade in progress
   never times out; the grace period only covers a room change the guest
   performs without darkening the screen at all. */
static void UpdateRoomTransition(
    const Ppu *ppu,
    uint8_t map_id,
    const Lufia2RuntimeMap *map,
    uint16_t cell_x,
    uint16_t cell_y) {
    TrackPlayerStep(cell_x, cell_y);

    uint16_t target = 0;
    if (!s_step_valid ||
        !Lufia2RoomDataFindTransition(
            map_id, map->width, map->height, cell_x, cell_y,
            s_step_direction, &target)) {
        s_transition_armed = false;
        return;
    }

    if (!s_transition_armed || target != s_transition_room_id) {
        s_transition_armed = true;
        s_transition_room_id = target;
        s_transition_frames = 0;
    }

    if (!ScreenIsBlack(ppu)) {
        if (ScreenIsFading(ppu)) {
            /* The fade owns the timing from here on. */
            s_transition_frames = 0;
            return;
        }
        if (++s_transition_frames <= LUFIA2_TRANSITION_GRACE_FRAMES)
            return;
    }

    s_transition_armed = false;
    if (s_forced_room && s_forced_room_id == target)
        return;
    s_forced_room = true;
    s_forced_room_id = target;
    fprintf(stderr,
        "[room-data] transition at cell %u,%u %s -> room %u\n",
        (unsigned)cell_x, (unsigned)cell_y,
        Lufia2RoomDirectionName(s_step_direction), (unsigned)target);
}

/* Point the shadow at this frame's source and camera. */
static Lufia2MapWidescreenResult AdoptSource(
    Ppu *ppu,
    const Lufia2RuntimeMap *map,
    const Lufia2RoomSelection *room) {
    if (map != &s_map)
        s_map = *map;
    s_room_active = room != NULL;
    if (room && room != &s_room)
        s_room = *room;
    s_settle_frames = 0;

    const uint32_t world_x = (uint32_t)ppu->hScroll[0];
    const uint32_t world_y = (uint32_t)ppu->vScroll[0];
    for (int bg = 0; bg < 2; bg++) {
        WsShadowSetWorld(bg, world_x, world_y);
        WsShadowSetScroll(
            bg, (uint32_t)ppu->hScroll[bg], (uint32_t)ppu->vScroll[bg]);
        WsShadowSetBlankTile(bg, 0);
    }
    return LUFIA2_MAP_WIDESCREEN_ACTIVE;
}

/* An authored map is described by the room file, not inferred from the
   screen. It stays in widescreen for as long as the player is inside it:
   crossing between Visible Areas only changes which area is drawn, and a
   cell no area owns keeps the room the player came from. */
static Lufia2MapWidescreenResult PrepareAuthoredMap(
    Ppu *ppu,
    const Lufia2RuntimeMap *map,
    uint64_t signature,
    uint8_t map_id,
    Lufia2RoomLookupResult lookup,
    const Lufia2RoomSelection *room) {
    s_rejected_map_signature = 0;

    if (s_remembered_room) {
        const bool same_place = signature == s_remembered_signature &&
            s_player_cell_valid &&
            s_player_cell_x == s_remembered_cell_x &&
            s_player_cell_y == s_remembered_cell_y;
        s_remembered_room = false;
        if (same_place && !s_forced_room) {
            s_forced_room = true;
            s_forced_room_id = s_remembered_room_id;
        }
    }

    /* An applied transition outranks ownership until ownership agrees with
       it again, which is exactly when the player has left the doorway. */
    Lufia2RoomSelection forced;
    if (s_forced_room) {
        if (lookup == LUFIA2_ROOM_FOUND &&
            room->room_id == s_forced_room_id) {
            s_forced_room = false;
        } else if (Lufia2RoomDataSelectRoomById(
                map_id, map->width, map->height, s_forced_room_id, &forced)) {
            room = &forced;
            lookup = LUFIA2_ROOM_FOUND;
        } else {
            s_forced_room = false;
        }
    }

    if (lookup == LUFIA2_ROOM_MAP_NOT_READY) {
        if (s_active && s_room_active && signature == s_map_signature)
            return AdoptSource(ppu, map, &s_room);
        Lufia2DeactivateMapWidescreen();
        return NotReadyYet();
    }

    if (lookup != LUFIA2_ROOM_FOUND) {
        /* No area owns this cell. On an authored map that is the open part
           of a town, not an unsupported scene: the author declared the map
           and only needed areas where rooms have to be separated. Show the
           complete processed map there, the way a map with no room data is
           shown, instead of dropping the whole map to 4:3. */
        if (!s_active || signature != s_map_signature || s_room_active) {
            WsShadowReset();
            s_active = true;
            s_map_signature = signature;
            s_last_collision_cell_valid = false;
            fprintf(stderr,
                "[video] authored map $%02X: no room owns the player, "
                "using the full map\n",
                (unsigned)map_id);
        }
        return AdoptSource(ppu, map, NULL);
    }

    if (!s_active || signature != s_map_signature ||
        !s_room_active || room->room_id != s_room.room_id) {
        WsShadowReset();
        s_active = true;
        s_map_signature = signature;
        s_last_collision_cell_valid = false;
        fprintf(stderr,
            "[video] authored room source: map $%02X, room %u (%s)\n",
            (unsigned)map_id, (unsigned)room->room_id,
            room->name && *room->name ? room->name : "unnamed");
    }
    return AdoptSource(ppu, map, room);
}

/* A map without authored rooms is inferred, so it keeps the conservative
   checks: the processed map must explain what the PPU displays, and an
   enclosed indoor area must not be widened. */
static Lufia2MapWidescreenResult PrepareInferredMap(
    Ppu *ppu,
    const Lufia2RuntimeMap *map,
    uint64_t signature) {
    if (signature == s_rejected_map_signature)
        return LUFIA2_MAP_WIDESCREEN_DISABLED;
    s_rejected_map_signature = 0;

    if (!s_active || signature != s_map_signature) {
        if (!NativeViewportMatches(ppu, map)) {
            Lufia2DeactivateMapWidescreen();
            return NotReadyYet();
        }
        if (HasRoomBorders(map))
            return RejectMap(signature, "room borders");
        if (CurrentAreaIsEnclosed(ppu, map))
            return RejectMap(signature, "closed area");

        WsShadowReset();
        s_active = true;
        s_map_signature = signature;
        s_last_collision_cell_valid = false;
        fprintf(stderr,
            "[video] full map source: %ux%u blocks, %u block definitions\n",
            (unsigned)map->width, (unsigned)map->height,
            (unsigned)map->block_count);
    } else if (CurrentAreaIsEnclosed(ppu, map)) {
        return RejectMap(signature, "closed area");
    }

    return AdoptSource(ppu, map, NULL);
}

Lufia2MapWidescreenResult Lufia2PrepareMapWidescreen(
    Ppu *ppu,
    int margin_pixels) {
    const uint32_t generation = Lufia2MapLoadGeneration();
    if (generation != s_map_generation) {
        /* A completed load replaced bank $7F; nothing cached from the
           previous map is still valid. */
        s_map_generation = generation;
        s_rejected_map_signature = 0;
        s_settle_frames = LUFIA2_SETTLE_FRAMES;
        s_player_cell_valid = false;
        s_step_valid = false;
        s_transition_armed = false;
        s_forced_room = false;
        Lufia2DeactivateMapWidescreen();
    }

    if (Lufia2MapLoadInProgress()) {
        /* The guest is rewriting bank $7F. */
        s_settle_frames = LUFIA2_SETTLE_FRAMES;
        Lufia2DeactivateMapWidescreen();
        return LUFIA2_MAP_WIDESCREEN_LOADING;
    }

    if (!ppu || margin_pixels <= 0) {
        Lufia2DeactivateMapWidescreen();
        return LUFIA2_MAP_WIDESCREEN_DISABLED;
    }

    /* $00 is the overworld, which renders from Mode 7 and never loads a
       processed map. Evaluating it as a regular map judges whatever the
       previous map left in bank $7F, and a rejection recorded under this id
       then sticks. */
    if (g_ram[0x05ac] == 0) {
        Lufia2DeactivateMapWidescreen();
        return LUFIA2_MAP_WIDESCREEN_DISABLED;
    }

    Lufia2RuntimeMap map;
    if (!ReadRuntimeMap(&map)) {
        Lufia2DeactivateMapWidescreen();
        return NotReadyYet();
    }

    uint16_t player_x;
    uint16_t player_y;
    CurrentPlayerPosition(ppu, &map, &player_x, &player_y);
    const uint8_t runtime_map_id = g_ram[0x05ac];
    const uint8_t map_id = runtime_map_id ? (uint8_t)(runtime_map_id - 1) : 0;
    UpdateRoomTransition(
        ppu, map_id, &map,
        (uint16_t)(player_x / LUFIA2_BLOCK_SIZE),
        (uint16_t)(player_y / LUFIA2_BLOCK_SIZE));

    Lufia2RoomSelection room;
    const Lufia2RoomLookupResult room_lookup = Lufia2RoomDataFindOwner(
        map_id, map.width, map.height, player_x, player_y,
        s_room_active ? s_room.room_id : LUFIA2_ROOM_ID_NONE, &room);
    ReportRoomLookup(
        map_id, map.width,
        (uint32_t)(player_y / LUFIA2_BLOCK_SIZE) * map.width +
            player_x / LUFIA2_BLOCK_SIZE,
        room_lookup, room_lookup == LUFIA2_ROOM_FOUND ? &room : NULL);

    if (room_lookup == LUFIA2_ROOM_INVALID) {
        Lufia2DeactivateMapWidescreen();
        return LUFIA2_MAP_WIDESCREEN_DISABLED;
    }

    const uint64_t signature = MapSignature(&map);
    if (room_lookup == LUFIA2_ROOM_NOT_AUTHORED)
        return PrepareInferredMap(ppu, &map, signature);
    return PrepareAuthoredMap(
        ppu, &map, signature, map_id, room_lookup, &room);
}

bool Lufia2MapWidescreenIsActive(void) {
    Lufia2RuntimeMap map;
    return s_active && s_map.data && ReadRuntimeMap(&map) &&
        MapSignature(&map) == s_map_signature;
}

void Lufia2FinalizeMapWidescreen(Ppu *ppu, int margin_pixels) {
    if (!s_active || !ppu || margin_pixels <= 0 || !s_map.data)
        return;

    FillWideViewport(
        &s_map, s_room_active ? &s_room : NULL, margin_pixels,
        (uint32_t)ppu->hScroll[0],
        (uint32_t)ppu->vScroll[0]);
}

bool Lufia2MapWidescreenWorldPointIsVisible(uint16_t x, uint16_t y) {
    if (!s_active || !s_room_active || !s_map.data)
        return true;
    const uint32_t cell_x = x / LUFIA2_BLOCK_SIZE;
    const uint32_t cell_y = y / LUFIA2_BLOCK_SIZE;
    if (cell_x >= s_map.width || cell_y >= s_map.height)
        return false;
    return Lufia2RoomDataCellIsVisible(
        &s_room, cell_y * s_map.width + cell_x);
}
