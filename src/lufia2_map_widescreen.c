#include "lufia2_map_widescreen.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "snes/ws_shadow.h"

enum {
    LUFIA2_BANK_SIZE = 0x10000,
    LUFIA2_MAP_HEADER_SIZE = 10,
    LUFIA2_MAP_LAYER_HEADER_SIZE = 4,
    LUFIA2_BLOCKSET_HEADER_SIZE = 16,
    LUFIA2_TILE_SIZE = 8,
    LUFIA2_BLOCK_SIZE = 16,
    LUFIA2_NATIVE_WIDTH = 256,
    LUFIA2_VISIBLE_HEIGHT = 224,
    LUFIA2_SCROLL_TILES = 128,
    LUFIA2_SCROLL_PIXELS = LUFIA2_SCROLL_TILES * LUFIA2_TILE_SIZE,
    LUFIA2_MAX_MAP_CELLS = 16384,
    LUFIA2_MAX_VRAM_BACKUPS = 256,
    LUFIA2_COLLISION_TOP = 0x04,
    LUFIA2_COLLISION_LEFT = 0x08,
    LUFIA2_ROOM_BORDER = 0x10,
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

static bool s_active;
static uint64_t s_map_signature;
static uint64_t s_rejected_map_signature;
static Lufia2RuntimeMap s_map;

static uint8_t s_collision_component[LUFIA2_MAX_MAP_CELLS];
static uint16_t s_component_queue[LUFIA2_MAX_MAP_CELLS];
static uint16_t s_last_collision_cell;
static bool s_last_collision_cell_valid;

static Lufia2VramBackup s_vram_backups[LUFIA2_MAX_VRAM_BACKUPS];
static unsigned s_vram_backup_count;

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

static uint16_t ResolveWideTile(
    const Lufia2RuntimeMap *map,
    int logical_layer,
    uint32_t world_tile_x,
    uint32_t world_tile_y) {
    uint16_t entry = 0;
    (void)ResolveMapTile(
        map, logical_layer, world_tile_x, world_tile_y, &entry);
    return entry;
}

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

        for (uint32_t dy = 0; dy < 8; dy++) {
            for (uint32_t dx = 0; dx < 16; dx++) {
                uint16_t expected;
                if (!ResolveMapTile(
                        map, logical_layer,
                        world_tile_x0 + dx, world_tile_y0 + dy,
                        &expected)) {
                    continue;
                }

                const uint16_t address = (uint16_t)(
                    map_base + (((world_tile_y0 + dy) & 31u) << 5) +
                    ((world_tile_x0 + dx) & 31u));
                checked++;
                if (ppu->vram[address & 0x7fffu] == expected)
                    matched++;
            }
        }
    }

    return checked >= 64 && matched * 100u >= checked * 90u;
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
                    ResolveWideTile(map, logical_layer, tile_x, tile_y));
            }
        }
    }
}

void Lufia2DeactivateMapWidescreen(void) {
    if (s_active)
        WsShadowReset();
    s_active = false;
    s_map_signature = 0;
    s_map.data = NULL;
    s_last_collision_cell_valid = false;
}

static void OverlayTile(Ppu *ppu, uint16_t address, uint16_t value) {
    address &= 0x7fff;
    for (unsigned i = 0; i < s_vram_backup_count; i++) {
        if (s_vram_backups[i].address == address) {
            ppu->vram[address] = value;
            return;
        }
    }
    if (s_vram_backup_count >= LUFIA2_MAX_VRAM_BACKUPS)
        return;

    s_vram_backups[s_vram_backup_count].address = address;
    s_vram_backups[s_vram_backup_count].value = ppu->vram[address];
    s_vram_backup_count++;
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
                ResolveWideTile(map, logical_layer, tile_x, tile_y));
        }
    }
}

void Lufia2BeginMapRenderOverlay(Ppu *ppu) {
    s_vram_backup_count = 0;
    if (!s_active || !ppu || !s_map.data)
        return;

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
    }
}

void Lufia2EndMapRenderOverlay(Ppu *ppu) {
    if (ppu) {
        for (unsigned i = 0; i < s_vram_backup_count; i++) {
            ppu->vram[s_vram_backups[i].address] =
                s_vram_backups[i].value;
        }
    }
    s_vram_backup_count = 0;
}

static bool RejectMap(uint64_t signature, const char *reason) {
    fprintf(stderr,
        "[video] widescreen disabled for indoor map $%02X (%s)\n",
        (unsigned)g_ram[0x05ac], reason);
    Lufia2DeactivateMapWidescreen();
    s_rejected_map_signature = signature;
    return false;
}

bool Lufia2PrepareMapWidescreen(Ppu *ppu, int margin_pixels) {
    Lufia2RuntimeMap map;
    if (!ppu || margin_pixels <= 0 || !ReadRuntimeMap(&map)) {
        Lufia2DeactivateMapWidescreen();
        return false;
    }

    const uint64_t signature = MapSignature(&map);
    if (signature == s_rejected_map_signature)
        return false;
    if (s_rejected_map_signature != 0 &&
        signature != s_rejected_map_signature) {
        s_rejected_map_signature = 0;
    }

    const bool map_changed = !s_active || signature != s_map_signature;
    if (map_changed) {
        if (!NativeViewportMatches(ppu, &map)) {
            Lufia2DeactivateMapWidescreen();
            return false;
        }

        s_last_collision_cell_valid = false;
        if (HasRoomBorders(&map))
            return RejectMap(signature, "room borders");
        if (CurrentAreaIsEnclosed(ppu, &map))
            return RejectMap(signature, "closed area");

        WsShadowReset();
        s_active = true;
        s_map_signature = signature;
        fprintf(stderr,
            "[video] full map source: %ux%u blocks, %u block definitions\n",
            (unsigned)map.width, (unsigned)map.height,
            (unsigned)map.block_count);
    } else if (CurrentAreaIsEnclosed(ppu, &map)) {
        return RejectMap(signature, "closed area");
    }

    s_map = map;

    const uint32_t world_x = (uint32_t)ppu->hScroll[0];
    const uint32_t world_y = (uint32_t)ppu->vScroll[0];
    for (int bg = 0; bg < 2; bg++) {
        WsShadowSetWorld(bg, world_x, world_y);
        WsShadowSetScroll(
            bg, (uint32_t)ppu->hScroll[bg], (uint32_t)ppu->vScroll[bg]);
        WsShadowSetBlankTile(bg, 0);
    }
    return true;
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
        &s_map, margin_pixels,
        (uint32_t)ppu->hScroll[0],
        (uint32_t)ppu->vScroll[0]);
}
