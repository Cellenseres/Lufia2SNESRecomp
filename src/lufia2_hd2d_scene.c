#include "lufia2_hd2d.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* World/interface split and actor extraction. No PPU and no GL in here: the
 * input is a captured frame, the output is three pictures and a list. */

enum {
    SCENE_PIXELS = LUFIA2_HD2D_MAX_WIDTH * LUFIA2_HD2D_HEIGHT,
    /* A field's actor picture reaches this far past the frame: beside it
     * for whoever stands in the view's widened margins, and above it for
     * the heads of those who stand at its top. */
    CAST_BESIDE = 64,
    CAST_ABOVE = 48,
    CAST_PIXELS = (LUFIA2_HD2D_MAX_WIDTH + 2 * CAST_BESIDE) *
                  (LUFIA2_HD2D_HEIGHT + CAST_ABOVE),
    /* One 5-bit colour step is 8; the PPU halves before it expands. */
    COLOUR_TOLERANCE = 10,
    SAME_TOLERANCE = 2,
    /* Enemy pictures idle a pixel or two around their record. */
    ENEMY_SLACK = 4,
    /* A flash or dissolve reaches a little past the picture. */
    ENEMY_EFFECT_SLACK = 16,
    FLOATER_MAX_HEIGHT = 12,
    HERO_FIRST_PALETTE = 4,
    /* Interface coverage that counts as a panel behind a party member. */
    PANEL_ALPHA = 64,
    /* The status bar spans the picture and is at least this tall. */
    STATUS_BAR_MIN_COLUMNS = 224,
    STATUS_BAR_MIN_ROWS = 24,
};

static uint8_t s_background[SCENE_PIXELS * 4];
static uint8_t s_actors[CAST_PIXELS * 4];
/* Where the actor picture lies: picture column and row of its first pixel,
 * and its size. The frame itself, except on a field. */
static struct {
    int x, y;
    int width, height;
} s_cast;

static void SetCast(Lufia2Hd2dScene *scene, int x, int y, int width,
                    int height) {
    s_cast.x = x;
    s_cast.y = y;
    s_cast.width = width;
    s_cast.height = height;
    scene->actors_x = x;
    scene->actors_y = y;
    scene->actors_width = (unsigned)width;
    scene->actors_height = (unsigned)height;
}

/* The actor pixel at a picture column and row inside the cast. */
static inline uint8_t *ActorAt(int x, int y) {
    return s_actors + ((size_t)(y - s_cast.y) * (size_t)s_cast.width +
                       (size_t)(x - s_cast.x)) * 4u;
}
static uint8_t s_interface[SCENE_PIXELS * 4];
/* World-only composite the interface is measured against, R,G,B per pixel. */
static uint8_t s_world[SCENE_PIXELS * 3];

static inline unsigned Alpha(uint32_t argb) { return argb >> 24; }

static inline void PutRgba(uint8_t *out, uint32_t argb) {
    out[0] = (uint8_t)(argb >> 16);
    out[1] = (uint8_t)(argb >> 8);
    out[2] = (uint8_t)argb;
    out[3] = (uint8_t)(argb >> 24);
}

static bool InEnemyRect(const Lufia2Hd2dFrame *frame, int x, int y,
                        int slack) {
    for (unsigned i = 0; i < frame->enemy_count; i++) {
        const Lufia2Hd2dEnemy *enemy = &frame->enemies[i];
        if (x >= enemy->x - slack && x < enemy->x + enemy->width + slack &&
            y >= enemy->y - slack && y < enemy->y + enemy->height + slack)
            return true;
    }
    return false;
}

/* BG2 belongs to the world only while the battle draws its enemies there.
 * Sprites are masked off the status bar's raster band, which keeps that bar
 * with the interface; colour math on BG2 means a translucent window unless
 * the pixel sits on an enemy, where it is a flash or a dissolve. */
static bool Bg2IsWorld(const Lufia2Hd2dFrame *frame, int screen_x, int y) {
    const Lufia2Hd2dLine *line = &frame->lines[y];
    if (!frame->enemies_on_bg2 || (line->main_enable & 0x12u) != 0x12u)
        return false;
    return !(line->cgadsub & 0x02u) ||
           InEnemyRect(frame, screen_x, y, ENEMY_EFFECT_SLACK);
}

static inline int Clamp255(int value) {
    return value < 0 ? 0 : value > 255 ? 255 : value;
}

static bool SameColour(uint32_t a, uint32_t b) {
    for (int shift = 0; shift < 24; shift += 8)
        if (abs((int)((a >> shift) & 0xff) - (int)((b >> shift) & 0xff)) >
            SAME_TOLERANCE)
            return false;
    return true;
}

/* The overlay that turns the world colour into the authentic one. `plain`
 * is the pixel an interface layer holds here, if any: where the authentic
 * frame shows exactly that, it is that layer, opaque, whatever colour math
 * the scanline has switched on for the layers around it. */
static void SolveInterface(const uint8_t world[3], uint32_t flat,
                           uint32_t plain, const Lufia2Hd2dLine *line,
                           uint8_t out[4]) {
    const int f[3] = {(int)((flat >> 16) & 0xff), (int)((flat >> 8) & 0xff),
                      (int)(flat & 0xff)};
    int difference = 0;
    for (int c = 0; c < 3; c++) {
        const int d = abs(f[c] - world[c]);
        if (d > difference) difference = d;
    }
    out[0] = out[1] = out[2] = out[3] = 0;
    if (difference <= SAME_TOLERANCE)
        return;
    if (Alpha(plain) && SameColour(flat, plain)) {
        out[0] = (uint8_t)f[0];
        out[1] = (uint8_t)f[1];
        out[2] = (uint8_t)f[2];
        out[3] = 255;
        return;
    }

    const bool math = (line->cgadsub & 0x3fu) != 0;
    const bool half = (line->cgadsub & 0x40u) != 0;
    const bool subtract = (line->cgadsub & 0x80u) != 0;

    if (math && half && !subtract) {
        /* flat = (world + other) / 2: the other colour at half coverage. */
        int other[3];
        bool fits = true;
        for (int c = 0; c < 3; c++) {
            other[c] = 2 * f[c] - world[c];
            if (other[c] < -COLOUR_TOLERANCE ||
                other[c] > 255 + COLOUR_TOLERANCE)
                fits = false;
        }
        if (fits) {
            for (int c = 0; c < 3; c++)
                out[c] = (uint8_t)(Clamp255(other[c]) / 2);
            out[3] = 128;
            return;
        }
    } else if (math && !half && !subtract) {
        /* flat = world + other: light, no coverage. */
        bool fits = true;
        for (int c = 0; c < 3; c++)
            if (f[c] - world[c] < -COLOUR_TOLERANCE) fits = false;
        if (fits) {
            for (int c = 0; c < 3; c++)
                out[c] = (uint8_t)Clamp255(f[c] - world[c]);
            return;
        }
    } else if (math && subtract) {
        /* A darkening: black at the coverage that dims this much. */
        const int lum_f = f[0] * 2 + f[1] * 5 + f[2];
        const int lum_w = world[0] * 2 + world[1] * 5 + world[2];
        if (lum_w > 64 && lum_f <= lum_w) {
            out[3] = (uint8_t)(255 - 255 * lum_f / lum_w);
            return;
        }
    }

    out[0] = (uint8_t)f[0];
    out[1] = (uint8_t)f[1];
    out[2] = (uint8_t)f[2];
    out[3] = 255;
}

typedef struct Box {
    int x0, y0, x1, y1;      /* picture coordinates, exclusive end */
} Box;

static Box InCast(Box box) {
    if (box.x0 < s_cast.x) box.x0 = s_cast.x;
    if (box.y0 < s_cast.y) box.y0 = s_cast.y;
    if (box.x1 > s_cast.x + s_cast.width) box.x1 = s_cast.x + s_cast.width;
    if (box.y1 > s_cast.y + s_cast.height) box.y1 = s_cast.y + s_cast.height;
    return box;
}

/* Tight bounds of the opaque actor pixels inside `box`. */
static bool TightBox(Box box, Box *tight) {
    box = InCast(box);
    Box found = {INT16_MAX, INT16_MAX, INT16_MIN, INT16_MIN};
    for (int y = box.y0; y < box.y1; y++) {
        const uint8_t *row = ActorAt(box.x0, y);
        for (int x = box.x0; x < box.x1; x++, row += 4) {
            if (!row[3])
                continue;
            if (x < found.x0) found.x0 = x;
            if (x >= found.x1) found.x1 = x + 1;
            if (y < found.y0) found.y0 = y;
            if (y >= found.y1) found.y1 = y + 1;
        }
    }
    if (found.x1 == INT16_MIN)
        return false;
    *tight = found;
    return true;
}

/* While commands are chosen each party member is drawn inside its own
 * status card: a portrait on a panel, not someone standing in the room. The
 * panel shows around the sprite as interface coverage. Battle only, where
 * the actor picture is the frame. */
static bool OnPanel(const Lufia2Hd2dScene *scene, Box box) {
    unsigned around = 0, panel = 0;
    for (int y = box.y0; y < box.y1; y++) {
        const size_t row = (size_t)y * scene->width;
        for (int x = box.x0; x < box.x1; x++) {
            if (ActorAt(x, y)[3])
                continue;
            around++;
            if (s_interface[(row + (size_t)x) * 4 + 3] >= PANEL_ALPHA)
                panel++;
        }
    }
    return around && panel * 2u >= around;
}

/* Hands the actor pixels in `box` over to the interface, as they are. */
static void MoveToInterface(const Lufia2Hd2dScene *scene, Box box) {
    for (int y = box.y0; y < box.y1; y++) {
        const size_t row = (size_t)y * scene->width;
        for (int x = box.x0; x < box.x1; x++) {
            uint8_t *actor = ActorAt(x, y);
            if (!actor[3])
                continue;
            memcpy(s_interface + (row + (size_t)x) * 4, actor, 4);
            memset(actor, 0, 4);
        }
    }
}

/* Takes the actor pixels in `box` out of the scene. */
static void Remove(Box box) {
    box = InCast(box);
    for (int y = box.y0; y < box.y1; y++)
        memset(ActorAt(box.x0, y), 0, (size_t)(box.x1 - box.x0) * 4u);
}

/* Draws one OAM entry at brightness `level` into a picture whose rows are
 * `pitch` pixels apart and whose first pixel is the frame's column `left`,
 * row `top`. */
static void PutSprite(const Lufia2Hd2dFrame *frame,
                      const Lufia2Hd2dSprite *sprite, unsigned level,
                      uint8_t *rgba, size_t pitch, int left, int top,
                      int width, int height) {
    enum { WORDS = 16 };
    const uint16_t *colours =
        frame->sprite_cgram + (unsigned)sprite->palette * 16u;
    const unsigned chars = frame->sprite_chars[(sprite->tile >> 8) & 1u];
    const int at_x = (int)frame->native_x + sprite->x - left;
    const int at_y = sprite->y - top;
    for (int y = 0; y < sprite->height; y++) {
        const int row = at_y + y;
        if (row < 0 || row >= height)
            continue;
        const unsigned sy = (unsigned)(
            sprite->flip & 2u ? sprite->height - 1 - y : y);
        for (int x = 0; x < sprite->width; x++) {
            const int column = at_x + x;
            if (column < 0 || column >= width)
                continue;
            const unsigned sx = (unsigned)(
                sprite->flip & 1u ? sprite->width - 1 - x : x);
            /* Characters lie sixteen to a row; a sprite's go across and
             * down from its first, each way wrapping on its own. */
            const unsigned tile =
                ((sprite->tile + (sx >> 3)) & 0x0fu) |
                (((sprite->tile >> 4) + (sy >> 3)) & 0x0fu) << 4;
            const unsigned at = chars + tile * WORDS + (sy & 7u);
            const uint16_t low = frame->vram[at & 0x7fffu];
            const uint16_t high = frame->vram[(at + 8u) & 0x7fffu];
            const unsigned bit = 7u - (sx & 7u);
            const unsigned index =
                ((low >> bit) & 1u) | (((low >> (8u + bit)) & 1u) << 1) |
                (((high >> bit) & 1u) << 2) |
                (((high >> (8u + bit)) & 1u) << 3);
            if (!index)
                continue;
            const unsigned colour = colours[index];
            uint8_t *out = rgba + ((size_t)row * pitch + (size_t)column) * 4u;
            out[0] = (uint8_t)((colour & 0x1fu) * 255u / 31u * level / 15u);
            out[1] = (uint8_t)(((colour >> 5) & 0x1fu) * 255u / 31u * level /
                               15u);
            out[2] = (uint8_t)(((colour >> 10) & 0x1fu) * 255u / 31u * level /
                               15u);
            out[3] = 255;
        }
    }
}

/* A field's sprites, drawn from OAM: the last entry first, so that the
 * first ends up on top, as on the PPU. */
static void DrawSprites(const Lufia2Hd2dFrame *frame) {
    if (!frame->vram)
        return;
    for (unsigned n = frame->sprite_count; n-- > 0;)
        PutSprite(frame, &frame->sprites[n], frame->brightness, s_actors,
                  (size_t)s_cast.width, s_cast.x, s_cast.y, s_cast.width,
                  s_cast.height);
}

static void AddActor(Lufia2Hd2dScene *scene, Lufia2Hd2dActorKind kind,
                     Box box) {
    if (scene->actor_count == LUFIA2_HD2D_MAX_ACTORS)
        return;
    Lufia2Hd2dActor *actor = &scene->actor[scene->actor_count++];
    actor->kind = kind;
    actor->x0 = (int16_t)box.x0;
    actor->y0 = (int16_t)box.y0;
    actor->x1 = (int16_t)box.x1;
    actor->y1 = (int16_t)box.y1;
    actor->foot = (int16_t)box.y1;
}

static unsigned Root(uint8_t *parent, unsigned i) {
    while (parent[i] != i) {
        parent[i] = parent[parent[i]];
        i = parent[i];
    }
    return i;
}

/* Is sprite `index` the tile `column`, `row` of the grid that starts at
 * sprite `first`? */
static bool GridMate(const Lufia2Hd2dFrame *frame, const bool *used,
                     unsigned first, unsigned index, unsigned column,
                     unsigned row) {
    if (index >= frame->sprite_count || !used[index])
        return false;
    const Lufia2Hd2dSprite *a = &frame->sprites[first];
    const Lufia2Hd2dSprite *b = &frame->sprites[index];
    return b->slot == a->slot + (index - first) && b->width == a->width &&
           b->height == a->height && b->palette == a->palette &&
           b->priority == a->priority &&
           b->x == a->x + (int)(column * a->width) &&
           b->y == a->y + (int)(row * a->height);
}

/* Number of sprites, from `first` on, that form one row-major tile grid. */
static unsigned GridTiles(const Lufia2Hd2dFrame *frame, const bool *used,
                          unsigned first) {
    unsigned columns = 1;
    while (GridMate(frame, used, first, first + columns, columns, 0))
        columns++;
    unsigned rows = 1;
    for (;;) {
        unsigned column = 0;
        while (column < columns &&
               GridMate(frame, used, first, first + rows * columns + column,
                        column, rows))
            column++;
        if (column < columns)
            break;
        rows++;
    }
    return columns * rows;
}

static void BuildActors(const Lufia2Hd2dFrame *frame,
                        Lufia2Hd2dScene *scene) {
    const int native_x = (int)frame->native_x;
    scene->actor_count = 0;

    /* Enemies: one billboard per battle record, whichever layer holds it. */
    for (unsigned i = 0; i < frame->enemy_count; i++) {
        const Lufia2Hd2dEnemy *enemy = &frame->enemies[i];
        const Box box = {
            native_x + enemy->x - ENEMY_SLACK, enemy->y - ENEMY_SLACK,
            native_x + enemy->x + enemy->width + ENEMY_SLACK,
            enemy->y + enemy->height + ENEMY_SLACK,
        };
        Box tight;
        if (TightBox(box, &tight))
            AddActor(scene, LUFIA2_HD2D_ACTOR_ENEMY, tight);
    }
    const unsigned enemy_actors = scene->actor_count;

    /* Everything else on OBJ. A body is a grid of equal tiles in
     * consecutive OAM slots, row by row, and that grid is one object however
     * closely the next one stands: party members line up shoulder to
     * shoulder. Loose tiles -- damage digits, a cursor -- join whatever
     * loose tile they touch. */
    uint8_t parent[LUFIA2_HD2D_OAM_SLOTS];
    bool used[LUFIA2_HD2D_OAM_SLOTS], gridded[LUFIA2_HD2D_OAM_SLOTS];
    for (unsigned i = 0; i < frame->sprite_count; i++) {
        const Lufia2Hd2dSprite *sprite = &frame->sprites[i];
        parent[i] = (uint8_t)i;
        gridded[i] = false;
        used[i] = !InEnemyRect(frame, sprite->x + sprite->width / 2,
                               sprite->y + sprite->height / 2, ENEMY_SLACK);
    }
    for (unsigned i = 0; i < frame->sprite_count;) {
        const unsigned tiles = used[i] ? GridTiles(frame, used, i) : 1u;
        for (unsigned k = 0; tiles > 1u && k < tiles; k++) {
            parent[i + k] = (uint8_t)i;
            gridded[i + k] = true;
        }
        i += tiles;
    }
    for (unsigned i = 0; i < frame->sprite_count; i++) {
        if (!used[i] || gridded[i]) continue;
        const Lufia2Hd2dSprite *a = &frame->sprites[i];
        for (unsigned j = i + 1; j < frame->sprite_count; j++) {
            if (!used[j] || gridded[j]) continue;
            const Lufia2Hd2dSprite *b = &frame->sprites[j];
            if (a->x <= b->x + b->width && b->x <= a->x + a->width &&
                a->y <= b->y + b->height && b->y <= a->y + a->height)
                parent[Root(parent, j)] = (uint8_t)Root(parent, i);
        }
    }
    for (unsigned i = 0; i < frame->sprite_count; i++) {
        if (!used[i] || Root(parent, i) != i)
            continue;
        Box cluster = {0, 0, 0, 0};
        bool any = false, hero = false;
        for (unsigned j = i; j < frame->sprite_count; j++) {
            if (!used[j] || Root(parent, j) != i)
                continue;
            const Lufia2Hd2dSprite *sprite = &frame->sprites[j];
            const Box box = {native_x + sprite->x, sprite->y,
                             native_x + sprite->x + sprite->width,
                             sprite->y + sprite->height};
            Box tight;
            if (!TightBox(box, &tight))
                continue;
            if (!any) {
                cluster = tight;
                any = true;
            } else {
                if (tight.x0 < cluster.x0) cluster.x0 = tight.x0;
                if (tight.y0 < cluster.y0) cluster.y0 = tight.y0;
                if (tight.x1 > cluster.x1) cluster.x1 = tight.x1;
                if (tight.y1 > cluster.y1) cluster.y1 = tight.y1;
            }
            /* Battle keeps the party on the upper palettes; on a map
             * whatever is drawn stands on the ground. */
            if (sprite->palette >= HERO_FIRST_PALETTE ||
                frame->kind != LUFIA2_HD2D_BATTLE)
                hero = true;
        }
        if (!any)
            continue;
        /* Party members use the upper palettes; the cursor, damage digits
         * and effect sprites do not, and neither stands on anything. */
        const bool floater =
            !hero || cluster.y1 - cluster.y0 <= FLOATER_MAX_HEIGHT;
        if (!floater && frame->kind == LUFIA2_HD2D_BATTLE) {
            /* A battle comes up with the party standing where its status
             * cards are about to be drawn. Until they are, it is nowhere:
             * not yet in a card and not out on the floor. */
            if (frame->battle_opening) {
                Remove(cluster);
                continue;
            }
            if (OnPanel(scene, cluster)) {
                MoveToInterface(scene, cluster);
                continue;
            }
        }
        AddActor(scene, floater ? LUFIA2_HD2D_ACTOR_FLOATER
                                : LUFIA2_HD2D_ACTOR_HERO, cluster);
    }

    /* World pixels nobody claimed, such as an effect drawn on BG2, still
     * have to be drawn: one catch-all billboard. */
    {
        Box rest = {INT16_MAX, INT16_MAX, INT16_MIN, INT16_MIN};
        for (int y = s_cast.y; y < s_cast.y + s_cast.height; y++) {
            const uint8_t *row = ActorAt(s_cast.x, y);
            for (int x = s_cast.x; x < s_cast.x + s_cast.width;
                 x++, row += 4) {
                if (!row[3])
                    continue;
                bool covered = false;
                for (unsigned i = 0; i < scene->actor_count && !covered; i++) {
                    const Lufia2Hd2dActor *actor = &scene->actor[i];
                    covered = x >= actor->x0 && x < actor->x1 &&
                              y >= actor->y0 && y < actor->y1;
                }
                if (covered)
                    continue;
                if (x < rest.x0) rest.x0 = x;
                if (x >= rest.x1) rest.x1 = x + 1;
                if (y < rest.y0) rest.y0 = y;
                if (y >= rest.y1) rest.y1 = y + 1;
            }
        }
        if (rest.x1 != INT16_MIN)
            AddActor(scene, LUFIA2_HD2D_ACTOR_FLOATER, rest);
    }

    /* A floater takes the footing of whoever it hovers by. */
    for (unsigned i = enemy_actors; i < scene->actor_count; i++) {
        Lufia2Hd2dActor *floater = &scene->actor[i];
        if (floater->kind != LUFIA2_HD2D_ACTOR_FLOATER)
            continue;
        const int cx = (floater->x0 + floater->x1) / 2;
        const int cy = (floater->y0 + floater->y1) / 2;
        long best = -1;
        int foot = floater->y1 + 16;
        for (unsigned j = 0; j < scene->actor_count; j++) {
            const Lufia2Hd2dActor *other = &scene->actor[j];
            if (other->kind == LUFIA2_HD2D_ACTOR_FLOATER)
                continue;
            const long dx = (other->x0 + other->x1) / 2 - cx;
            const long dy = (other->y0 + other->y1) / 2 - cy;
            const long distance = dx * dx + dy * dy;
            if (best < 0 || distance < best) {
                best = distance;
                foot = other->foot;
            }
        }
        floater->foot = (int16_t)(foot > floater->y1 ? foot : floater->y1);
    }
}

static bool Near(const uint8_t *pixel, const uint8_t *colour,
                 int tolerance) {
    return abs(pixel[0] - colour[0]) <= tolerance &&
           abs(pixel[1] - colour[1]) <= tolerance &&
           abs(pixel[2] - colour[2]) <= tolerance;
}

/* Clears an opaque plate or rule pixel and reports that it did. */
static bool Dissolve(uint8_t *pixel, const uint8_t plate[3], int tolerance) {
    static const uint8_t kBlack[3] = {0, 0, 0};
    if (pixel[3] != 255 ||
        !(Near(pixel, plate, tolerance) || Near(pixel, kBlack, tolerance)))
        return false;
    memset(pixel, 0, 4);
    return true;
}

/* The status bar that runs along the bottom while actions play out is one
 * opaque strip: a dark plate, ruled off in black, with a card per party slot
 * set into it. Over the scene only the cards are wanted, so the plate and
 * the rules are dissolved from the strip's edges inwards; each card's own
 * border is a closed ring of a lighter colour and stops the flood.
 *
 * The strip is found by what it is, rows at the bottom of the picture that
 * the interface covers from side to side: the raster state under it differs
 * between the action phase and the victory screen. */
static void CutOutStatusCards(const Lufia2Hd2dScene *scene) {
    static uint32_t stack[SCENE_PIXELS];
    static const uint8_t kBlack[3] = {0, 0, 0};
    const int width = (int)scene->width, height = (int)scene->height;
    const int y1 = height;
    int y0 = height;
    while (y0 > 0) {
        const uint8_t *row =
            s_interface + (size_t)(y0 - 1) * scene->width * 4;
        unsigned opaque = 0;
        for (int x = 0; x < width; x++)
            opaque += row[x * 4 + 3] == 255;
        if (opaque < STATUS_BAR_MIN_COLUMNS)
            break;
        y0--;
    }
    if (y1 - y0 < STATUS_BAR_MIN_ROWS)
        return;

    /* The plate colour is what the strip's second row is made of. */
    uint8_t plate[3] = {0, 0, 0};
    bool found = false;
    for (int x = 0; x < width && !found; x++) {
        const uint8_t *pixel =
            s_interface + ((size_t)(y0 + 1) * scene->width + (size_t)x) * 4;
        if (pixel[3] == 255 && !Near(pixel, kBlack, 2)) {
            memcpy(plate, pixel, 3);
            found = true;
        }
    }
    if (!found)
        return;
    /* A fade pulls every colour together; keep the card borders apart. */
    const int tolerance = 5 + (int)(scene->brightness * 6.0f);

    /* Seeds: the strip's outline, and whatever borders on open ground such
     * as the picture's windowed-off edge columns. A pixel is cleared as it
     * is queued, so it is queued once. */
    size_t count = 0;
    for (int y = y0; y < y1; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t *pixel =
                s_interface + ((size_t)y * scene->width + (size_t)x) * 4;
            const bool open = y == y0 || y == y1 - 1 || x == 0 ||
                              x == width - 1 || !pixel[-1] || !pixel[7];
            if (open && Dissolve(pixel, plate, tolerance))
                stack[count++] = (uint32_t)(y * width + x);
        }
    }
    while (count) {
        const uint32_t at = stack[--count];
        const int x = (int)(at % (uint32_t)width);
        const int y = (int)(at / (uint32_t)width);
        uint8_t *pixel = s_interface + (size_t)at * 4;
        if (x > 0 && Dissolve(pixel - 4, plate, tolerance))
            stack[count++] = at - 1u;
        if (x < width - 1 && Dissolve(pixel + 4, plate, tolerance))
            stack[count++] = at + 1u;
        if (y > y0 && Dissolve(pixel - (size_t)width * 4, plate, tolerance))
            stack[count++] = at - (uint32_t)width;
        if (y < y1 - 1 &&
            Dissolve(pixel + (size_t)width * 4, plate, tolerance))
            stack[count++] = at + (uint32_t)width;
    }
}

/* ---- field ---------------------------------------------------------- */

enum {
    FIELD_PIXELS = LUFIA2_HD2D_FIELD_MAX_WIDE * 8 *
                   LUFIA2_HD2D_FIELD_MAX_HIGH * 8,
    FIELD_TILES = LUFIA2_HD2D_FIELD_MAX_WIDE * LUFIA2_HD2D_FIELD_MAX_HIGH,
    CHARS = 1024,
    CHAR_WORDS = 16,
    TILE_NUMBER = 0x03ff,
    TILE_PRIORITY = 0x2000,
    TILE_HFLIP = 0x4000,
    TILE_VFLIP = 0x8000,
    /* Tiles a body may reach below its overhang, a house being the tallest
     * thing meant. */
    BODY_TILES = 20,
    /* Tiles below a lintel that it stands in front of: whoever walks under
     * it is that much taller than their feet. */
    HELD_TILES = 3,
    /* An overhang with nothing under it, this many tiles down a column or
     * more, is no upright thing but the top of a wall seen from above. */
    RIM_TILES = 6,
    /* Things side by side are one thing if they stand within this many
     * tiles of each other. */
    JOIN_TILES = 6,
    /* A body this many times taller than wide is no building. */
    THIN_RATIO = 3,
    /* Heights, in picture rows. Whatever blocks the way without being drawn
     * over the sprites is no taller than they are from the knee down, and
     * neither is a parapet seen from above; a room's wall top is where
     * their heads are; a tree is a tree, however many of them the picture
     * stacks up a column; forest stands a little lower than its edge. */
    LOW_HEIGHT = 16,
    RIM_HEIGHT = 16,
    WALL_HEIGHT = 32,
    FIGURE_HEIGHT = 48,
    /* A building is a box two storeys high: its walls rise from where they
     * stand, a row for a row, and what is drawn above that is its roof,
     * lying on top. How deep the roof goes and how high its ridge is the
     * picture does not say, and the game blocks the whole of it alike. */
    BUILDING_HEIGHT = 48,
    CANOPY_HEIGHT = 24,
    FLAT_TOP = 255,              /* base row of a thing of one height */
    RAMP = 254,                  /* base row of a room's wall: see s_ramp */
    /* A room's wall rises a row for each row it lies north of the floor,
     * which is how its face is drawn, and this many for each pixel it lies
     * to the side of the floor or south of it, where it is seen from above
     * and only its foot shows. */
    WALL_SLOPE = 2,
};

static uint8_t s_lower[FIELD_PIXELS * 4];
static uint8_t s_upper[FIELD_PIXELS * 4];
static uint8_t s_relief[FIELD_PIXELS * 4];
/* Height of a room's walls at each pixel of the window. */
static uint8_t s_ramp[FIELD_PIXELS];
static uint8_t s_floor[FIELD_TILES];

/* A layer's characters, unpacked as they are first asked for. */
typedef struct CharSet {
    uint16_t base;
    uint8_t known[CHARS];        /* 1 read, 2 has picture, 4 one colour */
    uint8_t pixels[CHARS][64];
} CharSet;

static CharSet s_chars[2];

static const uint8_t *Char(CharSet *set, const uint16_t *vram,
                           unsigned tile) {
    uint8_t *pixels = set->pixels[tile];
    if (set->known[tile])
        return pixels;
    const unsigned at = set->base + tile * CHAR_WORDS;
    uint8_t any = 0, plain = 4;
    for (unsigned row = 0; row < 8u; row++) {
        const uint16_t low = vram[(at + row) & 0x7fffu];
        const uint16_t high = vram[(at + 8u + row) & 0x7fffu];
        for (unsigned x = 0; x < 8u; x++) {
            const unsigned bit = 7u - x;
            const uint8_t index = (uint8_t)(
                ((low >> bit) & 1u) | (((low >> (8u + bit)) & 1u) << 1) |
                (((high >> bit) & 1u) << 2) |
                (((high >> (8u + bit)) & 1u) << 3));
            pixels[row * 8u + x] = index;
            if (index) any = 2;
            if (index != pixels[0]) plain = 0;
        }
    }
    set->known[tile] = (uint8_t)(1u | any | plain);
    return pixels;
}

/* What one layer shows in one tile of the window. */
typedef struct TileLook {
    bool drawn;                  /* shows anything at all */
    bool figure;                 /* picture, not a tile of one flat colour */
    bool high;                   /* and drawn over the sprites */
} TileLook;

typedef struct FieldWork {
    const Lufia2Hd2dField *field;
    const uint16_t *vram;
    CharSet *lower_chars, *upper_chars;
    unsigned wide, high;
    bool rooms;
} FieldWork;

static TileLook Look(const FieldWork *work, bool upper, unsigned tx,
                     unsigned ty) {
    const Lufia2Hd2dField *field = work->field;
    const uint16_t entry =
        (upper ? field->upper : field->lower)[(size_t)ty * work->wide + tx];
    CharSet *set = upper ? work->upper_chars : work->lower_chars;
    (void)Char(set, work->vram, entry & TILE_NUMBER);
    const uint8_t known = set->known[entry & TILE_NUMBER];
    /* The lower layer is opaque where it has no picture: the backdrop. */
    TileLook look;
    look.drawn = upper ? (known & 2u) != 0 : true;
    look.figure = !(known & 4u) && look.drawn;
    look.high = look.figure && (entry & TILE_PRIORITY) != 0;
    return look;
}

static uint8_t Facts(const FieldWork *work, unsigned tx, unsigned ty) {
    return work->field->facts[(size_t)ty * work->wide + tx];
}

static bool Blocked(const FieldWork *work, unsigned tx, unsigned ty) {
    return !(Facts(work, tx, ty) & LUFIA2_HD2D_TILE_OPEN);
}

/* In a room the walls are drawn on the upper layer, as beams and posts seen
 * from above and as faces with windows in them, and the same pieces serve
 * wherever a wall runs, over the edge of the floor included. They are the
 * wall, whatever cell they are in. */
static bool UpperWall(const FieldWork *work, unsigned tx, unsigned ty) {
    return work->rooms && (Facts(work, tx, ty) & LUFIA2_HD2D_TILE_WALL) &&
           Look(work, true, tx, ty).drawn;
}

/* A tile as something that may stand on its own: a wall is not. */
static TileLook Thing(const FieldWork *work, bool upper, unsigned tx,
                      unsigned ty) {
    TileLook look = Look(work, upper, tx, ty);
    if (upper && UpperWall(work, tx, ty))
        look.figure = look.high = false;
    return look;
}

/* The window row a body in the lower layer ends on, coming down from an
 * overhang that ends on `from`; `from` itself when nothing there is one. A
 * body is the solid thing the overhang belongs to -- the house under its
 * ridge, the wall under its coping -- so it ends at ground one can walk on,
 * at water, at the void, or at a plain surface. What the upper layer hangs
 * on it, a sign or a shutter, is part of it. */
static unsigned BodyEnd(const FieldWork *work, unsigned tx, unsigned from) {
    for (unsigned ty = from; ty < work->high; ty++) {
        if (ty - from >= BODY_TILES)
            break;
        if (Facts(work, tx, ty) &
            (LUFIA2_HD2D_TILE_OPEN | LUFIA2_HD2D_TILE_HIDDEN |
             LUFIA2_HD2D_TILE_GROUND | LUFIA2_HD2D_TILE_TEXTURE |
             LUFIA2_HD2D_TILE_LIQUID))
            return ty;
    }
    return from;
}

/* How one layer's tile rises: its picture is a face standing on window row
 * `base`, a row of picture to a row of height, and no higher than `cap`.
 * Base zero lies flat; FLAT_TOP is all of one height, `cap`. */
typedef struct Rise {
    uint8_t base, cap;
} Rise;

static Rise s_rise[2][FIELD_TILES];      /* upper, lower */

typedef enum RunKind {
    RUN_LOW,                     /* blocks the way, under the sprites */
    RUN_BODY,                    /* an overhang and the body under it */
    RUN_FIGURE,                  /* an overhang the upper layer holds up */
    RUN_LOOSE,                   /* an overhang with nothing under it */
    RUN_RIM,                     /* the top of a wall, seen from above */
} RunKind;

/* A run of tiles down one column that belongs together. */
typedef struct Run {
    uint16_t tx;
    uint8_t top, over_end, foot;
    uint8_t base;                /* row its face stands on */
    uint8_t kind;
    bool settled;
    uint16_t parent;
} Run;

static Run s_runs[FIELD_TILES / 2];
static uint16_t s_column_runs[LUFIA2_HD2D_FIELD_MAX_WIDE + 2];

static unsigned RunRoot(unsigned i) {
    while (s_runs[i].parent != i) {
        s_runs[i].parent = s_runs[s_runs[i].parent].parent;
        i = s_runs[i].parent;
    }
    return i;
}

/* Rows two runs in neighbouring columns share. */
static int RunOverlap(const Run *a, const Run *b) {
    const int top = a->top > b->top ? a->top : b->top;
    const int foot = a->foot < b->foot ? a->foot : b->foot;
    return foot - top;
}

/* A room's walls. Everything that is not floor is wall, as high as the
 * sprites. Where the floor lies south of it a wall shows its face, and
 * rises a row for each row of the picture, as it was drawn. Elsewhere it is
 * seen from above. What the lower layer draws of it, a cave's ragged edge,
 * slopes up from the floor, so that it leans as a wall does and stays in
 * one piece; the upper layer's beams and posts, and the dark beyond them,
 * are level at the wall's height. */
static void FindWalls(const FieldWork *work) {
    const unsigned wide = work->wide, high = work->high;
    const unsigned pixels_wide = wide * 8u, pixels_high = high * 8u;

    for (unsigned ty = 0; ty < high; ty++)
        for (unsigned tx = 0; tx < wide; tx++) {
            const uint8_t facts = Facts(work, tx, ty);
            s_floor[(size_t)ty * wide + tx] =
                !(facts & LUFIA2_HD2D_TILE_HIDDEN) &&
                (facts & (LUFIA2_HD2D_TILE_OPEN | LUFIA2_HD2D_TILE_GROUND |
                          LUFIA2_HD2D_TILE_LIQUID)) &&
                !UpperWall(work, tx, ty);
        }

    /* Height of the slope at every pixel: the least, over the floor, of
     * what it costs to come from there. Across, then up and down. */
    for (unsigned y = 0; y < pixels_high; y++) {
        uint8_t *row = s_ramp + (size_t)y * pixels_wide;
        const uint8_t *floor = s_floor + (size_t)(y >> 3) * wide;
        unsigned height = 255;
        for (unsigned x = 0; x < pixels_wide; x++) {
            height = floor[x >> 3] ? 0u : height + WALL_SLOPE;
            if (height > 255u) height = 255u;
            row[x] = (uint8_t)height;
        }
        height = 255;
        for (unsigned x = pixels_wide; x-- > 0;) {
            height = floor[x >> 3] ? 0u : height + WALL_SLOPE;
            if (height > 255u) height = 255u;
            if (height < row[x]) row[x] = (uint8_t)height;
        }
    }
    for (unsigned x = 0; x < pixels_wide; x++) {
        unsigned height = 255;
        for (unsigned y = pixels_high; y-- > 0;) {
            uint8_t *at = s_ramp + (size_t)y * pixels_wide + x;
            height = height + 1u < *at ? height + 1u : *at;
            *at = (uint8_t)height;
        }
        height = 255;
        for (unsigned y = 0; y < pixels_high; y++) {
            uint8_t *at = s_ramp + (size_t)y * pixels_wide + x;
            height = height + WALL_SLOPE < *at ? height + WALL_SLOPE : *at;
            *at = (uint8_t)height;
        }
    }

    /* A beam that runs over the edge of the floor hides the floor under
     * it, and the map draws nothing there. Seen from another place some of
     * it shows, so the floor beside it is carried on beneath. */
    for (unsigned ty = 0; ty < high; ty++)
        for (unsigned tx = 0; tx < wide; tx++) {
            const uint8_t facts = Facts(work, tx, ty);
            if (!UpperWall(work, tx, ty) || Look(work, false, tx, ty).figure ||
                (facts & LUFIA2_HD2D_TILE_HIDDEN) ||
                !(facts & (LUFIA2_HD2D_TILE_OPEN | LUFIA2_HD2D_TILE_GROUND)))
                continue;
            const struct { int x, y; } from[4] = {
                {(int)tx, (int)ty - 1}, {(int)tx - 1, (int)ty},
                {(int)tx + 1, (int)ty}, {(int)tx, (int)ty + 1},
            };
            for (unsigned i = 0; i < 4u; i++) {
                if (from[i].x < 0 || from[i].y < 0 ||
                    from[i].x >= (int)wide || from[i].y >= (int)high)
                    continue;
                /* A floor tile, or one just carried on from above. */
                const bool floor =
                    s_floor[(size_t)from[i].y * wide + (size_t)from[i].x];
                const bool carried =
                    i == 0 && UpperWall(work, tx, ty - 1u) &&
                    !Look(work, false, tx, ty - 1u).figure &&
                    (Facts(work, tx, ty - 1u) &
                     (LUFIA2_HD2D_TILE_OPEN | LUFIA2_HD2D_TILE_GROUND));
                if (!floor && !carried)
                    continue;
                for (unsigned y = 0; y < 8u; y++)
                    memcpy(s_lower + (((size_t)ty * 8u + y) * wide * 8u +
                                      (size_t)tx * 8u) * 4u,
                           s_lower + (((size_t)from[i].y * 8u + y) * wide *
                                          8u + (size_t)from[i].x * 8u) * 4u,
                           8u * 4u);
                break;
            }
        }

    for (unsigned tx = 0; tx < wide; tx++) {
        unsigned base = FLAT_TOP;
        for (unsigned ty = high; ty-- > 0;) {
            const size_t at = (size_t)ty * wide + tx;
            if (s_floor[at]) {
                base = ty;
                continue;
            }
            if (UpperWall(work, tx, ty)) {
                s_rise[0][at].base = (uint8_t)base;
                s_rise[0][at].cap = WALL_HEIGHT;
            }
            /* Under a beam that runs over the floor's edge the floor is
             * floor still. */
            if (Facts(work, tx, ty) &
                    (LUFIA2_HD2D_TILE_OPEN | LUFIA2_HD2D_TILE_GROUND |
                     LUFIA2_HD2D_TILE_LIQUID) &&
                !(Facts(work, tx, ty) & LUFIA2_HD2D_TILE_HIDDEN))
                continue;
            /* A slope carries what is drawn on it. The dark beyond a
             * built room's walls has nothing drawn on it, and is simply
             * as high as the walls. */
            if (!s_rise[1][at].base) {
                s_rise[1][at].base =
                    Look(work, false, tx, ty).figure ? RAMP : FLAT_TOP;
                s_rise[1][at].cap = WALL_HEIGHT;
            }
            /* What else the upper layer lays on a wall lies as the wall
             * does. */
            if (!s_rise[0][at].base && Look(work, true, tx, ty).drawn)
                s_rise[0][at] = s_rise[1][at];
        }
    }
}

/* How high everything on the map is. The picture is drawn the way every map
 * of the game is: a step back on the ground and a step up are both one row
 * of the screen. So each tile is either ground, at no height, or part of
 * something that rises from a row further down the screen; saying which row
 * is saying how tall and how far away each of its pixels is.
 *
 * The game itself tells the important part. It draws over its sprites
 * exactly what someone can walk behind -- a roof's ridge, a tree's crown, a
 * wall's coping -- so every such overhang stands on something further south:
 *
 *  - a body in the lower layer, down to ground one can walk on: a house
 *    under its ridge, a bridge's face under its rail;
 *  - or the rest of the same thing on the upper layer, down to where it
 *    stops blocking the way: a trunk, the posts of a gate;
 *  - or nothing. Then it is part of whatever stands beside it, the rim of a
 *    crown or the eaves of a roof; failing that it stands on its own lower
 *    edge, or, over a way through, hangs as far in front of whoever walks
 *    under it as they are tall. A long one is the top of a wall.
 *
 * Bodies side by side are one building and share its foremost base, so its
 * picture stays whole; so are trunks and posts. What blocks the way but is
 * drawn under the sprites is low: furniture, barrels, crates. And in a room
 * everything outside the floor is wall, as high as the sprites, its face
 * coming down to the floor where the floor lies in front of it. Returns the
 * greatest height, in rows. */
static unsigned FindRelief(const FieldWork *work, bool enclosed) {
    const unsigned wide = work->wide, high = work->high;
    unsigned runs = 0;
    memset(s_rise, 0, sizeof s_rise);
    for (unsigned tx = 0; tx < wide; tx++) {
        s_column_runs[tx] = (uint16_t)runs;
        for (unsigned ty = 0; ty < high;) {
            const unsigned top = ty;
            while (ty < high && (Thing(work, true, tx, ty).high ||
                                 Thing(work, false, tx, ty).high))
                ty++;
            const unsigned over_end = ty;
            while (ty < high && Blocked(work, tx, ty)) {
                const TileLook upper = Thing(work, true, tx, ty);
                if (!upper.figure || upper.high)
                    break;
                ty++;
            }
            if (ty == top) {
                ty++;
                continue;
            }
            if (runs == sizeof s_runs / sizeof s_runs[0])
                break;
            Run *run = &s_runs[runs];
            run->tx = (uint16_t)tx;
            run->top = (uint8_t)top;
            run->over_end = (uint8_t)over_end;
            run->foot = run->base = (uint8_t)ty;
            run->kind = RUN_LOW;
            run->settled = true;
            run->parent = (uint16_t)runs;
            if (over_end > top && ty > over_end) {
                run->kind = RUN_FIGURE;
            } else if (over_end > top) {
                /* In a room what is under a coping is the wall, which rises
                 * with every other wall below. */
                const unsigned end =
                    enclosed ? over_end : BodyEnd(work, tx, over_end);
                if (end > over_end) {
                    run->kind = RUN_BODY;
                    run->foot = run->base = (uint8_t)end;
                } else if (enclosed || over_end - top >= RIM_TILES) {
                    run->kind = RUN_RIM;
                } else {
                    /* Its own lower edge, unless someone can walk through
                     * under it: from its cell to the one south of it, or,
                     * where it ends halfway down a cell, in that cell. */
                    const uint8_t over = Facts(work, tx, over_end - 1u);
                    const bool through =
                        (over & LUFIA2_HD2D_TILE_OPEN) && over_end < high &&
                        !Blocked(work, tx, over_end) &&
                        ((over_end & 1u) ||
                         (over & LUFIA2_HD2D_TILE_THROUGH));
                    run->kind = RUN_LOOSE;
                    run->settled = false;
                    if (through)
                        run->base = (uint8_t)(over_end + HELD_TILES);
                }
            }
            runs++;
            ty = run->foot > top ? run->foot : top + 1u;
        }
    }
    s_column_runs[wide] = s_column_runs[wide + 1u] = (uint16_t)runs;

    /* Bodies that touch side by side and stand about as far south are one
     * building, on its foremost base; trunks and posts likewise. */
    for (unsigned tx = 0; tx + 1u < wide; tx++)
        for (unsigned i = s_column_runs[tx]; i < s_column_runs[tx + 1u]; i++)
            for (unsigned j = s_column_runs[tx + 1u];
                 j < s_column_runs[tx + 2u]; j++) {
                const Run *a = &s_runs[i], *b = &s_runs[j];
                const int apart = (int)a->base - (int)b->base;
                if ((a->kind == RUN_BODY || a->kind == RUN_FIGURE) &&
                    a->kind == b->kind && RunOverlap(a, b) > 0 &&
                    apart <= JOIN_TILES && apart >= -JOIN_TILES)
                    s_runs[RunRoot(j)].parent = (uint16_t)RunRoot(i);
            }
    for (unsigned i = 0; i < runs; i++) {
        Run *root = &s_runs[RunRoot(i)];
        if (s_runs[i].base > root->base)
            root->base = s_runs[i].base;
    }
    /* A building is about as wide as it is tall. A body many times taller
     * than it is wide is a wall running away from the viewer, seen from
     * above: the side of a canal under its corner post. It lies flat, and
     * the post stands for itself. */
    {
        static uint8_t left[FIELD_TILES / 2], right[FIELD_TILES / 2];
        static uint8_t over[FIELD_TILES / 2];
        for (unsigned i = 0; i < runs; i++) {
            left[i] = 255;
            right[i] = 0;
            over[i] = 255;
        }
        for (unsigned i = 0; i < runs; i++) {
            const unsigned root = RunRoot(i);
            const Run *run = &s_runs[i];
            if (run->tx < left[root]) left[root] = (uint8_t)run->tx;
            if (run->tx > right[root]) right[root] = (uint8_t)run->tx;
            if (run->over_end < over[root]) over[root] = run->over_end;
        }
        for (unsigned i = 0; i < runs; i++) {
            const unsigned root = RunRoot(i);
            Run *run = &s_runs[i];
            if (run->kind != RUN_BODY ||
                s_runs[root].base - over[root] <
                    THIN_RATIO * (right[root] - left[root] + 1u))
                continue;
            run->kind = RUN_LOOSE;
            run->settled = false;
            run->foot = run->over_end;
            run->base = run->over_end;
        }
    }
    /* Trunks and posts that touch stand in one row. A building's walls
     * each stand where they stand: a wing set back meets the ground further
     * back, and the roof, level, joins them. */
    for (unsigned i = 0; i < runs; i++)
        if (s_runs[i].kind == RUN_FIGURE)
            s_runs[i].base = s_runs[RunRoot(i)].base;
        else if (s_runs[i].kind == RUN_BODY)
            s_runs[i].base = s_runs[i].foot;

    /* An overhang with nothing under it belongs to what stands beside it,
     * and what belongs to that in turn: outwards from the things that
     * stand, a column at a time. */
    for (unsigned pass = 0; pass < 6u; pass++) {
        bool settled_any = false;
        for (unsigned n = 0; n < runs; n++) {
            const unsigned i = pass & 1u ? runs - 1u - n : n;
            Run *run = &s_runs[i];
            if (run->settled)
                continue;
            const Run *best = NULL;
            int most = 0;
            for (int side = -1; side <= 1; side += 2) {
                const int tx = (int)run->tx + side;
                if (tx < 0 || tx >= (int)wide)
                    continue;
                for (unsigned j = s_column_runs[tx];
                     j < s_column_runs[tx + 1]; j++) {
                    const Run *other = &s_runs[j];
                    const int overlap = RunOverlap(run, other);
                    if (!other->settled || other->kind == RUN_LOW ||
                        other->kind == RUN_RIM || overlap <= most ||
                        other->base < run->over_end)
                        continue;
                    best = other;
                    most = overlap;
                }
            }
            if (best) {
                run->base = best->base;
                run->kind = best->kind;
                run->settled = true;
                settled_any = true;
            }
        }
        if (!settled_any)
            break;
    }

    for (unsigned i = 0; i < runs; i++) {
        const Run *run = &s_runs[i];
        const unsigned tx = run->tx;
        for (unsigned y = run->top; y < run->foot; y++) {
            const size_t at = (size_t)y * wide + tx;
            Rise rise;
            if (run->kind == RUN_LOW ||
                (run->kind == RUN_RIM && y >= run->over_end)) {
                rise.base = run->foot;
                rise.cap = LOW_HEIGHT;
            } else if (run->kind == RUN_RIM) {
                rise.base = FLAT_TOP;
                rise.cap = enclosed ? WALL_HEIGHT : RIM_HEIGHT;
            } else {
                rise.base = run->base;
                rise.cap = run->kind == RUN_FIGURE ? FIGURE_HEIGHT
                                                   : BUILDING_HEIGHT;
            }
            if (Thing(work, true, tx, y).figure)
                s_rise[0][at] = rise;
            /* Under a body's overhang the lower layer shows more of the
             * body, the roof below its ridge; under a tree's, the ground. */
            if (run->kind == RUN_BODY
                    ? y >= run->over_end ||
                      !(Facts(work, tx, y) & (LUFIA2_HD2D_TILE_GROUND |
                                              LUFIA2_HD2D_TILE_LIQUID |
                                              LUFIA2_HD2D_TILE_HIDDEN))
                    : Thing(work, false, tx, y).high)
                s_rise[1][at] = rise;
        }
    }

    if (enclosed) {
        FindWalls(work);
    } else {
        /* What is left of the lower layer and blocks the way is forest. Up
         * each column, it rises from where the open ground in front of it
         * ends. */
        for (unsigned tx = 0; tx < wide; tx++) {
            unsigned base = FLAT_TOP;
            for (unsigned ty = high; ty-- > 0;) {
                const size_t at = (size_t)ty * wide + tx;
                const uint8_t facts = Facts(work, tx, ty);
                if ((facts & (LUFIA2_HD2D_TILE_HIDDEN | LUFIA2_HD2D_TILE_OPEN |
                              LUFIA2_HD2D_TILE_GROUND |
                              LUFIA2_HD2D_TILE_LIQUID)) ||
                    !(facts & LUFIA2_HD2D_TILE_TEXTURE)) {
                    base = ty;
                    continue;
                }
                if (!s_rise[1][at].base) {
                    s_rise[1][at].base = (uint8_t)base;
                    s_rise[1][at].cap = CANOPY_HEIGHT;
                }
                /* What the upper layer lays on it lies at the same height. */
                if (!s_rise[0][at].base && Look(work, true, tx, ty).figure)
                    s_rise[0][at] = s_rise[1][at];
            }
        }
    }

    /* Height of every pixel, and whether what shows there is drawn over the
     * sprites. */
    unsigned tallest = 0;
    for (unsigned ty = 0; ty < high; ty++)
        for (unsigned tx = 0; tx < wide; tx++) {
            const size_t at = (size_t)ty * wide + tx;
            /* The PPU's order: a tile of either layer with its priority set
             * is drawn over the sprites, the lower layer's as well. */
            const bool upper_over =
                (work->field->upper[at] & TILE_PRIORITY) != 0;
            const bool lower_over =
                (work->field->lower[at] & TILE_PRIORITY) != 0;
            for (unsigned y = 0; y < 8u; y++) {
                const unsigned row = ty * 8u + y;
                uint8_t heights[2];
                for (unsigned layer = 0; layer < 2u; layer++) {
                    const Rise rise = s_rise[layer][at];
                    unsigned height = 0;
                    if (rise.base == FLAT_TOP)
                        height = rise.cap;
                    else if (rise.base == RAMP)
                        height = 255;
                    else if (rise.base * 8u > row)
                        height = rise.base * 8u - row;
                    if (height > rise.cap) height = rise.cap;
                    heights[layer] = (uint8_t)height;
                }
                /* What is drawn on a thing is no further back than it. */
                if (heights[0] < heights[1] && s_rise[1][at].base != RAMP)
                    heights[0] = heights[1];
                const bool ramp[2] = {s_rise[0][at].base == RAMP,
                                      s_rise[1][at].base == RAMP};
                uint8_t *out = s_relief + ((size_t)row * wide * 8u +
                                           (size_t)tx * 8u) * 4u;
                const uint8_t *upper = s_upper + ((size_t)row * wide * 8u +
                                                  (size_t)tx * 8u) * 4u;
                const uint8_t *slope = s_ramp + (size_t)row * wide * 8u +
                                       (size_t)tx * 8u;
                for (unsigned x = 0; x < 8u; x++, out += 4, upper += 4) {
                    out[0] = ramp[0] && slope[x] < heights[0] ? slope[x]
                                                              : heights[0];
                    out[1] = ramp[1] && slope[x] < heights[1] ? slope[x]
                                                              : heights[1];
                    out[2] = (upper[3] && upper_over) || lower_over ? 255u
                                                                    : 0u;
                    out[3] = upper[3] ? 255u : 0u;
                    if (out[0] > tallest) tallest = out[0];
                    if (out[1] > tallest) tallest = out[1];
                }
            }
        }
    return tallest;
}

static void PutTile(uint8_t *picture, unsigned wide, unsigned tx,
                    unsigned ty, const uint8_t *pixels, uint16_t entry,
                    const uint8_t palette[256][3], const uint8_t *backdrop) {
    const uint8_t (*colours)[3] = palette + ((entry >> 10) & 7u) * 16u;
    for (unsigned y = 0; y < 8u; y++) {
        const unsigned from_y = entry & TILE_VFLIP ? 7u - y : y;
        uint8_t *out = picture + (((size_t)ty * 8u + y) * wide * 8u +
                                  (size_t)tx * 8u) * 4u;
        for (unsigned x = 0; x < 8u; x++, out += 4) {
            const uint8_t index =
                pixels[from_y * 8u + (entry & TILE_HFLIP ? 7u - x : x)];
            const uint8_t *colour = index ? colours[index] : backdrop;
            if (colour) {
                out[0] = colour[0];
                out[1] = colour[1];
                out[2] = colour[2];
                out[3] = 255;
            } else {
                out[0] = out[1] = out[2] = out[3] = 0;
            }
        }
    }
}

/* A town or a dungeon: the map's two layers in colour, what of them stands,
 * the sprites, and whatever the game writes over the lot. */
static void BuildField(const Lufia2Hd2dFrame *frame, Lufia2Hd2dScene *scene) {
    const Lufia2Hd2dField *field = &frame->field;
    const unsigned width = frame->width, height = frame->height;
    const unsigned wide = field->tiles_wide, high = field->tiles_high;
    const uint32_t *bg3 = frame->layer[LUFIA2_HD2D_BG3];
    const uint32_t *obj = frame->layer[LUFIA2_HD2D_OBJ];

    uint8_t palette[256][3];
    for (unsigned i = 0; i < 256u; i++) {
        const unsigned colour = field->cgram[i];
        palette[i][0] = (uint8_t)((colour & 0x1fu) * 255u / 31u);
        palette[i][1] = (uint8_t)(((colour >> 5) & 0x1fu) * 255u / 31u);
        palette[i][2] = (uint8_t)(((colour >> 10) & 0x1fu) * 255u / 31u);
    }

    FieldWork work = {field, frame->vram, &s_chars[0], &s_chars[0], wide,
                      high, field->rooms};
    s_chars[0].base = field->lower_chars;
    memset(s_chars[0].known, 0, sizeof s_chars[0].known);
    if (field->upper_chars != field->lower_chars) {
        work.upper_chars = &s_chars[1];
        s_chars[1].base = field->upper_chars;
        memset(s_chars[1].known, 0, sizeof s_chars[1].known);
    }
    for (unsigned ty = 0; ty < high; ty++)
        for (unsigned tx = 0; tx < wide; tx++) {
            const size_t at = (size_t)ty * wide + tx;
            PutTile(s_lower, wide, tx, ty,
                    Char(work.lower_chars, frame->vram,
                         field->lower[at] & TILE_NUMBER),
                    field->lower[at], palette, palette[0]);
            PutTile(s_upper, wide, tx, ty,
                    Char(work.upper_chars, frame->vram,
                         field->upper[at] & TILE_NUMBER),
                    field->upper[at], palette, NULL);
        }

    scene->ground.lower = s_lower;
    scene->ground.upper = s_upper;
    scene->ground.relief = s_relief;
    scene->ground.tiles_wide = wide;
    scene->ground.tiles_high = high;
    scene->ground.origin_x = field->origin_x;
    scene->ground.origin_y = field->origin_y;
    scene->ground.map_x = field->map_x;
    scene->ground.map_y = field->map_y;
    memcpy(scene->ground.backdrop, palette[0], 3);
    scene->ground.enclosed = field->rooms;
    scene->ground.tallest = FindRelief(&work, field->rooms);

    /* The sprites stand on the map. They are drawn here and not taken from
     * the PPU's picture, which ends where its screen does. */
    SetCast(scene, -CAST_BESIDE, -CAST_ABOVE, (int)width + 2 * CAST_BESIDE,
            (int)height + CAST_ABOVE);
    memset(s_actors, 0, (size_t)s_cast.width * (size_t)s_cast.height * 4u);
    DrawSprites(frame);

    /* Of the picture itself the third layer is written over the map:
     * windows, text, the name of the place. Where the game blends that layer
     * in, the blend is measured against what the layers below it show,
     * sprites under the tiles drawn over them. */
    const unsigned level = frame->brightness;
    for (unsigned y = 0; y < height; y++) {
        const Lufia2Hd2dLine *line = &frame->lines[y];
        const int wy = (int)y - field->origin_y;
        for (unsigned x = 0; x < width; x++) {
            const size_t i = (size_t)y * width + x;
            const int wx = (int)x - field->origin_x;
            memset(s_interface + i * 4, 0, 4);
            if (wx < 0 || wy < 0 || wx >= (int)(wide * 8u) ||
                wy >= (int)(high * 8u))
                continue;
            const size_t tile = (size_t)(wy >> 3) * wide + (size_t)(wx >> 3);
            if (!Alpha(bg3[i]))
                continue;
            const size_t at = ((size_t)wy * wide * 8u + (size_t)wx) * 4u;
            const uint8_t *lower = s_lower + at, *upper = s_upper + at;
            const bool upper_high = upper[3] &&
                                    (field->upper[tile] & TILE_PRIORITY);
            const bool lower_high = (field->lower[tile] & TILE_PRIORITY) != 0;
            uint8_t world[3];
            if (!upper_high && !lower_high && Alpha(obj[i])) {
                world[0] = (uint8_t)(obj[i] >> 16);
                world[1] = (uint8_t)(obj[i] >> 8);
                world[2] = (uint8_t)obj[i];
            } else {
                const uint8_t *top = upper[3] && (upper_high || !lower_high)
                                         ? upper : lower;
                for (int c = 0; c < 3; c++)
                    world[c] = (uint8_t)(top[c] * level / 15u);
            }
            SolveInterface(world, frame->flat[i], bg3[i], line,
                           s_interface + i * 4);
        }
    }
    scene->background_x1 = width;
    scene->background_cover = 1.0f;
    BuildActors(frame, scene);
}

void Lufia2Hd2dBuildScene(const Lufia2Hd2dFrame *frame,
                         Lufia2Hd2dScene *scene) {
    const unsigned width = frame->width, height = frame->height;
    memset(scene, 0, sizeof *scene);
    scene->kind = frame->kind;
    scene->plane = frame->plane;
    scene->background = s_background;
    scene->actors = s_actors;
    scene->interface = s_interface;
    scene->width = width;
    scene->height = height;
    scene->native_x = frame->native_x;
    scene->background_id = frame->background_id;
    scene->brightness = (float)frame->brightness / 15.0f;
    scene->serial = frame->serial;

    const uint32_t *bg1 = frame->layer[LUFIA2_HD2D_BG1];
    const uint32_t *bg2 = frame->layer[LUFIA2_HD2D_BG2];
    const uint32_t *bg3 = frame->layer[LUFIA2_HD2D_BG3];
    const uint32_t *obj = frame->layer[LUFIA2_HD2D_OBJ];
    unsigned x0 = width, x1 = 0;
    size_t covered = 0;
    SetCast(scene, 0, 0, (int)width, (int)height);

    /* The world map's ground is the plane itself, drawn by the renderer;
     * of the picture only the sprites are taken, and they all stand on it.
     * Nothing else is laid over the scene. */
    if (frame->kind == LUFIA2_HD2D_MAP) {
        memset(s_background, 0, (size_t)width * height * 4);
        BuildField(frame, scene);
        return;
    }
    if (frame->kind == LUFIA2_HD2D_WORLD_MAP) {
        const size_t pixels = (size_t)width * height;
        memset(s_background, 0, pixels * 4);
        memset(s_interface, 0, pixels * 4);
        for (size_t i = 0; i < pixels; i++)
            PutRgba(s_actors + i * 4, Alpha(obj[i]) ? obj[i] : 0u);
        scene->background_x1 = width;
        scene->background_cover = 1.0f;
        BuildActors(frame, scene);
        return;
    }

    for (unsigned y = 0; y < height; y++) {
        const Lufia2Hd2dLine *line = &frame->lines[y];
        for (unsigned x = 0; x < width; x++) {
            const size_t i = (size_t)y * width + x;
            uint32_t world = frame->backdrop;

            const uint32_t back = Alpha(bg1[i]) ? bg1[i] : 0u;
            PutRgba(s_background + i * 4, back);
            if (back) {
                world = back;
                covered++;
                if (x < x0) x0 = x;
                if (x >= x1) x1 = x + 1;
            }

            /* BG3 lies over BG2 wherever both belong to the interface. */
            uint32_t actor = 0, plain = Alpha(bg3[i]) ? bg3[i] : 0u;
            if (Alpha(bg2[i])) {
                if (Bg2IsWorld(frame, (int)x - (int)frame->native_x, (int)y))
                    actor = bg2[i];
                else if (!plain)
                    plain = bg2[i];
            }
            if (Alpha(obj[i]))
                actor = obj[i];
            PutRgba(s_actors + i * 4, actor);
            if (actor)
                world = actor;

            uint8_t *rgb = s_world + i * 3;
            rgb[0] = (uint8_t)(world >> 16);
            rgb[1] = (uint8_t)(world >> 8);
            rgb[2] = (uint8_t)world;
            SolveInterface(rgb, frame->flat[i], plain, line,
                           s_interface + i * 4);
        }
    }
    if (x1 <= x0) {
        x0 = 0;
        x1 = width;
    }
    scene->background_x0 = x0;
    scene->background_x1 = x1;
    scene->background_cover =
        (float)covered / (float)((size_t)(x1 - x0) * height);

    CutOutStatusCards(scene);
    BuildActors(frame, scene);
}

/* ---- recording ------------------------------------------------------ */

/* A frame with a field is version 3 with a field block appended, and has a
 * version of its own; frames without one are still written as version 3. */
enum {
    RECORD_MAGIC = 0x4448324c /* "L2HD" */,
    RECORD_VERSION = 10,
    RECORD_VERSION_FIELD = 11,
};

typedef struct RecordField {
    uint32_t tiles_wide, tiles_high;
    int32_t origin_x, origin_y;
    int32_t map_x, map_y;
    uint16_t lower_chars, upper_chars;
    uint32_t map_id, rooms;
} RecordField;

typedef struct RecordHeader {
    uint32_t magic, version;
    uint32_t width, height, native_x, serial;
    uint32_t backdrop;
    uint32_t enemy_count, sprite_count;
    /* The third: 1 enemies on BG2, 2 the battle is opening. */
    uint8_t background_id, brightness, enemies_on_bg2, kind;
    /* Zero by zero when the frame has no plane, or no terrain. */
    uint32_t tiles_wide, tiles_high;
    uint32_t terrain_wide, terrain_high;
    float origin_x, origin_y;
} RecordHeader;

bool Lufia2Hd2dSaveFrame(const char *path, const Lufia2Hd2dFrame *frame) {
    if (!path || !frame || !frame->flat)
        return false;
    FILE *file = fopen(path, "wb");
    if (!file)
        return false;
    const bool has_field = frame->kind == LUFIA2_HD2D_MAP &&
                           frame->field.lower != NULL;
    const RecordHeader header = {
        RECORD_MAGIC, has_field ? RECORD_VERSION_FIELD : RECORD_VERSION,
        frame->width, frame->height,
        frame->native_x, frame->serial, frame->backdrop,
        frame->enemy_count, frame->sprite_count, frame->background_id,
        frame->brightness,
        (uint8_t)((frame->enemies_on_bg2 ? 1u : 0u) |
                  (frame->battle_opening ? 2u : 0u)),
        (uint8_t)frame->kind,
        frame->plane.tiles ? frame->plane.tiles_wide : 0u,
        frame->plane.tiles ? frame->plane.tiles_high : 0u,
        frame->plane.terrain ? frame->plane.terrain_wide : 0u,
        frame->plane.terrain ? frame->plane.terrain_high : 0u,
        frame->plane.origin_x, frame->plane.origin_y,
    };
    const size_t picture = (size_t)frame->width * frame->height * 4u;
    const size_t cells = (size_t)header.tiles_wide * header.tiles_high;
    const size_t blocks = (size_t)header.terrain_wide * header.terrain_high;
    bool ok = frame->vram &&
              fwrite(&header, sizeof header, 1, file) == 1 &&
              fwrite(frame->lines, sizeof frame->lines, 1, file) == 1 &&
              fwrite(frame->enemies, sizeof frame->enemies, 1, file) == 1 &&
              fwrite(frame->sprites, sizeof frame->sprites, 1, file) == 1 &&
              fwrite(frame->vram, sizeof(uint16_t), LUFIA2_HD2D_VRAM_WORDS,
                     file) == LUFIA2_HD2D_VRAM_WORDS &&
              fwrite(frame->sprite_chars, sizeof frame->sprite_chars, 1,
                     file) == 1 &&
              fwrite(frame->sprite_cgram, sizeof frame->sprite_cgram, 1,
                     file) == 1;
    for (unsigned i = 0; ok && i < LUFIA2_HD2D_LAYERS; i++)
        ok = fwrite(frame->layer[i], 1, picture, file) == picture;
    ok = ok && fwrite(frame->flat, 1, picture, file) == picture;
    if (ok && cells)
        ok = fwrite(frame->plane.tiles, 1, cells, file) == cells &&
             fwrite(frame->plane.chars, 1, LUFIA2_HD2D_PLANE_CHAR_BYTES,
                    file) == LUFIA2_HD2D_PLANE_CHAR_BYTES &&
             fwrite(frame->plane.cgram, sizeof frame->plane.cgram, 1,
                    file) == 1;
    if (ok && blocks)
        ok = fwrite(frame->plane.terrain, 1, blocks, file) == blocks;
    if (ok && has_field) {
        const Lufia2Hd2dField *field = &frame->field;
        const RecordField block = {
            field->tiles_wide, field->tiles_high, field->origin_x,
            field->origin_y, field->map_x, field->map_y,
            field->lower_chars, field->upper_chars, field->map_id,
            field->rooms ? 1u : 0u,
        };
        const size_t tiles = (size_t)field->tiles_wide * field->tiles_high;
        ok = fwrite(&block, sizeof block, 1, file) == 1 &&
             fwrite(field->lower, sizeof(uint16_t), tiles, file) == tiles &&
             fwrite(field->upper, sizeof(uint16_t), tiles, file) == tiles &&
             fwrite(field->facts, 1, tiles, file) == tiles &&
             fwrite(field->cgram, sizeof field->cgram, 1, file) == 1;
    }
    return fclose(file) == 0 && ok;
}

void Lufia2Hd2dFreeFrame(Lufia2Hd2dFrame *frame) {
    if (!frame)
        return;
    for (unsigned i = 0; i < LUFIA2_HD2D_LAYERS; i++)
        free((void *)frame->layer[i]);
    free((void *)frame->flat);
    free((void *)frame->plane.tiles);
    free((void *)frame->plane.chars);
    free((void *)frame->plane.terrain);
    free((void *)frame->field.lower);
    free((void *)frame->field.upper);
    free((void *)frame->field.facts);
    free((void *)frame->vram);
    memset(frame, 0, sizeof *frame);
}

bool Lufia2Hd2dLoadFrame(const char *path, Lufia2Hd2dFrame *frame) {
    if (!path || !frame)
        return false;
    memset(frame, 0, sizeof *frame);
    FILE *file = fopen(path, "rb");
    if (!file)
        return false;
    RecordHeader header;
    bool ok = fread(&header, sizeof header, 1, file) == 1 &&
              header.magic == RECORD_MAGIC &&
              (header.version == RECORD_VERSION ||
               header.version == RECORD_VERSION_FIELD) &&
              header.width >= LUFIA2_HD2D_NATIVE_WIDTH &&
              header.width <= LUFIA2_HD2D_MAX_WIDTH &&
              header.height == LUFIA2_HD2D_HEIGHT &&
              header.enemy_count <= LUFIA2_HD2D_ENEMIES &&
              header.sprite_count <= LUFIA2_HD2D_OAM_SLOTS &&
              header.kind < LUFIA2_HD2D_KINDS &&
              header.tiles_wide <= 1024u && header.tiles_high <= 1024u &&
              header.terrain_wide <= 512u && header.terrain_high <= 512u;
    ok = ok && fread(frame->lines, sizeof frame->lines, 1, file) == 1 &&
         fread(frame->enemies, sizeof frame->enemies, 1, file) == 1 &&
         fread(frame->sprites, sizeof frame->sprites, 1, file) == 1;
    if (ok) {
        uint16_t *vram = (uint16_t *)malloc(
            LUFIA2_HD2D_VRAM_WORDS * sizeof(uint16_t));
        frame->vram = vram;
        ok = vram &&
             fread(vram, sizeof(uint16_t), LUFIA2_HD2D_VRAM_WORDS, file) ==
                 LUFIA2_HD2D_VRAM_WORDS &&
             fread(frame->sprite_chars, sizeof frame->sprite_chars, 1,
                   file) == 1 &&
             fread(frame->sprite_cgram, sizeof frame->sprite_cgram, 1,
                   file) == 1;
    }
    if (ok) {
        frame->width = header.width;
        frame->height = header.height;
        frame->native_x = header.native_x;
        frame->serial = header.serial;
        frame->backdrop = header.backdrop;
        frame->enemy_count = header.enemy_count;
        frame->sprite_count = header.sprite_count;
        frame->background_id = header.background_id;
        frame->brightness = header.brightness;
        frame->enemies_on_bg2 = (header.enemies_on_bg2 & 1u) != 0;
        frame->battle_opening = (header.enemies_on_bg2 & 2u) != 0;
        frame->kind = (Lufia2Hd2dKind)header.kind;
    }
    const size_t picture = (size_t)header.width * header.height * 4u;
    for (unsigned i = 0; ok && i <= LUFIA2_HD2D_LAYERS; i++) {
        uint32_t *pixels = (uint32_t *)malloc(picture);
        ok = pixels && fread(pixels, 1, picture, file) == picture;
        if (i < LUFIA2_HD2D_LAYERS) frame->layer[i] = pixels;
        else frame->flat = pixels;
    }
    const size_t cells = (size_t)header.tiles_wide * header.tiles_high;
    if (ok && cells) {
        uint8_t *tiles = (uint8_t *)malloc(cells);
        uint8_t *chars = (uint8_t *)malloc(LUFIA2_HD2D_PLANE_CHAR_BYTES);
        frame->plane.tiles = tiles;
        frame->plane.chars = chars;
        frame->plane.tiles_wide = header.tiles_wide;
        frame->plane.tiles_high = header.tiles_high;
        frame->plane.origin_x = header.origin_x;
        frame->plane.origin_y = header.origin_y;
        ok = tiles && chars && fread(tiles, 1, cells, file) == cells &&
             fread(chars, 1, LUFIA2_HD2D_PLANE_CHAR_BYTES, file) ==
                 LUFIA2_HD2D_PLANE_CHAR_BYTES &&
             fread(frame->plane.cgram, sizeof frame->plane.cgram, 1,
                   file) == 1;
    }
    const size_t blocks = (size_t)header.terrain_wide * header.terrain_high;
    if (ok && cells && blocks) {
        uint8_t *terrain = (uint8_t *)malloc(blocks);
        frame->plane.terrain = terrain;
        frame->plane.terrain_wide = header.terrain_wide;
        frame->plane.terrain_high = header.terrain_high;
        ok = terrain && fread(terrain, 1, blocks, file) == blocks;
    }
    if (ok && header.version == RECORD_VERSION_FIELD) {
        Lufia2Hd2dField *field = &frame->field;
        RecordField block;
        ok = fread(&block, sizeof block, 1, file) == 1 &&
             block.tiles_wide && block.tiles_high &&
             block.tiles_wide <= LUFIA2_HD2D_FIELD_MAX_WIDE &&
             block.tiles_high <= LUFIA2_HD2D_FIELD_MAX_HIGH;
        if (ok) {
            const size_t tiles = (size_t)block.tiles_wide * block.tiles_high;
            uint16_t *lower = (uint16_t *)malloc(tiles * sizeof(uint16_t));
            uint16_t *upper = (uint16_t *)malloc(tiles * sizeof(uint16_t));
            uint8_t *facts = (uint8_t *)malloc(tiles);
            field->lower = lower;
            field->upper = upper;
            field->facts = facts;
            field->tiles_wide = block.tiles_wide;
            field->tiles_high = block.tiles_high;
            field->origin_x = block.origin_x;
            field->origin_y = block.origin_y;
            field->map_x = block.map_x;
            field->map_y = block.map_y;
            field->lower_chars = block.lower_chars;
            field->upper_chars = block.upper_chars;
            field->map_id = (uint8_t)block.map_id;
            field->rooms = block.rooms != 0;
            ok = lower && upper && facts &&
                 fread(lower, sizeof(uint16_t), tiles, file) == tiles &&
                 fread(upper, sizeof(uint16_t), tiles, file) == tiles &&
                 fread(facts, 1, tiles, file) == tiles &&
                 fread(field->cgram, sizeof field->cgram, 1, file) == 1;
        }
    }
    fclose(file);
    if (!ok)
        Lufia2Hd2dFreeFrame(frame);
    return ok;
}
