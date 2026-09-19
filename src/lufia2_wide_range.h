#pragma once

#include <stdint.h>

/* Which shadow tiles a widescreen frame writes, and which it reads. */
/* A tile the lookup wants but the fill missed goes transparent. */

enum {
    LUFIA2_WIDE_TILE = 8,
    LUFIA2_WIDE_NATIVE_WIDTH = 256,
    LUFIA2_WIDE_VISIBLE_HEIGHT = 224,
};

typedef struct Lufia2WideTileRange {
    /* Half-open, as the fill loops iterate them. */
    uint32_t tile_x0, tile_x1;
    uint32_t tile_y0, tile_y1;
} Lufia2WideTileRange;

/* One tile of slack per side; upward it saturates at zero. */
static inline Lufia2WideTileRange Lufia2WideFillRange(
    uint32_t origin_x, uint32_t world_y, uint32_t margin_pixels) {
    const uint32_t reach = margin_pixels + LUFIA2_WIDE_TILE;
    const uint32_t right = origin_x + LUFIA2_WIDE_NATIVE_WIDTH + reach;
    const uint32_t bottom =
        world_y + LUFIA2_WIDE_VISIBLE_HEIGHT + LUFIA2_WIDE_TILE;
    const uint32_t top =
        world_y > LUFIA2_WIDE_TILE ? world_y - LUFIA2_WIDE_TILE : 0u;
    Lufia2WideTileRange range;

    range.tile_x0 = (origin_x - reach) >> 3;
    range.tile_x1 = (right + 7u) >> 3;
    range.tile_y0 = top >> 3;
    range.tile_y1 = (bottom + 7u) >> 3;
    return range;
}

/* The tile asked for at screen column `screen_x`. */
static inline uint32_t Lufia2WideLookupTileX(
    uint32_t origin_x, int32_t screen_x) {
    return (uint32_t)((int32_t)origin_x + screen_x) >> 3;
}

/* The tile row for screen line `screen_y`. */
static inline uint32_t Lufia2WideLookupTileY(
    uint32_t world_y, int32_t screen_y) {
    return (uint32_t)((int32_t)world_y + screen_y) >> 3;
}
