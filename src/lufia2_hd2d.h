#ifndef LUFIA2_HD2D_H
#define LUFIA2_HD2D_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* HD-2D presentation.
 *
 * The software PPU keeps drawing the authentic frame. While a scene that is
 * staged here owns the screen it additionally exports each layer in
 * isolation, through the runner's host-overlay extraction, so the presenter
 * can rebuild the picture as a lit, depth-sorted diorama instead of one flat
 * image:
 *
 *   capture (lufia2_hd2d.c)        PPU layers, raster state, game records
 *   scene   (lufia2_hd2d_scene.c)  world picture, actors, interface overlay
 *   render  (lufia2_hd2d_gl.c)     OpenGL passes
 *
 * Battle is a room: a backdrop and a floor read off one picture. The world
 * map is a ground plane: the Mode 7 plane itself, looked at from a tilted
 * camera instead of straight down. A town or a dungeon is a ground plane too,
 * the map's two tile layers, with whatever the game draws over its sprites
 * stood up on it.
 *
 * Only the capture knows the PPU; the scene builder and the renderer work on
 * the plain structures below, so a recorded frame replays without the game.
 * Guest state is left alone but for one thing, described at
 * Lufia2Hd2dInstallHooks. With the feature off no surface is bound and the
 * PPU output is byte-identical. */

enum {
    LUFIA2_HD2D_BG1 = 0,
    LUFIA2_HD2D_BG2,
    LUFIA2_HD2D_BG3,
    LUFIA2_HD2D_OBJ,
    LUFIA2_HD2D_LAYERS,

    LUFIA2_HD2D_NATIVE_WIDTH = 256,
    LUFIA2_HD2D_HEIGHT = 224,
    LUFIA2_HD2D_MAX_WIDTH = 800,
    LUFIA2_HD2D_OAM_SLOTS = 128,
    LUFIA2_HD2D_ENEMIES = 6,
    LUFIA2_HD2D_MAX_ACTORS = 48,

    /* Mode 7: 256 characters of 8x8 palette indices, laid out as the PPU
     * addresses them, and a 256-colour palette. */
    LUFIA2_HD2D_PLANE_CHAR_BYTES = 128 * 128,
    LUFIA2_HD2D_PLANE_COLOURS = 256,

    /* The field: a window of the map in 8x8 tiles, wider and taller than
     * the picture, and all of VRAM for its characters. */
    LUFIA2_HD2D_FIELD_MAX_WIDE = 128,
    LUFIA2_HD2D_FIELD_MAX_HIGH = 56,
    LUFIA2_HD2D_VRAM_WORDS = 0x8000,
};

/* What the game's own map says of one tile of the field window. */
enum {
    /* Someone can stand on the tile's cell. */
    LUFIA2_HD2D_TILE_OPEN = 1 << 0,
    /* The cell is another room's, or lies outside the map. */
    LUFIA2_HD2D_TILE_HIDDEN = 1 << 1,
    /* The lower layer draws the cell with a block it also draws where one
     * can stand: floor, grass, street. */
    LUFIA2_HD2D_TILE_GROUND = 1 << 2,
    /* The lower layer tiles the map with this cell's block: a surface, like
     * forest canopy or paving, not a picture of one thing. */
    LUFIA2_HD2D_TILE_TEXTURE = 1 << 3,
    /* The lower layer's tile is one the game animates. */
    LUFIA2_HD2D_TILE_LIQUID = 1 << 4,
    /* Nothing stops a step from the tile's cell to the cell south of it. */
    LUFIA2_HD2D_TILE_THROUGH = 1 << 5,
    /* The upper layer's tile is one the map draws on its walls: on cells
     * nobody can stand on and whose ground is no floor. */
    LUFIA2_HD2D_TILE_WALL = 1 << 6,
};

/* What is being staged. Each kind has its own switch. */
typedef enum Lufia2Hd2dKind {
    LUFIA2_HD2D_BATTLE = 0,
    LUFIA2_HD2D_WORLD_MAP,
    /* Towns and dungeons. */
    LUFIA2_HD2D_MAP,
    LUFIA2_HD2D_KINDS,
} Lufia2Hd2dKind;

/* The Mode 7 plane the world map's flat picture is a window on. */
typedef struct Lufia2Hd2dPlane {
    /* Character number per 8x8 cell, row-major: the whole 512x512 world
     * when it could be rebuilt, else the 128x128 ring the PPU holds. The
     * plane wraps at its edges either way. */
    const uint8_t *tiles;
    unsigned tiles_wide, tiles_high;
    const uint8_t *chars;        /* LUFIA2_HD2D_PLANE_CHAR_BYTES */
    /* CGRAM as the game wrote it, BGR555; the frame's brightness applies. */
    uint16_t cgram[LUFIA2_HD2D_PLANE_COLOURS];
    /* The game's own terrain class for each 16x16 block of the plane, or
     * NULL when only the ring is known: $00 land, $20 forest, $30 sea, $40
     * mountain. It is what tells the renderer which ground stands tall. */
    const uint8_t *terrain;
    unsigned terrain_wide, terrain_high;
    /* Plane pixel under the picture's SNES x = 0, row 0. The picture shows
     * the plane one to one and unrotated; a frame that does not is not
     * captured. */
    float origin_x, origin_y;
} Lufia2Hd2dPlane;

/* The field map around the picture: both tile layers as the PPU would hold
 * them had its tilemaps been large enough, with what is needed to draw them
 * and to tell what on them stands. */
typedef struct Lufia2Hd2dField {
    /* Tilemap words -- character, palette, priority, flips -- row-major:
     * the lower layer is BG2, the upper BG1. */
    const uint16_t *lower, *upper;
    const uint8_t *facts;        /* LUFIA2_HD2D_TILE_* per tile */
    unsigned tiles_wide, tiles_high;
    /* Picture column and row of the window's first tile; negative where
     * the window reaches past the picture. */
    int origin_x, origin_y;
    /* Map pixel under the picture's first column and row. */
    int map_x, map_y;
    /* Word addresses of the layers' characters in the frame's VRAM. */
    uint16_t lower_chars, upper_chars;
    uint16_t cgram[LUFIA2_HD2D_PLANE_COLOURS];
    uint8_t map_id;
    /* The map is made of rooms, of which the game shows one at a time: a
     * house's inside, a cave. Beyond a room's walls there is nothing. */
    bool rooms;
} Lufia2Hd2dField;

/* One OAM entry, decoded. Battle and the world map take the pixels from the
 * OBJ layer; a field draws them from `tile`. */
typedef struct Lufia2Hd2dSprite {
    /* SNES screen coordinates of the top-left. On a field they are where
     * the sprite really is, past any edge of the screen: OAM wraps, and the
     * game's own record of who stands where says which turn it is on. */
    int16_t x, y;
    uint8_t width, height;
    uint8_t priority;        /* 0..3 */
    uint8_t palette;         /* 0..7 */
    uint8_t slot;
    uint8_t flip;            /* 1 mirrored left to right, 2 top to bottom */
    uint16_t tile;           /* character, with the table in bit 8 */
} Lufia2Hd2dSprite;

/* Composition state of one scanline. */
typedef struct Lufia2Hd2dLine {
    uint8_t main_enable;     /* TM */
    uint8_t sub_enable;      /* TS */
    uint8_t cgwsel;
    uint8_t cgadsub;
} Lufia2Hd2dLine;

/* An enemy slot's picture rectangle, from the battle's own records. */
typedef struct Lufia2Hd2dEnemy {
    int16_t x, y;
    uint16_t width, height;
} Lufia2Hd2dEnemy;

typedef struct Lufia2Hd2dFrame {
    Lufia2Hd2dKind kind;
    /* ARGB8888, alpha 0 or 255, master brightness already applied. `width`
     * columns per row; SNES x = 0 is column `native_x`. */
    const uint32_t *layer[LUFIA2_HD2D_LAYERS];
    /* The authentic composite of the same frame, same geometry. */
    const uint32_t *flat;
    unsigned width, height;
    unsigned native_x;

    uint32_t serial;         /* guest frame this picture belongs to */
    uint8_t background_id;
    uint8_t brightness;      /* 0..15 */
    uint32_t backdrop;       /* ARGB of CGRAM entry 0 at this brightness */

    Lufia2Hd2dLine lines[LUFIA2_HD2D_HEIGHT];

    /* Enemies are OBJ while commands are chosen and BG2 tiles while actions
     * play out; the interface owns whichever of the two they are not on. */
    bool enemies_on_bg2;
    /* The battle has come up but none of its panels has: the party stands
     * where its status cards are about to be drawn. */
    bool battle_opening;
    Lufia2Hd2dEnemy enemies[LUFIA2_HD2D_ENEMIES];
    unsigned enemy_count;

    Lufia2Hd2dSprite sprites[LUFIA2_HD2D_OAM_SLOTS];
    unsigned sprite_count;
    /* What the sprites are drawn from: all of VRAM, where in it the two
     * sprite character tables lie, and the sprites' half of the palette as
     * the game wrote it. A field's sprites are drawn from these by the
     * scene builder, not taken from the PPU's picture, which ends where its
     * screen does. */
    const uint16_t *vram;        /* LUFIA2_HD2D_VRAM_WORDS */
    uint16_t sprite_chars[2];
    uint16_t sprite_cgram[128];

    /* World map only. */
    Lufia2Hd2dPlane plane;
    /* Towns and dungeons only. */
    Lufia2Hd2dField field;
} Lufia2Hd2dFrame;

typedef enum Lufia2Hd2dActorKind {
    LUFIA2_HD2D_ACTOR_ENEMY = 0,
    /* A party member out on the floor. One shown inside its status card,
     * as while commands are chosen, belongs to the interface instead. */
    LUFIA2_HD2D_ACTOR_HERO,
    /* Cursor, damage digits, effect sprites: no footing of their own. */
    LUFIA2_HD2D_ACTOR_FLOATER,
} Lufia2Hd2dActorKind;

/* A billboard: a tight rectangle of the actor picture and the screen row
 * where it meets the floor. */
typedef struct Lufia2Hd2dActor {
    Lufia2Hd2dActorKind kind;
    int16_t x0, y0, x1, y1;  /* picture columns/rows, exclusive end */
    int16_t foot;            /* floor contact row, picture coordinates */
} Lufia2Hd2dActor;

/* The field window made ready to draw. */
typedef struct Lufia2Hd2dGround {
    /* Premultiplied RGBA8888 at full brightness, `tiles_wide * 8` by
     * `tiles_high * 8`: the lower layer is opaque throughout, the upper one
     * where it has picture. */
    const uint8_t *lower, *upper;
    /* Four bytes a pixel of the window. First and second: how many rows of
     * picture the thing the upper and the lower layer draws there is above
     * the ground; a step back on the ground and a step up are both one row
     * of this picture, so that is also how far south of where it is drawn
     * it stands. Third: 255 where what shows is drawn over the sprites.
     * Fourth: 255 where the upper layer has picture. */
    const uint8_t *relief;
    unsigned tiles_wide, tiles_high;
    int origin_x, origin_y;      /* as in the field */
    int map_x, map_y;
    uint8_t backdrop[3];         /* what shows where there is no map */
    unsigned tallest;            /* greatest height in the relief, in rows */
    /* A room or a cave: beyond its walls there is nothing. */
    bool enclosed;
} Lufia2Hd2dGround;

typedef struct Lufia2Hd2dScene {
    Lufia2Hd2dKind kind;
    /* Premultiplied RGBA8888 in memory order R,G,B,A; `width` x `height`. */
    const uint8_t *background;   /* BG1; unused on the world map */
    /* World sprites: OBJ and BG2 enemies. This picture has a size of its
     * own, `actors_width` x `actors_height`, and its first pixel lies at
     * picture column `actors_x`, row `actors_y`: on a field it reaches past
     * the frame, above it most of all. */
    const uint8_t *actors;
    int actors_x, actors_y;
    unsigned actors_width, actors_height;
    /* What the authentic frame shows beyond the world picture: panels, text,
     * icons, flashes, and the party portraits in their status cards.
     * out = interface.rgb + (1 - interface.a) * scene. */
    const uint8_t *interface;
    unsigned width, height;
    unsigned native_x;

    /* Opaque BG1 column range; the PPU windows a few edge columns away. */
    unsigned background_x0, background_x1;
    /* Share of that range's pixels BG1 actually covers. A transition that
     * windows the background away lowers it. */
    float background_cover;

    Lufia2Hd2dActor actor[LUFIA2_HD2D_MAX_ACTORS];
    unsigned actor_count;

    uint8_t background_id;
    float brightness;            /* 0..1 */
    uint32_t serial;

    /* World map only; borrowed from the frame. */
    Lufia2Hd2dPlane plane;
    /* Towns and dungeons only. */
    Lufia2Hd2dGround ground;
} Lufia2Hd2dScene;

/* ---- capture -------------------------------------------------------- */

struct Ppu;
struct Lufia2BattleState;

/* The user's switch for each kind, and whether the active presenter can draw
 * a scene at all. Capture runs only while both hold. */
void Lufia2Hd2dSetEnabled(Lufia2Hd2dKind kind, bool enabled);
void Lufia2Hd2dSetAvailable(bool available);
bool Lufia2Hd2dEnabled(Lufia2Hd2dKind kind);

/* The game leaves out of OAM whoever stands outside its 256-pixel screen. A
 * staged field shows more than that, so while one is up the game's own test
 * is given a wider window. This is the one place the presentation reaches
 * into the guest: two words of scratch the sprite routine has just written,
 * and only under the interpreter, which is where the routine can be met.
 * Call once at start-up with the ROM. */
void Lufia2Hd2dInstallHooks(const uint8_t *rom, size_t rom_size);

/* What the host's own layout decision says is on screen. */
typedef enum Lufia2Hd2dShown {
    LUFIA2_HD2D_SHOWN_OTHER = 0,
    LUFIA2_HD2D_SHOWN_WORLD_MAP,
    LUFIA2_HD2D_SHOWN_FIELD,
} Lufia2Hd2dShown;

/* Bracket the PPU scanout of one guest frame. Begin arms the captures when
 * a staged scene is visible -- Battle from its own state, the world map and
 * the field when the host says their layout is up -- and returns whether it
 * did; `render_flags` gains the new-renderer bit the extraction needs. End
 * publishes the layers together with `flat`, the frame the PPU just drew. */
bool Lufia2Hd2dBeginFrame(struct Ppu *ppu,
                         const struct Lufia2BattleState *battle,
                         Lufia2Hd2dShown shown, unsigned frame_width,
                         unsigned extra, int *render_flags);
void Lufia2Hd2dEndFrame(struct Ppu *ppu, const uint8_t *wram,
                       const uint8_t *flat, uint32_t serial);

/* The most recent staged frame, or NULL when the last frame was not one. */
const Lufia2Hd2dFrame *Lufia2Hd2dCurrent(void);

/* ---- scene ---------------------------------------------------------- */

/* Splits a frame into world and interface. The scene borrows storage owned
 * by the builder; it stays valid until the next call. */
void Lufia2Hd2dBuildScene(const Lufia2Hd2dFrame *frame,
                         Lufia2Hd2dScene *scene);

/* ---- recording ------------------------------------------------------ */

/* A self-contained frame for offline replay (scripts/tests/hd2d_preview.c).
 * Load returns a frame that owns its planes; free it with Free. */
bool Lufia2Hd2dSaveFrame(const char *path, const Lufia2Hd2dFrame *frame);
bool Lufia2Hd2dLoadFrame(const char *path, Lufia2Hd2dFrame *frame);
void Lufia2Hd2dFreeFrame(Lufia2Hd2dFrame *frame);

#endif
