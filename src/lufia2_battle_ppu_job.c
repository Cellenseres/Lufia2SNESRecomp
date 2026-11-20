#include "lufia2_battle_ppu_job.h"
#include "lufia2_battle_effects.h"
#include "lufia2_battle_ui.h"
#include "lufia2_ppu_offload.h"
#include "lufia2_runtime.h"
#include "lufia2_ppu_render_clone.h"
#include "lufia2_ppu_row_cache.h"
#include "lufia2_ppu_obj_pixels.h"
#include "lufia2_ppu_host.h"
#include "snesrecomp_platform/snes_ppu_frame_key.h"
#include "snesrecomp_platform/snes_ppu_tile_plan.h"
#include "snesrecomp_platform/snes_ppu_source_compact.h"
#include <stdlib.h>
#include <string.h>
#if defined(LUFIA2_PPU_CAPTURE_DIAGNOSTICS) || defined(LUFIA2_PPU_JOB_TRACE)
#include <stdio.h>
#include "snesrecomp_platform/task.h"
#endif

typedef struct BattleJobPolicyKey {
    Lufia2PpuRowKey row;
    uint8_t object_policy[LUFIA2_PPU_ROW_OBJECT_POLICY];
    uint8_t layout[LUFIA2_BATTLE_UI_LAYOUT_BYTES], last_brightness;
    uint16_t first_sprite_line;
    uint64_t art;
} BattleJobPolicyKey;

struct Lufia2BattlePpuJob {
    SnesRecompPpuFrame *frame;
    SnesRecompPpuFrameKey *input_key;
    BattleJobPolicyKey policy_key;
    SnesRecompPpuJobImage cached_images[LUFIA2_BATTLE_JOB_IMAGES];
    SnesRecompPpuOverlay cached_overlays[LUFIA2_BATTLE_JOB_OVERLAYS];
    uint16_t cached_image_count, cached_overlay_count, cached_source_count;
    unsigned cached_card_top, cached_band_count;
    bool cache_valid, reused, cached_cards_ready, hud_reused;
    unsigned cached_packed_rows, cached_card_pixels;
    uint8_t hud_input_rows[224];
    /* Only original RGB overlays read primary OBJ. Completed isolated cards
     * do not; their scene objects are refreshed from the current capture. */
    uint8_t hud_object_rows[224];
    uint16_t cached_primary_objects[SNESRECOMP_PPU_MAX_WIDTH*224];
    uint64_t cache_hits, cache_misses;
    uint32_t cache_reject;
    SnesRecompPpuFrameVersions input_versions;
    Lufia2BattleEffectsPixelPolicy cached_effects;
    bool cached_sprite_rows[224*kPpuBufWidth];
    SnesPpuResolvedWindow windows[SNES_PPU_MAX_BANDS*6];
    SnesPpuRasterBand bands[SNES_PPU_MAX_BANDS];
    Ppu reference;
    uint32_t canvas[SNESRECOMP_PPU_MAX_WIDTH*224];
    uint32_t packed[SNESRECOMP_PPU_MAX_WIDTH*224];
    uint32_t cards[LUFIA2_BATTLE_UI_CARDS*LUFIA2_BATTLE_UI_CARD_HEIGHT*LUFIA2_BATTLE_UI_CARD_WIDTH];
    uint32_t compact_art[SNESRECOMP_PPU_MAX_WIDTH*224];
    uint8_t hud_rows[224], scene_rows[224];
    SnesPpuRasterBand scenes[224];
    uint16_t scene_palettes[224][256];
    uint16_t objects[SNESRECOMP_PPU_MAX_WIDTH*224];
    unsigned width;
    const bool *sprite_rows;
    const SnesPpuFrameCapture *qualified_capture;
    const uint64_t *qualified_coverage;
    const SnesPpuRasterBand *qualified_bands[224];
    uint16_t first_sprite_line;
    bool qualified_rows;
    bool palette_override;
    SnesRecompPpuSource sources[SNESRECOMP_PPU_JOB_MAX_SOURCES];
    uint16_t source_count;
    /* Only last row's sources can grow into this row. */
    uint16_t source_row, open_sources[32], row_sources[32];
    uint8_t open_count, row_count;
    bool rows_overflowed;
};
Lufia2BattlePpuJob *Lufia2BattlePpuJobCreate(void) {
    Lufia2BattlePpuJob *c=calloc(1,sizeof *c);
    if(c) {
        c->frame=snesrecomp_ppu_frame_create();
        if(!c->frame) {free(c);return NULL;}
        /* Optional acceleration. Exhaustion retains ordinary exact preparation. */
        c->input_key=snesrecomp_ppu_frame_key_create();
    }
    return c;
}
void Lufia2BattlePpuJobDestroy(Lufia2BattlePpuJob *c) {
    if(c) {snesrecomp_ppu_frame_key_destroy(c->input_key);snesrecomp_ppu_frame_release(c->frame);free(c);}
}
const SnesRecompPpuFrame *Lufia2BattlePpuJobFrame(const Lufia2BattlePpuJob *c) {return c?c->frame:NULL;}
bool Lufia2BattlePpuJobReused(const Lufia2BattlePpuJob *c) {return c && c->reused;}
uint32_t Lufia2BattlePpuJobCacheReject(const Lufia2BattlePpuJob *c) {return c?c->cache_reject:0;}
static bool CachePolicy(BattleJobPolicyKey *key,const Ppu *p,uint16_t first_line) {
    memset(key,0,sizeof *key);
    /* Interlaced alternate views retain serial preparation. Effect actors
     * have a separate owned policy and qualified-mask certificate below. */
    if(!lufia2_ppu_host_costs_enabled || PPU_objInterlace(p) || p->frameInterlace ||
        !(key->art=Lufia2BattleEffectsArtGeneration(p)) ||
        !Lufia2BattleUiCacheKey(key->layout)) return false;
    memcpy(key->row.registers,&p->inidisp,sizeof key->row.registers);
    memcpy(key->row.policy_head,&p->extraLeftCur,sizeof key->row.policy_head);
    memcpy(key->row.policy_tail,&p->wsLayerClamp,sizeof key->row.policy_tail);
    /* Pure qualification reads current hints and boolean grace only. The
     * primary timeline has already updated motion history exactly once;
     * isolated pixel replay never updates or consumes its X/signature history. */
#define COPY_OBJECT_POLICY(field) memcpy(key->object_policy + \
    (offsetof(Ppu,field)-offsetof(Ppu,wsOamMotionGraceOn)),&p->field,sizeof p->field)
    COPY_OBJECT_POLICY(wsOamMotionGraceOn);COPY_OBJECT_POLICY(wsOamLeftHintStrict);
    COPY_OBJECT_POLICY(wsOamLeftHint);COPY_OBJECT_POLICY(wsOamRightHintStrict);
    COPY_OBJECT_POLICY(wsOamRightHint);COPY_OBJECT_POLICY(lastMosaicModulo);
#undef COPY_OBJECT_POLICY
    for(unsigned i=0;i<128;++i)
        key->object_policy[offsetof(Ppu,wsOamMotionGrace)-offsetof(Ppu,wsOamMotionGraceOn)+i]=
            p->wsOamMotionGrace[i]!=0;
    memcpy(key->row.brightness,p->brightnessMult,sizeof key->row.brightness);
    memcpy(key->row.half_brightness,p->brightnessMultHalf,sizeof key->row.half_brightness);
    memcpy(key->row.mosaic,p->mosaicModulo,sizeof key->row.mosaic);
    key->row.mosaic_start=p->mosaicStartLine;key->row.overscan=p->frameOverscan;
    key->row.interlace=p->frameInterlace;key->row.render_flags=p->renderFlags;
    key->row.debug_layers=g_snes_ppu_dbg_layer_mask;
    key->last_brightness=p->lastBrightnessMult;key->first_sprite_line=first_line;
    return true;
}
static int SignedScroll(unsigned value) { value&=1023u;return value<512u?(int)value:(int)value-1024; }
static void ResetSources(Lufia2BattlePpuJob *c) {
    c->source_count=0;c->source_row=UINT16_MAX;
    c->open_count=c->row_count=0;c->rows_overflowed=false;
}
static void TouchSource(Lufia2BattlePpuJob *c,unsigned index) {
    if(c->row_count==sizeof c->row_sources/sizeof *c->row_sources) {c->rows_overflowed=true;return;}
    c->row_sources[c->row_count++]=(uint16_t)index;
}
static bool AddSource(Lufia2BattlePpuJob *c,SnesRecompPpuSource next) {
    if(next.y!=c->source_row) {
        /* Keep the full search's lowest-index-first order. */
        c->open_count=0;
        if(c->source_row!=UINT16_MAX && next.y==c->source_row+1u)
            for(unsigned i=0;i<c->row_count;++i) {
                unsigned at=c->open_count++;
                while(at && c->open_sources[at-1]>c->row_sources[i]) {
                    c->open_sources[at]=c->open_sources[at-1];--at;
                }
                c->open_sources[at]=c->row_sources[i];
            }
        c->row_count=0;c->source_row=next.y;
    }
    const unsigned candidates=c->rows_overflowed?c->source_count:c->open_count;
    for(unsigned k=0;k<candidates;++k) {
        const unsigned i=c->rows_overflowed?k:c->open_sources[k];
        SnesRecompPpuSource *r=c->sources+i;
        if(r->x!=next.x || r->width!=next.width || r->y+r->height!=next.y ||
            r->source_x!=next.source_x || r->source_width!=next.source_width ||
            r->screens!=next.screens || r->priority!=next.priority) continue;
        if(r->source_height==r->height && r->source_y+r->height==next.source_y) {
            ++r->height;r->source_height=r->height;TouchSource(c,i);return true;
        }
        if(r->source_height==1 && r->source_y==next.source_y) {++r->height;TouchSource(c,i);return true;}
    }
    if(c->source_count==SNESRECOMP_PPU_JOB_MAX_SOURCES) return false;
    TouchSource(c,c->source_count);
    c->sources[c->source_count++]=next;return true;
}
static bool SourceRange(Lufia2BattlePpuJob *c,int left,int right,int offset,
    unsigned y,unsigned source_y,const SnesRecompPpuJobImage *image,uint8_t screens,uint8_t priority) {
    while(left<right) {
        int sx=left+offset,end=right;bool clamp=false;
        if(sx<0) {sx=0;clamp=true;if(end>-offset) end=-offset;}
        else if(sx>=(int)image->width) {sx=image->width-1;clamp=true;}
        else if(end+offset>(int)image->width) end=image->width-offset;
        if(end<=left) return false;
        const SnesRecompPpuSource r={.x=(uint16_t)left,.y=(uint16_t)y,.width=(uint16_t)(end-left),.height=1,
            .source_x=(uint16_t)sx,.source_y=(uint16_t)source_y,
            .source_width=(uint16_t)(clamp?1:end-left),.source_height=1,.screens=screens,.priority=priority};
        if(!AddSource(c,r)) return false;
        left=end;
    }
    return true;
}
static void RecordedObjects(Ppu *p,const SnesPpuFrameCapture *cap,const uint64_t *coverage,unsigned y) {
    p->lineHasSprites=false;
    /* Frame freeze certifies coverage from every qualified pixel. Empty rows
     * cannot consume inherited OBJ scratch during pure pixel replay. */
    if(!cap->obj_pixels || (coverage && !coverage[y])) return;
    for(unsigned x=0;x<kPpuBufWidth;++x) p->objBuffer.data[x]=0x0500u;
    for(unsigned block=0;block<(cap->canvas_width+7u)/8u;++block) {
        if(coverage && !(coverage[y]&(UINT64_C(1)<<block))) continue;
        const unsigned end=(block+1u)*8u<cap->canvas_width?(block+1u)*8u:cap->canvas_width;
        for(unsigned x=block*8u;x<end;++x) {
            const unsigned pixel=cap->obj_pixels[y*cap->obj_pitch+x];
            if(!(pixel&0x800u)) continue;
            p->lineHasSprites=true;
            p->objBuffer.data[(int)x-cap->canvas_extra+kPpuExtraLeftRight]=(PpuZbufType)
                ((pixel&255u)|((pixel&0x400u)?0x0400u:0x0600u)|((2u+4u*((pixel>>8)&3u))<<12));
        }
    }
}
static void RecordedRegisters(Ppu *p,const SnesPpuFrameCapture *cap,
    const SnesPpuRasterBand *band,unsigned y) {
    memcpy(&p->inidisp,band->regs,PPU_SAVESTATE_REGS_SIZE);
    if(cap->cgram_lines) memcpy(p->cgram,cap->cgram_lines+y*256u,sizeof p->cgram);
    p->mosaicStartLine=(uint8_t)(y+1u);
}
typedef struct BattleRowInputs {
    const SnesPpuFrameCapture *capture;
    const uint64_t *coverage;
    const bool *sprites;
    unsigned y;
    bool ready;
} BattleRowInputs;
static void PrepareRecordedInputs(void *context,Ppu *ppu,unsigned line) {
    BattleRowInputs *row=context;
    (void)line;
    if(row->ready) return;
    RecordedObjects(ppu,row->capture,row->coverage,row->y);
    if(row->sprites) Lufia2BattleEffectsRestoreSpriteRow(row->sprites+row->y*kPpuBufWidth);
    row->ready=true;
}
static bool RecordedSceneObjects(const Lufia2BattlePpuJob *c,Ppu *p,unsigned line) {
    const SnesPpuFrameCapture *cap=c->qualified_capture;
    if(!c->qualified_rows || !cap || !cap->obj_pixels || !c->sprite_rows ||
        line<c->first_sprite_line || line-c->first_sprite_line>=cap->visible_height) return false;
    const unsigned row=line-c->first_sprite_line;
    const SnesPpuRasterBand *band=c->qualified_bands[row];
    const size_t base=offsetof(Ppu,inidisp);
    if(!band || band->forced_blank ||
        band->regs[offsetof(Ppu,obsel)-base]!=p->obsel ||
        ((band->regs[offsetof(Ppu,oamaddh)-base]^p->oamaddh)&128u) ||
        (PPU_objPriority(p) && ((band->regs[offsetof(Ppu,oamaddl)-base]^p->oamaddl)&254u)) ||
        ((band->regs[offsetof(Ppu,setini)-base]^p->setini)&2u)) return false;
    RecordedObjects(p,cap,c->qualified_coverage,row);
    Lufia2BattleEffectsRestoreSpriteRow(c->sprite_rows+row*kPpuBufWidth);
    return true;
}
static void RefreshSceneObjects(Lufia2BattlePpuJob *c,Ppu *p,unsigned line) {
    const SnesPpuFrameCapture *cap=c->qualified_capture;
    const unsigned y=line-1u,extra=(c->width-256u)/2u;
    const bool recorded=RecordedSceneObjects(c,p,line);
    if(!recorded) Lufia2PpuQualifyBattleSourceLine(p,line);
    const bool *visible=Lufia2BattleEffectsVisibleSpriteRow(p);
    const unsigned offset=kPpuExtraLeftRight-extra;
    if(!p->lineHasSprites) memset(c->objects+y*c->width,0,c->width*sizeof *c->objects);
    else if(recorded) {
        /* RecordedSceneObjects still restores the native scratch for its
         * caller. The GPU plane can keep the original packed values directly;
         * the historical source row is not necessarily the destination row. */
        const unsigned source_row=line-c->first_sprite_line;
        Lufia2PpuObjCopyRow(c->objects+y*c->width,
            cap->obj_pixels+source_row*cap->obj_pitch,visible+offset,c->width,extra);
    } else Lufia2PpuObjPackRow(c->objects+y*c->width,p->objBuffer.data+offset,
        p->lineHasSprites,visible+offset,c->width,extra);
}
static bool RecordGpuScene(void *context,Ppu *p,unsigned line) {
    Lufia2BattlePpuJob *c=context;
    if(line && line<=224) c->scene_rows[line-1u]=0;
    if(!line || line>224 || PPU_mode(p)!=1 || (p->mosaic&15u)) return false;
    /* The serial HUD clone borrows the current row's brightness tables even
     * when it restores earlier scene registers. Capture the actual pixel
     * brightness, without changing those historical registers or guest time. */
    const uint8_t brightness=p->lastBrightnessMult;
    if(brightness!=PPU_brightness(p)) {
        if(brightness>15u) return false;
        for(unsigned i=0;i<32;++i) {
            const unsigned value=((i<<3)|(i>>2))*brightness/15u;
            if(p->brightnessMult[i]!=value || p->brightnessMultHalf[i*2]!=value ||
                p->brightnessMultHalf[i*2+1]!=value) return false;
        }
        for(unsigned i=32;i<63;++i) if(p->brightnessMult[i]!=p->brightnessMult[31]) return false;
    }
    RefreshSceneObjects(c,p,line);
    const unsigned y=line-1u;
    const uint8_t display=p->inidisp;
    p->inidisp=(uint8_t)((display&0xf0u)|brightness);
    Lufia2CapturePpuVisualBand(c->scenes+y,p,line);
    p->inidisp=display;
    const SnesPpuFrameCapture *cap=c->qualified_capture;
    const uint16_t *palette=cap->cgram_lines?cap->cgram_lines+y*256u:cap->cgram;
    if(memcmp(p->cgram,palette,sizeof p->cgram)) {
        if(!c->palette_override) {
            for(unsigned row=0;row<224;++row)
                memcpy(c->scene_palettes[row],cap->cgram_lines?cap->cgram_lines+row*256u:cap->cgram,sizeof p->cgram);
            c->palette_override=true;
        }
    }
    if(c->palette_override) memcpy(c->scene_palettes[y],p->cgram,sizeof p->cgram);
    c->scene_rows[y]=1;return true;
}
static SnesRecompPpuResult FreezeJobFrame(SnesRecompPpuFrame *frame,const SnesPpuFrameCapture *cap) {
    return snesrecomp_ppu_frame_freeze(frame,cap,NULL,0);
}
/* The admitted HUD reuse path already certifies all background, palette and
 * layout inputs. Its derived bands/windows/art sources remain owned here;
 * only selected object rows belong to the new frame. Freeze still owns the
 * complete new capture and can refuse reuse while the previous frame is busy. */
static SnesRecompPpuResult FreezeDerivedFrame(Lufia2BattlePpuJob *c,
    const SnesPpuFrameCapture *cap,unsigned band_count) {
    SnesPpuFrameCapture resolved=*cap;resolved.bands=c->bands;resolved.band_count=band_count;
    if(c->palette_override) {
        resolved.cgram_lines=c->scene_palettes[0];
        resolved.cgram_line_count=224;
        resolved.raster_memory_flags|=SNES_PPU_RASTER_MEMORY_CGRAM;
    }
    /* Scene rows have already been produced. Copy only base rows which
     * survived, instead of copying the entire plane before overwriting them. */
    for(unsigned y=0;y<224;++y) if(!c->scene_rows[y]) {
        if(cap->obj_pixels) memcpy(c->objects+y*c->width,cap->obj_pixels+y*cap->obj_pitch,c->width*sizeof(uint16_t));
        else memset(c->objects+y*c->width,0,c->width*sizeof(uint16_t));
    }
    resolved.resolved_windows=c->windows;resolved.obj_pixels=c->objects;resolved.obj_pitch=c->width;
    if(c->hud_reused) {
        const SnesRecompPpuObjectUpdate update={.pixels=c->objects,.oam=cap->oam,
            .high_oam=cap->high_oam,.pitch=(uint16_t)c->width,
            .vram_dirty_words=cap->vram_dirty_words,.cgram_dirty_entries=cap->cgram_dirty_entries};
        const SnesRecompPpuResult result=snesrecomp_ppu_frame_replace_objects(c->frame,&update);
        if(result!=SNESRECOMP_PPU_UNSUPPORTED) return result;
    }
    return FreezeJobFrame(c->frame,&resolved);
}
static SnesRecompPpuResult CompactJobArt(const SnesRecompPpuJobImage *images,
    SnesRecompPpuSource *sources,uint16_t count,uint32_t *packed,uint32_t capacity,SnesRecompPpuJobImage *output_image) {
    return snesrecomp_ppu_source_image_compact(images,0,sources,count,packed,capacity,
        SNESRECOMP_PPU_SOURCE_COMPACT_EDGE_TILES,output_image,sources);
}
static SnesRecompPpuResult SampleJobEdge(const SnesPpuFrameCapture *cap,const SnesPpuRasterBand *band,
    const SnesRecompWindowSpan *windows,unsigned count,uint8_t layer,uint16_t x,uint16_t y,
    uint8_t *opaque_screens,uint8_t *priority) {
    return snesrecomp_ppu_mode1_layer_screens_windows(cap,band,windows,count,layer,x,y,opaque_screens,priority);
}
static SnesRecompPpuResult PrepareInternal(Lufia2BattlePpuJob *c,
    const Ppu *live,const SnesRecompPpuFrame *frame,SnesRecompPpuJobImage *images,uint16_t *image_count,
    SnesRecompPpuOverlay *overlays,uint16_t *overlay_count,const SnesRecompPpuSource **sources,uint16_t *source_count,
    bool reuse_hud) {
    const SnesPpuFrameCapture *cap=snesrecomp_ppu_frame_capture(frame);
    if(!c || !live || !cap || !images || !image_count || !overlays || !overlay_count || !sources || !source_count)
        return SNESRECOMP_PPU_INVALID;
    *image_count=*overlay_count=*source_count=0;*sources=NULL;ResetSources(c);
    if(!Lufia2BattleEffectsGpuCompatible(live) || cap->visible_height!=224 || lufia2_ppu_offload_recording ||
        cap->canvas_width!=256u+2u*live->extraLeftRight || cap->canvas_extra!=live->extraLeftRight ||
        snesrecomp_ppu_mode1_raw_supports(cap)!=SNES_PPU_SUPPORTED) return SNESRECOMP_PPU_UNSUPPORTED;
    c->qualified_capture=cap;
    c->qualified_coverage=snesrecomp_ppu_frame_obj_coverage(frame);
    if(!reuse_hud)c->palette_override=false;
    memset(c->qualified_bands,0,sizeof c->qualified_bands);
    for(unsigned bi=0;bi<cap->band_count;++bi)
        for(unsigned y=cap->bands[bi].y_begin;y<cap->bands[bi].y_end;++y)
            c->qualified_bands[y]=cap->bands+bi;
#if defined(LUFIA2_PPU_CAPTURE_DIAGNOSTICS) || defined(LUFIA2_PPU_JOB_TRACE)
    uint64_t phase[7]={snesrecomp_now_us()};
#endif
    unsigned centre_left=0,centre_right=0;
    if(!reuse_hud && !Lufia2BattleEffectsGpuArtPalette(live,NULL,
        images,&centre_left,&centre_right)) return SNESRECOMP_PPU_RESOURCE_LIMIT;
    const unsigned width=cap->canvas_width;c->width=width;
    if(!reuse_hud)memset(c->scene_rows,0,sizeof c->scene_rows);
    if(!reuse_hud && (images[0].width!=width || images[0].height!=224)) return SNESRECOMP_PPU_UNSUPPORTED;
    Lufia2PpuLineClone(&c->reference,live);
    Ppu *p=&c->reference;p->renderBuffer=(uint8_t *)c->canvas;p->renderPitch=width*4u;
    memcpy(p->vram,cap->vram,sizeof p->vram);memcpy(p->cgram,cap->cgram,sizeof p->cgram);
    if(cap->oam) memcpy(p->oam,cap->oam,sizeof p->oam);
    if(cap->high_oam) memcpy(p->highOam,cap->high_oam,sizeof p->highOam);
    p->renderVram=NULL; /* This clone owns the immutable captured VRAM. */
#if defined(LUFIA2_PPU_CAPTURE_DIAGNOSTICS) || defined(LUFIA2_PPU_JOB_TRACE)
    phase[1]=snesrecomp_now_us();
#endif
    unsigned packed_rows,card_pixels;
    c->hud_reused=reuse_hud;
    if(reuse_hud) {
        /* Retain the completed HUD and its background-derived resources.
         * Current scene objects still use this frame's qualified inputs. */
        memcpy(images,c->cached_images,c->cached_image_count*sizeof *images);
        memcpy(overlays,c->cached_overlays,c->cached_overlay_count*sizeof *overlays);
        *image_count=c->cached_image_count;*overlay_count=c->cached_overlay_count;
        packed_rows=c->cached_packed_rows;card_pixels=c->cached_card_pixels;
        for(unsigned y=0;y<224;++y) if(c->scene_rows[y]) {
            memcpy(&p->inidisp,c->scenes[y].regs,PPU_SAVESTATE_REGS_SIZE);
            RefreshSceneObjects(c,p,y+1u);
        }
#if defined(LUFIA2_PPU_CAPTURE_DIAGNOSTICS) || defined(LUFIA2_PPU_JOB_TRACE)
        const uint64_t refresh_end=snesrecomp_now_us();
#endif
        const SnesRecompPpuResult frozen=FreezeDerivedFrame(c,cap,c->cached_band_count);
#if defined(LUFIA2_PPU_CAPTURE_DIAGNOSTICS) || defined(LUFIA2_PPU_JOB_TRACE)
        fprintf(stderr,"Battle reuse us: clone=%llu refresh=%llu freeze=%llu\n",
            (unsigned long long)(phase[1]-phase[0]),(unsigned long long)(refresh_end-phase[1]),
            (unsigned long long)(snesrecomp_now_us()-refresh_end));
#endif
        if(frozen!=SNESRECOMP_PPU_OK) return frozen;
        c->source_count=c->cached_source_count;
        *sources=c->sources;*source_count=c->source_count;
        return SNESRECOMP_PPU_OK;
    } else {
        /* The complete immutable capture is available before any pixel work.
         * If no row can consume a relocated HUD, priority-index generation
         * would only rediscover that fact. Keep all rows in the GPU job. */
        bool may_relocate_hud=false;
        for(unsigned bi=0;bi<cap->band_count && !may_relocate_hud;++bi) {
            const SnesPpuRasterBand *band=cap->bands+bi;
            memcpy(&p->inidisp,band->regs,PPU_SAVESTATE_REGS_SIZE);
            for(unsigned y=band->y_begin;y<band->y_end;++y)
                if(Lufia2BattleEffectsGpuHudMayRelocateLine(p,y+1u)) {
                    may_relocate_hud=true;break;
                }
        }
        bool direct_party=false;
        uint8_t direct_party_rows[224]={0};
        memset(c->hud_rows,0,sizeof c->hud_rows);
        for(unsigned bi=0;bi<cap->band_count;++bi) {
            const SnesPpuRasterBand *band=cap->bands+bi;
            for(unsigned y=band->y_begin;y<band->y_end;++y) {
                RecordedRegisters(p,cap,band,y);
                if(PPU_forcedBlank(p)) {Lufia2PpuUpdateBrightness(p);continue;}
                if(!may_relocate_hud) continue;
                BattleRowInputs inputs={cap,c->qualified_coverage,c->sprite_rows,y,false};
                if(Lufia2BattleEffectsGpuHudLineWithInputs(p,y+1u,Lufia2PpuReplayBattleSourceLine,
                    Lufia2PpuReplayBattleIndexLine,PrepareRecordedInputs,&inputs)) {
                    PrepareRecordedInputs(&inputs,p,y+1u);
                    /* An admitted title/party row returns before any scene draw
                     * or mutation. Its recorded registers/objects are already
                     * installed above; do not restore the same row twice. */
                    c->hud_rows[y]=1;
                    const int direct=Lufia2BattleEffectsGpuPartyLineScoped(p,y+1u,
                        Lufia2PpuReplayBattleSourceLine);
                    if(direct<0) return SNESRECOMP_PPU_UNSUPPORTED;
                    if(direct) {direct_party=true;direct_party_rows[y]=1;c->cache_reject|=0x80000u;}
                    else Lufia2PpuReplayPixelLine(p,y+1u);
                }
            }
        }
        Lufia2BattleUiFinish(p->renderBuffer,p->renderPitch);
        Lufia2BattleUiCardView cards;
        const bool gpu_cards=Lufia2BattleUiReadCards(&cards);
        if(direct_party && !gpu_cards) return SNESRECOMP_PPU_UNSUPPORTED;
        if(!gpu_cards) Lufia2BattleUiCompose(p->renderBuffer,width,224,true);
        memcpy(c->hud_input_rows,c->hud_rows,sizeof c->hud_input_rows);
        for(unsigned y=0;y<224;++y)
            c->hud_object_rows[y]=(uint8_t)(c->hud_input_rows[y] &&
                !(gpu_cards && direct_party_rows[y]));
        /* Completed modern card rows contain only their GPU scene. Retain that
         * scene while omitting transparent CPU post rows from the payload. */
        if(gpu_cards) for(unsigned y=0;y<224;++y) if(c->hud_rows[y] && c->scene_rows[y]) {
            const uint32_t *row=c->canvas+y*width;
            unsigned x=0;while(x<width && !row[x]) ++x;
            if(x==width) c->hud_rows[y]=0;
        }
        packed_rows=0;
        for(unsigned y=0;y<224;) {
            if(!c->hud_rows[y]) {++y;continue;}
            const unsigned begin=y,source_y=packed_rows;
            const bool scene=c->scene_rows[y]!=0;
            while(y<224 && c->hud_rows[y] && (c->scene_rows[y]!=0)==scene) {
                uint32_t *destination=c->packed+packed_rows*width;
                memcpy(destination,c->canvas+y*width,width*sizeof(uint32_t));
                /* One packed image may contain both opaque CPU fallback rows and
                 * transparent GPU-scene primitives. Certify the entire payload as
                 * premultiplied, including rows consumed by REPLACE overlays. */
                if(!scene) for(unsigned x=0;x<width;++x) destination[x]|=0xff000000u;
                ++packed_rows;++y;
            }
            if(*overlay_count==LUFIA2_BATTLE_JOB_OVERLAYS) return SNESRECOMP_PPU_RESOURCE_LIMIT;
            overlays[(*overlay_count)++]=(SnesRecompPpuOverlay){.y=(uint16_t)begin,.width=(uint16_t)width,
                .height=(uint16_t)(y-begin),.source_y=(uint16_t)source_y,.image=1,.blend=(uint8_t)(scene?SNESRECOMP_PPU_OVERLAY_PREMULTIPLIED:SNESRECOMP_PPU_OVERLAY_REPLACE)};
        }
        card_pixels=gpu_cards?sizeof c->cards/sizeof *c->cards:0u;
        *image_count=1;
        if(packed_rows) images[(*image_count)++]=(SnesRecompPpuJobImage){c->packed,width,(uint16_t)width,(uint16_t)packed_rows};
        if(gpu_cards) {
            if(*overlay_count+LUFIA2_BATTLE_UI_CARDS>LUFIA2_BATTLE_JOB_OVERLAYS)
                return SNESRECOMP_PPU_RESOURCE_LIMIT;
            memcpy(c->cards,cards.pixels,sizeof c->cards);
            const uint8_t image=(uint8_t)*image_count;
            images[(*image_count)++]=(SnesRecompPpuJobImage){c->cards,LUFIA2_BATTLE_UI_CARD_WIDTH,
                LUFIA2_BATTLE_UI_CARD_WIDTH,LUFIA2_BATTLE_UI_CARDS*LUFIA2_BATTLE_UI_CARD_HEIGHT};
            for(unsigned card=0;card<LUFIA2_BATTLE_UI_CARDS;++card) {
                const unsigned height=cards.top<224u?224u-cards.top:0u;
                if(!height) continue;
                overlays[(*overlay_count)++]=(SnesRecompPpuOverlay){.x=(uint16_t)cards.left[card],
                    .y=(uint16_t)cards.top,.width=LUFIA2_BATTLE_UI_CARD_WIDTH,
                    .height=(uint16_t)(height<LUFIA2_BATTLE_UI_CARD_HEIGHT?height:LUFIA2_BATTLE_UI_CARD_HEIGHT),
                    .source_y=(uint16_t)(card*LUFIA2_BATTLE_UI_CARD_HEIGHT),.image=image,
                    .blend=SNESRECOMP_PPU_OVERLAY_PREMULTIPLIED};
            }
        }
        c->cached_packed_rows=packed_rows;c->cached_card_pixels=card_pixels;
    }
#if defined(LUFIA2_PPU_CAPTURE_DIAGNOSTICS) || defined(LUFIA2_PPU_JOB_TRACE)
    phase[2]=snesrecomp_now_us();
#endif
    /* Covered CPU rows need no base work. Transparent HUD rows retain their
     * recorded alternate scene on the GPU, before the final opaque primitives. */
    unsigned band_count=0;
    uint8_t window_boundaries[225];
    Lufia2CapturePpuLayoutBoundaries(window_boundaries,live);
    SnesPpuResolvedWindow windows[6];
    const size_t first=offsetof(SnesPpuRasterBand,bg_mode);
    const SnesPpuRasterBand *previous_source=NULL;
    bool previous_covered=false;
    for(unsigned bi=0;bi<cap->band_count;++bi) {
        const SnesPpuRasterBand *original=cap->bands+bi;
        for(unsigned y=original->y_begin;y<original->y_end;++y) {
            const SnesPpuRasterBand *source=c->scene_rows[y]?c->scenes+y:original;
            const bool covered=c->hud_rows[y] && !c->scene_rows[y];
            const bool inherited=previous_source==source && previous_covered==covered;
            previous_source=source;previous_covered=covered;
            /* A repeated immutable source has the same adjusted registers and
             * layer bounds. Window policy can change only at its known cuts. */
            if(band_count && inherited && !window_boundaries[y+1u]) {
                c->bands[band_count-1u].y_end=(uint16_t)(y+1u);continue;
            }
            SnesPpuRasterBand next=*source;
            next.y_begin=(uint16_t)y;next.y_end=(uint16_t)(y+1u);
            memcpy(&p->inidisp,next.regs,PPU_SAVESTATE_REGS_SIZE);
            /* Match the software HUD viewport before exporting tile data.
             * Its BG2 map wraps outside the native centre otherwise. */
            if(PPU_bgTilemapAdr(p,1)==0x5c00u && PPU_bgTileAdr(p,1)==0x4000u) {
                next.bg[1].margin_left=0;
                next.bg[1].margin_right=0;
            }
            if(Lufia2BattleEffectsPlane(p,2u)) {
                next.bg[2].margin_left=(uint8_t)p->extraLeftCur;
                next.bg[2].margin_right=(uint8_t)p->extraRightCur;
            }
            if(covered) {next.forced_blank=true;next.regs[0]|=0x80u;}
            const bool same=band_count &&
                !memcmp((const uint8_t *)(c->bands+band_count-1u)+first,
                    (const uint8_t *)&next+first,sizeof next-first);
            /* Reuse the existing raster-state comparison, without a second
             * row key. Immutable window policy changes only at these cuts. */
            if(same && !window_boundaries[y+1u]) {
                c->bands[band_count-1u].y_end=next.y_end;continue;
            }
            Lufia2BattleEffectsGpuWindows(p,y+1u,windows);
            if(same &&
                !memcmp(c->windows+(band_count-1u)*6u,windows,sizeof windows)) {
                c->bands[band_count-1u].y_end=next.y_end;continue;
            }
            if(band_count==SNES_PPU_MAX_BANDS) return SNESRECOMP_PPU_RESOURCE_LIMIT;
            c->bands[band_count]=next;memcpy(c->windows+band_count*6u,windows,sizeof windows);++band_count;
        }
    }
#if defined(LUFIA2_PPU_CAPTURE_DIAGNOSTICS) || defined(LUFIA2_PPU_JOB_TRACE)
    phase[3]=snesrecomp_now_us();
#endif
    const SnesRecompPpuResult frozen=FreezeDerivedFrame(c,cap,band_count);
    if(frozen!=SNESRECOMP_PPU_OK) return frozen;
    cap=snesrecomp_ppu_frame_capture(c->frame);
#if defined(LUFIA2_PPU_CAPTURE_DIAGNOSTICS) || defined(LUFIA2_PPU_JOB_TRACE)
    phase[4]=snesrecomp_now_us();
#endif
    /* Deferred HUD scenes can replace a row's captured palette. Resolve art
     * from that same completed history, rather than the original snapshot. */
    const uint16_t *art_palette=cap->cgram_lines?cap->cgram_lines+32:cap->cgram+32;
    bool raster_palette=false;
    if(cap->cgram_lines) for(unsigned y=0;y<224;++y)
        if(memcmp(cap->cgram_lines+y*256u+32,art_palette,32*sizeof(uint16_t))) {
            raster_palette=true;break;
        }
    SnesRecompWindowSpan spans[SNESRECOMP_WINDOW_MAX_SPANS];
    unsigned window_count=0,window_band=0;bool window_band_valid=false;
    for(unsigned bi=0;bi<cap->band_count;++bi) {
        const SnesPpuRasterBand *b=cap->bands+bi;
        if(b->forced_blank || b->bg[0].tilemap_word_addr || b->bg[0].char_word_addr!=0x2000u) continue;
        const int offset=(int)(images[0].width-256u)/2-cap->canvas_extra+SignedScroll(b->bg[0].h_scroll);
        const int left=(int)centre_left-offset,right=(int)centre_right-offset;
        const int active_left=(int)cap->canvas_extra-(int)b->bg[0].margin_left;
        const int active_right=(int)cap->canvas_extra+256+(int)b->bg[0].margin_right;
        /* Wave bands mostly share the previous band's window spans. */
        if(!window_band_valid || !snesrecomp_ppu_window_spans_equal(cap,window_band,bi))
            window_count=snesrecomp_ppu_window_spans(cap,b,spans,SNESRECOMP_WINDOW_MAX_SPANS);
        window_band=bi;window_band_valid=true;
        if(!window_count) return SNESRECOMP_PPU_INVALID;
        for(unsigned y=b->y_begin;y<b->y_end;++y) {
            if(c->hud_rows[y] && !c->scene_rows[y]) continue;
            int source_y=(int)y+SignedScroll(b->bg[0].v_scroll+1u);
            if(source_y<0) source_y=0;
            if(source_y>=224) source_y=223;
            uint8_t priorities[2][2]={{0,0},{0,0}};
            for(unsigned side=0;side<2;++side) {
                const int edge=side?right-1:left;
                uint8_t opaque_screens=0,priority=0;
                if(edge>=0 && edge<(int)width && SampleJobEdge(cap,b,spans,window_count,0,
                    (uint16_t)edge,(uint16_t)y,&opaque_screens,&priority)!=SNESRECOMP_PPU_OK) return SNESRECOMP_PPU_UNSUPPORTED;
                priorities[0][side]=(opaque_screens&1u)?priority:0;
                priorities[1][side]=(opaque_screens&2u)?priority:0;
            }
            for(unsigned sub=0;sub<2;++sub) {
                if(!((sub?b->sub_enable:b->main_enable)&1u) || (sub && !(b->cgwsel&2u))) continue;
                if(left>active_left && !SourceRange(c,active_left,left<active_right?left:active_right,offset,y,(unsigned)source_y,
                    images,(uint8_t)(1u<<sub),priorities[sub][0])) return SNESRECOMP_PPU_RESOURCE_LIMIT;
                if(right<active_right && !SourceRange(c,right>active_left?right:active_left,active_right,offset,y,(unsigned)source_y,
                    images,(uint8_t)(1u<<sub),priorities[sub][1])) return SNESRECOMP_PPU_RESOURCE_LIMIT;
            }
        }
    }
#if defined(LUFIA2_PPU_CAPTURE_DIAGNOSTICS) || defined(LUFIA2_PPU_JOB_TRACE)
    phase[5]=snesrecomp_now_us();
#endif
    if(c->source_count) {
        if(raster_palette) {
            /* Only the authored margins need palette resolution. Native tiles,
             * OBJ, windows, colour math and brightness remain GPU work. The
             * completed HUD canvas is free now: its needed rows are packed. */
            memset(c->canvas,0,(size_t)width*224u*sizeof *c->canvas);
            for(unsigned i=0;i<c->source_count;++i) {
                SnesRecompPpuSource *r=c->sources+i;
                for(unsigned y=0;y<r->height;++y) {
                    const unsigned sy=r->source_y+(r->source_height==1?0u:y);
                    if(!Lufia2BattleEffectsGpuArtRow(cap->cgram_lines+(r->y+y)*256u+32,
                        r->source_x,sy,r->width,r->source_width==1,
                        c->canvas+(r->y+y)*width+r->x)) return SNESRECOMP_PPU_UNSUPPORTED;
                }
                r->source_x=r->x;r->source_y=r->y;
                r->source_width=r->width;r->source_height=r->height;
            }
            images[0]=(SnesRecompPpuJobImage){c->canvas,width,(uint16_t)width,224};
        } else if(!Lufia2BattleEffectsGpuArtPalette(live,art_palette,images,
            &centre_left,&centre_right)) return SNESRECOMP_PPU_RESOURCE_LIMIT;
        SnesRecompPpuJobImage compact;
        const SnesRecompPpuResult result=CompactJobArt(images,c->sources,c->source_count,c->compact_art,
            sizeof c->compact_art/sizeof *c->compact_art,&compact);
        if(result==SNESRECOMP_PPU_OK) images[0]=compact;
        else if(result!=SNESRECOMP_PPU_UNSUPPORTED) return result;
    } else {
        static const uint32_t transparent;
        images[0]=(SnesRecompPpuJobImage){&transparent,1,1,1};
    }
    if((uint32_t)images[0].width*images[0].height+(uint32_t)width*packed_rows+card_pixels>
        SNESRECOMP_PPU_JOB_MAX_PIXELS) return SNESRECOMP_PPU_RESOURCE_LIMIT;
#if defined(LUFIA2_PPU_CAPTURE_DIAGNOSTICS) || defined(LUFIA2_PPU_JOB_TRACE)
    phase[6]=snesrecomp_now_us();
    static unsigned phase_reports;
    if(phase_reports++<80u)fprintf(stderr,"Battle phases us: clone=%llu hud=%llu bands=%llu freeze=%llu edges=%llu art=%llu\n",
        (unsigned long long)(phase[1]-phase[0]),(unsigned long long)(phase[2]-phase[1]),
        (unsigned long long)(phase[3]-phase[2]),(unsigned long long)(phase[4]-phase[3]),
        (unsigned long long)(phase[5]-phase[4]),(unsigned long long)(phase[6]-phase[5]));
#endif
    *sources=c->sources;*source_count=c->source_count;return SNESRECOMP_PPU_OK;
}

static SnesRecompPpuResult PrepareWithHud(Lufia2BattlePpuJob *c,
    const Ppu *live,const SnesRecompPpuFrame *frame,SnesRecompPpuJobImage *images,uint16_t *image_count,
    SnesRecompPpuOverlay *overlays,uint16_t *overlay_count,const SnesRecompPpuSource **sources,uint16_t *source_count,bool reuse_hud) {
    if(c) {c->cache_valid=false;c->reused=false;}
    if(!c || !live || !Lufia2BattleEffectsBeginGpuSceneScoped(RecordGpuScene,c)) return SNESRECOMP_PPU_INVALID;
    Lufia2PpuRowPolicy policy;
    Lufia2PpuRowPolicyCapture(&policy,live);
    const Lufia2PpuRowPolicy *previous=Lufia2BattleEffectsBindRowPolicy(&policy);
    const SnesRecompPpuResult result=PrepareInternal(c,live,frame,images,image_count,
        overlays,overlay_count,sources,source_count,reuse_hud);
    Lufia2BattleEffectsBindRowPolicy(previous);
    Lufia2BattleEffectsEndGpuScene();return result;
}
SnesRecompPpuResult Lufia2BattlePpuJobPrepare(Lufia2BattlePpuJob *c,
    const Ppu *live,const SnesRecompPpuFrame *frame,SnesRecompPpuJobImage *images,uint16_t *image_count,
    SnesRecompPpuOverlay *overlays,uint16_t *overlay_count,const SnesRecompPpuSource **sources,uint16_t *source_count) {
    return PrepareWithHud(c,live,frame,images,image_count,overlays,overlay_count,sources,source_count,false);
}

SnesRecompPpuResult Lufia2BattlePpuJobPrepareRecorded(Lufia2BattlePpuJob *c,
    const Ppu *live,const SnesRecompPpuFrame *frame,
    const Lufia2BattleEffectsPixelPolicy *policy,const bool *sprite_rows,
    SnesRecompPpuJobImage *images,uint16_t *image_count,
    SnesRecompPpuOverlay *overlays,uint16_t *overlay_count,
    const SnesRecompPpuSource **sources,uint16_t *source_count) {
    return Lufia2BattlePpuJobPrepareRecordedRows(c,live,frame,policy,sprite_rows,UINT16_MAX,
        images,image_count,overlays,overlay_count,sources,source_count);
}
static bool HudInputsMatch(const Lufia2BattlePpuJob *c,const SnesPpuFrameCapture *cap,
    const BattleJobPolicyKey *key,const Lufia2BattleEffectsPixelPolicy *policy) {
    if(!cap->obj_pixels || !!policy->scripts!=!!c->cached_effects.scripts ||
        key->art!=c->policy_key.art || key->last_brightness!=c->policy_key.last_brightness ||
        key->first_sprite_line!=c->policy_key.first_sprite_line ||
        !snesrecomp_u8_equal((const uint8_t *)&key->row,
            (const uint8_t *)&c->policy_key.row,sizeof key->row) ||
        !snesrecomp_u8_equal(key->layout,c->policy_key.layout,sizeof key->layout)) return false;
    for(unsigned y=0;y<224;++y) if(c->hud_input_rows[y]) {
        /* A CPU-rendered alternate scene may consult uncaptured actor rows.
         * Those retain full preparation until that separate view is owned. */
        if(!c->scene_rows[y]) return false;
        /* With an owned GPU scene, DrawHudOverlay copies CPU colours only
         * from its original 256-pixel centre. SpriteVisible always admits
         * centre pixels; the ownership mask clips only the two margins.
         * Current scene OBJ/masks are rebuilt independently below. */
        if(c->hud_object_rows[y] &&
            !snesrecomp_u16_equal(c->cached_primary_objects+y*cap->canvas_width+cap->canvas_extra,
                cap->obj_pixels+y*cap->obj_pitch+cap->canvas_extra,256u)) return false;
    }
    return true;
}
SnesRecompPpuResult Lufia2BattlePpuJobPrepareRecordedRows(Lufia2BattlePpuJob *c,
    const Ppu *live,const SnesRecompPpuFrame *frame,
    const Lufia2BattleEffectsPixelPolicy *policy,const bool *sprite_rows,uint16_t first_sprite_line,
    SnesRecompPpuJobImage *images,uint16_t *image_count,
    SnesRecompPpuOverlay *overlays,uint16_t *overlay_count,
    const SnesRecompPpuSource **sources,uint16_t *source_count) {
    if(!c || !live || !frame || !policy || !sprite_rows || !images || !image_count ||
        !overlays || !overlay_count || !sources || !source_count) return SNESRECOMP_PPU_INVALID;
    c->reused=c->hud_reused=false;c->cache_reject=0;
#if defined(LUFIA2_PPU_JOB_TRACE)
    const uint64_t wrapper_begin=snesrecomp_now_us();
#endif
    BattleJobPolicyKey key;
    const SnesPpuFrameCapture *input=snesrecomp_ppu_frame_capture(frame);
    const bool cacheable=c->input_key && frame!=c->frame && input && input->obj_pixels && input->visible_height==224u &&
        CachePolicy(&key,live,first_sprite_line);
    const bool effects_equal=cacheable && c->cache_valid &&
        Lufia2BattleEffectsPixelRenderPoliciesEqual(policy,&c->cached_effects);
    const bool masks_equal=effects_equal && (!policy->scripts ||
        snesrecomp_u8_equal((const uint8_t *)sprite_rows,
            (const uint8_t *)c->cached_sprite_rows,sizeof c->cached_sprite_rows));
    if(cacheable && c->cache_valid && effects_equal && masks_equal &&
        snesrecomp_u8_equal((const uint8_t *)&key,(const uint8_t *)&c->policy_key,sizeof key) &&
        snesrecomp_ppu_frame_key_matches(c->input_key,frame) &&
        (c->cached_cards_ready ? Lufia2BattleUiReuseCards(c->cards,c->cached_card_top) :
            Lufia2BattleUiReuseEmpty())) {
        memcpy(images,c->cached_images,c->cached_image_count*sizeof *images);
        memcpy(overlays,c->cached_overlays,c->cached_overlay_count*sizeof *overlays);
        *image_count=c->cached_image_count;*overlay_count=c->cached_overlay_count;
        *source_count=c->cached_source_count;*sources=c->sources;
        c->reused=true;++c->cache_hits;return SNESRECOMP_PPU_OK;
    }
    ++c->cache_misses;
    if(!c->cache_valid) c->cache_reject|=1u;
    if(!cacheable) c->cache_reject|=4u;
    if(cacheable && c->cache_valid && !effects_equal) c->cache_reject|=2u;
    if(effects_equal && !masks_equal) c->cache_reject|=8192u;
    if(cacheable && c->cache_valid) {
        const SnesRecompPpuFrameVersions *v=snesrecomp_ppu_frame_versions(frame);
        if(v) {
            if(v->vram!=c->input_versions.vram) c->cache_reject|=16u;
            if(v->palette!=c->input_versions.palette) c->cache_reject|=32u;
            if(v->objects!=c->input_versions.objects) c->cache_reject|=64u;
        }
        if(!snesrecomp_u8_equal((const uint8_t *)&key,(const uint8_t *)&c->policy_key,sizeof key)) {
            c->cache_reject|=8u;
            if(memcmp(key.row.registers,c->policy_key.row.registers,sizeof key.row.registers)) c->cache_reject|=256u;
            if(memcmp(key.row.policy_head,c->policy_key.row.policy_head,sizeof key.row.policy_head)) c->cache_reject|=512u;
            if(memcmp(key.row.policy_tail,c->policy_key.row.policy_tail,sizeof key.row.policy_tail)) c->cache_reject|=1024u;
            if(memcmp(key.object_policy,c->policy_key.object_policy,sizeof key.object_policy)) c->cache_reject|=2048u;
            if(memcmp(key.layout,c->policy_key.layout,sizeof key.layout)) c->cache_reject|=4096u;
        }
        if(!c->cache_reject) {
            /* Other captured metadata or UI admission; the high bits name the part. */
            c->cache_reject=128u;
            c->cache_reject|=snesrecomp_ppu_frame_key_mismatch(c->input_key,frame)<<20;
            if(!(c->cached_cards_ready ? Lufia2BattleUiReuseCards(c->cards,c->cached_card_top) :
                    Lufia2BattleUiReuseEmpty())) c->cache_reject|=0x8000u;
        }
    }
    const bool reuse_hud=cacheable && c->cache_valid &&
        snesrecomp_ppu_frame_key_matches_background(c->input_key,frame) &&
        HudInputsMatch(c,input,&key,policy) &&
        (c->cached_cards_ready ? Lufia2BattleUiReuseCards(c->cards,c->cached_card_top) :
            Lufia2BattleUiReuseEmpty());
    if(reuse_hud)c->cache_reject|=0x40000u; /* HUD reused; objects are refreshed. */
    /* A failed or partial rebuild must never validate an earlier output. */
    c->cache_valid=false;
    bool previous_sprites[kPpuBufWidth];
    Lufia2BattleEffectsCaptureSpriteRow(previous_sprites);
    const Lufia2BattleEffectsPixelPolicy *previous=Lufia2BattleEffectsBindPixelPolicy(policy);
    c->sprite_rows=sprite_rows;
    c->first_sprite_line=first_sprite_line;c->qualified_rows=first_sprite_line!=UINT16_MAX;
#if defined(LUFIA2_PPU_JOB_TRACE)
    const uint64_t wrapper_internal=snesrecomp_now_us();
#endif
    const SnesRecompPpuResult result=PrepareWithHud(c,live,frame,images,image_count,
        overlays,overlay_count,sources,source_count,reuse_hud);
#if defined(LUFIA2_PPU_JOB_TRACE)
    const uint64_t wrapper_commit=snesrecomp_now_us();
#endif
#if defined(LUFIA2_PPU_CAPTURE_DIAGNOSTICS)
    static unsigned reports;
    static bool root_checked;
    static const char *root;
    if (!root_checked) { root = getenv("LUFIA2_PPU_JOB_TRACE_ROOT"); root_checked = true; }
    if (root && *root && reports < 40u &&
        image_count && overlay_count && source_count) {
        ++reports;
        unsigned pixels = 0u;
        if (images) for (unsigned i = 0; i < *image_count && i < LUFIA2_BATTLE_JOB_IMAGES; ++i)
            pixels += (unsigned)images[i].width * images[i].height;
        fprintf(stderr, "Battle prepare result=%u images=%u overlays=%u sources=%u partial_sources=%u pixels=%u\n",
            (unsigned)result, *image_count, *overlay_count, *source_count, c->source_count, pixels);
    }
#endif
    if(cacheable && result==SNESRECOMP_PPU_OK &&
        *image_count<=LUFIA2_BATTLE_JOB_IMAGES && *overlay_count<=LUFIA2_BATTLE_JOB_OVERLAYS &&
        (!*source_count || images[0].pixels==c->compact_art)) {
        Lufia2BattleUiCardView cards;
        const bool cards_ready=Lufia2BattleUiReadCards(&cards);
        /* Incomplete modern cards are already restored into the ordinary
         * packed overlay. Reuse that exact finished output, never partial UI. */
        if((cards_ready || !Lufia2BattleUiActive()) &&
            snesrecomp_ppu_frame_key_commit(c->input_key,frame)) {
            for(unsigned y=0;y<224;++y) if(c->hud_object_rows[y])
                memcpy(c->cached_primary_objects+y*input->canvas_width+input->canvas_extra,
                    input->obj_pixels+y*input->obj_pitch+input->canvas_extra,256u*sizeof(uint16_t));
            c->cached_effects=*policy;
            if(policy->scripts) memcpy(c->cached_sprite_rows,sprite_rows,sizeof c->cached_sprite_rows);
            c->policy_key=key;c->input_versions=*snesrecomp_ppu_frame_versions(frame);c->cached_cards_ready=cards_ready;c->cached_card_top=cards_ready?cards.top:0;
            c->cached_image_count=*image_count;c->cached_overlay_count=*overlay_count;
            c->cached_source_count=*source_count;
            c->cached_band_count=snesrecomp_ppu_frame_capture(c->frame)->band_count;
            memcpy(c->cached_images,images,*image_count*sizeof *images);
            memcpy(c->cached_overlays,overlays,*overlay_count*sizeof *overlays);
            c->cache_valid=true;
        }
    }
    if(cacheable && result==SNESRECOMP_PPU_OK && !c->cache_valid) {
        if(*source_count && images[0].pixels!=c->compact_art) c->cache_reject|=0x20000u;
        Lufia2BattleUiCardView view;
        if(!Lufia2BattleUiReadCards(&view)) c->cache_reject|=0x10000u;
    }
    c->sprite_rows=NULL;
    c->qualified_capture=NULL;c->qualified_rows=false;
    c->qualified_coverage=NULL;
    Lufia2BattleEffectsBindPixelPolicy(previous);
    Lufia2BattleEffectsRestoreSpriteRow(previous_sprites);
#if defined(LUFIA2_PPU_JOB_TRACE)
    fprintf(stderr,"Battle wrapper us: prefix=%llu internal=%llu commit=%llu\n",
        (unsigned long long)(wrapper_internal-wrapper_begin),
        (unsigned long long)(wrapper_commit-wrapper_internal),
        (unsigned long long)(snesrecomp_now_us()-wrapper_commit));
#endif
    return result;
}

/* The helper consumes only final host policy here. Pixel memory and qualified
 * rows come from the separately owned frame/offload capture, never this copy.
 * Refuse borrowed render views rather than retaining a live PPU pointer. */
bool Lufia2BattlePpuJobSnapshotPolicy(Ppu *out,const Ppu *live) {
    if(!out || !live || out==live || !Lufia2BattleEffectsGpuCompatible(live)) return false;
    for(unsigned i=0;i<kPpuOverlaySource_Count;++i)
        if(live->overlayRenderBuffer[i] || live->overlayCaptures[i].flags) return false;
    Lufia2PpuLineClone(out,live);
    out->renderVram=NULL;out->renderBuffer=NULL;out->renderPitch=0;
    out->widescreenLineEnhancerContext=NULL;
    /* Their disabled scratch is intentionally not copied by LineClone. */
    memset(out->overlayRenderBuffer,0,sizeof out->overlayRenderBuffer);
    memset(out->overlayCaptures,0,sizeof out->overlayCaptures);
    return true;
}

/* Pure host preparation; caller owns policy/capture and excludes every other
 * renderer use until completion. Guest execution must never enter this half. */
bool Lufia2BattlePpuJobFreeze(Lufia2BattlePpuJob *c,Lufia2PpuOffload *offload,
    const Ppu *live,const SnesRecompPpuFrame *frame) {
    if(!c || !offload || !live || !frame) return false;
    SnesRecompPpuJobImage images[LUFIA2_BATTLE_JOB_IMAGES];
    SnesRecompPpuOverlay overlays[LUFIA2_BATTLE_JOB_OVERLAYS];
    const SnesRecompPpuSource *sources=NULL;const bool *sprite_rows=NULL;
    uint16_t image_count=0,overlay_count=0,source_count=0;
    const Lufia2BattleEffectsPixelPolicy *policy=Lufia2PpuOffloadBattlePolicy(offload,&sprite_rows);
#if defined(LUFIA2_PPU_JOB_TRACE)
    const uint64_t freeze_begin=snesrecomp_now_us();
    const bool prepared=Lufia2BattlePpuJobPrepareRecordedRows(c,live,frame,policy,sprite_rows,0,
        images,&image_count,overlays,&overlay_count,&sources,&source_count)==SNESRECOMP_PPU_OK;
    const uint64_t freeze_payload=snesrecomp_now_us();
    const bool done=prepared &&
        (((c->reused || c->hud_reused) && Lufia2PpuOffloadRebindResolvedSources(offload,c->frame)) ||
            Lufia2PpuOffloadFreezeResolvedSources(offload,c->frame,images,image_count,overlays,
                overlay_count,sources,source_count));
    fprintf(stderr,"Battle jobfreeze us: prepare=%llu payload=%llu\n",
        (unsigned long long)(freeze_payload-freeze_begin),
        (unsigned long long)(snesrecomp_now_us()-freeze_payload));
    return done;
#else
    return Lufia2BattlePpuJobPrepareRecordedRows(c,live,frame,policy,sprite_rows,0,images,&image_count,
        overlays,&overlay_count,&sources,&source_count)==SNESRECOMP_PPU_OK &&
        (((c->reused || c->hud_reused) && Lufia2PpuOffloadRebindResolvedSources(offload,c->frame)) ||
            Lufia2PpuOffloadFreezeResolvedSources(offload,c->frame,images,image_count,overlays,
                overlay_count,sources,source_count));
#endif
}
/* A refused battle frame restarts the effect pixels. */
void Lufia2BattlePpuJobRefused(const Ppu *live,const SnesRecompPpuFrame *frame) {
    const SnesPpuFrameCapture *cap=snesrecomp_ppu_frame_capture(frame);
    Lufia2BattleEffectsRestartPixels(cap?cap->canvas_width:256u+2u*live->extraLeftRight,224u);
}
bool Lufia2BattlePpuJobSubmit(Lufia2BattlePpuJob *c,Lufia2PpuOffload *offload,
    const Ppu *live,const SnesRecompPpuFrame *frame) {
    if(!c || !offload || !live || !frame) return false;
    SnesRecompPpuJobImage images[LUFIA2_BATTLE_JOB_IMAGES];
    SnesRecompPpuOverlay overlays[LUFIA2_BATTLE_JOB_OVERLAYS];
    const SnesRecompPpuSource *sources=NULL;const bool *sprite_rows=NULL;
    uint16_t image_count=0,overlay_count=0,source_count=0;
    const Lufia2BattleEffectsPixelPolicy *policy=Lufia2PpuOffloadBattlePolicy(offload,&sprite_rows);
    /* ppu_runLine qualifies OBJ at line - 1 before RecordLine freezes it. */
    if(Lufia2BattlePpuJobPrepareRecordedRows(c,live,frame,policy,sprite_rows,0,images,&image_count,
        overlays,&overlay_count,&sources,&source_count)==SNESRECOMP_PPU_OK &&
        (((c->reused || c->hud_reused) && Lufia2PpuOffloadRebindResolvedSources(offload,c->frame)) ||
            Lufia2PpuOffloadFreezeResolvedSources(offload,c->frame,images,image_count,overlays,
                overlay_count,sources,source_count)) && Lufia2PpuOffloadPrepareFrozen(offload) &&
        Lufia2PpuOffloadSubmit(offload)) return true;
    const SnesPpuFrameCapture *cap=snesrecomp_ppu_frame_capture(frame);
    Lufia2BattleEffectsRestartPixels(cap?cap->canvas_width:256u+2u*live->extraLeftRight,224u);
    return false;
}
