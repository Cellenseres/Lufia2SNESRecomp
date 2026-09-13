#include "lufia2_intro_mode7_world.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern uint8_t g_ram[0x20000];

/* Rebuilds the 4096x4096 world the guest only ever holds a 1024x1024 window
 * of, read out of the original loader ($86:CCFC, $86:CDF5, $86:AE05,
 * $86:AC6C, $86:AD82):
 *
 *   layout      128x128 big blocks at $7F:4040, one big block = 32x32 pixels
 *   tables      three entries at $86:CE32, picked by $09EA; +9/+11 the
 *               big-block pointer and bank, +12/+14 the block pointer and bank
 *   big block   bigblock[id * 8 + quadrant * 2], quadrants TL TR BL BR
 *   block       block[id * 5 + 0/1/2/3], tiles TL BL TR BR
 *   ring        torus aligned: ring tile (rx, ry) always holds the world tile
 *               congruent to (rx, ry) modulo 128
 *
 * Because the ring is torus aligned the window origin is the only unknown, and
 * $11E8/$11EA is not trusted for it -- the origin is solved from the captured
 * ring, which also identifies the table entry. Nothing in WRAM or VRAM is
 * modified. */

enum {
    WORLD_LAYOUT_WRAM = 0x14040u, /* $7F:4040 */
    WORLD_BIG_WIDTH = 128u,
    WORLD_BIG_HEIGHT = 128u,
    WORLD_LAYOUT_BYTES = WORLD_BIG_WIDTH * WORLD_BIG_HEIGHT * 2u,

    WORLD_TILE_WIDTH = 512u,
    WORLD_TILE_HEIGHT = 512u,
    WORLD_TILE_COUNT = WORLD_TILE_WIDTH * WORLD_TILE_HEIGHT,
    WORLD_PIXEL_SIZE = WORLD_TILE_WIDTH * 8u,
    WORLD_FIXED_WRAP = WORLD_PIXEL_SIZE * 256u,

    RING_TILES = 128u,
    RING_PIXELS = RING_TILES * 8u,
    RING_TILE_COUNT = RING_TILES * RING_TILES,
    RING_BANDS = WORLD_TILE_WIDTH / RING_TILES, /* 4 */
    RING_FIXED_MASK = 0x3ffffu,

    /* $86:CE32, LoROM file offset. */
    AREA_TABLE_ROM = 0x34e32u,
    AREA_TABLE_STRIDE = 18u,
    AREA_TABLE_COUNT = 3u,
    AREA_BIG_PTR = 9u,
    AREA_BIG_BANK = 11u,
    AREA_BLOCK_PTR = 12u,
    AREA_BLOCK_BANK = 14u,
    AREA_SELECT_WRAM = 0x09eau, /* $7E:09EA */
    CAMERA_X_WRAM = 0x11e8u,
    CAMERA_Y_WRAM = 0x11eau,

    BIG_BLOCK_STRIDE = 8u,
    BLOCK_STRIDE = 5u,
    /* $86:AC9D indexes the big-block table with a 16-bit Y through two
     * pointers, so it spans at most 64 KiB; the block table is reached through
     * one long pointer and ends at the top of its bank. */
    BIG_BLOCK_SPAN = 0x10000u,

    /* The ring is uploaded incrementally while scrolling, so a block just
     * come into view can lag a frame. Budget eight stale ring lines, spent
     * only when the previous origin or the camera agrees. */
    STALE_TILES_MAX = 8u * RING_TILES,
    ORIGIN_DRIFT_MAX = 16u,

    /* Keep a non-world scene from paying for a cold search every frame. */
    SEARCH_RETRY_FREE = 8u,
    SEARCH_RETRY_PERIOD = 16u,
};

typedef struct AreaTables {
    size_t big_block_base;
    size_t big_block_span;
    size_t block_base;
    size_t block_span;
} AreaTables;

typedef uint16_t PrefixPlane[RING_TILES + 1u][RING_TILES + 1u];

static const uint8_t *s_rom;
static size_t s_rom_size;

static uint8_t s_world_tiles[WORLD_TILE_COUNT];
static uint64_t s_world_signature;
static unsigned s_world_entry;
static bool s_world_valid;

static uint64_t s_locked_signature;
static unsigned s_locked_entry;
static unsigned s_locked_origin_x;
static unsigned s_locked_origin_y;
static bool s_locked;

static PrefixPlane *s_prefix; /* RING_BANDS * RING_BANDS, allocated on demand */

static uint64_t s_reported_signature;
static bool s_reported;
static uint64_t s_failed_signature;
static unsigned s_failed_count;

static uint16_t ReadLe16(const uint8_t *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8u));
}

static bool RomRangeValid(size_t offset, size_t bytes) {
    return s_rom && offset <= s_rom_size && bytes <= s_rom_size - offset;
}

static size_t LoRomOffset(unsigned bank, unsigned address) {
    return (size_t)(bank & 0x7fu) * 0x8000u + (address - 0x8000u);
}

static uint64_t LayoutSignature(const uint8_t *layout) {
    uint64_t hash = UINT64_C(1469598103934665603);
    for (unsigned i = 0; i < WORLD_LAYOUT_BYTES; i++) {
        hash ^= layout[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

/* Reads one entry of the $86:CE32 composition table. */
static bool AreaTablesFor(unsigned entry, AreaTables *out) {
    const uint8_t *record;
    unsigned big_bank, big_address, block_bank, block_address;

    if (entry >= AREA_TABLE_COUNT ||
        !RomRangeValid(AREA_TABLE_ROM, AREA_TABLE_COUNT * AREA_TABLE_STRIDE))
        return false;

    record = s_rom + AREA_TABLE_ROM + (size_t)entry * AREA_TABLE_STRIDE;
    big_address = ReadLe16(record + AREA_BIG_PTR);
    big_bank = record[AREA_BIG_BANK];
    block_address = ReadLe16(record + AREA_BLOCK_PTR);
    block_bank = record[AREA_BLOCK_BANK];
    if (big_address < 0x8000u || block_address < 0x8000u)
        return false;

    out->big_block_base = LoRomOffset(big_bank, big_address);
    out->big_block_span = BIG_BLOCK_SPAN;
    out->block_base = LoRomOffset(block_bank, block_address);
    /* A long indexed read leaves ROM once it passes the end of its bank. */
    out->block_span = 0x10000u - block_address;

    if (!RomRangeValid(out->big_block_base, 1u) ||
        !RomRangeValid(out->block_base, 1u))
        return false;
    if (out->big_block_span > s_rom_size - out->big_block_base)
        out->big_block_span = s_rom_size - out->big_block_base;
    if (out->block_span > s_rom_size - out->block_base)
        out->block_span = s_rom_size - out->block_base;
    return true;
}

static unsigned HintedAreaEntry(void) {
    const unsigned selector = g_ram[AREA_SELECT_WRAM];
    return selector < AREA_TABLE_COUNT ? selector : 0u;
}

/* $86:AD82's own origin, in world tiles. A hint only: it breaks ties between
 * windows the ring cannot tell apart, such as one entirely in open sea. */
static unsigned HintedOrigin(unsigned camera_wram) {
    const unsigned camera = ReadLe16(&g_ram[camera_wram]);
    const unsigned block = ((camera >> 4u) - 32u) & 0xffu;
    return block * 2u;
}

/* A uniform ring says nothing about where its window sits -- as does the
 * all-zero layout WRAM holds before the resource is decompressed. Fail closed
 * rather than lock onto an arbitrary origin. */
static bool RingIsAmbiguous(const uint16_t *vram) {
    const uint8_t first = (uint8_t)vram[0];

    for (unsigned i = 1; i < RING_TILE_COUNT; i++)
        if ((uint8_t)vram[i] != first)
            return false;
    return true;
}

/* Rebuilds the full 512x512 tile world for one composition table. */
static bool BuildWorldTiles(unsigned entry, uint64_t signature) {
    const uint8_t *layout = &g_ram[WORLD_LAYOUT_WRAM];
    AreaTables tables;

    if (s_world_valid && s_world_entry == entry &&
        s_world_signature == signature)
        return true;

    s_world_valid = false;
    s_world_entry = entry;
    s_world_signature = signature;
    if (!AreaTablesFor(entry, &tables))
        return false;

    for (unsigned big_y = 0; big_y < WORLD_BIG_HEIGHT; big_y++) {
        for (unsigned big_x = 0; big_x < WORLD_BIG_WIDTH; big_x++) {
            const size_t layout_offset =
                ((size_t)big_y * WORLD_BIG_WIDTH + big_x) * 2u;
            const unsigned big_id = ReadLe16(layout + layout_offset);
            const size_t big_offset = (size_t)big_id * BIG_BLOCK_STRIDE;
            const uint8_t *big;

            if (big_offset + BIG_BLOCK_STRIDE > tables.big_block_span)
                return false;
            big = s_rom + tables.big_block_base + big_offset;

            /* Big-block quadrant order is TL, TR, BL, BR. */
            for (unsigned quadrant = 0; quadrant < 4u; quadrant++) {
                const unsigned block_id = ReadLe16(big + quadrant * 2u);
                const size_t block_offset = (size_t)block_id * BLOCK_STRIDE;
                const unsigned block_x = quadrant & 1u;
                const unsigned block_y = quadrant >> 1u;
                const uint8_t *block;

                if (block_offset + 4u > tables.block_span)
                    return false;
                block = s_rom + tables.block_base + block_offset;

                /* Block bytes are TL, BL, TR, BR. */
                for (unsigned tile_y = 0; tile_y < 2u; tile_y++) {
                    for (unsigned tile_x = 0; tile_x < 2u; tile_x++) {
                        const unsigned dst_x =
                            big_x * 4u + block_x * 2u + tile_x;
                        const unsigned dst_y =
                            big_y * 4u + block_y * 2u + tile_y;
                        s_world_tiles[(size_t)dst_y * WORLD_TILE_WIDTH +
                                      dst_x] = block[tile_x * 2u + tile_y];
                    }
                }
            }
        }
    }

    s_world_valid = true;
    return true;
}

static uint8_t WorldTile(unsigned tile_x, unsigned tile_y) {
    return s_world_tiles[(size_t)(tile_y & (WORLD_TILE_HEIGHT - 1u)) *
                             WORLD_TILE_WIDTH +
                         (tile_x & (WORLD_TILE_WIDTH - 1u))];
}

/* Ring tiles the window at (origin_x, origin_y) fails to explain. */
static unsigned OriginMismatch(const uint16_t *vram,
                               unsigned origin_x,
                               unsigned origin_y,
                               unsigned *stale_columns,
                               unsigned *stale_rows) {
    bool column_seen[RING_TILES] = {false};
    bool row_seen[RING_TILES] = {false};
    unsigned mismatch = 0;

    for (unsigned ring_y = 0; ring_y < RING_TILES; ring_y++) {
        const unsigned world_y =
            origin_y + ((ring_y - origin_y) & (RING_TILES - 1u));
        for (unsigned ring_x = 0; ring_x < RING_TILES; ring_x++) {
            const unsigned world_x =
                origin_x + ((ring_x - origin_x) & (RING_TILES - 1u));
            const uint8_t actual =
                (uint8_t)vram[(size_t)ring_y * RING_TILES + ring_x];
            if (actual != WorldTile(world_x, world_y)) {
                mismatch++;
                column_seen[ring_x] = true;
                row_seen[ring_y] = true;
            }
        }
    }

    if (stale_columns || stale_rows) {
        unsigned columns = 0, rows = 0;
        for (unsigned i = 0; i < RING_TILES; i++) {
            columns += column_seen[i] ? 1u : 0u;
            rows += row_seen[i] ? 1u : 0u;
        }
        if (stale_columns)
            *stale_columns = columns;
        if (stale_rows)
            *stale_rows = rows;
    }
    return mismatch;
}

static unsigned OriginDrift(unsigned a, unsigned b, unsigned modulus) {
    const unsigned forward = (a - b) % modulus;
    const unsigned backward = (b - a) % modulus;
    return forward < backward ? forward : backward;
}

static unsigned RectSum(const PrefixPlane plane,
                        unsigned x0, unsigned y0, unsigned x1, unsigned y1) {
    return (unsigned)plane[y1][x1] - plane[y0][x1] -
           plane[y1][x0] + plane[y0][x0];
}

/* One summed-area table of mismatches per world band pair; band (bx, by)
 * covers world tiles (bx * 128 + ring_x, by * 128 + ring_y). */
static void BuildPrefixPlanes(const uint16_t *vram) {
    for (unsigned band_y = 0; band_y < RING_BANDS; band_y++) {
        for (unsigned band_x = 0; band_x < RING_BANDS; band_x++) {
            PrefixPlane *plane = &s_prefix[band_y * RING_BANDS + band_x];

            memset(&(*plane)[0][0], 0, sizeof (*plane)[0]);
            for (unsigned y = 0; y < RING_TILES; y++) {
                (*plane)[y + 1u][0] = 0;
                for (unsigned x = 0; x < RING_TILES; x++) {
                    const uint8_t actual =
                        (uint8_t)vram[(size_t)y * RING_TILES + x];
                    const unsigned differs =
                        actual != WorldTile(band_x * RING_TILES + x,
                                            band_y * RING_TILES + y);
                    (*plane)[y + 1u][x + 1u] = (uint16_t)(
                        (*plane)[y][x + 1u] + (*plane)[y + 1u][x] -
                        (*plane)[y][x] + differs);
                }
            }
        }
    }
}

/* An origin splits the torus-aligned ring into four rectangles of fixed world
 * band, so each candidate costs eight summed-area lookups and all 512x512 can
 * be searched exhaustively. Ties -- featureless terrain such as open sea -- are
 * broken by the camera, which by definition cannot contradict the ring. */
static unsigned SearchOrigin(const uint16_t *vram,
                             unsigned hint_x,
                             unsigned hint_y,
                             unsigned *origin_x,
                             unsigned *origin_y) {
    unsigned best = RING_TILE_COUNT + 1u;
    unsigned best_drift = ~0u;

    BuildPrefixPlanes(vram);
    for (unsigned band_y = 0; band_y < RING_BANDS; band_y++) {
        for (unsigned band_x = 0; band_x < RING_BANDS; band_x++) {
            const unsigned next_x = (band_x + 1u) & (RING_BANDS - 1u);
            const unsigned next_y = (band_y + 1u) & (RING_BANDS - 1u);
            /* Ring lines before the split wrapped into the next band. */
            const PrefixPlane *top_left =
                &s_prefix[next_y * RING_BANDS + next_x];
            const PrefixPlane *top_right =
                &s_prefix[next_y * RING_BANDS + band_x];
            const PrefixPlane *bottom_left =
                &s_prefix[band_y * RING_BANDS + next_x];
            const PrefixPlane *bottom_right =
                &s_prefix[band_y * RING_BANDS + band_x];

            for (unsigned split_y = 0; split_y < RING_TILES; split_y++) {
                for (unsigned split_x = 0; split_x < RING_TILES; split_x++) {
                    unsigned candidate_x, candidate_y, drift;
                    unsigned total =
                        RectSum(*top_left, 0, 0, split_x, split_y);
                    if (total > best)
                        continue;
                    total += RectSum(*top_right, split_x, 0,
                                     RING_TILES, split_y);
                    if (total > best)
                        continue;
                    total += RectSum(*bottom_left, 0, split_y,
                                     split_x, RING_TILES);
                    if (total > best)
                        continue;
                    total += RectSum(*bottom_right, split_x, split_y,
                                     RING_TILES, RING_TILES);
                    if (total > best)
                        continue;

                    candidate_x = band_x * RING_TILES + split_x;
                    candidate_y = band_y * RING_TILES + split_y;
                    drift = OriginDrift(candidate_x, hint_x,
                                        WORLD_TILE_WIDTH) +
                            OriginDrift(candidate_y, hint_y,
                                        WORLD_TILE_HEIGHT);
                    if (total < best || drift < best_drift) {
                        best = total;
                        best_drift = drift;
                        *origin_x = candidate_x;
                        *origin_y = candidate_y;
                    }
                }
            }
        }
    }
    return best;
}

static uint32_t WrapWorldFixed(int64_t value) {
    int64_t remainder = value % (int64_t)WORLD_FIXED_WRAP;
    if (remainder < 0)
        remainder += (int64_t)WORLD_FIXED_WRAP;
    return (uint32_t)remainder;
}

/* Lifts one axis of a captured 18-bit Mode 7 line into the world plane. The
 * anchor is the centre of the original 256 pixel screen, which the guest keeps
 * inside the ring, so the margins can run past the ring edge. */
static uint32_t LiftAxis(uint32_t local_start,
                         int32_t step,
                         unsigned origin_tile) {
    enum { ANCHOR_SCREEN_X = 128u };
    const uint32_t local_anchor =
        (local_start + (uint32_t)((int64_t)step * ANCHOR_SCREEN_X)) &
        RING_FIXED_MASK;
    const unsigned local_pixel = local_anchor >> 8u;
    const unsigned origin_pixel = origin_tile * 8u;
    const unsigned delta =
        (local_pixel - (origin_pixel & (RING_PIXELS - 1u))) &
        (RING_PIXELS - 1u);
    const unsigned world_pixel =
        (origin_pixel + delta) & (WORLD_PIXEL_SIZE - 1u);
    const uint32_t world_anchor =
        (world_pixel << 8u) | (local_anchor & 0xffu);

    return WrapWorldFixed((int64_t)world_anchor -
                          (int64_t)step * ANCHOR_SCREEN_X);
}

static void ReportRejection(uint64_t signature,
                            unsigned entry,
                            unsigned mismatch,
                            unsigned origin_x,
                            unsigned origin_y,
                            unsigned stale_columns,
                            unsigned stale_rows) {
    if (s_reported && s_reported_signature == signature)
        return;
    s_reported = true;
    s_reported_signature = signature;
    fprintf(stderr,
        "[video] Intro full-world Mode 7: no window explains the ring "
        "(best entry=%u origin=%u,%u mismatch=%u/%u columns=%u rows=%u, "
        "$09EA=%u camera origin=%u,%u)\n",
        entry, origin_x, origin_y, mismatch, (unsigned)RING_TILE_COUNT,
        stale_columns, stale_rows,
        (unsigned)g_ram[AREA_SELECT_WRAM],
        HintedOrigin(CAMERA_X_WRAM), HintedOrigin(CAMERA_Y_WRAM));
}

void Lufia2IntroMode7WorldInit(const uint8_t *rom, size_t rom_size) {
    s_rom = rom;
    s_rom_size = rom_size;
    s_world_valid = false;
    s_world_signature = 0;
    s_locked = false;
    s_reported = false;
    s_failed_signature = 0;
    s_failed_count = 0;
}

void Lufia2IntroMode7WorldStateChanged(void) {
    s_world_valid = false;
    s_world_signature = 0;
    s_locked = false;
    s_locked_signature = 0;
    s_reported = false;
    s_reported_signature = 0;
    s_failed_signature = 0;
    s_failed_count = 0;
}

Lufia2IntroMode7WorldStatus Lufia2IntroMode7WorldPrepare(
    const SnesPpuFrameCapture *capture,
    const SnesRecompMode7Line *local_lines,
    unsigned line_count,
    SnesRecompMode7Line *world_lines,
    unsigned world_line_capacity,
    SnesRecompMode7MapSource *world_source) {
    static const unsigned kNoEntry = (unsigned)-1;
    uint64_t signature;
    unsigned best_entry = kNoEntry, best_mismatch = RING_TILE_COUNT + 1u;
    unsigned best_x = 0, best_y = 0;
    unsigned stale_columns = 0, stale_rows = 0;
    bool any_table = false;

    if (!capture || !capture->vram || !local_lines || !world_lines ||
        !world_source || !line_count || line_count > world_line_capacity)
        return LUFIA2_INTRO_WORLD_INVALID_ARGUMENT;
    if (!s_rom)
        return LUFIA2_INTRO_WORLD_NO_ROM;
    if (RingIsAmbiguous(capture->vram))
        return LUFIA2_INTRO_WORLD_RING_MISMATCH;

    signature = LayoutSignature(&g_ram[WORLD_LAYOUT_WRAM]);

    /* The origin locked last frame usually still holds. */
    if (s_locked && s_locked_signature == signature &&
        BuildWorldTiles(s_locked_entry, signature) &&
        !OriginMismatch(capture->vram, s_locked_origin_x, s_locked_origin_y,
                        NULL, NULL)) {
        best_entry = s_locked_entry;
        best_mismatch = 0;
        best_x = s_locked_origin_x;
        best_y = s_locked_origin_y;
    }

    if (best_entry == kNoEntry) {
        unsigned entry_order[AREA_TABLE_COUNT];
        unsigned order_count = 0;
        const unsigned hinted = HintedAreaEntry();
        const unsigned hint_x = HintedOrigin(CAMERA_X_WRAM);
        const unsigned hint_y = HintedOrigin(CAMERA_Y_WRAM);

        if (s_failed_signature != signature) {
            s_failed_signature = signature;
            s_failed_count = 0;
        }
        if (s_failed_count > SEARCH_RETRY_FREE &&
            s_failed_count % SEARCH_RETRY_PERIOD) {
            s_failed_count++;
            return LUFIA2_INTRO_WORLD_RING_MISMATCH;
        }

        if (!s_prefix) {
            s_prefix = (PrefixPlane *)malloc(
                sizeof *s_prefix * RING_BANDS * RING_BANDS);
            if (!s_prefix)
                return LUFIA2_INTRO_WORLD_INVALID_LAYOUT;
        }

        entry_order[order_count++] = hinted;
        for (unsigned entry = 0; entry < AREA_TABLE_COUNT; entry++)
            if (entry != hinted)
                entry_order[order_count++] = entry;

        for (unsigned i = 0; i < order_count; i++) {
            const unsigned entry = entry_order[i];
            unsigned origin_x = 0, origin_y = 0, mismatch;

            if (!BuildWorldTiles(entry, signature))
                continue;
            any_table = true;
            mismatch = SearchOrigin(capture->vram, hint_x, hint_y,
                                    &origin_x, &origin_y);
            if (mismatch < best_mismatch) {
                best_mismatch = mismatch;
                best_entry = entry;
                best_x = origin_x;
                best_y = origin_y;
            }
            /* Entries 0 and 2 name identical tables, so stop at the first
             * one that explains the ring well enough. */
            if (mismatch <= STALE_TILES_MAX)
                break;
        }

        if (best_entry == kNoEntry) {
            s_failed_count++;
            return any_table ? LUFIA2_INTRO_WORLD_RING_MISMATCH
                             : LUFIA2_INTRO_WORLD_INVALID_LAYOUT;
        }
        if (!BuildWorldTiles(best_entry, signature)) {
            s_failed_count++;
            return LUFIA2_INTRO_WORLD_INVALID_LAYOUT;
        }
    }

    if (best_mismatch) {
        /* Spend the stale budget only when a source independent of the ring
         * puts the window in the same place. */
        const bool near_lock =
            s_locked && s_locked_entry == best_entry &&
            OriginDrift(best_x, s_locked_origin_x, WORLD_TILE_WIDTH) <=
                ORIGIN_DRIFT_MAX &&
            OriginDrift(best_y, s_locked_origin_y, WORLD_TILE_HEIGHT) <=
                ORIGIN_DRIFT_MAX;
        const bool near_camera =
            OriginDrift(best_x, HintedOrigin(CAMERA_X_WRAM),
                        WORLD_TILE_WIDTH) <= ORIGIN_DRIFT_MAX &&
            OriginDrift(best_y, HintedOrigin(CAMERA_Y_WRAM),
                        WORLD_TILE_HEIGHT) <= ORIGIN_DRIFT_MAX;

        OriginMismatch(capture->vram, best_x, best_y,
                       &stale_columns, &stale_rows);
        if (best_mismatch > STALE_TILES_MAX || (!near_lock && !near_camera)) {
            ReportRejection(signature, best_entry, best_mismatch,
                            best_x, best_y, stale_columns, stale_rows);
            s_failed_count++;
            return LUFIA2_INTRO_WORLD_RING_MISMATCH;
        }
    }

    /* Refresh on every accepted frame, not only exact ones -- that is what
     * keeps the tolerance window centred while the camera flies. */
    s_failed_count = 0;
    s_locked = true;
    s_locked_signature = signature;
    s_locked_entry = best_entry;
    s_locked_origin_x = best_x;
    s_locked_origin_y = best_y;
    s_reported = false;

    for (unsigned y = 0; y < line_count; y++) {
        world_lines[y] = local_lines[y];
        world_lines[y].start_x =
            LiftAxis(local_lines[y].start_x, local_lines[y].step_x, best_x);
        world_lines[y].start_y =
            LiftAxis(local_lines[y].start_y, local_lines[y].step_y, best_y);
    }
    world_source->tiles = s_world_tiles;
    world_source->width_tiles = WORLD_TILE_WIDTH;
    world_source->height_tiles = WORLD_TILE_HEIGHT;
    return LUFIA2_INTRO_WORLD_READY;
}

const char *Lufia2IntroMode7WorldStatusName(
    Lufia2IntroMode7WorldStatus status) {
    switch (status) {
    case LUFIA2_INTRO_WORLD_READY:
        return "ready";
    case LUFIA2_INTRO_WORLD_NO_ROM:
        return "verified ROM unavailable";
    case LUFIA2_INTRO_WORLD_INVALID_LAYOUT:
        return "invalid full-world layout";
    case LUFIA2_INTRO_WORLD_RING_MISMATCH:
        return "PPU ring does not match full world";
    case LUFIA2_INTRO_WORLD_INVALID_ARGUMENT:
    default:
        return "invalid frame data";
    }
}
