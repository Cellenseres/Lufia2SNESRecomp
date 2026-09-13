#include "lufia2_overlay_ui.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "desktop/sdl_compat.h"
#include "host_paths.h"
#include "snes_osd.h"
#include "snes_rewind.h"

typedef struct UiImage {
    uint32_t *pixels;
    size_t capacity;
    int width, height;
} UiImage;

enum {
    PANEL_TEX_SIZE = 16,
    REWIND_H = 112,
    REWIND_SOURCE_Y = 41,
    REWIND_CELL_W = 74,
    REWIND_CELL_H = 65,
    REWIND_CELL_STEP = 78,
    REWIND_CELLS = 6,
    REWIND_THUMB_GAP = 1,
};

/* Data-driven 5x7 font. A game can replace it through the theme. */
static const char kFontChars[] =
    " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-.:/%+!?()";
static const uint8_t kFontRows[][7] = {
    {0,0,0,0,0,0,0},
    {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},
    {14,17,1,2,4,8,31},{30,1,1,14,1,1,30},
    {2,6,10,18,31,2,2},{31,16,30,1,1,17,14},
    {6,8,16,30,17,17,14},{31,1,2,4,8,8,8},
    {14,17,17,14,17,17,14},{14,17,17,15,1,2,12},
    {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},
    {14,17,16,16,16,17,14},{30,17,17,17,17,17,30},
    {31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
    {14,17,16,23,17,17,15},{17,17,17,31,17,17,17},
    {14,4,4,4,4,4,14},{7,2,2,2,18,18,12},
    {17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
    {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},
    {14,17,17,17,17,17,14},{30,17,17,30,16,16,16},
    {14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
    {15,16,16,14,1,1,30},{31,4,4,4,4,4,4},
    {17,17,17,17,17,17,14},{17,17,17,17,17,10,4},
    {17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
    {17,17,10,4,4,4,4},{31,1,2,4,8,16,31},
    {0,0,0,31,0,0,0},{0,0,0,0,0,12,12},
    {0,12,12,0,12,12,0},{1,2,4,8,16,0,0},
    {17,2,4,8,17,0,0},{0,4,4,31,4,4,0},
    {4,4,4,4,4,0,4},{14,17,1,2,4,0,4},
    {2,4,8,8,8,4,2},{8,4,2,2,2,4,8},
};
static const Lufia2OverlayUiFont kDefaultFont = {
    kFontChars, &kFontRows[0][0],
    (int)(sizeof(kFontRows) / sizeof(kFontRows[0])), 5, 7, 6,
};

static uint32_t s_panel_pixels[PANEL_TEX_SIZE * PANEL_TEX_SIZE];
static const Lufia2OverlayUiNineSlice kDefaultPanel = {
    s_panel_pixels, PANEL_TEX_SIZE, PANEL_TEX_SIZE, 5, 5, 5, 5, false,
};
static const Lufia2OverlayUiTheme kDefaultTheme = {
    &kDefaultFont, &kDefaultPanel,
    0xFFF2C66Du, 0xFFF7EFD9u, 0xFF9EABC5u,
    0xB0000000u, 0xFF293A60u, 1, 1.0f, 8,
    LUFIA2_OVERLAY_UI_MATCH_GAME_PIXELS, false,
};

static Lufia2OverlayUiTheme s_theme;
static Lufia2OverlayUiTheme s_default_theme;
static Lufia2OverlayUiNineSlice s_asset_panel;
static uint32_t *s_asset_panel_pixels;
static bool s_theme_ready, s_panel_ready, s_assets_attempted, s_user_scale_ready;
static UiImage s_rewind, s_fps, s_toast, s_volume;
static SnesRecompOverlayLayer s_layers[4];
static SnesRecompOverlayFrame s_frame;
static char s_toast_text[96];
static uint32_t s_toast_expire, s_volume_expire;
static int s_volume_percent;
static float s_user_scale;

static void init_panel(void) {
    if (s_panel_ready) return;
    for (int y = 0; y < PANEL_TEX_SIZE; y++) {
        for (int x = 0; x < PANEL_TEX_SIZE; x++) {
            int d = x < y ? x : y;
            int opposite = PANEL_TEX_SIZE - 1 - x < PANEL_TEX_SIZE - 1 - y
                ? PANEL_TEX_SIZE - 1 - x : PANEL_TEX_SIZE - 1 - y;
            if (opposite < d) d = opposite;
            uint32_t c = 0xE60C1730u;
            if (d == 0) c = 0xF0060B18u;
            else if (d == 1) c = 0xFFF2C66Du;
            else if (d == 2) c = 0xFF6D82B3u;
            else if (d == 3) c = 0xFF243B6Au;
            else if (d == 4) c = 0xFF101E3Du;
            s_panel_pixels[y * PANEL_TEX_SIZE + x] = c;
        }
    }
    s_panel_ready = true;
}

static uint16_t read_u16_le(const uint8_t *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static bool load_tga_argb(const char *path, uint32_t **out_pixels,
                          int *out_width, int *out_height) {
    uint8_t header[18];
    FILE *file = fopen(path, "rb");
    if (!file) return false;
    bool ok = fread(header, 1, sizeof(header), file) == sizeof(header);
    int width = ok ? (int)read_u16_le(header + 12) : 0;
    int height = ok ? (int)read_u16_le(header + 14) : 0;
    int depth = ok ? header[16] : 0;
    ok = ok && header[1] == 0 && header[2] == 2 &&
         (depth == 24 || depth == 32) && width > 0 && height > 0 &&
         width <= 4096 && height <= 4096 &&
         (size_t)width <= SIZE_MAX / (size_t)height;
    size_t count = ok ? (size_t)width * height : 0;
    ok = ok && count <= SIZE_MAX / sizeof(uint32_t) &&
         fseek(file, header[0], SEEK_CUR) == 0;
    uint32_t *pixels = ok ? (uint32_t *)malloc(count * sizeof(*pixels)) : NULL;
    ok = ok && pixels != NULL;
    bool top_origin = (header[17] & 0x20u) != 0;
    bool right_origin = (header[17] & 0x10u) != 0;
    int bytes_per_pixel = depth / 8;
    for (int file_y = 0; ok && file_y < height; file_y++) {
        for (int file_x = 0; file_x < width; file_x++) {
            uint8_t bgra[4] = {0, 0, 0, 255};
            if (fread(bgra, 1, (size_t)bytes_per_pixel, file) !=
                (size_t)bytes_per_pixel) {
                ok = false;
                break;
            }
            uint32_t a = bgra[3];
            uint32_t r = ((uint32_t)bgra[2] * a + 127u) / 255u;
            uint32_t g = ((uint32_t)bgra[1] * a + 127u) / 255u;
            uint32_t b = ((uint32_t)bgra[0] * a + 127u) / 255u;
            int x = right_origin ? width - 1 - file_x : file_x;
            int y = top_origin ? file_y : height - 1 - file_y;
            pixels[(size_t)y * width + x] =
                (a << 24) | (r << 16) | (g << 8) | b;
        }
    }
    fclose(file);
    if (!ok) {
        free(pixels);
        return false;
    }
    *out_pixels = pixels;
    *out_width = width;
    *out_height = height;
    return true;
}

static bool load_slice_margins(const char *path, int width, int height,
                               int *left, int *top, int *right, int *bottom,
                               bool *tiled) {
    FILE *file = fopen(path, "r");
    if (!file) return false;
    char line[256];
    bool found = false;
    while (fgets(line, sizeof(line), file)) {
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if (!*p || *p == '#') continue;
        char mode[16] = "";
        const int fields = sscanf(
            p, "%d %d %d %d %15s", left, top, right, bottom, mode);
        if (fields >= 4 &&
            (fields == 4 || strcmp(mode, "tile") == 0 ||
             strcmp(mode, "tiled") == 0)) {
            *tiled = strcmp(mode, "tile") == 0 ||
                     strcmp(mode, "tiled") == 0;
            found = true;
            break;
        }
    }
    fclose(file);
    return found && *left >= 0 && *top >= 0 && *right >= 0 && *bottom >= 0 &&
           *left + *right < width && *top + *bottom < height;
}

static void load_default_assets(void) {
    if (s_assets_attempted) return;
    s_assets_attempted = true;
    s_default_theme = kDefaultTheme;

    char tga_path[1024];
    char slice_path[1024];
    const char *tga_leaf = "assets/img/lufia2_menu_panel.tga";
    const char *slice_leaf = "assets/img/lufia2_menu_panel.9slice";
    if (!snesrecomp_exe_dir_path(tga_leaf, tga_path, sizeof(tga_path)))
        snprintf(tga_path, sizeof(tga_path), "%s", tga_leaf);
    if (!snesrecomp_exe_dir_path(slice_leaf, slice_path, sizeof(slice_path)))
        snprintf(slice_path, sizeof(slice_path), "%s", slice_leaf);

    FILE *probe = fopen(tga_path, "rb");
    if (!probe) return;
    fclose(probe);

    int width = 0, height = 0;
    if (!load_tga_argb(tga_path, &s_asset_panel_pixels, &width, &height)) {
        fprintf(stderr, "[Lufia2 UI] Invalid panel TGA '%s'; using built-in skin.\n",
                tga_path);
        return;
    }
    int left = width / 4, right = width / 4;
    int top = height / 4, bottom = height / 4;
    bool tiled = false;
    if (!load_slice_margins(slice_path, width, height,
                            &left, &top, &right, &bottom, &tiled)) {
        fprintf(stderr,
                "[Lufia2 UI] Missing/invalid '%s'; using quarter-size slices.\n",
                slice_path);
    }
    s_asset_panel = (Lufia2OverlayUiNineSlice){
        s_asset_panel_pixels, width, height, left, top, right, bottom, tiled,
    };
    s_default_theme.panel = &s_asset_panel;
    /* Match the captured Lufia menu palette when its panel asset is active. */
    s_default_theme.accent = 0xFF4A3110u;
    s_default_theme.text = 0xFF422921u;
    s_default_theme.muted = 0xFF7B6342u;
    s_default_theme.shadow = 0x00000000u;
    s_default_theme.track = 0xFF7B6342u;
    fprintf(stderr, "[Lufia2 UI] Loaded panel skin '%s' "
            "(%dx%d; %d %d %d %d; %s).\n",
            tga_path, width, height, left, top, right, bottom,
            tiled ? "tiled" : "stretched");
}

const Lufia2OverlayUiTheme *Lufia2OverlayUiDefaultTheme(void) {
    init_panel();
    load_default_assets();
    return &s_default_theme;
}

void Lufia2OverlayUiSetTheme(const Lufia2OverlayUiTheme *theme) {
    init_panel();
    load_default_assets();
    s_theme = theme ? *theme : s_default_theme;
    if (!s_theme.font || !s_theme.font->characters || !s_theme.font->rows ||
        s_theme.font->glyph_count <= 0 || s_theme.font->glyph_width <= 0 ||
        s_theme.font->glyph_width > 8 || s_theme.font->glyph_height <= 0 ||
        s_theme.font->advance < s_theme.font->glyph_width)
        s_theme.font = &kDefaultFont;
    if (!s_theme.panel) s_theme.panel = &kDefaultPanel;
    if (s_theme.font_scale < 1) s_theme.font_scale = 1;
    if (s_theme.scale <= 0.0f) s_theme.scale = 1.0f;
    if (s_theme.margin_dp < 0) s_theme.margin_dp = 0;
    if (s_theme.scale_mode != LUFIA2_OVERLAY_UI_MATCH_GAME_PIXELS &&
        s_theme.scale_mode != LUFIA2_OVERLAY_UI_HOST_DPI) {
        s_theme.scale_mode = LUFIA2_OVERLAY_UI_MATCH_GAME_PIXELS;
    }
    s_theme_ready = true;
}

void Lufia2OverlayUiPush(const char *message, int duration_ms) {
    snprintf(s_toast_text, sizeof(s_toast_text), "%s", message ? message : "");
    s_toast_expire = SDL_GetTicks() +
        (uint32_t)(duration_ms > 0 ? duration_ms : 2000);
}

static void push_slot(int slot, const char *action) {
    char text[48];
    snprintf(text, sizeof(text), "Slot %d %s", slot, action);
    Lufia2OverlayUiPush(text, 0);
}
void Lufia2OverlayUiPushSlotSaved(int slot) { push_slot(slot, "saved"); }
void Lufia2OverlayUiPushSlotLoaded(int slot) { push_slot(slot, "loaded"); }
void Lufia2OverlayUiPushSlotEmpty(int slot) { push_slot(slot, "empty"); }

void Lufia2OverlayUiNoteVolume(int percent) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    s_volume_percent = percent;
    s_volume_expire = SDL_GetTicks() + 1500u;
}

static bool image_resize(UiImage *image, int width, int height) {
    if (!image || width <= 0 || height <= 0 ||
        (size_t)width > SIZE_MAX / (size_t)height) return false;
    size_t count = (size_t)width * height;
    if (count > SIZE_MAX / sizeof(uint32_t)) return false;
    if (count > image->capacity) {
        uint32_t *p = (uint32_t *)realloc(image->pixels, count * sizeof(*p));
        if (!p) return false;
        image->pixels = p;
        image->capacity = count;
    }
    image->width = width;
    image->height = height;
    memset(image->pixels, 0, count * sizeof(uint32_t));
    return true;
}

static void pixel(UiImage *image, int x, int y, uint32_t colour) {
    if ((unsigned)x < (unsigned)image->width &&
        (unsigned)y < (unsigned)image->height)
        image->pixels[(size_t)y * image->width + x] = colour;
}

static void rect(UiImage *image, int x, int y, int w, int h, uint32_t colour) {
    for (int yy = y; yy < y + h; yy++)
        for (int xx = x; xx < x + w; xx++) pixel(image, xx, yy, colour);
}

static void nine_slice(UiImage *image) {
    const Lufia2OverlayUiNineSlice *s = s_theme.panel;
    if (!s || !s->pixels || s->width <= 0 || s->height <= 0 ||
        s->left < 0 || s->right < 0 || s->top < 0 || s->bottom < 0 ||
        s->left + s->right >= s->width || s->top + s->bottom >= s->height ||
        s->left + s->right >= image->width ||
        s->top + s->bottom >= image->height) s = &kDefaultPanel;
    int sw = s->width - s->left - s->right;
    int sh = s->height - s->top - s->bottom;
    int dw = image->width - s->left - s->right;
    int dh = image->height - s->top - s->bottom;
    for (int y = 0; y < image->height; y++) {
        int sy = y < s->top ? y : y >= image->height - s->bottom
            ? s->height - (image->height - y)
            : s->top + (s->tiled
                ? (y - s->top) % sh
                : (y - s->top) * sh / dh);
        for (int x = 0; x < image->width; x++) {
            int sx = x < s->left ? x : x >= image->width - s->right
                ? s->width - (image->width - x)
                : s->left + (s->tiled
                    ? (x - s->left) % sw
                    : (x - s->left) * sw / dw);
            image->pixels[(size_t)y * image->width + x] =
                s->pixels[sy * s->width + sx];
        }
    }
}

/* Status badges need the visual border, but not the two 8-pixel transition
 * tiles used by full-size Lufia windows. Sample the same panel's outer four
 * pixels and tile its clean centre so a one-line badge can be 16 pixels high. */
static void compact_nine_slice(UiImage *image) {
    const Lufia2OverlayUiNineSlice *s = s_theme.panel;
    const int border = 4;
    if (!s || !s->pixels || s->width <= border * 2 ||
        s->height <= border * 2 || s->left < border || s->top < border ||
        s->right < border || s->bottom < border ||
        s->left + s->right >= s->width ||
        s->top + s->bottom >= s->height ||
        image->width <= border * 2 || image->height <= border * 2) {
        nine_slice(image);
        return;
    }
    int sw = s->width - s->left - s->right;
    int sh = s->height - s->top - s->bottom;
    for (int y = 0; y < image->height; y++) {
        int sy = y < border ? y : y >= image->height - border
            ? s->height - (image->height - y)
            : s->top + (y - border) % sh;
        for (int x = 0; x < image->width; x++) {
            int sx = x < border ? x : x >= image->width - border
                ? s->width - (image->width - x)
                : s->left + (x - border) % sw;
            image->pixels[(size_t)y * image->width + x] =
                s->pixels[sy * s->width + sx];
        }
    }
}

static const uint8_t *glyph(char character) {
    const Lufia2OverlayUiFont *font = s_theme.font;
    unsigned char c = (unsigned char)character;
    if (c >= 'a' && c <= 'z') c = (unsigned char)toupper(c);
    const char *found = strchr(font->characters, c);
    if (!found) found = strchr(font->characters, '?');
    ptrdiff_t index = found ? found - font->characters : -1;
    return index >= 0 && index < font->glyph_count
        ? font->rows + index * font->glyph_height : NULL;
}

static int text_width(const char *text) {
    if (!text || !text[0]) return 0;
    const Lufia2OverlayUiFont *f = s_theme.font;
    return ((int)strlen(text) * f->advance -
            (f->advance - f->glyph_width)) * s_theme.font_scale;
}

static int tile_aligned(int value) {
    return (value + 7) & ~7;
}

static void text_pass(UiImage *image, int x, int y, const char *text,
                      uint32_t colour, int offset) {
    const Lufia2OverlayUiFont *f = s_theme.font;
    int scale = s_theme.font_scale;
    for (size_t i = 0; text && text[i]; i++) {
        const uint8_t *g = glyph(text[i]);
        if (!g) continue;
        for (int row = 0; row < f->glyph_height; row++)
            for (int col = 0; col < f->glyph_width; col++)
                if (g[row] & (1u << (f->glyph_width - 1 - col)))
                    rect(image, x + (int)i * f->advance * scale + col * scale + offset,
                         y + row * scale + offset, scale, scale, colour);
    }
}

static void text(UiImage *image, int x, int y, const char *value, uint32_t colour) {
    if (s_theme.text_shadow && (s_theme.shadow >> 24) != 0)
        text_pass(image, x, y, value, s_theme.shadow, 1);
    text_pass(image, x, y, value, colour, 0);
}

static bool text_panel(UiImage *image, const char *value, uint32_t colour) {
    int h = tile_aligned(s_theme.font->glyph_height * s_theme.font_scale + 34);
    int w = tile_aligned(text_width(value) + 36);
    if (!image_resize(image, w, h)) return false;
    nine_slice(image);
    text(image, (w - text_width(value)) / 2,
         (h - s_theme.font->glyph_height * s_theme.font_scale) / 2,
         value, colour);
    return true;
}

static bool compact_text_panel(UiImage *image, const char *value,
                               uint32_t colour) {
    int w = tile_aligned(text_width(value) + 12);
    int h = tile_aligned(
        s_theme.font->glyph_height * s_theme.font_scale + 8);
    if (w < 16) w = 16;
    if (h < 16) h = 16;
    if (!image_resize(image, w, h)) return false;
    compact_nine_slice(image);
    text(image, (w - text_width(value)) / 2,
         (h - s_theme.font->glyph_height * s_theme.font_scale) / 2,
         value, colour);
    return true;
}

static void button(UiImage *image, int x, int y, char label) {
    for (int yy = -5; yy <= 5; yy++)
        for (int xx = -5; xx <= 5; xx++)
            if (xx * xx + yy * yy <= 25) pixel(image, x + xx, y + yy, s_theme.accent);
    char value[2] = {label, 0};
    uint32_t accent = s_theme.accent;
    unsigned luminance = 54u * ((accent >> 16) & 255u) +
                         183u * ((accent >> 8) & 255u) +
                          19u * (accent & 255u);
    uint32_t label_colour = luminance < 128u * 256u
        ? 0xFFF7EFD9u : 0xFF10182Bu;
    text(image, x - text_width(value) / 2, y - 3, value, label_colour);
}

static unsigned bright_border_pixels(const uint32_t *source, int source_w,
                                     int sx, int sy) {
    unsigned score = 0;
    for (int x = 0; x < REWIND_CELL_W; x++) {
        uint32_t top = source[(size_t)sy * source_w + sx + x];
        uint32_t bottom = source[(size_t)(sy + REWIND_CELL_H - 1) *
                                 source_w + sx + x];
        if ((top & 0x00F0F0F0u) == 0x00F0F0F0u) score++;
        if ((bottom & 0x00F0F0F0u) == 0x00F0F0F0u) score++;
    }
    for (int y = 1; y < REWIND_CELL_H - 1; y++) {
        uint32_t left = source[(size_t)(sy + y) * source_w + sx];
        uint32_t right = source[(size_t)(sy + y) * source_w +
                                sx + REWIND_CELL_W - 1];
        if ((left & 0x00F0F0F0u) == 0x00F0F0F0u) score++;
        if ((right & 0x00F0F0F0u) == 0x00F0F0F0u) score++;
    }
    return score;
}

static int selected_source_cell(const uint32_t *source, int source_w) {
    int selected = REWIND_CELLS - 1;
    unsigned best = 0;
    for (int cell = 0; cell < REWIND_CELLS; cell++) {
        unsigned score = bright_border_pixels(
            source, source_w, 11 + cell * REWIND_CELL_STEP,
            REWIND_SOURCE_Y);
        if (score > best) {
            best = score;
            selected = cell;
        }
    }
    return selected;
}

static bool rewind_panel(int max_width) {
    const uint32_t *source = NULL;
    int source_w = 0, source_h = 0;
    if (!snes_rewind_overlay_image(&source, &source_w, &source_h) || !source ||
        source_w < 480 || source_h < REWIND_SOURCE_Y + REWIND_CELL_H)
        return false;

    int visible = (max_width - 32 + REWIND_THUMB_GAP) /
                  (REWIND_CELL_W + REWIND_THUMB_GAP);
    if (visible > REWIND_CELLS) visible = REWIND_CELLS;
    if (visible < 1) return false;
    int content_w = visible * REWIND_CELL_W +
                    (visible - 1) * REWIND_THUMB_GAP;
    int panel_w = tile_aligned(content_w + 32);
    if (panel_w > max_width ||
        !image_resize(&s_rewind, panel_w, REWIND_H)) return false;

    nine_slice(&s_rewind);
    text(&s_rewind, 16, 12, "REWIND", s_theme.text);
    char time_value[32];
    float seconds = snes_rewind_selected_seconds();
    if (seconds < 0.05f) snprintf(time_value, sizeof(time_value), "NOW");
    else {
        int whole = (int)seconds;
        int tenths = (int)((seconds - whole) * 10.0f + 0.5f);
        if (tenths > 9) { whole++; tenths = 0; }
        snprintf(time_value, sizeof(time_value), "-%d.%d S", whole, tenths);
    }
    text(&s_rewind, panel_w - 16 - text_width(time_value), 12,
         time_value, s_theme.text);

    int selected = selected_source_cell(source, source_w);
    int first = selected - visible / 2;
    if (first < 0) first = 0;
    if (first + visible > REWIND_CELLS)
        first = REWIND_CELLS - visible;
    int start_x = (panel_w - content_w) / 2;
    for (int cell = 0; cell < visible; cell++) {
        int source_cell = first + cell;
        int sx = 11 + source_cell * REWIND_CELL_STEP;
        int dx = start_x + cell *
            (REWIND_CELL_W + REWIND_THUMB_GAP);
        for (int y = 0; y < REWIND_CELL_H; y++) {
            memcpy(&s_rewind.pixels[(size_t)(24 + y) * panel_w + dx],
                   &source[(size_t)(REWIND_SOURCE_Y + y) * source_w + sx],
                   REWIND_CELL_W * sizeof(uint32_t));
        }
    }
    rect(&s_rewind, 16, 93, panel_w - 32, 1, s_theme.track);
    button(&s_rewind, 20, 102, 'A');
    text(&s_rewind, 29, 99, "SELECT", s_theme.text);
    button(&s_rewind, 82, 102, 'B');
    text(&s_rewind, 91, 99, "BACK", s_theme.text);
    return true;
}

static bool volume_panel(void) {
    if (!image_resize(&s_volume, 48, 88)) return false;
    nine_slice(&s_volume);
    rect(&s_volume, 21, 17, 6, 43, s_theme.track);
    int level = 43 * s_volume_percent / 100;
    rect(&s_volume, 21, 60 - level, 6, level, s_theme.accent);
    char value[16];
    snprintf(value, sizeof(value), "%d%%", s_volume_percent);
    text(&s_volume, (s_volume.width - text_width(value)) / 2, 68,
         value, s_theme.text);
    return true;
}

static float ui_scale(void) {
    if (!s_user_scale_ready) {
        const char *value = getenv("SNESRECOMP_UI_SCALE");
        char *end = NULL;
        float parsed = value ? strtof(value, &end) : 1.0f;
        if (!value || end == value || parsed < 0.5f || parsed > 3.0f) parsed = 1.0f;
        s_user_scale = parsed;
        s_user_scale_ready = true;
    }
    return s_user_scale;
}

static float snap(float value) {
    if (value < 0.5f) value = 0.5f;
    if (value > 4.0f) value = 4.0f;
    return (float)((int)(value * 2.0f + 0.5f)) * 0.5f;
}

static int scaled(int value, float scale) {
    int result = (int)(value * scale + 0.5f);
    return result > 0 ? result : 1;
}

static float fit(float preferred, UiImage *image, int w, int h) {
    float sx = (float)w / image->width, sy = (float)h / image->height;
    if (preferred > sx) preferred = sx;
    if (preferred > sy) preferred = sy;
    return preferred > 0.0f ? preferred : 0.0f;
}

static void layer(UiImage *image, int x, int y, float scale) {
    SnesRecompOverlayLayer *out = &s_layers[s_frame.layer_count++];
    *out = (SnesRecompOverlayLayer){
        image->pixels, SNESRECOMP_PIXEL_FORMAT_ARGB8888,
        image->width, image->height, image->width * (int)sizeof(uint32_t),
        x, y, scaled(image->width, scale), scaled(image->height, scale), false,
    };
}

const SnesRecompOverlayFrame *Lufia2OverlayUiBuild(
    SnesRecompPresenter *presenter, int frame_width, int frame_height,
    bool include_rewind) {
    int dw = 0, dh = 0;
    if (!presenter || !(snesrecomp_presenter_capabilities(presenter) &
                       SNESRECOMP_PRESENT_CAP_OVERLAYS) ||
        !snesrecomp_presenter_get_drawable_size(presenter, &dw, &dh) ||
        dw <= 0 || dh <= 0) return NULL;
    if (!s_theme_ready) Lufia2OverlayUiSetTheme(NULL);
    const bool match_game = s_theme.scale_mode ==
        LUFIA2_OVERLAY_UI_MATCH_GAME_PIXELS &&
        frame_width > 0 && frame_height > 0;
    if (match_game) {
        dw = frame_width;
        dh = frame_height;
    }
    s_frame = (SnesRecompOverlayFrame){
        s_layers, 0,
        match_game ? SNESRECOMP_OVERLAY_SPACE_FRAME
                   : SNESRECOMP_OVERLAY_SPACE_PRESENTATION,
        dw, dh,
    };
    float dpi = match_game ? 1.0f :
        snesrecomp_presenter_display_scale(presenter);
    float preferred = match_game
        ? (float)((int)(s_theme.scale * ui_scale() + 0.5f))
        : snap(dpi * s_theme.scale * ui_scale());
    if (preferred < 1.0f) preferred = 1.0f;
    int margin = scaled(s_theme.margin_dp, dpi);
    int aw = dw - 2 * margin, ah = dh - 2 * margin;
    int bottom = margin;
    int fps_right_margin = margin;

    if (include_rewind && rewind_panel(dw)) {
        /* The rewind core has already rasterised each framed preview cell.
         * Never resample those pixels a second time in the Lufia skin. */
        float scale = match_game ? 1.0f : fit(preferred, &s_rewind, aw, ah);
        if (scale > 0.0f) {
            int w = scaled(s_rewind.width, scale), h = scaled(s_rewind.height, scale);
            int x = (dw - w) / 2;
            layer(&s_rewind, x, dh - margin - h, scale);
            fps_right_margin = dw - x - w;
            bottom += h + margin / 2;
        }
    }
    uint32_t now = SDL_GetTicks();
    if (s_toast_text[0] && (int32_t)(s_toast_expire - now) > 0 &&
        text_panel(&s_toast, s_toast_text, s_theme.text)) {
        float scale = fit(preferred, &s_toast, aw, dh - bottom - margin);
        if (scale > 0.0f) {
            int w = scaled(s_toast.width, scale), h = scaled(s_toast.height, scale);
            layer(&s_toast, (dw - w) / 2, dh - bottom - h, scale);
        }
    } else if (s_toast_text[0] && (int32_t)(s_toast_expire - now) <= 0) {
        s_toast_text[0] = 0;
    }
    char fps[48] = "";
    if (snes_osd_fps_visible())
        snprintf(fps, sizeof(fps), "%d FPS%s", (int)(snes_osd_fps() + 0.5f),
                 snes_osd_turbo() ? "  TURBO" : "");
    else if (snes_osd_turbo()) snprintf(fps, sizeof(fps), "TURBO");
    if (fps[0] && compact_text_panel(&s_fps, fps, s_theme.text)) {
        float scale = fit(preferred, &s_fps, aw, ah);
        if (scale > 0.0f) {
            int w = scaled(s_fps.width, scale);
            layer(&s_fps, dw - fps_right_margin - w, margin, scale);
        }
    }
    if ((int32_t)(s_volume_expire - now) > 0 && volume_panel()) {
        float scale = fit(preferred, &s_volume, aw, ah);
        if (scale > 0.0f) {
            int w = scaled(s_volume.width, scale), h = scaled(s_volume.height, scale);
            layer(&s_volume, dw - margin - w, (dh - h) / 2, scale);
        }
    }
    return s_frame.layer_count ? &s_frame : NULL;
}

void Lufia2OverlayUiShutdown(void) {
    free(s_rewind.pixels); free(s_fps.pixels);
    free(s_toast.pixels); free(s_volume.pixels);
    s_rewind = (UiImage){0}; s_fps = (UiImage){0};
    s_toast = (UiImage){0}; s_volume = (UiImage){0};
    free(s_asset_panel_pixels);
    s_asset_panel_pixels = NULL;
    s_asset_panel = (Lufia2OverlayUiNineSlice){0};
    s_frame = (SnesRecompOverlayFrame){0};
    memset(s_layers, 0, sizeof(s_layers));
    s_toast_text[0] = '\0';
    s_toast_expire = 0;
    s_volume_expire = 0;
    s_volume_percent = 0;
    s_user_scale = 0.0f;
    s_user_scale_ready = false;
    s_assets_attempted = false;
    s_theme_ready = false;
}
