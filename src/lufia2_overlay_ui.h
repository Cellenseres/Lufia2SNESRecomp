#ifndef LUFIA2_OVERLAY_UI_H
#define LUFIA2_OVERLAY_UI_H

#include <stdbool.h>
#include <stdint.h>

#include "snesrecomp_platform/presenter.h"

struct Lufia2UiPanelAsset;

typedef struct Lufia2OverlayUiFont {
    /* One byte per glyph row, low bits left-to-right. Unsupported input falls
     * back to '?'; lowercase is mapped to uppercase by the renderer. */
    const char *characters;
    const uint8_t *rows;
    int glyph_count;
    int glyph_width;
    int glyph_height;
    int advance;
} Lufia2OverlayUiFont;

typedef struct Lufia2OverlayUiNineSlice {
    const uint32_t *pixels;
    int width;
    int height;
    int left;
    int top;
    int right;
    int bottom;
    /* Repeat the scalable regions instead of stretching them. This mirrors
     * an SNES tilemap and keeps patterned pixel-art edges undistorted. */
    bool tiled;
} Lufia2OverlayUiNineSlice;

typedef enum Lufia2OverlayUiScaleMode {
    /* One UI source pixel occupies exactly one emulated-frame pixel. The
     * presenter then applies the same aspect and window transform to both. */
    LUFIA2_OVERLAY_UI_MATCH_GAME_PIXELS = 0,
    /* Conventional desktop UI sizing based on the host display DPI. */
    LUFIA2_OVERLAY_UI_HOST_DPI,
} Lufia2OverlayUiScaleMode;

/* Game-owned skin for the shared runner overlays. Colours and texture pixels
 * are premultiplied ARGB8888. Another recomp can replace the font, nine-slice,
 * palette and spacing without changing the presenter or guest framebuffer. */
typedef struct Lufia2OverlayUiTheme {
    const Lufia2OverlayUiFont *font;
    const Lufia2OverlayUiNineSlice *panel;
    uint32_t accent;
    uint32_t text;
    uint32_t muted;
    uint32_t shadow;
    uint32_t track;
    int font_scale;
    float scale;
    int margin_dp;
    Lufia2OverlayUiScaleMode scale_mode;
    bool text_shadow;
} Lufia2OverlayUiTheme;

const Lufia2OverlayUiTheme *Lufia2OverlayUiDefaultTheme(void);
/* Takes ownership of asset->pixels on success and clears the caller's asset.
 * Install before resolving the default theme. */
bool Lufia2OverlayUiInstallRomPanel(struct Lufia2UiPanelAsset *asset);
/* The structure is copied; font/texture storage referenced by it must remain
 * alive until another theme is selected or the UI is shut down. */
void Lufia2OverlayUiSetTheme(const Lufia2OverlayUiTheme *theme);

void Lufia2OverlayUiPush(const char *message, int duration_ms);
void Lufia2OverlayUiPushSlotSaved(int slot);
void Lufia2OverlayUiPushSlotLoaded(int slot);
void Lufia2OverlayUiPushSlotEmpty(int slot);
void Lufia2OverlayUiNoteVolume(int percent);

/* Builds either game-pixel or presentation-space layers for the current
 * frame. Returned storage remains owned by this module and valid through
 * present. */
const SnesRecompOverlayFrame *Lufia2OverlayUiBuild(
    SnesRecompPresenter *presenter,
    int frame_width,
    int frame_height,
    bool include_rewind);

void Lufia2OverlayUiShutdown(void);

#endif
