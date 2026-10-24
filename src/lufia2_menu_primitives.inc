/* Native-index geometry. No finished colours or destination pixel map. */
#include "snesrecomp_platform/snes_ppu_index_delta.h"
typedef struct MenuPrimitiveStorage {
    SnesRecompPpuPrimitive rectangles[2][SNESRECOMP_PPU_JOB_MAX_PRIMITIVES];
    SnesRecompPpuPrimitive *output;
    struct {uint32_t first,count;} parts[2][3+LUFIA2_MENU_UI_MAX_OBJECTS];
    unsigned active;
    Lufia2MenuUiPrimitivePlan plan;
    uint16_t line_band[HEIGHT];
    const SnesPpuRasterBand *bands;
    uint32_t previous_x[SNESRECOMP_PPU_MAX_WIDTH];
    Lufia2MenuUiObject objects[LUFIA2_MENU_UI_MAX_OBJECTS];
    MenuDestination targets[LUFIA2_MENU_UI_MAX_OBJECTS];
    uint16_t priority[3][HEIGHT][WIDTH],background[HEIGHT][WIDTH];
    uint8_t owners[HEIGHT][WIDTH];
    uint8_t coverage[3][HEIGHT][WIDTH];
    uint16_t text_owners[HEIGHT][WIDTH];
    uint64_t changed[9][HEIGHT],part_changed[2][3][HEIGHT];
    uint16_t oam[SNES_PPU_OAM_WORDS];
    uint8_t high_oam[SNES_PPU_HIGH_OAM_BYTES],oam_owners[128];
    SnesPpuRasterBand source_bands[SNES_PPU_MAX_BANDS];
    uint64_t source_regions,hits,misses,rejects,source_owner,vram_version,objects_version;
    uint64_t part_hits,part_misses;
    unsigned object_count,width,band_count;
    bool valid,has_oam;
} MenuPrimitiveStorage;
static MenuPrimitiveStorage *s_menu_primitives;
static void MenuPrimitiveStats(Lufia2MenuUiCacheStats *stats) {
    if(!s_menu_primitives) return;
    stats->geometry_hits+=s_menu_primitives->hits;stats->geometry_misses+=s_menu_primitives->misses;
    stats->geometry_rejects+=s_menu_primitives->rejects;
}
static const SnesRecompPpuMaterial s_menu_materials[6]={
    {.main_enable=3,.occluding_bg=1,.flags=SNESRECOMP_PPU_MATERIAL_SOURCE_COLOUR},
    {.main_enable=6,.occluding_bg=1,.flags=SNESRECOMP_PPU_MATERIAL_SOURCE_COLOUR},
    {.main_enable=18,.occluding_bg=1,.flags=SNESRECOMP_PPU_MATERIAL_SOURCE_COLOUR},
    {.main_enable=3,.occluding_bg=1,.flags=SNESRECOMP_PPU_MATERIAL_PALETTE_COLOUR},
    {.main_enable=6,.occluding_bg=1,.flags=SNESRECOMP_PPU_MATERIAL_PALETTE_COLOUR},
    {.main_enable=18,.occluding_bg=1,.flags=SNESRECOMP_PPU_MATERIAL_PALETTE_COLOUR}
};
static unsigned MenuPrimitiveMaterial(const SnesPpuRasterBand *band,unsigned plane) {
    const unsigned layer=plane==0?0:plane==1?2:4;
    const unsigned logic_offset=offsetof(Ppu,wbgobjlog)-offsetof(Ppu,inidisp);
    const unsigned logic=band->regs[logic_offset]|(unsigned)band->regs[logic_offset+1u]<<8;
    const bool isolated=((band->cgadsub&63u) || band->window_sel || logic) &&
        ((band->cgadsub&(1u<<layer)) || (band->cgwsel>>6) || band->window_sel || logic);
    return plane+(isolated?0u:3u);
}
static bool MenuPrimitiveAppend(MenuPrimitiveStorage *p,unsigned plane,unsigned owner,
    unsigned sx,unsigned sy,unsigned x,unsigned y,unsigned width,uint16_t z) {
    const SnesPpuRasterBand *band=p->bands+p->line_band[sy];
    const unsigned material=MenuPrimitiveMaterial(band,plane);
    const SnesRecompPpuPrimitive next={
        .source_x=(uint16_t)(sx+(s_width-WIDTH)/2),.source_y=(uint16_t)sy,
        .x=(uint16_t)x,.y=(uint16_t)y,.width=(uint16_t)width,.height=1,
        .priority=z,.material=(uint16_t)material,.owner=(uint16_t)owner,
        .source=(uint16_t)(plane==1?2:0),
        .kind=(uint8_t)(plane==2?SNESRECOMP_PPU_PRIMITIVE_OBJ_CAPTURE:SNESRECOMP_PPU_PRIMITIVE_BG)
    };
    const uint32_t previous=p->previous_x[x];
    if(previous!=UINT32_MAX) {
        SnesRecompPpuPrimitive *old=p->output+previous;
        if(old->y+old->height==y && old->source_y+old->height==sy &&
            old->source_x==next.source_x && old->width==width && old->priority==z &&
            old->material==material && old->owner==owner && old->kind==next.kind &&
            old->source==next.source && p->line_band[old->source_y]==p->line_band[sy]) {
            ++old->height;return true;
        }
    }
    if(p->plan.count==SNESRECOMP_PPU_JOB_MAX_PRIMITIVES) return false;
    p->previous_x[x]=p->plan.count;p->output[p->plan.count++]=next;
    return true;
}
static bool MenuPrimitiveVisible(unsigned plane,unsigned sx,unsigned sy,unsigned owner,bool catalogued) {
    if(!(s_priority[plane][sy][sx]&255u) ||
        s_priority[plane][sy][sx]<=s_background_priority[sy][sx]) return false;
    if(!catalogued)
        return !s_coverage[plane][sy][sx] || (plane==2 && s_sprite_owner[sy][sx]==255);
    return (plane!=2 || s_sprite_owner[sy][sx]==owner) &&
        (plane!=1 || s_text_owner[sy][sx]==owner);
}
/* Compare actual dependencies, not a global source-region epoch: a cursor
 * moving over an unchanged window must not discard that window's geometry.
 * Source and destination object records are checked by the caller. */
/* A miss computes every index dependency once. Overlapping windows and text
 * objects then query a conservative eight-pixel certificate rather than
 * rescanning the same captured planes. Warm whole-plan hits never enter here. */
static bool MenuPrimitiveChanges(MenuPrimitiveStorage *p) {
    for(unsigned plane=0;plane<3;++plane) {
        if(!snesrecomp_ppu_index_delta_u16(&p->priority[plane][0][0],&s_priority[plane][0][0],
            WIDTH,WIDTH,WIDTH,HEIGHT,p->changed[plane]) ||
           !snesrecomp_ppu_index_delta_u8(&p->coverage[plane][0][0],&s_coverage[plane][0][0],
            WIDTH,WIDTH,WIDTH,HEIGHT,p->changed[6+plane])) return false;
    }
    if(!snesrecomp_ppu_index_delta_u16(&p->background[0][0],&s_background_priority[0][0],
        WIDTH,WIDTH,WIDTH,HEIGHT,p->changed[3]) ||
       !snesrecomp_ppu_index_delta_u8(&p->owners[0][0],&s_sprite_owner[0][0],
        WIDTH,WIDTH,WIDTH,HEIGHT,p->changed[4]) ||
       !snesrecomp_ppu_index_delta_u16(&p->text_owners[0][0],&s_text_owner[0][0],
        WIDTH,WIDTH,WIDTH,HEIGHT,p->changed[5])) return false;
    for(unsigned plane=0;plane<3;++plane) for(unsigned y=0;y<HEIGHT;++y) {
        const uint64_t common=p->changed[plane][y]|p->changed[3][y]|
            (plane==2?p->changed[4][y]:0);
        p->part_changed[0][plane][y]=common|p->changed[6+plane][y];
        p->part_changed[1][plane][y]=common|(plane==1?p->changed[5][y]:0);
    }
    return true;
}
static bool MenuPrimitivePartEqual(const MenuPrimitiveStorage *p,unsigned plane,
    const Lufia2MenuUiObject *o) {
    const unsigned left=o?(unsigned)o->x:0,top=o?(unsigned)o->y:0;
    const unsigned width=o?o->width:WIDTH,height=o?o->height:HEIGHT;
    return snesrecomp_ppu_index_region_unchanged(p->part_changed[o!=NULL][plane],
        WIDTH,HEIGHT,left,top,width,height);
}
static void MenuPrimitiveSaveRows(void *destination,const void *source,size_t row_bytes,
    const uint64_t *changed,bool incremental) {
    if(!incremental) {memcpy(destination,source,row_bytes*HEIGHT);return;}
    for(unsigned y=0;y<HEIGHT;++y) if(changed[y])
        memcpy((uint8_t *)destination+y*row_bytes,(const uint8_t *)source+y*row_bytes,row_bytes);
}
static bool MenuPrimitiveReusePart(MenuPrimitiveStorage *p,unsigned part,unsigned previous,bool same) {
    const uint32_t first=p->parts[p->active][previous].first,count=p->parts[p->active][previous].count;
    p->parts[p->active^1u][part].first=p->plan.count;
    p->parts[p->active^1u][part].count=0;
    if(!same) {++p->part_misses;return false;}
    /* The total count cannot exceed the previous successful plan when every
     * part is retained, but a mixed frame is checked before each copy. */
    if(count>SNESRECOMP_PPU_JOB_MAX_PRIMITIVES-p->plan.count) return false;
    memcpy(p->output+p->plan.count,p->rectangles[p->active]+first,count*sizeof *p->output);
    p->plan.count+=count;p->parts[p->active^1u][part].count=count;++p->part_hits;return true;
}
/* Retain the exact raster dependency check. Separating shading-only changes
 * was correct but did not provide a measurable benefit in the game replay. */
static bool MenuPrimitiveBandsEqual(const SnesPpuRasterBand *previous,
    const SnesPpuRasterBand *current,unsigned count) {
    return snesrecomp_u8_equal((const uint8_t *)previous,(const uint8_t *)current,
        count*sizeof *current);
}
static bool MenuPrimitiveTargetsEqual(const MenuDestination *previous,
    const MenuDestination *current,unsigned count) {
    /* Return-by-value structs have unspecified padding. Cache decisions must
     * compare destination fields, never stack bytes outside those fields. */
    for(unsigned i=0;i<count;++i)
        if(!MenuBlitTargetEqual(previous+i,current+i)) return false;
    return true;
}
static bool MenuPrimitiveObjectsEqual(const Lufia2MenuUiObject *previous,
    const Lufia2MenuUiObject *current,unsigned count) {
    for(unsigned i=0;i<count;++i)
        if(!MenuBlitObjectEqual(previous+i,current+i)) return false;
    return true;
}
static const Lufia2MenuUiPrimitivePlan *MenuBuildPrimitivePlan(const SnesRecompPpuFrame *graphics) {
    const SnesPpuFrameCapture *capture=snesrecomp_ppu_frame_capture(graphics);
    if(!s_active || !s_native_base || s_rows!=HEIGHT || s_move_highlight || s_preview_path[0] ||
        !capture || capture->canvas_width!=s_width || capture->visible_height!=HEIGHT ||
        capture->canvas_extra!=(s_width-WIDTH)/2 || !capture->obj_pixels) return NULL;
    if(!s_menu_primitives) s_menu_primitives=calloc(1,sizeof *s_menu_primitives);
    MenuPrimitiveStorage *p=s_menu_primitives;if(!p) return NULL;
    const uint64_t resolve_begin=MenuClock();
    for(unsigned i=0;i<s_count;++i) s_targets[i]=ResolveObjectDestination(i,0);
    s_menu_resolve_us+=MenuClock()-resolve_begin;
    const uint64_t check_begin=MenuClock();
    const SnesRecompPpuFrameVersions *versions=snesrecomp_ppu_frame_versions(graphics);
    /* The immutable frame certifies the memory from which all three source
     * index planes and the background priority were recorded. Palette-only
     * changes do not change geometry. A changed certificate still takes the
     * exact plane comparison, retaining geometry after irrelevant uploads. */
    /* A qualified pixel plane alone does not certify the winning OAM slot:
     * identical sprite pixels may have different menu owners. Include the
     * original OAM and its metadata-to-owner association in the certificate. */
    const bool exact_bands=p->band_count==capture->band_count &&
        MenuPrimitiveBandsEqual(p->source_bands,capture->bands,capture->band_count);
    const bool same_sources=exact_bands && capture->oam && capture->high_oam && p->has_oam &&
        p->source_owner==versions->owner && p->vram_version==versions->vram &&
        p->objects_version==versions->objects &&
        snesrecomp_u16_equal(p->oam,capture->oam,SNES_PPU_OAM_WORDS) &&
        snesrecomp_u8_equal(p->high_oam,capture->high_oam,sizeof p->high_oam) &&
        snesrecomp_u8_equal(p->oam_owners,s_oam_slot,sizeof p->oam_owners);
    const bool same=p->valid && p->width==s_width && p->object_count==s_count &&
        p->source_regions==s_source_region_version && exact_bands &&
        MenuPrimitiveObjectsEqual(p->objects,s_objects,s_count) &&
        MenuPrimitiveTargetsEqual(p->targets,s_targets,s_count) &&
        (same_sources || (snesrecomp_u8_equal((const uint8_t *)&p->priority,(const uint8_t *)&s_priority,sizeof s_priority) &&
        snesrecomp_u8_equal((const uint8_t *)&p->background,(const uint8_t *)&s_background_priority,sizeof s_background_priority) &&
        snesrecomp_u8_equal((const uint8_t *)&p->owners,(const uint8_t *)&s_sprite_owner,sizeof s_sprite_owner)));
    s_menu_check_us+=MenuClock()-check_begin;
    if(same
        ) {
        p->source_owner=versions->owner;p->vram_version=versions->vram;
        p->objects_version=versions->objects;
        memcpy(p->source_bands,capture->bands,capture->band_count*sizeof *capture->bands);
        p->has_oam=capture->oam && capture->high_oam;
        if(p->has_oam) {
            memcpy(p->oam,capture->oam,sizeof p->oam);
            memcpy(p->high_oam,capture->high_oam,sizeof p->high_oam);
            memcpy(p->oam_owners,s_oam_slot,sizeof p->oam_owners);
        }
        ++p->hits;return &p->plan;
    }
    const bool incremental=p->valid && p->width==s_width && MenuPrimitiveChanges(p);
    bool reusable=incremental && p->object_count==s_count && exact_bands;
    ++p->misses;p->valid=false;
    p->output=p->rectangles[p->active^1u];
    p->plan=(Lufia2MenuUiPrimitivePlan){p->output,s_menu_materials,0,6};
    p->bands=capture->bands;
    for(unsigned band=0;band<capture->band_count;++band) {
        for(unsigned y=capture->bands[band].y_begin;y<capture->bands[band].y_end;++y)
            p->line_band[y]=(uint16_t)band;
    }
    /* Uncatalogued source pixels precede relocated objects, as in Compose. */
    for(unsigned plane=0;plane<3;++plane) {
        if(MenuPrimitiveReusePart(p,plane,plane,reusable && MenuPrimitivePartEqual(p,plane,NULL))) continue;
        memset(p->previous_x,255,sizeof p->previous_x);
        for(unsigned y=0;y<HEIGHT;++y)
        for(unsigned x=0;x<WIDTH;) {
            if(!MenuPrimitiveVisible(plane,x,y,0,false)) {++x;continue;}
            const unsigned first=x;const uint16_t z=s_priority[plane][y][x++];
            while(x<WIDTH && s_priority[plane][y][x]==z && MenuPrimitiveVisible(plane,x,y,0,false)) ++x;
            if(!MenuPrimitiveAppend(p,plane,UINT16_MAX,first,y,first+(s_width-WIDTH)/2,y,x-first,z)) return NULL;
        }
        p->parts[p->active^1u][plane].count=p->plan.count-p->parts[p->active^1u][plane].first;
    }
    for(unsigned i=0;i<s_count;++i) {
        const Lufia2MenuUiObject *o=s_objects+i;const MenuDestination *d=s_targets+i;
        const unsigned plane=o->kind==WINDOW?0:o->kind==TEXT?1:2;
        const bool same_object=reusable && MenuPrimitiveObjectsEqual(p->objects+i,o,1) &&
            MenuPrimitiveTargetsEqual(p->targets+i,d,1);
        const bool empty=o->kind==GROUP || o->kind==PARTY_HIGHLIGHT || !d->visible;
        if(MenuPrimitiveReusePart(p,3+i,3+i,
            same_object && (empty || MenuPrimitivePartEqual(p,plane,o)))) continue;
        if(empty) continue;
        memset(p->previous_x,255,sizeof p->previous_x);
        const int dw=o->kind==WINDOW || d->width<o->width?d->width:o->width;
        const int dh=o->kind==WINDOW || d->height<o->height?d->height:o->height;
        const int left=d->x<0?-d->x:0,top=d->y<0?-d->y:0;
        const int right=dw<(int)s_width-d->x?dw:(int)s_width-d->x;
        const int bottom=dh<HEIGHT-d->y?dh:HEIGHT-d->y;
        if(left>=right || top>=bottom) continue;
        uint16_t source_x[kPpuBufWidth];
        for(int x=left;x<right;++x) {
            const unsigned sx=o->kind==WINDOW?FrameSliceCoordinate((unsigned)x,d->width,o->width):(unsigned)x;
            source_x[x-left]=sx<o->width && sx+o->x<WIDTH?(uint16_t)(sx+o->x):UINT16_MAX;
        }
        for(int y=top;y<bottom;++y) {
            unsigned sy=o->kind==WINDOW?FrameSliceCoordinate((unsigned)y,d->height,o->height):(unsigned)y;
            if(sy>=o->height || sy+o->y>=HEIGHT) continue;
            sy+=o->y;
            const unsigned owner=plane==2?o->slot:i;
            for(int x=left;x<right;) {
                const unsigned sx=source_x[x-left];
                if(sx==UINT16_MAX || !MenuPrimitiveVisible(plane,sx,sy,owner,true)) {++x;continue;}
                const int first=x;const uint16_t z=s_priority[plane][sy][sx];++x;
                while(x<right && source_x[x-left]==sx+(unsigned)(x-first) &&
                    s_priority[plane][sy][source_x[x-left]]==z &&
                    MenuPrimitiveVisible(plane,source_x[x-left],sy,owner,true)) ++x;
                if(!MenuPrimitiveAppend(p,plane,owner,sx,sy,(unsigned)(d->x+first),
                    (unsigned)(d->y+y),(unsigned)(x-first),z)) return NULL;
            }
        }
        p->parts[p->active^1u][3+i].count=p->plan.count-p->parts[p->active^1u][3+i].first;
    }
    p->width=s_width;p->object_count=s_count;p->source_regions=s_source_region_version;
    p->source_owner=versions->owner;p->vram_version=versions->vram;
    p->objects_version=versions->objects;
    p->has_oam=capture->oam && capture->high_oam;
    if(p->has_oam) {
        memcpy(p->oam,capture->oam,sizeof p->oam);
        memcpy(p->high_oam,capture->high_oam,sizeof p->high_oam);
        memcpy(p->oam_owners,s_oam_slot,sizeof p->oam_owners);
    }
    p->band_count=capture->band_count;
    memcpy(p->source_bands,capture->bands,capture->band_count*sizeof *capture->bands);
    memcpy(p->objects,s_objects,s_count*sizeof *s_objects);memcpy(p->targets,s_targets,s_count*sizeof *s_targets);
    for(unsigned plane=0;plane<3;++plane) {
        MenuPrimitiveSaveRows(p->priority[plane],s_priority[plane],sizeof s_priority[plane][0],p->changed[plane],incremental);
        MenuPrimitiveSaveRows(p->coverage[plane],s_coverage[plane],sizeof s_coverage[plane][0],p->changed[6+plane],incremental);
    }
    MenuPrimitiveSaveRows(p->background,s_background_priority,sizeof s_background_priority[0],p->changed[3],incremental);
    MenuPrimitiveSaveRows(p->owners,s_sprite_owner,sizeof s_sprite_owner[0],p->changed[4],incremental);
    MenuPrimitiveSaveRows(p->text_owners,s_text_owner,sizeof s_text_owner[0],p->changed[5],incremental);
    p->active^=1u;p->valid=true;
    return &p->plan;
}
const Lufia2MenuUiPrimitivePlan *Lufia2MenuUiPrimitives(const SnesRecompPpuFrame *graphics) {
    const uint64_t begin=MenuClock(),previous=s_menu_check_us+s_menu_resolve_us;
    const Lufia2MenuUiPrimitivePlan *plan=MenuBuildPrimitivePlan(graphics);
    const uint64_t duration=MenuClock()-begin,children=s_menu_check_us+s_menu_resolve_us-previous;
    /* Exclusive compile time; resolve/check are reported separately. */
    s_menu_compile_us+=duration>children?duration-children:0;
    if(!plan && s_menu_primitives) ++s_menu_primitives->rejects;
    return plan;
}
