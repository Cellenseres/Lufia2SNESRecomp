#include "lufia2_intro_tree_job.h"
#include <stdlib.h>
#include <string.h>

enum { CACHE_COLUMNS = 48, CACHE_WIDTH = CACHE_COLUMNS * 8, TILE_WORDS = 8 };
typedef struct CachedColumn {
    int32_t column;
    uint16_t entries[LUFIA2_INTRO_TREE_ROWS];
    uint16_t words[LUFIA2_INTRO_TREE_ROWS][TILE_WORDS];
    bool valid, known;
} CachedColumn;
struct Lufia2IntroTreeJob {
    uint32_t *pixels;
    CachedColumn columns[CACHE_COLUMNS];
    uint32_t palette[32];
    uint16_t max_height, height, scroll_y;
    bool valid;
    Lufia2IntroTreeJobStats stats;
};
static unsigned ColumnSlot(int32_t column) {
    return (unsigned)(((column % CACHE_COLUMNS) + CACHE_COLUMNS) % CACHE_COLUMNS);
}
Lufia2IntroTreeJob *Lufia2IntroTreeJobCreate(uint16_t height) {
    if (!height || height > SNES_PPU_MAX_BANDS) return NULL;
    Lufia2IntroTreeJob *c = calloc(1, sizeof *c);
    if (!c) return NULL;
    c->pixels = calloc((size_t)CACHE_WIDTH * height, sizeof *c->pixels);
    if (!c->pixels) { free(c); return NULL; }
    c->max_height = height;
    return c;
}
void Lufia2IntroTreeJobDestroy(Lufia2IntroTreeJob *c) {
    if (c) { free(c->pixels); free(c); }
}
void Lufia2IntroTreeJobGetStats(const Lufia2IntroTreeJob *c, Lufia2IntroTreeJobStats *out) {
    if (out) *out = c ? c->stats : (Lufia2IntroTreeJobStats){0};
}
SnesRecompPpuResult Lufia2IntroTreeJobPrepare(Lufia2IntroTreeJob *c,
    const SnesRecompPpuFrame *frame, const Lufia2IntroTreeSnapshot *s,
    SnesRecompPpuJobImage images[LUFIA2_INTRO_TREE_JOB_IMAGES],
    SnesRecompPpuOverlay rectangles[LUFIA2_INTRO_TREE_JOB_RECTS], uint16_t *count) {
    const SnesPpuFrameCapture *cap = snesrecomp_ppu_frame_capture(frame);
    if (!c || !cap || !s || !images || !rectangles || !count || s->fine_x >= 8u ||
        s->width != cap->canvas_width || s->column_count != (s->fine_x + s->width + 7u) / 8u ||
        s->first_column > INT32_MAX - s->column_count) return SNESRECOMP_PPU_INVALID;
    if (s->column_count > CACHE_COLUMNS || cap->visible_height > c->max_height)
        return SNESRECOMP_PPU_RESOURCE_LIMIT;
    for (unsigned i = 0; i < s->column_count; ++i)
        if (s->known[i] > 1u) return SNESRECOMP_PPU_INVALID;
    for (unsigned i = 0; i < 32; ++i)
        if (s->palette[i] >> 24 != 255u) return SNESRECOMP_PPU_INVALID;
    const unsigned height = cap->visible_height;
    const bool changed = !c->valid || c->height != height || c->scroll_y != s->scroll_y ||
        memcmp(c->palette, s->palette, sizeof c->palette) != 0;
    /* Offscreen slots must not later reuse pixels from an older palette,
     * vertical scroll or image height when they reenter the viewport. */
    if (changed) for (unsigned slot = 0; slot < CACHE_COLUMNS; ++slot) c->columns[slot].valid = false;
    Lufia2IntroTreeJobStats stats = {0};
    for (unsigned i = 0; i < s->column_count; ++i) {
        const int32_t column = s->first_column + (int32_t)i;
        const unsigned slot = ColumnSlot(column);
        CachedColumn *old = c->columns + slot;
        uint16_t words[LUFIA2_INTRO_TREE_ROWS][TILE_WORDS] = {{0}};
        if (s->known[i]) for (unsigned row = 0; row < LUFIA2_INTRO_TREE_ROWS; ++row)
            for (unsigned y = 0; y < TILE_WORDS; ++y)
                words[row][y] = cap->vram[(s->chars + (s->entries[i][row] & 1023u) * 8u + y) & 32767u];
        const bool same = !changed && old->valid && old->column == column &&
            old->known == (s->known[i] != 0) && (!s->known[i] ||
            (!memcmp(old->entries, s->entries[i], sizeof old->entries) &&
             !memcmp(old->words, words, sizeof words)));
        if (same) { ++stats.reused_columns; continue; }
        for (unsigned y = 0; y < height; ++y) {
            const unsigned line = y + s->scroll_y;
            const unsigned row = (line >> 3) % LUFIA2_INTRO_TREE_ROWS;
            const uint16_t entry = s->entries[i][row];
            const unsigned sy = entry & 0x8000u ? 7u - (line & 7u) : line & 7u;
            const uint16_t planes = words[row][sy];
            for (unsigned x = 0; x < 8; ++x) {
                const unsigned bit = entry & 0x4000u ? x : 7u - x;
                const unsigned value = (planes >> bit & 1u) | ((planes >> (8u + bit) & 1u) << 1);
                c->pixels[(size_t)y * CACHE_WIDTH + slot * 8u + x] =
                    s->known[i] && value ? s->palette[((entry >> 10) & 7u) * 4u + value] : 0u;
            }
        }
        old->valid = true; old->column = column; old->known = s->known[i] != 0;
        memcpy(old->entries, s->entries[i], sizeof old->entries);
        memcpy(old->words, words, sizeof words);
        ++stats.decoded_columns; stats.decoded_pixels += height * 8u;
    }
    memcpy(c->palette, s->palette, sizeof c->palette);
    c->height = (uint16_t)height; c->scroll_y = s->scroll_y; c->valid = true; c->stats = stats;
    for (unsigned i = 0; i < LUFIA2_INTRO_TREE_JOB_IMAGES; ++i)
        images[i] = (SnesRecompPpuJobImage){c->pixels + i * 64u, CACHE_WIDTH, 64, (uint16_t)height};
    unsigned source_x = ColumnSlot(s->first_column) * 8u + s->fine_x;
    *count = 0;
    for (unsigned x = 0; x < s->width;) {
        const unsigned fine = source_x % 64u;
        const unsigned width = s->width - x < 64u - fine ? s->width - x : 64u - fine;
        rectangles[(*count)++] = (SnesRecompPpuOverlay){.x = (uint16_t)x,
            .source_x = (uint16_t)fine, .image = (uint8_t)(source_x / 64u),
            .width = (uint16_t)width, .height = (uint16_t)height,
            .blend = SNESRECOMP_PPU_OVERLAY_STRAIGHT};
        x += width; source_x = (source_x + width) % CACHE_WIDTH;
    }
    return SNESRECOMP_PPU_OK;
}
