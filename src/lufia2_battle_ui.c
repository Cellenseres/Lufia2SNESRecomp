#include "lufia2_battle_ui.h"
#include "lufia2_ui_layout_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#undef HIBYTE
#include <windows.h>
#endif

static Lufia2BattleUiLayout s_layout;
static uint8_t s_layout_bytes[LUFIA2_BATTLE_UI_LAYOUT_BYTES];
static char s_layout_path[1024], s_preview_path[1024];
static bool s_loaded, s_active, s_capturing, s_ready;
static unsigned s_width, s_height, s_top, s_preview_frames;
static uint64_t s_rows;
static uint32_t s_cards[LUFIA2_BATTLE_UI_CARDS][LUFIA2_BATTLE_UI_CARD_HEIGHT][LUFIA2_BATTLE_UI_CARD_WIDTH];
static uint32_t s_original[LUFIA2_UI_HEIGHT][LUFIA2_UI_NATIVE_WIDTH];
static uint8_t s_original_mask[LUFIA2_UI_HEIGHT][LUFIA2_UI_NATIVE_WIDTH];

bool Lufia2BattleUiDecode(const uint8_t *data, size_t size,
                         Lufia2BattleUiLayout *layout) {
    if (!data || !layout || size != LUFIA2_BATTLE_UI_LAYOUT_BYTES ||
        memcmp(data, "L2UI", 4) || Lufia2UiRead16(data + 4) != 1 ||
        Lufia2UiRead16(data + 6) != size || Lufia2UiRead16(data + 8) != LUFIA2_UI_CANVAS_WIDTH ||
        Lufia2UiRead16(data + 10) != LUFIA2_UI_HEIGHT || Lufia2UiRead16(data + 12) != LUFIA2_BATTLE_UI_CARD_HEIGHT ||
        Lufia2UiRead16(data + 14) != LUFIA2_BATTLE_UI_CARDS || Lufia2UiRead32(data + 20) ||
        Lufia2UiRead32(data + 16) != Lufia2UiChecksum(data + LUFIA2_UI_LAYOUT_HEADER_BYTES,
                                                  size - LUFIA2_UI_LAYOUT_HEADER_BYTES))
        return false;
    Lufia2BattleUiLayout candidate = {0};
    for (unsigned card = 0; card < LUFIA2_BATTLE_UI_CARDS; ++card) {
        const uint8_t *record = data + LUFIA2_UI_LAYOUT_HEADER_BYTES + card * 12;
        const unsigned anchor = Lufia2UiRead16(record);
        const int offset = (int)(int16_t)Lufia2UiRead16(record + 2);
        if (anchor > 1000 || offset < -1024 || offset > 1024 ||
            Lufia2UiRead16(record + 4) != LUFIA2_BATTLE_UI_CARD_WIDTH || Lufia2UiRead16(record + 6) ||
            Lufia2UiRead16(record + 8) != card || Lufia2UiRead16(record + 10))
            return false;
        candidate.cards[card].anchor = (uint16_t)anchor;
        candidate.cards[card].offset = (int16_t)offset;
        const int x = Lufia2BattleUiCardX(&candidate, card, LUFIA2_UI_CANVAS_WIDTH);
        if (x < 0 || x > LUFIA2_UI_CANVAS_WIDTH - LUFIA2_BATTLE_UI_CARD_WIDTH) return false;
    }
    *layout = candidate;
    return true;
}

int Lufia2BattleUiCardX(const Lufia2BattleUiLayout *layout,
                       unsigned card, unsigned width) {
    if (!layout || card >= LUFIA2_BATTLE_UI_CARDS || width < LUFIA2_BATTLE_UI_CARD_WIDTH || width > 4096) return -1;
    return (int)((layout->cards[card].anchor * (width - LUFIA2_BATTLE_UI_CARD_WIDTH) + 500u) / 1000u) +
        layout->cards[card].offset;
}

void Lufia2BattleUiDraw(const Lufia2BattleUiLayout *layout,
                       const uint32_t *cards, uint32_t *pixels,
                       unsigned width, unsigned height, unsigned top) {
    if (!layout || !cards || !pixels || width > 4096 || height > 4096 || top >= height)
        return;
    for (unsigned card = 0; card < LUFIA2_BATTLE_UI_CARDS; ++card) {
        const int left = Lufia2BattleUiCardX(layout, card, width);
        for (unsigned y = 0; y < LUFIA2_BATTLE_UI_CARD_HEIGHT && top + y < height; ++y)
            for (unsigned x = 0; x < LUFIA2_BATTLE_UI_CARD_WIDTH; ++x) {
                const uint32_t pixel = cards[(card * LUFIA2_BATTLE_UI_CARD_HEIGHT + y) *
                    LUFIA2_BATTLE_UI_CARD_WIDTH + x];
                if ((pixel >> 24) && left + (int)x >= 0 &&
                    left + (int)x < (int)width)
                    pixels[(top + y) * width + (unsigned)(left + (int)x)] = pixel;
            }
    }
}

void Lufia2BattleUiReset(void) {
    s_active = s_capturing = s_ready = false;
    s_rows = 0;
}

void Lufia2BattleUiInit(const char *layout_path, const char *preview_path) {
    snprintf(s_layout_path, sizeof s_layout_path, "%s", layout_path ? layout_path : "");
    snprintf(s_preview_path, sizeof s_preview_path, "%s", preview_path ? preview_path : "");
    s_loaded = false;
    s_preview_frames = 0;
    memset(s_layout_bytes, 0, sizeof s_layout_bytes);
    Lufia2BattleUiReset();
}

static void ReloadLayout(void) {
    if (!s_layout_path[0]) return;
    FILE *file = fopen(s_layout_path, "rb");
    if (!file) return;
    uint8_t data[LUFIA2_BATTLE_UI_LAYOUT_BYTES + 1];
    const size_t size = fread(data, 1, sizeof data, file);
    const bool failed = ferror(file) != 0;
    fclose(file);
    if (failed || (size == sizeof s_layout_bytes &&
        s_loaded && !memcmp(data, s_layout_bytes, size))) return;
    Lufia2BattleUiLayout candidate;
    if (!Lufia2BattleUiDecode(data, size, &candidate)) return;
    s_layout = candidate;
    memcpy(s_layout_bytes, data, sizeof s_layout_bytes);
    s_loaded = true;
}

void Lufia2BattleUiBegin(bool enabled, unsigned width, unsigned height) {
    Lufia2BattleUiReset();
    if (!enabled || width < LUFIA2_UI_NATIVE_WIDTH || width > 4096 || height != LUFIA2_UI_HEIGHT) return;
    ReloadLayout();
    if (!s_loaded) return;
    for (unsigned card = 0; card < LUFIA2_BATTLE_UI_CARDS; ++card) {
        const int x = Lufia2BattleUiCardX(&s_layout, card, width);
        if (x < 0 || x + LUFIA2_BATTLE_UI_CARD_WIDTH > (int)width) return;
    }
    s_width = width;
    s_height = height;
    s_active = s_capturing = true;
    memset(s_cards, 0, sizeof s_cards);
    memset(s_original_mask, 0, sizeof s_original_mask);
}

bool Lufia2BattleUiActive(void) { return s_active; }
bool Lufia2BattleUiCapturing(void) { return s_active && s_capturing; }

void Lufia2BattleUiRecord(unsigned row, unsigned line,
                         const uint32_t cards[LUFIA2_BATTLE_UI_CARDS][LUFIA2_BATTLE_UI_CARD_WIDTH],
                         const uint32_t original[256],
                         const uint8_t original_mask[256]) {
    if (!Lufia2BattleUiCapturing() || row >= LUFIA2_BATTLE_UI_CARD_HEIGHT || line >= LUFIA2_UI_HEIGHT || line < row)
        return;
    if (!s_rows) s_top = line - row;
    if (line - row != s_top) return;
    for (unsigned card = 0; card < LUFIA2_BATTLE_UI_CARDS; ++card)
        memcpy(s_cards[card][row], cards[card], sizeof s_cards[card][row]);
    memcpy(s_original[line], original, sizeof s_original[line]);
    memcpy(s_original_mask[line], original_mask, sizeof s_original_mask[line]);
    s_rows |= UINT64_C(1) << row;
}

void Lufia2BattleUiFinish(uint8_t *pixels, size_t pitch) {
    if (!s_capturing) return;
    s_capturing = false;
    s_ready = s_rows == ((UINT64_C(1) << LUFIA2_BATTLE_UI_CARD_HEIGHT) - 1u);
    if (!s_ready) {
        if (pixels && pitch >= s_width * 4u)
            for (unsigned y = 0; y < s_height; ++y)
                for (unsigned x = 0; x < LUFIA2_UI_NATIVE_WIDTH; ++x)
                    if (s_original_mask[y][x])
                        memcpy(pixels + y * pitch + ((s_width - LUFIA2_UI_NATIVE_WIDTH) / 2 + x) * 4u,
                               &s_original[y][x], 4);
        s_active = false;
    }
}

static void ExportPreview(const uint8_t *scene) {
    if (!s_preview_path[0] || ++s_preview_frames % 30 != 1) return;
    char temporary[1060];
    snprintf(temporary, sizeof temporary, "%s.tmp", s_preview_path);
    FILE *file = fopen(temporary, "wb");
    if (!file) return;
    uint8_t header[32] = {'L','2','U','P'};
    Lufia2UiWrite32(header + 4, 1);
    Lufia2UiWrite32(header + 8, s_width);
    Lufia2UiWrite32(header + 12, s_height);
    Lufia2UiWrite32(header + 16, s_top);
    Lufia2UiWrite32(header + 20, LUFIA2_BATTLE_UI_CARD_WIDTH);
    Lufia2UiWrite32(header + 24, LUFIA2_BATTLE_UI_CARD_HEIGHT);
    Lufia2UiWrite32(header + 28, LUFIA2_BATTLE_UI_CARDS);
    bool ok = fwrite(header, 1, sizeof header, file) == sizeof header;
    const size_t scene_bytes = (size_t)s_width * s_height * 4;
    ok = ok && fwrite(scene, 1, scene_bytes, file) == scene_bytes;
    ok = ok && fwrite(s_cards, 1, sizeof s_cards, file) == sizeof s_cards;
    ok = fclose(file) == 0 && ok;
    if (ok) {
#ifdef _WIN32
        if (!MoveFileExA(temporary, s_preview_path,
                         MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            remove(temporary);
#else
        if (rename(temporary, s_preview_path)) remove(temporary);
#endif
    } else remove(temporary);
}

void Lufia2BattleUiCompose(uint8_t *pixels, unsigned width,
                          unsigned height, bool authoritative) {
    if (!pixels || !s_ready || !s_active || width != s_width || height != s_height)
        return;
    if (authoritative) ExportPreview(pixels);
    Lufia2BattleUiDraw(&s_layout, &s_cards[0][0][0],
                       (uint32_t *)pixels, width, height, s_top);
}
