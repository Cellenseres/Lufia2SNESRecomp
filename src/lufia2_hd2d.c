#include "lufia2_hd2d.h"

#include <stdlib.h>
#include <string.h>

#include "lufia2_battle.h"
#include "lufia2_intro_mode7_world.h"
#include "lufia2_map_load.h"
#include "lufia2_map_widescreen.h"
#include "lufia2_runtime.h"
#include "cpu_state.h"
#include "snes/interp_bridge.h"
#include "snes/ppu.h"
#include "snesrecomp_platform/snes_ppu_mode7.h"

/* Battle records the capture reads. Presentation only: none is written. */
enum {
    /* 1 while enemies are BG2 tiles, 2 while they are OBJ. */
    WRAM_SPRITE_BUILD_MODE = 0x15ab,
    WRAM_ENEMY_GROUP_IDS = 0x1300,       /* three graphics groups */
    WRAM_ENEMY_GROUP_WIDTHS = 0x1315,    /* in 16-pixel tiles */
    WRAM_ENEMY_GROUP_HEIGHTS = 0x1318,
    WRAM_ENEMY_X = 0x132d,               /* six slots, top-left */
    WRAM_ENEMY_Y = 0x1333,
    WRAM_ENEMY_GROUP_KEYS = 0x1345,      /* $FF: slot empty */
    ENEMY_GROUPS = 3,
    ENEMIES_ON_BG2 = 1,
};

_Static_assert(LUFIA2_HD2D_MAX_WIDTH == kPpuBufWidth,
               "HD-2D surfaces must hold the widest PPU frame");

static const PpuOverlaySource kSources[LUFIA2_HD2D_LAYERS] = {
    kPpuOverlaySource_Bg1,
    kPpuOverlaySource_Bg2,
    kPpuOverlaySource_Bg3,
    kPpuOverlaySource_Obj,
};

/* Full-frame surfaces in the framebuffer's coordinate system. */
static uint32_t s_surface[LUFIA2_HD2D_LAYERS]
                         [LUFIA2_HD2D_MAX_WIDTH * LUFIA2_HD2D_HEIGHT];
static uint32_t s_flat[LUFIA2_HD2D_MAX_WIDTH * LUFIA2_HD2D_HEIGHT];
static Lufia2Hd2dFrame s_frame;
static bool s_enabled[LUFIA2_HD2D_KINDS] = {true, true, true};
static bool s_available;
static bool s_armed;
static bool s_valid;
/* A battle's panels have been seen since it came up. */
static bool s_battle_opened;
/* Pixels the game's sprite window is widened by on either side; zero while
 * no field is staged. */
static unsigned s_sprite_margin;
/* The window the game's sprite routine last tested its actors against, as
 * the hook saw it: map pixels. */
typedef struct SpriteWindow {
    bool seen;
    uint16_t left, right, top, bottom;
} SpriteWindow;
/* As the routine is running, and as it stood for the frame being taken. */
static SpriteWindow s_sprite_window, s_frame_window;

extern uint8_t g_ram[0x20000];

/* The world map's plane, as the PPU holds it. */
static uint8_t s_ring_tiles[SNESRECOMP_MODE7_TEXTURE_TEXELS];
static uint8_t s_plane_chars[LUFIA2_HD2D_PLANE_CHAR_BYTES];

void Lufia2Hd2dSetEnabled(Lufia2Hd2dKind kind, bool enabled) {
    if ((unsigned)kind < LUFIA2_HD2D_KINDS)
        s_enabled[kind] = enabled;
}

void Lufia2Hd2dSetAvailable(bool available) { s_available = available; }

bool Lufia2Hd2dEnabled(Lufia2Hd2dKind kind) {
    return (unsigned)kind < LUFIA2_HD2D_KINDS && s_enabled[kind] &&
           s_available;
}

bool Lufia2Hd2dBeginFrame(Ppu *ppu, const Lufia2BattleState *battle,
                         Lufia2Hd2dShown shown, unsigned frame_width,
                         unsigned extra, int *render_flags) {
    s_armed = false;
    if (!ppu || !battle || frame_width < kPpuXPixels ||
        frame_width > LUFIA2_HD2D_MAX_WIDTH ||
        extra * 2u != frame_width - kPpuXPixels)
        return false;
    /* Battle draws in Mode 1, which is what layer extraction is wired for;
     * the world map is Mode 7, of which only the sprites are extracted. */
    if (battle->active && battle->display_ready && PPU_mode(ppu) == 1 &&
        Lufia2Hd2dEnabled(LUFIA2_HD2D_BATTLE))
        s_frame.kind = LUFIA2_HD2D_BATTLE;
    else if (shown == LUFIA2_HD2D_SHOWN_WORLD_MAP && !battle->active &&
             PPU_mode(ppu) == 7 && Lufia2Hd2dEnabled(LUFIA2_HD2D_WORLD_MAP))
        s_frame.kind = LUFIA2_HD2D_WORLD_MAP;
    else if (shown == LUFIA2_HD2D_SHOWN_FIELD && !battle->active &&
             PPU_mode(ppu) == 1 && Lufia2Hd2dEnabled(LUFIA2_HD2D_MAP))
        s_frame.kind = LUFIA2_HD2D_MAP;
    else
        return false;

    PpuClearOverlayCaptures(ppu);
    PpuClearOverlayBindings(ppu);
    for (unsigned i = 0; i < LUFIA2_HD2D_LAYERS; i++) {
        /* Flags 0: a copy. The authentic frame stays complete, so every
         * fallback and every non-Battle consumer sees what it always saw. */
        if (!PpuBindOverlaySurface(ppu, kSources[i], (uint8_t *)s_surface[i],
                                   (size_t)frame_width * 4) ||
            !PpuSetOverlayCapture(ppu, kSources[i], -(int)extra, 0,
                                  (int)frame_width, LUFIA2_HD2D_HEIGHT, 0)) {
            PpuClearOverlayCaptures(ppu);
            PpuClearOverlayBindings(ppu);
            return false;
        }
    }
    if (render_flags)
        *render_flags |= kPpuRenderFlags_NewRenderer;

    s_frame.width = frame_width;
    s_frame.height = LUFIA2_HD2D_HEIGHT;
    s_frame.native_x = extra;
    s_frame.background_id = battle->background_id;
    s_armed = true;
    return true;
}

static void SpriteSize(unsigned obsel_size, bool large,
                       uint8_t *width, uint8_t *height) {
    static const uint8_t kSizes[8][2][2] = {
        {{8, 8}, {16, 16}},  {{8, 8}, {32, 32}},   {{8, 8}, {64, 64}},
        {{16, 16}, {32, 32}}, {{16, 16}, {64, 64}}, {{32, 32}, {64, 64}},
        {{16, 32}, {32, 64}}, {{16, 32}, {32, 32}},
    };
    *width = kSizes[obsel_size & 7u][large][0];
    *height = kSizes[obsel_size & 7u][large][1];
}

static void DecodeSprites(const Ppu *ppu, Lufia2Hd2dFrame *frame) {
    frame->sprite_count = 0;
    for (unsigned slot = 0; slot < LUFIA2_HD2D_OAM_SLOTS; slot++) {
        const unsigned index = slot * 2u;
        const uint8_t high = ppu->highOam[index >> 3] >> (index & 7u);
        Lufia2Hd2dSprite sprite;
        SpriteSize(PPU_objSize(ppu), (high & 2u) != 0,
                   &sprite.width, &sprite.height);

        int x = ppu->oam[index] & 0xff;
        int y = ppu->oam[index] >> 8;
        if (high & 1u) x -= 256;
        /* Rows wrap at 256; a sprite hanging over the top edge is drawn. */
        if (y >= LUFIA2_HD2D_HEIGHT && y + sprite.height > 256) y -= 256;
        if (y >= LUFIA2_HD2D_HEIGHT || x <= -(int)sprite.width)
            continue;

        const uint16_t attribute = ppu->oam[index + 1u];
        sprite.x = (int16_t)x;
        sprite.y = (int16_t)y;
        sprite.priority = (uint8_t)((attribute >> 12) & 3u);
        sprite.palette = (uint8_t)((attribute >> 9) & 7u);
        sprite.slot = (uint8_t)slot;
        sprite.flip = (uint8_t)(attribute >> 14);
        sprite.tile = (uint16_t)(attribute & 0x01ffu);
        frame->sprites[frame->sprite_count++] = sprite;
    }
}

static void DecodeEnemies(const uint8_t *wram, Lufia2Hd2dFrame *frame) {
    frame->enemy_count = 0;
    frame->enemies_on_bg2 = false;
    if (!wram)
        return;
    frame->enemies_on_bg2 = wram[WRAM_SPRITE_BUILD_MODE] == ENEMIES_ON_BG2;
    for (unsigned slot = 0; slot < LUFIA2_HD2D_ENEMIES; slot++) {
        const uint8_t key = wram[WRAM_ENEMY_GROUP_KEYS + slot];
        if (key == 0xffu)
            continue;
        Lufia2Hd2dEnemy enemy = {
            (int16_t)wram[WRAM_ENEMY_X + slot],
            (int16_t)wram[WRAM_ENEMY_Y + slot], 32, 32,
        };
        for (unsigned group = 0; group < ENEMY_GROUPS; group++) {
            if (wram[WRAM_ENEMY_GROUP_IDS + group] != key)
                continue;
            if (wram[WRAM_ENEMY_GROUP_WIDTHS + group])
                enemy.width =
                    (uint16_t)(wram[WRAM_ENEMY_GROUP_WIDTHS + group] * 16u);
            if (wram[WRAM_ENEMY_GROUP_HEIGHTS + group])
                enemy.height =
                    (uint16_t)(wram[WRAM_ENEMY_GROUP_HEIGHTS + group] * 16u);
            break;
        }
        frame->enemies[frame->enemy_count++] = enemy;
    }
}

/* The plane behind the world map's picture, or false when this frame's
 * picture is not a plain window on it: the game tilts and turns the plane
 * itself for some scenes, and those keep their own presentation. */
static bool CapturePlane(Lufia2Hd2dFrame *frame) {
    static SnesRecompMode7Line lines[LUFIA2_HD2D_HEIGHT];
    static SnesRecompMode7Line world_lines[LUFIA2_HD2D_HEIGHT];
    SnesPpuFrameCapture capture;
    SnesRecompMode7MapSource world;
    const SnesRecompMode7Line *first = lines;
    Lufia2Hd2dPlane *plane = &frame->plane;

    if (!Lufia2CapturePpuFrame(&capture, frame->width, frame->native_x) ||
        snesrecomp_ppu_mode7_supports(&capture) != SNES_PPU_SUPPORTED ||
        !snesrecomp_ppu_mode7_compile_lines(&capture, lines,
                                            LUFIA2_HD2D_HEIGHT) ||
        !snesrecomp_ppu_mode7_unpack_vram(capture.vram, s_ring_tiles,
                                          s_plane_chars))
        return false;
    for (unsigned y = 0; y < LUFIA2_HD2D_HEIGHT; y++) {
        if (lines[y].step_x != 256 || lines[y].step_y != 0 ||
            lines[y].start_x != lines[0].start_x ||
            ((lines[y].start_y - lines[0].start_y) &
             SNESRECOMP_MODE7_COORD_MASK) != y * 256u)
            return false;
    }

    /* The PPU holds a 1024-pixel ring of the world, enough for a view
     * straight down; a tilted one sees past it, so prefer the whole world. */
    plane->tiles = s_ring_tiles;
    plane->tiles_wide = plane->tiles_high = SNESRECOMP_MODE7_TEXTURE_DIM;
    if (Lufia2IntroMode7WorldPrepare(&capture, lines, LUFIA2_HD2D_HEIGHT,
                                     world_lines, LUFIA2_HD2D_HEIGHT,
                                     &world) == LUFIA2_INTRO_WORLD_READY) {
        plane->tiles = world.tiles;
        plane->tiles_wide = world.width_tiles;
        plane->tiles_high = world.height_tiles;
        plane->terrain = Lufia2IntroMode7WorldTerrain(
            &plane->terrain_wide, &plane->terrain_high);
        first = world_lines;
    }
    plane->chars = s_plane_chars;
    plane->origin_x = (float)first->start_x / 256.0f;
    plane->origin_y = (float)first->start_y / 256.0f;
    memcpy(plane->cgram, capture.cgram, sizeof plane->cgram);
    return true;
}

/* ---- field --------------------------------------------------------- */

enum {
    /* Pixels of map taken beyond the picture: the tilted camera sees past
     * its top most of all. */
    FIELD_BESIDE = 64,
    FIELD_ABOVE = 112,
    FIELD_BELOW = 48,
    /* The game streams its tilemaps a column or a row at a time behind the
     * edges of the screen, so only tiles well inside it are read from them. */
    FIELD_SAFE_X0 = 16,
    FIELD_SAFE_X1 = 240,
    FIELD_SAFE_Y0 = 8,
    FIELD_SAFE_Y1 = 208,
    FIELD_CHARS = 0x4000,            /* VRAM word address, both layers */
    FIELD_CHAR_WORDS = 16,
    FIELD_TILES = 1024,
    FIELD_BLOCKS = 1024,
    FIELD_MAX_CELLS = 16384,
    /* A surface, as against a picture of something: a block the lower layer
     * draws at least one cell in this many of the whole map with. Forest
     * canopy, grass and paving are; a roof, though it repeats, is not. */
    FIELD_TEXTURE_SHARE = 25,
    WRAM_PLAYER_X = 0x1ddae,         /* map pixels, 16 to a cell */
    WRAM_PLAYER_Y = 0x1de3e,
    WRAM_MAP_ID = 0x05ac,
    /* The field's actors: where each stands, by the same two tables the
     * party leader heads, and what takes one off the screen. */
    FIELD_ACTORS = 0x48,
    WRAM_ACTOR_X = WRAM_PLAYER_X,
    WRAM_ACTOR_Y = WRAM_PLAYER_Y,
    WRAM_ACTOR_STATE = 0x0622,
    WRAM_ACTOR_DRAW = 0x1e316,
    ACTOR_STATE_GONE = 0x04,
    ACTOR_DRAW_HIDDEN = 0x80,
    /* How far from its actor a sprite of it may lie, and still be its. */
    ACTOR_REACH = 48,
    /* The sprite routine, $83:A21A, has just stored its window in direct
     * page $54..$5A when it comes to this instruction. */
    SPRITE_WINDOW_PC = 0x83a295,
    SPRITE_WINDOW_ROM = 0x1a279,
    SPRITE_WINDOW_LEFT = 0x54,
    SPRITE_WINDOW_RIGHT = 0x56,
    SPRITE_WINDOW_TOP = 0x58,
    SPRITE_WINDOW_BOTTOM = 0x5a,
    /* The staged view shows a little more than the frame, and a sprite is
     * as wide as a cell. */
    SPRITE_MARGIN = 32,
    /* Where the routine takes its camera from, and how its window lies
     * about it: an actor is drawn when its place plus $30, $20 is inside. */
    WRAM_SPRITE_CAMERA_X = 0x1220,
    WRAM_SPRITE_CAMERA_Y = 0x1228,
    SPRITE_WINDOW_X0 = 0x20,
    SPRITE_WINDOW_X1 = 0x130,
    SPRITE_WINDOW_Y0 = 0x10,
    SPRITE_WINDOW_Y1 = 0x110,
    SPRITE_PLACE_X = 0x30,
    SPRITE_PLACE_Y = 0x20,
    /* Two bits of an actor's drawing byte move it in the routine's order:
     * to the back, or to the front. */
    ACTOR_DRAW_BEHIND = 0x01,
    ACTOR_DRAW_BEFORE = 0x02,
    TILE_NUMBER = 0x03ff,
    TILE_PRIORITY = 0x2000,
};

static uint16_t s_field_lower[LUFIA2_HD2D_FIELD_MAX_WIDE *
                              LUFIA2_HD2D_FIELD_MAX_HIGH];
static uint16_t s_field_upper[LUFIA2_HD2D_FIELD_MAX_WIDE *
                              LUFIA2_HD2D_FIELD_MAX_HIGH];
static uint8_t s_field_facts[LUFIA2_HD2D_FIELD_MAX_WIDE *
                             LUFIA2_HD2D_FIELD_MAX_HIGH];
static uint16_t s_vram[LUFIA2_HD2D_VRAM_WORDS];

/* Which cells of the map someone can stand on. */
static uint8_t s_open[FIELD_MAX_CELLS];
static uint16_t s_queue[FIELD_MAX_CELLS];
static uint8_t s_ground_blocks[FIELD_BLOCKS / 8];
static uint16_t s_block_cells[FIELD_BLOCKS];
static bool s_map_rooms;
/* Characters the upper layer draws on the map's walls. */
static uint8_t s_wall_tiles[FIELD_TILES / 8];

/* What the game animates, learnt by watching: water and lava are tiles it
 * redraws or palette rows it cycles. Kept per map, so only the first visit
 * has anything to learn. */
static uint8_t s_cycled_rows[256];
static uint8_t s_animated[256][FIELD_TILES / 8];
static struct {
    bool primed;
    uint32_t generation;
    uint8_t map_id;
    uint16_t cgram[128];
    uint16_t chars[FIELD_TILES * FIELD_CHAR_WORDS];
} s_watch;

static void WatchAnimation(const Ppu *ppu, uint8_t map_id) {
    const uint16_t *chars = ppu->vram + FIELD_CHARS;
    const uint32_t generation = Lufia2MapLoadGeneration();
    if (s_watch.primed && s_watch.generation == generation &&
        s_watch.map_id == map_id) {
        for (unsigned row = 0; row < 8u; row++)
            if (memcmp(s_watch.cgram + row * 16u, ppu->cgram + row * 16u,
                       16u * sizeof(uint16_t)))
                s_cycled_rows[map_id] |= (uint8_t)(1u << row);
        for (unsigned tile = 0; tile < FIELD_TILES; tile++)
            if (memcmp(s_watch.chars + tile * FIELD_CHAR_WORDS,
                       chars + tile * FIELD_CHAR_WORDS,
                       FIELD_CHAR_WORDS * sizeof(uint16_t)))
                s_animated[map_id][tile >> 3] |= (uint8_t)(1u << (tile & 7u));
    }
    s_watch.primed = true;
    s_watch.generation = generation;
    s_watch.map_id = map_id;
    memcpy(s_watch.cgram, ppu->cgram, sizeof s_watch.cgram);
    memcpy(s_watch.chars, chars, sizeof s_watch.chars);
}

/* Everywhere someone could walk to, and the blocks the lower layer draws
 * there. On open ground that is asked of the party alone: one cell opened
 * inside a house would open the whole of it, which has no walls within. A
 * map of rooms is asked of everyone in the room that is shown, because a
 * room may be shut off from where the party stands by no more than a door. */
static void FindOpenCells(const Lufia2FieldMap *map, int player_x,
                          int player_y) {
    const int wide = map->width, high = map->height;
    unsigned head = 0, tail = 0;
    memset(s_open, 0, (size_t)wide * high);
    memset(s_ground_blocks, 0, sizeof s_ground_blocks);
    if (player_x >= 0 && player_y >= 0 && player_x < wide &&
        player_y < high) {
        s_open[player_y * wide + player_x] = 1;
        s_queue[tail++] = (uint16_t)(player_y * wide + player_x);
    }
    s_map_rooms = false;
    for (int y = 0; y < high && !s_map_rooms; y++)
        for (int x = 0; x < wide && !s_map_rooms; x++)
            s_map_rooms = Lufia2FieldMapHidden(x, y);
    for (int y = 0; s_map_rooms && y < high; y++)
        for (int x = 0; x < wide; x++)
            if (!s_open[y * wide + x] && Lufia2FieldMapOccupied(x, y) &&
                !Lufia2FieldMapHidden(x, y)) {
                s_open[y * wide + x] = 1;
                s_queue[tail++] = (uint16_t)(y * wide + x);
            }
    while (head < tail) {
        const unsigned cell = s_queue[head++];
        const int x = (int)(cell % (unsigned)wide);
        const int y = (int)(cell / (unsigned)wide);
        const struct { int x, y; bool open; } next[4] = {
            {x + 1, y, Lufia2FieldMapOpenRight(x, y)},
            {x - 1, y, Lufia2FieldMapOpenRight(x - 1, y)},
            {x, y + 1, Lufia2FieldMapOpenDown(x, y)},
            {x, y - 1, Lufia2FieldMapOpenDown(x, y - 1)},
        };
        for (unsigned i = 0; i < 4u; i++) {
            if (!next[i].open)
                continue;
            const unsigned to = (unsigned)(next[i].y * wide + next[i].x);
            if (s_open[to])
                continue;
            s_open[to] = 1;
            s_queue[tail++] = (uint16_t)to;
        }
    }
    for (unsigned i = 0; i < tail; i++) {
        const unsigned cell = s_queue[i];
        const int x = (int)(cell % (unsigned)wide);
        const int y = (int)(cell / (unsigned)wide);
        const uint16_t block = Lufia2FieldMapBlock(1, x, y);
        /* Under an overhang the lower layer shows the top of what one walks
         * behind, a roof or a crown, not ground. */
        uint16_t over = 0;
        for (int t = 0; t < 4; t++)
            over |= Lufia2FieldMapTile(0, x * 2 + (t & 1), y * 2 + (t >> 1)) |
                    Lufia2FieldMapTile(1, x * 2 + (t & 1), y * 2 + (t >> 1));
        if (block < FIELD_BLOCKS && !(over & TILE_PRIORITY) &&
            !Lufia2FieldMapHidden(x, y))
            s_ground_blocks[block >> 3] |= (uint8_t)(1u << (block & 7u));
    }
    memset(s_block_cells, 0, sizeof s_block_cells);
    memset(s_wall_tiles, 0, sizeof s_wall_tiles);
    for (int y = 0; y < high; y++)
        for (int x = 0; x < wide; x++) {
            const uint16_t block = Lufia2FieldMapBlock(1, x, y);
            if (block < FIELD_BLOCKS)
                s_block_cells[block]++;
            if (Lufia2FieldMapHidden(x, y))
                continue;
            /* A wall: nobody can stand there, and what the lower layer
             * draws is drawn nowhere that somebody can. */
            if (s_open[y * wide + x] || block >= FIELD_BLOCKS ||
                ((s_ground_blocks[block >> 3] >> (block & 7u)) & 1u))
                continue;
            for (int t = 0; t < 4; t++) {
                const unsigned tile = Lufia2FieldMapTile(
                    0, x * 2 + (t & 1), y * 2 + (t >> 1)) & TILE_NUMBER;
                s_wall_tiles[tile >> 3] |= (uint8_t)(1u << (tile & 7u));
            }
        }
}

/* A field's sprites, where they really are. OAM holds nine bits of column
 * and eight of row, which is enough for the PPU's screen and not for a view
 * that reaches past it: a head above the top row and feet below the bottom
 * one are the same number. Each sprite belongs to an actor, and the game
 * knows where on the map every actor stands.
 *
 * Only the actors the game drew this frame are asked. Its routine draws
 * those inside a window one screen high, and an actor anywhere else on the
 * map, a screen further north in the same street, would answer to the same
 * row and carry the sprite off with it. */
static void DecodeFieldSprites(const Ppu *ppu, const uint8_t *wram,
                               const Lufia2FieldMap *map,
                               Lufia2Hd2dFrame *frame) {
    struct {
        int x, y;
        unsigned order;          /* the routine draws the highest first */
        bool hidden, done;
    } actor[FIELD_ACTORS];
    unsigned actors = 0;

    uint16_t left, right, top, bottom;
    if (s_frame_window.seen) {
        left = s_frame_window.left;
        right = s_frame_window.right;
        top = s_frame_window.top;
        bottom = s_frame_window.bottom;
    } else {
        const unsigned camera_x = wram[WRAM_SPRITE_CAMERA_X] |
                                  (wram[WRAM_SPRITE_CAMERA_X + 1] << 8);
        const unsigned camera_y = wram[WRAM_SPRITE_CAMERA_Y] |
                                  (wram[WRAM_SPRITE_CAMERA_Y + 1] << 8);
        left = (uint16_t)(camera_x + SPRITE_WINDOW_X0 - s_sprite_margin);
        right = (uint16_t)(camera_x + SPRITE_WINDOW_X1 + s_sprite_margin);
        top = (uint16_t)(camera_y + SPRITE_WINDOW_Y0);
        bottom = (uint16_t)(camera_y + SPRITE_WINDOW_Y1);
    }

    for (unsigned slot = 0; slot < FIELD_ACTORS; slot++) {
        const uint8_t draw = wram[WRAM_ACTOR_DRAW + slot];
        if ((wram[WRAM_ACTOR_STATE + slot] & ACTOR_STATE_GONE) ||
            (draw & ACTOR_DRAW_HIDDEN))
            continue;
        const unsigned map_x = wram[WRAM_ACTOR_X + slot * 2u] |
                               (wram[WRAM_ACTOR_X + slot * 2u + 1u] << 8);
        const unsigned map_y = wram[WRAM_ACTOR_Y + slot * 2u] |
                               (wram[WRAM_ACTOR_Y + slot * 2u + 1u] << 8);
        /* The routine's own test, in its own sixteen bits. */
        const uint16_t place_x = (uint16_t)(map_x + SPRITE_PLACE_X);
        const uint16_t place_y = (uint16_t)(map_y + SPRITE_PLACE_Y);
        if ((int16_t)(uint16_t)(place_x - left) < 0 ||
            (int16_t)(uint16_t)(place_x - right) >= 0 ||
            (int16_t)(uint16_t)(place_y - top) < 0 ||
            (int16_t)(uint16_t)(place_y - bottom) >= 0)
            continue;
        /* The picture's row y shows map row camera + 1 + y; an actor's
         * place is the top of the tile its feet are in. */
        actor[actors].x = (int)map_x - map->camera_x;
        actor[actors].y = (int)map_y - map->camera_y - 1;
        actor[actors].order = place_y +
                              (draw & ACTOR_DRAW_BEHIND ? 0u : 0x1000u) +
                              (draw & ACTOR_DRAW_BEFORE ? 0x2000u : 0u);
        actor[actors].hidden =
            Lufia2FieldMapHidden((int)map_x >> 4, (int)map_y >> 4);
        actor[actors].done = false;
        actors++;
    }

    frame->sprite_count = 0;
    int current = -1;
    for (unsigned slot = 0; slot < LUFIA2_HD2D_OAM_SLOTS; slot++) {
        const unsigned index = slot * 2u;
        const uint8_t high = ppu->highOam[index >> 3] >> (index & 7u);
        const unsigned column = (ppu->oam[index] & 0xffu) |
                                ((high & 1u) ? 0x100u : 0u);
        const unsigned row = ppu->oam[index] >> 8;
        Lufia2Hd2dSprite sprite;
        SpriteSize(PPU_objSize(ppu), (high & 2u) != 0,
                   &sprite.width, &sprite.height);
        /* Where the game parks the entries it does not use. */
        if (column == 0u && row == 0xf0u)
            continue;

        /* Whose it can be: every drawn actor it lies within reach of, the
         * nearest way round both ways. */
        int nearest = -1, apart_nearest = ACTOR_REACH + 1;
        int x = 0, y = 0;
        bool one_place = true;
        struct { int x, y; } place[FIELD_ACTORS];
        for (unsigned i = 0; i < actors; i++) {
            const int dx =
                (int)((column - (unsigned)actor[i].x + 256u) & 511u) - 256;
            const int dy =
                (int)((row - (unsigned)(actor[i].y - 8) + 128u) & 255u) - 128;
            place[i].x = actor[i].x + dx;
            place[i].y = actor[i].y - 8 + dy;
            const int apart = abs(dx) + abs(dy);
            if (apart > ACTOR_REACH) {
                place[i].x = INT16_MIN;
                continue;
            }
            if (nearest >= 0 && (place[i].x != place[nearest].x ||
                                 place[i].y != place[nearest].y))
                one_place = false;
            if (apart < apart_nearest) {
                apart_nearest = apart;
                nearest = (int)i;
            }
        }
        /* Two drawn actors a screen apart, one at its top and one at its
         * bottom, can still answer to the same row. The routine's order
         * settles it: an actor's entries follow one another, and the actors
         * come highest order first. */
        int whose = nearest;
        if (nearest >= 0 && !one_place) {
            if (current >= 0 && place[current].x != INT16_MIN) {
                whose = current;
            } else {
                whose = -1;
                for (unsigned i = 0; i < actors; i++)
                    if (place[i].x != INT16_MIN && !actor[i].done &&
                        (whose < 0 || actor[i].order > actor[whose].order))
                        whose = (int)i;
                if (whose < 0)
                    whose = nearest;
            }
        }
        bool hidden = false;
        if (whose >= 0) {
            x = place[whose].x;
            y = place[whose].y;
            hidden = actor[whose].hidden;
            if (current >= 0 && current != whose)
                actor[current].done = true;
            current = whose;
        } else {
            /* Nobody's: where the PPU would put it. */
            x = column >= 384u ? (int)column - 512 : (int)column;
            y = (int)row;
            if (y >= LUFIA2_HD2D_HEIGHT) {
                if (y + sprite.height <= 256)
                    continue;
                y -= 256;
            }
        }
        /* The game itself draws whoever is on its screen, whatever room
         * they are in; past the screen, only the room that is shown. */
        if (hidden && (x + sprite.width <= 0 || x >= kPpuXPixels ||
                       y + sprite.height <= 0 || y >= LUFIA2_HD2D_HEIGHT))
            continue;

        const uint16_t attribute = ppu->oam[index + 1u];
        sprite.x = (int16_t)x;
        sprite.y = (int16_t)y;
        sprite.priority = (uint8_t)((attribute >> 12) & 3u);
        sprite.palette = (uint8_t)((attribute >> 9) & 7u);
        sprite.slot = (uint8_t)slot;
        sprite.flip = (uint8_t)(attribute >> 14);
        sprite.tile = (uint16_t)(attribute & 0x01ffu);
        frame->sprites[frame->sprite_count++] = sprite;
    }
}

/* Is the cell part of a surface the lower layer tiles the map with? */
static bool InTexture(const Lufia2FieldMap *map, int x, int y) {
    const uint16_t block = Lufia2FieldMapBlock(1, x, y);
    return block < FIELD_BLOCKS &&
           (unsigned)s_block_cells[block] * FIELD_TEXTURE_SHARE >=
               (unsigned)map->width * map->height;
}

/* The map around the picture, or false when this frame is not a plain
 * window on it: a layer that scrolls on its own, a raster effect, or a map
 * that does not explain the screen keeps the game's own presentation. */
static bool CaptureField(const Ppu *ppu, const uint8_t *wram,
                         Lufia2Hd2dFrame *frame) {
    Lufia2Hd2dField *field = &frame->field;
    Lufia2FieldMap map;
    if (ppu->hScroll[1] != ppu->hScroll[0] ||
        ppu->vScroll[1] != ppu->vScroll[0] ||
        (ppu->screenEnabled[0] & 0x13u) != 0x13u)
        return false;
    for (unsigned y = 0; y < LUFIA2_HD2D_HEIGHT; y++) {
        const Ppu *line = (const Ppu *)(const void *)Lufia2LineRegisters(y);
        if (line && (line->hScroll[0] != ppu->hScroll[0] ||
                     line->vScroll[0] != ppu->vScroll[0] ||
                     line->hScroll[1] != ppu->hScroll[1] ||
                     line->vScroll[1] != ppu->vScroll[1] ||
                     PPU_mode(line) != 1))
            return false;
    }
    if (!Lufia2FieldMapRead(ppu, &map))
        return false;

    /* The map's number is that of the next map from the moment a door is
     * touched; the one on the screen is the last one that was loaded. */
    static uint8_t shown_map;
    if (!Lufia2MapLoadInProgress() && wram[WRAM_MAP_ID])
        shown_map = wram[WRAM_MAP_ID];
    const uint8_t map_id = shown_map;
    WatchAnimation(ppu, map_id);
    FindOpenCells(&map,
                  (wram[WRAM_PLAYER_X] | (wram[WRAM_PLAYER_X + 1] << 8)) / 16,
                  (wram[WRAM_PLAYER_Y] | (wram[WRAM_PLAYER_Y + 1] << 8)) / 16);
    DecodeFieldSprites(ppu, wram, &map, frame);

    /* Map pixel under the picture's first column and row. */
    const int map_x = map.camera_x - (int)frame->native_x;
    const int map_y = map.camera_y + 1;
    const int tile_x0 = (map_x - FIELD_BESIDE) >> 3;
    const int tile_y0 = (map_y - FIELD_ABOVE) >> 3;
    const int tile_x1 = (map_x + (int)frame->width + FIELD_BESIDE + 7) >> 3;
    const int tile_y1 = (map_y + LUFIA2_HD2D_HEIGHT + FIELD_BELOW + 7) >> 3;
    if (tile_x1 - tile_x0 > LUFIA2_HD2D_FIELD_MAX_WIDE ||
        tile_y1 - tile_y0 > LUFIA2_HD2D_FIELD_MAX_HIGH)
        return false;
    field->tiles_wide = (unsigned)(tile_x1 - tile_x0);
    field->tiles_high = (unsigned)(tile_y1 - tile_y0);
    field->origin_x = tile_x0 * 8 - map_x;
    field->origin_y = tile_y0 * 8 - map_y;
    field->map_x = map_x;
    field->map_y = map_y;

    const uint16_t upper_base = (uint16_t)PPU_bgTilemapAdr(ppu, 0);
    const uint16_t lower_base = (uint16_t)PPU_bgTilemapAdr(ppu, 1);
    for (int ty = tile_y0; ty < tile_y1; ty++) {
        for (int tx = tile_x0; tx < tile_x1; tx++) {
            const size_t at = (size_t)(ty - tile_y0) * field->tiles_wide +
                              (size_t)(tx - tile_x0);
            const int screen_x = tx * 8 - map.camera_x;
            const int screen_y = ty * 8 - map_y;
            if (screen_x >= FIELD_SAFE_X0 && screen_x + 8 <= FIELD_SAFE_X1 &&
                screen_y >= FIELD_SAFE_Y0 && screen_y + 8 <= FIELD_SAFE_Y1) {
                const unsigned ring = (((unsigned)ty & 31u) << 5) |
                                      ((unsigned)tx & 31u);
                s_field_upper[at] = ppu->vram[(upper_base + ring) & 0x7fffu];
                s_field_lower[at] = ppu->vram[(lower_base + ring) & 0x7fffu];
            } else {
                s_field_upper[at] = Lufia2FieldMapTile(0, tx, ty);
                s_field_lower[at] = Lufia2FieldMapTile(1, tx, ty);
            }

            const int cell_x = tx >> 1, cell_y = ty >> 1;
            const bool inside = cell_x >= 0 && cell_y >= 0 &&
                                cell_x < map.width && cell_y < map.height;
            const uint16_t block = Lufia2FieldMapBlock(1, cell_x, cell_y);
            const unsigned tile = s_field_lower[at] & TILE_NUMBER;
            uint8_t facts = 0;
            if (inside && s_open[cell_y * map.width + cell_x])
                facts |= LUFIA2_HD2D_TILE_OPEN;
            if (Lufia2FieldMapHidden(cell_x, cell_y))
                facts |= LUFIA2_HD2D_TILE_HIDDEN;
            if (block < FIELD_BLOCKS &&
                (s_ground_blocks[block >> 3] >> (block & 7u)) & 1u)
                facts |= LUFIA2_HD2D_TILE_GROUND;
            if (inside && InTexture(&map, cell_x, cell_y))
                facts |= LUFIA2_HD2D_TILE_TEXTURE;
            if (Lufia2FieldMapOpenDown(cell_x, cell_y))
                facts |= LUFIA2_HD2D_TILE_THROUGH;
            {
                const unsigned over = s_field_upper[at] & TILE_NUMBER;
                if ((s_wall_tiles[over >> 3] >> (over & 7u)) & 1u)
                    facts |= LUFIA2_HD2D_TILE_WALL;
            }
            if (((s_cycled_rows[map_id] >> ((s_field_lower[at] >> 10) & 7u)) &
                 1u) ||
                ((s_animated[map_id][tile >> 3] >> (tile & 7u)) & 1u))
                facts |= LUFIA2_HD2D_TILE_LIQUID;
            s_field_facts[at] = facts;
        }
    }

    memcpy(field->cgram, ppu->cgram, sizeof field->cgram);
    field->lower = s_field_lower;
    field->upper = s_field_upper;
    field->facts = s_field_facts;
    field->lower_chars = (uint16_t)(((ppu->bgTileAdr >> 4) & 15u) << 12);
    field->upper_chars = (uint16_t)((ppu->bgTileAdr & 15u) << 12);
    field->map_id = map_id;
    field->rooms = s_map_rooms;
    return true;
}

void Lufia2Hd2dEndFrame(Ppu *ppu, const uint8_t *wram, const uint8_t *flat,
                       uint32_t serial) {
    /* The window belongs to the guest frame that has just run. */
    s_frame_window = s_sprite_window;
    s_sprite_window.seen = false;
    if (!s_armed || !ppu || !flat) {
        s_armed = false;
        s_valid = false;
        s_sprite_margin = 0;
        return;
    }
    s_armed = false;
    PpuClearOverlayCaptures(ppu);
    PpuClearOverlayBindings(ppu);
    memset(&s_frame.plane, 0, sizeof s_frame.plane);
    memset(&s_frame.field, 0, sizeof s_frame.field);
    DecodeSprites(ppu, &s_frame);
    memcpy(s_vram, ppu->vram, sizeof s_vram);
    s_frame.vram = s_vram;
    {
        /* By the end of the frame the registers may already be set up for
         * the next; the middle of the picture was drawn with these. */
        const Ppu *line = (const Ppu *)(const void *)Lufia2LineRegisters(
            LUFIA2_HD2D_HEIGHT / 2u);
        if (!line)
            line = ppu;
        s_frame.sprite_chars[0] = (uint16_t)PPU_objTileAdr1(line);
        s_frame.sprite_chars[1] = (uint16_t)(PPU_objTileAdr2(line) & 0x7fffu);
    }
    memcpy(s_frame.sprite_cgram, ppu->cgram + 128, sizeof s_frame.sprite_cgram);
    if ((s_frame.kind == LUFIA2_HD2D_WORLD_MAP && !CapturePlane(&s_frame)) ||
        (s_frame.kind == LUFIA2_HD2D_MAP &&
         !CaptureField(ppu, wram, &s_frame))) {
        s_valid = false;
        s_sprite_margin = 0;
        return;
    }
    /* The next frame the game makes is for a view this wide. */
    s_sprite_margin = s_frame.kind == LUFIA2_HD2D_MAP
                          ? s_frame.native_x + SPRITE_MARGIN : 0u;

    for (unsigned i = 0; i < LUFIA2_HD2D_LAYERS; i++)
        s_frame.layer[i] = s_surface[i];
    /* The host goes on to compose margins and its own UI into the PPU
     * buffer; the split below needs the frame exactly as the PPU left it. */
    memcpy(s_flat, flat, (size_t)s_frame.width * s_frame.height * 4u);
    s_frame.flat = s_flat;
    s_frame.serial = serial;
    s_frame.brightness =
        PPU_forcedBlank(ppu) ? 0u : (uint8_t)PPU_brightness(ppu);

    const uint16_t colour = ppu->cgram[0];
    const unsigned r = ppu->brightnessMult[colour & 0x1fu];
    const unsigned g = ppu->brightnessMult[(colour >> 5) & 0x1fu];
    const unsigned b = ppu->brightnessMult[(colour >> 10) & 0x1fu];
    s_frame.backdrop = 0xff000000u | (r << 16) | (g << 8) | b;

    /* HDMA rewrites the layer enables and colour math down the screen, so
     * the state of the last line says nothing about the rest. */
    for (unsigned y = 0; y < LUFIA2_HD2D_HEIGHT; y++) {
        const Ppu *line = (const Ppu *)(const void *)Lufia2LineRegisters(y);
        if (!line)
            line = ppu;
        s_frame.lines[y].main_enable = line->screenEnabled[0];
        s_frame.lines[y].sub_enable = line->screenEnabled[1];
        s_frame.lines[y].cgwsel = line->cgwsel;
        s_frame.lines[y].cgadsub = line->cgadsub;
    }

    /* The battle records keep their last values outside Battle. */
    DecodeEnemies(s_frame.kind == LUFIA2_HD2D_BATTLE ? wram : NULL,
                  &s_frame);

    /* A battle shows its fighters a moment before it draws its panels. */
    if (s_frame.kind != LUFIA2_HD2D_BATTLE || !s_valid)
        s_battle_opened = false;
    if (s_frame.kind == LUFIA2_HD2D_BATTLE && !s_battle_opened) {
        const size_t pixels = (size_t)s_frame.width * s_frame.height;
        const uint32_t *bg2 = s_surface[LUFIA2_HD2D_BG2];
        const uint32_t *bg3 = s_surface[LUFIA2_HD2D_BG3];
        for (size_t i = 0; i < pixels && !s_battle_opened; i++)
            s_battle_opened =
                (bg3[i] >> 24) ||
                (!s_frame.enemies_on_bg2 && (bg2[i] >> 24));
    }
    s_frame.battle_opening =
        s_frame.kind == LUFIA2_HD2D_BATTLE && !s_battle_opened;
    s_valid = true;
}

/* ---- the game's sprite window -------------------------------------- */

static void WidenSpriteWindow(CpuState *cpu, uint32_t pc24) {
    (void)pc24;
    const unsigned margin = s_sprite_margin;
    const unsigned page = cpu->D;
    if (page + SPRITE_WINDOW_BOTTOM + 1u >= 0x2000u)
        return;
    uint8_t *left = g_ram + page + SPRITE_WINDOW_LEFT;
    uint8_t *right = g_ram + page + SPRITE_WINDOW_RIGHT;
    const uint8_t *top = g_ram + page + SPRITE_WINDOW_TOP;
    const uint8_t *bottom = g_ram + page + SPRITE_WINDOW_BOTTOM;
    const unsigned from = ((unsigned)(left[0] | (left[1] << 8)) - margin) &
                          0xffffu;
    const unsigned to = ((unsigned)(right[0] | (right[1] << 8)) + margin) &
                        0xffffu;
    if (margin) {
        left[0] = (uint8_t)from;
        left[1] = (uint8_t)(from >> 8);
        right[0] = (uint8_t)to;
        right[1] = (uint8_t)(to >> 8);
    }
    /* What the routine goes on to test against tells afterwards which
     * actors its sprites belong to. */
    s_sprite_window.seen = true;
    s_sprite_window.left = (uint16_t)from;
    s_sprite_window.right = (uint16_t)to;
    s_sprite_window.top = (uint16_t)(top[0] | (top[1] << 8));
    s_sprite_window.bottom = (uint16_t)(bottom[0] | (bottom[1] << 8));
}

void Lufia2Hd2dInstallHooks(const uint8_t *rom, size_t rom_size) {
    /* Camera plus $20 and $130 across, $10 and $110 down, each stored as
     * it is made; then the first instruction of the walk over the actors. */
    static const uint8_t kWindow[] = {
        0xa5, 0x9f, 0x18, 0x69, 0x20, 0x00, 0x85, 0x54, 0x18, 0x69,
        0x10, 0x01, 0x85, 0x56, 0xa5, 0xa1, 0x18, 0x69, 0x10, 0x00,
        0x85, 0x58, 0x18, 0x69, 0x00, 0x01, 0x85, 0x5a, 0x7b, 0xa8,
    };
    if (!rom || rom_size < SPRITE_WINDOW_ROM + sizeof kWindow ||
        memcmp(rom + SPRITE_WINDOW_ROM, kWindow, sizeof kWindow))
        return;
    interp_bridge_set_pre_opcode_hook(SPRITE_WINDOW_PC, WidenSpriteWindow);
}

const Lufia2Hd2dFrame *Lufia2Hd2dCurrent(void) {
    return s_valid ? &s_frame : NULL;
}
