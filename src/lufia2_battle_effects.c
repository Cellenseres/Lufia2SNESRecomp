/* Sample battle art before sprites and colour math. */
#include "lufia2_battle_effects.h"
#include "lufia2_ppu_render_clone.h"
#include "lufia2_ppu_host.h"
#if defined(LUFIA2_NATIVE_PPU_TIMELINE)
#include "lufia2_ppu_timeline.h"
#endif
#include "lufia2_ppu_row_cache.h"
#include "snes/ws_shadow.h"
#include "snesrecomp_platform/snes_bitplanes.h"
#include "snesrecomp_platform/snes_ppu_index_row.h"
#include "snesrecomp_platform/snes_ppu_colour_table.h"

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
_Static_assert(_Alignof(Lufia2BattleEffectsPixelPolicy)>=4 &&
    !(offsetof(Lufia2BattleEffectsPixelPolicy,groups)&3u) &&
    !(offsetof(Lufia2BattleEffectsPixelPolicy,oam)&3u) &&
    (offsetof(Lufia2BattleEffectsPixelPolicy,actors)&3u)==(0x54b7u&3u),
    "host battle policy must match WRAM alignment");

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

static const Lufia2BattleEffectsPixelPolicy *s_pixel_policy;
typedef struct EffectActorList {
    const uint8_t *actors[EFFECT_ACTOR_COUNT];
    unsigned count;
    uint32_t epoch;
    bool valid,rows_valid,interlace;
    unsigned size_mode;
    uint32_t rows[256][2];
} EffectActorList;
static EffectActorList s_actor_primary,s_actor_replay;
static Lufia2BattleEffectsSceneRecorder *s_gpu_scene;
static void *s_gpu_scene_context;
bool Lufia2BattleEffectsBeginGpuScene(Lufia2BattleEffectsSceneRecorder *recorder,void *context) {
    if(!recorder || s_gpu_scene) return false;
    s_gpu_scene=recorder;s_gpu_scene_context=context;return true;
}
void Lufia2BattleEffectsEndGpuScene(void) {s_gpu_scene=NULL;s_gpu_scene_context=NULL;}
static Lufia2BattleEffectsPixelPolicy s_cached_pixel_policy;
static bool s_cached_pixel_policy_valid;
static void InvalidatePixelRows(bool preserve_scene);
static bool PixelPoliciesEqual(const Lufia2BattleEffectsPixelPolicy *a,
    const Lufia2BattleEffectsPixelPolicy *b) {
    if(a->scripts!=b->scripts) return false;
    return !a->scripts || (a->selector==b->selector &&
        snesrecomp_u8_equal(a->groups,b->groups,sizeof a->groups) &&
        snesrecomp_u8_equal(a->oam,b->oam,sizeof a->oam) &&
        snesrecomp_u8_equal(a->actors,b->actors,sizeof a->actors));
}
static const uint8_t *PixelRam(unsigned address) {
    if (!s_pixel_policy) return g_ram + address;
    if (address == BATTLE_EFFECT_SCRIPTS) return &s_pixel_policy->scripts;
    if (address == 0x15ab) return &s_pixel_policy->selector;
    if (address >= 0x15c0 && address < 0x1620) return s_pixel_policy->groups + (address - 0x15c0);
    if (address >= 0x100 && address < 0x320) return s_pixel_policy->oam + (address - 0x100);
    return s_pixel_policy->actors + (address - EFFECT_ACTOR_FIRST);
}
void Lufia2BattleEffectsCapturePixelPolicy(Lufia2BattleEffectsPixelPolicy *p) {
    memset(p->alignment_padding,0,sizeof p->alignment_padding);
    memset(p->actor_padding,0,sizeof p->actor_padding);
    p->scripts = g_ram[BATTLE_EFFECT_SCRIPTS];
    p->selector = g_ram[0x15ab];
    memcpy(p->groups, g_ram + 0x15c0, sizeof p->groups);
    memcpy(p->oam, g_ram + 0x100, sizeof p->oam);
    memcpy(p->actors, g_ram + EFFECT_ACTOR_FIRST, sizeof p->actors);
}
bool Lufia2BattleEffectsPixelPolicyMatches(const Lufia2BattleEffectsPixelPolicy *p) {
    if (p->scripts != g_ram[BATTLE_EFFECT_SCRIPTS]) return false;
    /* Inactive scripts never consult actor or OAM metadata. */
    return !p->scripts || (p->selector == g_ram[0x15ab] &&
        snesrecomp_u8_equal(p->groups, g_ram + 0x15c0, sizeof p->groups) &&
        snesrecomp_u8_equal(p->oam, g_ram + 0x100, sizeof p->oam) &&
        snesrecomp_u8_equal(p->actors, g_ram + EFFECT_ACTOR_FIRST, sizeof p->actors));
}
const Lufia2BattleEffectsPixelPolicy *Lufia2BattleEffectsBindPixelPolicy(
    const Lufia2BattleEffectsPixelPolicy *p) {
    const Lufia2BattleEffectsPixelPolicy *previous = s_pixel_policy;
    if (p && (!s_cached_pixel_policy_valid ||
        !PixelPoliciesEqual(p, &s_cached_pixel_policy))) {
        /* Qualified rows certify their resulting OBJ pixels separately. */
        const bool preserve_scene=s_cached_pixel_policy_valid &&
            s_cached_pixel_policy.scripts==p->scripts;
        InvalidatePixelRows(preserve_scene);
        s_cached_pixel_policy = *p;
        s_cached_pixel_policy_valid = true;
    }
    s_actor_replay.valid=false;s_actor_replay.count=0;
    s_pixel_policy = p; return previous;
}
static uint32_t *s_art;
static uint32_t *s_gpu_uniform_art;
static unsigned s_gpu_uniform_colour;
static bool s_gpu_uniform_valid;
static uint16_t s_gpu_palette[32];
static bool s_gpu_palette_effect;
static uint16_t s_gpu_row_palette[32];
static SnesRecompPpuColourDelta s_gpu_row_delta[32];
static bool s_gpu_row_valid, s_gpu_row_uniform, s_gpu_row_effect;
static uint8_t s_nearest_palette[32768];
static uint8_t s_nearest_background;
static bool s_nearest_valid;
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
static bool s_line_colours_valid;
static uint8_t s_line_palette[2][kPpuBufWidth];
static bool s_index_line;
static bool s_sprite_pixels[kPpuBufWidth];
static uint8_t s_scene_registers[PPU_SAVESTATE_REGS_SIZE];
static bool s_scene_valid;
static bool s_hud_pending[224];
static unsigned s_hud_pending_begin=224, s_hud_pending_end;
static uint8_t s_hud_kind[224];
static uint8_t s_hud_registers[224][PPU_SAVESTATE_REGS_SIZE];
static uint8_t s_hud_layers[224][256];
static bool s_hud_drawing;
static Lufia2PpuRowCache *s_hud_rows[2];
static bool s_hud_rows_attempted[2];
static const Lufia2PpuRowPolicy *s_row_policy;
static Lufia2BattleEffectsLineRenderer *s_hud_renderer;
static Lufia2PpuRowCache *s_scene_rows;
static struct BattleRow {
    PpuPixelPrioBufs backgrounds[2], objects;
    bool sprite_mask[kPpuBufWidth];
    bool sprites;
} *s_scene_inputs;
static bool s_scene_rows_attempted;
static const Ppu *s_scene_pending;
static uint8_t s_scene_seen[224];
static int s_party_top;
const Lufia2PpuRowPolicy *Lufia2BattleEffectsBindRowPolicy(const Lufia2PpuRowPolicy *policy) {
    const Lufia2PpuRowPolicy *previous=s_row_policy;
    s_row_policy=policy;
    for(unsigned i=0;i<2;++i) if(s_hud_rows[i]) s_hud_rows[i]->policy_checked=false;
    if(s_scene_rows) s_scene_rows->policy_checked=false;
    return previous;
}
static void InvalidatePixelRows(bool preserve_scene) {
    for (unsigned i = 0; i < 2; ++i)
        if (s_hud_rows[i]) memset(s_hud_rows[i]->valid, 0, sizeof s_hud_rows[i]->valid);
    if (s_scene_rows && !preserve_scene) memset(s_scene_rows->valid, 0, sizeof s_scene_rows->valid);
}
static struct {
    bool ready;
    unsigned left, right;
    unsigned text_left, text_right;
    unsigned source_left;
} s_title;
static bool s_title_certified;
static unsigned s_title_text_address;
static uint16_t s_title_map[128], s_title_text[60 * 16];

void Lufia2BattleEffectsInit(const uint8_t *rom, size_t size) {
    s_actor_primary.valid=s_actor_replay.valid=false;
    s_gpu_row_valid=false;
    s_cached_pixel_policy_valid = false;
    s_nearest_valid=false;
    s_title_certified=false;
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
    if(!s_nearest_valid || s_nearest_background!=id) {
        memset(s_nearest_palette,0,sizeof s_nearest_palette);
        s_nearest_background=id;s_nearest_valid=true;
    }
    if(s_nearest_palette[colour]) return s_nearest_palette[colour];
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
    s_nearest_palette[colour]=nearest;
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
            if(art[i]>>24) palette[i] = NearestPalette(ArtColour(art[i]), id);
    free(s_art);
    free(s_art_palette);
    free(s_gpu_uniform_art);s_gpu_uniform_art=NULL;s_gpu_uniform_valid=false;s_gpu_row_valid=false;
    s_art = art;
    s_art_palette = palette;
    s_art_width = width;
    s_art_height = height;
    s_center_left = asset.center_x;
    s_center_right = asset.center_x + asset.center_width;
    s_background = id;
    s_loaded = true;
    for (unsigned i = 0; i < 2; ++i)
        if (s_hud_rows[i]) memset(s_hud_rows[i]->valid, 0, sizeof s_hud_rows[i]->valid);
    if (s_scene_rows) memset(s_scene_rows->valid, 0, sizeof s_scene_rows->valid);
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
    Lufia2BattleEffectsRestartPixels(width, height);
    return s_active;
}

void Lufia2BattleEffectsRestartPixels(unsigned width, unsigned height) {
    s_actor_primary.valid=s_actor_replay.valid=false;
    s_scene_valid = false;
    s_title.ready = false;
    memset(s_hud_pending, 0, sizeof s_hud_pending);
    s_hud_pending_begin=224;s_hud_pending_end=0;
    s_party_top = -1;
    for (unsigned i = 0; i < 2; ++i) Lufia2PpuRowCacheBegin(s_hud_rows[i]);
    Lufia2PpuRowCacheBegin(s_scene_rows);
    memset(s_scene_seen, 0, sizeof s_scene_seen);
    s_scene_pending = NULL;
    Lufia2BattleUiBegin(s_active, width, height);
}

bool Lufia2BattleEffectsActive(const Ppu *ppu) {
    return s_active && ppu && PPU_mode(ppu) == 1;
}

bool Lufia2BattleEffectsPlane(const Ppu *ppu, unsigned layer) {
    if (!Lufia2BattleEffectsActive(ppu) || !*PixelRam(BATTLE_EFFECT_SCRIPTS) || layer != 2u)
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
bool Lufia2BattleEffectsGpuCompatible(const Ppu *ppu) {
    return Lufia2BattleEffectsActive(ppu) &&
        !s_effect_clear_pending && !ppu->widescreenLineEnhancer && !ppu->renderVram &&
        !(ppu->mosaic&15u) && !ppu->wsMode2CaptureLayer && !ppu->mode7Hd.pixels;
}
bool Lufia2BattleEffectsGpuArt(const Ppu *ppu,SnesRecompPpuJobImage *image,
    unsigned *center_left,unsigned *center_right) {
    return Lufia2BattleEffectsGpuArtPalette(ppu,ppu?ppu->cgram+32:NULL,
        image,center_left,center_right);
}
bool Lufia2BattleEffectsGpuArtPalette(const Ppu *ppu,const uint16_t *palette,
    SnesRecompPpuJobImage *image,unsigned *center_left,unsigned *center_right) {
    if(!Lufia2BattleEffectsGpuCompatible(ppu) || !image || !center_left || !center_right) return false;
    bool uniform=palette!=NULL;
    if(palette) for(unsigned i=1;i<32;++i) uniform &= palette[i]==palette[0];
    bool palette_effect=false;
    if(palette && s_palettes_valid && *PixelRam(BATTLE_EFFECT_SCRIPTS))
        palette_effect=memcmp(palette,s_original_palettes[s_background],sizeof s_gpu_palette)!=0;
    const uint32_t *pixels=s_art;
    if(uniform || palette_effect) {
        if(!s_gpu_uniform_art) s_gpu_uniform_art=malloc((size_t)s_art_width*s_art_height*sizeof(uint32_t));
        if(!s_gpu_uniform_art) return false;
        if(!s_gpu_uniform_valid || s_gpu_palette_effect!=(!uniform && palette_effect) ||
            (uniform?s_gpu_uniform_colour!=palette[0]:memcmp(s_gpu_palette,palette,sizeof s_gpu_palette)!=0)) {
            const unsigned c=palette[0];
            const uint32_t colour=0xff000000u|((c&31u)<<19)|((c>>5&31u)<<11)|((c>>10&31u)<<3);
            SnesRecompPpuColourDelta deltas[32];
            if(!uniform) for(unsigned i=0;i<32;++i)
                snesrecomp_ppu_colour_delta_prepare(deltas+i,
                    s_original_palettes[s_background][i],palette[i]);
            for(unsigned i=0;i<s_art_width*s_art_height;++i) {
                if(!(s_art[i]>>24)) {s_gpu_uniform_art[i]=0;continue;}
                if(uniform) {s_gpu_uniform_art[i]=colour;continue;}
                const unsigned adjusted=snesrecomp_ppu_colour_delta_rgb5(
                    deltas+s_art_palette[i],(uint16_t)ArtColour(s_art[i]));
                s_gpu_uniform_art[i]=0xff000000u|((adjusted&31u)<<19)|
                    ((adjusted>>5&31u)<<11)|((adjusted>>10&31u)<<3);
            }
            s_gpu_uniform_colour=c;s_gpu_uniform_valid=true;
            s_gpu_palette_effect=!uniform && palette_effect;
            memcpy(s_gpu_palette,palette,sizeof s_gpu_palette);
        }
        pixels=s_gpu_uniform_art;
    }
    *image=(SnesRecompPpuJobImage){pixels,s_art_width,(uint16_t)s_art_width,(uint16_t)s_art_height};
    *center_left=s_center_left;*center_right=s_center_right;return true;
}
bool Lufia2BattleEffectsGpuArtRow(const uint16_t palette[32],unsigned source_x,
    unsigned source_y,unsigned count,bool repeat,uint32_t *pixels) {
    if(!palette || !pixels || !s_art || !s_art_palette || source_y>=s_art_height ||
        source_x>=s_art_width || !count || (!repeat && count>s_art_width-source_x)) return false;
    const bool effect=s_palettes_valid && *PixelRam(BATTLE_EFFECT_SCRIPTS);
    if(!s_gpu_row_valid || s_gpu_row_effect!=effect ||
        memcmp(s_gpu_row_palette,palette,sizeof s_gpu_row_palette)) {
        s_gpu_row_uniform=true;
        for(unsigned i=1;i<32;++i) s_gpu_row_uniform &= palette[i]==palette[0];
        if(effect && !s_gpu_row_uniform) for(unsigned i=0;i<32;++i)
            snesrecomp_ppu_colour_delta_prepare(s_gpu_row_delta+i,
                s_original_palettes[s_background][i],palette[i]);
        memcpy(s_gpu_row_palette,palette,sizeof s_gpu_row_palette);
        s_gpu_row_effect=effect;s_gpu_row_valid=true;
    }
    const size_t begin=(size_t)source_y*s_art_width+source_x;
    for(unsigned x=0;x<count;++x) {
        const size_t at=begin+(repeat?0u:x);
        unsigned colour=0;
        if(s_art[at]>>24) {
            colour=s_gpu_row_uniform?palette[0]:ArtColour(s_art[at]);
            if(!s_gpu_row_uniform && effect)
                colour=snesrecomp_ppu_colour_delta_rgb5(s_gpu_row_delta+s_art_palette[at],(uint16_t)colour);
            pixels[x]=0xff000000u|((colour&31u)<<19)|((colour>>5&31u)<<11)|((colour>>10&31u)<<3);
        } else pixels[x]=0;
    }
    return true;
}

bool Lufia2BattleEffectsSprite(const Ppu *ppu, unsigned slot) {
    if (!Lufia2BattleEffectsActive(ppu) || !*PixelRam(BATTLE_EFFECT_SCRIPTS) || slot >= 128u)
        return false;
    /* Three actor lists contain effect sprites. */
    for (unsigned group = 0; group < 3; ++group) {
        const unsigned first = *PixelRam(0x15c1u + group);
        const unsigned count = *PixelRam(0x15deu + group * 4u);
        if (!*PixelRam(0x15dbu + group * 4u) || !count ||
            first >= 128u || count > 128u - first ||
            slot < first || slot - first >= count)
            continue;
        /* Match the OAM actually uploaded this frame. */
        const uint8_t *record = PixelRam(0x100u + slot * 4u);
        const unsigned high_shift = (slot & 3u) * 2u;
        return ppu->oam[slot * 2u] == (unsigned)(record[0] | (record[1] << 8)) &&
            ppu->oam[slot * 2u + 1u] == (unsigned)(record[2] | (record[3] << 8)) &&
            ((ppu->highOam[slot / 4u] ^ *PixelRam(0x300u + slot / 4u)) &
             (3u << high_shift)) == 0;
    }
    return false;
}

void Lufia2BattleEffectsCaptureSpriteRow(bool pixels[kPpuBufWidth]) {
    memcpy(pixels, s_sprite_pixels, sizeof s_sprite_pixels);
}
void Lufia2BattleEffectsRestoreSpriteRow(const bool pixels[kPpuBufWidth]) {
    memcpy(s_sprite_pixels, pixels, sizeof s_sprite_pixels);
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
    return Lufia2BattleEffectsActive(ppu) && *PixelRam(BATTLE_EFFECT_SCRIPTS) &&
        at >= 0 && at < kPpuBufWidth && s_sprite_pixels[at];
}

static unsigned ActorWord(const uint8_t *actor, unsigned offset) {
    return actor[offset] | (actor[offset + 1] << 8);
}

static int ActorCoordinate(const uint8_t *actor, unsigned base, unsigned offset) {
    const unsigned value = (ActorWord(actor, base) + ActorWord(actor, offset)) & 65535u;
    return value < 32768u ? (int)value : (int)value - 65536;
}

/* Only certified guest intervals or an owned replay policy may be reused. */
static EffectActorList *OrderedEffectActors(void) {
    uint32_t epoch=0;
#if defined(LUFIA2_NATIVE_PPU_TIMELINE)
    if(!s_pixel_policy) epoch=Lufia2PpuTimelineReadOnlyEpoch();
#endif
    EffectActorList *list=s_pixel_policy?&s_actor_replay:epoch?&s_actor_primary:NULL;
    if(!list) {s_actor_primary.valid=false;return NULL;}
    if(!list->valid || (!s_pixel_policy && list->epoch!=epoch)) {
        static const uint8_t order[3]={1,2,0};
        list->count=0;
        for(unsigned group=0;group<3;++group) for(unsigned id=0;id<EFFECT_ACTOR_COUNT;++id) {
            const uint8_t *actor=PixelRam(EFFECT_ACTOR_FIRST+id*EFFECT_ACTOR_STRIDE);
            if(actor[ACTOR_ACTIVE] && actor[ACTOR_VISIBLE] &&
                ((actor[ACTOR_KIND]-2u)&3u)==order[group])
                list->actors[list->count++]=actor;
        }
        list->epoch=epoch;list->valid=true;list->rows_valid=false;
    }
    return list;
}

static const uint32_t *EffectActorRow(EffectActorList *list,const Ppu *ppu,
    const uint8_t sizes[8][2],unsigned line) {
    if(line>=256u) return NULL;
    const unsigned mode=PPU_objSize(ppu);
    const bool interlace=PPU_objInterlace(ppu)!=0;
    if(!list->rows_valid || list->size_mode!=mode || list->interlace!=interlace) {
        memset(list->rows,0,sizeof list->rows);
        for(unsigned i=0;i<list->count;++i) {
            const uint8_t *a=list->actors[i];
            const int top=ActorCoordinate(a,ACTOR_BASE_Y,ACTOR_OFFSET_Y)-(a[ACTOR_SIZE]?16:8);
            const int height=sizes[mode][a[ACTOR_SIZE]&1u]/(interlace?2:1);
            const int begin=top<0?0:top,end=top+height>256?256:top+height;
            for(int y=begin;y<end;++y) list->rows[y][i>>5]|=UINT32_C(1)<<(i&31u);
        }
        list->size_mode=mode;list->interlace=interlace;list->rows_valid=true;
    }
    return list->rows[line];
}

bool Lufia2BattleEffectsSpriteMargins(Ppu *ppu, const uint16_t *vram, unsigned line) {
    if (!Lufia2BattleEffectsActive(ppu) || !*PixelRam(BATTLE_EFFECT_SCRIPTS))
        return false;
    static const uint8_t sizes[8][2] = {
        {8, 16}, {8, 32}, {8, 64}, {16, 32},
        {16, 64}, {32, 64}, {16, 32}, {16, 32},
    };
    static const uint8_t group_order[3] = {1, 2, 0};
    EffectActorList *ordered=OrderedEffectActors();
    const uint32_t *row_actors=ordered?EffectActorRow(ordered,ppu,sizes,line):NULL;
    if(row_actors && !(row_actors[0]|row_actors[1])) return false;
    bool occupied[kPpuBufWidth] = {false};
    bool drew = false;
    bool cleared = false;
    /* Recover actor pieces culled before OAM upload. */
    const unsigned entries=ordered?ordered->count:3u*EFFECT_ACTOR_COUNT;
    for(unsigned entry=0;entry<entries;++entry) {
            if(row_actors && !(row_actors[entry>>5]&(UINT32_C(1)<<(entry&31u)))) continue;
            const uint8_t *actor=ordered?ordered->actors[entry]:
                PixelRam(EFFECT_ACTOR_FIRST+(entry%EFFECT_ACTOR_COUNT)*EFFECT_ACTOR_STRIDE);
            if(!ordered && (!actor[ACTOR_ACTIVE] || !actor[ACTOR_VISIBLE] ||
                ((actor[ACTOR_KIND]-2u)&3u)!=group_order[entry/EFFECT_ACTOR_COUNT]))
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
            if (!selector) selector = (2u + *PixelRam(0x15ab)) & 3u;
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

/* Reuse certified planar rows for HUD source, padding and card extraction.
 * Each synchronous view checks current VRAM, independently of RGB caches. */
typedef SnesRecompPpuIndexRow HudScanline;
static SnesRecompPpuIndexRowCache s_hud_scan_rows[224];
static const HudScanline *CachedHudScanline(const Ppu *ppu,unsigned line) {
    if(!line || line>224u) return NULL;
    SnesPpuBgState bg={0};
    bg.tilemap_word_addr=(uint16_t)PPU_bgTilemapAdr(ppu,1);
    bg.char_word_addr=(uint16_t)PPU_bgTileAdr(ppu,1);bg.bpp=4;
    bg.wide=(ppu->bgXsc[1]&1u)!=0;bg.tall=(ppu->bgXsc[1]&2u)!=0;
    return snesrecomp_ppu_index_row4(s_hud_scan_rows+line-1u,PpuRenderVram(ppu),
        &bg,line+ppu->vScroll[1],ppu->hScroll[1],(ppu->hScroll[1]&511u)?64u:32u);
}
static unsigned ScanTile(const HudScanline *scan, unsigned x) {
    return scan->words[((x + scan->scroll) & 511u) >> 3];
}
static unsigned ScanPadding(const HudScanline *scan, bool right, bool party) {
    for (unsigned inset = 0; inset < 256; ++inset) {
        const unsigned pixel = scan->pixels[right ? 255u - inset : inset];
        if (pixel != 0 && pixel != HUD_PADDING && !(party && pixel == HUD_PANEL_BORDER)) return inset;
    }
    return 256;
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
    const unsigned first=HudTile(ppu,line,(0u-ppu->hScroll[1])&511u);
    unsigned kind=0;
    while(kind<3 && first!=edges[kind][0]) ++kind;
    if(kind==3) return false;
    for(unsigned x=1;x<sizeof columns/sizeof *columns;++x) {
        const unsigned screen_x=(columns[x]*8u-ppu->hScroll[1])&511u;
        if(HudTile(ppu,line,screen_x)!=edges[kind][x]) return false;
    }
    return true;
}

static bool TitleLine(const Ppu *ppu, unsigned line) {
    const unsigned row = ((line + ppu->vScroll[1]) & 511u) >> 3;
    if (row < 28 || row > 31 || PPU_bigTiles(ppu, 1) || (ppu->hScroll[1] & 511u) ||
        ppu->vram[TITLE_TILEMAP] != TITLE_CORNER ||
        ppu->vram[TITLE_TILEMAP + 31] != (TITLE_CORNER | 0x4000u))
        return false;
    if (s_title.ready) return true;
    const unsigned text_address = (PPU_bgTileAdr(ppu, 1) + (TITLE_GLYPHS & 1023u) * 16u) & 0x7fffu;
    const uint16_t *vram = PpuRenderVram(ppu);
    if (s_title_certified && text_address == s_title_text_address &&
        !memcmp(s_title_map, ppu->vram + TITLE_TILEMAP, sizeof s_title_map)) {
        bool unchanged = true;
        for (unsigned i = 0; i < 60u * 16u; ++i)
            if (s_title_text[i] != vram[(text_address + i) & 0x7fffu]) { unchanged = false; break; }
        if (unchanged) { s_title.ready = true; return true; }
    }
    s_title_certified = false;
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
    s_title_certified = true; s_title_text_address = text_address;
    memcpy(s_title_map, ppu->vram + TITLE_TILEMAP, sizeof s_title_map);
    for (unsigned i = 0; i < 60u * 16u; ++i) s_title_text[i] = vram[(text_address + i) & 0x7fffu];
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

typedef struct HudGeometry {
    uint64_t decodes;
    unsigned scroll, row, map_row, kind, title[5];
    uint8_t layers[256], mask[256];
    int16_t sources[256];
    int16_t cards[4][64];
    bool valid, cards_valid;
} HudGeometry;
static HudGeometry s_hud_geometry[224];

/* Index certificates cover both map words and planar bytes. Geometry cannot
 * observe palette animation, live RAM or the subsequent GPU scene choice. */
static const HudGeometry *CachedHudGeometry(const Ppu *ppu,unsigned line,
                                           const HudScanline *scan) {
    HudGeometry *g=s_hud_geometry+line-1u;
    const unsigned kind=s_hud_kind[line-1u];
    const bool party=kind==HUD_PARTY, title=kind==HUD_TITLE;
    const unsigned map_row=((line+ppu->vScroll[1])&511u)>>3;
    const unsigned title_key[5]={s_title.left,s_title.right,s_title.text_left,
                                s_title.text_right,s_title.source_left};
    const uint64_t decodes=s_hud_scan_rows[line-1u].decodes;
    if(g->valid && g->decodes==decodes && g->scroll==scan->scroll &&
        g->row==scan->row && g->map_row==map_row && g->kind==kind &&
        (!title || !memcmp(g->title,title_key,sizeof title_key)) &&
        !memcmp(g->layers,s_hud_layers[line-1u],sizeof g->layers)) return g;
    g->valid=true;g->decodes=decodes;g->scroll=scan->scroll;
    g->row=scan->row;g->map_row=map_row;g->kind=kind;
    memcpy(g->title,title_key,sizeof title_key);
    memcpy(g->layers,s_hud_layers[line-1u],sizeof g->layers);
    const unsigned left=ScanPadding(scan,false,party);
    const unsigned right=256u-ScanPadding(scan,true,party);
    unsigned blank=8;
    if(title) for(unsigned x=8;x<248;++x) if(scan->pixels[x]==HUD_FILL) {
        blank=x;break;
    }
    for(unsigned x=0;x<256;++x) {
        int source=title?TitleSource(ppu,line,x,blank):
            (x>=left && x<right?(int)x:-1);
        if(party && g->layers[x]==1u) {
            const unsigned word=ScanTile(scan,x),tile=word&1023u;
            if((HorizontalPartyFrameTile(tile) && HudTileRow(ppu,line,word)<2u) ||
                (PartyFrameTile(tile) && scan->pixels[x]==HUD_PANEL_BORDER)) source=-1;
        }
        if(title && source>=0 && ((unsigned)source<left || (unsigned)source>=right)) source=-1;
        g->sources[x]=(int16_t)source;g->mask[x]=(uint8_t)(source>=0);
    }
    g->cards_valid=false;
    if(!party) return g;
    unsigned left_edge=8,right_edge=56;
    for(unsigned x=0;x<8;++x) {
        const unsigned l=scan->pixels[(8u+x-scan->scroll)&511u];
        const unsigned r=scan->pixels[(239u-x-scan->scroll)&511u];
        if(left_edge==8 && l && l!=HUD_PADDING && l!=HUD_PANEL_BORDER) left_edge=x;
        if(right_edge==56 && r && r!=HUD_PADDING && r!=HUD_PANEL_BORDER) right_edge=64-x;
    }
    for(unsigned card=0;card<4;++card) for(unsigned x=0;x<64;++x) {
        const unsigned map_x=x<8?8+x:x<56?16+card*56+x-8:232+x-56;
        const unsigned source=(map_x-scan->scroll)&511u;
        if(source>=256) return g;
        const unsigned pixel=scan->pixels[source],word=ScanTile(scan,source),tile=word&1023u;
        const bool cap=x<8 || x>=56;
        const bool transparent=!pixel || (PartyFrameTile(tile) && pixel==HUD_PANEL_BORDER) ||
            (cap && pixel==HUD_PADDING && (x<left_edge || x>=right_edge)) ||
            (HorizontalPartyFrameTile(tile) && HudTileRow(ppu,line,word)<2u);
        g->cards[card][x]=(int16_t)(transparent?-1:(int)source);
    }
    g->cards_valid=true;
    return g;
}

static bool RowsRecorded(const Ppu *ppu) {
    /* Serial rendering may populate these caches from a different live policy.
     * The next immutable preparation must establish its certificate afresh. */
    if (!s_pixel_policy) s_cached_pixel_policy_valid = false;
    if (getenv("LUFIA2_BATTLE_CACHE_REFERENCE") || getenv("LUFIA2_PPU_HOST_REFERENCE") ||
        (*PixelRam(BATTLE_EFFECT_SCRIPTS) && !s_pixel_policy) || s_effect_clear_pending ||
        ppu->widescreenLineEnhancer || ppu->wsMode2CaptureLayer || ppu->mode7Hd.pixels ||
        256u + 2u * ppu->extraLeftRight > LUFIA2_PPU_CACHE_WIDTH) return false;
    for (unsigned bg = 0; bg < 4; ++bg) if (WsShadowLayerActive(bg)) return false;
    for (unsigned source = 0; source < kPpuOverlaySource_Count; ++source)
        if (ppu->overlayRenderBuffer[source]) return false;
    return true;
}

static bool AllocateSceneRows(void) {
    if (!s_scene_rows_attempted) {
        s_scene_rows_attempted = true;
        s_scene_rows = calloc(1, sizeof *s_scene_rows);
        s_scene_inputs = calloc(224, sizeof *s_scene_inputs);
        if (!s_scene_rows || !s_scene_inputs) {
            free(s_scene_rows); free(s_scene_inputs);
            s_scene_rows = NULL; s_scene_inputs = NULL;
        }
    }
    return s_scene_rows!=NULL;
}
static void CertifySceneObjects(const Ppu *ppu,unsigned line) {
    struct BattleRow *input=s_scene_inputs+line-1u;
    const bool objects=((ppu->screenEnabled[0]|ppu->screenEnabled[1])&16u)!=0;
    if(objects && (input->sprites!=ppu->lineHasSprites ||
        (ppu->lineHasSprites &&
         (!snesrecomp_u16_equal(input->objects.data,ppu->objBuffer.data,kPpuBufWidth) ||
          (*PixelRam(BATTLE_EFFECT_SCRIPTS) && !snesrecomp_u8_equal((const uint8_t *)input->sprite_mask,
            (const uint8_t *)s_sprite_pixels,sizeof s_sprite_pixels)))))) {
        s_scene_rows->valid[line-1u]=0;
        input->sprites=ppu->lineHasSprites;
        if(ppu->lineHasSprites) {
            input->objects=ppu->objBuffer;
            if(*PixelRam(BATTLE_EFFECT_SCRIPTS)) memcpy(input->sprite_mask,s_sprite_pixels,sizeof input->sprite_mask);
        }
    }
}
#if defined(LUFIA2_PPU_CAPTURE_DIAGNOSTICS)
#include <stdio.h>
static void TraceBattleRowInputs(const Ppu *ppu,unsigned line) {
    static int enabled=-1;static unsigned seen;
    if(enabled<0)enabled=getenv("LUFIA2_PPU_JOB_TRACE_ROOT")!=NULL;
    if(!enabled || seen>=6u)return;
    ++seen;
    unsigned shadow=0,overlay=0;
    for(unsigned i=0;i<4;++i)if(WsShadowLayerActive(i))shadow|=1u<<i;
    for(unsigned i=0;i<kPpuOverlaySource_Count;++i)if(ppu->overlayRenderBuffer[i])overlay|=1u<<i;
    fprintf(stderr,"Battle row inputs: line=%u drawing=%u active=%u host=%u policy=%u scripts=%u pending=%u shadow=%u overlay=%u enhancer=%u mode2=%u mode7=%u mode=%u extra=%u/%u/%u flags=%lu\n",
        line,s_hud_drawing,Lufia2BattleEffectsActive(ppu),lufia2_ppu_host_costs_enabled,
        s_pixel_policy!=NULL,*PixelRam(BATTLE_EFFECT_SCRIPTS),s_effect_clear_pending,shadow,overlay,
        ppu->widescreenLineEnhancer!=NULL,ppu->wsMode2CaptureLayer,ppu->mode7Hd.pixels!=NULL,
        PPU_mode(ppu),ppu->extraLeftRight,ppu->extraLeftCur,ppu->extraRightCur,(unsigned long)ppu->renderFlags);
}
#endif
bool Lufia2BattleEffectsReadRow(Ppu *ppu, unsigned line) {
#if defined(LUFIA2_PPU_CAPTURE_DIAGNOSTICS)
    TraceBattleRowInputs(ppu,line);
#endif
    s_scene_pending = NULL;
    if (s_hud_drawing || !Lufia2BattleEffectsActive(ppu) || !line || line > 224u ||
        !RowsRecorded(ppu) || !AllocateSceneRows()) return false;
    /* Repeated standalone draws do not carry the runtime's frame/write
     * contract. Recheck their bytes instead of trusting its dirty counter. */
    if (s_scene_seen[line - 1u]) Lufia2PpuRowCacheBegin(s_scene_rows);
    s_scene_seen[line - 1u] = 1;
    struct BattleRow *input = s_scene_inputs + line - 1u;
    /* Qualification already ran on the guest timeline. Certify its resulting
     * row instead of invalidating the entire scene for unrelated OAM motion.
     * Pixel replay cannot read OAM or OBJ character memory again. */
    CertifySceneObjects(ppu,line);
    Lufia2PpuRowCacheBindPolicy(s_scene_rows,s_row_policy);
    const unsigned width = 256u + 2u * ppu->extraLeftRight;
    uint32_t *row = (uint32_t *)(ppu->renderBuffer + (line - 1u) * ppu->renderPitch);
    const bool objects=((ppu->screenEnabled[0]|ppu->screenEnabled[1])&16u)!=0;
    /* BG2/BG3-only HUD rows do not sample authored BG1. Their tile palette
     * dependencies are certified below; a scene flash must not evict them. */
    const unsigned screens=ppu->screenEnabled[0]|ppu->screenEnabled[1];
    uint16_t palette_blocks=(screens&1u) && Lufia2BattleEffectsBackground(ppu)?12u:0u;
    if (objects && ppu->lineHasSprites)
        for (unsigned x = 0; x < kPpuBufWidth; ++x)
            if (ppu->objBuffer.data[x] & 255u)
                palette_blocks |= (uint16_t)(1u << ((ppu->objBuffer.data[x] & 255u) >> 4));
    if (!Lufia2PpuRowCacheReadInputs(s_scene_rows, ppu, line, 0, row, width, false,
        screens & 7u, palette_blocks)) {
        if (s_scene_rows->pending) {
            s_scene_pending = ppu;
        }
        return false;
    }
    memcpy(ppu->bgBuffers, input->backgrounds, sizeof input->backgrounds);
    return true;
}

void Lufia2BattleEffectsStoreRow(const Ppu *ppu, unsigned line) {
    if (s_scene_pending != ppu || !line || line > 224u) return;
    memcpy(s_scene_inputs[line - 1u].backgrounds, ppu->bgBuffers, sizeof ppu->bgBuffers);
    Lufia2PpuRowCacheStore(s_scene_rows,
        (const uint32_t *)(ppu->renderBuffer + (line - 1u) * ppu->renderPitch));
    s_scene_pending = NULL;
}

static void DrawHudSource(Ppu *ppu, unsigned line, unsigned pass,
                         Lufia2BattleEffectsLineRenderer *draw) {
    Lufia2PpuRowCache *cache = NULL;
    bool eligible = RowsRecorded(ppu);
    if (eligible) {
        if (s_hud_renderer != draw) {
            for (unsigned i = 0; i < 2; ++i)
                if (s_hud_rows[i]) memset(s_hud_rows[i]->valid, 0, sizeof s_hud_rows[i]->valid);
            s_hud_renderer = draw;
        }
        if (!s_hud_rows_attempted[pass]) {
            s_hud_rows_attempted[pass] = true;
            s_hud_rows[pass] = calloc(1, sizeof *s_hud_rows[pass]);
        }
        cache = s_hud_rows[pass];
    }
    const unsigned width = 256u + 2u * ppu->extraLeftRight;
    uint32_t *row = (uint32_t *)(ppu->renderBuffer + (line - 1u) * ppu->renderPitch);
    const unsigned screens = ppu->screenEnabled[0] | ppu->screenEnabled[1];
    Lufia2PpuRowCacheBindPolicy(cache,s_row_policy);
    /* Only the resulting RGB row is reused. HUD extraction, guest sprite
     * evaluation, live layout and ownership callbacks still execute. */
    if (!Lufia2PpuRowCacheReadInputs(cache, ppu, line, 0, row, width,
        (screens & 16u) != 0, screens & 7u, screens & 1u ? 12u : 0u)) {
        draw(ppu, line);
        Lufia2PpuRowCacheStore(cache, row);
    }
}

void Lufia2BattleEffectsCacheStats(uint64_t *hits, uint64_t *misses) {
    uint64_t h = 0, m = 0;
    if (s_scene_rows) { h += s_scene_rows->hits; m += s_scene_rows->misses; }
    for (unsigned i = 0; i < 2; ++i) if (s_hud_rows[i]) {
        h += s_hud_rows[i]->hits; m += s_hud_rows[i]->misses;
    }
    if (hits) *hits = h;
    if (misses) *misses = m;
}

static bool CaptureParty(Ppu *ppu, unsigned line, const uint32_t original[256],
                         const HudGeometry *geometry, bool current_row,
                         Lufia2BattleEffectsLineRenderer *draw) {
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
    /* A current BG2/BG3-only primary row is already the isolated card view.
     * Removed sub layers matter only for enabled colour math. Pending rows
     * retain the original draw because their palette/brightness may differ. */
    const bool identical=current_row && !(ppu->screenEnabled[0]&~6u) &&
        (!(ppu->screenEnabled[1]&~6u) || !PPU_addSubscreen(ppu) ||
         PPU_preventMathMode(ppu)==3 || !(PPU_mathEnabled(ppu)&38u)) &&
        !ppu->widescreenLineEnhancer && !getenv("LUFIA2_BATTLE_CACHE_REFERENCE") &&
        !getenv("LUFIA2_PPU_HOST_REFERENCE");
    const uint32_t *row=original;
    if(!identical) {
        Lufia2PpuLineClone(&ui,ppu);
        ui.renderBuffer=(uint8_t *)frame;
        ui.renderPitch=(256u+2u*ui.extraLeftRight)*4u;
        ui.screenEnabled[0]&=6u;ui.screenEnabled[1]&=6u;
        ui.widescreenLineEnhancer=NULL;
        memset(ui.overlayRenderBuffer,0,sizeof ui.overlayRenderBuffer);
        s_hud_drawing=true;DrawHudSource(&ui,line,0,draw);s_hud_drawing=false;
        row=frame+(line-1u)*(256u+2u*ui.extraLeftRight)+ui.extraLeftRight;
    }
    uint32_t cards[4][64];
    if(!geometry->cards_valid) return false;
    for (unsigned card = 0; card < 4; ++card)
        for (unsigned x = 0; x < 64; ++x) {
            const int source=geometry->cards[card][x];
            cards[card][x]=source<0?0:row[source]|0xff000000u;
        }
    uint32_t opaque_original[256];
    const uint32_t *original_pixels=original;
    if(s_gpu_scene) {
        for(unsigned x=0;x<256;++x) opaque_original[x]=original[x]|0xff000000u;
        original_pixels=opaque_original;
    }
    Lufia2BattleUiRecord(line - 1u - (unsigned)s_party_top, line - 1u,
                         cards, original_pixels, geometry->mask);
    return true;
}

static void DrawHudOverlay(Ppu *ppu, unsigned line, const uint8_t *hud_registers,
                           Lufia2BattleEffectsLineRenderer *draw) {
    static Ppu scene;
    uint32_t centre[256];
    uint8_t *destination = ppu->renderBuffer + (line - 1u) * ppu->renderPitch;
    const unsigned centre_at = ppu->extraLeftRight;
    Lufia2PpuLineClone(&scene, ppu);
    if (hud_registers) memcpy(&scene.inidisp, hud_registers, PPU_SAVESTATE_REGS_SIZE);
    const bool party = s_hud_kind[line - 1u] == HUD_PARTY;
    const HudScanline *scan=CachedHudScanline(&scene,line);
    const HudGeometry *geometry=CachedHudGeometry(&scene,line,scan);
    const int16_t *sources=geometry->sources;
    memcpy(centre, destination + centre_at * 4u, sizeof centre);
    const bool modern_party = party && CaptureParty(&scene,line,centre,geometry,!hud_registers,draw);
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
    const bool gpu_scene=s_gpu_scene && s_gpu_scene(s_gpu_scene_context,&scene,line);
    if(gpu_scene) memset(destination,0,(256u+2u*ppu->extraLeftRight)*sizeof(uint32_t));
    else DrawHudSource(&scene, line, 1, draw);
    s_hud_drawing = false;
    if (modern_party) return;
    /* Keep the frames; reveal scenery outside them. */
    for (unsigned x = 0; x < 256; ++x) {
        const int source = sources[x];
        if (source >= 0 && (party || s_hud_layers[line - 1u][source] == 1u))
            {
            const uint32_t pixel=centre[source]|(gpu_scene?0xff000000u:0u);
            memcpy(destination + (centre_at + x) * 4u,&pixel,4);
        }
        const unsigned layer = s_hud_layers[line - 1u][x];
        if (layer == 4u || layer == 6u)
            {
            const uint32_t pixel=centre[x]|(gpu_scene?0xff000000u:0u);
            memcpy(destination + (centre_at + x) * 4u,&pixel,4);
        }
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
        for (unsigned row = s_hud_pending_begin; row < s_hud_pending_end; ++row)
            if (s_hud_pending[row]) {
                DrawHudOverlay(ppu, row + 1u, s_hud_registers[row], draw);
                s_hud_pending[row] = false;
            }
        s_hud_pending_begin=224;s_hud_pending_end=0;
    } else if (hud) {
        const unsigned kind = TitleLine(ppu, line) ? HUD_TITLE :
            PartyLine(ppu, line) ? HUD_PARTY : 0;
        if (!kind) return;
        s_hud_kind[line - 1u] = (uint8_t)kind;
        if (s_scene_valid) DrawHudOverlay(ppu, line, NULL, draw);
        else {
            s_hud_pending[line - 1u] = true;
            if(line-1u<s_hud_pending_begin) s_hud_pending_begin=line-1u;
            if(line>s_hud_pending_end) s_hud_pending_end=line;
            memcpy(s_hud_registers[line - 1u], &ppu->inidisp, PPU_SAVESTATE_REGS_SIZE);
        }
    }
}
bool Lufia2BattleEffectsGpuHudLineWithInputs(Ppu *ppu,unsigned line,Lufia2BattleEffectsLineRenderer *draw,
    Lufia2BattleEffectsLineRenderer *indices,Lufia2BattleEffectsLineInputs *inputs,void *context) {
    if(!Lufia2BattleEffectsGpuCompatible(ppu) || !line || line>224u) return false;
    if(PPU_forcedBlank(ppu)) return true;
    if((ppu->screenEnabled[0]&2u) && PPU_bgTilemapAdr(ppu,1)==HUD_TILEMAP &&
        PPU_bgTileAdr(ppu,1)==HUD_GRAPHICS) {
        if(TitleLine(ppu,line) || PartyLine(ppu,line)) return true;
        /* An identical saved scene needs no new visibility decision. HUD
         * rows above still run, and pending title rows must still flush. */
        if(s_scene_valid && s_hud_pending_begin>=s_hud_pending_end &&
            !memcmp(s_scene_registers,&ppu->inidisp,sizeof s_scene_registers)) return false;
        /* Ordinary BG2 rows have no game-specific HUD relocation. Preserve
         * the exact scene/pending-title decision using main priority indices;
         * the GPU generates their finished pixels, including colour math. */
        const HudScanline *scan=PPU_bigTiles(ppu,1)?NULL:CachedHudScanline(ppu,line);
        if(!scan || scan->has_visible_pixels) {
            if(inputs) {inputs(context,ppu,line);inputs=NULL;}
            indices(ppu,line);
            for(unsigned x=0;x<256;++x)
                if(((ppu->bgBuffers[0].data[x+kPpuExtraLeftRight]>>8)&15u)==1u) return false;
        }
    }
    if(ppu->screenEnabled[0]&5u) {
        memcpy(s_scene_registers,&ppu->inidisp,sizeof s_scene_registers);s_scene_valid=true;
        for(unsigned row=s_hud_pending_begin;row<s_hud_pending_end;++row) if(s_hud_pending[row]) {
            if(inputs) {inputs(context,ppu,line);inputs=NULL;}
            DrawHudOverlay(ppu,row+1u,s_hud_registers[row],draw);s_hud_pending[row]=false;
        }
        s_hud_pending_begin=224;s_hud_pending_end=0;
    }
    return false;
}

bool Lufia2BattleEffectsGpuHudLine(Ppu *ppu,unsigned line,Lufia2BattleEffectsLineRenderer *draw,
    Lufia2BattleEffectsLineRenderer *indices) {
    return Lufia2BattleEffectsGpuHudLineWithInputs(ppu,line,draw,indices,NULL,NULL);
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
         !(layer == 4u && *PixelRam(BATTLE_EFFECT_SCRIPTS)) &&
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
    s_index_line=false;
    s_uniform_palette = Lufia2BattleEffectsActive(ppu);
    /* Inactive scenes cannot populate the artwork plane. Invalidate it in
     * constant time instead of clearing both full-width colour arrays on
     * every menu/field scanline. The reference keeps the original clears. */
    s_line_colours_valid=s_uniform_palette || !lufia2_ppu_host_costs_enabled;
    if(s_line_colours_valid) memset(s_line_colours,0,sizeof s_line_colours);
    s_palette_changed = false;
    if (s_uniform_palette) {
        /* Palettes 2 and 3 carry background flashes. */
        s_palette_colour = ppu->cgram[32];
        for (unsigned i = 33; i < 64; ++i) {
            if (ppu->cgram[i] != s_palette_colour)
                s_uniform_palette = false;
        }
        if (s_palettes_valid && *PixelRam(BATTLE_EFFECT_SCRIPTS))
            for (unsigned i = 0; i < 32; ++i)
                if (ppu->cgram[32 + i] != s_original_palettes[s_background][i])
                    s_palette_changed = true;
    }
}
void Lufia2BattleEffectsBeginIndexLine(void) {s_index_line=true;}
void Lufia2BattleEffectsEndIndexLine(void) {s_index_line=false;}
void Lufia2BattleEffectsGpuWindows(const Ppu *ppu,unsigned line,SnesPpuResolvedWindow out[6]) {
    for(unsigned layer=0;layer<6;++layer) {
        SnesPpuResolvedWindow *r=out+layer;
        memset(r,0,sizeof *r);r->selector=255u;
        int w1l=ppu->window1left,w1r=ppu->window1right;
        int w2l=ppu->window2left,w2r=ppu->window2right;
        uint32_t flags=ppu->windowsel>>(layer*4u)&15u;
        const bool effect=Lufia2BattleEffectsPlane(ppu,layer);
        const int left=effect?-(int)ppu->extraLeftCur:
            -PpuWidescreenLayerExtra(ppu,layer,(int)line,(int)ppu->extraLeftCur);
        const int right=256+(effect?(int)ppu->extraRightCur:
            PpuWidescreenLayerExtra(ppu,layer,(int)line,(int)ppu->extraRightCur));
        if(!Lufia2BattleEffectsWindows(ppu,layer,left,right,&w1l,&w1r,&w2l,&w2r,&flags)) continue;
        /* Empty enabled intervals still contribute their inverted constant
         * value to AND/XOR/XNOR; retain the original selector bits. */
        *r=(SnesPpuResolvedWindow){(int16_t)w1l,(int16_t)w1r,(int16_t)w2l,(int16_t)w2r,
            (uint8_t)flags,(uint8_t)(ppu->wbgobjlog>>(layer*2u)&3u)};
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
            if(!s_index_line) {
                s_line_colours[sub][at] = colour;
                s_line_palette[sub][at] = s_art_palette[(size_t)source_y * s_art_width + source_x];
            }
            /* Art keeps BG1's layer and priority. */
            background->data[at] = priorities[source_x >= (int)s_center_right];
        }
    }
}

unsigned Lufia2BattleEffectsColour(const Ppu *ppu, unsigned pixel,
                                  unsigned index, bool sub) {
    if (!Lufia2BattleEffectsOpaque(pixel, index, sub))
        return ppu->cgram[pixel & 255u];
    const uint32_t colour = s_line_colours[sub][index];
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
    return s_line_colours_valid && (pixel & 0x0fffu) == 0 && (s_line_colours[sub][index] >> 24) != 0;
}


void Lufia2BattleEffectsShutdown(void) {
    memset(&s_actor_primary,0,sizeof s_actor_primary);
    memset(&s_actor_replay,0,sizeof s_actor_replay);
    s_row_policy=NULL;
    s_index_line=false;
    s_gpu_row_valid=false;
    for(unsigned y=0;y<224;++y) s_hud_geometry[y].valid=false;
    s_cached_pixel_policy_valid = s_title_certified = false;
    s_nearest_valid=false;
    free(s_art);
    free(s_art_palette);
    free(s_gpu_uniform_art);s_gpu_uniform_art=NULL;s_gpu_uniform_valid=false;
    s_art = NULL;
    s_art_palette = NULL;
    s_palettes_valid = false;
    s_loaded = s_active = false;
    s_effect_clear_pending = false;
    for (unsigned i = 0; i < 2; ++i) {
        free(s_hud_rows[i]); s_hud_rows[i] = NULL;
        s_hud_rows_attempted[i] = false;
    }
    s_hud_renderer = NULL;
    free(s_scene_rows); free(s_scene_inputs);
    s_scene_rows = NULL; s_scene_inputs = NULL;
    s_scene_rows_attempted = false; s_scene_pending = NULL;
    Lufia2BattleEffectsBeginLine(NULL);
}
