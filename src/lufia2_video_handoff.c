#include "lufia2_video_handoff.h"

#include <string.h>

typedef struct HandoffBlit {
    uint8_t *frame;
    const uint8_t *saved_frame;
    size_t width;
    size_t height;
    size_t pitch;
    size_t margin_width;
    unsigned brightness;
    unsigned saved_brightness;
    bool extend_to_edge;
} HandoffBlit;

typedef struct HandoffRow {
    unsigned mosaic_size;
    int scroll_delta_x;
    int scroll_delta_y;
} HandoffRow;

static bool SceneIsWide(Lufia2VideoHandoffScene scene) {
    return scene == LUFIA2_VIDEO_HANDOFF_SCENE_WIDE ||
           scene == LUFIA2_VIDEO_HANDOFF_SCENE_WIDE_EFFECTS ||
           scene == LUFIA2_VIDEO_HANDOFF_SCENE_WIDE_MAP;
}

static bool SceneTracksScroll(Lufia2VideoHandoffScene scene) {
    return scene == LUFIA2_VIDEO_HANDOFF_SCENE_WIDE_MAP;
}

static bool SceneSupportsFullFrameEffects(Lufia2VideoHandoffScene scene) {
    return scene == LUFIA2_VIDEO_HANDOFF_SCENE_WIDE_EFFECTS;
}

static uint8_t ClampBrightness(uint8_t brightness) {
    return brightness > 15u ? 15u : brightness;
}

static uint8_t ClampMosaicSize(uint8_t size) {
    if (size < 1u)
        return 1u;
    return size > 16u ? 16u : size;
}

static uint32_t ScalePixel(
    uint32_t pixel, unsigned numerator, unsigned denominator) {
    if (!denominator || numerator >= denominator)
        return pixel;

    const uint32_t alpha = pixel & 0xff000000u;
    const uint32_t red = ((pixel >> 16) & 0xffu) * numerator / denominator;
    const uint32_t green = ((pixel >> 8) & 0xffu) * numerator / denominator;
    const uint32_t blue = (pixel & 0xffu) * numerator / denominator;
    return alpha | (red << 16) | (green << 8) | blue;
}

static uint32_t LoadPixel(const uint8_t *address) {
    uint32_t pixel;
    memcpy(&pixel, address, sizeof(pixel));
    return pixel;
}

static void StorePixel(uint8_t *address, uint32_t pixel) {
    memcpy(address, &pixel, sizeof(pixel));
}

/* Scroll registers wrap within the 1024-pixel tilemap. */
static int ScrollDelta(uint16_t current, uint16_t saved) {
    enum {
        PPU_SCROLL_PERIOD = 1024,
        PPU_SCROLL_MASK = PPU_SCROLL_PERIOD - 1,
    };
    int delta = ((int)current - (int)saved) & PPU_SCROLL_MASK;
    if (delta >= PPU_SCROLL_PERIOD / 2)
        delta -= PPU_SCROLL_PERIOD;
    return delta;
}

static int MosaicOrigin(int coordinate, unsigned size) {
    if (size <= 1u)
        return coordinate;
    if (coordinate >= 0)
        return coordinate - coordinate % (int)size;
    const int distance = -coordinate;
    return coordinate - ((int)size - distance % (int)size) % (int)size;
}

static int ClampToSpan(int value, int span) {
    if (value < 0)
        return 0;
    return value >= span ? span - 1 : value;
}

static void BlitSavedPixel(
    const HandoffBlit *blit, const HandoffRow *row, size_t x, size_t y) {
    const int base_x = (int)x + row->scroll_delta_x;
    const int base_y = (int)y + row->scroll_delta_y;
    if (!blit->extend_to_edge &&
        (base_x < 0 || base_y < 0 || base_x >= (int)blit->width ||
         base_y >= (int)blit->height)) {
        return;
    }

    const int saved_x = ClampToSpan(
        MosaicOrigin((int)x - (int)blit->margin_width, row->mosaic_size) +
            (int)blit->margin_width + row->scroll_delta_x,
        (int)blit->width);
    const int saved_y = ClampToSpan(
        MosaicOrigin((int)y, row->mosaic_size) + row->scroll_delta_y,
        (int)blit->height);

    StorePixel(
        blit->frame + y * blit->pitch + x * sizeof(uint32_t),
        ScalePixel(
            LoadPixel(
                blit->saved_frame + (size_t)saved_y * blit->pitch +
                (size_t)saved_x * sizeof(uint32_t)),
            blit->brightness,
            blit->saved_brightness));
}

static HandoffRow ResolveRow(const Lufia2VideoHandoff *handoff, size_t y) {
    HandoffRow row = {handoff->current_mosaic_size, 0, 0};

    if (handoff->saved_scroll_tracks) {
        row.scroll_delta_x =
            ScrollDelta(handoff->current_scroll_x, handoff->saved_scroll_x);
        row.scroll_delta_y =
            ScrollDelta(handoff->current_scroll_y, handoff->saved_scroll_y);
        if (y < handoff->current_raster_lines &&
            y < handoff->saved_raster_lines) {
            row.scroll_delta_x = ScrollDelta(
                handoff->current_line_scroll_x[y],
                handoff->saved_line_scroll_x[y]);
            row.scroll_delta_y = ScrollDelta(
                handoff->current_line_scroll_y[y],
                handoff->saved_line_scroll_y[y]);
        }
    }
    if (y < handoff->current_raster_lines)
        row.mosaic_size = handoff->current_line_mosaic_size[y];
    return row;
}

static bool SpendEffectBudget(Lufia2VideoHandoff *handoff) {
    if (!handoff->current_raster_effect) {
        handoff->raster_effect_frames = 0;
        return false;
    }
    if (handoff->raster_effect_frames <
            LUFIA2_VIDEO_HANDOFF_MAX_EFFECT_FRAMES) {
        handoff->raster_effect_frames++;
    }
    return handoff->raster_effect_frames <
           LUFIA2_VIDEO_HANDOFF_MAX_EFFECT_FRAMES;
}

static void SaveScrollSource(
    Lufia2VideoHandoff *handoff,
    const uint8_t *frame,
    size_t height,
    size_t pitch,
    uint8_t *saved_frame) {
    memcpy(saved_frame, frame, height * pitch);
    handoff->frame_valid = true;
    handoff->saved_brightness = handoff->current_brightness;
    handoff->saved_scroll_tracks = SceneTracksScroll(handoff->current_scene);
    handoff->saved_full_frame_effects =
        SceneSupportsFullFrameEffects(handoff->current_scene);
    handoff->saved_scroll_x = handoff->current_scroll_x;
    handoff->saved_scroll_y = handoff->current_scroll_y;
    handoff->saved_raster_lines = handoff->current_raster_lines;
    if (handoff->saved_raster_lines) {
        const size_t bytes = handoff->saved_raster_lines * sizeof(uint16_t);
        memcpy(
            handoff->saved_line_scroll_x,
            handoff->current_line_scroll_x, bytes);
        memcpy(
            handoff->saved_line_scroll_y,
            handoff->current_line_scroll_y, bytes);
    }
}

void Lufia2VideoHandoffReset(Lufia2VideoHandoff *handoff) {
    if (!handoff)
        return;
    memset(handoff, 0, sizeof *handoff);
    handoff->current_mosaic_size = 1;
}

void Lufia2VideoHandoffObserve(
    Lufia2VideoHandoff *handoff,
    Lufia2VideoHandoffScene scene,
    uint8_t brightness,
    uint8_t mosaic_size,
    uint16_t scroll_x,
    uint16_t scroll_y) {
    if (!handoff)
        return;

    const bool wide_scene = SceneIsWide(scene);
    const bool loading_scene =
        scene == LUFIA2_VIDEO_HANDOFF_SCENE_MAP_LOADING;

    handoff->current_scene = scene;
    handoff->current_brightness = ClampBrightness(brightness);
    handoff->current_mosaic_size = ClampMosaicSize(mosaic_size);
    handoff->current_scroll_x = scroll_x;
    handoff->current_scroll_y = scroll_y;
    handoff->current_raster_lines = 0;
    handoff->current_raster_effect = handoff->current_mosaic_size > 1u;

    if (wide_scene) {
        handoff->holding = false;
    } else if (handoff->holding) {
        if (!handoff->current_brightness) {
            handoff->holding = false;
        } else if (handoff->current_brightness < handoff->held_brightness) {
            handoff->held_brightness = handoff->current_brightness;
        } else if (handoff->current_brightness > handoff->held_brightness &&
                   !loading_scene) {
            /* A bridge follows a scene out. A 4:3 frame getting brighter is
               the next scene arriving, and it owns its margins. Strict,
               because a fade may spend two frames on one level. */
            handoff->holding = false;
        }
    } else if (handoff->frame_valid && handoff->previous_wide_scene &&
               handoff->saved_brightness && handoff->current_brightness) {
        handoff->holding = true;
        handoff->held_brightness = handoff->current_brightness;
        if (handoff->held_brightness > handoff->saved_brightness)
            handoff->held_brightness = handoff->saved_brightness;
    }
    /* The layout survives a forced blank because the caller relabels it, the
       picture does not: what follows a black frame is a new scene and must
       not arm a bridge from the old one. */
    handoff->previous_wide_scene = wide_scene && handoff->current_brightness;
}

void Lufia2VideoHandoffObserveRaster(
    Lufia2VideoHandoff *handoff,
    const uint16_t *scroll_x,
    const uint16_t *scroll_y,
    const uint8_t *mosaic_size,
    size_t line_count) {
    if (!handoff || !scroll_x || !scroll_y || !mosaic_size)
        return;
    if (line_count > LUFIA2_VIDEO_HANDOFF_MAX_RASTER_LINES)
        line_count = LUFIA2_VIDEO_HANDOFF_MAX_RASTER_LINES;

    handoff->current_raster_lines = line_count;
    for (size_t y = 0; y < line_count; y++) {
        const uint8_t size = ClampMosaicSize(mosaic_size[y]);
        handoff->current_line_scroll_x[y] = scroll_x[y];
        handoff->current_line_scroll_y[y] = scroll_y[y];
        handoff->current_line_mosaic_size[y] = size;
        if (size > handoff->current_mosaic_size)
            handoff->current_mosaic_size = size;
        if (size > 1u ||
            (handoff->current_scene !=
                 LUFIA2_VIDEO_HANDOFF_SCENE_WIDE_EFFECTS &&
             y && (scroll_x[y] != scroll_x[0] ||
                   scroll_y[y] != scroll_y[0]))) {
            handoff->current_raster_effect = true;
        }
    }
}

bool Lufia2VideoHandoffIsHolding(const Lufia2VideoHandoff *handoff) {
    return handoff && handoff->holding;
}

void Lufia2VideoHandoffApply(
    Lufia2VideoHandoff *handoff,
    uint8_t *frame,
    uint8_t *saved_frame,
    size_t width,
    size_t height,
    size_t margin_width,
    size_t scroll_source_overlap) {
    if (!handoff || !frame || !saved_frame || !width || !height ||
        !margin_width || width > SIZE_MAX / sizeof(uint32_t)) {
        return;
    }
    const size_t pitch = width * sizeof(uint32_t);
    if (height > SIZE_MAX / pitch)
        return;

    const bool raster_effect = SpendEffectBudget(handoff);
    const bool effect_ready =
        raster_effect && handoff->frame_valid && handoff->saved_brightness &&
        (handoff->saved_scroll_tracks || handoff->saved_full_frame_effects);
    const bool wide_scene = SceneIsWide(handoff->current_scene);

    if (!handoff->holding && !(wide_scene && effect_ready)) {
        /* The center is only a scroll source; it is never presented stale. */
        if (wide_scene && handoff->current_brightness &&
            !handoff->current_raster_effect) {
            SaveScrollSource(handoff, frame, height, pitch, saved_frame);
        }
        return;
    }

    size_t edge_width = margin_width;
    if (handoff->saved_scroll_tracks) {
        if (scroll_source_overlap > SIZE_MAX - edge_width)
            return;
        edge_width += scroll_source_overlap;
    }
    if (edge_width > width / 2u)
        edge_width = width / 2u;

    const HandoffBlit blit = {
        frame,
        saved_frame,
        width,
        height,
        pitch,
        margin_width,
        handoff->holding
            ? handoff->held_brightness
            : handoff->current_brightness,
        handoff->saved_brightness,
        raster_effect,
    };
    for (size_t y = 0; y < height; y++) {
        const HandoffRow row = ResolveRow(handoff, y);
        if (effect_ready) {
            for (size_t x = 0; x < width; x++)
                BlitSavedPixel(&blit, &row, x, y);
            continue;
        }
        for (size_t x = 0; x < edge_width; x++) {
            BlitSavedPixel(&blit, &row, x, y);
            BlitSavedPixel(&blit, &row, width - 1u - x, y);
        }
    }
}
