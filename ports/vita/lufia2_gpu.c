#include "lufia2_runtime.h"
#include <string.h>
#include "snes/ppu.h"
#include "snes/ws_shadow.h"
#include "snesrecomp_platform/gpu_bg.h"
#include "snesrecomp_platform/snes_bg_upload.h"
#include "snesrecomp_platform/snes_ppu_mode7.h"
#include "snesrecomp_platform/snes_ppu_obj.h"
#include "snesrecomp_platform/snes_ppu_semantic_gpu.h"
#include "snesrecomp_platform/snes_ppu_trace.h"
#include "snesrecomp_platform/host_boot.h"
extern Ppu *g_ppu;
enum { LUFIA2_GPU_LINES = 224 };
#if SNESRECOMP_PLATFORM_HAS_GPU_BG

/* Inputs that determine a cached widescreen tilemap. */
typedef struct Lufia2TilemapKey {
    uint16_t map_word;
    uint16_t h_scroll;
    uint16_t v_scroll;
    uint32_t vram_generation;
    uint64_t source_id;
    uint8_t margin_left;
    uint8_t margin_right;
    bool wide;
    bool tall;
    bool repeat;
    bool valid;
} Lufia2TilemapKey;

enum {
    LUFIA2_DIRECT_CACHE_LAYERS = 2,
    LUFIA2_SEMANTIC_CACHE_LAYERS = 3,
};

static Lufia2TilemapKey
    s_direct_tilemap_key[LUFIA2_DIRECT_CACHE_LAYERS];
static uint32_t s_direct_tilemap_serial[LUFIA2_DIRECT_CACHE_LAYERS];
static Lufia2TilemapKey
    s_semantic_tilemap_key[LUFIA2_SEMANTIC_CACHE_LAYERS];
static uint32_t s_semantic_tilemap_serial[LUFIA2_SEMANTIC_CACHE_LAYERS];

static bool Lufia2TilemapKeysEqual(const Lufia2TilemapKey *a,
                                    const Lufia2TilemapKey *b) {
    return a->map_word == b->map_word &&
           a->h_scroll == b->h_scroll &&
           a->v_scroll == b->v_scroll &&
           a->vram_generation == b->vram_generation &&
           a->source_id == b->source_id &&
           a->margin_left == b->margin_left &&
           a->margin_right == b->margin_right &&
           a->wide == b->wide && a->tall == b->tall &&
           a->repeat == b->repeat && a->valid == b->valid;
}

/* Builds the 64x64 wrapped tilemap sampled by GXM. Native cells come from
 * VRAM; widened margin cells use the game's world-tile shadow. */
static void Lufia2BuildGpuTilemap(uint8_t *dst_rgba,
                                  const Lufia2GpuPpuInput *input,
                                  const SnesPpuBgState *state, int bg,
                                  unsigned margin_left,
                                  unsigned margin_right,
                                  bool repeat) {
    const SnesPpuFrameCapture *cap = input->capture;
    const uint16_t *vram = cap->vram;
    const unsigned map_word = state->tilemap_word_addr;
    const bool wide = state->wide;
    const bool tall = state->tall;
    const int hscroll = (int)state->h_scroll;
    const int vscroll = (int)state->v_scroll;
    /* Repeat layers wrap native cells instead of using the world shadow. */
    const bool shadow = !repeat && input->margin_tile_lookup != NULL;

    const int x_first = (hscroll - (int)margin_left) >> 3;
    const int x_last = (hscroll + 256 + (int)margin_right + 7) >> 3;
    const int y_first = vscroll >> 3;
    const int y_last = (vscroll + LUFIA2_GPU_LINES + 7) >> 3;

    for (int ty = y_first; ty <= y_last; ty++) {
        for (int tx = x_first; tx <= x_last; tx++) {
            /* Where this tile's left edge falls on the SNES's own screen. */
            const int screen_x = tx * 8 - hscroll;
            uint16_t entry = 0;
            bool have = false;

            /* Negative world coordinates are outside the shadow. */
            if (shadow && tx >= 0 && ty >= 0 &&
                (screen_x < 0 || screen_x + 8 > 256)) {
                have = input->margin_tile_lookup(
                    input->margin_tile_opaque, (unsigned)bg,
                    (uint32_t)tx, (uint32_t)ty, &entry);
            }
            if (!have) {
                if (!repeat && (screen_x < 0 || screen_x + 8 > 256)) {
                    /* Missing margin cells match the software path's blank. */
                    entry = 0;
                } else {
                    const unsigned mx = (unsigned)tx & (wide ? 63u : 31u);
                    const unsigned my = (unsigned)ty & (tall ? 63u : 31u);
                    const unsigned screen =
                        (my >> 5) * (wide ? 2u : 1u) + (mx >> 5);
                    const unsigned within = ((my & 31u) << 5) | (mx & 31u);
                    entry =
                        vram[(map_word + screen * 0x400u + within) & 0x7FFFu];
                }
            }

            snesrecomp_bg_pack_entry(
                dst_rgba + (((size_t)((unsigned)ty & 63u) * 64u +
                             ((unsigned)tx & 63u)) * 4u),
                entry);
        }
    }
}

/* Last GPU admission or failure result, exposed through diagnostics. */
static const char *s_gpu_bail = "never ran";

/* Whether the semantic compositor may claim production frames.
 *
 * This is a performance decision, not a correctness one, and it is off by
 * default because the first hardware measurement said so: a semantic
 * production frame cost 20-36 ms against the software renderer's ~10 ms, so
 * accepting those frames turned 53 fps into 25-34. The path is exact -- the
 * 21-case hardware matrix says so -- it is simply not fast enough yet, and
 * shipping an exact renderer that halves the frame rate is not shipping an
 * improvement.
 *
 * Diagnostic replay is unaffected: it renders through the readback path,
 * which is where exactness is measured and where throughput does not matter.
 * Dropping the marker file turns it on for a measurement run. */
static bool s_semantic_production;
static bool s_mode7_production;

void Lufia2SetSemanticProductionEnabled(bool enabled) {
    s_semantic_production = enabled;
}

void Lufia2SetMode7ProductionEnabled(bool enabled) {
    s_mode7_production = enabled;
}

/* What the sprite work actually cost, for the frame the diagnostics print.
 * TASK-05 moves sprite pixels to the GPU but deliberately keeps evaluation on
 * the CPU, so "how much CPU did that leave behind" is a number the next
 * hardware log has to state rather than a claim. */
static unsigned s_obj_eval_us;
static unsigned s_obj_slivers;
static bool s_obj_range_over;
static bool s_obj_time_over;

/* Renders an accepted capture without reading live PPU state. */
static bool Lufia2LiveMarginTileLookup(void *opaque, unsigned bg,
                                       uint32_t world_x, uint32_t world_y,
                                       uint16_t *entry) {
    (void)opaque;
    return WsShadowLayerActive((int)bg) &&
           WsShadowLookupWorldTile((int)bg, world_x, world_y, entry);
}

static bool Lufia2CaptureNeedsSemantic(const SnesPpuFrameCapture *cap) {
    const SnesPpuRasterBand *first = NULL;
    for (unsigned i = 0; i < cap->band_count; i++) {
        const SnesPpuRasterBand *b = &cap->bands[i];
        const unsigned on = b->main_enable | b->sub_enable;
        if (b->forced_blank)
            continue;
        if (!first)
            first = b;
        if ((on & 0x14u) || (b->sub_enable & 7u) ||
            ((b->main_window_enable | b->sub_window_enable) & 7u) ||
            (b->cgadsub & 0x27u) || (b->cgwsel & 0xf2u) ||
            (first && (b->main_enable != first->main_enable ||
                       b->sub_enable != first->sub_enable ||
                       b->bg3_priority != first->bg3_priority)))
            return true;
    }
    return false;
}

/* Exact basic Mode 7 path. The portable layer has already rejected every
 * state the first GPU implementation cannot reproduce, and compiles the
 * signed affine coordinates without Vita-specific policy. This function only
 * supplies captured resources and the same authoritative OBJ evaluation used
 * by the established semantic renderer. */
static bool Lufia2GpuPpuRenderMode7(const Lufia2GpuPpuInput *input) {
    static SnesRecompMode7Line lines[LUFIA2_GPU_LINES];
    static SnesRecompObjSliver obj_slivers[SNESRECOMP_OBJ_MAX_SLIVERS];
    static uint8_t obj_line_priority[LUFIA2_GPU_LINES];
    const SnesPpuFrameCapture *cap = input->capture;
    SnesRecompGpuSemanticFrame frame;
    SnesRecompObjFrame obj;
    bool wants_obj = false;

    memset(&frame, 0, sizeof frame);
    memset(&obj, 0, sizeof obj);
    if (!snesrecomp_ppu_mode7_compile_lines(cap, lines,
                                             LUFIA2_GPU_LINES)) {
        s_gpu_bail = "mode7 affine compile failed";
        return false;
    }
    frame.capture = cap;
    frame.mode7_lines = lines;
    frame.mode7_line_count = cap->visible_height;
    frame.present_mask = input->backdrop_only ? 0u : 1u;
    frame.readback_pixels = input->readback_pixels;
    frame.readback_pitch_bytes = input->readback_pitch_bytes;

    for (unsigned i = 0; i < cap->band_count; i++)
        if (!cap->bands[i].forced_blank &&
            ((cap->bands[i].main_enable |
              cap->bands[i].sub_enable) & 0x10u))
            wants_obj = true;

    obj.slivers = obj_slivers;
    obj.capacity = SNESRECOMP_OBJ_MAX_SLIVERS;
    obj.line_priority = obj_line_priority;
    obj.line_capacity = LUFIA2_GPU_LINES;
    s_obj_eval_us = 0;
    s_obj_slivers = 0;
    s_obj_range_over = false;
    s_obj_time_over = false;
    if (wants_obj && !input->backdrop_only) {
        const uint64_t obj_t0 = Lufia2HostNowUs();
        if (!snesrecomp_ppu_obj_evaluate(cap, &obj)) {
            s_gpu_bail = "mode7 OBJ evaluation failed";
            return false;
        }
        s_obj_eval_us = (unsigned)(Lufia2HostNowUs() - obj_t0);
        s_obj_slivers = obj.count;
        s_obj_range_over = obj.range_over;
        s_obj_time_over = obj.time_over;
        frame.obj = &obj;
        if (g_ppu && !input->readback_pixels) {
            g_ppu->rangeOver = obj.range_over;
            g_ppu->timeOver = obj.time_over;
        }
    }

    s_gpu_bail = "mode7 semantic";
    if (!snesrecomp_gpu_bg_render_semantic(&frame)) {
        s_gpu_bail = "mode7 render failed";
        return false;
    }
    return true;
}

static bool Lufia2GpuPpuRenderSemantic(const Lufia2GpuPpuInput *input) {
    enum { kBgCount = 3 };
    static uint16_t scroll_x[kBgCount][LUFIA2_GPU_LINES];
    static uint16_t scroll_y[kBgCount][LUFIA2_GPU_LINES];
    static uint8_t tilemap[kBgCount][64 * 64 * 4];
    static SnesRecompObjSliver obj_slivers[SNESRECOMP_OBJ_MAX_SLIVERS];
    static uint8_t obj_line_priority[LUFIA2_GPU_LINES];
    SnesRecompObjFrame obj;
    SnesRecompGpuSemanticFrame frame;
    const SnesPpuFrameCapture *cap = input->capture;
    bool wants_obj = false;

    memset(&frame, 0, sizeof frame);
    frame.capture = cap;
    frame.readback_pixels = input->readback_pixels;
    frame.readback_pitch_bytes = input->readback_pitch_bytes;
    if (!snesrecomp_ppu_expand_semantic_scroll(
            cap, scroll_x[0], scroll_y[0], scroll_x[1], scroll_y[1],
            scroll_x[2], scroll_y[2], LUFIA2_GPU_LINES)) {
        s_gpu_bail = "semantic scroll expansion failed";
        return false;
    }

    for (unsigned i = 0; i < cap->band_count; i++)
        if (!cap->bands[i].forced_blank &&
            ((cap->bands[i].main_enable | cap->bands[i].sub_enable) & 0x10u))
            wants_obj = true;

    memset(&obj, 0, sizeof obj);
    obj.slivers = obj_slivers;
    obj.capacity = SNESRECOMP_OBJ_MAX_SLIVERS;
    obj.line_priority = obj_line_priority;
    obj.line_capacity = LUFIA2_GPU_LINES;
    s_obj_eval_us = 0;
    s_obj_slivers = 0;
    s_obj_range_over = false;
    s_obj_time_over = false;
    if (wants_obj && !input->backdrop_only) {
        const uint64_t obj_t0 = Lufia2HostNowUs();
        if (!snesrecomp_ppu_obj_evaluate(cap, &obj)) {
            s_gpu_bail = "obj evaluation failed";
            return false;
        }
        s_obj_eval_us = (unsigned)(Lufia2HostNowUs() - obj_t0);
        s_obj_slivers = obj.count;
        s_obj_range_over = obj.range_over;
        s_obj_time_over = obj.time_over;
        frame.obj = &obj;
        /* The guest can read the sprite overflow flags back through $213E, and
         * an accepted frame never runs the software renderer that would
         * otherwise have set them. A diagnostic replay has no guest, so it
         * leaves the live PPU alone. */
        if (g_ppu && !input->readback_pixels) {
            g_ppu->rangeOver = obj.range_over;
            g_ppu->timeOver = obj.time_over;
        }
    }

    snesrecomp_gpu_bg_upload(cap->vram, cap->cgram);
    for (unsigned bg = 0; bg < kBgCount; bg++) {
        const SnesPpuBgState *state = NULL;
        for (unsigned i = 0; i < cap->band_count; i++) {
            const SnesPpuRasterBand *b = &cap->bands[i];
            if (!b->forced_blank &&
                ((b->main_enable | b->sub_enable) & (1u << bg))) {
                state = &b->bg[bg];
                break;
            }
        }
        if (!state || input->backdrop_only)
            continue;
        {
            SnesRecompGpuBgLayer *layer = &frame.layer[bg];
            const bool custom_tilemap =
                (state->margin_left || state->margin_right) &&
                !state->margin_repeats;
            const Lufia2TilemapKey want = {
                .map_word = state->tilemap_word_addr,
                .h_scroll = state->h_scroll,
                .v_scroll = state->v_scroll,
                .vram_generation = snesrecomp_gpu_bg_vram_generation(),
                .source_id = input->tilemap_source_id,
                .margin_left = state->margin_left,
                .margin_right = state->margin_right,
                .wide = state->wide,
                .tall = state->tall,
                .repeat = state->margin_repeats,
                .valid = true,
            };
            if (custom_tilemap && !Lufia2TilemapKeysEqual(
                    &s_semantic_tilemap_key[bg], &want)) {
                Lufia2BuildGpuTilemap(tilemap[bg], input, state, (int)bg,
                                      state->margin_left, state->margin_right,
                                      state->margin_repeats);
                s_semantic_tilemap_key[bg] = want;
                s_semantic_tilemap_serial[bg]++;
            }
            layer->tilemap_word_addr = state->tilemap_word_addr;
            layer->tile_word_addr = state->char_word_addr;
            layer->wide = state->wide;
            layer->tall = state->tall;
            layer->bpp = state->bpp;
            layer->palette_base = 0;
            layer->tilemap_slot = bg;
            layer->margin_left = state->margin_left;
            layer->margin_right = state->margin_right;
            layer->scroll_x = scroll_x[bg];
            layer->scroll_y = scroll_y[bg];
            layer->tilemap_rgba = custom_tilemap ? tilemap[bg] : NULL;
            layer->tilemap_serial = custom_tilemap
                ? (0x80000000u | s_semantic_tilemap_serial[bg]) : 0u;
            frame.present_mask |= 1u << bg;
        }
    }

    s_gpu_bail = "semantic";
    if (!snesrecomp_gpu_bg_render_semantic(&frame)) {
        s_gpu_bail = "semantic render failed";
        return false;
    }
    return true;
}

bool Lufia2GpuPpuRenderInput(const Lufia2GpuPpuInput *input) {
    /* Fast path for frames that need only BG1/BG2 and raster brightness.
     * BG3, windows, subscreen and colour math use the semantic path above. */
    enum { kBgCount = 2, kMaxDraws = 7 };
    static uint16_t scroll_x[kBgCount][LUFIA2_GPU_LINES];
    static uint16_t scroll_y[kBgCount][LUFIA2_GPU_LINES];
    static uint8_t brightness[LUFIA2_GPU_LINES];
    static uint8_t forced_blank[LUFIA2_GPU_LINES];
    static uint8_t tilemap[kBgCount][64 * 64 * 4];
    SnesRecompGpuBgLayer layer[kBgCount];
    bool present[kBgCount] = { false, false };
    SnesRecompGpuBgLayer draw[kMaxDraws];
    unsigned ndraw = 0;
    unsigned enabled = 0;
    const SnesPpuFrameCapture *cap;
    const SnesPpuRasterBand *band = NULL;
    bool all_blank = true;
    unsigned width, lines, extra_left;

    if (!input || !input->capture) {
        s_gpu_bail = "invalid capture";
        return false;
    }
    cap = input->capture;
    width = cap->canvas_width;
    lines = cap->visible_height;
    extra_left = cap->canvas_extra;
    if (!width || !lines || lines > LUFIA2_GPU_LINES) {
        s_gpu_bail = "invalid geometry";
        return false;
    }
    /* Readback always exercises an accepted Mode 7 frame. Production remains
     * a runtime opt-in until its first hardware comparison has passed, so the
     * same binary can fall back immediately without touching guest state. */
    if ((input->readback_pixels || s_mode7_production) &&
        snesrecomp_ppu_mode7_backend_supports(
            cap, snesrecomp_gpu_mode7_available()) == SNES_PPU_SUPPORTED)
        return Lufia2GpuPpuRenderMode7(input);
    {
        const SnesPpuUnsupported support =
            snesrecomp_ppu_phase1_backend_supports(
                cap, snesrecomp_gpu_bg_available());
        if (support != SNES_PPU_SUPPORTED) {
            s_gpu_bail = snes_ppu_unsupported_text(support);
            return false;
        }
    }

    for (unsigned i = 0; i < cap->band_count; i++) {
        const SnesPpuRasterBand *raster = &cap->bands[i];
        if (!raster->forced_blank && !band)
            band = raster;
        if (!raster->forced_blank)
            all_blank = false;
    }
    if (!all_blank && Lufia2CaptureNeedsSemantic(cap)) {
        if (!input->readback_pixels && !s_semantic_production) {
            s_gpu_bail = "semantic disabled in production";
            return false;
        }
        return Lufia2GpuPpuRenderSemantic(input);
    }
    if (!snesrecomp_ppu_phase1_expand_lines(
            cap, scroll_x[0], scroll_y[0], scroll_x[1], scroll_y[1],
            brightness, forced_blank, LUFIA2_GPU_LINES)) {
        s_gpu_bail = "raster expansion failed";
        return false;
    }

    /* Whole-frame blank retains the cheap exact path and needs neither VRAM
     * unpack nor palette work. Mixed blank continues through the raster mask. */
    if (all_blank) {
        const bool began = input->readback_pixels
            ? snesrecomp_gpu_bg_begin_readback(width, lines, extra_left)
            : snesrecomp_gpu_bg_begin(width, lines, extra_left);
        if (!began) {
            s_gpu_bail = "begin failed (blank)";
            return false;
        }
        s_gpu_bail = "forced blank";
        snesrecomp_gpu_bg_clear_black();
        return input->readback_pixels
            ? snesrecomp_gpu_bg_end_readback(
                  input->readback_pixels, input->readback_pitch_bytes)
            : snesrecomp_gpu_bg_end();
    }
    if (!band) {
        s_gpu_bail = "no visible resource band";
        return false;
    }

    if (band->bg_mode != 1) {
        s_gpu_bail = "not mode 1";
        return false;
    }
    /* Everything below -- the atlas layout, the tilemap indexing, the shader's
     * floor(vWorld / 8) -- assumes 8x8 characters. A layer switched to 16x16
     * would render as a quarter of itself, so hand the whole frame back rather
     * than draw something plausible and wrong. */
    if (band->bg[0].big_tiles || band->bg[1].big_tiles) {
        s_gpu_bail = "big tiles";
        return false;
    }

    /* Upload first so the generation used by the tilemap key describes this
     * capture rather than the preceding frame. The call remains outside the
     * scene, where its cache maintenance is allowed to synchronize. */
    snesrecomp_gpu_bg_upload(cap->vram, cap->cgram);

    for (int bg = 0; bg < kBgCount; bg++) {
        const SnesPpuBgState *state = &band->bg[bg];
        const unsigned on_screen =
            (unsigned)(band->main_enable | band->sub_enable);
        unsigned margin_left, margin_right;
        bool repeat;

        /* A layer on neither screen contributes nothing, and drawing it would
         * paint over the one below. */
        if (!(on_screen & (1u << bg)))
            continue;

        /* The capture has already applied PpuWidescreenLayerExtra. A repeat
         * layer records its real margins plus margin_repeats, because software
         * fills those pixels by cyclically copying the native scanline. */
        repeat = state->margin_repeats;

        /* How far this layer is allowed past the native window. The rule is
         * the renderer's own -- widen mask, clamp/mirror/repeat bits, clamp
         * and repeat bands -- so ask it rather than restate it. A centered
         * layout has already set extraLeftCur/extraRightCur to 0, and a menu's
         * widen mask already excludes the window and text layers, so obeying
         * this is what keeps 4:3 scenes at 4:3 and menu boxes out of the
         * margins. */
        margin_left = state->margin_left;
        margin_right = state->margin_right;

        memset(&layer[bg], 0, sizeof layer[bg]);
        layer[bg].tilemap_word_addr = state->tilemap_word_addr;
        layer[bg].tile_word_addr = state->char_word_addr;
        layer[bg].wide = state->wide;
        layer[bg].tall = state->tall;
        layer[bg].bpp = state->bpp;
        layer[bg].palette_base = 0;
        layer[bg].tilemap_slot = (unsigned)bg;
        layer[bg].margin_left = margin_left;
        layer[bg].margin_right = margin_right;
        /* The tilemap texture depends on the map in VRAM, the layer's
         * descriptor, the margins and -- because texel index is world tile
         * modulo 64 -- the scroll. It does not depend on anything else, so a
         * static scene rebuilds nothing. Before this, a still camera over
         * unchanged VRAM rebuilt 4096 entries and copied 16 KB every frame
         * because the caller-supplied buffer bypassed the existing key. */
        {
            const Lufia2TilemapKey want = {
                .map_word = state->tilemap_word_addr,
                .h_scroll = state->h_scroll,
                .v_scroll = state->v_scroll,
                .vram_generation = snesrecomp_gpu_bg_vram_generation(),
                .source_id = input->tilemap_source_id,
                .margin_left = (uint8_t)margin_left,
                .margin_right = (uint8_t)margin_right,
                .wide = layer[bg].wide,
                .tall = layer[bg].tall,
                .repeat = repeat,
                .valid = true,
            };
            if (!Lufia2TilemapKeysEqual(&s_direct_tilemap_key[bg], &want)) {
                Lufia2BuildGpuTilemap(tilemap[bg], input, state, bg,
                                      margin_left, margin_right, repeat);
                s_direct_tilemap_key[bg] = want;
                s_direct_tilemap_serial[bg]++;
            }
            /* The pointer is handed over every frame; the serial decides
             * whether this rotating set still needs a copy of it. */
            layer[bg].tilemap_rgba = tilemap[bg];
            layer[bg].tilemap_serial = s_direct_tilemap_serial[bg];
        }
        layer[bg].scroll_x = scroll_x[bg];
        layer[bg].scroll_y = scroll_y[bg];
        present[bg] = true;
        enabled++;
    }

    (void)enabled; /* backdrop-only visible frames are valid and exact */

    /* Back to front, in the order the hardware's priority ladder puts them.
     * A layer appears twice because its two priorities sit at different
     * heights, and BG3's high priority is either the very top of the frame or
     * near the very bottom depending on one register bit -- which is how a
     * menu gets its text above the window frames. */
    {
        static const struct { unsigned bg, prio; } kOrder[] = {
            { 1u, 0u },   /* BG2 low  */
            { 0u, 0u },   /* BG1 low  */
            { 1u, 1u },   /* BG2 high */
            { 0u, 1u },   /* BG1 high */
        };
        for (unsigned i = 0; i < sizeof kOrder / sizeof kOrder[0]; i++) {
            const unsigned bg = kOrder[i].bg;
            if (!present[bg])
                continue;
            draw[ndraw] = layer[bg];
            draw[ndraw].priority = kOrder[i].prio;
            ndraw++;
        }
    }

    /* Before the scene: this can fence, and a fence inside a scene waits on
     * commands that have not been submitted yet. */
    const bool began = input->readback_pixels
        ? snesrecomp_gpu_bg_begin_readback(width, lines, extra_left)
        : snesrecomp_gpu_bg_begin(width, lines, extra_left);
    if (!began) {
        s_gpu_bail = "begin failed";
        return false;
    }
    s_gpu_bail = "drawn";
    snesrecomp_gpu_bg_clear_raster(brightness, forced_blank);

    if (!input->backdrop_only) {
        for (unsigned i = 0; i < ndraw; i++)
            snesrecomp_gpu_bg_draw(&draw[i]);
    }

    return input->readback_pixels
        ? snesrecomp_gpu_bg_end_readback(
              input->readback_pixels, input->readback_pitch_bytes)
        : snesrecomp_gpu_bg_end();
}

bool Lufia2GpuPpuRender(const SnesPpuFrameCapture *cap, bool backdrop_only) {
    const Lufia2GpuPpuInput input = {
        .capture = cap,
        .margin_tile_lookup = Lufia2LiveMarginTileLookup,
        .margin_tile_opaque = NULL,
        .tilemap_source_id = 0,
        .backdrop_only = backdrop_only,
        .readback_pixels = NULL,
        .readback_pitch_bytes = 0,
    };
    return Lufia2GpuPpuRenderInput(&input);
}

void Lufia2GpuPpuResetDiagnosticState(void) {
    static const uint16_t zero_vram[
        SNES_PPU_TRACE_VRAM_BYTES / sizeof(uint16_t)];
    static const uint16_t zero_cgram[
        SNES_PPU_TRACE_CGRAM_BYTES / sizeof(uint16_t)];

    /* Change serials so every rotating GPU set reloads custom tilemaps. */
    for (unsigned i = 0; i < LUFIA2_DIRECT_CACHE_LAYERS; i++) {
        s_direct_tilemap_key[i].valid = false;
        if (++s_direct_tilemap_serial[i] == 0)
            s_direct_tilemap_serial[i] = 1;
    }
    for (unsigned i = 0; i < LUFIA2_SEMANTIC_CACHE_LAYERS; i++) {
        s_semantic_tilemap_key[i].valid = false;
        if (++s_semantic_tilemap_serial[i] == 0)
            s_semantic_tilemap_serial[i] = 1;
    }

    /* Emptying the content cache marks rows removed by the previous trace. */
    (void)snesrecomp_gpu_bg_upload(zero_vram, zero_cgram);
}

#else  /* no native GPU backend on this platform */

/* The portable build has no GPU backend, so every frame is the software
 * renderer's. Saying so through the same predicate every target uses keeps the
 * main loop identical on both platforms. */
bool Lufia2GpuPpuRenderInput(const Lufia2GpuPpuInput *input) {
    (void)input;
    return false;
}

bool Lufia2GpuPpuRender(const SnesPpuFrameCapture *cap, bool backdrop_only) {
    (void)cap; (void)backdrop_only;
    return false;
}

void Lufia2GpuPpuResetDiagnosticState(void) {
}

/* Nothing to switch on without a GPU backend, but the main loop still reads
 * the marker file and calls this on every target. */
void Lufia2SetSemanticProductionEnabled(bool enabled) {
    (void)enabled;
}

void Lufia2SetMode7ProductionEnabled(bool enabled) {
    (void)enabled;
}

#endif


/* Serial CleanMain CPU drawing remains authoritative for guest-visible state.
 * The GPU consumes a captured frame only; never advance HDMA/IRQs a second time.
 */
static const char *s_frame_status = "not attempted";
const char *Lufia2GxmFrameStatus(void) { return s_frame_status; }

bool Lufia2GxmCanDefer(unsigned width, unsigned extra) {
#if SNESRECOMP_PLATFORM_HAS_GPU_BG
    SnesPpuFrameCapture capture;
    SnesPpuUnsupported support;
    if (!snesrecomp_host_gpu_backgrounds_enabled()) {
        s_frame_status = "preflight: GPU disabled";
        return false;
    }
    if (!Lufia2CapturePpuFrame(&capture, width, extra)) {
        s_frame_status = "preflight: capture rejected";
        return false;
    }
    support = snesrecomp_ppu_phase1_backend_supports(
        &capture, snesrecomp_gpu_bg_available());
    if (support == SNES_PPU_SUPPORTED) {
        if (Lufia2CaptureNeedsSemantic(&capture) &&
            !s_semantic_production) {
            s_frame_status = "preflight: semantic disabled";
            return false;
        }
        s_frame_status = "preflight: phase1 eligible";
        return true;
    }
    if (!s_mode7_production) {
        s_frame_status = "preflight: Mode 7 disabled";
        return false;
    }
    support = snesrecomp_ppu_mode7_backend_supports(
        &capture, snesrecomp_gpu_mode7_available());
    if (support != SNES_PPU_SUPPORTED) {
        /* This text is stable storage owned by the portable platform. Keeping
         * the exact admission reason visible avoids another hardware build
         * merely to distinguish shader availability from capture semantics. */
        s_frame_status = snes_ppu_unsupported_text(support);
        return false;
    }
    s_frame_status = "preflight: Mode 7 eligible";
    return true;
#else
    (void)width; (void)extra;
    s_frame_status = "preflight: GPU backend unavailable";
    return false;
#endif
}

bool Lufia2TryGxmFrame(unsigned width, unsigned extra) {
    SnesPpuFrameCapture capture;
    if (!snesrecomp_host_gpu_backgrounds_enabled()) {
        s_frame_status = "disabled by marker";
        return false;
    }
    if (Lufia2PpuRasterMemoryFlags()) {
        s_frame_status = "HDMA/IRQ CPU fallback";
        return false;
    }
    if (!Lufia2CapturePpuFrame(&capture, width, extra)) {
        s_frame_status = "capture rejected";
        return false;
    }
    bool range_over = g_ppu->rangeOver, time_over = g_ppu->timeOver;
    bool drawn = Lufia2GpuPpuRender(&capture, false);
#if SNESRECOMP_PLATFORM_HAS_GPU_BG
    s_frame_status = drawn ? "drawn" : s_gpu_bail;
#else
    s_frame_status = "GPU backend unavailable";
#endif
    g_ppu->rangeOver = range_over;
    g_ppu->timeOver = time_over;
    return drawn;
}
