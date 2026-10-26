#include "lufia2_menu_ui.h"
#include "snes/dma.h"
#include "lufia2_ppu_render_clone.h"
#include "lufia2_ppu_row_cache.h"
#include "lufia2_battle_effects.h"
#include "snes/ws_shadow.h"
#include "lufia2_ui_layout_internal.h"
#include "lufia2_ppu_host.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#undef HIBYTE
#include <windows.h>
#endif

extern uint8_t g_ram[0x20000];
extern Dma *g_dma;
extern uint8_t g_snesrecomp_last_hdmaen;

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
static bool s_native_base;
static bool s_native_primitives;
static bool s_rgb_initialization_pending;
static const Ppu *s_primitive_colour_ppu;
static const Ppu *s_cached_index_ppu;
static unsigned s_cached_index_line;
static unsigned s_primitive_colour_line;
static Lufia2MenuUiColourRenderer *s_primitive_colour_draw;
static Lufia2PpuRowCache *s_row_cache;
static bool s_row_cache_attempted;
static Lufia2MenuUiRenderer *s_row_cache_renderer;
static bool s_row_cache_enabled;
static Lufia2PpuRowCache *s_highlight_rows[2];
static bool s_highlight_rows_attempted, s_highlight_cache_enabled;
static Lufia2MenuUiRenderer *s_highlight_renderer;
/* Each variant is an immutable host view while its synchronous draw runs. */
static struct {
    uint16_t source[0x8000], pixels[2][0x8000];
    unsigned tiles[LUFIA2_MENU_UI_MAX_OBJECTS], count, map;
    const uint16_t *view;
    uint32_t writes;
    bool valid, checked;
} s_highlight_vram;
static Lufia2MenuUiObject s_objects[LUFIA2_MENU_UI_MAX_OBJECTS];
enum { OBJECT_INDEX_SIZE=4096, LAYOUT_INDEX_SIZE=8192 };
static uint16_t s_object_index[OBJECT_INDEX_SIZE],s_layout_index[LAYOUT_INDEX_SIZE];
static bool s_object_index_valid,s_layout_index_valid;
static unsigned s_object_index_count,s_layout_index_count;
static MenuDestination s_targets[LUFIA2_MENU_UI_MAX_OBJECTS];
static uint32_t s_planes[3][HEIGHT][WIDTH];
static uint16_t s_priority[3][HEIGHT][WIDTH];
static uint16_t s_background_priority[HEIGHT][WIDTH];
static uint8_t s_sprite_owner[HEIGHT][WIDTH], s_oam_slot[128], s_line_owner[WIDTH];
static uint8_t s_coverage[3][HEIGHT][WIDTH];
static uint16_t s_text_owner[HEIGHT][WIDTH];
static uint64_t s_source_region_version;
/* Text collection asks for occupancy, not the final ordered owner. Keep that
 * boolean union incrementally instead of scanning the object list per tile. */
static uint32_t s_text_occupancy[HEIGHT][WIDTH / 32];
static struct {
    Lufia2MenuUiObject objects[LUFIA2_MENU_UI_MAX_OBJECTS];
    uint32_t regions[MAX_RECORDS];
    unsigned count,region_count;
    bool valid;
    uint64_t hits,misses;
} s_source_regions;
static unsigned s_rows;
static uint64_t (*s_menu_clock)(void);
static uint64_t s_menu_prepare_us, s_menu_check_us, s_menu_resolve_us, s_menu_compile_us;
void Lufia2MenuUiSetClock(uint64_t (*clock_us)(void)) { s_menu_clock=clock_us; }
static uint64_t MenuClock(void) { return s_menu_clock?s_menu_clock():0; }
static int s_text_scroll[HEIGHT];
static bool s_text_rows, s_description_window;
static int s_text_row_y[32];
static unsigned s_text_row_vscroll;
static bool s_text_row_y_valid;
static bool s_layout_coordinates;
static bool s_move_highlight;
static uint32_t s_highlight[HEIGHT][kPpuBufWidth];
static unsigned s_text_hscroll, s_text_vscroll;
static uint8_t s_last_layout[LUFIA2_UI_LAYOUT_HEADER_BYTES + MAX_RECORDS * LUFIA2_MENU_UI_RECORD];
static size_t s_last_layout_size;
enum { SPRITE_STATE_START = 0x11d8, SPRITE_STATE_END = 0x1509 };
static uint8_t s_latched_sprites[SPRITE_STATE_END - SPRITE_STATE_START];
static uint8_t s_previous_sprites[SPRITE_STATE_END - SPRITE_STATE_START];
static bool s_latched_valid;
static bool s_latched_available;
static bool s_previous_available;
static const uint8_t *s_sprite_state;
static uint8_t s_object_snapshot[LUFIA2_MENU_UI_MAX_OBJECTS];
static uint64_t s_layout_generation;
static void MenuIndexedLayoutReset(void);
static struct {
    Lufia2MenuUiObject objects[LUFIA2_MENU_UI_MAX_OBJECTS];
    uint32_t text_occupancy[HEIGHT][WIDTH/32];
    uint16_t maps[3*1024],scroll[5],description_top;
    int text_y[32];
    uint8_t rom_tables[16],members;
    unsigned count,theme;
    uint32_t scene;
    uint64_t layout;
    bool valid, description_window;
} s_menu_structure;

static const uint8_t *SpriteSnapshot(unsigned snapshot) {
    return snapshot == 2 ? s_previous_sprites : snapshot == 1 ? s_latched_sprites : NULL;
}

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
    s_previous_available = s_latched_available;
    if (s_previous_available)
        memcpy(s_previous_sprites, s_latched_sprites, sizeof s_previous_sprites);
    s_latched_valid = s_latched_available = false;
    if (!s_rom || !ppu || PPU_mode(ppu) != 1 ||
        PPU_bgTileAdr(ppu, 0) != 0x4000 || PPU_bgTileAdr(ppu, 2) != 0x6000) {
        s_previous_available = false;
        return;
    }
    /* Effect coordinates can advance before their next OAM upload. */
    memcpy(s_latched_sprites, g_ram + SPRITE_STATE_START, sizeof s_latched_sprites);
    s_latched_available = true;
    s_latched_valid = MatchSprites(ppu, s_latched_sprites);
}
static void ReadTextRowScroll(void) {
    s_text_row_y_valid=false;
    s_text_rows = false;
    s_description_window = false;
    if (!g_dma) return;
    bool active = false;
    /* Description setup reuses F7..FB for the OBJ mask before BG3 changes.
       Read the live direct BG3VOFS descriptor, not that shared scratch. */
    for (unsigned i = 0; i < 8; ++i) {
        const DmaChannel *channel = &g_dma->channel[i];
        /* $82:FE98 publishes the description's direct WOBJSEL table.
           Its text reuses the list tilemap but is not a six-row list. */
        if ((g_snesrecomp_last_hdmaen & (1u << i)) &&
            channel->bAdr == 0x25 && channel->mode == 2 &&
            !channel->indirect && !channel->fromB &&
            channel->aBank == 0x7e && channel->aAdr == 0x81c0)
            s_description_window = true;
        if ((g_snesrecomp_last_hdmaen & (1u << i)) &&
            channel->bAdr == 0x12 && channel->mode == 2 &&
            !channel->indirect && !channel->fromB &&
            channel->aBank == 0x7e && channel->aAdr == 0x80c0)
            active = true;
    }
    if (!active) return;
    unsigned pointer = 0x80c0;
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
static int TextRowYReference(const Ppu *ppu, unsigned tile_row) {
    int source = (int)tile_row * 8 - 1;
    if (s_text_rows) for (int y = 0; y < HEIGHT; ++y)
        if (((y + s_text_scroll[y]) & 255) == (source & 255)) return y;
    return (int)(((unsigned)source + 1 - ppu->vScroll[2]) & 255u) - 1;
}
static void PrepareTextRowPositions(const Ppu *ppu) {
    for(unsigned row=0;row<32;++row)
        s_text_row_y[row]=(int)((row*8u-ppu->vScroll[2])&255u)-1;
    /* The previous lookup scanned all visible lines for every text tile.
     * Preserve its first-match rule, including wrap and nonmonotonic HDMA,
     * but derive all 32 source-row positions in one ordered scan. */
    uint32_t seen=0;
    if(s_text_rows) for(unsigned y=0;y<HEIGHT;++y) {
        const unsigned source=(y+(unsigned)s_text_scroll[y])&255u;
        if((source&7u)!=7u) continue;
        const unsigned row=((source+1u)&255u)>>3;
        const uint32_t bit=UINT32_C(1)<<row;
        if(!(seen&bit)) {seen|=bit;s_text_row_y[row]=(int)y;}
    }
    s_text_row_vscroll=ppu->vScroll[2];s_text_row_y_valid=true;
}
static int TextRowY(const Ppu *ppu,unsigned tile_row) {
    if(s_text_row_y_valid && s_text_row_vscroll==ppu->vScroll[2] && tile_row<32)
        return s_text_row_y[tile_row];
    return TextRowYReference(ppu,tile_row);
}
static bool ObjectFitsInside(const Lufia2MenuUiObject *object, const Lufia2MenuUiObject *container) {
    return object->x >= container->x && object->y >= container->y &&
           object->x + object->width <= container->x + container->width &&
           object->y + object->height <= container->y + container->height;
}
static unsigned MenuIndexHash(uint32_t id,uint32_t scene,unsigned mask) {
    uint32_t key=id^(scene*0x9e3779b9u);
    key^=key>>16;key*=0x7feb352du;key^=key>>15;
    return key&mask;
}
static void BuildObjectIndex(void) {
    memset(s_object_index,255,sizeof s_object_index);
    for(unsigned i=0;i<s_count;++i) {
        unsigned at=MenuIndexHash(s_objects[i].id,0,OBJECT_INDEX_SIZE-1);
        while(s_object_index[at]!=UINT16_MAX) {
            if(s_objects[s_object_index[at]].id==s_objects[i].id) break;
            at=(at+1)&(OBJECT_INDEX_SIZE-1);
        }
        /* Preserve the original first-match rule for duplicate IDs. */
        if(s_object_index[at]==UINT16_MAX) s_object_index[at]=(uint16_t)i;
    }
    s_object_index_count=s_count;s_object_index_valid=true;
}
static int FindObject(uint32_t id) {
    if(s_object_index_valid && s_object_index_count==s_count) {
        unsigned at=MenuIndexHash(id,0,OBJECT_INDEX_SIZE-1);
        while(s_object_index[at]!=UINT16_MAX) {
            const unsigned index=s_object_index[at];
            if(s_objects[index].id==id) return (int)index;
            at=(at+1)&(OBJECT_INDEX_SIZE-1);
        }
        return -1;
    }
    for (unsigned i = 0; i < s_count; ++i) if (s_objects[i].id == id) return (int)i;
    return -1;
}
static void BuildLayoutIndex(void) {
    memset(s_layout_index,255,sizeof s_layout_index);
    for(unsigned i=0;i<s_layout_count;++i) {
        unsigned at=MenuIndexHash(s_layout[i].id,s_layout[i].scene,LAYOUT_INDEX_SIZE-1);
        while(s_layout_index[at]!=UINT16_MAX) {
            const MenuLayoutRecord *old=s_layout+s_layout_index[at];
            if(old->id==s_layout[i].id && old->scene==s_layout[i].scene) break;
            at=(at+1)&(LAYOUT_INDEX_SIZE-1);
        }
        if(s_layout_index[at]==UINT16_MAX) s_layout_index[at]=(uint16_t)i;
    }
    s_layout_index_count=s_layout_count;s_layout_index_valid=true;
}
static const MenuLayoutRecord *FindLayout(uint32_t scene,uint32_t id) {
    if(s_layout_index_valid && s_layout_index_count==s_layout_count) {
        unsigned at=MenuIndexHash(id,scene,LAYOUT_INDEX_SIZE-1);
        while(s_layout_index[at]!=UINT16_MAX) {
            const MenuLayoutRecord *l=s_layout+s_layout_index[at];
            if(l->scene==scene && l->id==id) return l;
            at=(at+1)&(LAYOUT_INDEX_SIZE-1);
        }
        return NULL;
    }
    for(unsigned i=0;i<s_layout_count;++i)
        if(s_layout[i].scene==scene && s_layout[i].id==id) return s_layout+i;
    return NULL;
}
static const MenuLayoutRecord *FindLayoutOverride(unsigned index);
static MenuDestination ResolveObjectDestination(unsigned index, unsigned depth);
static void MarkTextRectangle(unsigned x,unsigned y,unsigned width,unsigned height) {
    const unsigned first=x/32u,last=(x+width-1u)/32u;
    const uint32_t left=UINT32_MAX<<(x&31u);
    const uint32_t right=UINT32_MAX>>(31u-((x+width-1u)&31u));
    for(unsigned row=y;row<y+height;++row) {
        if(first==last) s_text_occupancy[row][first]|=left&right;
        else {
            s_text_occupancy[row][first]|=left;
            for(unsigned word=first+1u;word<last;++word) s_text_occupancy[row][word]=UINT32_MAX;
            s_text_occupancy[row][last]|=right;
        }
    }
}
static int AddObject(uint32_t id, unsigned kind, int x, int y, unsigned w, unsigned h, unsigned slot) {
    if (!w || !h || s_count == LUFIA2_MENU_UI_MAX_OBJECTS || x < 0 || y < 0 ||
        x + w > WIDTH || y + h > HEIGHT) return -1;
    unsigned i = s_count++;
    s_objects[i] = (Lufia2MenuUiObject){id, 0, (int16_t)x, (int16_t)y,
        (uint16_t)w, (uint16_t)h, (uint8_t)kind, (uint8_t)slot};
    if(kind==TEXT) MarkTextRectangle((unsigned)x,(unsigned)y,w,h);
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
    return (unsigned)x<WIDTH && (unsigned)y<HEIGHT &&
        ((s_text_occupancy[y][(unsigned)x/32u]>>((unsigned)x&31u))&1u)!=0;
}
static void CollectDescriptionText(void) {
    if (!s_description_window) return;
    const int top = (g_ram[0x1532] | (unsigned)g_ram[0x1533] << 8) - 1;
    for (unsigned i = 0; i < s_count; ++i) {
        const Lufia2MenuUiObject *window = &s_objects[i];
        if (window->kind != WINDOW || window->width < 32 || window->height < 64 ||
            top < window->y + 6 || top >= window->y + window->height - 8) continue;
        /* Description glyphs cross tile-row boundaries. Move the original
           rendered text area intact, not independently recognised tile runs
           or inventory slots which split the first letters off a line. */
        const unsigned row = ((window->id >> 8) & 255u) + 1u;
        const unsigned column = (window->id & 255u) + 1u;
        const int index = AddObject(0x20000000u | row << 8 | column, TEXT,
            window->x + 8, window->y + 6, window->width - 16, window->height - 12, 255);
        if (index >= 0) s_objects[index].parent = window->id;
        return;
    }
}
static void CollectListSlots(const Ppu *ppu) {
    if ((s_theme != 2 && s_theme != 3) || s_description_window) return;
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
    memset(s_object_snapshot, 0, sizeof s_object_snapshot);
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
        if (available && !SpriteMatches(ppu, state, slot, frame, first, count)) {
            /* Prefer the original slot order over matching elsewhere. */
            for (unsigned snapshot = 0; snapshot < 3; ++snapshot) {
                if ((snapshot == 1 && !s_latched_available) ||
                    (snapshot == 2 && !s_previous_available)) continue;
                const uint8_t *candidate = SpriteSnapshot(snapshot);
                if (!SpriteByte(candidate, 0x11d8 + slot)) continue;
                uint32_t candidate_frame;
                unsigned pieces = SpritePieces(candidate, slot, &candidate_frame);
                if (!pieces || first + pieces > 128) continue;
                bool free = true;
                for (unsigned p = 0; p < pieces; ++p) free &= s_oam_slot[first + p] == 255;
                if (!free || !SpriteMatches(ppu, candidate, slot, candidate_frame, first, pieces)) continue;
                count = pieces; frame = candidate_frame; s_sprite_state = candidate;
                break;
            }
        }
        if (!available || !SpriteMatches(ppu, s_sprite_state, slot, frame, first, count)) {
            /* One advanced/unmatched effect must not discard every later
             * portrait. Recover only a unique, complete original OAM run;
             * ambiguous or unknown pieces retain their original rendering. */
            int found = -1; unsigned found_count = 0; const uint8_t *found_state = NULL;
            bool ambiguous = false;
            for (unsigned snapshot = 0; snapshot < 3; ++snapshot) {
                if ((snapshot == 1 && !s_latched_available) ||
                    (snapshot == 2 && !s_previous_available)) continue;
                const uint8_t *candidate = SpriteSnapshot(snapshot);
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
        s_object_snapshot[index] = s_sprite_state == s_previous_sprites ? 2 : s_sprite_state != NULL;
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
    BuildLayoutIndex();
    ++s_layout_generation;
    return true;
}
static bool s_layout_resident;
static time_t s_layout_poll;
static bool s_layout_polled;
static void ReloadLayout(void) {
    if(s_layout_resident) return;
    const time_t now = time(NULL);
    if (s_layout_polled && now == s_layout_poll) return;
    s_layout_poll = now;
    s_layout_polled = true;
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

void Lufia2MenuUiPreload(void) {ReloadLayout();s_layout_resident=true;}

static void CollectTextBindings(const Ppu *ppu) {
    for (unsigned i = 0; i < s_layout_count; ++i) {
        const MenuLayoutRecord *l = &s_layout[i];
        if (l->kind != TEXT_REGION || l->scene != s_scene || FindObject(l->id) >= 0) continue;
        if (s_description_window) {
            const int parent = FindObject(l->parent);
            /* Item/spell layouts still contain inventory-row bindings with
               the same window ID. They must not split the description. */
            if (parent >= 0 && s_objects[parent].kind == WINDOW &&
                s_objects[parent].height >= 64) continue;
        }
        unsigned tx = l->id >> 28 == 10 ? l->id & 31 : l->id & 255;
        unsigned ty = l->id >> 28 == 10 ? (l->id >> 5) & 31 : (l->id >> 8) & 255;
        int x = (int)tx * 8 - ppu->hScroll[2], y = TextRowY(ppu, ty);
        if (ty == 0 && y == -1) y = 0;
        int index = AddObject(l->id, TEXT, x, y, l->source_width, l->source_height, 255);
        if (index >= 0) s_objects[index].parent = l->parent;
    }
}

void Lufia2MenuUiInit(const uint8_t *rom, size_t size, const char *layout, const char *preview) {
    s_layout_resident=false;
    s_layout_polled=false;
    s_rom = rom; s_rom_size = size;
    snprintf(s_layout_path, sizeof s_layout_path, "%s", layout ? layout : "");
    snprintf(s_preview_path, sizeof s_preview_path, "%s", preview ? preview : "");
    s_layout_count = 0; s_frames = 0; s_active = false; s_last_layout_size = 0;
    s_menu_structure.valid=false;
    s_object_index_valid=s_layout_index_valid=false;
    s_latched_valid = s_latched_available = s_previous_available = false; s_sprite_state = NULL;
    MenuIndexedLayoutReset();
}
void Lufia2MenuUiReset(Ppu *ppu) {
    /* Clear the current frame's bindings, retaining the owned layout cache.
     * Its source objects, resolved destinations, scene, width, layout revision
     * and OAM owners are checked again before reuse. Init still invalidates it. */
    s_object_index_valid=false;
    s_native_base = false;
    s_native_primitives = false;
    s_rgb_initialization_pending=false;
    s_primitive_colour_ppu=NULL;s_primitive_colour_draw=NULL;
    s_cached_index_ppu=NULL;s_cached_index_line=0;
    s_active = false; s_rows = 0; s_move_highlight = false;
    s_text_row_y_valid=false;
    if (ppu) {
        PpuClearOverlayCaptures(ppu);
        PpuClearOverlayBindings(ppu);
    }
}
static void MenuBlitSourceRegionsChanged(void);
static void PrepareSourceRegions(void) {
    bool equal=s_source_regions.valid && s_source_regions.count==s_count;
    for(unsigned i=0;equal && i<s_count;++i) {
        const Lufia2MenuUiObject *a=s_source_regions.objects+i,*b=s_objects+i;
        equal=a->id==b->id && a->parent==b->parent && a->x==b->x && a->y==b->y &&
            a->width==b->width && a->height==b->height && a->kind==b->kind && a->slot==b->slot;
    }
    unsigned regions=0;
    for(unsigned i=0;i<s_layout_count;++i) if(s_layout[i].scene==s_scene && s_layout[i].kind==TEXT_REGION) {
        if(regions>=s_source_regions.region_count || s_source_regions.regions[regions]!=s_layout[i].id) equal=false;
        ++regions;
    }
    if(equal && regions==s_source_regions.region_count) {++s_source_regions.hits;return;}
    ++s_source_regions.misses;
    ++s_source_region_version;
    /* Exact source inputs certify both derived masks and ordered owners. */
    MenuBlitSourceRegionsChanged();
    memset(s_coverage,0,sizeof s_coverage);memset(s_text_owner,255,sizeof s_text_owner);
    for(unsigned i=0;i<s_count;++i) {
        const Lufia2MenuUiObject *o=s_objects+i;
        if(o->kind==GROUP || o->kind==PARTY_HIGHLIGHT) continue;
        const unsigned plane=o->kind==WINDOW?0:o->kind==TEXT?1:2;
        for(unsigned y=0;y<o->height;++y) {
            memset(&s_coverage[plane][o->y+y][o->x],1,o->width);
            if(plane==1) for(unsigned x=0;x<o->width;++x)
                s_text_owner[o->y+y][o->x+x]=(uint16_t)i;
        }
    }
    regions=0;
    /* Ordered edited text regions retain the original last-owner rule. */
    for(unsigned r=0;r<s_layout_count;++r) {
        const MenuLayoutRecord *l=s_layout+r;
        if(l->scene!=s_scene || l->kind!=TEXT_REGION) continue;
        s_source_regions.regions[regions++]=l->id;
        const int index=FindObject(l->id);if(index<0) continue;
        const Lufia2MenuUiObject *o=s_objects+index;
        for(unsigned y=0;y<o->height;++y) for(unsigned x=0;x<o->width;++x)
            s_text_owner[o->y+y][o->x+x]=(uint16_t)index;
    }
    memcpy(s_source_regions.objects,s_objects,s_count*sizeof *s_objects);
    s_source_regions.count=s_count;s_source_regions.region_count=regions;s_source_regions.valid=true;
}
/* Structural recognition depends on tilemaps, row projection, party count,
 * fixed ROM tables and the loaded layout. Sprite attachment remains live:
 * cursor/effect state and original/previous OAM snapshots are never cached. */
static bool MenuStructure(Ppu *ppu) {
    uint16_t scroll[5]={ppu->hScroll[0],ppu->vScroll[0],ppu->hScroll[1],ppu->vScroll[1],ppu->hScroll[2]};
    uint8_t tables[16];
    for(unsigned i=0;i<8;++i) {tables[i]=BusByte(0x82a36b+i);tables[i+8]=BusByte(0x8ee576+i);}
    const uint16_t *maps=PpuRenderVram(ppu);
    if(s_menu_structure.valid && s_menu_structure.layout==s_layout_generation &&
       s_menu_structure.members==g_ram[0x0a7a] &&
       s_menu_structure.description_window==s_description_window &&
       (!s_description_window || s_menu_structure.description_top==
            (g_ram[0x1532] | (unsigned)g_ram[0x1533] << 8)) &&
       !memcmp(s_menu_structure.scroll,scroll,sizeof scroll) &&
       !memcmp(s_menu_structure.text_y,s_text_row_y,sizeof s_text_row_y) &&
       !memcmp(s_menu_structure.rom_tables,tables,sizeof tables) &&
       snesrecomp_u16_equal(s_menu_structure.maps,maps,3*1024)) {
        s_count=s_menu_structure.count;s_scene=s_menu_structure.scene;s_theme=s_menu_structure.theme;
        memcpy(s_objects,s_menu_structure.objects,s_count*sizeof *s_objects);
        memcpy(s_text_occupancy,s_menu_structure.text_occupancy,sizeof s_text_occupancy);
        return true;
    }
    s_menu_structure.valid=false;
    CollectWindows(ppu);
    if (!s_count || s_count == LUFIA2_MENU_UI_MAX_OBJECTS) return false;
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
    CollectPartyFields(ppu);
    CollectPartyHighlights(ppu);
    CollectDescriptionText();
    CollectListSlots(ppu);
    CollectNumericFields(ppu);
    CollectTextBindings(ppu);
    CollectTextRuns(ppu);

    if(s_count==LUFIA2_MENU_UI_MAX_OBJECTS) return false;
    s_menu_structure.count=s_count;s_menu_structure.scene=s_scene;s_menu_structure.theme=s_theme;
    s_menu_structure.layout=s_layout_generation;s_menu_structure.members=g_ram[0x0a7a];
    memcpy(s_menu_structure.scroll,scroll,sizeof scroll);
    memcpy(s_menu_structure.text_y,s_text_row_y,sizeof s_text_row_y);
    memcpy(s_menu_structure.rom_tables,tables,sizeof tables);
    memcpy(s_menu_structure.maps,maps,sizeof s_menu_structure.maps);
    memcpy(s_menu_structure.objects,s_objects,s_count*sizeof *s_objects);
    memcpy(s_menu_structure.text_occupancy,s_text_occupancy,sizeof s_text_occupancy);
    s_menu_structure.description_window=s_description_window;
    s_menu_structure.description_top=(uint16_t)(g_ram[0x1532] | (unsigned)g_ram[0x1533] << 8);
    s_menu_structure.valid=true;return true;
}
static void MenuPrepare(Ppu *ppu, bool enabled, unsigned width, bool prefer_indices) {
    Lufia2MenuUiReset(ppu);
    Lufia2PpuRowCacheBegin(s_row_cache);
    const char *cache_switch = getenv("SNESRECOMP_MENU_ROW_CACHE");
    s_row_cache_enabled = !cache_switch || *cache_switch != '0';
    for (unsigned i = 0; i < 2; ++i) Lufia2PpuRowCacheBegin(s_highlight_rows[i]);
    s_highlight_vram.checked = false;
    const char *highlight_switch = getenv("SNESRECOMP_MENU_HIGHLIGHT_CACHE");
    const char *reference_switch = getenv("LUFIA2_PPU_HOST_REFERENCE");
    s_highlight_cache_enabled = s_row_cache_enabled &&
        (!highlight_switch || *highlight_switch != '0') &&
        (!reference_switch || *reference_switch == '0');
    if (!ppu || !enabled || width <= WIDTH || width > kPpuBufWidth || !s_rom ||
        PPU_mode(ppu) != 1 || PPU_bgTilemapAdr(ppu, 0) != 0 ||
        PPU_bgTilemapAdr(ppu, 1) != 0x400 || PPU_bgTilemapAdr(ppu, 2) != 0x800 ||
        PPU_bgTileAdr(ppu, 0) != 0x4000 || PPU_bgTileAdr(ppu, 2) != 0x6000 ||
        (ppu->obsel >> 5) != 0 || PPU_forcedBlank(ppu)) return;
    s_count = 0; s_width = width;
    memset(s_text_occupancy,0,sizeof s_text_occupancy);
    s_layout_coordinates = false;
    s_text_hscroll = ppu->hScroll[2]; s_text_vscroll = ppu->vScroll[2];
    ReadTextRowScroll();
    PrepareTextRowPositions(ppu);
    ReloadLayout();
    if(!MenuStructure(ppu)) return;
    CollectSprites(ppu);
    if (s_count == LUFIA2_MENU_UI_MAX_OBJECTS) return;
    BuildObjectIndex();
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
    s_rgb_initialization_pending=prefer_indices && lufia2_ppu_host_costs_enabled;
    if(!s_rgb_initialization_pending) memset(s_planes, 0, sizeof s_planes);
    memset(s_priority, 0, sizeof s_priority);
    memset(s_sprite_owner, 255, sizeof s_sprite_owner);
    PrepareSourceRegions();
    memset(s_line_owner, 255, sizeof s_line_owner);
    static const PpuOverlaySource sources[] = {kPpuOverlaySource_Bg1, kPpuOverlaySource_Bg3, kPpuOverlaySource_Obj};
    for (unsigned i = 0; i < 3; ++i) {
        PpuBindOverlaySurface(ppu, sources[i], (uint8_t *)s_planes[i], WIDTH * 4);
        PpuSetOverlayCapture(ppu, sources[i], 0, 0, WIDTH, HEIGHT, kPpuOverlayFlag_RemoveFromGame);
    }
    s_active = true;
}
void Lufia2MenuUiPrepare(Ppu *ppu, bool enabled, unsigned width) {
    bool prefer_indices=false;
    Lufia2MenuUiPrepareCapture(ppu,enabled,width,prefer_indices);
}
void Lufia2MenuUiPrepareCapture(Ppu *ppu,bool enabled,unsigned width,bool prefer_indices) {
    const uint64_t begin=MenuClock();
    MenuPrepare(ppu,enabled,width,prefer_indices);
    s_menu_prepare_us+=MenuClock()-begin;
}
void Lufia2MenuUiBeforePixelLine(const Ppu *ppu) {
    if(s_active && !s_rendering && s_rgb_initialization_pending &&
        !Lufia2MenuUiPrimitiveCaptureActive(ppu)) {
        memset(s_planes,0,sizeof s_planes);
        s_rgb_initialization_pending=false;
    }
}
void Lufia2MenuUiSpritePixel(int x, unsigned oam) {
    if (s_active && !s_rendering && x >= 0 && x < WIDTH && oam < 128) s_line_owner[x] = s_oam_slot[oam];
}
uint16_t Lufia2MenuUiSpriteOwner(unsigned oam) {
    return s_active && !s_rendering && oam<128?s_oam_slot[oam]:255u;
}
static void MenuCopyPriority(uint16_t *out,const PpuZbufType *in) {
    if(sizeof *out==sizeof *in) memcpy(out,in,WIDTH*sizeof *out);
    else for(unsigned x=0;x<WIDTH;++x) out[x]=(uint16_t)in[x];
}
void Lufia2MenuUiPlane(Ppu *ppu, unsigned layer, unsigned line) {
    if (!s_active || s_rendering || !line || line > HEIGHT || (layer != 0 && layer != 2)) return;
    unsigned plane = layer == 0 ? 0 : 1;
    MenuCopyPriority(s_priority[plane][line-1],ppu->overlayBuffers[layer].data+kPpuExtraLeftRight);
}
bool Lufia2MenuUiCachedIndexRow(Ppu *ppu,unsigned line,const uint16_t *bg1,
    const uint16_t *bg3,const uint16_t *background) {
    if(!s_active || !Lufia2MenuUiPrimitiveCaptureActive(ppu) || !line || line>HEIGHT ||
        !bg1 || !bg3 || !background) return false;
    memcpy(s_priority[0][line-1],bg1,WIDTH*sizeof *bg1);
    memcpy(s_priority[1][line-1],bg3,WIDTH*sizeof *bg3);
    memcpy(s_background_priority[line-1],background,WIDTH*sizeof *background);
    s_cached_index_ppu=ppu;s_cached_index_line=line;return true;
}
static bool PrepareHighlightVram(const Ppu *ppu) {
    /* Borrowed views have no independent write certificate. Leave those
     * callers on the original per-line path. All core VRAM stores increment
     * vramWriteCount; compare the full live snapshot again each frame. */
    if (ppu->renderVram) return false;
    unsigned tiles[LUFIA2_MENU_UI_MAX_OBJECTS], count = 0;
    unsigned map = PPU_bgTilemapAdr(ppu, 1);
    if (map + 1023u >= 0x8000u) return false;
    for (unsigned i = 0; i < s_count; ++i) {
        const Lufia2MenuUiObject *o = &s_objects[i];
        if (o->kind != PARTY_HIGHLIGHT) continue;
        unsigned address = 0x8ee576 + o->slot * 2;
        unsigned tile = (BusByte(address) | (unsigned)BusByte(address + 1) << 8) / 2;
        if (map + tile + 4u * 32u + 14u >= 0x8000u) return false;
        tiles[count++] = tile;
    }
    const uint16_t *view = PpuRenderVram(ppu);
    bool changed = !s_highlight_vram.valid;
    if (!s_highlight_vram.checked || s_highlight_vram.view != view ||
        s_highlight_vram.writes != ppu->vramWriteCount) {
        changed |= !snesrecomp_u16_equal(s_highlight_vram.source, view, 0x8000);
        if (changed) memcpy(s_highlight_vram.source, view, sizeof s_highlight_vram.source);
        s_highlight_vram.checked = true;
        s_highlight_vram.view = view;
        s_highlight_vram.writes = ppu->vramWriteCount;
    }
    changed |= count != s_highlight_vram.count || map != s_highlight_vram.map ||
        memcmp(tiles, s_highlight_vram.tiles, count * sizeof *tiles) != 0;
    if (changed) {
        uint16_t *normal = s_highlight_vram.pixels[0];
        memcpy(normal, s_highlight_vram.source, sizeof s_highlight_vram.source);
        for (unsigned i = 0; i < count; ++i)
            for (unsigned y = 0; y < 5; ++y) for (unsigned x = 0; x < 15; ++x) {
                unsigned at = map + tiles[i] + y * 32 + x;
                normal[at] = (normal[at] & ~0x1c00u) | 0x0c00;
            }
        uint16_t *shaded = s_highlight_vram.pixels[1];
        memcpy(shaded, normal, sizeof s_highlight_vram.pixels[0]);
        for (unsigned tile = 0; tile < 1024; ++tile) {
            unsigned at = map + tile;
            if ((shaded[at] & 0x1c00) == 0x0c00)
                shaded[at] = (shaded[at] & ~0x1c00u) | 0x0800;
        }
        memcpy(s_highlight_vram.tiles, tiles, count * sizeof *tiles);
        s_highlight_vram.count = count;
        s_highlight_vram.map = map;
        s_highlight_vram.valid = true;
        /* These host views can change without a guest VRAM write. Recheck
         * their owned snapshots, retaining rows whose recorded memory read
         * sets are unchanged. Clearing every row here also discarded BG2
         * results after unrelated menu text or OBJ uploads. */
        for (unsigned i = 0; i < 2; ++i) if (s_highlight_rows[i]) {
            const char *reference=getenv("LUFIA2_MENU_HIGHLIGHT_FLUSH_REFERENCE");
            if(reference && *reference!='0')
                memset(s_highlight_rows[i]->valid,0,sizeof s_highlight_rows[i]->valid);
            Lufia2PpuRowCacheBegin(s_highlight_rows[i]);
        }
    }
    return true;
}


bool Lufia2MenuUiBackgroundRedrawn(const Ppu *ppu,unsigned line) {
#if SNESRECOMP_TRACE
    /* Retain the reference colour-window callback order for tracing. */
    (void)ppu;(void)line;return false;
#else
    return ppu && line && line<=HEIGHT && PPU_mode(ppu)==1 &&
        s_active && !s_rendering && !Lufia2BattleEffectsActive(ppu) &&
        !ppu->widescreenLineEnhancer && !ppu->wsMode2CaptureLayer && !ppu->mode7Hd.pixels &&
        s_width==256u+2u*ppu->extraLeftRight && ppu->extraLeftCur==ppu->extraLeftRight &&
        ppu->extraRightCur==ppu->extraLeftRight &&
        (s_native_base || s_move_highlight || PPU_mathEnabled(ppu) || ppu->wbgobjlog || ppu->windowsel);
#endif
}

bool Lufia2MenuUiNativeBaseEligible(const Ppu *ppu) {
#if SNESRECOMP_TRACE || !LUFIA2_PPU_HOST_PATCH
    (void)ppu;return false;
#else
    if(!ppu || !lufia2_ppu_host_costs_enabled || PPU_mode(ppu)!=1 || !s_active ||
        s_rendering || s_preview_path[0] || Lufia2BattleEffectsActive(ppu) ||
        ppu->widescreenLineEnhancer || ppu->renderVram || ppu->wsMode2CaptureLayer ||
        ppu->mode7Hd.pixels || s_width!=256u+2u*ppu->extraLeftRight ||
        ppu->extraLeftCur!=ppu->extraLeftRight || ppu->extraRightCur!=ppu->extraLeftRight) return false;
    for(unsigned source=0;source<kPpuOverlaySource_Count;++source) {
        const int plane=source==0?0:source==2?1:source==4?2:-1;
        if(plane<0) {
            if(ppu->overlayRenderBuffer[source] || ppu->overlayCaptures[source].flags) return false;
            continue;
        }
        const PpuOverlayCapture *c=ppu->overlayCaptures+source;
        if(ppu->overlayRenderBuffer[source]!=(const uint8_t *)s_planes[plane] ||
            ppu->overlayRenderPitch[source]!=WIDTH*4u || c->x0 || c->y0 ||
            c->x1!=WIDTH || c->y1!=HEIGHT || c->flags!=kPpuOverlayFlag_RemoveFromGame ||
            (source==4 && (c->oamFirst || c->oamCount!=128))) return false;
    }
    for(unsigned layer=0;layer<4;++layer) if(WsShadowLayerActive(layer)) return false;
    return true;
#endif
}
bool Lufia2MenuUiBeginNativeBase(const Ppu *ppu) {
    if(s_native_base || !Lufia2MenuUiNativeBaseEligible(ppu)) return false;
    if(s_move_highlight && !PrepareHighlightVram(ppu)) return false;
    s_native_base=true;return true;
}
const uint16_t *Lufia2MenuUiNativeBaseVram(void) {
    return s_native_base && s_move_highlight?s_highlight_vram.pixels[0]:NULL;
}
bool Lufia2MenuUiBeginNativePrimitives(const Ppu *ppu) {
    if(!s_native_base || s_move_highlight || !Lufia2MenuUiNativeBaseEligible(ppu)) return false;
    s_native_primitives=true;return true;
}
bool Lufia2MenuUiPrimitiveCaptureActive(const Ppu *ppu) {
    return ppu && s_native_primitives && !s_rendering;
}
void Lufia2MenuUiEndNativeBase(void) { s_native_base=false;s_native_primitives=false; }
bool Lufia2MenuUiRecordIndexedLine(const Ppu *ppu,unsigned line) {
    if(!Lufia2MenuUiPrimitiveCaptureActive(ppu) || !s_active || !line ||
       line>HEIGHT || s_rows+1u!=line)return false;
    memcpy(s_sprite_owner[line-1u],s_line_owner,WIDTH);
    memset(s_line_owner,255,sizeof s_line_owner);
    s_cached_index_ppu=NULL;s_cached_index_line=0;
    s_primitive_colour_ppu=NULL;s_primitive_colour_line=0;s_primitive_colour_draw=NULL;
    ++s_rows;return true;
}
uint32_t *Lufia2MenuUiSpriteCaptureRow(const Ppu *ppu,unsigned line) {
    if(!ppu || !s_active || s_rendering || s_native_primitives || !line || line>HEIGHT)
        return NULL;
    return s_planes[2][line-1];
}
bool Lufia2MenuUiPrimitiveRowsForLine(const Ppu *ppu,unsigned line,
    Lufia2MenuUiPrimitiveRows *rows) {
    if(!rows || !line || line>HEIGHT || !Lufia2MenuUiNativeBaseEligible(ppu)) return false;
    for(unsigned i=0;i<3;++i) rows->pixels[i]=s_planes[i][line-1];
    for(unsigned i=0;i<2;++i) rows->priority[i]=s_priority[i][line-1];
    return true;
}
void Lufia2MenuUiPrimitiveRenderer(const Ppu *ppu,unsigned line,Lufia2MenuUiColourRenderer *draw) {
    s_primitive_colour_ppu=ppu;s_primitive_colour_line=line;s_primitive_colour_draw=draw;
}
void Lufia2MenuUiReplayBaseLine(Ppu *ppu,unsigned line,Lufia2MenuUiRenderer *draw) {
    if(!ppu || !draw || !line || line>HEIGHT) return;
    const bool previous=s_rendering;s_rendering=true;
    draw(ppu,line);s_rendering=previous;
}

static bool s_replay_callback;
static void MenuPixelClone(Ppu *destination,const Ppu *source) {
    if(s_replay_callback && PPU_mode(source)==1 && !source->mode7Hd.pixels)
        Lufia2PpuLineClone(destination,source);
    else Lufia2PpuRenderClone(destination,source);
}
void Lufia2MenuUiLine(Ppu *ppu, unsigned line, Lufia2MenuUiRenderer *draw) {
    const bool cached_indices=s_native_primitives && s_cached_index_ppu==ppu && s_cached_index_line==line;
    s_cached_index_ppu=NULL;s_cached_index_line=0;
    Lufia2MenuUiColourRenderer *colour=s_primitive_colour_ppu==ppu && s_primitive_colour_line==line?
        s_primitive_colour_draw:NULL;
    s_primitive_colour_draw=NULL;
    s_primitive_colour_ppu=NULL;
    if (!s_active || s_rendering || !line || line > HEIGHT) return;
    static const unsigned layers[] = {0, 2, 4};
    if(s_native_primitives) {
        /* Unowned OBJ spans have no menu consumer. Resolve their index mask
         * directly, without copying a whole plane then clearing it pixel by
         * pixel or writing discarded RGB. Qualified winners/owner hooks have
         * already run on the authoritative timeline. */
        uint16_t *out=s_priority[2][line-1];
        const PpuZbufType *in=ppu->overlayBuffers[4].data+kPpuExtraLeftRight;
        for(unsigned x=0;x<WIDTH;x+=8) {
            uint32_t owners[2];memcpy(owners,s_line_owner+x,sizeof owners);
            if((owners[0]&owners[1])==UINT32_MAX) memset(out+x,0,8u*sizeof *out);
            else for(unsigned i=0;i<8;++i) out[x+i]=s_line_owner[x+i]==255?0:(uint16_t)in[x+i];
        }
    } else MenuCopyPriority(s_priority[2][line-1],ppu->overlayBuffers[4].data+kPpuExtraLeftRight);
    if(!cached_indices) MenuCopyPriority(s_background_priority[line-1],ppu->bgBuffers[0].data+kPpuExtraLeftRight);
    memcpy(s_sprite_owner[line - 1], s_line_owner, WIDTH);
    for (unsigned x = 0; !s_native_primitives && x < WIDTH; ++x) if (s_line_owner[x] == 255) {
        s_planes[2][line - 1][x] = 0;
        s_priority[2][line - 1][x] = 0;
    }
    memset(s_line_owner, 255, sizeof s_line_owner);
    if(s_native_primitives) {++s_rows;return;}
    /* Keep native colour math on isolated primitives. */
    if (draw && (PPU_mathEnabled(ppu) || ppu->wbgobjlog || ppu->windowsel)) {
        static Ppu original;
        static uint32_t frame[kPpuBufWidth * HEIGHT];
        for (unsigned pass = 0; pass < 4; ++pass) {
            /* The cleaned highlight view below replaces this entire backdrop.
             * Primitive colour passes still run; only the known pure callback
             * may omit the intermediate RGB row that has no consumer. */
            if (pass == 3 && s_replay_callback && (s_native_base || s_move_highlight)) continue;
            if (pass < 3) {
                /* Without windows, the capture already contains exact
                 * CGRAM/brightness RGB. Math on another layer (often BG2)
                 * cannot change this primitive. Window masking is resolved
                 * later than capture, so keep the original isolated pass for
                 * every configured window or colour-window clipping mode. */
                if (!(PPU_mathEnabled(ppu) & (1u << layers[pass])) && !PPU_clipMode(ppu) &&
                    !ppu->windowsel && !ppu->wbgobjlog) continue;
                const uint32_t *captured = s_planes[pass][line - 1];
                unsigned x = 0;
                while (x < WIDTH && !captured[x]) ++x;
                /* These host-only passes replace already captured primitive
                 * pixels. An empty row has no replacement destinations. Each
                 * pass uses an independent clone; the final background pass
                 * still renders the full line. */
                if (x == WIDTH) continue;
            }
            uint32_t *row = frame + (line - 1) * s_width;
            Lufia2PpuRowCache *cache = NULL;
            bool cache_allowed = s_row_cache_enabled &&
                !Lufia2BattleEffectsActive(ppu);
            for (unsigned bg = 0; cache_allowed && bg < 4; ++bg)
                cache_allowed = !WsShadowLayerActive(bg);
            if (cache_allowed) {
                if (!s_row_cache_attempted) {
                    s_row_cache_attempted = true;
                    s_row_cache = calloc(1, sizeof *s_row_cache);
                }
                cache = s_row_cache;
                if (cache && s_row_cache_renderer != draw) {
                    memset(cache->valid, 0, sizeof cache->valid);
                    s_row_cache_renderer = draw;
                }
            }
            const unsigned main = ppu->screenEnabled[0] &
                (2u | (pass < 3 ? 1u << layers[pass] : 0));
            const unsigned sub=Lufia2PpuRowSubscreen(ppu);
            const bool objects = ((main | sub) & 16u) != 0;
            const unsigned backgrounds = (main | sub) & 7u;
            if (!Lufia2PpuRowCacheReadViewLayers(cache,ppu,line,pass,row,s_width,
                    objects,backgrounds,PpuRenderVram(ppu),main)) {
                if (!(pass<3 && s_replay_callback && colour && colour(ppu,line,pass,row))) {
                    MenuPixelClone(&original, ppu);
                    PpuClearOverlayCaptures(&original);
                    PpuClearOverlayBindings(&original);
                    original.renderBuffer = (uint8_t *)frame;
                    original.renderPitch = s_width * 4;
                    original.screenEnabled[0] &= (uint8_t)(2 | (pass < 3 ? 1u << layers[pass] : 0));
                    original.widescreenLineEnhancer = NULL;
                    s_rendering = true;
                    draw(&original, line);
                    s_rendering = false;
                }
                Lufia2PpuRowCacheStore(cache, row);
            }
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
        bool prepared = s_highlight_cache_enabled && PrepareHighlightVram(ppu);
        if (!prepared) memcpy(clean, PpuRenderVram(ppu), sizeof clean);
        for (unsigned i = 0; !prepared && i < s_count; ++i) {
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
            if(!pass && s_native_base && s_replay_callback) continue;
            if (pass && !prepared) {
                /* Shade the background at the destination; moving a copied
                 * background patch would break its repeating tile pattern. */
                for (unsigned tile = 0; tile < 1024; ++tile) {
                    unsigned at = PPU_bgTilemapAdr(ppu, 1) + tile;
                    if ((clean[at] & 0x1c00) == 0x0c00) clean[at] = (clean[at] & ~0x1c00u) | 0x0800;
                }
            }
            uint32_t *row = frame + (line - 1) * s_width;
            Lufia2PpuRowCache *cache = NULL;
            bool cache_allowed = prepared && !Lufia2BattleEffectsActive(ppu);
            for (unsigned bg = 0; cache_allowed && bg < 4; ++bg)
                cache_allowed = !WsShadowLayerActive(bg);
            if (cache_allowed) {
                if (!s_highlight_rows_attempted) {
                    s_highlight_rows_attempted = true;
                    for (unsigned i = 0; i < 2; ++i)
                        s_highlight_rows[i] = calloc(1, sizeof *s_highlight_rows[i]);
                }
                if (s_highlight_renderer != draw) {
                    for (unsigned i = 0; i < 2; ++i)
                        if (s_highlight_rows[i])
                            memset(s_highlight_rows[i]->valid, 0, sizeof s_highlight_rows[i]->valid);
                    s_highlight_renderer = draw;
                }
                cache = s_highlight_rows[pass];
            }
            const unsigned main=ppu->screenEnabled[0]&2u;
            const unsigned sub=Lufia2PpuRowSubscreen(ppu);
            const bool objects=((main|sub)&16u)!=0;
            const uint16_t *view=prepared?s_highlight_vram.pixels[pass]:clean;
            if (!Lufia2PpuRowCacheReadViewLayers(cache,ppu,line,0,row,s_width,
                    objects,(main|sub)&7u,view,main)) {
                MenuPixelClone(&backdrop,ppu);
                PpuClearOverlayCaptures(&backdrop);PpuClearOverlayBindings(&backdrop);
                backdrop.renderBuffer=(uint8_t *)frame;backdrop.renderPitch=s_width*4;
                backdrop.screenEnabled[0]=(uint8_t)main;
                backdrop.widescreenLineEnhancer=NULL;backdrop.renderVram=view;
                s_rendering = true; draw(&backdrop, line); s_rendering = false;
                Lufia2PpuRowCacheStore(cache, row);
            }
            if (pass) memcpy(s_highlight[line - 1], row, s_width * 4);
            else memcpy(ppu->renderBuffer + (line - 1) * ppu->renderPitch, row, s_width * 4);
        }
    }
    ++s_rows;
}

void Lufia2MenuUiReplayLine(Ppu *ppu,unsigned line,Lufia2MenuUiRenderer *draw) {
    const bool previous=s_replay_callback;s_replay_callback=true;
    Lufia2MenuUiLine(ppu,line,draw);s_replay_callback=previous;
}
void Lufia2MenuUiRecoverPrimitiveRow(Ppu *ppu,unsigned line,Lufia2MenuUiRenderer *draw) {
    if(!ppu || !draw || !line || line>HEIGHT) return;
    uint8_t live_owner[WIDTH];memcpy(live_owner,s_line_owner,WIDTH);
    s_native_primitives=false;
    /* Recorded prefix rows are fully overwritten here. The original renderer
     * clears every remaining row before writing it; a full clear on the next
     * line would erase this correctly reconstructed prefix. */
    s_rgb_initialization_pending=false;
    const unsigned y=line-1u,brightness=PPU_brightness(ppu);
    static const unsigned sources[]={0,2,4};
    for(unsigned plane=0;plane<3;++plane) {
        memset(&ppu->overlayBuffers[sources[plane]],0,sizeof ppu->overlayBuffers[0]);
        for(unsigned x=0;x<WIDTH;++x) {
            const uint16_t z=s_priority[plane][y][x];
            ppu->overlayBuffers[sources[plane]].data[x+kPpuExtraLeftRight]=z;
            const unsigned index=z&255u,c=ppu->cgram[index];uint32_t rgb=0xff000000u;
            for(unsigned channel=0;channel<3;++channel) {
                const unsigned level=(c>>(channel*5u))&31u;
                rgb|=((level*8u+level/4u)*brightness/15u)<<(16u-channel*8u);
            }
            s_planes[plane][y][x]=index?rgb:0;
        }
    }
    for(unsigned x=0;x<WIDTH;++x)
        ppu->bgBuffers[0].data[x+kPpuExtraLeftRight]=s_background_priority[y][x];
    memcpy(s_line_owner,s_sprite_owner[y],WIDTH);
    const unsigned saved_rows=s_rows;
    Lufia2MenuUiReplayLine(ppu,line,draw);
    s_rows=saved_rows;
    memcpy(s_line_owner,live_owner,WIDTH);
}
void Lufia2MenuUiRecoverIndexedRow(Ppu *ppu,unsigned line,
    Lufia2MenuUiRenderer *indices,Lufia2MenuUiRenderer *pixels) {
    if(!ppu || !indices || !pixels || !line || line>HEIGHT)return;
    uint8_t live_owner[WIDTH];memcpy(live_owner,s_line_owner,WIDTH);
    const unsigned rows=s_rows;const bool rendering=s_rendering;
    s_rendering=false;s_native_primitives=true;
    s_cached_index_ppu=NULL;s_cached_index_line=0;
    static const PpuOverlaySource sources[]={kPpuOverlaySource_Bg1,kPpuOverlaySource_Bg3,kPpuOverlaySource_Obj};
    for(unsigned i=0;i<3;++i) {
        PpuBindOverlaySurface(ppu,sources[i],(uint8_t *)s_planes[i],WIDTH*4u);
        PpuSetOverlayCapture(ppu,sources[i],0,0,WIDTH,HEIGHT,kPpuOverlayFlag_RemoveFromGame);
    }
    /* OBJ winners were already qualified and saved on the real timeline. */
    if(ppu->lineHasSprites)ppu->overlayBuffers[kPpuOverlaySource_Obj]=ppu->objBuffer;
    else memset(&ppu->overlayBuffers[kPpuOverlaySource_Obj],0,sizeof ppu->objBuffer);
    /* The saved OBJ row includes captured menu pieces. On the real primary
     * path those pieces were removed from this centre before BG composition.
     * Keep their qualified winners in the overlay, not in the backdrop Z. */
    PpuZbufType captured[WIDTH];
    memcpy(captured,ppu->objBuffer.data+kPpuExtraLeftRight,sizeof captured);
    for(unsigned x=0;x<WIDTH;++x)ppu->objBuffer.data[x+kPpuExtraLeftRight]=0x500u;
    indices(ppu,line);
    memcpy(ppu->objBuffer.data+kPpuExtraLeftRight,captured,sizeof captured);
    memcpy(s_line_owner,s_sprite_owner[line-1u],WIDTH);
    Lufia2MenuUiLine(ppu,line,NULL);s_rows=rows;
    Lufia2MenuUiRecoverPrimitiveRow(ppu,line,pixels);
    s_rendering=rendering;memcpy(s_line_owner,live_owner,WIDTH);
}

static const MenuLayoutRecord *FindLayoutOverride(unsigned index) {
    const MenuLayoutRecord *l=FindLayout(s_scene,s_objects[index].id);
    if(l && ((l->parent==s_objects[index].parent &&
        (l->kind==s_objects[index].kind || (l->kind==TEXT_REGION && s_objects[index].kind==TEXT))) ||
        (l->kind==SPRITE_SLOT && s_objects[index].kind==SPRITE))) return l;
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
    const uint8_t *state = SpriteSnapshot(s_object_snapshot[index]);
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
        for (unsigned scope = 0; scope < 2; ++scope) {
                const MenuLayoutRecord *l=FindLayout(scope?UINT32_MAX:s_scene,id);
                if(!l || l->kind!=CURSOR_POINT) continue;
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
static uint32_t SampleObjectSource(unsigned index, unsigned x, unsigned y,
                                  uint16_t *priority, uint32_t *source) {
    const Lufia2MenuUiObject *o = &s_objects[index];
    if (o->kind == PARTY_HIGHLIGHT) {
        if (!s_move_highlight) return 0;
        int dx = s_targets[index].x + x, dy = s_targets[index].y + y;
        if (dx < 0 || dx >= (int)s_width || dy < 0 || dy >= HEIGHT) return 0;
        *priority = s_background_priority[o->y][o->x];
        if (source) *source = snesrecomp_blit_sample(3, (unsigned)dx, (unsigned)dy);
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
    if (source) *source = snesrecomp_blit_sample(plane, sx, sy);
    return s_planes[plane][sy][sx];
}
static uint32_t SampleObjectPixel(unsigned index, unsigned x, unsigned y, uint16_t *priority) {
    return SampleObjectSource(index, x, y, priority, NULL);
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

static void ComposeObjectReference(unsigned i, uint8_t *pixels, unsigned width,
                                    uint16_t *priority) {
    const Lufia2MenuUiObject *o = &s_objects[i];
    const MenuDestination *d = &s_targets[i];
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

/* Resolve clipping and nine-slice coordinates once per object/row. The source
 * pixels, owner and priority tests remain live; moving cursors and animated
 * palettes do not require reconstructing a cached full-frame picture. */
static inline void ComposeObjectOutputRows(unsigned i, uint8_t *pixels,
    uint32_t *map, unsigned width, unsigned pitch, uint16_t *priority,
    unsigned first_row, unsigned end_row) {
    const Lufia2MenuUiObject *o = &s_objects[i];
    const MenuDestination *d = &s_targets[i];
    int dw = o->kind == WINDOW || o->kind == PARTY_HIGHLIGHT || d->width < o->width ? d->width : o->width;
    int dh = o->kind == WINDOW || o->kind == PARTY_HIGHLIGHT || d->height < o->height ? d->height : o->height;
    int left = d->x < 0 ? -d->x : 0, top = d->y < 0 ? -d->y : 0;
    int right = dw < (int)width - d->x ? dw : (int)width - d->x;
    int bottom = dh < HEIGHT - d->y ? dh : HEIGHT - d->y;
    if (top < (int)first_row - d->y) top = (int)first_row - d->y;
    if (bottom > (int)end_row - d->y) bottom = (int)end_row - d->y;
    if (left >= right || top >= bottom) return;
    if (o->kind == PARTY_HIGHLIGHT) {
        if (!s_move_highlight) return;
        uint16_t z = s_background_priority[o->y][o->x];
        for (int y = top; y < bottom; ++y) for (int x = left; x < right; ++x) {
            uint32_t colour = s_highlight[d->y + y][d->x + x];
            size_t at = (size_t)(d->y + y) * width + d->x + x;
            if (colour && z >= priority[at]) {
                priority[at] = z;
                if (map) map[(size_t)(d->y + y) * pitch + d->x + x] =
                    snesrecomp_blit_sample(3, (unsigned)(d->x + x), (unsigned)(d->y + y));
                else memcpy(pixels + at * 4, &colour, 4);
            }
        }
        return;
    }
    unsigned plane = o->kind == WINDOW ? 0 : o->kind == TEXT ? 1 : 2;
    /* The visible destination span is bounded by the consumer canvas width,
     * even when an authored window extends far beyond either edge. */
    uint16_t source_x[kPpuBufWidth];
    for (int x = left; x < right; ++x) {
        unsigned sx = o->kind == WINDOW ? FrameSliceCoordinate(x, d->width, o->width) : (unsigned)x;
        source_x[x - left] = sx < o->width && sx + o->x < WIDTH ?
            (uint16_t)(sx + o->x) : UINT16_MAX;
    }
    for (int y = top; y < bottom; ++y) {
        unsigned sy = o->kind == WINDOW ? FrameSliceCoordinate(y, d->height, o->height) : (unsigned)y;
        if (sy >= o->height || sy + o->y >= HEIGHT) continue;
        sy += o->y;
        for (int x = left; x < right; ++x) {
            unsigned sx = source_x[x - left];
            if (sx == UINT16_MAX ||
                (plane == 2 && s_sprite_owner[sy][sx] != o->slot) ||
                (plane == 1 && s_text_owner[sy][sx] != i)) continue;
            uint16_t z = s_priority[plane][sy][sx];
            if (z <= s_background_priority[sy][sx]) continue;
            uint32_t colour = s_planes[plane][sy][sx];
            size_t at = (size_t)(d->y + y) * width + d->x + x;
            if (colour && z >= priority[at]) {
                priority[at] = z;
                if (map) map[(size_t)(d->y + y) * pitch + d->x + x] =
                    snesrecomp_blit_sample(plane, sx, sy);
                else memcpy(pixels + at * 4, &colour, 4);
            }
        }
    }
}

static void ComposeObject(unsigned i, uint8_t *pixels, unsigned width,
                           uint16_t *priority) {
    const Lufia2MenuUiObject *o = &s_objects[i];
    const MenuDestination *d = &s_targets[i];
    int dw = o->kind == WINDOW || o->kind == PARTY_HIGHLIGHT || d->width < o->width ? d->width : o->width;
    int dh = o->kind == WINDOW || o->kind == PARTY_HIGHLIGHT || d->height < o->height ? d->height : o->height;
    int left = d->x < 0 ? -d->x : 0, top = d->y < 0 ? -d->y : 0;
    int right = dw < (int)width - d->x ? dw : (int)width - d->x;
    int bottom = dh < HEIGHT - d->y ? dh : HEIGHT - d->y;
    if (left >= right || top >= bottom) return;
    if (o->kind == PARTY_HIGHLIGHT) {
        if (!s_move_highlight) return;
        uint16_t z = s_background_priority[o->y][o->x];
        for (int y = top; y < bottom; ++y) for (int x = left; x < right; ++x) {
            uint32_t colour = s_highlight[d->y + y][d->x + x];
            size_t at = (size_t)(d->y + y) * width + d->x + x;
            if (colour && z >= priority[at]) {
                priority[at] = z;
                memcpy(pixels + at * 4, &colour, 4);
            }
        }
        return;
    }
    unsigned plane = o->kind == WINDOW ? 0 : o->kind == TEXT ? 1 : 2;
    /* The visible destination span is bounded by the consumer canvas width,
     * even when an authored window extends far beyond either edge. */
    uint16_t source_x[kPpuBufWidth];
    for (int x = left; x < right; ++x) {
        unsigned sx = o->kind == WINDOW ? FrameSliceCoordinate(x, d->width, o->width) : (unsigned)x;
        source_x[x - left] = sx < o->width && sx + o->x < WIDTH ?
            (uint16_t)(sx + o->x) : UINT16_MAX;
    }
    for (int y = top; y < bottom; ++y) {
        unsigned sy = o->kind == WINDOW ? FrameSliceCoordinate(y, d->height, o->height) : (unsigned)y;
        if (sy >= o->height || sy + o->y >= HEIGHT) continue;
        sy += o->y;
        for (int x = left; x < right; ++x) {
            unsigned sx = source_x[x - left];
            if (sx == UINT16_MAX ||
                (plane == 2 && s_sprite_owner[sy][sx] != o->slot) ||
                (plane == 1 && s_text_owner[sy][sx] != i)) continue;
            uint16_t z = s_priority[plane][sy][sx];
            if (z <= s_background_priority[sy][sx]) continue;
            uint32_t colour = s_planes[plane][sy][sx];
            size_t at = (size_t)(d->y + y) * width + d->x + x;
            if (colour && z >= priority[at]) {
                priority[at] = z;
                memcpy(pixels + at * 4, &colour, 4);
            }
        }
    }
}

void Lufia2MenuUiCompose(uint8_t *pixels, unsigned width, unsigned height, bool authoritative) {
    if (!s_active || !pixels || width != s_width || height != HEIGHT || !s_rows) return;
    static bool configured, reference;
    if (!configured) {
        reference = getenv("LUFIA2_MENU_COMPOSE_REFERENCE") != NULL;
        configured = true;
    }
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
        if (reference) ComposeObjectReference(i, pixels, width, priority);
        else ComposeObject(i, pixels, width, priority);
    }
}
unsigned Lufia2MenuUiObjects(const Lufia2MenuUiObject **objects, uint32_t *scene) {
    if (objects) *objects = s_objects;
    if (scene) *scene = s_scene;
    return s_active ? s_count : 0;
}
static void MenuBlitStats(Lufia2MenuUiCacheStats *stats);
static void MenuPrimitiveStats(Lufia2MenuUiCacheStats *stats);
void Lufia2MenuUiGetCacheStats(Lufia2MenuUiCacheStats *stats) {
    if (!stats) return;
    *stats = (Lufia2MenuUiCacheStats){0};
    stats->prepare_us=s_menu_prepare_us;stats->geometry_check_us=s_menu_check_us;
    stats->geometry_resolve_us=s_menu_resolve_us;stats->geometry_compile_us=s_menu_compile_us;
    for (unsigned i=0;i<2;++i) if(s_highlight_rows[i]) {
        stats->highlight_hits+=s_highlight_rows[i]->hits;
        stats->highlight_misses+=s_highlight_rows[i]->misses;
    }
    MenuBlitStats(stats);
    MenuPrimitiveStats(stats);
    if (s_row_cache) {
        stats->hits = s_row_cache->hits;
        stats->misses = s_row_cache->misses;
        stats->bytes = (uint32_t)sizeof *s_row_cache;
    }
}
static bool MenuResolveBlitRows(uint32_t *map, uint32_t pitch, size_t capacity,
    SnesRecompPpuJobImage images[4], uint16_t *image_count,
    unsigned first_row, unsigned end_row) {
    if (image_count) *image_count = 0;
    if (!s_active || !map || !images || !image_count || s_width > SNESRECOMP_BLIT_MAX_WIDTH ||
        pitch < s_width || (size_t)pitch > SIZE_MAX / HEIGHT ||
        capacity < (size_t)pitch * HEIGHT || s_rows != HEIGHT ||
        s_preview_path[0] || first_row > end_row || end_row > HEIGHT) return false;
    static uint16_t priority[SNESRECOMP_BLIT_MAX_WIDTH * HEIGHT];
    memset(priority + first_row * s_width, 0, s_width * (end_row - first_row) * sizeof *priority);
    for (unsigned y = first_row; y < end_row; ++y) for (unsigned x = 0; x < s_width; ++x)
        map[(size_t)y * pitch + x] = SNESRECOMP_BLIT_EMPTY;
    for (unsigned plane = 0; plane < 3; ++plane) {
        images[plane] = (SnesRecompPpuJobImage){&s_planes[plane][0][0], WIDTH, WIDTH, HEIGHT};
        for (unsigned y = first_row; y < end_row; ++y) for (unsigned x = 0; x < WIDTH; ++x) {
            if (s_coverage[plane][y][x] && (plane != 2 || s_sprite_owner[y][x] != 255)) continue;
            const unsigned z = s_priority[plane][y][x];
            const unsigned dx = (s_width - WIDTH) / 2 + x;
            const size_t at = (size_t)y * s_width + dx;
            if (s_planes[plane][y][x] && z > s_background_priority[y][x] && z >= priority[at]) {
                priority[at] = (uint16_t)z;
                map[(size_t)y * pitch + dx] = snesrecomp_blit_sample(plane, x, y);
            }
        }
    }
    for (unsigned i = 0; i < s_count; ++i) {
        const Lufia2MenuUiObject *o = &s_objects[i];
        const MenuDestination *d = &s_targets[i];
        if (o->kind == GROUP || !d->visible) continue;
        ComposeObjectOutputRows(i, NULL, map, s_width, pitch, priority, first_row, end_row);
    }
    *image_count = s_move_highlight ? 4 : 3;
    if (s_move_highlight) images[3] = (SnesRecompPpuJobImage){&s_highlight[0][0],
        kPpuBufWidth, (uint16_t)s_width, HEIGHT};
    return true;
}
bool Lufia2MenuUiResolveBlits(uint32_t *map, uint32_t pitch, size_t capacity,
                            SnesRecompPpuJobImage images[4], uint16_t *image_count) {
    if (image_count) *image_count=0;
    if (!s_active || !map || !images || !image_count || s_width > SNESRECOMP_BLIT_MAX_WIDTH ||
        pitch < s_width || (size_t)pitch > SIZE_MAX/HEIGHT || capacity < (size_t)pitch*HEIGHT ||
        s_rows != HEIGHT || s_preview_path[0]) return false;
    for (unsigned i = 0; i < s_count; ++i) s_targets[i] = ResolveObjectDestination(i, 0);
    return MenuResolveBlitRows(map, pitch, capacity, images, image_count, 0, HEIGHT);
}
#include "lufia2_menu_blit_plan.inc"
#include "lufia2_menu_primitives.inc"
#include "lufia2_menu_indexed_layout.inc"
