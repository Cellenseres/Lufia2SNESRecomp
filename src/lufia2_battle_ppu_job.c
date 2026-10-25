#include "lufia2_battle_ppu_job.h"
#include "lufia2_battle_effects.h"
#include "lufia2_battle_ui.h"
#include "lufia2_ppu_offload.h"
#include "lufia2_runtime.h"
#include "lufia2_ppu_render_clone.h"
#include "lufia2_ppu_row_cache.h"
#include "snesrecomp_platform/snes_ppu_tile_plan.h"
#include "snesrecomp_platform/snes_ppu_source_compact.h"
#include <stdlib.h>
#include <string.h>
#if defined(LUFIA2_PPU_CAPTURE_DIAGNOSTICS)
#include <stdio.h>
#endif

struct Lufia2BattlePpuJob {
    SnesRecompPpuFrame *frame;
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
};
Lufia2BattlePpuJob *Lufia2BattlePpuJobCreate(void) {
    Lufia2BattlePpuJob *c=calloc(1,sizeof *c);
    if(c) {c->frame=snesrecomp_ppu_frame_create();if(!c->frame) {free(c);return NULL;}}
    return c;
}
void Lufia2BattlePpuJobDestroy(Lufia2BattlePpuJob *c) {
    if(c) {snesrecomp_ppu_frame_release(c->frame);free(c);}
}
const SnesRecompPpuFrame *Lufia2BattlePpuJobFrame(const Lufia2BattlePpuJob *c) {return c?c->frame:NULL;}
static int SignedScroll(unsigned value) { value&=1023u;return value<512u?(int)value:(int)value-1024; }
static bool AddSource(Lufia2BattlePpuJob *c,SnesRecompPpuSource next) {
    for(unsigned i=0;i<c->source_count;++i) {
        SnesRecompPpuSource *r=c->sources+i;
        if(r->x!=next.x || r->width!=next.width || r->y+r->height!=next.y ||
            r->source_x!=next.source_x || r->source_width!=next.source_width ||
            r->screens!=next.screens || r->priority!=next.priority) continue;
        if(r->source_height==r->height && r->source_y+r->height==next.source_y) {
            ++r->height;r->source_height=r->height;return true;
        }
        if(r->source_height==1 && r->source_y==next.source_y) {++r->height;return true;}
    }
    if(c->source_count==SNESRECOMP_PPU_JOB_MAX_SOURCES) return false;
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
    if(!RecordedSceneObjects(c,p,line)) Lufia2PpuQualifyBattleSourceLine(p,line);
    const unsigned y=line-1u,extra=(c->width-256u)/2u;
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
    if(!p->lineHasSprites) memset(c->objects+y*c->width,0,c->width*sizeof *c->objects);
    else for(unsigned x=0;x<c->width;++x) {
        const int logical=(int)x-(int)extra;
        const unsigned native=p->objBuffer.data[logical+kPpuExtraLeftRight];
        c->objects[y*c->width+x]=p->lineHasSprites && (native&255u) &&
            Lufia2BattleEffectsSpriteVisible(p,logical)?
            (uint16_t)((native&255u)|(((native>>12)-2u)/4u<<8)|
                (((native>>8)&15u)==4u?0x400u:0u)|0x800u):0;
    }
    c->scene_rows[y]=1;return true;
}
static SnesRecompPpuResult PrepareInternal(Lufia2BattlePpuJob *c,
    const Ppu *live,const SnesRecompPpuFrame *frame,SnesRecompPpuJobImage *images,uint16_t *image_count,
    SnesRecompPpuOverlay *overlays,uint16_t *overlay_count,const SnesRecompPpuSource **sources,uint16_t *source_count) {
    const SnesPpuFrameCapture *cap=snesrecomp_ppu_frame_capture(frame);
    if(!c || !live || !cap || !images || !image_count || !overlays || !overlay_count || !sources || !source_count)
        return SNESRECOMP_PPU_INVALID;
    *image_count=*overlay_count=*source_count=0;*sources=NULL;c->source_count=0;
    if(!Lufia2BattleEffectsGpuCompatible(live) || cap->visible_height!=224 || lufia2_ppu_offload_recording ||
        cap->canvas_width!=256u+2u*live->extraLeftRight || cap->canvas_extra!=live->extraLeftRight ||
        snesrecomp_ppu_mode1_raw_supports(cap)!=SNES_PPU_SUPPORTED) return SNESRECOMP_PPU_UNSUPPORTED;
    c->qualified_capture=cap;
    c->qualified_coverage=snesrecomp_ppu_frame_obj_coverage(frame);
    c->palette_override=false;
    memset(c->qualified_bands,0,sizeof c->qualified_bands);
    for(unsigned bi=0;bi<cap->band_count;++bi)
        for(unsigned y=cap->bands[bi].y_begin;y<cap->bands[bi].y_end;++y)
            c->qualified_bands[y]=cap->bands+bi;
    unsigned centre_left,centre_right;
    if(!Lufia2BattleEffectsGpuArtPalette(live,NULL,
        images,&centre_left,&centre_right)) return SNESRECOMP_PPU_RESOURCE_LIMIT;
    const unsigned width=cap->canvas_width;c->width=width;
    memset(c->scene_rows,0,sizeof c->scene_rows);
    for(unsigned y=0;y<224;++y) {
        if(cap->obj_pixels) memcpy(c->objects+y*width,cap->obj_pixels+y*cap->obj_pitch,width*sizeof(uint16_t));
        else memset(c->objects+y*width,0,width*sizeof(uint16_t));
    }
    if(images[0].width!=width || images[0].height!=224) return SNESRECOMP_PPU_UNSUPPORTED;
    Lufia2PpuLineClone(&c->reference,live);
    Ppu *p=&c->reference;p->renderBuffer=(uint8_t *)c->canvas;p->renderPitch=width*4u;
    memcpy(p->vram,cap->vram,sizeof p->vram);memcpy(p->cgram,cap->cgram,sizeof p->cgram);
    if(cap->oam) memcpy(p->oam,cap->oam,sizeof p->oam);
    if(cap->high_oam) memcpy(p->highOam,cap->high_oam,sizeof p->highOam);
    p->renderVram=NULL; /* This clone owns the immutable captured VRAM. */
    memset(c->hud_rows,0,sizeof c->hud_rows);
    for(unsigned bi=0;bi<cap->band_count;++bi) {
        const SnesPpuRasterBand *band=cap->bands+bi;
        for(unsigned y=band->y_begin;y<band->y_end;++y) {
            RecordedRegisters(p,cap,band,y);
            if(PPU_forcedBlank(p)) {Lufia2PpuUpdateBrightness(p);continue;}
            BattleRowInputs inputs={cap,c->qualified_coverage,c->sprite_rows,y,false};
            if(Lufia2BattleEffectsGpuHudLineWithInputs(p,y+1u,Lufia2PpuReplayBattleSourceLine,
                Lufia2PpuReplayBattleIndexLine,PrepareRecordedInputs,&inputs)) {
                PrepareRecordedInputs(&inputs,p,y+1u);
                /* An admitted title/party row returns before any scene draw
                 * or mutation. Its recorded registers/objects are already
                 * installed above; do not restore the same row twice. */
                c->hud_rows[y]=1;Lufia2PpuReplayPixelLine(p,y+1u);
            }
        }
    }
    Lufia2BattleUiFinish(p->renderBuffer,p->renderPitch);
    Lufia2BattleUiCardView cards;
    const bool gpu_cards=Lufia2BattleUiReadCards(&cards);
    if(!gpu_cards) Lufia2BattleUiCompose(p->renderBuffer,width,224,true);
    /* Completed modern card rows contain only their GPU scene. Retain that
     * scene while omitting transparent CPU post rows from the payload. */
    if(gpu_cards) for(unsigned y=0;y<224;++y) if(c->hud_rows[y] && c->scene_rows[y]) {
        const uint32_t *row=c->canvas+y*width;
        unsigned x=0;while(x<width && !row[x]) ++x;
        if(x==width) c->hud_rows[y]=0;
    }
    unsigned packed_rows=0;
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
    const unsigned card_pixels=gpu_cards?sizeof c->cards/sizeof *c->cards:0u;
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
    SnesPpuFrameCapture resolved=*cap;resolved.bands=c->bands;resolved.band_count=band_count;
    if(c->palette_override) {
        resolved.cgram_lines=c->scene_palettes[0];
        resolved.cgram_line_count=224;
        resolved.raster_memory_flags|=SNES_PPU_RASTER_MEMORY_CGRAM;
    }
    resolved.resolved_windows=c->windows;resolved.obj_pixels=c->objects;resolved.obj_pitch=width;
    const SnesRecompPpuResult frozen=snesrecomp_ppu_frame_freeze(c->frame,&resolved,NULL,0);
    if(frozen!=SNESRECOMP_PPU_OK) return frozen;
    cap=snesrecomp_ppu_frame_capture(c->frame);
    /* Deferred HUD scenes can replace a row's captured palette. Resolve art
     * from that same completed history, rather than the original snapshot. */
    const uint16_t *art_palette=cap->cgram_lines?cap->cgram_lines+32:cap->cgram+32;
    bool raster_palette=false;
    if(cap->cgram_lines) for(unsigned y=0;y<224;++y)
        if(memcmp(cap->cgram_lines+y*256u+32,art_palette,32*sizeof(uint16_t))) {
            raster_palette=true;break;
        }
    for(unsigned bi=0;bi<cap->band_count;++bi) {
        const SnesPpuRasterBand *b=cap->bands+bi;
        if(b->forced_blank || b->bg[0].tilemap_word_addr || b->bg[0].char_word_addr!=0x2000u) continue;
        const int offset=(int)(images[0].width-256u)/2-cap->canvas_extra+SignedScroll(b->bg[0].h_scroll);
        const int left=(int)centre_left-offset,right=(int)centre_right-offset;
        const int active_left=(int)cap->canvas_extra-(int)b->bg[0].margin_left;
        const int active_right=(int)cap->canvas_extra+256+(int)b->bg[0].margin_right;
        SnesRecompWindowSpan windows[SNESRECOMP_WINDOW_MAX_SPANS];
        const unsigned window_count=snesrecomp_ppu_window_spans(cap,b,windows,SNESRECOMP_WINDOW_MAX_SPANS);
        if(!window_count) return SNESRECOMP_PPU_INVALID;
        for(unsigned y=b->y_begin;y<b->y_end;++y) {
            if(c->hud_rows[y] && !c->scene_rows[y]) continue;
            int source_y=(int)y+SignedScroll(b->bg[0].v_scroll+1u);
            if(source_y<0) source_y=0;
            if(source_y>=224) source_y=223;
            uint8_t priorities[2][2]={{0,0},{0,0}};
            for(unsigned sub=0;sub<2;++sub) for(unsigned side=0;side<2;++side) {
                const int edge=side?right-1:left;
                bool opaque=false;uint8_t priority=0;
                if(edge>=0 && edge<(int)width && snesrecomp_ppu_mode1_layer_sample_windows(cap,b,windows,window_count,0,sub,
                    (uint16_t)edge,(uint16_t)y,&opaque,&priority)!=SNESRECOMP_PPU_OK) return SNESRECOMP_PPU_UNSUPPORTED;
                priorities[sub][side]=opaque?priority:0;
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
        const SnesRecompPpuResult result=snesrecomp_ppu_source_image_compact(images,0,
            c->sources,c->source_count,c->compact_art,sizeof c->compact_art/sizeof *c->compact_art,
            SNESRECOMP_PPU_SOURCE_COMPACT_EDGE_TILES,&compact,c->sources);
        if(result==SNESRECOMP_PPU_OK) images[0]=compact;
        else if(result!=SNESRECOMP_PPU_UNSUPPORTED) return result;
    } else {
        static const uint32_t transparent;
        images[0]=(SnesRecompPpuJobImage){&transparent,1,1,1};
    }
    if((uint32_t)images[0].width*images[0].height+(uint32_t)width*packed_rows+card_pixels>
        SNESRECOMP_PPU_JOB_MAX_PIXELS) return SNESRECOMP_PPU_RESOURCE_LIMIT;
    *sources=c->sources;*source_count=c->source_count;return SNESRECOMP_PPU_OK;
}

SnesRecompPpuResult Lufia2BattlePpuJobPrepare(Lufia2BattlePpuJob *c,
    const Ppu *live,const SnesRecompPpuFrame *frame,SnesRecompPpuJobImage *images,uint16_t *image_count,
    SnesRecompPpuOverlay *overlays,uint16_t *overlay_count,const SnesRecompPpuSource **sources,uint16_t *source_count) {
    if(!c || !live || !Lufia2BattleEffectsBeginGpuScene(RecordGpuScene,c)) return SNESRECOMP_PPU_INVALID;
    Lufia2PpuRowPolicy policy;
    Lufia2PpuRowPolicyCapture(&policy,live);
    const Lufia2PpuRowPolicy *previous=Lufia2BattleEffectsBindRowPolicy(&policy);
    const SnesRecompPpuResult result=PrepareInternal(c,live,frame,images,image_count,
        overlays,overlay_count,sources,source_count);
    Lufia2BattleEffectsBindRowPolicy(previous);
    Lufia2BattleEffectsEndGpuScene();return result;
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
SnesRecompPpuResult Lufia2BattlePpuJobPrepareRecordedRows(Lufia2BattlePpuJob *c,
    const Ppu *live,const SnesRecompPpuFrame *frame,
    const Lufia2BattleEffectsPixelPolicy *policy,const bool *sprite_rows,uint16_t first_sprite_line,
    SnesRecompPpuJobImage *images,uint16_t *image_count,
    SnesRecompPpuOverlay *overlays,uint16_t *overlay_count,
    const SnesRecompPpuSource **sources,uint16_t *source_count) {
    if(!c || !policy || !sprite_rows) return SNESRECOMP_PPU_INVALID;
    bool previous_sprites[kPpuBufWidth];
    Lufia2BattleEffectsCaptureSpriteRow(previous_sprites);
    const Lufia2BattleEffectsPixelPolicy *previous=Lufia2BattleEffectsBindPixelPolicy(policy);
    c->sprite_rows=sprite_rows;
    c->first_sprite_line=first_sprite_line;c->qualified_rows=first_sprite_line!=UINT16_MAX;
    const SnesRecompPpuResult result=Lufia2BattlePpuJobPrepare(c,live,frame,images,image_count,
        overlays,overlay_count,sources,source_count);
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
    c->sprite_rows=NULL;
    c->qualified_capture=NULL;c->qualified_rows=false;
    c->qualified_coverage=NULL;
    Lufia2BattleEffectsBindPixelPolicy(previous);
    Lufia2BattleEffectsRestoreSpriteRow(previous_sprites);
    return result;
}

/* Guest-thread half of Submit: rows, sources and the job freeze. */
bool Lufia2BattlePpuJobFreeze(Lufia2BattlePpuJob *c,Lufia2PpuOffload *offload,
    const Ppu *live,const SnesRecompPpuFrame *frame) {
    if(!c || !offload || !live || !frame) return false;
    SnesRecompPpuJobImage images[LUFIA2_BATTLE_JOB_IMAGES];
    SnesRecompPpuOverlay overlays[LUFIA2_BATTLE_JOB_OVERLAYS];
    const SnesRecompPpuSource *sources=NULL;const bool *sprite_rows=NULL;
    uint16_t image_count=0,overlay_count=0,source_count=0;
    const Lufia2BattleEffectsPixelPolicy *policy=Lufia2PpuOffloadBattlePolicy(offload,&sprite_rows);
    return Lufia2BattlePpuJobPrepareRecordedRows(c,live,frame,policy,sprite_rows,0,images,&image_count,
        overlays,&overlay_count,&sources,&source_count)==SNESRECOMP_PPU_OK &&
        Lufia2PpuOffloadFreezeResolvedSources(offload,c->frame,images,image_count,overlays,
            overlay_count,sources,source_count);
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
        Lufia2PpuOffloadPrepareResolvedSources(offload,c->frame,images,image_count,overlays,
            overlay_count,sources,source_count) && Lufia2PpuOffloadSubmit(offload)) return true;
    const SnesPpuFrameCapture *cap=snesrecomp_ppu_frame_capture(frame);
    Lufia2BattleEffectsRestartPixels(cap?cap->canvas_width:256u+2u*live->extraLeftRight,224u);
    return false;
}
