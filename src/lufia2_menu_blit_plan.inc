/* Consumer geometry cache. Original guest state is never a cache key or a
 * cached result: these are already captured primitives and resolved layouts. */
#include "snesrecomp_platform/memory_compare.h"
typedef struct MenuBlitCache {
    Lufia2MenuUiObject objects[LUFIA2_MENU_UI_MAX_OBJECTS];
    MenuDestination targets[LUFIA2_MENU_UI_MAX_OBJECTS];
    uint16_t priority[3][HEIGHT][WIDTH], background[HEIGHT][WIDTH];
    uint8_t sprite_owner[HEIGHT][WIDTH];
    uint8_t coverage[3][HEIGHT][WIDTH];
    uint16_t text_owner[HEIGHT][WIDTH];
    uint8_t presence[4][HEIGHT][64], current_presence[4][HEIGHT][64];
    uint32_t map[SNESRECOMP_BLIT_MAX_WIDTH * HEIGHT];
    SnesRecompBlitRect rects[SNESRECOMP_BLIT_MAX_RECTS];
    SnesRecompPpuJobImage images[4];
    SnesRecompBlitPlan plan;
    unsigned width, count;
    bool valid;
    bool source_regions_changed, incremental;
    SnesRecompBlitCache *runs;
    unsigned runs_width;
    uint64_t updated_rows;
    uint64_t hits, misses, rejects;
} MenuBlitCache;
static MenuBlitCache *s_blit_cache;
static bool s_blit_cache_attempted;
static void MenuBlitStats(Lufia2MenuUiCacheStats *stats) {
    if (!s_blit_cache) return;
    stats->geometry_hits=s_blit_cache->hits;stats->geometry_misses=s_blit_cache->misses;
    stats->geometry_rows=s_blit_cache->updated_rows;stats->geometry_rejects=s_blit_cache->rejects;
}
static void MenuBlitSourceRegionsChanged(void) {
    if(s_blit_cache) s_blit_cache->source_regions_changed=true;
}
static bool MenuBlitObjectEqual(const Lufia2MenuUiObject *a, const Lufia2MenuUiObject *b) {
    return a->id == b->id && a->parent == b->parent && a->x == b->x && a->y == b->y &&
        a->width == b->width && a->height == b->height && a->kind == b->kind && a->slot == b->slot;
}
static bool MenuBlitTargetEqual(const MenuDestination *a, const MenuDestination *b) {
    return a->x == b->x && a->y == b->y && a->width == b->width &&
        a->height == b->height && a->visible == b->visible;
}
static unsigned MenuBlitObjectHeight(const Lufia2MenuUiObject *o, const MenuDestination *d) {
    return o->kind == WINDOW || o->kind == PARTY_HIGHLIGHT || d->height < o->height ?
        (unsigned)d->height : o->height;
}
static void MenuBlitMarkDestination(const Lufia2MenuUiObject *o, const MenuDestination *d,
                                    bool dirty[HEIGHT]) {
    if (!d->visible || o->kind == GROUP || d->height <= 0) return;
    const int end = d->y + (int)MenuBlitObjectHeight(o, d);
    for (int y = d->y < 0 ? 0 : d->y; y < end && y < HEIGHT; ++y) dirty[y] = true;
}
static unsigned MenuBlitDirtyRows(const MenuBlitCache *cache, unsigned width,
    unsigned image_count, bool dirty[HEIGHT],const bool same_priority[3][HEIGHT]) {
    if (!cache->valid || cache->width != width || cache->plan.image_count != image_count) {
        for (unsigned y = 0; y < HEIGHT; ++y) dirty[y] = true;
        return HEIGHT;
    }
    bool source[4][HEIGHT], background[HEIGHT];
    for (unsigned y = 0; y < HEIGHT; ++y) {
        background[y] = !snesrecomp_u16_equal(cache->background[y], s_background_priority[y], WIDTH);
        for (unsigned p = 0; p < 3; ++p) {
            source[p][y] = background[y] ||
                !snesrecomp_u8_equal(cache->presence[p][y], cache->current_presence[p][y], 64) ||
                !same_priority[p][y] ||
                (cache->source_regions_changed &&
                 !snesrecomp_u8_equal(cache->coverage[p][y], s_coverage[p][y], WIDTH));
        }
        source[1][y] |= cache->source_regions_changed &&
            !snesrecomp_u16_equal(cache->text_owner[y], s_text_owner[y], WIDTH);
        source[2][y] |= !snesrecomp_u8_equal(cache->sprite_owner[y], s_sprite_owner[y], WIDTH);
        source[3][y] = !snesrecomp_u8_equal(cache->presence[3][y], cache->current_presence[3][y], 64);
        /* Uncatalogued primitives retain their original screen coordinates. */
        dirty[y] = source[0][y] || source[1][y] || source[2][y];
    }
    const unsigned count = cache->count > s_count ? cache->count : s_count;
    for (unsigned i = 0; i < count; ++i) {
        if (i >= cache->count || i >= s_count ||
            !MenuBlitObjectEqual(cache->objects + i, s_objects + i) ||
            !MenuBlitTargetEqual(cache->targets + i, s_targets + i)) {
            if (i < cache->count) MenuBlitMarkDestination(cache->objects + i, cache->targets + i, dirty);
            if (i < s_count) MenuBlitMarkDestination(s_objects + i, s_targets + i, dirty);
            continue;
        }
        const Lufia2MenuUiObject *o = s_objects + i;
        const MenuDestination *d = s_targets + i;
        if (!d->visible || o->kind == GROUP || d->height <= 0) continue;
        if (o->kind == PARTY_HIGHLIGHT && background[o->y]) {
            MenuBlitMarkDestination(o, d, dirty); continue;
        }
        const unsigned height = MenuBlitObjectHeight(o, d);
        if (d->y >= HEIGHT || d->y + (int)height <= 0) continue;
        const unsigned plane = o->kind == WINDOW ? 0 : o->kind == TEXT ? 1 :
            o->kind == PARTY_HIGHLIGHT ? 3 : 2;
        const unsigned first = d->y < 0 ? (unsigned)-d->y : 0;
        const unsigned end = d->y + (int)height > HEIGHT ? (unsigned)(HEIGHT-d->y) : height;
        for (unsigned y = first; y < end; ++y) {
            const int dy = d->y + (int)y;
            if (dy < 0 || dy >= HEIGHT) continue;
            const unsigned sy = o->kind == PARTY_HIGHLIGHT ? (unsigned)dy : o->y +
                (o->kind == WINDOW ? FrameSliceCoordinate(y, d->height, o->height) : y);
            if (sy < HEIGHT && source[plane][sy]) dirty[dy] = true;
        }
    }
    unsigned count_rows = 0;
    for (unsigned y = 0; y < HEIGHT; ++y) count_rows += dirty[y];
    return count_rows;
}
static bool MenuBlitObjectsEqual(const MenuBlitCache *cache) {
    for (unsigned i = 0; i < s_count; ++i) {
        const Lufia2MenuUiObject *a = cache->objects + i, *b = s_objects + i;
        if (a->id != b->id || a->parent != b->parent || a->x != b->x || a->y != b->y ||
            a->width != b->width || a->height != b->height ||
            a->kind != b->kind || a->slot != b->slot) return false;
    }
    return true;
}
static bool MenuBlitTargetsEqual(const MenuBlitCache *cache) {
    for (unsigned i = 0; i < s_count; ++i) {
        const MenuDestination *a = cache->targets + i, *b = s_targets + i;
        /* Return-value padding is not a layout decision. In particular the
         * three bytes after bool are unspecified by the C ABI. */
        if (a->x != b->x || a->y != b->y || a->width != b->width ||
            a->height != b->height || a->visible != b->visible) return false;
    }
    return true;
}

const SnesRecompBlitPlan *Lufia2MenuUiBlitPlan(unsigned width) {
    if (!s_active || s_rows != HEIGHT || s_preview_path[0] ||
        width != s_width || width > SNESRECOMP_BLIT_MAX_WIDTH) return NULL;
    if (!s_blit_cache_attempted) {
        s_blit_cache_attempted = true;
        s_blit_cache = calloc(1, sizeof *s_blit_cache);
    }
    MenuBlitCache *cache = s_blit_cache;
    if (!cache) return NULL;
    const uint64_t check_begin=MenuClock();
    const char *reference = getenv("LUFIA2_MENU_BLIT_REFERENCE");
    const bool incremental = !reference || *reference == '0';
    if (cache->incremental != incremental) cache->valid = false;
    cache->incremental = incremental;
    for (unsigned i = 0; i < s_count; ++i) s_targets[i] = ResolveObjectDestination(i, 0);
    /* Palette animation changes source RGB, but only transparency can change
     * geometry. Captured opaque black remains present, exactly as Compose. */
    const unsigned image_count = s_move_highlight ? 4 : 3;
    bool same_priority[3][HEIGHT];
    memset(cache->current_presence, 0, sizeof cache->current_presence);
    for (unsigned p = 0; p < 3; ++p) for (unsigned y = 0; y < HEIGHT; ++y) {
        same_priority[p][y]=cache->valid && snesrecomp_u16_equal(cache->priority[p][y],s_priority[p][y],WIDTH);
        /* Complete native Mode-1 captures write either transparent zero or
         * opaque RGBA for each nonzero palette index. Primitive recolouring
         * preserves that presence; rejected OBJ owners clear both arrays.
         * An exact unchanged index/priority row therefore certifies geometry
         * without scanning its RGB pixels again. Serial/fallback callers keep
         * the unrestricted colour-based test below. */
        if(s_native_base && same_priority[p][y]) {
            memcpy(cache->current_presence[p][y],cache->presence[p][y],WIDTH/8);
            continue;
        }
        for (unsigned block = 0; block < WIDTH / 8; ++block) {
            const uint32_t *row = s_planes[p][y] + block * 8u;
            cache->current_presence[p][y][block] = (uint8_t)(
                (row[0] != 0) | (row[1] != 0) << 1 | (row[2] != 0) << 2 | (row[3] != 0) << 3 |
                (row[4] != 0) << 4 | (row[5] != 0) << 5 | (row[6] != 0) << 6 | (row[7] != 0) << 7);
        }
    }
    if (image_count == 4) for (unsigned y = 0; y < HEIGHT; ++y)
        for (unsigned block = 0; block < (width + 7u) / 8u; ++block) {
            unsigned bits = 0;
            for (unsigned bit = 0; bit < 8 && block * 8u + bit < width; ++bit)
                bits |= (s_highlight[y][block * 8u + bit] != 0) << bit;
            cache->current_presence[3][y][block] = (uint8_t)bits;
        }
    bool dirty[HEIGHT] = {false};
    const unsigned dirty_rows = incremental ? MenuBlitDirtyRows(cache, width, image_count, dirty,same_priority) : HEIGHT;
    s_menu_check_us+=MenuClock()-check_begin;
    if ((incremental && !dirty_rows) || (!incremental && !cache->source_regions_changed &&
        cache->valid && cache->width == width && cache->count == s_count &&
        cache->plan.image_count == image_count &&
        MenuBlitObjectsEqual(cache) &&
        MenuBlitTargetsEqual(cache) &&
        snesrecomp_u8_equal(&cache->presence[0][0][0], &cache->current_presence[0][0][0], sizeof cache->presence) &&
        snesrecomp_u16_equal(&cache->priority[0][0][0], &s_priority[0][0][0], sizeof s_priority / sizeof(uint16_t)) &&
        snesrecomp_u16_equal(&cache->background[0][0], &s_background_priority[0][0], sizeof s_background_priority / sizeof(uint16_t)) &&
        /* Exact source-region ownership already invalidates this cache when
         * rebuilt. Per-pixel sprite ownership and live priorities remain
         * independent certificates; no guest dirty flag substitutes for them. */
        snesrecomp_u8_equal(&cache->sprite_owner[0][0], &s_sprite_owner[0][0], sizeof s_sprite_owner))) {
        if (cache->source_regions_changed) {
            memcpy(cache->coverage,s_coverage,sizeof s_coverage);
            memcpy(cache->text_owner,s_text_owner,sizeof s_text_owner);
            cache->source_regions_changed=false;
        }
        /* Invisible metadata changes may leave the resolved map unchanged. */
        memcpy(cache->objects,s_objects,s_count*sizeof *s_objects);
        memcpy(cache->targets,s_targets,s_count*sizeof *s_targets);cache->count=s_count;
        ++cache->hits;
        return &cache->plan;
    }
    ++cache->misses;
    cache->valid = false;
    uint32_t rectangles = 0;
    uint16_t images = 0;
    const uint64_t resolve_begin=MenuClock();
    bool resolved = true;
    if (incremental) {
        for (unsigned y = 0; y < HEIGHT;) {
            if (!dirty[y]) { ++y; continue; }
            const unsigned first = y;
            while (y < HEIGHT && dirty[y]) ++y;
            resolved &= MenuResolveBlitRows(cache->map, width, sizeof cache->map / sizeof *cache->map,
                                            cache->images, &images, first, y);
        }
        s_menu_resolve_us+=MenuClock()-resolve_begin;
        const uint64_t compile_begin=MenuClock();
        if (!cache->runs || cache->runs_width != width) {
            snesrecomp_blit_cache_destroy(cache->runs);
            cache->runs = snesrecomp_blit_cache_create((uint16_t)width, HEIGHT);
            cache->runs_width = width;
        }
        resolved = resolved && cache->runs && snesrecomp_blit_cache_compile(cache->runs,
            cache->map, width, dirty, cache->images, images, cache->rects,
            SNESRECOMP_BLIT_MAX_RECTS, &rectangles);
        s_menu_compile_us+=MenuClock()-compile_begin;
    } else {
        resolved=Lufia2MenuUiResolveBlits(cache->map,width,
            sizeof cache->map / sizeof *cache->map,cache->images,&images);
        s_menu_resolve_us+=MenuClock()-resolve_begin;
        const uint64_t compile_begin=MenuClock();
        if(resolved) resolved=snesrecomp_blit_compile(cache->map,width,cache->images,images,
            (uint16_t)width,HEIGHT,cache->rects,SNESRECOMP_BLIT_MAX_RECTS,&rectangles);
        s_menu_compile_us+=MenuClock()-compile_begin;
    }
    if (!resolved) {
        ++cache->rejects;
        return NULL;
    }
    memcpy(cache->objects, s_objects, s_count * sizeof *s_objects);
    memcpy(cache->targets, s_targets, s_count * sizeof *s_targets);
    memcpy(cache->presence, cache->current_presence, sizeof cache->presence);
    memcpy(cache->priority, s_priority, sizeof s_priority);
    memcpy(cache->background, s_background_priority, sizeof s_background_priority);
    memcpy(cache->sprite_owner, s_sprite_owner, sizeof s_sprite_owner);
    if (cache->source_regions_changed || !cache->plan.image_count) {
        memcpy(cache->coverage, s_coverage, sizeof s_coverage);
        memcpy(cache->text_owner, s_text_owner, sizeof s_text_owner);
    }
    cache->source_regions_changed = false;
    cache->updated_rows += dirty_rows;
    cache->width = width; cache->count = s_count;
    cache->plan = (SnesRecompBlitPlan){cache->images, cache->rects, rectangles, images, (uint16_t)width, HEIGHT, 0};
    cache->valid = true;
    return &cache->plan;
}
