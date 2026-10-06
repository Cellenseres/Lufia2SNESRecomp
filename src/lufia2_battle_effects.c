/* Sample battle art before sprites and colour math. */
#include "lufia2_battle_effects.h"

#include <stdlib.h>
#include <string.h>

#include "lufia2_battle_widescreen.h"
#include "lufia2_battle_ui.h"
#include "lufia2_margin_assets.h"

extern uint8_t g_ram[];
enum {
    BATTLE_EFFECT_SCRIPTS = 0x1bec,
    EFFECT_CLEANUP_REQUEST = 0x15b3,
    EFFECT_CLEAR_BUFFER = 0x3000,
    EFFECT_CLEAR_BYTES = 0x0800,
    EFFECT_CLEAR_MAP = 0x0800,
    EFFECT_UPLOAD_QUEUE = 0x1a8f,
    EFFECT_UPLOAD_COUNT = 16,
    EFFECT_UPLOAD_STRIDE = 6,
    EFFECT_ACTOR_FIRST = 0x54b7,
    EFFECT_ACTOR_COUNT = 64,
    EFFECT_ACTOR_STRIDE = 0x2d,
    ACTOR_ACTIVE = 0,
    ACTOR_BASE_X = 0x1b,
    ACTOR_BASE_Y = 0x1d,
    ACTOR_OFFSET_X = 0x1f,
    ACTOR_OFFSET_Y = 0x21,
    ACTOR_VISIBLE = 0x26,
    ACTOR_TILE = 0x27,
    ACTOR_PALETTE = 0x28,
    ACTOR_KIND = 0x29,
    ACTOR_SIZE = 0x2a,
    ACTOR_ATTRIBUTES = 0x2b,
    HUD_TILEMAP = 0x5c00,
    HUD_GRAPHICS = 0x4000,
    TITLE_TILEMAP = 0x5f80,
    TITLE_CORNER = 0x2160,
    TITLE_RULE = 0x2161,
    TITLE_EDGE = 0x2162,
    TITLE_GLYPHS = 0x2180,
    HUD_TEXT = 3,
    HUD_FILL = 8,
    HUD_PADDING = 9,
    HUD_PANEL_BORDER = 15,
    HUD_TITLE = 1,
    HUD_PARTY = 2,
};

static uint32_t *s_art;
static uint8_t *s_art_palette;
static uint16_t s_original_palettes[25][32];
static bool s_palettes_valid;
static bool s_palette_changed;
static unsigned s_art_width;
static unsigned s_art_height;
static unsigned s_center_left;
static unsigned s_center_right;
static uint8_t s_background;
static bool s_loaded;
static bool s_active;
static bool s_effect_clear_pending;
static bool s_uniform_palette;
static unsigned s_palette_colour;
/* Transparent pixels retain their palette colour. */
static uint32_t s_line_colours[2][kPpuBufWidth];
static uint8_t s_line_palette[2][kPpuBufWidth];
static bool s_sprite_pixels[kPpuBufWidth];
static uint8_t s_scene_registers[PPU_SAVESTATE_REGS_SIZE];
static bool s_scene_valid;
static bool s_hud_pending[224];
static uint8_t s_hud_kind[224];
static uint8_t s_hud_registers[224][PPU_SAVESTATE_REGS_SIZE];
static uint8_t s_hud_layers[224][256];
static bool s_hud_drawing;
static int s_party_top;
static struct {
    bool ready;
    unsigned left, right;
    unsigned text_left, text_right;
    unsigned source_left;
} s_title;

void Lufia2BattleEffectsInit(const uint8_t *rom, size_t size) {
    s_palettes_valid = false;
    s_loaded = false;
    if (!rom || size < 0xbfd40u + 25u * 4u)
        return;
    for (unsigned id = 0; id < 25; ++id) {
        const size_t at = 0xbcd58u + rom[0xbfd40u + id * 4u + 2u] * 64u;
        if (at + 64u > size)
            return;
        for (unsigned colour = 0; colour < 32; ++colour)
            s_original_palettes[id][colour] =
                (uint16_t)(rom[at + colour * 2u] | (rom[at + colour * 2u + 1u] << 8));
    }
    s_palettes_valid = true;
}

static unsigned ArtColour(uint32_t colour) {
    return ((colour >> 19) & 31u) | ((colour >> 6) & 0x03e0u) |
           ((colour << 7) & 0x7c00u);
}

static uint8_t NearestPalette(unsigned colour, uint8_t id) {
    unsigned best = UINT32_MAX;
    uint8_t nearest = 1;
    for (unsigned i = 1; i < 32; ++i) {
        if (i == 16)
            continue;
        unsigned distance = 0;
        for (unsigned shift = 0; shift < 15; shift += 5) {
            const int delta = (int)((colour >> shift) & 31u) -
                (int)((s_original_palettes[id][i] >> shift) & 31u);
            distance += (unsigned)(delta * delta);
        }
        if (distance < best) {
            best = distance;
            nearest = (uint8_t)i;
        }
    }
    return nearest;
}

static bool LoadArt(uint8_t id, unsigned width, unsigned height) {
    SnesRecompMarginAsset asset;
    if (s_loaded && id == s_background && width == s_art_width && height == s_art_height)
        return true;
    if (!Lufia2MarginAssetFind(LUFIA2_MARGIN_SCENE_BATTLE, id, &asset) ||
        asset.canvas_width != width || asset.canvas_height != height ||
        asset.center_width > 256u || width < 256u || width > kPpuBufWidth ||
        height > 224u || ((width - 256u) & 1u))
        return false;
    uint32_t *art = calloc((size_t)width * height, sizeof *art);
    uint8_t *palette = calloc((size_t)width * height, sizeof *palette);
    if (!art || !palette) {
        free(art);
        free(palette);
        return false;
    }
    const SnesRecompMarginComposite destination = {
        .pixels = (uint8_t *)art, .pitch = (size_t)width * 4u,
        .width = (uint16_t)width, .height = (uint16_t)height,
        .brightness = 15u,
    };
    if (!snesrecomp_margin_asset_composite_argb8888(&asset, &destination)) {
        free(art);
        free(palette);
        return false;
    }
    if (s_palettes_valid)
        for (size_t i = 0; i < (size_t)width * height; ++i)
            palette[i] = NearestPalette(ArtColour(art[i]), id);
    free(s_art);
    free(s_art_palette);
    s_art = art;
    s_art_palette = palette;
    s_art_width = width;
    s_art_height = height;
    s_center_left = asset.center_x;
    s_center_right = asset.center_x + asset.center_width;
    s_background = id;
    s_loaded = true;
    return true;
}

static bool PendingEffectClear(void) {
    if (g_ram[BATTLE_EFFECT_SCRIPTS] ||
        g_ram[EFFECT_CLEANUP_REQUEST] != 1u)
        return false;
    for (unsigned slot = 0; slot < EFFECT_UPLOAD_COUNT; ++slot) {
        const uint8_t *upload = g_ram + EFFECT_UPLOAD_QUEUE +
            slot * EFFECT_UPLOAD_STRIDE;
        if ((upload[0] | (upload[1] << 8)) != EFFECT_CLEAR_BYTES ||
            (upload[2] | (upload[3] << 8)) != EFFECT_CLEAR_BUFFER ||
            (upload[4] | (upload[5] << 8)) != EFFECT_CLEAR_MAP)
            continue;
        for (unsigned byte = 0; byte < EFFECT_CLEAR_BYTES; ++byte)
            if (g_ram[EFFECT_CLEAR_BUFFER + byte])
                return false;
        return true;
    }
    return false;
}

bool Lufia2BattleEffectsPrepare(Ppu *ppu, bool wide,
                               unsigned width, unsigned height) {
    const char *enabled = getenv("LUFIA2_BATTLE_WIDE_EFFECTS");
    uint8_t id;
    const Lufia2BattleState battle = Lufia2BattleInspect(g_ram);
    s_effect_clear_pending = battle.active && battle.display_ready &&
        ppu && PPU_mode(ppu) == 1 && !PPU_forcedBlank(ppu) &&
        PendingEffectClear();
    s_active = wide && !(enabled && strcmp(enabled, "0") == 0) &&
        Lufia2BattleWidescreenMargin(&battle, ppu, &id) &&
        PPU_mode(ppu) == 1 && LoadArt(id, width, height);
    s_scene_valid = false;
    s_title.ready = false;
    memset(s_hud_pending, 0, sizeof s_hud_pending);
    s_party_top = -1;
    Lufia2BattleUiBegin(s_active, width, height);
    return s_active;
}

bool Lufia2BattleEffectsActive(const Ppu *ppu) {
    return s_active && ppu && PPU_mode(ppu) == 1;
}

bool Lufia2BattleEffectsPlane(const Ppu *ppu, unsigned layer) {
    if (!Lufia2BattleEffectsActive(ppu) || !g_ram[BATTLE_EFFECT_SCRIPTS] || layer != 2u)
        return false;
    /* Effect uploads target these four maps. */
    const unsigned map = PPU_bgTilemapAdr(ppu, layer);
    return map >= 0x0800u && map <= 0x1400u;
}

bool Lufia2BattleEffectsPlaneCleared(const Ppu *ppu, unsigned layer) {
    /* Skip cleared effects until their queued VRAM update. */
    return s_effect_clear_pending && ppu && layer == 2u &&
        PPU_mode(ppu) == 1 &&
        PPU_bgTilemapAdr(ppu, layer) == EFFECT_CLEAR_MAP &&
        PPU_bgTileAdr(ppu, layer) == 0x1000u;
}

bool Lufia2BattleEffectsBackground(const Ppu *ppu) {
    return Lufia2BattleEffectsActive(ppu) &&
        PPU_bgTilemapAdr(ppu, 0) == 0 &&
        PPU_bgTileAdr(ppu, 0) == 0x2000u;
}

bool Lufia2BattleEffectsSprite(const Ppu *ppu, unsigned slot) {
    if (!Lufia2BattleEffectsActive(ppu) || !g_ram[BATTLE_EFFECT_SCRIPTS] || slot >= 128u)
        return false;
    /* Three actor lists contain effect sprites. */
    for (unsigned group = 0; group < 3; ++group) {
        const unsigned first = g_ram[0x15c1u + group];
        const unsigned count = g_ram[0x15deu + group * 4u];
        if (!g_ram[0x15dbu + group * 4u] || !count ||
            first >= 128u || count > 128u - first ||
            slot < first || slot - first >= count)
            continue;
        /* Match the OAM actually uploaded this frame. */
        const uint8_t *record = g_ram + 0x100u + slot * 4u;
        const unsigned high_shift = (slot & 3u) * 2u;
        return ppu->oam[slot * 2u] == (unsigned)(record[0] | (record[1] << 8)) &&
            ppu->oam[slot * 2u + 1u] == (unsigned)(record[2] | (record[3] << 8)) &&
            ((ppu->highOam[slot / 4u] ^ g_ram[0x300u + slot / 4u]) &
             (3u << high_shift)) == 0;
    }
    return false;
}

void Lufia2BattleEffectsBeginSprites(void) {
    memset(s_sprite_pixels, 0, sizeof s_sprite_pixels);
}

void Lufia2BattleEffectsSpritePixel(int x, bool effect) {
    const int at = x + kPpuExtraLeftRight;
    if (at >= 0 && at < kPpuBufWidth)
        s_sprite_pixels[at] = effect;
}

bool Lufia2BattleEffectsSpriteVisible(const Ppu *ppu, int x) {
    if (x >= 0 && x < 256)
        return true;
    const int at = x + kPpuExtraLeftRight;
    return Lufia2BattleEffectsActive(ppu) && g_ram[BATTLE_EFFECT_SCRIPTS] &&
        at >= 0 && at < kPpuBufWidth && s_sprite_pixels[at];
}

static unsigned ActorWord(const uint8_t *actor, unsigned offset) {
    return actor[offset] | (actor[offset + 1] << 8);
}

static int ActorCoordinate(const uint8_t *actor, unsigned base, unsigned offset) {
    const unsigned value = (ActorWord(actor, base) + ActorWord(actor, offset)) & 65535u;
    return value < 32768u ? (int)value : (int)value - 65536;
}

bool Lufia2BattleEffectsSpriteMargins(Ppu *ppu, const uint16_t *vram, unsigned line) {
    if (!Lufia2BattleEffectsActive(ppu) || !g_ram[BATTLE_EFFECT_SCRIPTS])
        return false;
    static const uint8_t sizes[8][2] = {
        {8, 16}, {8, 32}, {8, 64}, {16, 32},
        {16, 64}, {32, 64}, {16, 32}, {16, 32},
    };
    static const uint8_t group_order[3] = {1, 2, 0};
    bool occupied[kPpuBufWidth] = {false};
    bool drew = false;
    bool cleared = false;
    /* Recover actor pieces culled before OAM upload. */
    for (unsigned order = 0; order < 3; ++order)
        for (unsigned id = 0; id < EFFECT_ACTOR_COUNT; ++id) {
            const uint8_t *actor = g_ram + EFFECT_ACTOR_FIRST + id * EFFECT_ACTOR_STRIDE;
            if (!actor[ACTOR_ACTIVE] || !actor[ACTOR_VISIBLE] ||
                ((actor[ACTOR_KIND] - 2u) & 3u) != group_order[order])
                continue;
            const int half = actor[ACTOR_SIZE] ? 16 : 8;
            const unsigned size = sizes[PPU_objSize(ppu)][actor[ACTOR_SIZE] & 1u];
            const int left = ActorCoordinate(actor, ACTOR_BASE_X, ACTOR_OFFSET_X) - half;
            const int top = ActorCoordinate(actor, ACTOR_BASE_Y, ACTOR_OFFSET_Y) - half;
            int row = (int)line - top;
            const unsigned height = PPU_objInterlace(ppu) ? size / 2 : size;
            if (row < 0 || row >= (int)height)
                continue;
            if (!cleared) {
                for (int x = -(int)ppu->extraLeftCur; x < 256 + ppu->extraRightCur; ++x)
                    if ((x < 0 || x >= 256) && s_sprite_pixels[x + kPpuExtraLeftRight]) {
                        ppu->objBuffer.data[x + kPpuExtraLeftRight] = 0x0500;
                        s_sprite_pixels[x + kPpuExtraLeftRight] = false;
                    }
                cleared = true;
            }
            unsigned selector = actor[ACTOR_KIND];
            if (!selector) selector = (2u + g_ram[0x15ab]) & 3u;
            const unsigned attributes = (uint8_t)((selector << 4) |
                actor[ACTOR_PALETTE] | actor[ACTOR_ATTRIBUTES]);
            const unsigned base = attributes & 1u ? PPU_objTileAdr2(ppu) : PPU_objTileAdr1(ppu);
            const unsigned palette = 128u + ((attributes >> 1) & 7u) * 16u;
            const unsigned priority = SPRITE_PRIO_TO_PRIO((attributes >> 4) & 3u,
                                                         (attributes & 8u) == 0);
            if (PPU_objInterlace(ppu)) row = row * 2 + (ppu->evenFrame ? 0 : 1);
            if (attributes & 128u) row = (int)size - 1 - row;
            for (unsigned col = 0; col < size; ++col) {
                const int x = left + (int)col;
                if (x < -(int)ppu->extraLeftCur || x >= 256 + ppu->extraRightCur ||
                    (x >= 0 && x < 256))
                    continue;
                const unsigned at = (unsigned)(x + kPpuExtraLeftRight);
                if (occupied[at]) continue;
                const unsigned source_x = attributes & 64u ? size - 1u - col : col;
                const unsigned tile = (((actor[ACTOR_TILE] >> 4) + ((unsigned)row >> 3)) << 4) |
                    ((actor[ACTOR_TILE] + (source_x >> 3)) & 15u);
                const unsigned address = (base + tile * 16u + ((unsigned)row & 7u)) & 0x7fffu;
                const uint32_t planes = vram[address] | ((uint32_t)vram[(address + 8u) & 0x7fffu] << 16);
                const uint32_t bits = planes >> (7u - (source_x & 7u));
                const unsigned pixel = (bits & 1u) | ((bits >> 7) & 2u) |
                    ((bits >> 14) & 4u) | ((bits >> 21) & 8u);
                if (!pixel) continue;
                ppu->objBuffer.data[at] = (uint16_t)((priority << 8) | palette | pixel);
                s_sprite_pixels[at] = occupied[at] = true;
                drew = true;
            }
        }
    return drew;
}

static unsigned HudTile(const Ppu *ppu, unsigned line, unsigned x) {
    const unsigned column = ((x + ppu->hScroll[1]) & 511u) >> 3;
    const unsigned row = ((line + ppu->vScroll[1]) & 511u) >> 3;
    unsigned address = PPU_bgTilemapAdr(ppu, 1) + (row & 31u) * 32u + (column & 31u);
    if ((ppu->bgXsc[1] & 1u) && column >= 32u) address += 1024u;
    if ((ppu->bgXsc[1] & 2u) && row >= 32u)
        address += ppu->bgXsc[1] & 1u ? 2048u : 1024u;
    return PpuRenderVram(ppu)[address & 0x7fffu];
}

static unsigned TilePixel(const Ppu *ppu, unsigned tile, unsigned row, unsigned column) {
    if (tile & 0x8000u) row = 7u - row;
    if (tile & 0x4000u) column = 7u - column;
    const unsigned at = (PPU_bgTileAdr(ppu, 1) + (tile & 1023u) * 16u + row) & 0x7fffu;
    const uint16_t *vram = PpuRenderVram(ppu);
    const uint32_t planes = vram[at] | ((uint32_t)vram[(at + 8u) & 0x7fffu] << 16);
    const uint32_t bits = planes >> (7u - column);
    return (bits & 1u) | ((bits >> 7) & 2u) |
        ((bits >> 14) & 4u) | ((bits >> 21) & 8u);
}

static unsigned HudPixel(const Ppu *ppu, unsigned line, unsigned x) {
    const unsigned tile = HudTile(ppu, line, x);
    return ((tile >> 6) & 0x70u) | TilePixel(ppu, tile,
        (line + ppu->vScroll[1]) & 7u, (x + ppu->hScroll[1]) & 7u);
}

static bool PartyFrameTile(unsigned tile) {
    return tile >= 0x156u && tile <= 0x15du;
}

static bool HorizontalPartyFrameTile(unsigned tile) {
    return tile == 0x156u || tile == 0x158u || tile == 0x15au || tile == 0x15bu;
}

static unsigned HudTileRow(const Ppu *ppu, unsigned line, unsigned word) {
    const unsigned row = (line + ppu->vScroll[1]) & 7u;
    return word & 0x8000u ? 7u - row : row;
}

static unsigned HudPadding(const Ppu *ppu, unsigned line, bool right, bool party) {
    /* Strip the outer panel, not the card borders. */
    for (unsigned inset = 0; inset < 256; ++inset) {
        const unsigned pixel = HudPixel(ppu, line, right ? 255u - inset : inset);
        if (pixel != 0 && pixel != HUD_PADDING &&
            !(party && pixel == HUD_PANEL_BORDER))
            return inset;
    }
    return 256;
}

static bool PartyLine(const Ppu *ppu, unsigned line) {
    if (PPU_bigTiles(ppu, 1))
        return false;
    /* Match the visible panel, including its internal dividers. */
    static const unsigned columns[] = {0, 1, 8, 15, 22, 29, 30};
    static const uint16_t edges[3][7] = {
        {0x215a, 0x215b, 0x2158, 0x2158, 0x2158, 0x615b, 0x615a},
        {0x215c, 0x215d, 0x2159, 0x2159, 0x2159, 0x615d, 0x615c},
        {0xa15a, 0xa15b, 0xa158, 0xa158, 0xa158, 0xe15b, 0xe15a},
    };
    for (unsigned kind = 0; kind < 3; ++kind) {
        bool matches = true;
        for (unsigned x = 0; x < sizeof columns / sizeof *columns; ++x) {
            const unsigned screen_x = (columns[x] * 8u - ppu->hScroll[1]) & 511u;
            matches &= HudTile(ppu, line, screen_x) == edges[kind][x];
        }
        if (matches) return true;
    }
    return false;
}

static bool TitleLine(const Ppu *ppu, unsigned line) {
    const unsigned row = ((line + ppu->vScroll[1]) & 511u) >> 3;
    if (row < 28 || row > 31 || PPU_bigTiles(ppu, 1) || (ppu->hScroll[1] & 511u) ||
        ppu->vram[TITLE_TILEMAP] != TITLE_CORNER ||
        ppu->vram[TITLE_TILEMAP + 31] != (TITLE_CORNER | 0x4000u))
        return false;
    if (s_title.ready) return true;
    /* Only the original four-row message template qualifies. */
    for (unsigned y = 0; y < 4; ++y)
        for (unsigned x = 0; x < 32; ++x) {
            unsigned expected;
            if (x == 0 || x == 31)
                expected = (y == 0 || y == 3 ? TITLE_CORNER : TITLE_EDGE) |
                    (x == 31 ? 0x4000u : 0) | (y == 3 ? 0x8000u : 0);
            else if (y == 0 || y == 3)
                expected = TITLE_RULE | (y == 3 ? 0x8000u : 0);
            else
                expected = TITLE_GLYPHS + (x - 1u) * 2u + y - 1u;
            if (ppu->vram[TITLE_TILEMAP + y * 32u + x] != expected) return false;
        }
    unsigned first = 248, last = 8;
    for (unsigned y = 0; y < 16; ++y)
        for (unsigned x = 8; x < 248; ++x) {
            const unsigned tile = (TITLE_GLYPHS & 1023u) + ((x - 8u) >> 3) * 2u + (y >> 3);
            if (TilePixel(ppu, tile, y & 7u, x & 7u) != HUD_TEXT) continue;
            if (x < first) first = x;
            if (x + 1u > last) last = x + 1u;
        }
    if (first >= last) return false;
    const unsigned text_width = last - first;
    const unsigned width = text_width > 224u ? 256u : text_width + 32u;
    s_title.left = (256u - width) / 2u;
    s_title.right = s_title.left + width;
    s_title.text_left = (256u - text_width) / 2u;
    s_title.text_right = s_title.text_left + text_width;
    s_title.source_left = first;
    s_title.ready = true;
    return true;
}

static int TitleSource(const Ppu *ppu, unsigned line, unsigned x, unsigned blank) {
    if (x < s_title.left || x >= s_title.right) return -1;
    if (x < s_title.left + 8u) return (int)(x - s_title.left);
    if (x >= s_title.right - 8u) return (int)(248u + x - (s_title.right - 8u));
    const unsigned row = ((line + ppu->vScroll[1]) & 511u) >> 3;
    if (row == 28 || row == 31) return (int)(8u + ((x - s_title.left) & 7u));
    if (x >= s_title.text_left && x < s_title.text_right)
        return (int)(s_title.source_left + x - s_title.text_left);
    return (int)blank;
}

static bool CaptureParty(Ppu *ppu, unsigned line, const uint32_t original[256],
                         const int sources[256], Lufia2BattleEffectsLineRenderer *draw) {
    if (!Lufia2BattleUiActive()) return false;
    if (!Lufia2BattleUiCapturing()) return true;
    const unsigned left = (8u - ppu->hScroll[1]) & 511u;
    const unsigned edge = HudTile(ppu, line, left);
    if (edge == 0x215bu)
        s_party_top = (int)line - 1 - (int)((line + ppu->vScroll[1]) & 7u);
    if (s_party_top < 0 || line - 1u < (unsigned)s_party_top ||
        line - 1u >= (unsigned)s_party_top + 48u) return false;
    static Ppu ui;
    static uint32_t frame[kPpuBufWidth * 224];
    ui = *ppu;
    ui.renderBuffer = (uint8_t *)frame;
    ui.renderPitch = (256u + 2u * ui.extraLeftRight) * 4u;
    ui.screenEnabled[0] &= 6u;
    ui.screenEnabled[1] &= 6u;
    ui.widescreenLineEnhancer = NULL;
    memset(ui.overlayRenderBuffer, 0, sizeof ui.overlayRenderBuffer);
    s_hud_drawing = true;
    draw(&ui, line);
    s_hud_drawing = false;
    uint32_t cards[4][64];
    uint8_t mask[256];
    const uint32_t *row = frame + (line - 1u) * (256u + 2u * ui.extraLeftRight) + ui.extraLeftRight;
    for (unsigned x = 0; x < 256; ++x) mask[x] = sources[x] >= 0;
    unsigned left_edge = 8, right_edge = 56;
    /* Keep the caps' inner shading. */
    for (unsigned x = 0; x < 8; ++x) {
        const unsigned left_pixel = HudPixel(&ui, line, (8u + x - ui.hScroll[1]) & 511u);
        const unsigned right_pixel = HudPixel(&ui, line, (239u - x - ui.hScroll[1]) & 511u);
        if (left_edge == 8 && left_pixel && left_pixel != HUD_PADDING &&
            left_pixel != HUD_PANEL_BORDER)
            left_edge = x;
        if (right_edge == 56 && right_pixel && right_pixel != HUD_PADDING &&
            right_pixel != HUD_PANEL_BORDER)
            right_edge = 64 - x;
    }
    for (unsigned card = 0; card < 4; ++card)
        for (unsigned x = 0; x < 64; ++x) {
            const unsigned map_x = x < 8 ? 8 + x :
                x < 56 ? 16 + card * 56 + x - 8 : 232 + x - 56;
            const unsigned source = (map_x - ui.hScroll[1]) & 511u;
            if (source >= 256) return false;
            const unsigned pixel = HudPixel(&ui, line, source);
            const unsigned word = HudTile(&ui, line, source);
            const unsigned tile = word & 1023u;
            const unsigned tile_row = HudTileRow(&ui, line, word);
            const bool rim = PartyFrameTile(tile);
            const bool horizontal = HorizontalPartyFrameTile(tile);
            const bool cap = x < 8 || x >= 56;
            const bool transparent = !pixel || (rim && pixel == HUD_PANEL_BORDER) ||
                (cap && pixel == HUD_PADDING && (x < left_edge || x >= right_edge)) ||
                (horizontal && tile_row < 2u);
            cards[card][x] = transparent ? 0 : row[source] | 0xff000000u;
        }
    Lufia2BattleUiRecord(line - 1u - (unsigned)s_party_top, line - 1u,
                         cards, original, mask);
    return true;
}

static void DrawHudOverlay(Ppu *ppu, unsigned line, const uint8_t *hud_registers,
                           Lufia2BattleEffectsLineRenderer *draw) {
    static Ppu scene;
    uint32_t centre[256];
    uint8_t *destination = ppu->renderBuffer + (line - 1u) * ppu->renderPitch;
    const unsigned centre_at = ppu->extraLeftRight;
    scene = *ppu;
    if (hud_registers) memcpy(&scene.inidisp, hud_registers, PPU_SAVESTATE_REGS_SIZE);
    const bool party = s_hud_kind[line - 1u] == HUD_PARTY;
    const bool title = s_hud_kind[line - 1u] == HUD_TITLE;
    const unsigned left = HudPadding(&scene, line, false, party);
    const unsigned right = 256u - HudPadding(&scene, line, true, party);
    int sources[256];
    unsigned blank = 8;
    if (title)
        for (unsigned x = 8; x < 248; ++x)
            if (HudPixel(&scene, line, x) == HUD_FILL) {
                blank = x;
                break;
            }
    for (unsigned x = 0; x < 256; ++x) {
        sources[x] = title ? TitleSource(&scene, line, x, blank) :
            (x >= left && x < right ? (int)x : -1);
        if (party && s_hud_layers[line - 1u][x] == 1u) {
            const unsigned word = HudTile(&scene, line, x);
            const unsigned tile = word & 1023u;
            const unsigned row = HudTileRow(&scene, line, word);
            /* The enclosing black rim is outside the cards. */
            if ((HorizontalPartyFrameTile(tile) && row < 2u) || (PartyFrameTile(tile) &&
                HudPixel(&scene, line, x) == HUD_PANEL_BORDER))
                sources[x] = -1;
        }
        if (title && sources[x] >= 0 &&
            ((unsigned)sources[x] < left || (unsigned)sources[x] >= right))
            sources[x] = -1;
    }
    memcpy(centre, destination + centre_at * 4u, sizeof centre);
    const bool modern_party = party && CaptureParty(&scene, line, centre, sources, draw);
    const uint16_t scroll_x = scene.hScroll[0], scroll_y = scene.vScroll[0];
    memcpy(&scene.inidisp, s_scene_registers, sizeof s_scene_registers);
    /* BG1 keeps scrolling beneath both HUD bands. */
    scene.hScroll[0] = scroll_x;
    scene.vScroll[0] = scroll_y;
    scene.screenEnabled[0] &= ~2u;
    scene.screenEnabled[1] &= ~2u;
    scene.widescreenLineEnhancer = NULL;
    memset(scene.overlayRenderBuffer, 0, sizeof scene.overlayRenderBuffer);
    s_hud_drawing = true;
    draw(&scene, line);
    s_hud_drawing = false;
    if (modern_party) return;
    /* Keep the frames; reveal scenery outside them. */
    for (unsigned x = 0; x < 256; ++x) {
        const int source = sources[x];
        if (source >= 0 && (party || s_hud_layers[line - 1u][source] == 1u))
            memcpy(destination + (centre_at + x) * 4u, centre + source, 4);
        const unsigned layer = s_hud_layers[line - 1u][x];
        if (layer == 4u || layer == 6u)
            memcpy(destination + (centre_at + x) * 4u, centre + x, 4);
    }
}

void Lufia2BattleEffectsHudLine(Ppu *ppu, unsigned line,
                               Lufia2BattleEffectsLineRenderer *draw) {
    if (s_hud_drawing || !Lufia2BattleEffectsActive(ppu) || !line || line > 224u)
        return;
    const bool hud_layer = (ppu->screenEnabled[0] & 2u) &&
        PPU_bgTilemapAdr(ppu, 1) == HUD_TILEMAP && PPU_bgTileAdr(ppu, 1) == HUD_GRAPHICS;
    bool hud = false;
    if (hud_layer)
        for (unsigned x = 0; x < 256; ++x) {
            const unsigned layer = (ppu->bgBuffers[0].data[x + kPpuExtraLeftRight] >> 8) & 15u;
            s_hud_layers[line - 1u][x] = (uint8_t)layer;
            hud |= layer == 1u;
        }
    if (!hud && (ppu->screenEnabled[0] & 5u)) {
        memcpy(s_scene_registers, &ppu->inidisp, sizeof s_scene_registers);
        s_scene_valid = true;
        /* The title bar precedes the first scene row. */
        for (unsigned row = 0; row < 224; ++row)
            if (s_hud_pending[row]) {
                DrawHudOverlay(ppu, row + 1u, s_hud_registers[row], draw);
                s_hud_pending[row] = false;
            }
    } else if (hud) {
        const unsigned kind = TitleLine(ppu, line) ? HUD_TITLE :
            PartyLine(ppu, line) ? HUD_PARTY : 0;
        if (!kind) return;
        s_hud_kind[line - 1u] = (uint8_t)kind;
        if (s_scene_valid) DrawHudOverlay(ppu, line, NULL, draw);
        else {
            s_hud_pending[line - 1u] = true;
            memcpy(s_hud_registers[line - 1u], &ppu->inidisp, PPU_SAVESTATE_REGS_SIZE);
        }
    }
}

static void ExtendSceneWindow(int left, int right,
                              int *window_left, int *window_right) {
    /* Preserve finite names; extend the scene crop. */
    if (*window_left <= 16 && *window_right >= 239) {
        *window_left = left;
        *window_right = right - 1;
    }
}

bool Lufia2BattleEffectsWindows(const Ppu *ppu, unsigned layer,
                               int left, int right, int *w1_left, int *w1_right,
                               int *w2_left, int *w2_right, uint32_t *flags) {
    if (!Lufia2BattleEffectsActive(ppu) ||
        (layer != 0u && layer != 1u && layer != 5u &&
         !(layer == 4u && g_ram[BATTLE_EFFECT_SCRIPTS]) &&
         !Lufia2BattleEffectsPlane(ppu, layer)))
        return false;
    /* Hide empty name placeholders. */
    if (layer == 5u && PPU_clipMode(ppu) == 0 &&
        PPU_halfColor(ppu) && ppu->fixedColor == 0) {
        if (*w1_left == 0 && *w1_right == 0)
            *flags &= ~3u;
        if (*w2_left == 0 && *w2_right == 0)
            *flags &= ~12u;
    }
    ExtendSceneWindow(left, right, w1_left, w1_right);
    ExtendSceneWindow(left, right, w2_left, w2_right);
    return true;
}

void Lufia2BattleEffectsBeginLine(const Ppu *ppu) {
    memset(s_line_colours, 0, sizeof s_line_colours);
    s_uniform_palette = Lufia2BattleEffectsActive(ppu);
    s_palette_changed = false;
    if (s_uniform_palette) {
        /* Palettes 2 and 3 carry background flashes. */
        s_palette_colour = ppu->cgram[32];
        for (unsigned i = 33; i < 64; ++i) {
            if (ppu->cgram[i] != s_palette_colour)
                s_uniform_palette = false;
        }
        if (s_palettes_valid && g_ram[BATTLE_EFFECT_SCRIPTS])
            for (unsigned i = 0; i < 32; ++i)
                if (ppu->cgram[32 + i] != s_original_palettes[s_background][i])
                    s_palette_changed = true;
    }
}

int Lufia2BattleEffectsMosaic(int x, unsigned size) {
    if (!size)
        return x;
    const int remainder = x % (int)size;
    return x - (remainder < 0 ? remainder + (int)size : remainder);
}

static int ScrollOffset(unsigned scroll) {
    scroll &= 1023u;
    return scroll < 512u ? (int)scroll : (int)scroll - 1024;
}

void Lufia2BattleEffectsMargin(Ppu *ppu, PpuPixelPrioBufs *background,
                              unsigned y, bool sub, int left, int right,
                              PpuZbufType low_priority, bool mosaic) {
    const int margin = (int)(s_art_width - 256u) / 2;
    const int shift = ScrollOffset(ppu->hScroll[0]);
    PpuZbufType priorities[2] = {low_priority, low_priority};
    const int edges[2] = {(int)s_center_left - margin - shift,
                         (int)s_center_right - margin - shift - 1};
    for (unsigned side = 0; side < 2; ++side) {
        if (edges[side] >= -ppu->extraLeftCur &&
            edges[side] < 256 + ppu->extraRightCur) {
            const unsigned pixel = background->data[edges[side] + kPpuExtraLeftRight];
            if ((pixel & 0x0f00u) == 0 && (pixel & 255u))
                priorities[side] = (PpuZbufType)(pixel & 0xf000u);
        }
    }
    /* $97:B5D3 sets H=0, V=-1. */
    int source_y = (int)y - 1 + ScrollOffset(ppu->vScroll[0] + 1u);
    if (mosaic)
        source_y = (int)ppu->mosaicModulo[y] + ScrollOffset(ppu->vScroll[0]);
    if (source_y < 0)
        source_y = 0;
    if (source_y >= (int)s_art_height)
        source_y = (int)s_art_height - 1;
    for (int x = left; x < right; ++x) {
        const int sampled_x = mosaic
            ? Lufia2BattleEffectsMosaic(x, PPU_mosaicSize(ppu)) : x;
        int source_x = sampled_x + margin + shift;
        if (source_x >= (int)s_center_left && source_x < (int)s_center_right)
            continue; /* Keep the native centre sample. */
        if (source_x < 0)
            source_x = 0;
        if (source_x >= (int)s_art_width)
            source_x = (int)s_art_width - 1;
        const unsigned at = (unsigned)(x + kPpuExtraLeftRight);
        const uint32_t colour = s_art[(size_t)source_y * s_art_width + source_x];
        background->data[at] = 0x0500u; /* No repeated native tiles in art gaps. */
        if (colour >> 24) {
            s_line_colours[sub][at] = colour;
            s_line_palette[sub][at] = s_art_palette[(size_t)source_y * s_art_width + source_x];
            /* Art keeps BG1's layer and priority. */
            background->data[at] = priorities[source_x >= (int)s_center_right];
        }
    }
}

unsigned Lufia2BattleEffectsColour(const Ppu *ppu, unsigned pixel,
                                  unsigned index, bool sub) {
    const uint32_t colour = s_line_colours[sub][index];
    if (!Lufia2BattleEffectsOpaque(pixel, index, sub))
        return ppu->cgram[pixel & 255u];
    if (s_uniform_palette)
        return s_palette_colour;
    /* Convert art to SNES RGB555. */
    const unsigned art = ArtColour(colour);
    if (!s_palette_changed)
        return art;
    const unsigned palette = s_line_palette[sub][index];
    const unsigned original = s_original_palettes[s_background][palette];
    const unsigned current = ppu->cgram[32 + palette];
    unsigned adjusted = 0;
    /* Preserve authored colours through palette fades. */
    for (unsigned shift = 0; shift < 15; shift += 5) {
        int channel = (int)((art >> shift) & 31u) +
            (int)((current >> shift) & 31u) - (int)((original >> shift) & 31u);
        if (channel < 0) channel = 0;
        if (channel > 31) channel = 31;
        adjusted |= (unsigned)channel << shift;
    }
    return adjusted;
}

bool Lufia2BattleEffectsOpaque(unsigned pixel, unsigned index, bool sub) {
    return (pixel & 0x0fffu) == 0 && (s_line_colours[sub][index] >> 24) != 0;
}

void Lufia2BattleEffectsShutdown(void) {
    free(s_art);
    free(s_art_palette);
    s_art = NULL;
    s_art_palette = NULL;
    s_palettes_valid = false;
    s_loaded = s_active = false;
    s_effect_clear_pending = false;
    Lufia2BattleEffectsBeginLine(NULL);
}
