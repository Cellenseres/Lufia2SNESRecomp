#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    LUFIA2_VIDEO_HANDOFF_MAX_RASTER_LINES = 224,
    /* Detection reads register traffic, which a standing HDMA split can
       imitate forever. The budget is far longer than any transition and
       falls back to the live frame, never to black. */
    LUFIA2_VIDEO_HANDOFF_MAX_EFFECT_FRAMES = 180,
};

/* What the frame's layout allows the handoff to do with it: present wide
   margins, follow the camera while doing so, or neither. */
typedef enum Lufia2VideoHandoffScene {
    LUFIA2_VIDEO_HANDOFF_SCENE_OWN_FRAME = 0,
    LUFIA2_VIDEO_HANDOFF_SCENE_MAP_LOADING,
    LUFIA2_VIDEO_HANDOFF_SCENE_WIDE,
    LUFIA2_VIDEO_HANDOFF_SCENE_WIDE_MAP,
} Lufia2VideoHandoffScene;

typedef struct Lufia2VideoHandoff {
    Lufia2VideoHandoffScene current_scene;
    bool previous_wide_scene;
    bool saved_scroll_tracks;
    bool frame_valid;
    bool holding;
    bool current_raster_effect;
    uint8_t current_brightness;
    uint8_t saved_brightness;
    uint8_t held_brightness;
    uint8_t current_mosaic_size;
    uint16_t current_scroll_x;
    uint16_t current_scroll_y;
    uint16_t saved_scroll_x;
    uint16_t saved_scroll_y;
    unsigned raster_effect_frames;
    size_t current_raster_lines;
    size_t saved_raster_lines;
    uint16_t current_line_scroll_x[LUFIA2_VIDEO_HANDOFF_MAX_RASTER_LINES];
    uint16_t current_line_scroll_y[LUFIA2_VIDEO_HANDOFF_MAX_RASTER_LINES];
    uint16_t saved_line_scroll_x[LUFIA2_VIDEO_HANDOFF_MAX_RASTER_LINES];
    uint16_t saved_line_scroll_y[LUFIA2_VIDEO_HANDOFF_MAX_RASTER_LINES];
    uint8_t current_line_mosaic_size[
        LUFIA2_VIDEO_HANDOFF_MAX_RASTER_LINES];
} Lufia2VideoHandoff;

void Lufia2VideoHandoffReset(Lufia2VideoHandoff *handoff);
void Lufia2VideoHandoffObserve(
    Lufia2VideoHandoff *handoff,
    Lufia2VideoHandoffScene scene,
    uint8_t brightness,
    uint8_t mosaic_size,
    uint16_t scroll_x,
    uint16_t scroll_y);
void Lufia2VideoHandoffObserveRaster(
    Lufia2VideoHandoff *handoff,
    const uint16_t *scroll_x,
    const uint16_t *scroll_y,
    const uint8_t *mosaic_size,
    size_t line_count);
bool Lufia2VideoHandoffIsHolding(const Lufia2VideoHandoff *handoff);
/* margin_width is one extended edge; scroll_source_overlap is the extra
   native columns the map hides while it streams. */
void Lufia2VideoHandoffApply(
    Lufia2VideoHandoff *handoff,
    uint8_t *frame,
    uint8_t *saved_frame,
    size_t width,
    size_t height,
    size_t margin_width,
    size_t scroll_source_overlap);
