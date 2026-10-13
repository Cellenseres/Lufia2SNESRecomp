#include "lufia2_hd2d_gl.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gl_core_3_1.h"

/* The diorama.
 *
 * The Battle picture is flat, but it was painted as a room: a backdrop that
 * meets a floor, with everyone standing on that floor. The renderer takes it
 * at its word. A reference camera is placed so that the room, seen through
 * it, is exactly the SNES picture; the backdrop and the floor are textured by
 * projecting the picture back out through that camera, and each actor is a
 * billboard standing where its feet touch the floor. With the real camera on
 * the reference one the result is the original frame, pixel for pixel, and
 * every departure from it -- parallax, focus, light -- follows from
 * the same geometry instead of being painted on.
 *
 * Units are picture pixels. The reference camera sits at the origin looking
 * down +Z with its eye level on picture row `horizon`; the floor is the plane
 * y = -EYE and the backdrop the plane z = wall_z, which the floor reaches on
 * picture row `wall_base`.
 *
 * The world map needs no such reading: it already is a ground plane, the
 * Mode 7 plane the game looks straight down on. Here the camera is tilted
 * back from that view about the spot the party stands on, so the land runs
 * off into the distance, and whatever the game draws as a sprite stands up
 * on it. Straight overhead and at the same distance it would show the game's
 * own picture again. */

enum {
    QUAD_FLOATS = 11,
    QUAD_VERTICES = 6,
    MAX_QUADS = LUFIA2_HD2D_MAX_ACTORS,
    BLOOM_LEVELS = 5,
};

static const float kFocal = 256.0f;
static const float kEye = 100.0f;
/* The passes are fill-rate bound and the picture is 224 rows of pixel art:
 * past this many pixels the scene is rendered smaller and scaled up. */
static const float kPixelBudget = 3.0e6f;

/* The battle pictures look down on their floor: the eye level lies well
 * above the top of the frame, on this picture row. */
static const float kHorizon = -42.0f;

/* The air of a place. Colours are linear. */
typedef struct Hd2dAir {
    float fog[3];            /* what distance fades towards */
    float fog_amount;
    float ray[3];            /* light shafts */
    float ray_amount;
    float mote[3];           /* dust, pollen, embers */
    float mote_amount;
} Hd2dAir;

/* Underground: cold haze, pale shafts through the dark. */
static const Hd2dAir kAirCave = {
    {0.035f, 0.060f, 0.120f}, 0.42f, {0.62f, 0.78f, 1.00f}, 0.058f,
    {1.00f, 0.93f, 0.80f}, 1.0f,
};
/* Open sky: distance pales instead of darkening, sunlight is warm. */
static const Hd2dAir kAirOpen = {
    {0.40f, 0.56f, 0.80f}, 0.24f, {1.00f, 0.90f, 0.68f}, 0.055f,
    {1.00f, 0.98f, 0.86f}, 0.55f,
};
/* Built halls: neutral gloom, window light. */
static const Hd2dAir kAirHall = {
    {0.050f, 0.055f, 0.075f}, 0.34f, {1.00f, 0.92f, 0.78f}, 0.060f,
    {1.00f, 0.95f, 0.84f}, 0.75f,
};
/* Lava: red heat haze and drifting embers. */
static const Hd2dAir kAirLava = {
    {0.240f, 0.045f, 0.015f}, 0.36f, {1.00f, 0.50f, 0.22f}, 0.050f,
    {1.00f, 0.52f, 0.16f}, 1.5f,
};

/* The world map: distance pales into the sky's own colour. No shafts and
 * no dust; the weather is in the ground pass. */
static const Hd2dAir kAirWorld = {
    {0.52f, 0.70f, 0.90f}, 0.62f, {1.00f, 0.95f, 0.80f}, 0.0f,
    {1.00f, 1.00f, 1.00f}, 0.0f,
};

/* Daylight over a town: a little haze, pollen in the sun. */
static const Hd2dAir kAirTown = {
    {0.50f, 0.62f, 0.80f}, 0.16f, {1.00f, 0.93f, 0.76f}, 0.0f,
    {1.00f, 0.97f, 0.82f}, 0.30f,
};
/* Inside a house or a cave: no distance to haze over, dust in the light. */
static const Hd2dAir kAirRoom = {
    {0.0f, 0.0f, 0.0f}, 0.0f, {1.00f, 0.90f, 0.72f}, 0.0f,
    {1.00f, 0.90f, 0.70f}, 0.55f,
};

/* World-map camera: how far it is tilted back from straight down, where on
 * the picture the party ends up, and the tilt it starts from when the map
 * comes up, which is nearly the game's own view. */
static const float kWorldPitch = 0.733f;         /* 42 degrees */
static const float kWorldFocal = 300.0f;
static const float kWorldFocusRow = 132.0f;
static const float kWorldPartyRow = 118.0f;
/* Picture row the party stands on in a town or a dungeon. */
static const float kFieldPartyRow = 120.0f;

/* How one battle background is staged. */
typedef struct Hd2dLook {
    float wall_base;         /* picture row where backdrop meets floor */
    const Hd2dAir *air;
    float reflect;           /* floor gloss */
    float relief;            /* emboss from the picture's own luminance */
} Hd2dLook;

/* Indexed by battle background id. Water and lava lie flat, so they belong
 * to the floor; a ship's rail and a panelled wall stand, so they do not.
 * Only dressed stone is given a gloss: earth does not mirror. */
static const Hd2dLook kLooks[] = {
    /* 00 desert          */ {44.0f, &kAirOpen, 0.00f, 0.30f},
    /* 01 dark cavern     */ {90.0f, &kAirCave, 0.00f, 0.40f},
    /* 02 seaside plaza   */ {80.0f, &kAirOpen, 0.07f, 0.30f},
    /* 03 ship deck       */ {128.0f, &kAirOpen, 0.00f, 0.30f},
    /* 04 grassland       */ {50.0f, &kAirOpen, 0.00f, 0.30f},
    /* 05 shrine          */ {64.0f, &kAirHall, 0.20f, 0.35f},
    /* 06 mossy cave      */ {60.0f, &kAirCave, 0.00f, 0.40f},
    /* 07 mountain path   */ {76.0f, &kAirOpen, 0.00f, 0.35f},
    /* 08 throne room     */ {134.0f, &kAirHall, 0.24f, 0.30f},
    /* 09 dark brickwork  */ {58.0f, &kAirHall, 0.05f, 0.40f},
    /* 0A castle hall     */ {64.0f, &kAirHall, 0.16f, 0.35f},
    /* 0B red cave        */ {68.0f, &kAirCave, 0.00f, 0.40f},
    /* 0C brown cave      */ {68.0f, &kAirCave, 0.00f, 0.40f},
    /* 0D cave stream     */ {52.0f, &kAirCave, 0.00f, 0.40f},
    /* 0E purple cave     */ {68.0f, &kAirCave, 0.00f, 0.40f},
    /* 0F waterfall cave  */ {50.0f, &kAirCave, 0.00f, 0.40f},
    /* 10 olive cave      */ {68.0f, &kAirCave, 0.00f, 0.40f},
    /* 11 overgrown cave  */ {68.0f, &kAirCave, 0.00f, 0.40f},
    /* 12 lava cave       */ {60.0f, &kAirLava, 0.00f, 0.40f},
    /* 13 lava cave, rock */ {60.0f, &kAirLava, 0.00f, 0.40f},
    /* 14 autumn cave     */ {68.0f, &kAirCave, 0.00f, 0.40f},
    /* 15 dungeon         */ {64.0f, &kAirHall, 0.05f, 0.40f},
    /* 16 brick tower     */ {66.0f, &kAirHall, 0.05f, 0.35f},
    /* 17 castle hall     */ {64.0f, &kAirHall, 0.16f, 0.35f},
    /* 18 blank           */ {68.0f, &kAirCave, 0.00f, 0.00f},
};

typedef struct Target {
    GLuint framebuffer;
    GLuint texture;
    int width, height;
} Target;

typedef struct Program {
    GLuint id;
} Program;

static struct {
    bool ready, failed;
    char error[512];

    GLuint empty_vao;
    GLuint quad_vao, quad_vbo;
    GLuint texture[3];           /* background, actors, interface */
    unsigned texture_width[3], texture_height[3];

    Program background, reflection, actor, focus, down, up, final,
        blit;
    /* Field. */
    Program field, field_actor;
    GLuint field_lower, field_upper, field_relief;
    unsigned field_wide, field_high;
    /* World map. */
    Program plane_bake, ground, world_actor;
    GLuint plane_tiles, plane_chars, plane_palette, plane_terrain;
    Target plane;                /* the plane in colour, with mipmaps */
    unsigned plane_wide, plane_high;
    uint64_t plane_signature, blocks_signature;
    bool plane_relief;           /* the plane knows what stands on it */
    unsigned plane_stack;        /* rows its tallest figure is carried up */

    Target scene_back;           /* backdrop + floor, alpha = blur radius */
    Target scene;                /* focused backdrop with the actors on it */
    Target bloom[BLOOM_LEVELS];
    Target output;
    int width, height;
} s_gl;

static void SetError(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(s_gl.error, sizeof s_gl.error, format, args);
    va_end(args);
    fprintf(stderr, "[hd2d] %s\n", s_gl.error);
}

const char *Lufia2Hd2dGlError(void) { return s_gl.error; }

/* ---- shaders -------------------------------------------------------- */

#define GLSL_VERSION "#version 330 core\n"

/* Shared by every pass that looks into the room. */
#define GLSL_ROOM \
    "uniform vec4 u_pic;\n"      /* width, height, 1/width, 1/height */ \
    "uniform vec4 u_proj;\n"     /* focal, centre x, horizon row, eye */ \
    "uniform vec4 u_cam;\n"      /* camera x, y, z; wall z */ \
    "uniform vec4 u_pp;\n"       /* lens shift x, y; wall base row; time */ \
    "uniform float u_zoom;\n"    /* camera focal length / reference one */ \
    "uniform vec4 u_cast;\n"     /* actor picture: column, row of its first
                                    pixel; 1/width, 1/height */ \
    "vec3 to_linear(vec3 c) { return c * c; }\n" \
    "vec3 to_gamma(vec3 c) { return sqrt(max(c, vec3(0.0))); }\n" \
    "float luma(vec3 c) { return dot(c, vec3(0.299, 0.587, 0.114)); }\n" \
    /* Nearest within a texel, a one-pixel linear seam between texels. */ \
    "vec4 sharp(sampler2D t, vec2 p, vec2 footprint) {\n" \
    "  vec2 base = floor(p - 0.5) + 0.5;\n" \
    "  vec2 f = p - base;\n" \
    "  f = clamp((f - 0.5) / max(footprint, vec2(1e-4)) + 0.5, 0.0, 1.0);\n" \
    "  return texture(t, (base + f) * u_pic.zw);\n" \
    "}\n" \
    /* The same, of the actor picture; and a plain tap of it. */ \
    "vec4 sharp_cast(sampler2D t, vec2 p, vec2 footprint) {\n" \
    "  p -= u_cast.xy;\n" \
    "  vec2 base = floor(p - 0.5) + 0.5;\n" \
    "  vec2 f = p - base;\n" \
    "  f = clamp((f - 0.5) / max(footprint, vec2(1e-4)) + 0.5, 0.0, 1.0);\n" \
    "  return texture(t, (base + f) * u_cast.zw);\n" \
    "}\n" \
    "vec4 cast_at(sampler2D t, vec2 p) {\n" \
    "  return texture(t, (p - u_cast.xy) * u_cast.zw);\n" \
    "}\n"

/* The actor shader serves the room, the world map and the field alike, so
 * it is put together when linked: version, this, then for a map its own
 * chunk, then the shader. */
static const char kVersion[] = GLSL_VERSION;
static const char kRoomChunk[] = GLSL_ROOM;

static const char kFullscreenVertex[] =
    GLSL_VERSION
    "void main(void) {\n"
    "  vec2 p = vec2(float((gl_VertexID << 1) & 2), float(gl_VertexID & 2));\n"
    "  gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);\n"
    "}\n";

static const char kQuadVertex[] =
    GLSL_VERSION
    "layout(location = 0) in vec3 a_position;\n"
    "layout(location = 1) in vec4 a_a;\n"
    "layout(location = 2) in vec4 a_b;\n"
    "uniform vec4 u_proj;\n"
    "uniform vec4 u_cam;\n"
    "uniform vec4 u_pp;\n"
    "uniform float u_zoom;\n"
    "uniform vec4 u_ndc;\n"      /* picture -> clip: ax, bx, ay, by */
    "out vec4 v_a;\n"
    "out vec4 v_b;\n"
    "out vec3 v_world;\n"
    "void main(void) {\n"
    "  vec3 v = a_position - u_cam.xyz;\n"
    "  float w = max(v.z, 1.0);\n"
    "  float sx = u_proj.y + u_pp.x + u_proj.x * u_zoom * v.x / w;\n"
    "  float sy = u_proj.z + u_pp.y - u_proj.x * u_zoom * v.y / w;\n"
    "  vec2 ndc = vec2(u_ndc.x * sx + u_ndc.y, u_ndc.z * sy + u_ndc.w);\n"
    "  gl_Position = vec4(ndc * w, 0.0, w);\n"
    "  v_a = a_a;\n"
    "  v_b = a_b;\n"
    "  v_world = a_position;\n"
    "}\n";

/* Backdrop and floor, by casting each pixel's ray into the room. */
static const char kBackgroundFragment[] =
    GLSL_VERSION GLSL_ROOM
    "uniform sampler2D u_background;\n"
    "uniform vec4 u_view;\n"     /* content x, y, 1/width, 1/height */
    "uniform vec4 u_columns;\n"  /* opaque columns x0, x1; brightness^2;
                                     room continuation 0..1 */
    "uniform vec4 u_focus;\n"    /* row, half range, far px/row, near px/row */
    "uniform vec4 u_light;\n"    /* pool x, z, radius; maximum blur */
    "uniform vec4 u_look;\n"     /* relief, fog amount, ambient, key */
    "uniform vec3 u_fog;\n"
    "out vec4 o_colour;\n"
    /* The picture, continued past each edge by its mirror image, so a
     * camera that leaves the reference still sees a room. */
    "vec2 room_texel(vec2 p) {\n"
    "  float w = u_columns.y - u_columns.x;\n"
    "  float t = (p.x - u_columns.x) / w;\n"
    "  t = 1.0 - abs(mod(t, 2.0) - 1.0);\n"
    "  float v = 1.0 - abs(mod(p.y / u_pic.y, 2.0) - 1.0);\n"
    "  return vec2(u_columns.x + clamp(t * w, 0.5, w - 0.5),\n"
    "              clamp(v * u_pic.y, 0.5, u_pic.y - 0.5));\n"
    "}\n"
    "void main(void) {\n"
    "  vec2 unit = (gl_FragCoord.xy - u_view.xy) * u_view.zw;\n"
    "  vec2 s = vec2(unit.x * u_pic.x, (1.0 - unit.y) * u_pic.y);\n"
    "  float focal = u_proj.x, eye = u_proj.w;\n"
    "  vec3 cam = u_cam.xyz;\n"
    "  vec3 ray = vec3((s.x - u_proj.y - u_pp.x) / (focal * u_zoom),\n"
    "                  -(s.y - u_proj.z - u_pp.y) / (focal * u_zoom), 1.0);\n"
    "  float t_wall = u_cam.w - cam.z;\n"
    "  vec3 hit = cam + t_wall * ray;\n"
    "  bool on_floor = false;\n"
    "  if (ray.y < -1e-5) {\n"
    "    float t_floor = (-eye - cam.y) / ray.y;\n"
    "    if (t_floor > 0.0 && t_floor < t_wall) {\n"
    "      hit = cam + t_floor * ray;\n"
    "      on_floor = true;\n"
    "    }\n"
    "  }\n"
    /* Where the reference camera saw this point of the room. */
    "  vec2 r = vec2(u_proj.y + focal * hit.x / hit.z,\n"
    "                u_proj.z - focal * hit.y / hit.z);\n"
    "  vec2 footprint = fwidth(r);\n"
    "  vec3 colour = to_linear(sharp(u_background, room_texel(r),\n"
    "                                footprint).rgb);\n"
    /* A picture with holes in it is mid-transition: do not repeat them. */
    "  if (r.x < u_columns.x || r.x > u_columns.y) colour *= u_columns.w;\n"
    /* Relief: the painting's own shading read as height, lit from upper
     * left. Smooth taps, so it models forms rather than pixel edges. */
    "  vec2 slope = vec2(-0.9, -1.1);\n"
    "  float lit_side = luma(texture(u_background,\n"
    "      room_texel(r + slope) * u_pic.zw).rgb);\n"
    "  float dark_side = luma(texture(u_background,\n"
    "      room_texel(r - slope) * u_pic.zw).rgb);\n"
    "  colour *= 1.0 + u_look.x * clamp((lit_side - dark_side) * 2.2,\n"
    "                                   -0.6, 0.9);\n"
    /* A pool of light where the fight is; the room falls away around it. */
    "  vec3 from_pool = (hit - vec3(u_light.x, -eye, u_light.y)) *\n"
    "                   vec3(1.0, 0.55, 1.0) / u_light.z;\n"
    "  float pool = 1.0 / (1.0 + 1.5 * dot(from_pool, from_pool));\n"
    "  colour *= u_look.z + u_look.w * pool;\n"
    /* Where backdrop and floor meet, each shades the other. */
    "  float corner = on_floor ? (u_cam.w - hit.z) : (hit.y + eye);\n"
    "  colour *= 1.0 - 0.30 * exp(-max(corner, 0.0) / (0.045 * u_cam.w));\n"
    "  float far = clamp(hit.z / u_cam.w, 0.0, 1.0);\n"
    "  colour = mix(colour,\n"
    "               u_fog * (0.6 * u_columns.z + 1.6 * luma(colour)),\n"
    "               u_look.y * far * far);\n"
    /* Blur radius. On the floor a thin lens makes it linear in the picture
     * row a depth appears on, and the backdrop is one depth throughout. */
    "  float row = u_proj.z + focal * eye / hit.z;\n"
    "  float away = row - u_focus.x;\n"
    "  float blur = max(abs(away) - u_focus.y, 0.0) *\n"
    "               (away < 0.0 ? u_focus.z : u_focus.w);\n"
    "  o_colour = vec4(colour, min(blur, u_light.w));\n"
    "}\n";

/* The floor's faint mirror image of an actor. */
static const char kReflectionFragment[] =
    GLSL_VERSION GLSL_ROOM
    "uniform sampler2D u_actors;\n"
    "in vec4 v_a;\n"             /* picture x, mirrored row, depth, gloss */
    "in vec4 v_b;\n"
    "out vec4 o_colour;\n"
    "void main(void) {\n"
    "  float depth = v_a.z;\n"
    "  vec2 p = v_a.xy;\n"
    "  p.x += sin(p.y * 1.9 + u_pp.w * 1.4) * 0.55 * depth;\n"
    "  vec4 sum = vec4(0.0);\n"
    "  float spread = 0.6 + 1.8 * depth;\n"
    "  for (int i = -2; i <= 2; i++) {\n"
    "    vec2 q = p + vec2(float(i) * spread * 0.5, 0.0);\n"
    "    if (q.x >= v_b.x && q.x <= v_b.z && q.y >= v_b.y && q.y <= v_b.w)\n"
    "      sum += cast_at(u_actors, q);\n"
    "  }\n"
    "  sum /= 5.0;\n"
    "  float fade = v_a.w * pow(1.0 - depth, 1.6);\n"
    "  o_colour = vec4(to_linear(sum.rgb) * fade, sum.a * fade * 0.55);\n"
    "}\n";

/* An actor billboard. */
static const char kActorFragment[] =
    "uniform sampler2D u_actors;\n"
    "in vec4 v_a;\n"             /* picture x, row, unlit flag, pool light */
    "in vec4 v_b;\n"             /* actor x0, y0, x1, foot */
    "in vec3 v_world;\n"
    "out vec4 o_colour;\n"
    "void main(void) {\n"
    "  vec2 p = v_a.xy;\n"
    "  vec2 footprint = fwidth(p);\n"
    "  if (p.x < v_b.x || p.x > v_b.z || p.y < v_b.y || p.y > v_b.w)\n"
    "    discard;\n"
    "#ifdef FIELD\n"
    /* The game's own rule: a sprite is hidden where the map draws a tile
     * over it, and nowhere else. */
    "  if (in_window(p - u_window.xy) &&\n"
    "      texelFetch(u_relief, ivec2(p - u_window.xy), 0).b > 0.5)\n"
    "    discard;\n"
    "#elif defined(WORLD)\n"
    /* A mountain between the eye and where this stands is in front of it. */
    "  Ray ray = view_ray(picture_point());\n"
    "  float mine = (u_eye.y + u_eye.z * u_tilt.y - v_world.z) /\n"
    "               max(-ray.along.y, 1e-4);\n"
    "  if (ray.reach - terrain(ray).back < mine - 0.05) discard;\n"
    "#endif\n"
    "  vec4 texel = sharp_cast(u_actors, p, footprint);\n"
    "  if (texel.a < 0.004) discard;\n"
    "  vec3 colour = to_linear(texel.rgb / texel.a);\n"
    "  if (v_a.z < 0.5) {\n"
    "    colour *= mix(1.0, v_a.w, 0.40);\n"
    /* Light catches the edge that faces it. */
    "    float behind = cast_at(u_actors, p + vec2(-1.0, -1.0)).a;\n"
    "    colour += vec3(1.0, 0.86, 0.62) * 0.20 * (1.0 - behind) *\n"
    "              (0.35 + luma(colour));\n"
    "  }\n"
    "  o_colour = vec4(colour * texel.a, texel.a);\n"
    "}\n";

/* Depth of field for the room: a disc as wide as the stored blur radius. */
static const char kFocusFragment[] =
    GLSL_VERSION
    "uniform sampler2D u_source;\n"
    "uniform vec2 u_texel;\n"
    "out vec4 o_colour;\n"
    "void main(void) {\n"
    "  vec2 uv = gl_FragCoord.xy * u_texel;\n"
    "  vec4 centre = textureLod(u_source, uv, 0.0);\n"
    "  float radius = centre.a;\n"
    "  if (radius < 0.5) {\n"
    "    o_colour = vec4(centre.rgb, 1.0);\n"
    "    return;\n"
    "  }\n"
    "  float lod = max(log2(radius) - 1.5, 0.0);\n"
    "  vec3 sum = centre.rgb;\n"
    "  float weight = 1.0;\n"
    "  for (int i = 0; i < 18; i++) {\n"
    "    float fi = float(i);\n"
    "    float reach = sqrt((fi + 0.5) / 18.0) * radius;\n"
    "    float turn = fi * 2.39996323;\n"
    "    vec4 tap = textureLod(u_source,\n"
    "        uv + vec2(cos(turn), sin(turn)) * reach * u_texel, lod);\n"
    /* A sharp neighbour does not smear into a soft pixel. */
    "    float w = clamp(tap.a - reach + 1.5, 0.15, 1.0);\n"
    "    sum += tap.rgb * w;\n"
    "    weight += w;\n"
    "  }\n"
    "  o_colour = vec4(sum / weight, 1.0);\n"
    "}\n";

static const char kDownFragment[] =
    GLSL_VERSION
    "uniform sampler2D u_source;\n"
    "uniform vec2 u_texel;\n"    /* of the source */
    "uniform float u_threshold;\n"
    "out vec4 o_colour;\n"
    "vec3 bright(vec3 c) {\n"
    "  if (u_threshold <= 0.0) return c;\n"
    "  float l = max(c.r, max(c.g, c.b));\n"
    "  float knee = u_threshold * 0.6;\n"
    "  float soft = clamp(l - u_threshold + knee, 0.0, 2.0 * knee);\n"
    "  soft = soft * soft / (4.0 * knee + 1e-4);\n"
    "  return c * max(soft, l - u_threshold) / max(l, 1e-4);\n"
    "}\n"
    "void main(void) {\n"
    "  vec2 uv = gl_FragCoord.xy * 2.0 * u_texel;\n"
    "  vec2 h = u_texel;\n"
    "  vec3 sum = bright(texture(u_source, uv).rgb) * 4.0;\n"
    "  sum += bright(texture(u_source, uv + vec2(-h.x, -h.y)).rgb);\n"
    "  sum += bright(texture(u_source, uv + vec2(h.x, -h.y)).rgb);\n"
    "  sum += bright(texture(u_source, uv + vec2(-h.x, h.y)).rgb);\n"
    "  sum += bright(texture(u_source, uv + vec2(h.x, h.y)).rgb);\n"
    "  o_colour = vec4(sum / 8.0, 1.0);\n"
    "}\n";

static const char kUpFragment[] =
    GLSL_VERSION
    "uniform sampler2D u_source;\n"
    "uniform vec2 u_texel;\n"    /* of the source */
    "out vec4 o_colour;\n"
    "void main(void) {\n"
    "  vec2 uv = gl_FragCoord.xy * 0.5 * u_texel;\n"
    "  vec2 h = u_texel * 0.5;\n"
    "  vec3 sum = texture(u_source, uv + vec2(-h.x * 2.0, 0.0)).rgb;\n"
    "  sum += texture(u_source, uv + vec2(-h.x, h.y)).rgb * 2.0;\n"
    "  sum += texture(u_source, uv + vec2(0.0, h.y * 2.0)).rgb;\n"
    "  sum += texture(u_source, uv + vec2(h.x, h.y)).rgb * 2.0;\n"
    "  sum += texture(u_source, uv + vec2(h.x * 2.0, 0.0)).rgb;\n"
    "  sum += texture(u_source, uv + vec2(h.x, -h.y)).rgb * 2.0;\n"
    "  sum += texture(u_source, uv + vec2(0.0, -h.y * 2.0)).rgb;\n"
    "  sum += texture(u_source, uv + vec2(-h.x, -h.y)).rgb * 2.0;\n"
    "  o_colour = vec4(sum / 12.0, 1.0);\n"
    "}\n";

/* Glow, air, grade, and the interface laid over the finished picture. */
static const char kFinalFragment[] =
    GLSL_VERSION GLSL_ROOM
    "uniform sampler2D u_scene;\n"
    "uniform sampler2D u_bloom;\n"
    "uniform sampler2D u_interface;\n"
    "uniform vec4 u_view;\n"
    "uniform vec2 u_texel;\n"
    "uniform vec4 u_air;\n"      /* ray amount, dust amount, brightness;
                                    1 when dust shows only where it is lit */
    "uniform vec3 u_ray;\n"
    "uniform vec3 u_mote;\n"
    "out vec4 o_colour;\n"
    "float hash(vec2 p) {\n"
    "  vec3 q = fract(vec3(p.xyx) * 0.1031);\n"
    "  q += dot(q, q.yzx + 33.33);\n"
    "  return fract((q.x + q.y) * q.z);\n"
    "}\n"
    "vec2 hash2(vec2 p) {\n"
    "  return vec2(hash(p), hash(p + 71.7));\n"
    "}\n"
    /* Shafts from a source above and to the left of the frame. */
    "float shafts(vec2 s, float time) {\n"
    "  vec2 from = s - vec2(u_proj.y - 0.80 * u_pic.x, -0.85 * u_pic.y);\n"
    "  float angle = atan(from.y, from.x);\n"
    "  float bands = 0.5 + 0.24 * sin(angle * 37.0 + time * 0.11) +\n"
    "                0.17 * sin(angle * 97.0 - time * 0.17 + 1.7) +\n"
    "                0.09 * sin(angle * 211.0 + time * 0.23 + 4.1);\n"
    "  bands = smoothstep(0.56, 1.0, bands);\n"
    "  float reach = smoothstep(2.7, 0.9, length(from) / u_pic.x);\n"
    "  float lift = smoothstep(u_pic.y * 0.82, u_pic.y * 0.12, s.y);\n"
    "  return bands * reach * lift;\n"
    "}\n"
    /* Motes at three distances: the nearest are large and out of focus. */
    "float motes(vec2 s, float time) {\n"
    "  float sum = 0.0;\n"
    "  for (int layer = 0; layer < 3; layer++) {\n"
    "    float fl = float(layer);\n"
    "    float cell = 30.0 + 26.0 * fl;\n"
    "    float size = 0.50 + 0.75 * fl;\n"
    "    vec2 drift = vec2(2.2 + 1.9 * fl, -1.1 - 0.9 * fl);\n"
    "    vec2 q = s + time * drift + fl * 19.3;\n"
    "    vec2 home = floor(q / cell - 0.5);\n"
    "    for (int j = 0; j <= 1; j++) {\n"
    "      for (int i = 0; i <= 1; i++) {\n"
    "        vec2 id = home + vec2(float(i), float(j));\n"
    "        vec2 h = hash2(id + fl * 37.1);\n"
    "        if (h.x + h.y < 0.95) continue;\n"
    "        vec2 at = (id + 0.15 + 0.7 * h) * cell +\n"
    "                  2.8 * vec2(sin(time * 0.61 + h.x * 6.283),\n"
    "                             cos(time * 0.47 + h.y * 6.283));\n"
    "        vec2 d = (q - at) / size;\n"
    "        float glint = 0.55 + 0.45 * sin(time * (0.9 + 1.7 * h.x) +\n"
    "                                        h.y * 6.283);\n"
    "        sum += glint * exp(-dot(d, d)) / (1.0 + 0.9 * fl);\n"
    "      }\n"
    "    }\n"
    "  }\n"
    "  return sum;\n"
    "}\n"
    "void main(void) {\n"
    "  vec2 uv = gl_FragCoord.xy * u_texel;\n"
    "  vec2 unit = (gl_FragCoord.xy - u_view.xy) * u_view.zw;\n"
    "  vec2 s = vec2(unit.x * u_pic.x, (1.0 - unit.y) * u_pic.y);\n"
    "  vec2 footprint = fwidth(s);\n"
    "  vec3 colour = textureLod(u_scene, uv, 0.0).rgb;\n"
    "  colour += texture(u_bloom, uv).rgb * 0.36;\n"
    "  float lit = u_air.z * u_air.z;\n"
    "  colour += u_ray * (u_air.x * lit * shafts(s, u_pp.w));\n"
    /* In a room the void beyond the walls has no light for dust to drift
     * through. */
    "  float shown = mix(1.0, smoothstep(0.004, 0.06,\n"
    "      luma(textureLod(u_scene, uv, 4.0).rgb)), u_air.w);\n"
    "  colour += u_mote * (u_air.y * lit * shown * 0.34 * motes(s, u_pp.w));\n"
    /* A shoulder for what the glow pushes past white. */
    "  colour = colour / (1.0 + max(colour - 0.75, 0.0) * 0.55);\n"
    "  vec2 off = uv - 0.5;\n"
    "  off.x *= 0.92;\n"
    "  colour *= 1.0 - 0.62 * smoothstep(0.30, 0.86, length(off));\n"
    "  vec3 graded = to_gamma(colour);\n"
    "  float tone = luma(graded);\n"
    "  graded = mix(vec3(tone), graded, 1.12);\n"
    "  graded += vec3(-0.012, -0.004, 0.022) * (1.0 - tone) +\n"
    "            vec3(0.020, 0.008, -0.016) * tone;\n"
    "  graded = clamp(graded, 0.0, 1.0);\n"
    /* The interface is flat and exact: it is the SNES picture's own. */
    "  if (clamp(unit, 0.0, 1.0) == unit) {\n"
    "    vec4 face = sharp(u_interface, s, footprint);\n"
    /* A translucent panel reads as glass: what lies under it is out of
     * focus. */
    "    float glass = smoothstep(0.25, 0.45, face.a) *\n"
    "                  (1.0 - smoothstep(0.60, 0.90, face.a));\n"
    "    if (glass > 0.0) {\n"
    "      vec3 soft = vec3(0.0);\n"
    "      for (int i = 0; i < 6; i++) {\n"
    "        float turn = float(i) * 1.0472;\n"
    "        soft += textureLod(u_scene, uv + vec2(cos(turn), sin(turn)) *\n"
    "                           u_texel * 10.0, 3.0).rgb;\n"
    "      }\n"
    "      soft = to_gamma(soft / 6.0);\n"
    "      graded = mix(graded, soft, 0.85 * glass);\n"
    "    }\n"
    "    graded = face.rgb + (1.0 - face.a) * graded;\n"
    "  }\n"
    "  float grain = hash(gl_FragCoord.xy + fract(u_pp.w) * 61.0);\n"
    "  graded += (grain - 0.5) / 160.0;\n"
    "  o_colour = vec4(graded, 1.0);\n"
    "}\n";

/* World map: the plane's characters and palette resolved to colour once,
 * so the ground pass can filter it like any picture. Alpha is zero on a
 * texel that lies flat; on one that stands it is one more than four times
 * how far, in texels, its card's foot lies south of its own block. */
static const char kPlaneBakeFragment[] =
    GLSL_VERSION
    "uniform sampler2D u_tiles;\n"
    "uniform sampler2D u_chars;\n"
    "uniform sampler2D u_palette;\n"   /* alpha: not a colour of the ground */
    "uniform sampler2D u_terrain;\n"   /* per block: r stands, b rows below */
    "out vec4 o_colour;\n"
    "int byte_at(sampler2D t, ivec2 p) {\n"
    "  return int(texelFetch(t, p, 0).r * 255.0 + 0.5);\n"
    "}\n"
    "void main(void) {\n"
    "  ivec2 pixel = ivec2(gl_FragCoord.xy);\n"
    "  ivec2 inside = pixel & 7;\n"
    "  int at = byte_at(u_tiles, pixel >> 3) * 64 + inside.y * 8 +\n"
    "           inside.x;\n"
    "  int index = byte_at(u_chars, ivec2(at & 127, at >> 7));\n"
    "  vec4 entry = texelFetch(u_palette, ivec2(index, 0), 0);\n"
    "  ivec2 blocks = textureSize(u_terrain, 0);\n"
    "  ivec2 block = (pixel >> 4) & (blocks - 1);\n"
    "  vec3 here = texelFetch(u_terrain, block, 0).rgb;\n"
    "  float rows = here.b * 255.0 / 64.0;\n"
    /* A great peak is drawn over the lesser ones beside it, which share its
     * blocks: they are carried part of the way up, not cut off. */
    "  if (here.r > 0.75) {\n"
    "    float t = (float(pixel.x & 15) + 0.5) / 16.0 - 0.5;\n"
    "    ivec2 beside = ivec2((block.x + (t < 0.0 ? -1 : 1)) &\n"
    "                         (blocks.x - 1), block.y);\n"
    "    vec3 there = texelFetch(u_terrain, beside, 0).rgb;\n"
    "    rows = mix(rows, there.r > 0.75 ? there.b * 255.0 / 64.0 : 0.0,\n"
    "               abs(t));\n"
    "  }\n"
    "  float stands = entry.a * step(0.25, here.r);\n"
    "  o_colour = vec4(entry.rgb,\n"
    "                  stands * (1.0 + floor(rows * 64.0 + 0.5)) / 255.0);\n"
    "}\n";

/* Shared by the world map's passes: the tilted camera, and the ray it sends
 * through a point of the picture. */
#define GLSL_WORLD \
    "uniform vec4 u_eye;\n"      /* focus x, y on plane; distance; focal */ \
    "uniform vec4 u_tilt;\n"     /* pitch sin, cos; centre column; focus row */ \
    "uniform float u_stretch;\n" /* of the picture, down the screen */ \
    "struct Ray {\n" \
    "  vec2 ground;\n"           /* where it meets the plane */ \
    "  vec2 along;\n"            /* travel over the plane per unit of reach */ \
    "  float fall;\n"            /* descent per unit of reach */ \
    "  float reach;\n"           /* how far it goes to meet the plane */ \
    "};\n" \
    "Ray view_ray(vec2 s) {\n" \
    "  float across = (s.x - u_tilt.z) / u_eye.w;\n" \
    "  float above = (u_tilt.w - s.y) / (u_eye.w * u_stretch);\n" \
    "  Ray ray;\n" \
    /* How steeply the ray descends; none of the sky's rays do. */ \
    "  ray.fall = u_tilt.x - above * u_tilt.y;\n" \
    "  ray.reach = u_eye.z * u_tilt.x / max(ray.fall, 1e-4);\n" \
    "  ray.along = vec2(across, -(above * u_tilt.x + u_tilt.y));\n" \
    "  ray.ground = vec2(u_eye.x, u_eye.y + u_eye.z * u_tilt.y) +\n" \
    "               ray.along * ray.reach;\n" \
    "  return ray;\n" \
    "}\n"

/* What stands on the world map. The game draws this map as it draws
 * everything on it, seen from the south, and a block holds one row of
 * figures: peaks, trees, a village. So each row of blocks stands as a card
 * that leans back from its southern edge with its own picture on its face.
 * Lying down it was squashed by the camera's tilt; leaning, it shows as it
 * was drawn, and whatever is behind is hidden below its crest. Only the
 * figure leans. The ground drawn around it stays where it is, which cuts the
 * card to the figure's outline.
 *
 * A ray is walked texel by texel from where it comes under the tallest
 * crest to where it meets the plane. */
#define GLSL_TERRAIN \
    "uniform sampler2D u_plane;\n"    /* alpha: see the bake */ \
    "uniform sampler2D u_terrain;\n"  /* per block: see UploadBlocks */ \
    "uniform vec4 u_extent;\n"   /* plane width, height, 1/width, 1/height */ \
    "uniform vec4 u_rise;\n"     /* height per texel of depth; tallest crest;
                                    1 when the terrain is known */ \
    "uniform vec4 u_view;\n"     /* content x, y, 1/width, 1/height */ \
    "struct Hit {\n" \
    "  vec2 at;\n" \
    "  float height;\n" \
    "  float back;\n"            /* reach it falls short of the plane by */ \
    "  ivec2 cell;\n" \
    "  bool wall;\n"             /* met a texel's side, not its face */ \
    "};\n" \
    "vec2 picture_point() {\n" \
    "  vec2 unit = (gl_FragCoord.xy - u_view.xy) * u_view.zw;\n" \
    "  return vec2(unit.x * u_pic.x, (1.0 - unit.y) * u_pic.y);\n" \
    "}\n" \
    "float card(ivec2 cell) {\n" \
    "  return texelFetch(u_plane, cell & (ivec2(u_extent.xy) - 1), 0).a;\n" \
    "}\n" \
    /* Where the card of a standing texel has its foot. */ \
    "float foot(ivec2 cell, float a) {\n" \
    "  return (floor(float(cell.y) / 16.0) + 1.0) * 16.0 +\n" \
    "         (a * 255.0 - 1.0) * 0.25;\n" \
    "}\n" \
    "float height_at(vec2 p) {\n" \
    "  ivec2 cell = ivec2(floor(p));\n" \
    "  float a = card(cell);\n" \
    "  return a > 0.0 ? u_rise.x * (foot(cell, a) - p.y) : 0.0;\n" \
    "}\n" \
    "vec4 block_at(ivec2 block) {\n" \
    "  return texelFetch(u_terrain,\n" \
    "                    block & (textureSize(u_terrain, 0) - 1), 0);\n" \
    "}\n" \
    "float figures_near(vec2 p) {\n" \
    "  return block_at(ivec2(floor(p / 16.0))).g;\n" \
    "}\n" \
    "Hit terrain(Ray ray) {\n" \
    "  Hit hit;\n" \
    "  hit.at = ray.ground;\n" \
    "  hit.height = 0.0;\n" \
    "  hit.back = 0.0;\n" \
    "  hit.cell = ivec2(floor(ray.ground));\n" \
    "  hit.wall = false;\n" \
    "  if (u_rise.z < 0.5 || ray.fall <= 1e-4) return hit;\n" \
    /* An opening in the ground is seen past whatever stands before it. */ \
    "  float lies = block_at(ivec2(floor(ray.ground / 16.0))).r;\n" \
    "  if (lies > 0.06 && lies < 0.25) return hit;\n" \
    "  float span = u_rise.y / ray.fall;\n" \
    "  vec2 from = ray.ground - ray.along * span;\n" \
    "  if (figures_near(ray.ground) + figures_near(from) +\n" \
    "      figures_near(0.5 * (ray.ground + from)) == 0.0)\n" \
    "    return hit;\n" \
    "  vec2 along = ray.along;\n" \
    "  if (abs(along.x) < 1e-6) along.x = 1e-6;\n" \
    "  ivec2 cell = ivec2(floor(from));\n" \
    "  ivec2 onward = ivec2(sign(along));\n" \
    "  vec2 pace = 1.0 / abs(along);\n" \
    "  vec2 next = (vec2(cell) + max(sign(along), 0.0) - from) / along;\n" \
    "  float s = 0.0, met = span;\n" \
    "  for (int i = 0; i < 160; i++) {\n" \
    /* A block the ray cannot meet anything in is crossed in one step: one
     * with no figure, or one it leaves still above the crest. */ \
    "    ivec2 block = cell >> 4;\n" \
    "    vec4 info = block_at(block);\n" \
    "    vec2 edge = (vec2(block + max(onward, ivec2(0))) * 16.0 - from) /\n" \
    "                along;\n" \
    "    float across = min(edge.x, edge.y);\n" \
    "    if (info.a < 0.75 ||\n" \
    "        u_rise.y - ray.fall * across >= u_rise.x * 48.0 * info.g) {\n" \
    "      s = across;\n" \
    "      if (s >= span) {\n" \
    "        cell = ivec2(floor(ray.ground));\n" \
    "        break;\n" \
    "      }\n" \
    /* Into the next block by count, not by rounding. */ \
    "      ivec2 low = block * 16;\n" \
    "      cell = clamp(ivec2(floor(from + along * s)), low, low + 15);\n" \
    "      if (edge.x < edge.y)\n" \
    "        cell.x = onward.x > 0 ? low.x + 16 : low.x - 1;\n" \
    "      else\n" \
    "        cell.y = onward.y > 0 ? low.y + 16 : low.y - 1;\n" \
    "      next = (vec2(cell) + max(sign(along), 0.0) - from) / along;\n" \
    "      continue;\n" \
    "    }\n" \
    "    float leave = min(next.x, next.y);\n" \
    "    float a = card(cell);\n" \
    "    if (a > 0.0 || leave >= span) {\n" \
    /* The card's face over this texel, as a line along the ray. */ \
    "      float stands = a > 0.0 ? u_rise.x : 0.0;\n" \
    "      float here = stands * (foot(cell, a) - from.y);\n" \
    "      float climb = -stands * along.y;\n" \
    "      float meet = (u_rise.y - here) / (ray.fall + climb);\n" \
    "      if (meet <= s) {\n" \
    "        met = s;\n" \
    "        hit.wall = true;\n" \
    "        break;\n" \
    "      }\n" \
    "      if (meet <= leave) {\n" \
    "        met = meet;\n" \
    "        break;\n" \
    "      }\n" \
    "    }\n" \
    "    if (next.x < next.y) {\n" \
    "      cell.x += onward.x;\n" \
    "      next.x += pace.x;\n" \
    "    } else {\n" \
    "      cell.y += onward.y;\n" \
    "      next.y += pace.y;\n" \
    "    }\n" \
    "    s = leave;\n" \
    "  }\n" \
    "  met = min(met, span);\n" \
    "  hit.cell = cell;\n" \
    "  hit.back = span - met;\n" \
    "  hit.at = from + along * met;\n" \
    "  hit.height = u_rise.y - ray.fall * met;\n" \
    "  return hit;\n" \
    "}\n"

/* What the room's actor shader needs to stand on the world map
 * instead. */
static const char kWorldVersion[] = GLSL_VERSION "#define WORLD\n";
static const char kWorldChunk[] = GLSL_WORLD GLSL_TERRAIN;

/* World map ground: each pixel's ray, met with the plane or with what
 * stands on it. */
static const char kGroundFragment[] =
    GLSL_VERSION GLSL_ROOM GLSL_WORLD GLSL_TERRAIN
    "uniform vec4 u_focus;\n"    /* far, near blur per distance; maximum;
                                    brightness^2 */
    "uniform vec4 u_haze;\n"     /* colour; amount */
    "out vec4 o_colour;\n"
    "float hash(vec2 p) {\n"
    "  vec3 q = fract(vec3(p.xyx) * 0.1031);\n"
    "  q += dot(q, q.yzx + 33.33);\n"
    "  return fract((q.x + q.y) * q.z);\n"
    "}\n"
    "float noise(vec2 p) {\n"
    "  vec2 i = floor(p), f = fract(p);\n"
    "  f = f * f * (3.0 - 2.0 * f);\n"
    "  return mix(mix(hash(i), hash(i + vec2(1.0, 0.0)), f.x),\n"
    "             mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)),\n"
    "                 f.x), f.y);\n"
    "}\n"
    "void main(void) {\n"
    "  Ray ray = view_ray(picture_point());\n"
    "  vec2 footprint = fwidth(ray.ground);\n"
    "  vec2 dx = dFdx(ray.ground) * u_extent.zw;\n"
    "  vec2 dy = dFdy(ray.ground) * u_extent.zw;\n"
    "  vec3 haze = u_haze.rgb * u_focus.w;\n"
    "  if (ray.fall <= 1e-4) {\n"
    "    vec3 zenith = vec3(0.16, 0.34, 0.70) * u_focus.w;\n"
    "    o_colour = vec4(mix(haze, zenith, clamp(-ray.fall * 3.0, 0.0, 1.0)),\n"
    "                    u_focus.z);\n"
    "    return;\n"
    "  }\n"
    "  Hit hit = terrain(ray);\n"
    "  vec2 p = hit.at;\n"
    "  float mine = card(hit.cell);\n"
    "  vec3 colour;\n"
    "  if (hit.wall) {\n"
    "    colour = texelFetch(u_plane,\n"
    "                        hit.cell & (ivec2(u_extent.xy) - 1), 0).rgb;\n"
    "  } else {\n"
    /* Crisp texels up close, filtered ones where they shrink. A leaning
     * face shows its texels taller than the ground does. */
    "    float lean = mine > 0.0 ? u_rise.x * -ray.along.y : 0.0;\n"
    "    vec2 seam = footprint * vec2(1.0, ray.fall / (ray.fall + lean));\n"
    "    vec2 base = floor(p - 0.5) + 0.5;\n"
    "    vec2 f = clamp((p - base - 0.5) / max(seam, vec2(1e-4)) + 0.5,\n"
    "                   0.0, 1.0);\n"
    /* Texels blend along a seam only with their neighbours on the same
     * card: across a crest the texel beside this one is not what shows. */
    "    if (u_rise.z > 0.5) {\n"
    "      ivec2 low = ivec2(floor(p - 0.5));\n"
    "      ivec2 side = clamp(hit.cell - low, ivec2(0), ivec2(1));\n"
    "      ivec2 other = hit.cell + 1 - 2 * side;\n"
    "      if (card(ivec2(other.x, hit.cell.y)) != mine) f.x = float(side.x);\n"
    "      if (card(ivec2(hit.cell.x, other.y)) != mine) f.y = float(side.y);\n"
    "    }\n"
    "    colour = textureGrad(u_plane, (base + f) * u_extent.zw, dx, dy).rgb;\n"
    "  }\n"
    "  colour = to_linear(colour) * u_focus.w;\n"
    "  if (u_rise.z > 0.5 && block_at(ivec2(floor(p / 16.0))).a > 0.25) {\n"
    /* What stands is lit from above: bright at its crest, dim at its
     * foot. */
    "    if (mine > 0.0)\n"
    "      colour *= mix(0.84, 1.10,\n"
    "                    smoothstep(0.0, u_rise.x * 16.0, hit.height));\n"
    /* And throws its shadow east and to the south. */
    "    float shade = 0.0;\n"
    "    for (int k = 1; k <= 8; k++) {\n"
    "      float along = float(k) / 8.0;\n"
    "      float t = along * along * u_rise.y;\n"
    "      float over = height_at(p - vec2(0.72, 0.52) * t) -\n"
    "                   (hit.height + t);\n"
    "      shade = max(shade, smoothstep(0.0, 1.2, over) *\n"
    "                         (1.0 - 0.45 * along));\n"
    "    }\n"
    "    colour *= 1.0 - 0.46 * shade;\n"
    "  }\n"
    /* Relief from the map's own shading, lit from the north-west: read
     * within one card, since what lies beside a crest in the plane is not
     * what lies beside it in the picture. */
    "  vec2 slope = vec2(-0.9, -1.1);\n"
    "  vec4 lit_side = textureGrad(u_plane, (p + slope) * u_extent.zw,\n"
    "                              dx, dy);\n"
    "  vec4 dark_side = textureGrad(u_plane, (p - slope) * u_extent.zw,\n"
    "                               dx, dy);\n"
    "  if (card(ivec2(floor(p + slope))) == mine &&\n"
    "      card(ivec2(floor(p - slope))) == mine)\n"
    "    colour *= 1.0 + 0.30 * clamp((luma(lit_side.rgb) -\n"
    "                                  luma(dark_side.rgb)) * 2.2,\n"
    "                                 -0.6, 0.9);\n"
    /* Cloud shadows drifting over the land. */
    "  vec2 drift = (p + u_pp.w * vec2(7.0, 3.0)) / 230.0;\n"
    "  float cloud = 0.50 * noise(drift) + 0.28 * noise(drift * 2.1 + 5.0) +\n"
    "                0.14 * noise(drift * 4.3 + 9.0) +\n"
    "                0.08 * noise(drift * 8.9 + 2.0);\n"
    "  colour *= 1.0 - 0.20 * smoothstep(0.40, 0.78, cloud);\n"
    "  float far = (ray.reach - hit.back) / u_eye.z;\n"
    "  colour = mix(colour, haze,\n"
    "               u_haze.a * (1.0 - exp(-max(far - 1.0, 0.0) * 0.42)));\n"
    "  float blur = far > 1.0 ? (far - 1.30) * u_focus.x\n"
    "                         : (0.80 - far) * u_focus.y;\n"
    "  o_colour = vec4(colour, clamp(blur, 0.0, u_focus.z));\n"
    "}\n";

/* A town or a dungeon. It is shown as the game shows it, from the game's
 * own point of view and pixel for pixel: nothing moves against anything
 * else. What it gains is light. Every pixel of the map's picture has a
 * height -- it shows a point that many rows above the ground, and as many
 * rows south of where it is drawn -- and from the heights come the shadows
 * things throw, the shade at their feet and the fall of the light. */
#define GLSL_FIELD \
    "uniform sampler2D u_lower;\n" \
    "uniform sampler2D u_upper;\n" \
    "uniform sampler2D u_relief;\n"   /* r upper, g lower height; b: drawn
                                         over the sprites; a: the upper
                                         layer has picture */ \
    "uniform vec4 u_window;\n"   /* picture column, row of the window's first
                                    pixel; its size in pixels */ \
    "uniform vec4 u_rise;\n"     /* -; greatest height in rows; -; 1 when
                                    anything has height */ \
    "uniform vec4 u_view;\n"     /* content x, y, 1/width, 1/height */ \
    "vec2 picture_point() {\n" \
    "  vec2 unit = (gl_FragCoord.xy - u_view.xy) * u_view.zw;\n" \
    "  return vec2(unit.x * u_pic.x, (1.0 - unit.y) * u_pic.y);\n" \
    "}\n" \
    "bool in_window(vec2 w) {\n" \
    "  return w.x >= 0.0 && w.y >= 0.0 && w.x < u_window.z &&\n" \
    "         w.y < u_window.w;\n" \
    "}\n" \
    /* Height of what the picture shows at a window pixel, and the layer
     * that shows it. */ \
    "float surface(vec2 w, out int layer) {\n" \
    "  layer = 2;\n" \
    "  if (!in_window(w)) return 0.0;\n" \
    "  vec4 relief = texelFetch(u_relief, ivec2(w), 0);\n" \
    "  if (relief.a > 0.5) {\n" \
    "    layer = 1;\n" \
    "    return relief.r * 255.0;\n" \
    "  }\n" \
    "  return relief.g * 255.0;\n" \
    "}\n"

static const char kFieldVersion[] = GLSL_VERSION "#define FIELD\n";
static const char kFieldChunk[] = GLSL_FIELD;

static const char kFieldFragment[] =
    GLSL_VERSION GLSL_ROOM GLSL_FIELD
    "uniform vec4 u_focus;\n"    /* blur per row beyond the sharp band;
                                    the band's half height in rows; maximum
                                    blur; brightness^2 */
    "uniform vec4 u_haze;\n"     /* colour; amount */
    "uniform vec4 u_light;\n"    /* 1 indoors; ambient; key; pool radius */
    "uniform vec4 u_centre;\n"   /* picture column, row the light and the
                                    focus are centred on; map column, row
                                    of the picture's first pixel */
    "uniform vec3 u_backdrop;\n"
    "out vec4 o_colour;\n"
    "float hash(vec2 p) {\n"
    "  vec3 q = fract(vec3(p.xyx) * 0.1031);\n"
    "  q += dot(q, q.yzx + 33.33);\n"
    "  return fract((q.x + q.y) * q.z);\n"
    "}\n"
    "float noise(vec2 p) {\n"
    "  vec2 i = floor(p), f = fract(p);\n"
    "  f = f * f * (3.0 - 2.0 * f);\n"
    "  return mix(mix(hash(i), hash(i + vec2(1.0, 0.0)), f.x),\n"
    "             mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)),\n"
    "                 f.x), f.y);\n"
    "}\n"
    /* Nearest within a texel, a seam as wide as one screen pixel between. */
    "vec2 crisp_at(vec2 w, vec2 seam) {\n"
    "  vec2 base = floor(w - 0.5) + 0.5;\n"
    "  vec2 f = clamp((w - base - 0.5) / max(seam, vec2(1e-4)) + 0.5,\n"
    "                 0.0, 1.0);\n"
    "  return (base + f) / u_window.zw;\n"
    "}\n"
    "void main(void) {\n"
    "  vec2 s = picture_point();\n"
    "  vec2 w = s - u_window.xy;\n"
    "  vec2 footprint = fwidth(s);\n"
    "  vec3 haze = u_haze.rgb * u_focus.w;\n"
    "  int layer;\n"
    "  float height = surface(w, layer);\n"
    "  vec3 colour = u_backdrop;\n"
    "  if (in_window(w)) {\n"
    "    colour = texture(u_lower, crisp_at(w, footprint)).rgb;\n"
    "    if (layer == 1) {\n"
    "      vec4 over = texture(u_upper, crisp_at(w, footprint));\n"
    "      colour = over.rgb + (1.0 - over.a) * colour;\n"
    "    }\n"
    "  }\n"
    "  colour = to_linear(colour) * u_focus.w;\n"
    /* The ground under this point. */
    "  vec2 under = s + vec2(0.0, height);\n"
    "  float lit = 1.0;\n"
    "  if (u_rise.w > 0.5) {\n"
    /* What rises throws a shadow east and to the south: a thing as far
     * north-west as it is tall reaches this spot. */
    "    float shade = 0.0;\n"
    "    float reach = min(u_rise.y, 72.0);\n"
    "    for (int i = 1; i <= 16; i++) {\n"
    "      float up = reach * (float(i) - 0.5 * hash(s + float(i))) / 16.0;\n"
    "      int other;\n"
    "      float over = surface(w - vec2(0.72 * up, 1.52 * up), other) -\n"
    "                   (height + up);\n"
    "      shade = max(shade, smoothstep(0.0, 2.0, over) *\n"
    "                         (1.0 - 0.5 * up / reach));\n"
    "    }\n"
    "    lit *= 1.0 - 0.44 * shade;\n"
    /* And darkens the ground at its foot. */
    "    if (height < 0.5) {\n"
    "      int other;\n"
    "      float foot = max(surface(w - vec2(0.0, 2.0), other),\n"
    "                       surface(w - vec2(0.0, 5.0), other) - 3.0);\n"
    "      lit *= 1.0 - 0.24 * smoothstep(0.5, 6.0, foot);\n"
    "    } else {\n"
    /* The light comes from above: bright at a crest, dim at a foot. */
    "      lit *= mix(0.90, 1.07, smoothstep(0.0, 40.0, height));\n"
    "    }\n"
    "  }\n"
    "  if (u_light.x > 0.5) {\n"
    /* Indoors the light is where the party is. */
    "    vec2 away = (under - u_centre.xy) / u_light.w;\n"
    "    lit *= u_light.y + u_light.z / (1.0 + 1.6 * dot(away, away));\n"
    "  } else {\n"
    /* Outdoors clouds drift over, and they keep to the map, not to the
     * picture scrolling over it. */
    "    vec2 drift = (under + u_centre.zw + u_pp.w * vec2(5.0, 2.0)) / 190.0;\n"
    "    float cloud = 0.50 * noise(drift) + 0.28 * noise(drift * 2.1 + 5.0) +\n"
    "                  0.14 * noise(drift * 4.3 + 9.0) +\n"
    "                  0.08 * noise(drift * 8.9 + 2.0);\n"
    "    lit *= u_light.y + u_light.z *\n"
    "                       (1.0 - 0.26 * smoothstep(0.42, 0.78, cloud));\n"
    "  }\n"
    "  colour *= lit;\n"
    /* Depth is counted in rows of ground from the row the party stands on:
     * sharp in a band about it, soft and a little hazy beyond. */
    "  float north = u_centre.y - under.y;\n"
    "  colour = mix(colour, haze,\n"
    "               u_haze.a * smoothstep(40.0, 190.0, north));\n"
    "  float blur = (abs(north) - u_focus.y) * u_focus.x;\n"
    "  o_colour = vec4(colour, clamp(blur, 0.0, u_focus.z));\n"
    "}\n";

/* A field's billboards: picture column, height above the ground
 * and picture row of the ground, seen as the game sees them. */
static const char kFieldQuadVertex[] =
    GLSL_VERSION
    "layout(location = 0) in vec3 a_position;\n"
    "layout(location = 1) in vec4 a_a;\n"
    "layout(location = 2) in vec4 a_b;\n"
    "uniform vec4 u_ndc;\n"
    "out vec4 v_a;\n"
    "out vec4 v_b;\n"
    "out vec3 v_world;\n"
    "void main(void) {\n"
    "  vec2 picture = vec2(a_position.x, a_position.z - a_position.y);\n"
    "  gl_Position = vec4(u_ndc.x * picture.x + u_ndc.y,\n"
    "                     u_ndc.z * picture.y + u_ndc.w, 0.0, 1.0);\n"
    "  v_a = a_a;\n"
    "  v_b = a_b;\n"
    "  v_world = a_position;\n"
    "}\n";

/* World map billboards: plane x, height above the ground and
 * plane y, seen through the tilted camera. */
static const char kWorldQuadVertex[] =
    GLSL_VERSION GLSL_WORLD
    "layout(location = 0) in vec3 a_position;\n"
    "layout(location = 1) in vec4 a_a;\n"
    "layout(location = 2) in vec4 a_b;\n"
    "uniform vec4 u_ndc;\n"
    "out vec4 v_a;\n"
    "out vec4 v_b;\n"
    "out vec3 v_world;\n"
    "void main(void) {\n"
    /* The eye stands back from the focus and above the ground. */
    "  float east = a_position.x - u_eye.x;\n"
    "  float north = u_eye.y + u_eye.z * u_tilt.y - a_position.z;\n"
    "  float up = a_position.y - u_eye.z * u_tilt.x;\n"
    "  float rise = up * u_tilt.y + north * u_tilt.x;\n"
    "  float w = max(north * u_tilt.y - up * u_tilt.x, 1.0);\n"
    "  float sx = u_tilt.z + u_eye.w * east / w;\n"
    "  float sy = u_tilt.w - u_eye.w * u_stretch * rise / w;\n"
    "  vec2 ndc = vec2(u_ndc.x * sx + u_ndc.y, u_ndc.z * sy + u_ndc.w);\n"
    "  gl_Position = vec4(ndc * w, 0.0, w);\n"
    "  v_a = a_a;\n"
    "  v_b = a_b;\n"
    "  v_world = a_position;\n"
    "}\n";

static const char kBlitVertex[] =
    GLSL_VERSION
    "out vec2 v_uv;\n"
    "void main(void) {\n"
    "  vec2 p = vec2(float((gl_VertexID << 1) & 2), float(gl_VertexID & 2));\n"
    "  v_uv = vec2(p.x, 1.0 - p.y);\n"
    "  gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);\n"
    "}\n";

static const char kBlitFragment[] =
    GLSL_VERSION
    "uniform sampler2D u_source;\n"
    "in vec2 v_uv;\n"
    "out vec4 o_colour;\n"
    "void main(void) { o_colour = vec4(texture(u_source, v_uv).rgb, 1.0); }\n";

/* ---- resources ------------------------------------------------------ */

static GLuint Compile(GLenum type, const char *const *parts, int count,
                      const char *name) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, count, parts, NULL);
    glCompileShader(shader);
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        char log[400];
        GLsizei length = 0;
        glGetShaderInfoLog(shader, (GLsizei)sizeof log, &length, log);
        log[sizeof log - 1] = '\0';
        SetError("%s %s shader: %s", name,
                 type == GL_VERTEX_SHADER ? "vertex" : "fragment",
                 length > 0 ? log : "no driver log");
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

/* A fragment shader may come in parts, the first of which opens with the
 * version. */
static bool LinkParts(Program *program, const char *vertex,
                      const char *const *fragment, int count,
                      const char *name) {
    const GLuint vs = Compile(GL_VERTEX_SHADER, &vertex, 1, name);
    const GLuint fs =
        vs ? Compile(GL_FRAGMENT_SHADER, fragment, count, name) : 0;
    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        return false;
    }
    program->id = glCreateProgram();
    glAttachShader(program->id, vs);
    glAttachShader(program->id, fs);
    glLinkProgram(program->id);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint linked = GL_FALSE;
    glGetProgramiv(program->id, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        char log[400];
        GLsizei length = 0;
        glGetProgramInfoLog(program->id, (GLsizei)sizeof log, &length, log);
        log[sizeof log - 1] = '\0';
        SetError("%s program: %s", name, length > 0 ? log : "no driver log");
        glDeleteProgram(program->id);
        program->id = 0;
        return false;
    }
    return true;
}

static bool Link(Program *program, const char *vertex, const char *fragment,
                 const char *name) {
    return LinkParts(program, vertex, &fragment, 1, name);
}

static GLint Uniform(const Program *program, const char *name) {
    return glGetUniformLocation(program->id, name);
}

static void Set1i(const Program *p, const char *n, int v) {
    glUniform1i(Uniform(p, n), v);
}
static void Set1f(const Program *p, const char *n, float v) {
    glUniform1f(Uniform(p, n), v);
}
static void Set2f(const Program *p, const char *n, float x, float y) {
    glUniform2f(Uniform(p, n), x, y);
}
static void Set3f(const Program *p, const char *n, const float v[3]) {
    glUniform3f(Uniform(p, n), v[0], v[1], v[2]);
}
static void Set4f(const Program *p, const char *n, float x, float y,
                  float z, float w) {
    glUniform4f(Uniform(p, n), x, y, z, w);
}

static void DestroyTarget(Target *target) {
    if (target->framebuffer) glDeleteFramebuffers(1, &target->framebuffer);
    if (target->texture) glDeleteTextures(1, &target->texture);
    memset(target, 0, sizeof *target);
}

static bool CreateTarget(Target *target, int width, int height,
                         GLint internal_format, GLenum type, bool mipmaps) {
    DestroyTarget(target);
    target->width = width;
    target->height = height;
    glGenTextures(1, &target->texture);
    glBindTexture(GL_TEXTURE_2D, target->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0,
                 GL_RGBA, type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (mipmaps)
        glGenerateMipmap(GL_TEXTURE_2D);
    glGenFramebuffers(1, &target->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, target->framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, target->texture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        SetError("render target %dx%d is incomplete", width, height);
        return false;
    }
    return true;
}

static bool EnsureTargets(int width, int height) {
    if (s_gl.width == width && s_gl.height == height)
        return true;
    s_gl.width = s_gl.height = 0;
    if (!CreateTarget(&s_gl.scene_back, width, height, GL_RGBA16F,
                      GL_HALF_FLOAT, true) ||
        !CreateTarget(&s_gl.scene, width, height, GL_RGBA16F,
                      GL_HALF_FLOAT, true) ||
        !CreateTarget(&s_gl.output, width, height, GL_RGBA8,
                      GL_UNSIGNED_BYTE, false))
        return false;
    for (int level = 0; level < BLOOM_LEVELS; level++) {
        int w = width >> (level + 1), h = height >> (level + 1);
        if (w < 1) w = 1;
        if (h < 1) h = 1;
        if (!CreateTarget(&s_gl.bloom[level], w, h, GL_RGBA16F,
                          GL_HALF_FLOAT, false))
            return false;
    }
    s_gl.width = width;
    s_gl.height = height;
    return true;
}

static bool EnsureReady(void) {
    if (s_gl.ready) return true;
    if (s_gl.failed) return false;
    s_gl.failed = true;

    const char *const actor[] = {kVersion, kRoomChunk, kActorFragment};
    const char *const world_actor[] = {
        kWorldVersion, kRoomChunk, kWorldChunk, kActorFragment,
    };
    const char *const field_actor[] = {
        kFieldVersion, kRoomChunk, kFieldChunk, kActorFragment,
    };
    if (!Link(&s_gl.background, kFullscreenVertex, kBackgroundFragment,
              "background") ||
        !Link(&s_gl.reflection, kQuadVertex, kReflectionFragment,
              "reflection") ||
        !LinkParts(&s_gl.actor, kQuadVertex, actor, 3, "actor") ||
        !Link(&s_gl.focus, kFullscreenVertex, kFocusFragment, "focus") ||
        !Link(&s_gl.down, kFullscreenVertex, kDownFragment, "bloom down") ||
        !Link(&s_gl.up, kFullscreenVertex, kUpFragment, "bloom up") ||
        !Link(&s_gl.final, kFullscreenVertex, kFinalFragment, "final") ||
        !Link(&s_gl.blit, kBlitVertex, kBlitFragment, "blit") ||
        !Link(&s_gl.plane_bake, kFullscreenVertex, kPlaneBakeFragment,
              "plane bake") ||
        !Link(&s_gl.ground, kFullscreenVertex, kGroundFragment, "ground") ||
        !LinkParts(&s_gl.world_actor, kWorldQuadVertex, world_actor, 4,
                   "world actor") ||
        !Link(&s_gl.field, kFullscreenVertex, kFieldFragment, "field") ||
        !LinkParts(&s_gl.field_actor, kFieldQuadVertex, field_actor, 4,
                   "field actor"))
        return false;

    glGenVertexArrays(1, &s_gl.empty_vao);
    glGenVertexArrays(1, &s_gl.quad_vao);
    glGenBuffers(1, &s_gl.quad_vbo);
    glBindVertexArray(s_gl.quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_gl.quad_vbo);
    const GLsizei stride = QUAD_FLOATS * (GLsizei)sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride,
                          (void *)(3 * sizeof(float)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride,
                          (void *)(7 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    glGenTextures(3, s_gl.texture);
    for (int i = 0; i < 3; i++) {
        glBindTexture(GL_TEXTURE_2D, s_gl.texture[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    s_gl.failed = false;
    s_gl.ready = true;
    return true;
}

void Lufia2Hd2dGlShutdown(void) {
    if (!s_gl.ready && !s_gl.failed)
        return;
    Program *programs[] = {
        &s_gl.background, &s_gl.reflection, &s_gl.actor,
        &s_gl.focus, &s_gl.down, &s_gl.up, &s_gl.final, &s_gl.blit,
        &s_gl.plane_bake, &s_gl.ground, &s_gl.world_actor, &s_gl.field,
        &s_gl.field_actor,
    };
    for (size_t i = 0; i < sizeof programs / sizeof programs[0]; i++)
        if (programs[i]->id) glDeleteProgram(programs[i]->id);
    DestroyTarget(&s_gl.scene_back);
    DestroyTarget(&s_gl.scene);
    DestroyTarget(&s_gl.output);
    DestroyTarget(&s_gl.plane);
    for (int level = 0; level < BLOOM_LEVELS; level++)
        DestroyTarget(&s_gl.bloom[level]);
    if (s_gl.plane_tiles) {
        const GLuint textures[4] = {
            s_gl.plane_tiles, s_gl.plane_chars, s_gl.plane_palette,
            s_gl.plane_terrain,
        };
        glDeleteTextures(4, textures);
    }
    if (s_gl.field_lower) {
        const GLuint textures[3] = {
            s_gl.field_lower, s_gl.field_upper, s_gl.field_relief,
        };
        glDeleteTextures(3, textures);
    }
    if (s_gl.texture[0]) glDeleteTextures(3, s_gl.texture);
    if (s_gl.quad_vbo) glDeleteBuffers(1, &s_gl.quad_vbo);
    if (s_gl.quad_vao) glDeleteVertexArrays(1, &s_gl.quad_vao);
    if (s_gl.empty_vao) glDeleteVertexArrays(1, &s_gl.empty_vao);
    memset(&s_gl, 0, sizeof s_gl);
}

static void Upload(const Lufia2Hd2dScene *scene) {
    const struct { const uint8_t *pixels; unsigned width, height; }
        pictures[3] = {
            {scene->background, scene->width, scene->height},
            {scene->actors, scene->actors_width, scene->actors_height},
            {scene->interface, scene->width, scene->height},
        };
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    for (int i = 0; i < 3; i++) {
        glBindTexture(GL_TEXTURE_2D, s_gl.texture[i]);
        if (s_gl.texture_width[i] != pictures[i].width ||
            s_gl.texture_height[i] != pictures[i].height)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                         (GLsizei)pictures[i].width,
                         (GLsizei)pictures[i].height, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, pictures[i].pixels);
        else
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                            (GLsizei)pictures[i].width,
                            (GLsizei)pictures[i].height, GL_RGBA,
                            GL_UNSIGNED_BYTE, pictures[i].pixels);
        s_gl.texture_width[i] = pictures[i].width;
        s_gl.texture_height[i] = pictures[i].height;
    }
}

/* ---- staging -------------------------------------------------------- */

typedef struct Stage {
    const Hd2dLook *look;
    float centre_x;              /* picture column of the optical axis */
    float wall_z;
    float camera[3];
    float lens[2];
    float zoom;
    float focus_row, focus_half;
    float light_x, light_z, light_radius;
} Stage;

static const Hd2dLook *LookFor(uint8_t background_id) {
    const size_t count = sizeof kLooks / sizeof kLooks[0];
    /* An id outside the table gets the plain cave staging. */
    return &kLooks[background_id < count ? background_id : 0x0bu];
}

/* Depth at which the floor shows on a picture row of the reference view. */
static float RowDepth(const Stage *stage, float row) {
    const float base = stage->look->wall_base + 1.0f;
    if (row < base) row = base;
    return kFocal * kEye / (row - kHorizon);
}

static float PoolLight(const Stage *stage, float x, float z) {
    const float dx = (x - stage->light_x) / stage->light_radius;
    const float dz = (z - stage->light_z) / stage->light_radius;
    return 0.62f + 0.78f / (1.0f + 1.5f * (dx * dx + dz * dz));
}

static void BuildStage(const Lufia2Hd2dScene *scene,
                       const Lufia2Hd2dView *view, Stage *stage) {
    stage->look = LookFor(scene->background_id);
    stage->centre_x = (float)scene->native_x + 128.0f;
    stage->wall_z = kFocal * kEye / (stage->look->wall_base - kHorizon);

    /* Everyone who stands on the floor is in focus and shares the light. */
    float near_row = 0.0f, far_row = 1e9f, sum_x = 0.0f, sum_z = 0.0f;
    unsigned standing = 0;
    for (unsigned i = 0; i < scene->actor_count; i++) {
        const Lufia2Hd2dActor *actor = &scene->actor[i];
        if (actor->kind == LUFIA2_HD2D_ACTOR_FLOATER)
            continue;
        const float foot = (float)actor->foot;
        const float z = RowDepth(stage, foot);
        if (foot > near_row) near_row = foot;
        if (foot < far_row) far_row = foot;
        sum_x += ((float)(actor->x0 + actor->x1) * 0.5f - stage->centre_x) *
                 z / kFocal;
        sum_z += z;
        standing++;
    }
    if (!standing) {
        near_row = 168.0f;
        far_row = 104.0f;
        sum_x = 0.0f;
        sum_z = RowDepth(stage, 136.0f);
        standing = 1;
    }
    stage->focus_row = (near_row + far_row) * 0.5f;
    stage->focus_half = (near_row - far_row) * 0.5f + 14.0f;
    stage->light_x = sum_x / (float)standing;
    stage->light_z = sum_z / (float)standing;
    stage->light_radius = 1.25f * 128.0f * stage->light_z / kFocal;

    /* The camera arrives from behind and above, then only breathes. Its
     * lens follows it -- shifted for sideways travel, lengthened as it backs
     * away -- so the nearest standing row never moves on screen: that is the
     * row the interface is laid out against. Everything behind it slides
     * and swells into place. */
    const float away = 1.0f - (view->intro < 0.0f ? 0.0f
                               : view->intro > 1.0f ? 1.0f : view->intro);
    const float arrive = away * away * away;
    const float t = (float)view->time;
    const float reach = RowDepth(stage, near_row);
    stage->camera[0] =
        reach * (-0.080f * arrive + 0.0075f * sinf(t * 0.37f));
    stage->camera[1] =
        reach * (0.055f * arrive + 0.0030f * sinf(t * 0.29f + 1.3f));
    stage->camera[2] =
        reach * (-0.260f * arrive + 0.0055f * sinf(t * 0.23f + 2.1f));
    stage->zoom = (reach - stage->camera[2]) / reach;
    stage->lens[0] = kFocal * stage->camera[0] / reach;
    stage->lens[1] = -kFocal * stage->camera[1] / reach;
}

static float *PutVertex(float *out, float x, float y, float z,
                        const float a[4], const float b[4]) {
    out[0] = x;
    out[1] = y;
    out[2] = z;
    memcpy(out + 3, a, 4 * sizeof(float));
    memcpy(out + 7, b, 4 * sizeof(float));
    return out + QUAD_FLOATS;
}

/* Two triangles from four corners given in strip order. */
static float *PutQuad(float *out, const float corner[4][3],
                      const float a[4][4], const float b[4]) {
    static const int kOrder[QUAD_VERTICES] = {0, 1, 2, 2, 1, 3};
    for (int i = 0; i < QUAD_VERTICES; i++) {
        const int c = kOrder[i];
        out = PutVertex(out, corner[c][0], corner[c][1], corner[c][2],
                        a[c], b);
    }
    return out;
}

/* World position of a picture point lying in the upright plane at `z`. */
static void Upright(const Stage *stage, float column, float row, float z,
                    float out[3]) {
    out[0] = (column - stage->centre_x) * z / kFocal;
    out[1] = -(row - kHorizon) * z / kFocal;
    out[2] = z;
}

static int ByDepth(const void *a, const void *b) {
    const Lufia2Hd2dActor *x = *(const Lufia2Hd2dActor *const *)a;
    const Lufia2Hd2dActor *y = *(const Lufia2Hd2dActor *const *)b;
    return (x->foot > y->foot) - (x->foot < y->foot);
}

typedef struct Batches {
    float reflections[MAX_QUADS * QUAD_VERTICES * QUAD_FLOATS];
    float actors[MAX_QUADS * QUAD_VERTICES * QUAD_FLOATS];
    int reflection_vertices, actor_vertices;
} Batches;

static void BuildBatches(const Lufia2Hd2dScene *scene, const Stage *stage,
                         Batches *batches) {
    const Lufia2Hd2dActor *order[LUFIA2_HD2D_MAX_ACTORS];
    for (unsigned i = 0; i < scene->actor_count; i++)
        order[i] = &scene->actor[i];
    /* Far to near: nearer billboards paint over farther ones. */
    qsort(order, scene->actor_count, sizeof order[0], ByDepth);

    float *reflection = batches->reflections;
    float *billboard = batches->actors;

    for (unsigned i = 0; i < scene->actor_count; i++) {
        const Lufia2Hd2dActor *actor = order[i];
        const float x0 = (float)actor->x0, x1 = (float)actor->x1;
        const float y0 = (float)actor->y0, y1 = (float)actor->y1;
        const float foot = (float)actor->foot;
        const float z = RowDepth(stage, foot);
        const float box[4] = {x0, y0, x1, y1};
        const bool floater = actor->kind == LUFIA2_HD2D_ACTOR_FLOATER;

        float ground[3];
        Upright(stage, (x0 + x1) * 0.5f, foot, z, ground);
        const float pool = PoolLight(stage, ground[0], z);

        /* One texel of margin so the edge filter has something to fade to;
         * the fragment stage clips back to the box. */
        {
            const float columns[2] = {x0 - 1.0f, x1 + 1.0f};
            const float rows[2] = {y0 - 1.0f, y1 + 1.0f};
            float corner[4][3], a[4][4];
            for (int c = 0; c < 4; c++) {
                const float column = columns[c & 1], row = rows[c >> 1];
                Upright(stage, column, row, z, corner[c]);
                a[c][0] = column;
                a[c][1] = row;
                a[c][2] = floater ? 1.0f : 0.0f;
                a[c][3] = pool;
            }
            const float clip[4] = {x0 - 0.5f, y0 - 0.5f, x1 + 0.5f,
                                   y1 + 0.5f};
            billboard = PutQuad(billboard, corner, a, clip);
        }
        /* Whoever is drawn up against the backdrop has no floor underfoot
         * to be mirrored in. */
        if (floater || foot < stage->look->wall_base - 2.0f)
            continue;

        const float scale = z / kFocal;
        const float height = (foot - y0) * scale;

        /* Reflection: the billboard mirrored in the floor plane. */
        if (stage->look->reflect > 0.0f) {
            static const float kDepth[2] = {0.0f, 0.8f};
            const float columns[2] = {x0 - 1.0f, x1 + 1.0f};
            float corner[4][3], a[4][4];
            for (int c = 0; c < 4; c++) {
                const float column = columns[c & 1], depth = kDepth[c >> 1];
                corner[c][0] = (column - stage->centre_x) * scale;
                corner[c][1] = -kEye - height * depth;
                corner[c][2] = z;
                a[c][0] = column;
                a[c][1] = foot - (foot - y0) * depth;
                a[c][2] = depth / kDepth[1];
                a[c][3] = stage->look->reflect;
            }
            reflection = PutQuad(reflection, corner, a, box);
        }
    }
    batches->reflection_vertices =
        (int)((reflection - batches->reflections) / QUAD_FLOATS);
    batches->actor_vertices =
        (int)((billboard - batches->actors) / QUAD_FLOATS);
}

/* ---- passes --------------------------------------------------------- */

/* Picture coordinates to clip space and back, through the content
 * rectangle. */
static void SetNdc(const Program *program, const Lufia2Hd2dScene *scene,
                   const Lufia2Hd2dView *view) {
    Set4f(program, "u_ndc",
          2.0f * (float)view->content_width /
              ((float)scene->width * (float)view->drawable_width),
          2.0f * (float)view->content_x / (float)view->drawable_width - 1.0f,
          -2.0f * (float)view->content_height /
              ((float)scene->height * (float)view->drawable_height),
          2.0f * (float)(view->content_y + view->content_height) /
              (float)view->drawable_height - 1.0f);
    Set4f(program, "u_view", (float)view->content_x, (float)view->content_y,
          1.0f / (float)view->content_width,
          1.0f / (float)view->content_height);
}

static void SetCast(const Program *program, const Lufia2Hd2dScene *scene) {
    Set4f(program, "u_cast", (float)scene->actors_x, (float)scene->actors_y,
          1.0f / (float)scene->actors_width,
          1.0f / (float)scene->actors_height);
}

static void SetRoom(const Program *program, const Lufia2Hd2dScene *scene,
                    const Stage *stage, const Lufia2Hd2dView *view) {
    Set4f(program, "u_pic", (float)scene->width, (float)scene->height,
          1.0f / (float)scene->width, 1.0f / (float)scene->height);
    Set4f(program, "u_proj", kFocal, stage->centre_x, kHorizon, kEye);
    Set4f(program, "u_cam", stage->camera[0], stage->camera[1],
          stage->camera[2], stage->wall_z);
    Set4f(program, "u_pp", stage->lens[0], stage->lens[1],
          stage->look->wall_base, (float)view->time);
    Set1f(program, "u_zoom", stage->zoom);
    SetCast(program, scene);
    SetNdc(program, scene, view);
}

static void Bind(int unit, GLuint texture) {
    glActiveTexture((GLenum)(GL_TEXTURE0 + unit));
    glBindTexture(GL_TEXTURE_2D, texture);
}

static void DrawBatch(const Program *program, const float *vertices,
                      int count) {
    if (!count)
        return;
    glUseProgram(program->id);
    glBindVertexArray(s_gl.quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_gl.quad_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)((size_t)count * QUAD_FLOATS * sizeof(float)),
                 vertices, GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, count);
}

static void Fullscreen(const Target *target) {
    glBindFramebuffer(GL_FRAMEBUFFER, target->framebuffer);
    glViewport(0, 0, target->width, target->height);
    glBindVertexArray(s_gl.empty_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

/* What the passes shared by every kind of scene need from its own. */
typedef struct Finish {
    const Hd2dAir *air;
    float centre_x;              /* picture column the shafts fan out from */
    bool dust_where_lit;
} Finish;

static void FocusPass(void) {
    Bind(0, s_gl.scene_back.texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glUseProgram(s_gl.focus.id);
    Set1i(&s_gl.focus, "u_source", 0);
    Set2f(&s_gl.focus, "u_texel", 1.0f / (float)s_gl.width,
          1.0f / (float)s_gl.height);
    Fullscreen(&s_gl.scene);
}

static Batches s_batches;

/* Battle: the room, what marks its floor, then the actors in focus. */
static void DrawRoom(const Lufia2Hd2dScene *scene,
                     const Lufia2Hd2dView *view, Finish *finish) {
    Stage stage;
    BuildStage(scene, view, &stage);
    BuildBatches(scene, &stage, &s_batches);

    const Hd2dLook *look = stage.look;
    const float per_row =
        (float)view->content_height / (float)scene->height;
    finish->air = look->air;
    finish->centre_x = stage.centre_x;

    /* Room: backdrop and floor, blur radius in alpha. */
    glDisable(GL_BLEND);
    glUseProgram(s_gl.background.id);
    SetRoom(&s_gl.background, scene, &stage, view);
    Bind(0, s_gl.texture[0]);
    Set1i(&s_gl.background, "u_background", 0);
    {
        const float whole = (scene->background_cover - 0.80f) / 0.15f;
        Set4f(&s_gl.background, "u_columns", (float)scene->background_x0,
              (float)scene->background_x1,
              scene->brightness * scene->brightness,
              whole < 0.0f ? 0.0f : whole > 1.0f ? 1.0f : whole);
    }
    Set4f(&s_gl.background, "u_focus", stage.focus_row, stage.focus_half,
          0.046f * per_row, 0.032f * per_row);
    Set4f(&s_gl.background, "u_light", stage.light_x, stage.light_z,
          stage.light_radius, 2.6f * per_row);
    Set4f(&s_gl.background, "u_look", look->relief, look->air->fog_amount,
          0.62f, 0.78f);
    Set3f(&s_gl.background, "u_fog", look->air->fog);
    Fullscreen(&s_gl.scene_back);

    /* A glossy floor mirrors who stands on it; that does not touch the
     * stored blur radius. */
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    Bind(0, s_gl.texture[1]);
    glUseProgram(s_gl.reflection.id);
    SetRoom(&s_gl.reflection, scene, &stage, view);
    Set1i(&s_gl.reflection, "u_actors", 0);
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
    DrawBatch(&s_gl.reflection, s_batches.reflections,
              s_batches.reflection_vertices);
    glDisable(GL_BLEND);

    FocusPass();

    /* The actors, in focus, on top. */
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                        GL_ONE_MINUS_SRC_ALPHA);
    Bind(0, s_gl.texture[1]);
    glUseProgram(s_gl.actor.id);
    SetRoom(&s_gl.actor, scene, &stage, view);
    Set1i(&s_gl.actor, "u_actors", 0);
    DrawBatch(&s_gl.actor, s_batches.actors, s_batches.actor_vertices);
    glDisable(GL_BLEND);
}

/* ---- world map ------------------------------------------------------ */

typedef struct WorldStage {
    float focus_x, focus_y;      /* plane point the camera turns about */
    float sine, cosine;          /* of its pitch above the ground */
    /* What the tilt takes from a picture is given back by how it stands, so
     * that it shows as tall as it was drawn: the height a leaning card gains
     * for each texel of ground it covers, and the height of an upright one
     * for each row of its picture. */
    float lean, upright;
    float stretch;               /* of the picture, down the screen */
    float distance, focal;
    float centre_x, focus_row;   /* where that point lands in the picture */
} WorldStage;

static uint64_t Mix(uint64_t hash, const void *data, size_t size) {
    const uint8_t *bytes = (const uint8_t *)data;
    for (size_t i = 0; i < size; i++) {
        hash ^= bytes[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static void PlaneTexture(GLuint *texture) {
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

enum {
    TERRAIN_FOREST = 0x20,
    TERRAIN_SEA = 0x30,
    TERRAIN_MOUNTAIN = 0x40,
    /* The one figure on the world map that is no thing standing on the
     * ground but an opening in it: the stairs down to a cave, four
     * characters in a square of the plane's sheet. Nothing in the game's
     * data tells it from a house, so it is named here. */
    STAIRS_FIRST = 198,
    STAIRS_BELOW = 16,
    /* Rows a figure may be carried up by the rows below it. */
    WORLD_STACK = 2,
};

/* The game says what kind of terrain a block is, not which of its pixels
 * are the peak, the tree or the village and which the ground they stand on.
 * The colours tell, and need no table: a colour is ground if the open
 * classes of terrain -- land, sand, sea -- are drawn in it to any extent.
 * Whatever else a block is drawn in is a figure. */
static void FindGround(const Lufia2Hd2dPlane *plane,
                       uint8_t ground[LUFIA2_HD2D_PLANE_COLOURS]) {
    enum { CLASSES = 16 };
    static uint32_t tiles_of[CLASSES][256];
    memset(tiles_of, 0, sizeof tiles_of);
    for (unsigned y = 0; y < plane->tiles_high; y++) {
        const uint8_t *terrain =
            plane->terrain + (size_t)(y / 2u) * plane->terrain_wide;
        const uint8_t *tiles = plane->tiles + (size_t)y * plane->tiles_wide;
        for (unsigned x = 0; x < plane->tiles_wide; x++)
            tiles_of[terrain[x / 2u] >> 4][tiles[x]]++;
    }
    memset(ground, 0, LUFIA2_HD2D_PLANE_COLOURS);
    for (unsigned class = 0; class < CLASSES; class++) {
        if (class == TERRAIN_MOUNTAIN >> 4 || class == TERRAIN_FOREST >> 4)
            continue;
        uint64_t uses[LUFIA2_HD2D_PLANE_COLOURS] = {0};
        uint64_t total = 0;
        for (unsigned tile = 0; tile < 256u; tile++) {
            const uint32_t count = tiles_of[class][tile];
            if (!count)
                continue;
            const uint8_t *pixels = plane->chars + tile * 64u;
            for (unsigned i = 0; i < 64u; i++)
                uses[pixels[i]] += count;
            total += (uint64_t)count * 64u;
        }
        for (unsigned colour = 0; colour < LUFIA2_HD2D_PLANE_COLOURS;
             colour++)
            if (total && uses[colour] * 100u >= total)
                ground[colour] = 1;
    }
}

/* What the block upkeep reads of a tile: whether it holds any figure, and
 * which columns of its top and bottom rows are figure, and bright figure. */
typedef struct TileFacts {
    uint8_t figure;
    uint8_t flat;                /* drawn in the ground, not standing on it */
    uint8_t top, bottom, top_bright, bottom_bright;
} TileFacts;

static void FindTileFacts(const Lufia2Hd2dPlane *plane,
                          const uint8_t ground[LUFIA2_HD2D_PLANE_COLOURS],
                          TileFacts facts[256]) {
    enum { BRIGHT = 130 };
    bool bright[LUFIA2_HD2D_PLANE_COLOURS];
    for (unsigned i = 0; i < LUFIA2_HD2D_PLANE_COLOURS; i++) {
        const unsigned colour = plane->cgram[i];
        bright[i] = (299u * (colour & 0x1fu) + 587u * ((colour >> 5) & 0x1fu) +
                     114u * ((colour >> 10) & 0x1fu)) * 255u / 31000u >=
                    BRIGHT;
    }
    memset(facts, 0, 256u * sizeof facts[0]);
    for (unsigned tile = 0; tile < 256u; tile++) {
        const uint8_t *pixels = plane->chars + tile * 64u;
        TileFacts *fact = &facts[tile];
        if (tile == STAIRS_FIRST || tile == STAIRS_FIRST + 1u ||
            tile == STAIRS_FIRST + STAIRS_BELOW ||
            tile == STAIRS_FIRST + STAIRS_BELOW + 1u) {
            fact->flat = 1;
            continue;
        }
        for (unsigned i = 0; i < 64u; i++)
            fact->figure |= !ground[pixels[i]];
        for (unsigned x = 0; x < 8u; x++) {
            const uint8_t top = pixels[x], bottom = pixels[56u + x];
            fact->top |= (uint8_t)(!ground[top] << x);
            fact->bottom |= (uint8_t)(!ground[bottom] << x);
            fact->top_bright |= (uint8_t)((!ground[top] && bright[top]) << x);
            fact->bottom_bright |=
                (uint8_t)((!ground[bottom] && bright[bottom]) << x);
        }
    }
}

static unsigned Bits(unsigned value) {
    unsigned count = 0;
    for (; value; value &= value - 1u)
        count++;
    return count;
}

/* One texel per block for the bake and the ray walk. Red: nothing stands
 * (0), an opening in the ground (32), figures stand (128), or stand and are
 * mountains (255). Blue: how many
 * rows below carry this one, in 64ths of 255. Alpha: the block holds a
 * figure (255) or lies beside one that does (128), which is as far as
 * shadows are thrown. Green: zero where no figure stands within reach, which
 * is as far as a ray can come under a crest before it lands; else a third of
 * the rows the tallest crest in the block is up, its own or one a neighbour
 * lends it. Returns the tallest stack of rows. */
static unsigned UploadBlocks(const Lufia2Hd2dPlane *plane,
                             const TileFacts facts[256]) {
    enum { REACH = 3 };
    const unsigned wide = plane->terrain_wide, high = plane->terrain_high;
    const size_t blocks = (size_t)wide * high;
    uint8_t *texels = (uint8_t *)calloc(blocks, 4);
    uint8_t *figure = (uint8_t *)calloc(blocks, 2);
    if (!texels || !figure) {
        free(texels);
        free(figure);
        return 0;
    }
    uint8_t *row_near = figure + blocks;
    const uint8_t *terrain = plane->terrain;
#define AT(x, y) ((size_t)(((y) + high) % high) * wide + ((x) + wide) % wide)

    for (unsigned y = 0; y < high; y++)
        for (unsigned x = 0; x < wide; x++) {
            const uint8_t class = terrain[AT(x, y)];
            uint8_t stands = class == TERRAIN_MOUNTAIN ? 255u
                             : class == TERRAIN_SEA ? 0u : 128u;
            /* Land with the sea on both sides of it is a bridge. */
            if (stands == 128u && class != TERRAIN_FOREST &&
                ((terrain[AT(x - 1, y)] == TERRAIN_SEA &&
                  terrain[AT(x + 1, y)] == TERRAIN_SEA) ||
                 (terrain[AT(x, y - 1)] == TERRAIN_SEA &&
                  terrain[AT(x, y + 1)] == TERRAIN_SEA)))
                stands = 0;
            const uint8_t *tiles =
                plane->tiles + (size_t)(y * 2u) * plane->tiles_wide + x * 2u;
            /* An opening in the ground lies flat, and stays in sight: it
             * is where the party goes in. */
            if (facts[tiles[0]].flat || facts[tiles[1]].flat ||
                facts[tiles[plane->tiles_wide]].flat ||
                facts[tiles[plane->tiles_wide + 1u]].flat)
                stands = 32u;
            texels[AT(x, y) * 4u] = stands;
            figure[AT(x, y)] =
                stands >= 128u &&
                (facts[tiles[0]].figure || facts[tiles[1]].figure ||
                           facts[tiles[plane->tiles_wide]].figure ||
                           facts[tiles[plane->tiles_wide + 1u]].figure);
        }

    /* A figure taller than a block goes on in the block to the north: a
     * tower, a castle, a great snow peak. Then that block's card does not
     * start again at the ground but carries on from the crest below. Rows of
     * peaks are each their own, though rock meets rock between them, so
     * mountains go on only where the bright of a snowfield does. Forest is
     * rows of trees. */
    unsigned tallest = 0;
    for (unsigned pass = 0; pass < WORLD_STACK; pass++)
        for (unsigned y = 0; y < high; y++)
            for (unsigned x = 0; x < wide; x++) {
                const size_t here = AT(x, y), below = AT(x, y + 1);
                const uint8_t class = terrain[here];
                if (!figure[here] || !figure[below] ||
                    class != terrain[below] || class == TERRAIN_FOREST ||
                    texels[below * 4u + 2u] != pass * 64u)
                    continue;
                const uint8_t *upper =
                    plane->tiles + (size_t)(y * 2u + 1u) * plane->tiles_wide +
                    x * 2u;
                const uint8_t *lower =
                    plane->tiles +
                    (size_t)((y * 2u + 2u) % plane->tiles_high) *
                        plane->tiles_wide + x * 2u;
                unsigned joined = 0;
                for (unsigned i = 0; i < 2u; i++) {
                    const TileFacts *above = &facts[upper[i]];
                    const TileFacts *under = &facts[lower[i]];
                    joined += class == TERRAIN_MOUNTAIN
                        ? Bits(above->bottom_bright & under->top_bright)
                        : Bits(above->bottom & under->top);
                }
                if (joined >= 3u) {
                    texels[here * 4u + 2u] = (uint8_t)((pass + 1u) * 64u);
                    tallest = pass + 1u;
                }
            }

    for (unsigned y = 0; y < high; y++)
        for (unsigned x = 0; x < wide; x++) {
            uint8_t near = 0;
            for (int d = -REACH; d <= REACH; d++)
                near |= figure[AT((int)x + d, y)];
            row_near[AT(x, y)] = near;
        }
    for (unsigned y = 0; y < high; y++)
        for (unsigned x = 0; x < wide; x++) {
            uint8_t near = 0;
            for (int d = -REACH; d <= REACH; d++)
                near |= row_near[AT(x, (int)y + d)];
            unsigned rows = texels[AT(x, y) * 4u + 2u];
            for (int dx = -1; dx <= 1; dx += 2)
                if (texels[AT((int)x + dx, y) * 4u + 2u] > rows)
                    rows = texels[AT((int)x + dx, y) * 4u + 2u];
            texels[AT(x, y) * 4u + 1u] =
                near ? (uint8_t)(85u * (1u + rows / 64u)) : 0u;
            uint8_t beside = 0;
            for (int dy = -1; dy <= 1; dy++)
                for (int dx = -1; dx <= 1; dx++)
                    beside |= figure[AT((int)x + dx, (int)y + dy)];
            texels[AT(x, y) * 4u + 3u] =
                figure[AT(x, y)] ? 255u : beside ? 128u : 0u;
        }
#undef AT
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)wide, (GLsizei)high, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, texels);
    free(figure);
    free(texels);
    return tallest;
}

/* Keeps the plane's colour picture current. The game animates the sea by
 * rewriting characters and palette, so it is redrawn whenever any of its
 * inputs changes, and not otherwise. It is drawn at full brightness: a fade
 * would otherwise redraw it every frame. */
static bool EnsurePlane(const Lufia2Hd2dPlane *plane) {
    enum { MAX_ANISOTROPY = 0x84fe };
    const size_t cells = (size_t)plane->tiles_wide * plane->tiles_high;
    const int width = (int)plane->tiles_wide * 8;
    const int height = (int)plane->tiles_high * 8;
    /* Terrain is of use only where it lines up with the tiles, two a block,
     * on a plane the shaders can wrap by masking. */
    const bool relief =
        plane->terrain && plane->terrain_wide * 2u == plane->tiles_wide &&
        plane->terrain_high * 2u == plane->tiles_high &&
        !(plane->tiles_wide & (plane->tiles_wide - 1u)) &&
        !(plane->tiles_high & (plane->tiles_high - 1u));
    const size_t blocks =
        relief ? (size_t)plane->terrain_wide * plane->terrain_high : 0u;
    uint64_t signature = UINT64_C(1469598103934665603) ^ cells;
    signature = Mix(signature, plane->tiles, cells);
    signature = Mix(signature, plane->chars, LUFIA2_HD2D_PLANE_CHAR_BYTES);
    signature = Mix(signature, plane->cgram, sizeof plane->cgram);
    signature = Mix(signature, plane->terrain, blocks);
    const bool resized = s_gl.plane_wide != plane->tiles_wide ||
                         s_gl.plane_high != plane->tiles_high;
    s_gl.plane_relief = relief;
    if (!resized && signature == s_gl.plane_signature)
        return true;

    if (!s_gl.plane_tiles) {
        PlaneTexture(&s_gl.plane_tiles);
        PlaneTexture(&s_gl.plane_chars);
        PlaneTexture(&s_gl.plane_palette);
        PlaneTexture(&s_gl.plane_terrain);
        s_gl.blocks_signature = 0;
    }
    if (resized) {
        s_gl.plane_wide = s_gl.plane_high = 0;
        if (!CreateTarget(&s_gl.plane, width, height, GL_RGBA8,
                          GL_UNSIGNED_BYTE, true))
            return false;
        /* The world is a torus, and seen at a grazing angle. Anisotropy is
         * an extension; a driver without it just says no. */
        glBindTexture(GL_TEXTURE_2D, s_gl.plane.texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, MAX_ANISOTROPY, 8.0f);
        while (glGetError() != GL_NO_ERROR) {
        }
    }

    uint8_t ground[LUFIA2_HD2D_PLANE_COLOURS];
    uint8_t palette[LUFIA2_HD2D_PLANE_COLOURS][4];
    if (relief)
        FindGround(plane, ground);
    for (unsigned i = 0; i < LUFIA2_HD2D_PLANE_COLOURS; i++) {
        const unsigned colour = plane->cgram[i];
        palette[i][0] = (uint8_t)((colour & 0x1fu) * 255u / 31u);
        palette[i][1] = (uint8_t)(((colour >> 5) & 0x1fu) * 255u / 31u);
        palette[i][2] = (uint8_t)(((colour >> 10) & 0x1fu) * 255u / 31u);
        palette[i][3] = relief && !ground[i] ? 255u : 0u;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    Bind(0, s_gl.plane_tiles);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, (GLsizei)plane->tiles_wide,
                 (GLsizei)plane->tiles_high, 0, GL_RED, GL_UNSIGNED_BYTE,
                 plane->tiles);
    Bind(1, s_gl.plane_chars);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 128, 128, 0, GL_RED,
                 GL_UNSIGNED_BYTE, plane->chars);
    Bind(2, s_gl.plane_palette);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, LUFIA2_HD2D_PLANE_COLOURS, 1, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, palette);
    /* The sea's animation changes nothing the blocks are made from. */
    TileFacts facts[256];
    uint64_t blocks_signature = UINT64_C(1469598103934665603) ^ blocks;
    if (relief) {
        FindTileFacts(plane, ground, facts);
        blocks_signature = Mix(blocks_signature, facts, sizeof facts);
        blocks_signature = Mix(blocks_signature, plane->tiles, cells);
        blocks_signature = Mix(blocks_signature, plane->terrain, blocks);
    }
    Bind(3, s_gl.plane_terrain);
    if (blocks_signature != s_gl.blocks_signature) {
        s_gl.plane_stack = 0;
        if (relief) {
            s_gl.plane_stack = UploadBlocks(plane, facts);
        } else {
            static const uint8_t kNone[4] = {0, 0, 0, 0};
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, kNone);
        }
        s_gl.blocks_signature = blocks_signature;
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glDisable(GL_BLEND);
    glUseProgram(s_gl.plane_bake.id);
    Set1i(&s_gl.plane_bake, "u_tiles", 0);
    Set1i(&s_gl.plane_bake, "u_chars", 1);
    Set1i(&s_gl.plane_bake, "u_palette", 2);
    Set1i(&s_gl.plane_bake, "u_terrain", 3);
    Fullscreen(&s_gl.plane);
    Bind(0, s_gl.plane.texture);
    glGenerateMipmap(GL_TEXTURE_2D);

    s_gl.plane_wide = plane->tiles_wide;
    s_gl.plane_high = plane->tiles_high;
    s_gl.plane_signature = signature;
    return true;
}

static void BuildWorldStage(const Lufia2Hd2dScene *scene,
                            WorldStage *stage) {
    /* One plane pixel is one picture pixel where the party stands, as it is
     * in the game's view, so sprites keep their size. The map is in this
     * view from its first frame; nothing eases into it. */
    stage->sine = sinf(kWorldPitch);
    stage->cosine = cosf(kWorldPitch);
    stage->lean = (1.0f - stage->sine) / stage->cosine;
    stage->upright = 1.0f / stage->cosine;
    stage->stretch = 1.0f;
    stage->focal = kWorldFocal;
    stage->distance = kWorldFocal;
    stage->centre_x = (float)scene->native_x + 128.0f;
    stage->focus_row = kWorldFocusRow;
    stage->focus_x = scene->plane.origin_x + 128.0f;
    stage->focus_y = scene->plane.origin_y + kWorldPartyRow;
}

static void BuildWorldBatches(const Lufia2Hd2dScene *scene,
                              const WorldStage *stage, float origin_x,
                              float origin_y, Batches *batches) {
    const Lufia2Hd2dActor *order[LUFIA2_HD2D_MAX_ACTORS];
    for (unsigned i = 0; i < scene->actor_count; i++)
        order[i] = &scene->actor[i];
    /* North to south: nearer billboards paint over farther ones. */
    qsort(order, scene->actor_count, sizeof order[0], ByDepth);

    /* The picture shows the plane one to one, so a picture column is a
     * plane column and a row above the feet is a height above the ground. */
    const float east = origin_x - (float)scene->native_x;
    float *billboard = batches->actors;

    for (unsigned i = 0; i < scene->actor_count; i++) {
        const Lufia2Hd2dActor *actor = order[i];
        const float x0 = (float)actor->x0, x1 = (float)actor->x1;
        const float y0 = (float)actor->y0, y1 = (float)actor->y1;
        const float foot = (float)actor->foot;
        const float south = origin_y + foot;
        const bool floater = actor->kind == LUFIA2_HD2D_ACTOR_FLOATER;

        {
            const float columns[2] = {x0 - 1.0f, x1 + 1.0f};
            const float rows[2] = {y0 - 1.0f, y1 + 1.0f};
            float corner[4][3], a[4][4];
            for (int c = 0; c < 4; c++) {
                const float column = columns[c & 1], row = rows[c >> 1];
                corner[c][0] = east + column;
                corner[c][1] = (foot - row) * stage->upright;
                corner[c][2] = south;
                a[c][0] = column;
                a[c][1] = row;
                a[c][2] = floater ? 1.0f : 0.0f;
                a[c][3] = 1.0f;
            }
            const float clip[4] = {x0 - 0.5f, y0 - 0.5f, x1 + 0.5f,
                                   y1 + 0.5f};
            billboard = PutQuad(billboard, corner, a, clip);
        }
    }
    batches->reflection_vertices = 0;
    batches->actor_vertices =
        (int)((billboard - batches->actors) / QUAD_FLOATS);
}

static void SetWorld(const Program *program, const Lufia2Hd2dScene *scene,
                     const WorldStage *stage, const Lufia2Hd2dView *view) {
    Set4f(program, "u_pic", (float)scene->width, (float)scene->height,
          1.0f / (float)scene->width, 1.0f / (float)scene->height);
    Set4f(program, "u_eye", stage->focus_x, stage->focus_y, stage->distance,
          stage->focal);
    Set4f(program, "u_tilt", stage->sine, stage->cosine, stage->centre_x,
          stage->focus_row);
    Set1f(program, "u_stretch", stage->stretch);
    Set4f(program, "u_pp", 0.0f, 0.0f, 0.0f, (float)view->time);
    SetCast(program, scene);
    SetNdc(program, scene, view);
    /* The plane and what stands on it. */
    Bind(1, s_gl.plane.texture);
    Bind(2, s_gl.plane_terrain);
    Set1i(program, "u_plane", 1);
    Set1i(program, "u_terrain", 2);
    Set4f(program, "u_extent", (float)s_gl.plane.width,
          (float)s_gl.plane.height, 1.0f / (float)s_gl.plane.width,
          1.0f / (float)s_gl.plane.height);
    Set4f(program, "u_rise", stage->lean,
          stage->lean * 16.0f * (float)(1u + s_gl.plane_stack),
          s_gl.plane_relief ? 1.0f : 0.0f, 0.0f);
}

/* World map: the ground, what marks it, then the actors in focus. */
static bool DrawWorld(const Lufia2Hd2dScene *scene,
                      const Lufia2Hd2dView *view, Finish *finish) {
    WorldStage stage;
    if (!scene->plane.tiles || !scene->plane.chars ||
        !EnsurePlane(&scene->plane))
        return false;
    BuildWorldStage(scene, &stage);
    BuildWorldBatches(scene, &stage, scene->plane.origin_x,
                      scene->plane.origin_y, &s_batches);

    const float per_row =
        (float)view->content_height / (float)scene->height;
    const float lit = scene->brightness * scene->brightness;
    finish->air = &kAirWorld;
    finish->centre_x = stage.centre_x;

    glDisable(GL_BLEND);
    glUseProgram(s_gl.ground.id);
    SetWorld(&s_gl.ground, scene, &stage, view);
    Set4f(&s_gl.ground, "u_focus", 1.2f * per_row, 4.0f * per_row,
          2.2f * per_row, lit);
    Set4f(&s_gl.ground, "u_haze", kAirWorld.fog[0], kAirWorld.fog[1],
          kAirWorld.fog[2], kAirWorld.fog_amount);
    Fullscreen(&s_gl.scene_back);

    FocusPass();

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                        GL_ONE_MINUS_SRC_ALPHA);
    Bind(0, s_gl.texture[1]);
    glUseProgram(s_gl.world_actor.id);
    SetWorld(&s_gl.world_actor, scene, &stage, view);
    Set1i(&s_gl.world_actor, "u_actors", 0);
    DrawBatch(&s_gl.world_actor, s_batches.actors, s_batches.actor_vertices);
    glDisable(GL_BLEND);
    return true;
}

/* ---- field ---------------------------------------------------------- */

static void FieldTexture(GLuint *texture, GLint filter) {
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

static void UploadGround(const Lufia2Hd2dGround *ground) {
    const GLsizei wide = (GLsizei)ground->tiles_wide;
    const GLsizei high = (GLsizei)ground->tiles_high;
    if (!s_gl.field_lower) {
        FieldTexture(&s_gl.field_lower, GL_LINEAR);
        FieldTexture(&s_gl.field_upper, GL_LINEAR);
        FieldTexture(&s_gl.field_relief, GL_NEAREST);
    }
    const bool resized = s_gl.field_wide != ground->tiles_wide ||
                         s_gl.field_high != ground->tiles_high;
    const struct { GLuint texture; const uint8_t *pixels; GLsizei scale; }
        planes[3] = {
            {s_gl.field_lower, ground->lower, 8},
            {s_gl.field_upper, ground->upper, 8},
            {s_gl.field_relief, ground->relief, 8},
        };
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    for (int i = 0; i < 3; i++) {
        glBindTexture(GL_TEXTURE_2D, planes[i].texture);
        if (resized)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, wide * planes[i].scale,
                         high * planes[i].scale, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, planes[i].pixels);
        else
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, wide * planes[i].scale,
                            high * planes[i].scale, GL_RGBA,
                            GL_UNSIGNED_BYTE, planes[i].pixels);
    }

    s_gl.field_wide = ground->tiles_wide;
    s_gl.field_high = ground->tiles_high;
}

static void SetField(const Program *program, const Lufia2Hd2dScene *scene,
                     const Lufia2Hd2dView *view) {
    const Lufia2Hd2dGround *ground = &scene->ground;
    Set4f(program, "u_pic", (float)scene->width, (float)scene->height,
          1.0f / (float)scene->width, 1.0f / (float)scene->height);
    Set4f(program, "u_pp", 0.0f, 0.0f, 0.0f, (float)view->time);
    SetCast(program, scene);
    SetNdc(program, scene, view);
    Bind(1, s_gl.field_lower);
    Bind(2, s_gl.field_upper);
    Bind(3, s_gl.field_relief);
    Set1i(program, "u_lower", 1);
    Set1i(program, "u_upper", 2);
    Set1i(program, "u_relief", 3);
    Set4f(program, "u_window", (float)ground->origin_x,
          (float)ground->origin_y, (float)ground->tiles_wide * 8.0f,
          (float)ground->tiles_high * 8.0f);
    Set4f(program, "u_rise", 0.0f, (float)ground->tallest, 0.0f,
          ground->tallest ? 1.0f : 0.0f);
}

/* A town or a dungeon: the map as the game shows it, lit; what marks it;
 * then the sprites in focus. */
static bool DrawField(const Lufia2Hd2dScene *scene,
                      const Lufia2Hd2dView *view, Finish *finish) {
    const Lufia2Hd2dGround *ground = &scene->ground;
    if (!ground->lower || !ground->upper || !ground->relief ||
        !ground->tiles_wide || !ground->tiles_high)
        return false;
    UploadGround(ground);
    /* A picture column and row are a point of the ground, and a row above
     * the feet is a row of height: the game's own view. */
    const WorldStage stage = {.upright = 1.0f};
    BuildWorldBatches(scene, &stage, (float)scene->native_x, 0.0f,
                      &s_batches);

    const Hd2dAir *air = ground->enclosed ? &kAirRoom : &kAirTown;
    const float per_row =
        (float)view->content_height / (float)scene->height;
    const float lit = scene->brightness * scene->brightness;
    const float centre_x = (float)scene->native_x + 128.0f;
    finish->air = air;
    finish->centre_x = centre_x;
    finish->dust_where_lit = ground->enclosed;

    glDisable(GL_BLEND);
    glUseProgram(s_gl.field.id);
    SetField(&s_gl.field, scene, view);
    Set4f(&s_gl.field, "u_focus", 0.011f * per_row, 62.0f, 0.62f * per_row,
          lit);
    Set4f(&s_gl.field, "u_haze", air->fog[0], air->fog[1], air->fog[2],
          air->fog_amount);
    Set4f(&s_gl.field, "u_centre", centre_x, kFieldPartyRow,
          (float)ground->map_x, (float)ground->map_y);
    if (ground->enclosed)
        Set4f(&s_gl.field, "u_light", 1.0f, 0.34f, 0.92f, 120.0f);
    else
        Set4f(&s_gl.field, "u_light", 0.0f, 0.10f, 0.98f, 0.0f);
    glUniform3f(Uniform(&s_gl.field, "u_backdrop"),
                (float)ground->backdrop[0] / 255.0f,
                (float)ground->backdrop[1] / 255.0f,
                (float)ground->backdrop[2] / 255.0f);
    Fullscreen(&s_gl.scene_back);

    FocusPass();

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                        GL_ONE_MINUS_SRC_ALPHA);
    Bind(0, s_gl.texture[1]);
    glUseProgram(s_gl.field_actor.id);
    SetField(&s_gl.field_actor, scene, view);
    Set1i(&s_gl.field_actor, "u_actors", 0);
    DrawBatch(&s_gl.field_actor, s_batches.actors, s_batches.actor_vertices);
    glDisable(GL_BLEND);
    return true;
}

bool Lufia2Hd2dGlRender(const Lufia2Hd2dScene *scene,
                       const Lufia2Hd2dView *target_view,
                       int target_framebuffer) {
    if (!scene || !target_view || target_view->drawable_width <= 0 ||
        target_view->drawable_height <= 0 ||
        target_view->content_width <= 0 ||
        target_view->content_height <= 0 || !scene->width || !scene->height)
        return false;
    while (glGetError() != GL_NO_ERROR) {
    }

    /* Render at the target's size, or at the pixel budget if that is less;
     * everything below works in the render size. */
    Lufia2Hd2dView scaled = *target_view;
    const Lufia2Hd2dView *view = &scaled;
    {
        const float pixels = (float)target_view->drawable_width *
                             (float)target_view->drawable_height;
        if (pixels > kPixelBudget) {
            const float shrink = sqrtf(kPixelBudget / pixels);
            scaled.drawable_width =
                (int)((float)target_view->drawable_width * shrink + 0.5f);
            scaled.drawable_height =
                (int)((float)target_view->drawable_height * shrink + 0.5f);
            scaled.content_x =
                (int)((float)target_view->content_x * shrink + 0.5f);
            scaled.content_y =
                (int)((float)target_view->content_y * shrink + 0.5f);
            scaled.content_width =
                (int)((float)target_view->content_width * shrink + 0.5f);
            scaled.content_height =
                (int)((float)target_view->content_height * shrink + 0.5f);
        }
    }
    if (!EnsureReady() ||
        !EnsureTargets(view->drawable_width, view->drawable_height))
        return false;
    Upload(scene);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_CULL_FACE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    Finish finish = {0};
    if (scene->kind == LUFIA2_HD2D_WORLD_MAP) {
        if (!DrawWorld(scene, view, &finish))
            return false;
    } else if (scene->kind == LUFIA2_HD2D_MAP) {
        if (!DrawField(scene, view, &finish))
            return false;
    } else {
        DrawRoom(scene, view, &finish);
    }

    Bind(0, s_gl.scene.texture);
    glGenerateMipmap(GL_TEXTURE_2D);

    /* Glow: down the chain, then back up adding each level in. */
    glUseProgram(s_gl.down.id);
    Set1i(&s_gl.down, "u_source", 0);
    for (int level = 0; level < BLOOM_LEVELS; level++) {
        const Target *source =
            level ? &s_gl.bloom[level - 1] : &s_gl.scene;
        Bind(0, source->texture);
        Set2f(&s_gl.down, "u_texel", 1.0f / (float)source->width,
              1.0f / (float)source->height);
        Set1f(&s_gl.down, "u_threshold", level ? 0.0f : 0.60f);
        Fullscreen(&s_gl.bloom[level]);
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glUseProgram(s_gl.up.id);
    Set1i(&s_gl.up, "u_source", 0);
    for (int level = BLOOM_LEVELS - 1; level > 0; level--) {
        const Target *source = &s_gl.bloom[level];
        Bind(0, source->texture);
        Set2f(&s_gl.up, "u_texel", 1.0f / (float)source->width,
              1.0f / (float)source->height);
        Fullscreen(&s_gl.bloom[level - 1]);
    }
    glDisable(GL_BLEND);

    /* Finish. */
    glUseProgram(s_gl.final.id);
    Set4f(&s_gl.final, "u_pic", (float)scene->width, (float)scene->height,
          1.0f / (float)scene->width, 1.0f / (float)scene->height);
    Set4f(&s_gl.final, "u_proj", kFocal, finish.centre_x, kHorizon, kEye);
    Set4f(&s_gl.final, "u_pp", 0.0f, 0.0f, 0.0f, (float)view->time);
    SetNdc(&s_gl.final, scene, view);
    Bind(0, s_gl.scene.texture);
    Bind(1, s_gl.bloom[0].texture);
    Bind(2, s_gl.texture[2]);
    Set1i(&s_gl.final, "u_scene", 0);
    Set1i(&s_gl.final, "u_bloom", 1);
    Set1i(&s_gl.final, "u_interface", 2);
    Set2f(&s_gl.final, "u_texel", 1.0f / (float)s_gl.width,
          1.0f / (float)s_gl.height);
    Set4f(&s_gl.final, "u_air", finish.air->ray_amount,
          finish.air->mote_amount, scene->brightness,
          finish.dust_where_lit ? 1.0f : 0.0f);
    Set3f(&s_gl.final, "u_ray", finish.air->ray);
    Set3f(&s_gl.final, "u_mote", finish.air->mote);
    Fullscreen(&s_gl.output);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(0);
    glUseProgram(0);
    if (target_framebuffer >= 0) {
        const bool same = s_gl.width == target_view->drawable_width &&
                          s_gl.height == target_view->drawable_height;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, s_gl.output.framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint)target_framebuffer);
        glBlitFramebuffer(0, 0, s_gl.width, s_gl.height, 0, 0,
                          target_view->drawable_width,
                          target_view->drawable_height, GL_COLOR_BUFFER_BIT,
                          same ? GL_NEAREST : GL_LINEAR);
        glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)target_framebuffer);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    const GLenum gl_error = glGetError();
    if (gl_error != GL_NO_ERROR) {
        SetError("OpenGL error 0x%04x while rendering", (unsigned)gl_error);
        s_gl.failed = true;
        s_gl.ready = false;
        return false;
    }
    return true;
}

/* Bottom-up framebuffer rows to a top-down ARGB8888 image. */
static uint8_t *ReadPixels(GLuint framebuffer, int x, int y, int width,
                           int height) {
    if (width <= 0 || height <= 0)
        return NULL;
    const size_t row = (size_t)width * 4;
    uint8_t *raw = (uint8_t *)malloc(row * (size_t)height);
    uint8_t *argb = (uint8_t *)malloc(row * (size_t)height);
    if (!raw || !argb) {
        free(raw);
        free(argb);
        return NULL;
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadPixels(x, y, width, height, GL_BGRA, GL_UNSIGNED_BYTE, raw);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    for (int line = 0; line < height; line++)
        memcpy(argb + (size_t)line * row,
               raw + (size_t)(height - 1 - line) * row, row);
    free(raw);
    return argb;
}

uint8_t *Lufia2Hd2dGlReadback(int *width, int *height) {
    if (!s_gl.ready || !s_gl.width || !s_gl.height)
        return NULL;
    if (width) *width = s_gl.width;
    if (height) *height = s_gl.height;
    return ReadPixels(s_gl.output.framebuffer, 0, 0, s_gl.width,
                      s_gl.height);
}

uint8_t *Lufia2Hd2dGlReadDefault(int x, int y, int width, int height) {
    return ReadPixels(0, x, y, width, height);
}

void Lufia2Hd2dGlBlit(uint32_t texture, int x, int y, int width,
                      int height) {
    if (!EnsureReady())
        return;
    glDisable(GL_BLEND);
    glViewport(x, y, width, height);
    glUseProgram(s_gl.blit.id);
    Bind(0, texture);
    Set1i(&s_gl.blit, "u_source", 0);
    glBindVertexArray(s_gl.empty_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glUseProgram(0);
}
