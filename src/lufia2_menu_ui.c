#include "lufia2_menu_ui.h"
#include "lufia2_ui_layout_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#undef HIBYTE
#include <windows.h>
#endif

extern uint8_t g_ram[0x20000];

enum { WIDTH = LUFIA2_UI_NATIVE_WIDTH, HEIGHT = LUFIA2_UI_HEIGHT,
       CANVAS = LUFIA2_UI_CANVAS_WIDTH, MAX_RECORDS = 4096,
       WINDOW = 0, TEXT = 1, SPRITE = 2, GROUP = 3, CURSOR = 4,
       CURSOR_POINT = 5, TEXT_REGION = 6, SPRITE_SLOT = 7, WINDOW_SOURCE = 8 };
enum { PARTY_HIGHLIGHT = 9 };
enum { MAIN_MENU_SCENE = 0x6098f96f, TIME_GOLD_WINDOW = 0x1000150e,
       GOLD_TEXT = 0x20001916, LEGACY_GOLD_SUFFIX = 0x2000191c };
typedef struct MenuLayoutRecord {
    uint32_t scene, id, parent;
    int16_t x, y;
    uint16_t width, height, kind, visible;
    uint16_t source_width, source_height;
} MenuLayoutRecord;
typedef struct MenuDestination { int x, y, width, height; bool visible; } MenuDestination;

static const uint8_t *s_rom;
static size_t s_rom_size;
static char s_layout_path[1024], s_preview_path[1024];
static MenuLayoutRecord s_layout[MAX_RECORDS];
static unsigned s_layout_count, s_count, s_width, s_frames;
static uint32_t s_scene;
static unsigned s_theme;
static bool s_active;
static bool s_rendering;
static Lufia2MenuUiObject s_objects[LUFIA2_MENU_UI_MAX_OBJECTS];
static MenuDestination s_targets[LUFIA2_MENU_UI_MAX_OBJECTS];
static uint32_t s_planes[3][HEIGHT][WIDTH];
static uint16_t s_priority[3][HEIGHT][WIDTH];
static uint16_t s_background_priority[HEIGHT][WIDTH];
static uint8_t s_sprite_owner[HEIGHT][WIDTH], s_oam_slot[128], s_line_owner[WIDTH];
static uint8_t s_coverage[3][HEIGHT][WIDTH];
static uint16_t s_text_owner[HEIGHT][WIDTH];
static unsigned s_rows;
static int s_text_scroll[HEIGHT];
static bool s_text_rows;
static bool s_layout_coordinates;
static bool s_move_highlight;
static uint32_t s_highlight[HEIGHT][kPpuBufWidth];
static unsigned s_text_hscroll, s_text_vscroll;
static uint8_t s_last_layout[LUFIA2_UI_LAYOUT_HEADER_BYTES + MAX_RECORDS * LUFIA2_MENU_UI_RECORD];
static size_t s_last_layout_size;
enum { SPRITE_STATE_START = 0x11d8, SPRITE_STATE_END = 0x1509 };
static uint8_t s_latched_sprites[SPRITE_STATE_END - SPRITE_STATE_START];
static bool s_latched_valid;
static bool s_latched_available;
static const uint8_t *s_sprite_state;
static bool s_object_latched[LUFIA2_MENU_UI_MAX_OBJECTS];

static uint32_t HashSceneWord(uint32_t hash, unsigned value) {
    for (unsigned i = 0; i < 4; ++i) hash = (hash ^ (uint8_t)(value >> (i * 8))) * 16777619u;
    return hash;
}
static uint8_t BusByte(uint32_t address) {
    unsigned bank = address >> 16, low = address & 65535u;
    if (bank == 0x7e || bank == 0x7f) return g_ram[(bank - 0x7e) * 65536u + low];
    if ((bank & 0x7f) < 0x40 && low < 0x2000) return g_ram[low];
    size_t offset = (bank & 0x7f) * 0x8000u + (low & 0x7fff);
    return low >= 0x8000 && offset < s_rom_size ? s_rom[offset] : 0xff;
}
static unsigned SpriteByte(const uint8_t *state, unsigned address) {
    return state ? state[address - SPRITE_STATE_START] : g_ram[address];
}
static unsigned SpriteWord(unsigned low, unsigned high, unsigned index) {
    return SpriteByte(s_sprite_state, low + index) | SpriteByte(s_sprite_state, high + index) << 8;
}
static unsigned SpritePieces(const uint8_t *state, unsigned slot, uint32_t *frame) {
    *frame = SpriteByte(state, 0x1328 + slot) | SpriteByte(state, 0x1358 + slot) << 8 |
        SpriteByte(state, 0x1238 + slot) << 16;
    unsigned count = 0;
    while (count < 128 && BusByte(*frame + 2 + count * 4) != 255) ++count;
    return count;
}
static bool SpriteMatches(const Ppu *ppu, const uint8_t *state, unsigned slot,
                          uint32_t frame, unsigned first, unsigned count) {
    if (!count || first + count > 128) return false;
    unsigned ox = SpriteByte(state, 0x1388 + slot) | SpriteByte(state, 0x13b8 + slot) << 8;
    unsigned oy = SpriteByte(state, 0x13e8 + slot) | SpriteByte(state, 0x1418 + slot) << 8;
    for (unsigned piece = 0; piece < count; ++piece) {
        unsigned at = first + piece, high = ppu->highOam[at / 4] >> ((at % 4) * 2);
        uint32_t entry = frame + 2 + piece * 4;
        unsigned x = (ox - (int8_t)BusByte(entry + 1)) & 511u;
        unsigned y = (oy - BusByte(entry)) & 255u;
        unsigned tile = BusByte(entry + 2) | BusByte(entry + 3) << 8;
        if ((ppu->oam[at * 2] & 255u) != (x & 255u) ||
            (unsigned)(ppu->oam[at * 2] >> 8) != y || ppu->oam[at * 2 + 1] != tile ||
            (high & 3u) != ((x >> 8) | ((BusByte(frame + 1) & 1u) << 1))) return false;
    }
    return true;
}
static bool MatchSprites(const Ppu *ppu, const uint8_t *state) {
    unsigned first = 0;
    for (unsigned n = 0; n < 48; ++n) {
        unsigned slot = (n + 12) % 48;
        if (!SpriteByte(state, 0x11d8 + slot)) continue;
        uint32_t frame;
        unsigned count = SpritePieces(state, slot, &frame);
        if (!SpriteMatches(ppu, state, slot, frame, first, count)) return false;
        first += count;
    }
    return first != 0;
}
void Lufia2MenuUiLatchSprites(const Ppu *ppu) {
    s_latched_valid = s_latched_available = false;
    if (!s_rom || !ppu || PPU_mode(ppu) != 1 ||
        PPU_bgTileAdr(ppu, 0) != 0x4000 || PPU_bgTileAdr(ppu, 2) != 0x6000) return;
    /* NMI has uploaded OAM; the guest has not yet prepared its next animation. */
    memcpy(s_latched_sprites, g_ram + SPRITE_STATE_START, sizeof s_latched_sprites);
    s_latched_available = true;
    s_latched_valid = MatchSprites(ppu, s_latched_sprites);
}
static void ReadTextRowScroll(void) {
    s_text_rows = false;
    if (g_ram[0xfa] != 0x12 || g_ram[0xfb] != 2 || g_ram[0xf9] != 0x7e) return;
    unsigned pointer = g_ram[0xf7] | (unsigned)g_ram[0xf8] << 8;
    if (pointer != 0x80c0) return;
    unsigned y = 0;
    while (y < HEIGHT && pointer < 0x81c0) {
        unsigned control = g_ram[pointer++];
        if (!control) break;
        unsigned lines = control & 127; if (!lines) lines = 128;
        int value = (int16_t)(g_ram[pointer] | (unsigned)g_ram[pointer + 1] << 8);
        pointer += 2;
        for (unsigned line = 0; line < lines && y < HEIGHT; ++line) {
            s_text_scroll[y++] = value;
            if ((control & 128) && line + 1 < lines) {
                if (pointer + 2 >= 0x81c0) return;
                value = (int16_t)(g_ram[pointer] | (unsigned)g_ram[pointer + 1] << 8);
                pointer += 2;
            }
        }
    }
    if (!y) return;
    for (unsigned line = y; line < HEIGHT; ++line) s_text_scroll[line] = s_text_scroll[y - 1];
    s_text_rows = true;
}
static int TextRowY(const Ppu *ppu, unsigned tile_row) {
    int source = (int)tile_row * 8 - 1;
    if (s_text_rows) for (int y = 0; y < HEIGHT; ++y)
        if (((y + s_text_scroll[y]) & 255) == (source & 255)) return y;
    return (int)(((unsigned)source + 1 - ppu->vScroll[2]) & 255u) - 1;
}
static bool ObjectFitsInside(const Lufia2MenuUiObject *object, const Lufia2MenuUiObject *container) {
    return object->x >= container->x && object->y >= container->y &&
           object->x + object->width <= container->x + container->width &&
           object->y + object->height <= container->y + container->height;
}
static int FindObject(uint32_t id) {
    for (unsigned i = 0; i < s_count; ++i) if (s_objects[i].id == id) return (int)i;
    return -1;
}
static const MenuLayoutRecord *FindLayoutOverride(unsigned index);
static MenuDestination ResolveObjectDestination(unsigned index, unsigned depth);
static int AddObject(uint32_t id, unsigned kind, int x, int y, unsigned w, unsigned h, unsigned slot) {
    if (!w || !h || s_count == LUFIA2_MENU_UI_MAX_OBJECTS || x < 0 || y < 0 ||
        x + w > WIDTH || y + h > HEIGHT) return -1;
    unsigned i = s_count++;
    s_objects[i] = (Lufia2MenuUiObject){id, 0, (int16_t)x, (int16_t)y,
        (uint16_t)w, (uint16_t)h, (uint8_t)kind, (uint8_t)slot};
    return (int)i;
}
static bool HasGlyph(unsigned word) { return (word & 1023u) != 0 && (word & 1023u) != 32; }
static int MenuCoordinate(unsigned pixel, unsigned scroll) {
    /* These menu maps repeat every 256 pixels; $03FE is -2, not 1022. */
    return (int)((pixel - scroll) & 255u);
}

static void CollectWindows(const Ppu *ppu) {
    const uint16_t *map = PpuRenderVram(ppu) + PPU_bgTilemapAdr(ppu, 0);
    for (unsigned y = 0; y < 32; ++y) for (unsigned x = 0; x < 32; ++x) {
        if ((map[y * 32 + x] & 0xc3ff) != 1) continue;
        unsigned right = x + 3, bottom = y + 2;
        while (right < 32 && (map[y * 32 + right] & 0xc3ff) != 0x4001) ++right;
        while (bottom < 32 && (map[bottom * 32 + x] & 0xc3ff) != 0x8001) ++bottom;
        if (right == 32 || bottom == 32 || (map[bottom * 32 + right] & 0xc3ff) != 0xc001) continue;
        int left = MenuCoordinate(x * 8, ppu->hScroll[0]);
        int top = MenuCoordinate(y * 8, ppu->vScroll[0]) - 1;
        int last_x = left + (right - x + 1) * 8, last_y = top + (bottom - y + 1) * 8;
        if (left < 0) left = 0;
        if (top < 0) top = 0;
        if (last_x > WIDTH) last_x = WIDTH;
        if (last_y > HEIGHT) last_y = HEIGHT;
        if (last_x > left && last_y > top)
            AddObject(0x10000000u | y << 8 | x, WINDOW, left, top, last_x - left, last_y - top, 255);
    }
}

static uint32_t SaveRecordWindow(unsigned record) {
    /* $82:EF5E..EF7F builds these four save-record windows. */
    static const uint32_t ids[] = {0x10000501u, 0x10000510u, 0x10001001u, 0x10001010u};
    for (unsigned i = 0; i < 4; ++i) {
        int at = FindObject(ids[i]);
        if (at < 0 || s_objects[at].kind != WINDOW ||
            s_objects[at].width != 120 || s_objects[at].height != 88) return 0;
    }
    return record < 4 ? ids[record] : 0;
}

static void CollectPartyFields(const Ppu *ppu) {
    /* START overlaps the usual name tile; it is not a party field. */
    if (SaveRecordWindow(0)) return;
    const uint16_t *map = PpuRenderVram(ppu) + PPU_bgTilemapAdr(ppu, 2);
    /* Original party coordinates: $82:A36B. */
    unsigned members = g_ram[0x0a7a];
    if (members > 4) return;
    for (unsigned member = 0; member < members; ++member) {
        unsigned address = 0x82a36b + member * 2;
        unsigned offset = BusByte(address) | (unsigned)BusByte(address + 1) << 8;
        unsigned tile = (offset - 0x3000) / 2;
        /* $80:8DB3 puts ordinary characters one tile row below X.
         * Keep old explicit-top-row layouts usable as well. */
        if (tile + 32 < 1024 && !HasGlyph(map[tile]) && HasGlyph(map[tile + 32])) tile += 32;
        if (tile >= 1024 || !HasGlyph(map[tile])) continue;
        int x = (tile % 32) * 8 - ppu->hScroll[2];
        int y = TextRowY(ppu, tile / 32);
        bool covered = false;
        Lufia2MenuUiObject area = {0, 0, (int16_t)x, (int16_t)y, 88, 24, GROUP, 255};
        for (unsigned i = 0; i < s_count; ++i)
            if (s_objects[i].kind == WINDOW && ObjectFitsInside(&area, &s_objects[i])) covered = true;
        if (covered) continue;
        int group = AddObject(0x40000000u | member, GROUP, x - 32, y - 8, 120, 41, member);
        if (group < 0) continue;
        static const unsigned widths[] = {48, 40, 88, 88};
        for (unsigned field = 0; field < 4; ++field) {
            int child = AddObject(0x50000000u | member << 8 | field, TEXT,
                x + (field == 1 ? 48 : 0), y + (field > 1 ? (field - 1) * 8 : 0),
                widths[field], 8, member);
            if (child >= 0) s_objects[child].parent = s_objects[group].id;
        }
    }
}

static void CollectPartyHighlights(const Ppu *ppu) {
    const uint16_t *map = PpuRenderVram(ppu) + PPU_bgTilemapAdr(ppu, 1);
    /* $82:9984 recolours a 15x5 BG2 rectangle from $8E:E576.
     * This is a backdrop selection, not a BG1 window definition. */
    for (unsigned member = 0; member < 4; ++member) {
        uint32_t parent = 0x40000000u | member;
        if (FindObject(parent) < 0) continue;
        unsigned address = 0x8ee576 + member * 2;
        unsigned tile = (BusByte(address) | (unsigned)BusByte(address + 1) << 8) / 2;
        unsigned tx = tile % 32, ty = tile / 32;
        if (tx + 15 > 32 || ty + 5 > 32) continue;
        bool selected = true;
        for (unsigned y = 0; y < 5; ++y) for (unsigned x = 0; x < 15; ++x)
            selected &= (map[(ty + y) * 32 + tx + x] & 0x1c00) == 0x0800;
        if (!selected) continue;
        int index = AddObject(0xb0000000u | member, PARTY_HIGHLIGHT,
            MenuCoordinate(tx * 8, ppu->hScroll[1]),
            MenuCoordinate(ty * 8, ppu->vScroll[1]) - 1, 120, 40, member);
        if (index >= 0) s_objects[index].parent = parent;
    }
}

static bool TextOwnsPoint(int x, int y) {
    for (unsigned i = 0; i < s_count; ++i) {
        const Lufia2MenuUiObject *o = &s_objects[i];
        if (o->kind == TEXT &&
            x >= o->x && x < o->x + o->width && y >= o->y && y < o->y + o->height) return true;
    }
    return false;
}
static void CollectListSlots(const Ppu *ppu) {
    if (s_theme != 2 && s_theme != 3) return;
    /* $82:AC84 and $82:8526 reserve six rows. */
    unsigned columns = s_theme == 3 ? 2 : 1;
    for (unsigned row = 0; row < 6; ++row) for (unsigned column = 0; column < columns; ++column) {
        unsigned tile = (0x3448u - 0x3000u + row * 0x80u + column * 0x1cu) / 2;
        int x = (tile % 32) * 8 - ppu->hScroll[2];
        int y = TextRowY(ppu, tile / 32);
        for (unsigned i = 0; i < s_count; ++i) {
            const Lufia2MenuUiObject *window = &s_objects[i];
            if (window->kind != WINDOW || x < window->x + 8 || y < window->y + 8 ||
                x + 8 > window->x + window->width - 8 ||
                window->height < 64 || TextRowY(ppu, 17) + 8 > window->y + window->height) continue;
            unsigned remaining = window->x + window->width - 8 - x;
            unsigned width = columns == 2 && remaining > 112 ? 112 : remaining;
            uint32_t parent = window->id;
            int index = AddObject(0x20000000u | (tile / 32) << 8 | (tile % 32), TEXT, x, y, width, 8, 255);
            if (index >= 0) s_objects[index].parent = parent;
            break;
        }
    }
}
static void CollectNumericFields(const Ppu *ppu) {
    if (s_scene != MAIN_MENU_SCENE || FindObject(TIME_GOLD_WINDOW) < 0) return;
    /* $82:A318/$8E:D506: "GOLD  " followed by $01, $20, $01, $0A8A.
     * $80:88EE prints seven right-aligned digits, including leading spaces.
     * Reserve the complete source field before finding occupied text runs.
     * The two trailing tiles retain the original window's editing margin. */
    const unsigned columns[] = {16, 22}, widths[] = {48, 72};
    for (unsigned field = 0; field < 2; ++field) {
        int index = AddObject(0x20001900u | columns[field], TEXT,
            (int)columns[field] * 8 - ppu->hScroll[2], TextRowY(ppu, 25), widths[field], 8, 255);
        if (index >= 0) s_objects[index].parent = TIME_GOLD_WINDOW;
    }
}
static void CollectTextRuns(const Ppu *ppu) {
    const uint16_t *map = PpuRenderVram(ppu) + PPU_bgTilemapAdr(ppu, 2);
    for (unsigned y = 0; y < 32; ++y) for (unsigned x = 0; x < 32;) {
        int px = (int)x * 8 - ppu->hScroll[2], py = TextRowY(ppu, y);
        if (!HasGlyph(map[y * 32 + x]) || TextOwnsPoint(px, py)) { ++x; continue; }
        unsigned end = x + 1;
        while (end < 32 && !TextOwnsPoint((int)end * 8 - ppu->hScroll[2], py) &&
               (HasGlyph(map[y * 32 + end]) || (end + 1 < 32 && HasGlyph(map[y * 32 + end + 1])))) ++end;
        unsigned boundary = end;
        while (boundary < 32 && !HasGlyph(map[y * 32 + boundary]) &&
               !TextOwnsPoint((int)boundary * 8 - ppu->hScroll[2], py)) ++boundary;
        for (unsigned i = 0; i < s_count; ++i) {
            const Lufia2MenuUiObject *p = &s_objects[i];
            if (p->kind == WINDOW && px >= p->x && px < p->x + p->width &&
                py >= p->y && py + 8 <= p->y + p->height) {
                unsigned edge = (p->x + p->width + ppu->hScroll[2]) / 8;
                if (boundary > edge) boundary = edge;
            }
        }
        int index = AddObject(0x20000000u | y << 8 | x, TEXT, px, py, (boundary - x) * 8, 8, 255);
        if (index >= 0) {
            Lufia2MenuUiObject *o = &s_objects[index];
            for (unsigned i = 0; i < (unsigned)index; ++i)
                if (s_objects[i].kind == WINDOW && ObjectFitsInside(o, &s_objects[i])) o->parent = s_objects[i].id;
        }
        x = end;
    }
}

static unsigned PartyMemberForSlot(unsigned slot) {
    if (slot < 4) return slot;
    if (slot >= 12 && slot < 16) return slot - 12;
    return slot >= 16 ? (slot - 16) / 8 : 4;
}

static void CollectSprites(const Ppu *ppu) {
    const uint8_t *state = s_latched_valid && MatchSprites(ppu, s_latched_sprites) ? s_latched_sprites : NULL;
    s_sprite_state = state;
    memset(s_object_latched, 0, sizeof s_object_latched);
    memset(s_oam_slot, 255, sizeof s_oam_slot);
    unsigned first = 0;
    /* $86:8BF5 emits these two slot ranges. */
    for (unsigned n = 0; n < 48; ++n) {
        unsigned slot = (n + 12) % 48;
        s_sprite_state = state;
        uint32_t frame = 0;
        unsigned count = SpriteByte(state, 0x11d8 + slot) ? SpritePieces(state, slot, &frame) : 0;
        bool available = first + count <= 128;
        if (available) for (unsigned p = 0; p < count; ++p) available &= s_oam_slot[first + p] == 255;
        if (!available || !SpriteMatches(ppu, state, slot, frame, first, count)) {
            /* One advanced/unmatched effect must not discard every later
             * portrait. Recover only a unique, complete original OAM run;
             * ambiguous or unknown pieces retain their original rendering. */
            int found = -1; unsigned found_count = 0; const uint8_t *found_state = NULL;
            bool ambiguous = false;
            for (unsigned snapshot = 0; snapshot < (s_latched_available ? 2u : 1u); ++snapshot) {
                const uint8_t *candidate = snapshot ? s_latched_sprites : NULL;
                if (!SpriteByte(candidate, 0x11d8 + slot)) continue;
                uint32_t candidate_frame;
                unsigned pieces = SpritePieces(candidate, slot, &candidate_frame);
                if (!pieces || pieces > 128) continue;
                for (unsigned at = 0; at + pieces <= 128; ++at) {
                    bool free = true;
                    for (unsigned p = 0; p < pieces; ++p) free &= s_oam_slot[at + p] == 255;
                    if (!free || !SpriteMatches(ppu, candidate, slot, candidate_frame, at, pieces)) continue;
                    if (found >= 0 && ((unsigned)found != at || found_count != pieces)) ambiguous = true;
                    found = (int)at; found_count = pieces; found_state = candidate;
                }
            }
            if (found < 0 || ambiguous) continue;
            first = (unsigned)found; count = found_count; s_sprite_state = found_state;
        }
        int left = WIDTH, top = HEIGHT, right = 0, bottom = 0;
        for (unsigned piece = 0; piece < count; ++piece) {
            unsigned at = first + piece;
            int x = ppu->oam[at * 2] & 255u;
            unsigned high = ppu->highOam[at / 4] >> ((at % 4) * 2);
            if (high & 1) x -= 256;
            int y = ppu->oam[at * 2] >> 8;
            unsigned size = high & 2 ? 16 : 8;
            s_oam_slot[at] = (uint8_t)slot;
            if (x < left) left = x;
            if (y < top) top = y;
            if (x + (int)size > right) right = x + size;
            if (y + (int)size > bottom) bottom = y + size;
        }
        first += count;
        if (left < 0) left = 0;
        if (top < 0) top = 0;
        if (right > WIDTH) right = WIDTH;
        if (bottom > HEIGHT) bottom = HEIGHT;
        if (right <= left || bottom <= top) continue;
        unsigned kind = slot >= 5 && slot <= 10 ? CURSOR : SPRITE;
        int index = AddObject(0x30000000u | slot, kind, left, top, right - left, bottom - top, slot);
        if (index < 0) continue;
        s_object_latched[index] = s_sprite_state != NULL;
        Lufia2MenuUiObject *o = &s_objects[index];
        unsigned best_area = UINT32_MAX;
        for (unsigned i = 0; i < (unsigned)index; ++i) {
            const Lufia2MenuUiObject *p = &s_objects[i];
            if (p->kind != SPRITE && p->kind != CURSOR && p->kind != PARTY_HIGHLIGHT && ObjectFitsInside(o, p) &&
                (unsigned)p->width * p->height < best_area) {
                o->parent = p->id;
                best_area = p->width * p->height;
            }
        }
        /* $82:999B/$99F3 keep party portraits in slots0..3. $82:B59A
         * reserves slots12..15 for their selection markers. $82:95FA
         * reserves eight item-effect slots per member, starting at16.
         * These bindings outlive a frame's changing bounds and position. */
        if (kind == SPRITE) {
            unsigned member = PartyMemberForSlot(slot);
            uint32_t group = 0x40000000u | member;
            if (member < 4 && FindObject(group) >= 0) o->parent = group;
            /* $82:F2FE/$F395 uses $82:F36B to reserve four portraits per
             * save record. Their frame bounds may cross the window edge. */
            for (unsigned record = 0; record < 4; ++record) {
                uint32_t window = SaveRecordWindow(record);
                unsigned address = 0x82f36b + record * 2;
                unsigned base = BusByte(address) | BusByte(address + 1) << 8;
                if (window && slot >= base && slot < base + 4) o->parent = window;
            }
        }
        if (kind == CURSOR) {
            int anchor_x = SpriteWord(0x1388, 0x13b8, slot);
            int row = (int)SpriteWord(0x13e8, 0x1418, slot) - 5;
            int distance = 33;
            for (unsigned i = 0; i < (unsigned)index; ++i) {
                const Lufia2MenuUiObject *text = &s_objects[i];
                int dx = text->x - anchor_x;
                if (text->kind == TEXT && row >= text->y && row < text->y + text->height &&
                    dx >= 0 && dx < distance) { o->parent = text->id; distance = dx; }
            }
        }
    }
    for (unsigned i = 0; i < s_count; ++i) {
        Lufia2MenuUiObject *o = &s_objects[i];
        if (o->kind != SPRITE || o->slot < 12 || o->id >> 28 != 3) continue;
        unsigned member = PartyMemberForSlot(o->slot);
        uint32_t portrait = 0x30000000u | member;
        if (o->parent == (0x40000000u | member) && FindObject(portrait) >= 0) o->parent = portrait;
    }
    /* Unmatched OAM remains visible and editable. */
    for (unsigned at = 0; at < 128; ++at) {
        if (s_oam_slot[at] != 255) continue;
        unsigned high = ppu->highOam[at / 4] >> ((at % 4) * 2);
        int x = (ppu->oam[at * 2] & 255) - (high & 1 ? 256 : 0);
        int y = ppu->oam[at * 2] >> 8;
        int size = high & 2 ? 16 : 8;
        int left = x < 0 ? 0 : x, top = y < 0 ? 0 : y;
        int right = x + size > WIDTH ? WIDTH : x + size;
        int bottom = y + size > HEIGHT ? HEIGHT : y + size;
        if (right <= left || bottom <= top) continue;
        s_oam_slot[at] = (uint8_t)(48 + at);
        AddObject(0x60000000u | at, SPRITE, left, top, right - left, bottom - top, 48 + at);
    }
}

static bool LayoutRecordValid(const MenuLayoutRecord *l, unsigned version, uint32_t reserved) {
    uint32_t source_extent = l->source_width | (uint32_t)l->source_height << 16;
    if (!l->scene || !l->id || l->id == l->parent || reserved ||
        l->x < (l->parent ? -CANVAS : 0) || l->y < (l->parent ? -HEIGHT : 0) ||
        !l->width || !l->height || l->x + l->width > CANVAS || l->y + l->height > HEIGHT ||
        l->kind > (version == 1 ? CURSOR : WINDOW_SOURCE) || l->visible > 1)
        return false;
    if (l->kind != TEXT_REGION && l->kind != WINDOW_SOURCE && l->kind != SPRITE_SLOT && source_extent)
        return false;

    switch (l->kind) {
    case TEXT_REGION:
        if (!l->source_width || !l->source_height ||
            l->source_width > WIDTH || l->source_height > HEIGHT)
            return false;
        if (l->id >> 28 == 10) return (l->id & 0x0fffffff) < 32 * 32;
        return l->id >> 28 == 2 && (l->id & 255) < 32 &&
            ((l->id >> 8) & 255) < 32 && !(l->id & 0x0fff0000);
    case WINDOW_SOURCE:
        return l->id >> 28 == 9 && l->scene == UINT32_MAX && !l->parent &&
            l->source_width && l->source_height && l->source_width <= WIDTH &&
            l->source_height <= HEIGHT && l->width >= 32 && l->height >= 24;
    case SPRITE_SLOT:
        return l->id >> 28 == 3 && (l->id & 0xfffffff) < 48 &&
            !l->parent && source_extent <= 3;
    case CURSOR_POINT:
        return l->id >> 28 == 7 && (l->scene != UINT32_MAX || !l->parent) &&
            ((l->id >> 16) & 255) < 36 && !(l->id & 0x0f000000) &&
            (l->id & 255) < 32 && ((l->id >> 8) & 255) < 32;
    case WINDOW:
        return l->width >= 32 && l->height >= 24;
    default:
        return true;
    }
}

bool Lufia2MenuUiLoad(const uint8_t *bytes, size_t size) {
    if (!bytes || size < LUFIA2_UI_LAYOUT_HEADER_BYTES || memcmp(bytes, "L2ML", 4) ||
        (Lufia2UiRead16(bytes + 4) != 1 && Lufia2UiRead16(bytes + 4) != 2) ||
        Lufia2UiRead16(bytes + 6) != LUFIA2_MENU_UI_RECORD || Lufia2UiRead16(bytes + 12) != CANVAS ||
        Lufia2UiRead16(bytes + 14) != HEIGHT || Lufia2UiRead32(bytes + 20)) return false;
    unsigned count = Lufia2UiRead32(bytes + 8);
    if (count > MAX_RECORDS || size != LUFIA2_UI_LAYOUT_HEADER_BYTES + count * LUFIA2_MENU_UI_RECORD ||
        Lufia2UiRead32(bytes + 16) != Lufia2UiChecksum(bytes + LUFIA2_UI_LAYOUT_HEADER_BYTES,
                                                   size - LUFIA2_UI_LAYOUT_HEADER_BYTES)) return false;
    const unsigned version = Lufia2UiRead16(bytes + 4);
    MenuLayoutRecord *candidate = malloc(count * sizeof *candidate);
    if (count && !candidate) return false;
    for (unsigned i = 0; i < count; ++i) {
        const uint8_t *r = bytes + LUFIA2_UI_LAYOUT_HEADER_BYTES + i * LUFIA2_MENU_UI_RECORD;
        candidate[i] = (MenuLayoutRecord){Lufia2UiRead32(r), Lufia2UiRead32(r + 4), Lufia2UiRead32(r + 8),
            (int16_t)Lufia2UiRead16(r + 12), (int16_t)Lufia2UiRead16(r + 14), Lufia2UiRead16(r + 16),
            Lufia2UiRead16(r + 18), Lufia2UiRead16(r + 20), Lufia2UiRead16(r + 22), Lufia2UiRead16(r + 24), Lufia2UiRead16(r + 26)};
        const MenuLayoutRecord *l = &candidate[i];
        if (!LayoutRecordValid(l, version, Lufia2UiRead32(r + 28))) { free(candidate); return false; }
        MenuLayoutRecord *entry = &candidate[i];
        if (entry->scene == MAIN_MENU_SCENE && entry->id == LEGACY_GOLD_SUFFIX &&
            entry->parent == TIME_GOLD_WINDOW && entry->kind == TEXT_REGION &&
            entry->source_width == 24 && entry->source_height == 8) {
            /* Existing layouts moved the preview's single final digit.
             * Keep that digit's destination; extend its field to the left. */
            entry->id = GOLD_TEXT;
            entry->x -= 48; entry->width += 48; entry->source_width = 72;
            if (entry->x < -CANVAS) { free(candidate); return false; }
        }
        for (unsigned j = 0; j < i; ++j)
            if (candidate[j].scene == l->scene && candidate[j].id == l->id) { free(candidate); return false; }
    }
    if (count) memcpy(s_layout, candidate, count * sizeof *candidate);
    free(candidate);
    s_layout_count = count;
    return true;
}
static void ReloadLayout(void) {
    FILE *file = s_layout_path[0] ? fopen(s_layout_path, "rb") : NULL;
    if (!file) return;
    static uint8_t bytes[LUFIA2_UI_LAYOUT_HEADER_BYTES + MAX_RECORDS * LUFIA2_MENU_UI_RECORD + 1];
    size_t size = fread(bytes, 1, sizeof bytes, file);
    bool ok = !ferror(file);
    fclose(file);
    if (ok && size <= sizeof s_last_layout &&
        (size != s_last_layout_size || memcmp(bytes, s_last_layout, size)) &&
        Lufia2MenuUiLoad(bytes, size)) {
        memcpy(s_last_layout, bytes, size);
        s_last_layout_size = size;
    }
}

static void CollectTextBindings(const Ppu *ppu) {
    for (unsigned i = 0; i < s_layout_count; ++i) {
        const MenuLayoutRecord *l = &s_layout[i];
        if (l->kind != TEXT_REGION || l->scene != s_scene || FindObject(l->id) >= 0) continue;
        unsigned tx = l->id >> 28 == 10 ? l->id & 31 : l->id & 255;
        unsigned ty = l->id >> 28 == 10 ? (l->id >> 5) & 31 : (l->id >> 8) & 255;
        int x = (int)tx * 8 - ppu->hScroll[2], y = TextRowY(ppu, ty);
        if (ty == 0 && y == -1) y = 0;
        int index = AddObject(l->id, TEXT, x, y, l->source_width, l->source_height, 255);
        if (index >= 0) s_objects[index].parent = l->parent;
    }
}

void Lufia2MenuUiInit(const uint8_t *rom, size_t size, const char *layout, const char *preview) {
    s_rom = rom; s_rom_size = size;
    snprintf(s_layout_path, sizeof s_layout_path, "%s", layout ? layout : "");
    snprintf(s_preview_path, sizeof s_preview_path, "%s", preview ? preview : "");
    s_layout_count = 0; s_frames = 0; s_active = false; s_last_layout_size = 0;
    s_latched_valid = s_latched_available = false; s_sprite_state = NULL;
}
void Lufia2MenuUiReset(Ppu *ppu) {
    s_active = false; s_rows = 0; s_move_highlight = false;
    if (ppu) {
        PpuClearOverlayCaptures(ppu);
        PpuClearOverlayBindings(ppu);
    }
}
void Lufia2MenuUiPrepare(Ppu *ppu, bool enabled, unsigned width) {
    Lufia2MenuUiReset(ppu);
    if (!ppu || !enabled || width <= WIDTH || width > kPpuBufWidth || !s_rom ||
        PPU_mode(ppu) != 1 || PPU_bgTilemapAdr(ppu, 0) != 0 ||
        PPU_bgTilemapAdr(ppu, 1) != 0x400 || PPU_bgTilemapAdr(ppu, 2) != 0x800 ||
        PPU_bgTileAdr(ppu, 0) != 0x4000 || PPU_bgTileAdr(ppu, 2) != 0x6000 ||
        (ppu->obsel >> 5) != 0 || PPU_forcedBlank(ppu)) return;
    s_count = 0; s_width = width;
    s_layout_coordinates = false;
    s_text_hscroll = ppu->hScroll[2]; s_text_vscroll = ppu->vScroll[2];
    ReadTextRowScroll();
    CollectWindows(ppu);
    if (!s_count || s_count == LUFIA2_MENU_UI_MAX_OBJECTS) return;
    s_scene = 2166136261u;
    s_theme = 0;
    const uint16_t *text = PpuRenderVram(ppu) + 0x800;
    for (unsigned i = 0; i < s_count; ++i) {
        const Lufia2MenuUiObject *o = &s_objects[i];
        if (o->kind != WINDOW) continue;
        s_scene = HashSceneWord(HashSceneWord(HashSceneWord(s_scene, o->id), o->width), o->height);
        if (o->width == 120 && o->height == 80) s_theme = 4;
        if (o->width == 184 && o->height == 64) s_theme = 1;
        /* Short headings distinguish identical list windows. */
        if (o->height == 24 && o->width <= 80) {
            char heading[16] = {0};
            unsigned letters = 0;
            unsigned row = ((o->id >> 8) & 255u) + 1;
            unsigned first_column = (o->x + ppu->hScroll[2]) / 8;
            bool text_in_row = false;
            if (row < 32) for (unsigned x = 2; x + 2 < (unsigned)o->width / 8; ++x)
                text_in_row |= HasGlyph(text[row * 32 + first_column + x]);
            if (!text_in_row) ++row; /* ordinary $80:8DB3 character row */
            for (unsigned x = 2; x + 2 < (unsigned)o->width / 8; ++x) {
                unsigned column = (o->x + ppu->hScroll[2]) / 8 + x;
                if (row < 32 && column < 32) {
                    unsigned tile = text[row * 32 + column] & 1023;
                    s_scene = HashSceneWord(s_scene, tile);
                    if (tile >= 'A' && tile <= 'Z' && letters < sizeof heading - 1)
                        heading[letters++] = (char)tile;
                }
            }
            if (!strcmp(heading, "ITEM")) s_theme = 2;
            if (!strcmp(heading, "SPELL")) s_theme = 3;
            if (!strcmp(heading, "STATUS")) s_theme = 5;
        }
    }
    if (!s_scene) s_scene = 1;
    ReloadLayout();
    CollectPartyFields(ppu);
    CollectPartyHighlights(ppu);
    CollectListSlots(ppu);
    CollectNumericFields(ppu);
    CollectTextBindings(ppu);
    CollectTextRuns(ppu);
    CollectSprites(ppu);
    if (s_count == LUFIA2_MENU_UI_MAX_OBJECTS) return;
    for (unsigned i = 0; i < s_count; ++i)
        if (FindLayoutOverride(i)) s_layout_coordinates = true;
    for (unsigned i = 0; i < s_layout_count; ++i)
        if (s_layout[i].scene == s_scene && s_layout[i].kind == CURSOR_POINT)
            s_layout_coordinates = true;
    for (unsigned i = 0; i < s_count; ++i) if (s_objects[i].kind == PARTY_HIGHLIGHT) {
        MenuDestination d = ResolveObjectDestination(i, 0);
        const Lufia2MenuUiObject *o = &s_objects[i];
        if (d.x != o->x + (int)(width - WIDTH) / 2 || d.y != o->y ||
            d.width != o->width || d.height != o->height || !d.visible) s_move_highlight = true;
    }
    if (s_move_highlight) memset(s_highlight, 0, sizeof s_highlight);
    memset(s_planes, 0, sizeof s_planes);
    memset(s_priority, 0, sizeof s_priority);
    memset(s_sprite_owner, 255, sizeof s_sprite_owner);
    memset(s_coverage, 0, sizeof s_coverage);
    memset(s_text_owner, 255, sizeof s_text_owner);
    for (unsigned i = 0; i < s_count; ++i) {
        const Lufia2MenuUiObject *o = &s_objects[i];
        if (o->kind == GROUP || o->kind == PARTY_HIGHLIGHT) continue;
        unsigned plane = o->kind == WINDOW ? 0 : o->kind == TEXT ? 1 : 2;
        for (unsigned y = 0; y < o->height; ++y) {
            memset(&s_coverage[plane][o->y + y][o->x], 1, o->width);
            if (plane == 1) for (unsigned x = 0; x < o->width; ++x)
                s_text_owner[o->y + y][o->x + x] = (uint16_t)i;
        }
    }
    /* Edited regions own their original pixels. */
    for (unsigned r = 0; r < s_layout_count; ++r) {
        const MenuLayoutRecord *l = &s_layout[r];
        if (l->scene != s_scene || l->kind != TEXT_REGION) continue;
        int index = FindObject(l->id);
        if (index < 0) continue;
        const Lufia2MenuUiObject *o = &s_objects[index];
        for (unsigned y = 0; y < o->height; ++y) for (unsigned x = 0; x < o->width; ++x)
            s_text_owner[o->y + y][o->x + x] = (uint16_t)index;
    }
    memset(s_line_owner, 255, sizeof s_line_owner);
    static const PpuOverlaySource sources[] = {kPpuOverlaySource_Bg1, kPpuOverlaySource_Bg3, kPpuOverlaySource_Obj};
    for (unsigned i = 0; i < 3; ++i) {
        PpuBindOverlaySurface(ppu, sources[i], (uint8_t *)s_planes[i], WIDTH * 4);
        PpuSetOverlayCapture(ppu, sources[i], 0, 0, WIDTH, HEIGHT, kPpuOverlayFlag_RemoveFromGame);
    }
    s_active = true;
}
void Lufia2MenuUiSpritePixel(int x, unsigned oam) {
    if (s_active && !s_rendering && x >= 0 && x < WIDTH && oam < 128) s_line_owner[x] = s_oam_slot[oam];
}
void Lufia2MenuUiPlane(Ppu *ppu, unsigned layer, unsigned line) {
    if (!s_active || s_rendering || !line || line > HEIGHT || (layer != 0 && layer != 2)) return;
    unsigned plane = layer == 0 ? 0 : 1;
    for (unsigned x = 0; x < WIDTH; ++x)
        s_priority[plane][line - 1][x] = (uint16_t)ppu->overlayBuffers[layer].data[x + kPpuExtraLeftRight];
}
void Lufia2MenuUiLine(Ppu *ppu, unsigned line, Lufia2MenuUiRenderer *draw) {
    if (!s_active || s_rendering || !line || line > HEIGHT) return;
    static const unsigned layers[] = {0, 2, 4};
    for (unsigned x = 0; x < WIDTH; ++x)
        s_priority[2][line - 1][x] = (uint16_t)ppu->overlayBuffers[4].data[x + kPpuExtraLeftRight];
    for (unsigned x = 0; x < WIDTH; ++x)
        s_background_priority[line - 1][x] = (uint16_t)ppu->bgBuffers[0].data[x + kPpuExtraLeftRight];
    memcpy(s_sprite_owner[line - 1], s_line_owner, WIDTH);
    for (unsigned x = 0; x < WIDTH; ++x) if (s_line_owner[x] == 255) {
        s_planes[2][line - 1][x] = 0;
        s_priority[2][line - 1][x] = 0;
    }
    memset(s_line_owner, 255, sizeof s_line_owner);
    /* Keep native colour math on isolated primitives. */
    if (draw && (PPU_mathEnabled(ppu) || ppu->wbgobjlog || ppu->windowsel)) {
        static Ppu original;
        static uint32_t frame[kPpuBufWidth * HEIGHT];
        for (unsigned pass = 0; pass < 4; ++pass) {
            original = *ppu;
            PpuClearOverlayCaptures(&original);
            PpuClearOverlayBindings(&original);
            original.renderBuffer = (uint8_t *)frame;
            original.renderPitch = s_width * 4;
            original.screenEnabled[0] &= (uint8_t)(2 | (pass < 3 ? 1u << layers[pass] : 0));
            original.widescreenLineEnhancer = NULL;
            s_rendering = true;
            draw(&original, line);
            s_rendering = false;
            const uint32_t *row = frame + (line - 1) * s_width;
            if (pass == 3) {
                memcpy(ppu->renderBuffer + (line - 1) * ppu->renderPitch, row, s_width * 4);
            } else for (unsigned x = 0; x < WIDTH; ++x)
                if (s_planes[pass][line - 1][x])
                    s_planes[pass][line - 1][x] = row[x + ppu->extraLeftRight] | 0xff000000u;
        }
    }
    if (draw && s_move_highlight) {
        static Ppu backdrop;
        static uint16_t clean[0x8000];
        static uint32_t frame[kPpuBufWidth * HEIGHT];
        /* Draw both palette versions through the same original renderer.
         * Only the host clone sees the cleaned tile map; guest VRAM, CPU
         * state, and the source selection remain untouched. */
        memcpy(clean, PpuRenderVram(ppu), sizeof clean);
        for (unsigned i = 0; i < s_count; ++i) {
            const Lufia2MenuUiObject *o = &s_objects[i];
            if (o->kind != PARTY_HIGHLIGHT) continue;
            unsigned address = 0x8ee576 + o->slot * 2;
            unsigned tile = (BusByte(address) | (unsigned)BusByte(address + 1) << 8) / 2;
            for (unsigned y = 0; y < 5; ++y) for (unsigned x = 0; x < 15; ++x) {
                unsigned at = PPU_bgTilemapAdr(ppu, 1) + tile + y * 32 + x;
                clean[at] = (clean[at] & ~0x1c00u) | 0x0c00;
            }
        }
        for (unsigned pass = 0; pass < 2; ++pass) {
            backdrop = *ppu;
            PpuClearOverlayCaptures(&backdrop); PpuClearOverlayBindings(&backdrop);
            backdrop.renderBuffer = (uint8_t *)frame;
            backdrop.renderPitch = s_width * 4;
            backdrop.screenEnabled[0] &= 2;
            backdrop.widescreenLineEnhancer = NULL;
            backdrop.renderVram = clean;
            if (pass) {
                /* Shade the background at the destination; moving a copied
                 * background patch would break its repeating tile pattern. */
                for (unsigned tile = 0; tile < 1024; ++tile) {
                    unsigned at = PPU_bgTilemapAdr(ppu, 1) + tile;
                    if ((clean[at] & 0x1c00) == 0x0c00) clean[at] = (clean[at] & ~0x1c00u) | 0x0800;
                }
            }
            s_rendering = true; draw(&backdrop, line); s_rendering = false;
            const uint32_t *row = frame + (line - 1) * s_width;
            if (pass) memcpy(s_highlight[line - 1], row, s_width * 4);
            else memcpy(ppu->renderBuffer + (line - 1) * ppu->renderPitch, row, s_width * 4);
        }
    }
    ++s_rows;
}

static const MenuLayoutRecord *FindLayoutOverride(unsigned index) {
    for (unsigned i = 0; i < s_layout_count; ++i)
        if (s_layout[i].scene == s_scene && s_layout[i].id == s_objects[index].id &&
            ((s_layout[i].parent == s_objects[index].parent &&
              (s_layout[i].kind == s_objects[index].kind ||
               (s_layout[i].kind == TEXT_REGION && s_objects[index].kind == TEXT))) ||
             (s_layout[i].kind == SPRITE_SLOT && s_objects[index].kind == SPRITE))) return &s_layout[i];
    const Lufia2MenuUiObject *o = &s_objects[index];
    if (o->kind == WINDOW) for (unsigned i = 0; i < s_layout_count; ++i) {
        const MenuLayoutRecord *l = &s_layout[i];
        unsigned tile = l->id & 1023;
        if (l->kind == WINDOW_SOURCE && l->scene == UINT32_MAX &&
            o->id == (0x10000000u | (tile / 32) << 8 | (tile % 32)) &&
            o->width == l->source_width && o->height == l->source_height) return l;
    }
    return NULL;
}
static int SourceLayoutX(const Lufia2MenuUiObject *o) {
    if (!s_layout_coordinates) return o->x;
    if (o->kind == WINDOW) return (o->id & 255u) * 8;
    if (o->kind == TEXT || o->kind == GROUP) return o->x + s_text_hscroll;
    return o->x;
}
static int SourceLayoutY(const Lufia2MenuUiObject *o) {
    if (!s_layout_coordinates) return o->y;
    /* Layouts use original tile positions. Their source captures use the PPU
     * scroll (normally $03FE), which moves window borders by two pixels.
     * Resolve the whole parent graph in one coordinate system; HDMA text and
     * OBJ positions already have independent original screen coordinates. */
    if (o->kind == WINDOW) {
        int y = (int)((o->id >> 8) & 255u) * 8 - 1;
        return y < 0 ? 0 : y;
    }
    if (!s_text_rows && (o->kind == TEXT || o->kind == GROUP))
        return o->y + (int8_t)s_text_vscroll;
    return o->y;
}
static void ResolveCursorPoint(unsigned index, MenuDestination *target, unsigned depth) {
    const Lufia2MenuUiObject *o = &s_objects[index];
    if (o->kind != CURSOR || o->slot < 5 || o->slot > 10) return;
    const uint8_t *state = s_object_latched[index] ? s_latched_sprites : NULL;
    unsigned cursor = o->slot - 5;
    for (unsigned grid = 0; grid < 36; ++grid) {
        uint32_t base = 0xa6f518u + grid * 7;
        unsigned columns = BusByte(base + 2), rows = BusByte(base + 3);
        if (grid == 7 && SpriteByte(state, 0x14f1 + cursor) == 2) columns = 2; /* spell list */
        if (!columns || !rows || SpriteByte(state, 0x14cd + cursor) != BusByte(base) ||
            SpriteByte(state, 0x14df + cursor) != BusByte(base + 1) ||
            SpriteByte(state, 0x14fd + cursor) != BusByte(base + 4) ||
            SpriteByte(state, 0x1503 + cursor) != BusByte(base + 5) ||
            SpriteByte(state, 0x14c1 + cursor) != BusByte(base + 6)) continue;
        unsigned column = SpriteByte(state, 0x14d9 + cursor), row = SpriteByte(state, 0x14eb + cursor);
        if (BusByte(base + 6) & 1) row %= rows;
        if (column >= columns || row >= rows) continue;
        uint32_t id = 0x70000000u | grid << 16 | row << 8 | column;
        for (unsigned scope = 0; scope < 2; ++scope)
            for (unsigned i = 0; i < s_layout_count; ++i) {
                const MenuLayoutRecord *l = &s_layout[i];
                if (l->kind != CURSOR_POINT || l->id != id ||
                    l->scene != (scope ? UINT32_MAX : s_scene)) continue;
                MenuDestination anchor = {l->x + ((int)s_width - CANVAS) / 2, l->y, 0, 0, l->visible != 0};
                if (l->parent) {
                    int parent = FindObject(l->parent);
                    if (parent < 0 || s_objects[parent].kind == CURSOR || depth >= 8) continue;
                    MenuDestination p = ResolveObjectDestination(parent, depth + 1);
                    anchor.x = p.x + l->x; anchor.y = p.y + l->y;
                    anchor.visible &= p.visible;
                }
                target->x = o->x + anchor.x - (BusByte(base) + column * BusByte(base + 4));
                target->y = o->y + anchor.y - (BusByte(base + 1) + row * BusByte(base + 5));
                target->visible &= anchor.visible;
                return;
            }
    }
}
static MenuDestination ResolveObjectDestination(unsigned index, unsigned depth) {
    const Lufia2MenuUiObject *o = &s_objects[index];
    MenuDestination d = {SourceLayoutX(o) + (int)(s_width - WIDTH) / 2, SourceLayoutY(o), o->width, o->height, true};
    if (depth > 8) return d;
    const MenuLayoutRecord *l = o->kind == CURSOR ? NULL : FindLayoutOverride(index);
    if (l && l->kind == SPRITE_SLOT)
        return (MenuDestination){l->x + ((int)s_width - CANVAS) / 2, l->y,
            l->source_width & 1 ? o->width : l->width,
            l->source_width & 2 ? o->height : l->height, l->visible != 0};
    int parent = FindObject(o->parent);
    if (parent >= 0) {
        MenuDestination p = ResolveObjectDestination(parent, depth + 1);
        d.x = p.x + SourceLayoutX(o) - SourceLayoutX(&s_objects[parent]);
        d.y = p.y + SourceLayoutY(o) - SourceLayoutY(&s_objects[parent]);
        d.visible = p.visible;
        if (o->kind == PARTY_HIGHLIGHT) {
            d.width = p.width;
            d.height = p.height > 1 ? p.height - 1 : 1;
        }
        if (l) {
            d.x = p.x + l->x; d.y = p.y + l->y;
            d.width = l->width; d.height = l->height; d.visible &= l->visible != 0;
        }
    } else if (l) {
        d.x = l->x + ((int)s_width - CANVAS) / 2; d.y = l->y;
        d.width = l->width; d.height = l->height; d.visible = l->visible != 0;
    }
    ResolveCursorPoint(index, &d, depth);
    return d;
}
static unsigned FrameSliceCoordinate(unsigned x, unsigned target, unsigned source) {
    unsigned edge = source < 32 ? source / 2 : 16;
    if (x < edge) return x;
    if (x >= target - edge) return source - (target - x);
    return edge + (x - edge) % (source > edge * 2 ? source - edge * 2 : 1);
}
static uint32_t SampleObjectPixel(unsigned index, unsigned x, unsigned y, uint16_t *priority) {
    const Lufia2MenuUiObject *o = &s_objects[index];
    if (o->kind == PARTY_HIGHLIGHT) {
        if (!s_move_highlight) return 0;
        int dx = s_targets[index].x + x, dy = s_targets[index].y + y;
        if (dx < 0 || dx >= (int)s_width || dy < 0 || dy >= HEIGHT) return 0;
        *priority = s_background_priority[o->y][o->x];
        return s_highlight[dy][dx];
    }
    unsigned plane = o->kind == WINDOW ? 0 : o->kind == TEXT ? 1 : 2;
    unsigned sx = x, sy = y;
    if (o->kind == WINDOW) {
        sx = FrameSliceCoordinate(x, s_targets[index].width, o->width);
        sy = FrameSliceCoordinate(y, s_targets[index].height, o->height);
    }
    if (sx >= o->width || sy >= o->height) return 0;
    sx += o->x; sy += o->y;
    if (sx >= WIDTH || sy >= HEIGHT) return 0;
    if (plane == 2 && s_sprite_owner[sy][sx] != o->slot) return 0;
    if (plane == 1 && s_text_owner[sy][sx] != index) return 0;
    *priority = s_priority[plane][sy][sx];
    if (*priority <= s_background_priority[sy][sx]) return 0;
    return s_planes[plane][sy][sx];
}

static bool PublishPreview(const char *temporary, const char *path) {
#ifdef _WIN32
    return MoveFileExA(temporary, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return rename(temporary, path) == 0;
#endif
}
static void ExportPreview(const uint8_t *scene) {
    if (!s_preview_path[0] || ++s_frames % 15 != 1) return;
    char temporary[1060];
    snprintf(temporary, sizeof temporary, "%s.tmp", s_preview_path);
    FILE *file = fopen(temporary, "wb");
    if (!file) return;
    unsigned exported = 0;
    for (unsigned i = 0; i < s_count; ++i) exported += s_objects[i].kind != PARTY_HIGHLIGHT;
    uint8_t header[32] = {'L', '2', 'M', 'P'};
    Lufia2UiWrite32(header + 4, 1); Lufia2UiWrite32(header + 8, s_scene); Lufia2UiWrite32(header + 12, s_width);
    Lufia2UiWrite32(header + 16, HEIGHT); Lufia2UiWrite32(header + 20, exported); Lufia2UiWrite32(header + 24, s_frames);
    Lufia2UiWrite32(header + 28, s_theme);
    bool ok = fwrite(header, 1, 32, file) == 32;
    for (unsigned i = 0; i < s_count && ok; ++i) {
        const Lufia2MenuUiObject *o = &s_objects[i];
        if (o->kind == PARTY_HIGHLIGHT) continue;
        uint8_t r[32] = {0};
        Lufia2UiWrite32(r, o->id); Lufia2UiWrite32(r + 4, o->parent);
        Lufia2UiWrite32(r + 8, (unsigned)o->x | (unsigned)o->y << 16);
        Lufia2UiWrite32(r + 12, o->width | (unsigned)o->height << 16);
        Lufia2UiWrite32(r + 16, o->kind); Lufia2UiWrite32(r + 20, o->slot);
        ok = fwrite(r, 1, 32, file) == 32;
    }
    ok = ok && fwrite(scene, 4, s_width * HEIGHT, file) == s_width * HEIGHT;
    for (unsigned i = 0; i < s_count && ok; ++i) {
        const Lufia2MenuUiObject *o = &s_objects[i];
        if (o->kind == PARTY_HIGHLIGHT) continue;
        for (unsigned y = 0; y < o->height && ok; ++y) for (unsigned x = 0; x < o->width && ok; ++x) {
            unsigned plane = o->kind == WINDOW ? 0 : o->kind == TEXT ? 1 : 2;
            unsigned sx = o->x + x, sy = o->y + y;
            uint32_t colour = o->kind == GROUP || o->kind == PARTY_HIGHLIGHT ? 0 : s_planes[plane][sy][sx];
            if (plane == 2 && s_sprite_owner[sy][sx] != o->slot) colour = 0;
            if (s_priority[plane][sy][sx] <= s_background_priority[sy][sx]) colour = 0;
            ok = fwrite(&colour, 4, 1, file) == 1;
        }
    }
    ok = fclose(file) == 0 && ok;
    if (!ok || !PublishPreview(temporary, s_preview_path)) remove(temporary);
}

void Lufia2MenuUiCompose(uint8_t *pixels, unsigned width, unsigned height, bool authoritative) {
    if (!s_active || !pixels || width != s_width || height != HEIGHT || !s_rows) return;
    for (unsigned i = 0; i < s_count; ++i) s_targets[i] = ResolveObjectDestination(i, 0);
    static uint16_t priority[4096 * HEIGHT];
    memset(priority, 0, width * height * sizeof *priority);
    /* Preserve primitives outside the editable catalog. */
    for (unsigned plane = 0; plane < 3; ++plane) for (unsigned y = 0; y < HEIGHT; ++y)
        for (unsigned x = 0; x < WIDTH; ++x) {
            if (s_coverage[plane][y][x] && (plane != 2 || s_sprite_owner[y][x] != 255)) continue;
            unsigned z = s_priority[plane][y][x];
            uint32_t colour = s_planes[plane][y][x];
            size_t at = (size_t)y * width + (width - WIDTH) / 2 + x;
            if (colour && z > s_background_priority[y][x] && z >= priority[at]) {
                priority[at] = (uint16_t)z;
                memcpy(pixels + at * 4, &colour, 4);
            }
        }
    if (authoritative) ExportPreview(pixels);
    for (unsigned i = 0; i < s_count; ++i) {
        const Lufia2MenuUiObject *o = &s_objects[i];
        const MenuDestination *d = &s_targets[i];
        if (o->kind == GROUP || !d->visible) continue;
        unsigned draw_width = o->kind == WINDOW || o->kind == PARTY_HIGHLIGHT || d->width < o->width ? d->width : o->width;
        unsigned draw_height = o->kind == WINDOW || o->kind == PARTY_HIGHLIGHT || d->height < o->height ? d->height : o->height;
        for (unsigned y = 0; y < draw_height; ++y) for (unsigned x = 0; x < draw_width; ++x) {
            int dx = d->x + x, dy = d->y + y;
            if (dx < 0 || dx >= (int)width || dy < 0 || dy >= HEIGHT) continue;
            uint16_t z = 0;
            uint32_t colour = SampleObjectPixel(i, x, y, &z);
            size_t at = (size_t)dy * width + dx;
            if (colour && z >= priority[at]) {
                priority[at] = z;
                memcpy(pixels + at * 4, &colour, 4);
            }
        }
    }
}
unsigned Lufia2MenuUiObjects(const Lufia2MenuUiObject **objects, uint32_t *scene) {
    if (objects) *objects = s_objects;
    if (scene) *scene = s_scene;
    return s_active ? s_count : 0;
}
