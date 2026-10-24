#include "lufia2_ppu_offload.h"
#include "lufia2_runtime.h"
#include "lufia2_battle_effects.h"
#include "lufia2_menu_ui.h"
#include "lufia2_ppu_elastic_certificate.h"
#include "lufia2_ppu_obj_capture.h"
#include "lufia2_ppu_obj_defer.h"
#include "lufia2_ppu_host.h"
#include "lufia2_ppu_render_clone.h"
#include "snes/ppu.h"
#include "snes/ws_shadow.h"
#include "snesrecomp_platform/task.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#if defined(LUFIA2_PPU_CAPTURE_DIAGNOSTICS)
#include <stdio.h>
#include "snesrecomp_platform/snes_ppu_trace.h"
#endif

struct Lufia2PpuOffload {
    SnesRecompPresenter *presenter;
    SnesRecompPpuFrame *frame;
    SnesRecompPpuFrame *captured_frame;
    SnesRecompPpuJob *job;
    SnesRecompPpuFrame *graphics_frame;
    SnesRecompPpuJob *primitive_job;
    bool primitive_menu;
    SnesRecompPpuJob *indexed_job;
    bool indexed_menu;
    bool indexed_pixels;
    Lufia2PpuObjCapture selected_obj;
    SnesRecompPpuObjRun selected_runs[SNESRECOMP_OBJ_MAX_SLIVERS];
    Ppu reference, *live;
    Lufia2PpuElasticCertificate elastic;
    Lufia2BattleEffectsPixelPolicy battle_policy;
    bool battle_sprite_rows[LUFIA2_PPU_VISIBLE_LINES][kPpuBufWidth];
    bool battle;
    PpuPixelPrioBufs objects[LUFIA2_PPU_VISIBLE_LINES];
    union {
        uint16_t obj_pixels[SNESRECOMP_PPU_MAX_WIDTH * LUFIA2_PPU_VISIBLE_LINES];
        struct {
            SnesRecompObjSliver slivers[SNESRECOMP_OBJ_MAX_SLIVERS];
            uint8_t priorities[LUFIA2_PPU_VISIBLE_LINES];
        } qualified_objects;
    };
    uint8_t has_sprites[LUFIA2_PPU_VISIBLE_LINES], mosaic_start[LUFIA2_PPU_VISIBLE_LINES];
    unsigned width, extra, lines;
    uint16_t palettes[LUFIA2_PPU_VISIBLE_LINES][SNES_PPU_CGRAM_ENTRIES];
    bool record_palettes;
    uint8_t shadow_maps[2][SNESRECOMP_PPU_RESOLVED_MAP_BYTES];
    uint8_t shadow_known[2][64 * 64];
    Lufia2PpuShadowMemo shadow_memo[2];
    unsigned shadow_mask;
    uint64_t sequence;
    uint64_t capture_us, fallback_us;
    uint32_t timing_valid;
    SnesRecompPpuResult result;
    SnesPpuUnsupported unsupported;
    bool deferred, prepared, clear_destination, menu, menu_available;
    SnesPpuRasterBand menu_bands[SNES_PPU_MAX_BANDS];
    /* Frozen work for PrepareFrozen: a job, else the frame. */
    SnesRecompPpuJob *frozen_job;
    const SnesRecompPpuFrame *frozen_frame;
};
/* One game timeline records at a time; GPU ownership stays in the presenter. */
static Lufia2PpuOffload *s_recording;
bool lufia2_ppu_offload_recording;
static void SetRecording(Lufia2PpuOffload *context) {
    /* Every exit from recording joins the sprite helper first. */
    Lufia2PpuObjDeferStop();
    if(s_recording && lufia2_ppu_obj_capture==&s_recording->selected_obj)
        Lufia2PpuObjCaptureBind(NULL,NULL);
    s_recording = context;
    lufia2_ppu_offload_recording = context != NULL;
    if(context && context->indexed_menu) {
        context->selected_obj.runs=context->selected_runs;
        context->selected_obj.capacity=SNESRECOMP_OBJ_MAX_SLIVERS;
        Lufia2PpuObjCaptureBind(&context->selected_obj,context->live);
    }
}

Lufia2PpuOffload *Lufia2PpuOffloadCreate(SnesRecompPresenter *presenter) {
    if (!presenter || !snesrecomp_presenter_ppu_capabilities(presenter)) return NULL;
    Lufia2PpuOffload *c = calloc(1, sizeof(*c));
    if (!c) return NULL;
    c->presenter = presenter;
    c->frame = snesrecomp_ppu_frame_create();
    if (!c->frame) { free(c); return NULL; }
    const uint32_t caps=snesrecomp_presenter_ppu_capabilities(presenter);
    /* Indexed jobs carry tiles and qualified slivers, not RGB blit payloads.
     * Allocate their own contract first; the large compatibility buffers must
     * neither be a prerequisite nor exhaust the heap before this path exists. */
    if(caps&SNESRECOMP_PPU_CAP_INDEXED_LAYOUT) {
        c->graphics_frame=snesrecomp_ppu_frame_create();
        if(c->graphics_frame)c->indexed_job=snesrecomp_ppu_job_create_indexed(
            SNESRECOMP_PPU_INDEXED_MAX_CLAIMS,LUFIA2_MENU_UI_MAX_OBJECTS,
            SNESRECOMP_PPU_INDEXED_MAX_AXES);
        if(c->indexed_job && !Lufia2MenuUiReserveIndexedLayout()) {
            snesrecomp_ppu_job_release(c->indexed_job);c->indexed_job=NULL;
        }
        if(!c->indexed_job) {
            snesrecomp_ppu_frame_release(c->graphics_frame);c->graphics_frame=NULL;
        }
    }
    if(!c->indexed_job && (caps&SNESRECOMP_PPU_CAP_POST_BLITS))
        c->job=snesrecomp_ppu_job_create_blits(3u*256u*LUFIA2_PPU_VISIBLE_LINES+
            SNESRECOMP_PPU_JOB_MAX_PIXELS,SNESRECOMP_BLIT_MAX_RECTS);
    c->menu_available=c->job!=NULL;
    if(!c->indexed_job && (caps&SNESRECOMP_PPU_CAP_PRIMITIVES)) {
        c->graphics_frame=snesrecomp_ppu_frame_create();
        if(c->graphics_frame) c->primitive_job=snesrecomp_ppu_job_create_primitives(0,SNESRECOMP_PPU_JOB_MAX_PRIMITIVES);
    }
    if (!c->job && (snesrecomp_presenter_ppu_capabilities(presenter) &
        (SNESRECOMP_PPU_CAP_POST_OVERLAYS | SNESRECOMP_PPU_CAP_BG_SOURCES))) {
        c->job = snesrecomp_ppu_job_create(SNESRECOMP_PPU_JOB_MAX_PIXELS);
        /* Overlay resources are optional; an exhausted allocation retains the
         * existing exact frame renderer instead of disabling all offload. */
    }
    return c;
}
void Lufia2PpuOffloadDestroy(Lufia2PpuOffload *c) {
    if (!c) return;
    if (s_recording == c) SetRecording(NULL);
    if(c->menu && c->deferred) Lufia2MenuUiEndNativeBase();
    if (c->prepared) snesrecomp_presenter_ppu_cancel(c->presenter);
    snesrecomp_ppu_job_release(c->job);
    snesrecomp_ppu_job_release(c->primitive_job);
    snesrecomp_ppu_job_release(c->indexed_job);
    snesrecomp_ppu_frame_release(c->graphics_frame);
    snesrecomp_ppu_frame_release(c->frame);
    free(c);
}

static bool RecordedPolicy(const Ppu *p, uint32_t caps, bool menu) {
    if (p->widescreenLineEnhancer || p->renderVram || p->mode7Hd.pixels ||
        p->wsMode2CaptureLayer || p->wsHudSplitHeight ||
        p->wsHudOamSlots || p->wsHudOamSlots2) return false;
    for (unsigned i = 0; i < kPpuOverlaySource_Count; ++i)
        if (p->overlayRenderBuffer[i] && !menu) return false;
    for (unsigned layer = 0; layer < 4; ++layer) {
        /* Mode-1 clamp bands are already resolved into each captured band's
         * margins. No live policy lookup is needed during pixel replay. Other
         * modes and unrecorded source remappings retain the serial renderer. */
        if ((PPU_mode(p) != 1 && p->wsClampY1[layer] > p->wsClampY0[layer]) ||
            p->wsRepeatY1[layer] > p->wsRepeatY0[layer] ||
            p->wsStretchY1[layer] > p->wsStretchY0[layer] ||
            p->wsMarginGapL[layer] || p->wsMarginGapR[layer] ||
            p->wsViewportInsetL[layer] || p->wsViewportInsetR[layer] ||
            (WsShadowLayerActive(layer) && (p->extraLeftCur || p->extraRightCur) &&
                (PPU_mode(p) != 1 || layer >= 2 || PPU_bigTiles(p, layer) ||
                 ((p->wsLayerRepeat | p->wsLayerMirror) & (1u << layer))))) return false;
        for (unsigned b = 0; b < kPpuWsAnchorBands; ++b)
            if (p->wsAnchorY1[b][layer] > p->wsAnchorY0[b][layer]) return false;
        for (unsigned b = 0; b < kPpuWsWorldBands; ++b)
            if (p->wsWorldY1[b][layer] > p->wsWorldY0[b][layer]) return false;
        for (unsigned b = 0; b < kPpuWsElasticBands; ++b)
            if (p->wsElasticNSeg[b][layer] &&
                (PPU_mode(p) != 1 || !(caps & SNESRECOMP_PPU_CAP_ELASTIC))) return false;
    }
    return true;
}

bool Lufia2PpuOffloadBegin(Lufia2PpuOffload *c, Ppu *p,
                         unsigned width, unsigned extra, bool eligible) {
    if (!c || !p || s_recording || c->prepared || c->deferred) return false;
    snesrecomp_presenter_ppu_begin_frame(c->presenter);
    c->capture_us = c->fallback_us = 0;
    c->captured_frame = NULL;
    c->timing_valid = 0;
    c->result = SNESRECOMP_PPU_UNSUPPORTED;
    c->unsupported = SNES_PPU_UNSUPPORTED_LAYOUT_POLICY;
    const uint32_t caps = snesrecomp_presenter_ppu_capabilities(c->presenter);
    c->menu=false;
    c->primitive_menu=false;
    c->indexed_menu=false;
    c->indexed_pixels=false;
    const bool menu=((c->menu_available && (caps&SNESRECOMP_PPU_CAP_POST_BLITS)) ||
        (c->primitive_job && (caps&SNESRECOMP_PPU_CAP_PRIMITIVES)) ||
        (c->indexed_job && (caps&SNESRECOMP_PPU_CAP_INDEXED_LAYOUT))) && Lufia2MenuUiNativeBaseEligible(p);
    if (!eligible || !RecordedPolicy(p, caps, menu)) return false;
    const unsigned mode = PPU_mode(p);
    if (!PPU_forcedBlank(p) && ((mode == 7 && !(caps & SNESRECOMP_PPU_CAP_MODE7)) ||
        (mode == 1 && !(caps & SNESRECOMP_PPU_CAP_MODE1)) || (mode != 1 && mode != 7))) {
        c->unsupported = SNES_PPU_UNSUPPORTED_MODE;
        return false;
    }
    c->result = SNESRECOMP_PPU_INVALID;
    c->unsupported = SNES_PPU_UNSUPPORTED_RASTER_STATE;
    if (width < 256 || width > SNESRECOMP_PPU_MAX_WIDTH || extra > width - 256 ||
        !(p->renderFlags & kPpuRenderFlags_NewRenderer)) return false;
    c->result = SNESRECOMP_PPU_INCOMPLETE_MEMORY;
    c->unsupported = SNES_PPU_UNSUPPORTED_RASTER_MEMORY;
    if (Lufia2PpuPendingRasterMemoryFlags() & ~SNES_PPU_RASTER_MEMORY_CGRAM) return false;
    /* Presenter submission has copied all GPU-owned data before releasing its
     * job reference. Keep one capture owner across ordinary and overlay frames
     * so unchanged memory certificates remain useful to persistent caches. */
    if (c->job) {
        c->result = snesrecomp_ppu_job_detach_frame(c->job);
        if (c->result != SNESRECOMP_PPU_OK) return false;
    }
    if(c->primitive_job && snesrecomp_ppu_job_detach_frame(c->primitive_job)!=SNESRECOMP_PPU_OK) return false;
    if(c->indexed_job && snesrecomp_ppu_job_detach_frame(c->indexed_job)!=SNESRECOMP_PPU_OK) return false;
    c->result=snesrecomp_ppu_frame_discard_raster_view(c->frame);
    if(c->result!=SNESRECOMP_PPU_OK) return false;
    c->result = SNESRECOMP_PPU_OK; c->unsupported = SNES_PPU_SUPPORTED;
    /* Disabled layer-capture storage cannot be read by this admitted pixel path. */
    _Static_assert(offsetof(Ppu, wsMode2Bg1Palette) ==
        offsetof(Ppu, wsMode2Capture) + sizeof(p->wsMode2Capture), "PPU capture storage moved");
    const size_t begin = offsetof(Ppu, wsMode2Capture);
    const size_t end = offsetof(Ppu, wsMode2Bg1Palette) + sizeof(p->wsMode2Bg1Palette);
    Lufia2PpuCopyRange(&c->reference,p,0,begin);
    Lufia2PpuCopyRange(&c->reference,p,end,sizeof(*p)-end);
    if(!Lufia2PpuElasticPrepare(&c->elastic,&c->reference)) return false;
    c->live = p; c->width = width; c->extra = extra; c->lines = 0;
    c->battle = Lufia2BattleEffectsActive(p);
    if (c->battle) Lufia2BattleEffectsCapturePixelPolicy(&c->battle_policy);
    c->record_palettes = (Lufia2PpuPendingRasterMemoryFlags() & SNES_PPU_RASTER_MEMORY_CGRAM) != 0;
    if(menu && !Lufia2MenuUiBeginNativeBase(p)) return false;
    if(menu && (c->primitive_job || c->indexed_job))
        c->primitive_menu=Lufia2MenuUiBeginNativePrimitives(p);
    /* An indexed-only context cannot later manufacture legacy RGB primitives
     * for highlights or unsupported layouts. Reject before recording any row. */
    c->indexed_menu=c->primitive_menu && c->indexed_job && Lufia2MenuUiIndexedLayoutPlan();
    if(menu && !c->indexed_menu && !c->primitive_job && !c->menu_available) {
        Lufia2MenuUiEndNativeBase();return false;
    }
    c->indexed_pixels=c->indexed_menu && lufia2_ppu_host_costs_enabled;
    if(menu) c->reference.renderVram=Lufia2MenuUiNativeBaseVram();
    c->menu=menu;c->deferred = true;
    c->clear_destination = false;
    c->shadow_mask = 0;
    memset(c->shadow_memo, 0, sizeof c->shadow_memo);
    for (unsigned layer = 0; layer < 2; ++layer)
        if (WsShadowLayerActive(layer) && (p->extraLeftCur || p->extraRightCur)) {
            c->shadow_mask |= 1u << layer;
            memset(c->shadow_maps[layer], 0, sizeof c->shadow_maps[layer]);
            memset(c->shadow_known[layer], 0, sizeof c->shadow_known[layer]);
        }
    SetRecording(c);
    if (!menu && !c->battle && !c->indexed_pixels && !c->primitive_menu &&
        lufia2_ppu_host_costs_enabled) {
        const Lufia2PpuObjDeferTarget target = {c->objects, c->obj_pixels, width, extra};
        Lufia2PpuObjDeferStart(p, &target);
    }
    return true;
}

void Lufia2PpuOffloadDeferDestinationClear(Lufia2PpuOffload *c) {
    if (c && c->deferred && s_recording == c) c->clear_destination = true;
}

bool Lufia2PpuOffloadRecordLine(Ppu *p, unsigned line) {
    Lufia2PpuOffload *c = s_recording;
    if (!c || c->live != p || !line || line > LUFIA2_PPU_VISIBLE_LINES) return false;
    const Ppu *reference=&c->reference;
    if(c->menu && (!Lufia2MenuUiNativeBaseEligible(p) ||
        (reference->renderVram && PPU_bgTilemapAdr(p,1)!=PPU_bgTilemapAdr(reference,1)))) {
        Lufia2PpuOffloadFallback(c);
        c->result=SNESRECOMP_PPU_INCOMPLETE_LAYOUT;
        c->unsupported=SNES_PPU_UNSUPPORTED_LAYOUT_POLICY;return false;
    }
    if (c->battle && !Lufia2BattleEffectsPixelPolicyMatches(&c->battle_policy)) {
        Lufia2PpuOffloadFallback(c);
        c->result = SNESRECOMP_PPU_INCOMPLETE_LAYOUT;
        c->unsupported = SNES_PPU_UNSUPPORTED_LAYOUT_POLICY;
        return false;
    }
    /* Until policy histories are recorded, a mid-frame host remap change
     * retains serial drawing. Already recorded lines used the entry policy. */
    if (!Lufia2PpuElasticEqual(&c->elastic,&c->reference,p)) {
        Lufia2PpuOffloadFallback(c);
        c->result=SNESRECOMP_PPU_INCOMPLETE_LAYOUT;
        c->unsupported=SNES_PPU_UNSUPPORTED_LAYOUT_POLICY;
        return false;
    }
    for (unsigned layer = 0; layer < 2; ++layer)
        if ((c->shadow_mask & (1u << layer)) && !Lufia2PpuResolveShadowLineCached(p, line,
                layer, c->shadow_maps[layer], c->shadow_known[layer], &c->shadow_memo[layer])) {
            Lufia2PpuOffloadFallback(c);
            c->result = SNESRECOMP_PPU_INCOMPLETE_LAYOUT;
            c->unsupported = SNES_PPU_UNSUPPORTED_LAYOUT_POLICY;
            return false;
        }
    const unsigned y = line - 1;
    if (c->battle) Lufia2BattleEffectsCaptureSpriteRow(c->battle_sprite_rows[y]);
    /* Partial-frame fallback must not depend on the runtime's end-of-frame
     * history-valid flag. Copy only when palette changes were anticipated. */
    if (c->record_palettes) memcpy(c->palettes[y], p->cgram, sizeof c->palettes[y]);
#if defined(LUFIA2_OBJ_DEFER_CHECK)
    /* Serial row stays authoritative; the helper row is compared. */
    if (Lufia2PpuObjDeferTakeLine(y)) Lufia2PpuObjDeferCheck(y, p->lineHasSprites, &p->objBuffer);
#else
    /* The helper writes this row's objects and obj_pixels. */
    if (Lufia2PpuObjDeferTakeLine(y)) {
        c->has_sprites[y] = p->lineHasSprites;
        c->mosaic_start[y] = p->mosaicStartLine;
        Lufia2PpuObjDeferPublish(y, p->lineHasSprites);
        ++c->lines;
        return true;
    }
#endif
    if (p->lineHasSprites) c->objects[y] = p->objBuffer;
    if(c->primitive_menu && p->lineHasSprites) {
        /* All centre OAM slots were qualified into this overlay in original
         * OAM order. Never merge them by BG priority or qualify them again. */
        for(unsigned x=0;x<256;++x)
            c->objects[y].data[x+kPpuExtraLeftRight]=p->overlayBuffers[kPpuOverlaySource_Obj].data[x+kPpuExtraLeftRight];
    }
    c->has_sprites[y] = p->lineHasSprites;
    c->mosaic_start[y] = p->mosaicStartLine;
    /* The indexed contract consumes the already selected runs. Keep saved
     * native rows solely for a possible CPU fallback; do not additionally
     * manufacture and copy an unused per-pixel OBJ plane. */
    if(c->indexed_pixels) {
        if(!Lufia2MenuUiRecordIndexedLine(p,line)) {Lufia2PpuOffloadFallback(c);return false;}
        ++c->lines;return true;
    }
    if (!p->lineHasSprites) {
        memset(c->obj_pixels + y * c->width, 0, c->width * sizeof(uint16_t));
        ++c->lines;
        return !c->menu || c->indexed_pixels;
    }
    for (unsigned x = 0; x < c->width; ++x) {
        uint16_t native = c->primitive_menu?c->objects[y].data[(int)x-(int)c->extra+kPpuExtraLeftRight]:
            p->objBuffer.data[(int)x - (int)c->extra + kPpuExtraLeftRight];
        /* Native battle OBJ composition rejects ordinary sprites outside the
         * centre. The prepared GPU row must use that same qualified mask. */
        if (c->battle && (x < c->extra || x >= c->extra + 256u) &&
            !c->battle_sprite_rows[y][(int)x - (int)c->extra + kPpuExtraLeftRight]) native = 0;
        c->obj_pixels[y * c->width + x] = (native & 255u) && p->lineHasSprites ?
            (uint16_t)((native & 255u) | (((native >> 12) - 2u) / 4u << 8) |
                       (((native >> 8) & 15u) == 4u ? 0x400u : 0u) | 0x800u) : 0;
    }
    ++c->lines;
    /* Indexed menus defer both BG indices and RGB; compatibility menus still
     * record their original indices. Sprite qualification remains once. */
    return !c->menu || c->indexed_pixels;
}

bool Lufia2PpuOffloadEnd(Lufia2PpuOffload *c) {
    return Lufia2PpuOffloadEndWithOverlays(c, NULL, 0, NULL, 0);
}

bool Lufia2PpuOffloadEndWithOverlays(Lufia2PpuOffload *c,
    const SnesRecompPpuJobImage *images, uint16_t image_count,
    const SnesRecompPpuOverlay *overlays, uint16_t overlay_count) {
    return Lufia2PpuOffloadEndCapture(c) &&
        Lufia2PpuOffloadPrepareOverlays(c, images, image_count, overlays, overlay_count);
}

const SnesRecompPpuFrame *Lufia2PpuOffloadEndCapture(Lufia2PpuOffload *c) {
    if (!c || !c->deferred || s_recording != c) return NULL;
    SetRecording(NULL);
    const uint64_t begin = snesrecomp_now_us();
    c->timing_valid |= SNESRECOMP_PPU_TIME_CAPTURE;
    SnesPpuFrameCapture cap;
    if (c->lines != LUFIA2_PPU_VISIBLE_LINES ||
        !Lufia2CapturePpuFrame(&cap, c->width, c->extra)) {
        c->result = SNESRECOMP_PPU_INVALID;
        c->capture_us = snesrecomp_now_us() - begin;
        return NULL;
    }
    SnesRecompObjFrame qualified={0};
    const SnesRecompObjFrame *authority=NULL;
    if(c->indexed_pixels) {
        if(!c->selected_obj.valid) {
            c->result=SNESRECOMP_PPU_INCOMPLETE_MEMORY;
            c->capture_us=snesrecomp_now_us()-begin;return NULL;
        }
        memset(c->qualified_objects.priorities,0,sizeof c->qualified_objects.priorities);
        for(unsigned i=0;i<c->selected_obj.count;++i) {
            const SnesRecompObjSliver *sliver=&c->selected_obj.runs[i].sliver;
            if(sliver->line>=LUFIA2_PPU_VISIBLE_LINES || sliver->priority>3) {
                c->result=SNESRECOMP_PPU_INVALID;
                c->capture_us=snesrecomp_now_us()-begin;return NULL;
            }
            c->qualified_objects.slivers[i]=*sliver;
            c->qualified_objects.priorities[sliver->line]|=(uint8_t)(1u<<sliver->priority);
        }
        qualified=(SnesRecompObjFrame){.slivers=c->qualified_objects.slivers,
            .capacity=SNESRECOMP_OBJ_MAX_SLIVERS,.count=c->selected_obj.count,
            .line_priority=c->qualified_objects.priorities,.line_capacity=LUFIA2_PPU_VISIBLE_LINES,
            .range_over=c->live->rangeOver,.time_over=c->live->timeOver};
        authority=&qualified;cap.obj_pixels=NULL;cap.obj_pitch=0;
    } else {cap.obj_pixels=c->obj_pixels;cap.obj_pitch=c->width;}
    const uint64_t sequence=++c->sequence;
    if(c->primitive_menu) {
        c->result=snesrecomp_ppu_frame_freeze(c->graphics_frame,&cap,authority,sequence);
        if(c->result!=SNESRECOMP_PPU_OK) {c->capture_us=snesrecomp_now_us()-begin;return NULL;}
    }
    if(c->menu) {
        if(c->reference.renderVram) cap.vram=c->reference.renderVram;
        memcpy(c->menu_bands,cap.bands,cap.band_count*sizeof *cap.bands);
        for(unsigned i=0;i<cap.band_count;++i) c->menu_bands[i].main_enable&=2u;
        cap.bands=c->menu_bands;
    }
    for (unsigned layer = 0; layer < 2; ++layer)
        if (c->shadow_mask & (1u << layer)) {
            cap.resolved_bg[layer] = c->shadow_maps[layer];
            cap.resolved_bg_required |= (uint8_t)(1u << layer);
        }
    SnesRecompPpuFrame *frame = c->frame;
    if(c->primitive_menu && !c->reference.renderVram && !c->shadow_mask)
        c->result=snesrecomp_ppu_frame_freeze_raster_view(frame,c->graphics_frame,
            cap.bands,cap.band_count,sequence);
    else c->result = snesrecomp_ppu_frame_freeze(frame, &cap, authority, sequence);
    c->capture_us = snesrecomp_now_us() - begin;
    if (c->result != SNESRECOMP_PPU_OK) return NULL;
    c->captured_frame = frame;
    return frame;
}

bool Lufia2PpuOffloadPrepareOverlays(Lufia2PpuOffload *c,
    const SnesRecompPpuJobImage *images, uint16_t image_count,
    const SnesRecompPpuOverlay *overlays, uint16_t overlay_count) {
    return Lufia2PpuOffloadPrepareSources(c,images,image_count,overlays,overlay_count,NULL,0);
}
bool Lufia2PpuOffloadMenuBase(const Lufia2PpuOffload *c) {return c && c->deferred && c->menu;}
bool Lufia2PpuOffloadMenuPrimitives(const Lufia2PpuOffload *c) {return c && c->primitive_menu;}
bool Lufia2PpuOffloadFreezeMenu(Lufia2PpuOffload *c) {
    if(c) {c->frozen_job=NULL;c->frozen_frame=NULL;}
    if(!c || !c->deferred || !c->menu || (!c->job && !c->primitive_job && !c->indexed_job) || !c->captured_frame || c->prepared ||
        s_recording==c) return false;
    if(c->indexed_menu) {
        if(!c->selected_obj.valid) {c->result=SNESRECOMP_PPU_INCOMPLETE_MEMORY;return false;}
        const uint64_t begin=snesrecomp_now_us();
        c->result=Lufia2MenuUiFreezeIndexedLayout(c->indexed_job,c->captured_frame,c->graphics_frame,
            c->selected_obj.runs,c->selected_obj.count,c->selected_obj.completed_lines,c->sequence);
        c->capture_us+=snesrecomp_now_us()-begin;
        if(c->result!=SNESRECOMP_PPU_OK) return false;
        c->frozen_job=c->indexed_job;return true;
    }
    if(c->primitive_menu) {
        const Lufia2MenuUiPrimitivePlan *plan=Lufia2MenuUiPrimitives(c->graphics_frame);
        if(!plan) {c->result=SNESRECOMP_PPU_RESOURCE_LIMIT;return false;}
        const uint64_t begin=snesrecomp_now_us();
        c->result=snesrecomp_ppu_job_freeze_primitives(c->primitive_job,c->captured_frame,c->graphics_frame,
            NULL,0,plan->primitives,plan->count,plan->materials,plan->material_count,c->sequence);
        c->capture_us+=snesrecomp_now_us()-begin;
        if(c->result!=SNESRECOMP_PPU_OK) return false;
        c->frozen_job=c->primitive_job;return true;
    }
    const SnesRecompBlitPlan *plan=Lufia2MenuUiBlitPlan(c->width);
    if(!plan) {c->result=SNESRECOMP_PPU_RESOURCE_LIMIT;return false;}
    const uint64_t begin=snesrecomp_now_us();
    c->result=snesrecomp_ppu_job_freeze_blits(c->job,c->captured_frame,plan,c->sequence);
    c->capture_us+=snesrecomp_now_us()-begin;
    if(c->result!=SNESRECOMP_PPU_OK) return false;
    c->frozen_job=c->job;return true;
}
/* Presenter work only; safe on the helper core after a freeze. */
bool Lufia2PpuOffloadPrepareFrozen(Lufia2PpuOffload *c) {
    if(!c || (!c->frozen_job && !c->frozen_frame)) return false;
    c->result=c->frozen_job?snesrecomp_presenter_ppu_prepare_job(c->presenter,c->frozen_job):
        snesrecomp_presenter_ppu_prepare(c->presenter,c->frozen_frame);
#if defined(LUFIA2_PPU_CAPTURE_DIAGNOSTICS)
    /* Optional immutable replay evidence. Enabled only for diagnostic runs;
     * these IO-bearing frames are never used for performance acceptance. */
    static bool dumped_ok, dumped_rejected;
    static bool root_checked;
    static const char *root;
    if (!root_checked) { root = getenv("LUFIA2_PPU_JOB_TRACE_ROOT"); root_checked = true; }
    if (root && *root) {
    const SnesRecompPpuJobView *v = c->frozen_job ?
        snesrecomp_ppu_job_view(c->frozen_job) : NULL;
    static uint32_t trace_sequence;
    const uint32_t sequence = ++trace_sequence;
    const SnesRecompPpuFrame *trace_frame = v ? v->frame : c->frozen_frame;
    const SnesPpuFrameCapture *trace_capture = trace_frame ?
        snesrecomp_ppu_frame_capture(trace_frame) : NULL;
    bool mode7_trace = false;
    if (trace_capture) for (unsigned i = 0; i < trace_capture->band_count; ++i)
        mode7_trace |= !trace_capture->bands[i].forced_blank &&
            trace_capture->bands[i].bg_mode == 7u;
    if (root && *root && mode7_trace &&
        (sequence == 1u || sequence == 45u || sequence == 60u)) {
        char path[512];
        int n = snprintf(path, sizeof path, "%s/mode7-%04lu.sppu", root,
            (unsigned long)sequence);
        const bool written = n > 0 && (size_t)n < sizeof path &&
            snesrecomp_ppu_trace_write(path, trace_capture, sequence,
                c->result == SNESRECOMP_PPU_OK, SNES_PPU_SUPPORTED);
        fprintf(stderr, "Mode7 capture sequence=%lu saved=%u\n",
            (unsigned long)sequence, written);
    }
    if (root && *root && v && (v->source_count || v->overlay_count)) {
        const bool ok = c->result == SNESRECOMP_PPU_OK;
        bool *done = ok ? &dumped_ok : &dumped_rejected;
        if (!*done) {
            char path[512];
            int n = snprintf(path, sizeof path, "%s/%s-job.sppu", root,
                ok ? "accepted" : "rejected");
            *done = n > 0 && (size_t)n < sizeof path &&
                snesrecomp_ppu_trace_write_job(path, c->frozen_job, 0u,
                    ok ? 1u : 0u, SNES_PPU_SUPPORTED);
            fprintf(stderr, "PPU frozen job result=%u dumped=%u sources=%u overlays=%u images=%u\n",
                (unsigned)c->result, *done, v->source_count, v->overlay_count, v->image_count);
        }
    }
    }
#endif
    c->frozen_job=NULL;c->frozen_frame=NULL;
    c->prepared=c->result==SNESRECOMP_PPU_OK;return c->prepared;
}
bool Lufia2PpuOffloadPrepareMenu(Lufia2PpuOffload *c) {
    return Lufia2PpuOffloadFreezeMenu(c) && Lufia2PpuOffloadPrepareFrozen(c);
}
bool Lufia2PpuOffloadPrepareSources(Lufia2PpuOffload *c,
    const SnesRecompPpuJobImage *images, uint16_t image_count,
    const SnesRecompPpuOverlay *overlays, uint16_t overlay_count,
    const SnesRecompPpuSource *sources, uint16_t source_count) {
    return Lufia2PpuOffloadPrepareResolvedSources(c,c?c->captured_frame:NULL,
        images,image_count,overlays,overlay_count,sources,source_count);
}
bool Lufia2PpuOffloadPrepareResolvedSources(Lufia2PpuOffload *c,const SnesRecompPpuFrame *resolved,
    const SnesRecompPpuJobImage *images,uint16_t image_count,
    const SnesRecompPpuOverlay *overlays,uint16_t overlay_count,
    const SnesRecompPpuSource *sources,uint16_t source_count) {
    return Lufia2PpuOffloadFreezeResolvedSources(c,resolved,images,image_count,overlays,
        overlay_count,sources,source_count) && Lufia2PpuOffloadPrepareFrozen(c);
}
bool Lufia2PpuOffloadFreezeResolvedSources(Lufia2PpuOffload *c,const SnesRecompPpuFrame *resolved,
    const SnesRecompPpuJobImage *images,uint16_t image_count,
    const SnesRecompPpuOverlay *overlays,uint16_t overlay_count,
    const SnesRecompPpuSource *sources,uint16_t source_count) {
    if(c) {c->frozen_job=NULL;c->frozen_frame=NULL;}
    if (!c || !c->deferred || !c->captured_frame || c->prepared || s_recording == c) return false;
    const uint64_t begin = snesrecomp_now_us();
    const SnesRecompPpuFrame *frame = resolved;
    const SnesPpuFrameCapture *next=snesrecomp_ppu_frame_capture(frame);
    const SnesPpuFrameCapture *old=snesrecomp_ppu_frame_capture(c->captured_frame);
    if(!next || !old || next->canvas_width!=old->canvas_width || next->visible_height!=old->visible_height ||
        next->canvas_extra!=old->canvas_extra) {c->result=SNESRECOMP_PPU_INVALID;return false;}
    if (c->job && (image_count || overlay_count || source_count)) {
        c->result = snesrecomp_ppu_job_freeze_sources(c->job, frame, images, image_count,
            overlays, overlay_count, sources, source_count, c->sequence);
        c->capture_us += snesrecomp_now_us() - begin;
        if (c->result != SNESRECOMP_PPU_OK) return false;
    } else if (image_count || overlay_count || source_count) {
        c->result = SNESRECOMP_PPU_RESOURCE_LIMIT;
        return false;
    }
    if (overlay_count || source_count) c->frozen_job = c->job;
    else c->frozen_frame = frame;
    return true;
}

const Lufia2BattleEffectsPixelPolicy *Lufia2PpuOffloadBattlePolicy(
    const Lufia2PpuOffload *c, const bool **sprite_rows) {
    if (sprite_rows) *sprite_rows = c && c->battle ? c->battle_sprite_rows[0] : NULL;
    return c && c->battle ? &c->battle_policy : NULL;
}

void Lufia2PpuOffloadFallback(Lufia2PpuOffload *c) {
    if (!c || !c->deferred) return;
    if (s_recording == c) SetRecording(NULL);
    if (c->prepared) snesrecomp_presenter_ppu_cancel(c->presenter);
    c->prepared = false;
    const uint64_t begin = snesrecomp_now_us();
    c->timing_valid |= SNESRECOMP_PPU_TIME_FALLBACK;
    bool live_sprite_row[kPpuBufWidth];
    const Lufia2BattleEffectsPixelPolicy *previous = NULL;
    if (c->battle) {
        Lufia2BattleEffectsCaptureSpriteRow(live_sprite_row);
        previous = Lufia2BattleEffectsBindPixelPolicy(&c->battle_policy);
    }
    if (c->clear_destination) {
        for (unsigned y = 0; y < LUFIA2_PPU_VISIBLE_LINES; ++y)
            memset(c->reference.renderBuffer + (size_t)y * c->reference.renderPitch,
                0, c->width * 4u);
        c->clear_destination = false;
    }
    if(c->menu) {
        PpuClearOverlayCaptures(&c->reference);PpuClearOverlayBindings(&c->reference);
    }
    for (unsigned y = 0; y < c->lines; ++y) {
        memcpy(&c->reference.inidisp, Lufia2LineRegisters(y), PPU_SAVESTATE_REGS_SIZE);
        if (c->record_palettes) memcpy(c->reference.cgram, c->palettes[y], sizeof c->palettes[y]);
        c->reference.objBuffer = c->objects[y];
        c->reference.lineHasSprites = c->has_sprites[y];
        c->reference.mosaicStartLine = c->mosaic_start[y];
        if (c->battle) Lufia2BattleEffectsRestoreSpriteRow(c->battle_sprite_rows[y]);
        if(c->menu) {
            if(c->indexed_pixels)Lufia2MenuUiRecoverIndexedRow(&c->reference,y+1,
                Lufia2PpuReplayMenuIndexLine,Lufia2PpuReplayPixelLine);
            else if(c->primitive_menu)Lufia2MenuUiRecoverPrimitiveRow(&c->reference,y+1,Lufia2PpuReplayPixelLine);
              /* Index recovery borrows the UI surfaces again. The following
               * background-only pass must not clear those recovered colours. */
              PpuClearOverlayCaptures(&c->reference);PpuClearOverlayBindings(&c->reference);
            c->reference.screenEnabled[0]&=2u;
            Lufia2MenuUiReplayBaseLine(&c->reference,y+1,Lufia2PpuReplayPixelLine);
        } else Lufia2PpuReplayPixelLine(&c->reference, y + 1);
    }
    if (c->battle) {
        Lufia2BattleEffectsBindPixelPolicy(previous);
        Lufia2BattleEffectsRestoreSpriteRow(live_sprite_row);
    }
    c->deferred = false;
    if(c->menu) Lufia2MenuUiEndNativeBase();
    c->captured_frame = NULL;
    c->fallback_us = snesrecomp_now_us() - begin;
}

void Lufia2PpuOffloadBeforeMemoryWrite(Ppu *ppu, uint8_t port) {
    Lufia2PpuOffload *c = s_recording;
    if (!c || c->live != ppu) return;
    if (port != 0x04u && port != 0x18u && port != 0x19u &&
        !(port == 0x22u && !c->record_palettes)) return;
    /* Render completed rows from pre-write memory before the live PPU mutates.
     * Remaining rows resume the original serial path. This performs no guest,
     * HDMA, IRQ, sprite evaluation or status-flag advancement. */
    Lufia2PpuOffloadFallback(c);
    c->result = SNESRECOMP_PPU_INCOMPLETE_MEMORY;
    c->unsupported = SNES_PPU_UNSUPPORTED_RASTER_MEMORY;
}

bool Lufia2PpuOffloadSubmitPresenter(Lufia2PpuOffload *c) {
    if (!c || !c->prepared) return false;
    c->result = snesrecomp_presenter_ppu_submit(c->presenter);
    bool ok = c->result == SNESRECOMP_PPU_OK;
    c->prepared = false;
    if (ok) c->deferred = false;
    return ok;
}
void Lufia2PpuOffloadSubmitted(Lufia2PpuOffload *c) {
    if(c && c->menu) Lufia2MenuUiEndNativeBase();
}
bool Lufia2PpuOffloadSubmit(Lufia2PpuOffload *c) {
    const bool ok = Lufia2PpuOffloadSubmitPresenter(c);
    if (ok) Lufia2PpuOffloadSubmitted(c);
    return ok;
}

void Lufia2PpuOffloadStats(const Lufia2PpuOffload *c, SnesRecompPpuStats *out) {
    if (!out) return;
    snesrecomp_presenter_ppu_stats(c ? c->presenter : NULL, out);
    if (!c) { out->unsupported = SNES_PPU_UNSUPPORTED_BACKEND; return; }
    out->capture_us = c->capture_us; out->fallback_us = c->fallback_us;
    out->timing_valid |= c->timing_valid;
    out->last_result = c->result;
    if (c->unsupported) out->unsupported = c->unsupported;
}
