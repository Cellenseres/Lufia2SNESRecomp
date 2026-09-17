#include "lufia2_splash_credit.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lufia2_log.h"
#include "snes/ppu.h"

#define VRAM_WORDS 0x8000u
#define TILE_INDEX_COUNT 0x400u
#define TILEMAP_COLUMNS 32u
#define TILEMAP_ROWS 32u
/* Rows 28-31 are off-screen. */
#define VISIBLE_ROWS 28u
#define GLYPH_SIZE 8u
#define SPLASH_BG 0u
#define SPLASH_BG_BIT 0x01u
/* Anchor line: "LICENSED BY NINTENDO". */
#define ANCHOR_CELLS 20u
/* One blank row, matching the logo gap. */
#define CREDIT_ROW_GAP 2u
/* Above the 160 tiles the splash uploads. */
#define CREDIT_TILE_BASE 0x0100u
#define MAX_CREDIT_TILES 32u
#define STRIP_WIDTH (MAX_CREDIT_TILES * GLYPH_SIZE)

static const char kCreditText[] = "RECOMPILED BY CELLENSERES";

#define CREDIT_LENGTH ((unsigned)(sizeof kCreditText - 1u))

/* Identify the screen; not what gets drawn. */
static const uint8_t kAnchorGlyph[][GLYPH_SIZE] = {
    /* L */ { 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFE },
    /* I */ { 0x00, 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38 },
    /* C */ { 0x00, 0x7C, 0x82, 0x80, 0x80, 0x80, 0x82, 0x7C },
    /* E */ { 0x00, 0xFE, 0x80, 0x80, 0xFC, 0x80, 0x80, 0xFE },
    /* N */ { 0x00, 0x82, 0xC2, 0xA2, 0x92, 0x8A, 0x86, 0x82 },
    /* S */ { 0x00, 0x7C, 0x82, 0x80, 0x7C, 0x02, 0x82, 0x7C },
    /* D */ { 0x00, 0xF8, 0x84, 0x82, 0x82, 0x82, 0x84, 0xF8 },
    /* B */ { 0x00, 0xFC, 0x82, 0x82, 0xFC, 0x82, 0x82, 0xFC },
    /* Y */ { 0x00, 0x82, 0x44, 0x28, 0x10, 0x10, 0x10, 0x10 },
    /* T */ { 0x00, 0xFE, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 },
    /* O */ { 0x00, 0x7C, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7C }
};

enum {
    GLYPH_L = 0, GLYPH_I, GLYPH_C, GLYPH_E, GLYPH_N,
    GLYPH_S, GLYPH_D, GLYPH_B, GLYPH_Y, GLYPH_T, GLYPH_O
};

/* Cell layout of the anchor line; -1 is a space. */
static const int8_t kAnchorCell[ANCHOR_CELLS] = {
    GLYPH_L, GLYPH_I, GLYPH_C, GLYPH_E, GLYPH_N, GLYPH_S, GLYPH_E, GLYPH_D,
    -1,
    GLYPH_B, GLYPH_Y,
    -1,
    GLYPH_N, GLYPH_I, GLYPH_N, GLYPH_T, GLYPH_E, GLYPH_N, GLYPH_D, GLYPH_O
};

/* The splash never spells R, M or P. */
static const uint8_t kDrawnGlyph[][GLYPH_SIZE] = {
    /* R */ { 0x00, 0xFC, 0x82, 0x82, 0xFC, 0x88, 0x84, 0x82 },
    /* M */ { 0x00, 0x82, 0xC6, 0xAA, 0x92, 0x82, 0x82, 0x82 },
    /* P */ { 0x00, 0xFC, 0x82, 0x82, 0xFC, 0x80, 0x80, 0x80 }
};

/* Copied from that anchor cell, or drawn. */
static const struct {
    char letter;
    int8_t anchor_cell;
    int8_t drawn;
} kGlyphSource[] = {
    { ' ', -1, -1 },
    { 'B',  9, -1 }, { 'C',  2, -1 }, { 'D',  7, -1 }, { 'E',  3, -1 },
    { 'I',  1, -1 }, { 'L',  0, -1 }, { 'N',  4, -1 }, { 'O', 19, -1 },
    { 'S',  5, -1 }, { 'T', 15, -1 }, { 'Y', 10, -1 },
    { 'M', -1,  1 }, { 'P', -1,  2 }, { 'R', -1,  0 }
};

#define GLYPH_SOURCE_COUNT (sizeof kGlyphSource / sizeof *kGlyphSource)

static uint16_t s_shadow_vram[VRAM_WORDS];
static bool s_reported;

/* Whole words: "on" and "off" share a letter. */
static bool ChoiceIs(const char *choice, const char *name) {
    for (; *choice && *name; choice++, name++) {
        const char lower = (*choice >= 'A' && *choice <= 'Z')
            ? (char)(*choice + ('a' - 'A'))
            : *choice;
        if (lower != *name)
            return false;
    }
    return !*choice && !*name;
}

static bool CreditEnabled(void) {
    static int s_enabled = -1;

    if (s_enabled >= 0)
        return s_enabled != 0;

    const char *choice = getenv("LUFIA2_SPLASH_CREDIT");
    if (!choice || !*choice)
        s_enabled = 1;
    else if (ChoiceIs(choice, "off") || ChoiceIs(choice, "0") ||
             ChoiceIs(choice, "no") || ChoiceIs(choice, "n") ||
             ChoiceIs(choice, "false"))
        s_enabled = 0;
    else if (ChoiceIs(choice, "on") || ChoiceIs(choice, "1") ||
             ChoiceIs(choice, "yes") || ChoiceIs(choice, "y") ||
             ChoiceIs(choice, "true"))
        s_enabled = 1;
    else {
        /* A typo must not silently read as off. */
        fprintf(stderr,
            "[splash] LUFIA2_SPLASH_CREDIT=%s not understood; "
            "keeping the credit on\n", choice);
        s_enabled = 1;
    }
    return s_enabled != 0;
}

static unsigned TileWord(unsigned char_words, unsigned tile) {
    return (char_words + tile * 16u) & (VRAM_WORDS - 1u);
}

static unsigned MapWord(unsigned map_words, unsigned row, unsigned column) {
    return (map_words + row * TILEMAP_COLUMNS + column) & (VRAM_WORDS - 1u);
}

/* Set/clear is all the font uses. */
static uint8_t TileRowMask(
    const uint16_t *vram, unsigned char_words, unsigned tile, unsigned y) {
    const unsigned base = TileWord(char_words, tile);
    const uint16_t p01 = vram[(base + y) & (VRAM_WORDS - 1u)];
    const uint16_t p23 = vram[(base + 8u + y) & (VRAM_WORDS - 1u)];
    return (uint8_t)((p01 | (p01 >> 8) | p23 | (p23 >> 8)) & 0xFFu);
}

static void TileRowPixels(
    const uint16_t *vram, unsigned char_words, unsigned tile, unsigned y,
    uint8_t out[GLYPH_SIZE]) {
    const unsigned base = TileWord(char_words, tile);
    const uint16_t p01 = vram[(base + y) & (VRAM_WORDS - 1u)];
    const uint16_t p23 = vram[(base + 8u + y) & (VRAM_WORDS - 1u)];

    for (unsigned x = 0; x < GLYPH_SIZE; x++) {
        const unsigned bit = 7u - x;
        out[x] = (uint8_t)(((p01 >> bit) & 1u) |
                           (((p01 >> (8u + bit)) & 1u) << 1) |
                           (((p23 >> bit) & 1u) << 2) |
                           (((p23 >> (8u + bit)) & 1u) << 3));
    }
}

static bool TileMatches(
    const uint16_t *vram, unsigned char_words, unsigned tile,
    const uint8_t *want) {
    for (unsigned y = 0; y < GLYPH_SIZE; y++) {
        if (TileRowMask(vram, char_words, tile, y) != want[y])
            return false;
    }
    return true;
}

typedef struct AnchorLine {
    unsigned row;
    unsigned column;
    unsigned palette;
    unsigned colour;
    uint16_t tile[ANCHOR_CELLS];
} AnchorLine;

typedef struct CreditLayout {
    unsigned row;
    unsigned first_column;
    unsigned shift;
    unsigned tile_count;
} CreditLayout;

static bool AnchorLineAt(
    const uint16_t *vram, unsigned map_words, unsigned char_words,
    unsigned row, unsigned column, AnchorLine *out) {
    AnchorLine found;
    bool have_palette = false;

    memset(&found, 0, sizeof found);
    for (unsigned i = 0; i < ANCHOR_CELLS; i++) {
        const uint16_t entry = vram[MapWord(map_words, row, column + i)];

        if (kAnchorCell[i] < 0) {
            if (entry != 0u)
                return false;
            continue;
        }
        /* Flip or other palette: a different screen. */
        if (entry & 0xC000u)
            return false;
        if (!have_palette) {
            found.palette = (entry >> 10) & 7u;
            have_palette = true;
        } else if (((entry >> 10) & 7u) != found.palette) {
            return false;
        }
        if (!TileMatches(vram, char_words, entry & 0x3FFu,
                kAnchorGlyph[kAnchorCell[i]]))
            return false;
        found.tile[i] = (uint16_t)(entry & 0x3FFu);
    }
    if (!have_palette)
        return false;

    uint8_t first_row[GLYPH_SIZE];
    found.row = row;
    found.column = column;
    /* One colour; read it from L. */
    TileRowPixels(vram, char_words, found.tile[0], 1u, first_row);
    found.colour = first_row[0];
    if (!found.colour)
        return false;

    *out = found;
    return true;
}

/* Found by its own pixels, not at a fixed address. */
static bool FindAnchorLine(
    const uint16_t *vram, unsigned map_words, unsigned char_words,
    AnchorLine *out) {
    for (unsigned row = 0; row < VISIBLE_ROWS; row++) {
        for (unsigned column = 0;
             column + ANCHOR_CELLS <= TILEMAP_COLUMNS; column++) {
            if (AnchorLineAt(vram, map_words, char_words, row, column, out))
                return true;
        }
    }
    return false;
}

static bool SplashLayerReady(const Ppu *ppu) {
    return PPU_mode(ppu) == 1 &&
           ppu->screenEnabled[0] == (uint8_t)SPLASH_BG_BIT &&
           ppu->screenEnabled[1] == 0u &&
           ppu->hScroll[SPLASH_BG] == 0u &&
           ppu->vScroll[SPLASH_BG] == 0u &&
           !PPU_bgTilemapWider(ppu, SPLASH_BG) &&
           !PPU_bgTilemapHigher(ppu, SPLASH_BG);
}

static bool PlanCredit(
    const Ppu *ppu, unsigned map_words, const AnchorLine *anchor,
    CreditLayout *out) {
    const unsigned anchor_centre =
        anchor->column * GLYPH_SIZE + (ANCHOR_CELLS * GLYPH_SIZE) / 2u;
    const unsigned half = (CREDIT_LENGTH * GLYPH_SIZE) / 2u;

    if (anchor_centre < half)
        return false;

    /* 25 chars do not centre on the 8px grid. */
    const unsigned left_pixel = anchor_centre - half;
    out->first_column = left_pixel / GLYPH_SIZE;
    out->shift = left_pixel % GLYPH_SIZE;
    out->tile_count =
        (out->shift + CREDIT_LENGTH * GLYPH_SIZE + GLYPH_SIZE - 1u) /
        GLYPH_SIZE;
    out->row = anchor->row + CREDIT_ROW_GAP;

    if (out->tile_count > MAX_CREDIT_TILES ||
        out->first_column + out->tile_count > TILEMAP_COLUMNS ||
        out->row >= VISIBLE_ROWS ||
        CREDIT_TILE_BASE + out->tile_count > TILE_INDEX_COUNT)
        return false;

    /* Borders mirror the far end of the row. */
    const unsigned left_cells = ((unsigned)ppu->extraLeftCur + 7u) / 8u;
    const unsigned right_cells = ((unsigned)ppu->extraRightCur + 7u) / 8u;
    if (left_cells >= TILEMAP_COLUMNS ||
        out->first_column < right_cells ||
        out->first_column + out->tile_count > TILEMAP_COLUMNS - left_cells)
        return false;

    /* Must not cover what the screen shows. */
    for (unsigned i = 0; i < out->tile_count; i++) {
        if (ppu->vram[MapWord(map_words, out->row, out->first_column + i)])
            return false;
    }
    for (unsigned i = 0; i < TILEMAP_COLUMNS * TILEMAP_ROWS; i++) {
        const unsigned tile =
            ppu->vram[(map_words + i) & (VRAM_WORDS - 1u)] & 0x3FFu;
        if (tile >= CREDIT_TILE_BASE &&
            tile < CREDIT_TILE_BASE + out->tile_count)
            return false;
    }
    return true;
}

static bool ComposeStrip(
    const uint16_t *vram, unsigned char_words, const AnchorLine *anchor,
    const CreditLayout *layout, uint8_t strip[GLYPH_SIZE][STRIP_WIDTH]) {
    memset(strip, 0, GLYPH_SIZE * STRIP_WIDTH);

    for (unsigned k = 0; k < CREDIT_LENGTH; k++) {
        const unsigned x0 = layout->shift + k * GLYPH_SIZE;
        size_t entry = 0;

        while (entry < GLYPH_SOURCE_COUNT &&
               kGlyphSource[entry].letter != kCreditText[k])
            entry++;
        if (entry == GLYPH_SOURCE_COUNT)
            return false;

        const int8_t cell = kGlyphSource[entry].anchor_cell;
        const int8_t drawn = kGlyphSource[entry].drawn;
        for (unsigned y = 0; y < GLYPH_SIZE; y++) {
            uint8_t row[GLYPH_SIZE] = { 0 };

            if (cell >= 0) {
                TileRowPixels(vram, char_words, anchor->tile[cell], y, row);
            } else if (drawn >= 0) {
                for (unsigned x = 0; x < GLYPH_SIZE; x++) {
                    row[x] = ((kDrawnGlyph[drawn][y] >> (7u - x)) & 1u)
                        ? (uint8_t)anchor->colour
                        : 0u;
                }
            }
            for (unsigned x = 0; x < GLYPH_SIZE; x++) {
                if (row[x])
                    strip[y][x0 + x] = row[x];
            }
        }
    }
    return true;
}

static void WriteTile(
    uint16_t *vram, unsigned char_words, unsigned tile,
    const uint8_t strip[GLYPH_SIZE][STRIP_WIDTH], unsigned x0) {
    const unsigned base = TileWord(char_words, tile);

    for (unsigned y = 0; y < GLYPH_SIZE; y++) {
        uint16_t p01 = 0;
        uint16_t p23 = 0;
        for (unsigned x = 0; x < GLYPH_SIZE; x++) {
            const unsigned value = strip[y][x0 + x];
            const unsigned bit = 7u - x;
            p01 = (uint16_t)(p01 | ((value & 1u) << bit) |
                             (((value >> 1) & 1u) << (8u + bit)));
            p23 = (uint16_t)(p23 | (((value >> 2) & 1u) << bit) |
                             (((value >> 3) & 1u) << (8u + bit)));
        }
        vram[(base + y) & (VRAM_WORDS - 1u)] = p01;
        vram[(base + 8u + y) & (VRAM_WORDS - 1u)] = p23;
    }
}

static bool BuildCredit(Ppu *ppu) {
    AnchorLine anchor;
    CreditLayout layout;
    uint8_t strip[GLYPH_SIZE][STRIP_WIDTH];

    if (!SplashLayerReady(ppu))
        return false;

    const unsigned map_words = (unsigned)PPU_bgTilemapAdr(ppu, SPLASH_BG);
    const unsigned char_words = (unsigned)PPU_bgTileAdr(ppu, SPLASH_BG);

    if (!FindAnchorLine(ppu->vram, map_words, char_words, &anchor) ||
        !PlanCredit(ppu, map_words, &anchor, &layout) ||
        !ComposeStrip(ppu->vram, char_words, &anchor, &layout, strip))
        return false;

    /* Drawing only; the guest never reads this. */
    memcpy(s_shadow_vram, ppu->vram, sizeof s_shadow_vram);
    for (unsigned i = 0; i < layout.tile_count; i++) {
        const unsigned tile = CREDIT_TILE_BASE + i;
        WriteTile(s_shadow_vram, char_words, tile, strip, i * GLYPH_SIZE);
        s_shadow_vram[
            MapWord(map_words, layout.row, layout.first_column + i)] =
                (uint16_t)((anchor.palette << 10) | tile);
    }
    ppu->renderVram = s_shadow_vram;

    if (!s_reported) {
        LUFIA2_LOG(
            "[splash] credit \"%s\" at row %u column %u "
            "(anchor row %u, palette %u, colour %u)\n",
            kCreditText, layout.row, layout.first_column,
            anchor.row, anchor.palette, anchor.colour);
        s_reported = true;
    }
    return true;
}

void Lufia2SplashCreditRelease(Ppu *ppu) {
    if (ppu && ppu->renderVram == s_shadow_vram)
        ppu->renderVram = NULL;
}

void Lufia2SplashCreditPrepare(Ppu *ppu) {
    if (!ppu)
        return;
    if (!CreditEnabled() || !BuildCredit(ppu))
        Lufia2SplashCreditRelease(ppu);
}
