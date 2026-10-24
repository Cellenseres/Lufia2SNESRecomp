#include "lufia2_intro_trees.h"

#include <string.h>

enum {
    TREE_ROWS = 32,
    TREE_COLUMNS = 32,          /* what the guest's tilemap holds */
    TREE_HISTORY = 512,         /* virtual columns the host remembers */
    TREE_SCROLL_PERIOD = 1024,  /* the BG scroll register is ten bits */
    TREE_SCROLL_COLUMNS = TREE_SCROLL_PERIOD / 8,
    /* The guest recycles the oldest slots for arriving columns. */
    TREE_SETTLED_AFTER = 6,
    /* Room for a column to arrive out of sight. */
    TREE_LEAD_MARGINS = 3,
};

static uint16_t s_column[TREE_HISTORY][TREE_ROWS];
static bool s_known[TREE_HISTORY];
static int32_t s_stored[TREE_HISTORY];

static int32_t s_base;          /* accumulated scroll wraps, in columns */
static int32_t s_previous;      /* previous raw column, to detect a wrap */
static bool s_started;

void Lufia2IntroTreesReset(void) {
    memset(s_known, 0, sizeof s_known);
    s_base = 0;
    s_previous = 0;
    s_started = false;
}

/* The raw column repeats every 128 steps; carry the wraps. */
static int32_t AbsoluteColumn(int32_t raw) {
    if (!s_started) {
        s_started = true;
    } else if (raw < s_previous - TREE_SCROLL_COLUMNS / 2) {
        s_base += TREE_SCROLL_COLUMNS;
    } else if (raw > s_previous + TREE_SCROLL_COLUMNS / 2) {
        s_base -= TREE_SCROLL_COLUMNS;
    }
    s_previous = raw;
    return s_base + raw;
}

static uint16_t FrameScroll(
    const Ppu *ppu, unsigned layer, const uint16_t *line_scroll, size_t lines) {
    return (line_scroll && lines) ? line_scroll[0] : ppu->hScroll[layer];
}

static size_t Slot(int32_t column) {
    return (size_t)(((column % TREE_HISTORY) + TREE_HISTORY) % TREE_HISTORY);
}

/* A 32-column ring. */
static void ReadColumn(
    const Ppu *ppu, unsigned tilemap, int32_t raw, uint16_t *out) {
    const unsigned physical =
        (unsigned)(((raw % TREE_COLUMNS) + TREE_COLUMNS) % TREE_COLUMNS);
    for (unsigned row = 0; row < TREE_ROWS; row++)
        out[row] = ppu->vram[(tilemap + row * TREE_COLUMNS + physical) &
                             0x7fffu];
}

static void Remember(int32_t column, const uint16_t *entries) {
    const size_t slot = Slot(column);
    memcpy(s_column[slot], entries, sizeof s_column[slot]);
    s_stored[slot] = column;
    s_known[slot] = true;
}

static const uint16_t *Recall(int32_t column) {
    const size_t slot = Slot(column);
    if (!s_known[slot] || s_stored[slot] != column)
        return NULL;
    return s_column[slot];
}

void Lufia2IntroTreesObserve(
    const Ppu *ppu, unsigned layer, const uint16_t *line_scroll, size_t lines) {
    if (!ppu || layer >= 4u)
        return;

    const unsigned tilemap = (unsigned)PPU_bgTilemapAdr(ppu, layer);
    const int32_t raw = FrameScroll(ppu, layer, line_scroll, lines) >> 3;
    const int32_t left = AbsoluteColumn(raw);
    uint16_t entries[TREE_ROWS];

    for (int i = TREE_SETTLED_AFTER; i < TREE_COLUMNS; i++) {
        const int32_t column = left + i;
        ReadColumn(ppu, tilemap, raw + i, entries);
        Remember(column, entries);
    }

}

static uint32_t PaletteColour(const Ppu *ppu, unsigned index) {
    const uint16_t colour = ppu->cgram[index & 0xffu];
    return 0xff000000u |
           ((uint32_t)ppu->brightnessMult[colour & 0x1fu] << 16) |
           ((uint32_t)ppu->brightnessMult[(colour >> 5) & 0x1fu] << 8) |
           (uint32_t)ppu->brightnessMult[(colour >> 10) & 0x1fu];
}

/* Mode 1 BG3: two bitplanes, four colours, zero transparent. */
static unsigned TilePixel(
    const Ppu *ppu, unsigned chars, unsigned tile, unsigned x, unsigned y) {
    const uint16_t planes = ppu->vram[(chars + tile * 8u + y) & 0x7fffu];
    const unsigned bit = 7u - x;
    return ((planes >> bit) & 1u) | (((planes >> (8u + bit)) & 1u) << 1);
}

static int32_t FloorDivide8(int64_t value) {
    return (int32_t)((value >= 0) ? (value >> 3) : -(((-value) + 7) >> 3));
}

bool Lufia2IntroTreesCapture(const Ppu *ppu, unsigned layer, size_t width,
    size_t margin_width, const uint16_t *line_scroll, size_t lines,
    Lufia2IntroTreeSnapshot *out) {
    if (!ppu || layer >= 4u || !out || !width || width > 512u ||
        !margin_width || margin_width > 128u) return false;
    const uint16_t scroll = FrameScroll(ppu, layer, line_scroll, lines);
    const int32_t left = s_base + (int32_t)(scroll >> 3);
    const int64_t origin = (int64_t)s_base * 8 + scroll -
        TREE_LEAD_MARGINS * (int64_t)margin_width;
    const int32_t first = FloorDivide8(origin);
    const int32_t last = FloorDivide8(origin + (int64_t)width - 1);
    if (last - first + 1 > LUFIA2_INTRO_TREE_MAX_COLUMNS) return false;
    memset(out, 0, sizeof *out);
    out->first_column = first;
    out->width = (uint16_t)width;
    out->column_count = (uint16_t)(last - first + 1);
    out->fine_x = (uint16_t)(origin - (int64_t)first * 8);
    out->chars = (uint16_t)PPU_bgTileAdr(ppu, layer);
    out->scroll_y = ppu->vScroll[layer];
    for (unsigned i = 0; i < 32u; ++i) out->palette[i] = PaletteColour(ppu, i);
    const unsigned tilemap = (unsigned)PPU_bgTilemapAdr(ppu, layer);
    for (unsigned i = 0; i < out->column_count; ++i) {
        const int32_t column = first + (int32_t)i;
        if (column >= left + TREE_SETTLED_AFTER && column < left + TREE_COLUMNS) {
            ReadColumn(ppu, tilemap, column - s_base, out->entries[i]);
            out->known[i] = 1;
        } else {
            const uint16_t *entries = Recall(column);
            if (entries) {
                memcpy(out->entries[i], entries, sizeof out->entries[i]);
                out->known[i] = 1;
            }
        }
    }
    return true;
}

bool Lufia2IntroTreesCaptureUnshaded(const Ppu *ppu, unsigned layer, size_t width,
    size_t margin_width, const uint16_t *line_scroll, size_t lines,
    Lufia2IntroTreeSnapshot *out, uint8_t *brightness_reduction) {
    if(!ppu || !brightness_reduction || ppu->brightnessMult[31]%17u) return false;
    const unsigned brightness=ppu->brightnessMult[31]/17u;
    if(!Lufia2IntroTreesCapture(ppu,layer,width,margin_width,line_scroll,lines,out)) return false;
    for(unsigned index=0;index<32;++index) {
        const uint16_t colour=ppu->cgram[index];
        uint32_t raw=0xff000000u,shaded=0xff000000u;
        for(unsigned channel=0;channel<3;++channel) {
            const unsigned level=(colour>>(channel*5u))&31u;
            const unsigned value=level*8u+level/4u,shift=16u-channel*8u;
            raw|=value<<shift;shaded|=(value*brightness/15u)<<shift;
        }
        if(shaded!=out->palette[index]) return false;
        out->palette[index]=raw;
    }
    *brightness_reduction=(uint8_t)(15u-brightness);
    return true;
}

bool Lufia2IntroTreesPaint(
    const Ppu *ppu,
    unsigned layer,
    uint8_t *frame,
    size_t width,
    size_t height,
    size_t margin_width,
    const uint16_t *line_scroll,
    size_t lines) {
    if (!ppu || layer >= 4u || !frame || !width || !height || !margin_width)
        return false;
    if (width > SIZE_MAX / sizeof(uint32_t))
        return false;

    const unsigned tilemap = (unsigned)PPU_bgTilemapAdr(ppu, layer);
    const unsigned chars = (unsigned)PPU_bgTileAdr(ppu, layer);
    const uint16_t scroll = FrameScroll(ppu, layer, line_scroll, lines);
    const int32_t left = s_base + (int32_t)(scroll >> 3);
    const unsigned scroll_y = ppu->vScroll[layer];
    const size_t pitch = width * sizeof(uint32_t);
    bool complete = true;

    /* Shifted back so only the left needs remembered columns. */
    const int64_t origin =
        (int64_t)s_base * 8 + (int64_t)scroll -
        TREE_LEAD_MARGINS * (int64_t)margin_width;
    const int32_t first = FloorDivide8(origin);
    const int32_t last = FloorDivide8(origin + (int64_t)width - 1);
    uint16_t live[TREE_ROWS];

    for (int32_t column = first; column <= last; column++) {
        const uint16_t *entries;
        /* The oldest slots already hold the arriving strip. */
        if (column >= left + TREE_SETTLED_AFTER &&
            column < left + TREE_COLUMNS) {
            ReadColumn(ppu, tilemap, column - s_base, live);
            entries = live;
        } else {
            entries = Recall(column);
        }
        if (!entries) {
            complete = false;
            continue;
        }

        for (unsigned sub = 0; sub < 8u; sub++) {
            const int64_t x = (int64_t)column * 8 + sub - origin;
            if (x < 0 || x >= (int64_t)width)
                continue;
            for (size_t y = 0; y < height; y++) {
                const unsigned line = (unsigned)y + scroll_y;
                const uint16_t entry = entries[(line >> 3) % TREE_ROWS];
                const unsigned fine_y = line & 7u;
                const unsigned sx = (entry & 0x4000u) ? 7u - sub : sub;
                const unsigned sy = (entry & 0x8000u) ? 7u - fine_y : fine_y;
                const unsigned value =
                    TilePixel(ppu, chars, entry & 0x3ffu, sx, sy);
                if (!value)
                    continue;
                const uint32_t colour = PaletteColour(
                    ppu, ((entry >> 10) & 7u) * 4u + value);
                memcpy(frame + y * pitch + (size_t)x * sizeof(uint32_t),
                       &colour, sizeof colour);
            }
        }
    }
    return complete;
}
