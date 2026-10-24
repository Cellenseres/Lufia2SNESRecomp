#include "lufia2_intro_ppu_job.h"
#include "lufia2_intro_margin_job.h"
#include "lufia2_intro_tree_job.h"
#include <stdlib.h>
#include <string.h>

struct Lufia2IntroPpuJob {
    Lufia2IntroMarginJob *margins;
    Lufia2IntroTreeJob *trees;
    Lufia2IntroTreeSnapshot snapshot;
};
Lufia2IntroPpuJob *Lufia2IntroPpuJobCreate(void) {
    Lufia2IntroPpuJob *c = calloc(1, sizeof *c);
    if (!c) return NULL;
    c->margins = Lufia2IntroMarginJobCreate(128, 224);
    c->trees = Lufia2IntroTreeJobCreate(224);
    if (!c->margins || !c->trees) { Lufia2IntroPpuJobDestroy(c); return NULL; }
    return c;
}
void Lufia2IntroPpuJobDestroy(Lufia2IntroPpuJob *c) {
    if (c) {
        Lufia2IntroMarginJobDestroy(c->margins);
        Lufia2IntroTreeJobDestroy(c->trees);
        free(c);
    }
}
static SnesRecompPpuResult Prepare(Lufia2IntroPpuJob *c,
    const Ppu *ppu, const SnesRecompPpuFrame *frame, uint16_t margin,
    SnesRecompPpuJobImage images[LUFIA2_INTRO_JOB_IMAGES], uint16_t *image_count,
    SnesRecompPpuOverlay rectangles[LUFIA2_INTRO_JOB_RECTS], uint16_t *rectangle_count,
    bool unshaded_trees) {
    const SnesPpuFrameCapture *capture = snesrecomp_ppu_frame_capture(frame);
    if (!c || !ppu || !capture || !images || !image_count || !rectangles || !rectangle_count)
        return SNESRECOMP_PPU_INVALID;
    const Lufia2IntroPostState state = Lufia2IntroWidescreenGetPostState();
    if (!state.active) { *image_count = *rectangle_count = 0; return SNESRECOMP_PPU_OK; }
    uint8_t tree_brightness=0;
    if(state.trees_active) {
        const bool captured=unshaded_trees?
            Lufia2IntroTreesCaptureUnshaded(ppu,2,capture->canvas_width,margin,
                state.tree_scroll,state.tree_lines,&c->snapshot,&tree_brightness):
            Lufia2IntroTreesCapture(ppu,2,capture->canvas_width,margin,
                state.tree_scroll,state.tree_lines,&c->snapshot);
        if(!captured) return SNESRECOMP_PPU_UNSUPPORTED;
    }
    SnesRecompPpuJobImage prepared_images[LUFIA2_INTRO_JOB_IMAGES];
    SnesRecompPpuOverlay prepared_rects[LUFIA2_INTRO_JOB_RECTS];
    uint16_t art_count = 0, tree_count = 0;
    SnesRecompPpuResult result = Lufia2IntroMarginJobPrepareFrame(c->margins, frame, margin,
        PPU_forcedBlank(ppu) ? 0u : (unsigned)PPU_brightness(ppu),
        prepared_images, prepared_rects, &art_count);
    if (result != SNESRECOMP_PPU_OK) return result;
    if (state.trees_active) {
        result = Lufia2IntroTreeJobPrepare(c->trees, frame, &c->snapshot,
            prepared_images + art_count, prepared_rects + art_count, &tree_count);
        if (result != SNESRECOMP_PPU_OK) return result;
        for (unsigned i = art_count; i < art_count + tree_count; ++i) {
            prepared_rects[i].image = (uint8_t)(prepared_rects[i].image + art_count);
            prepared_rects[i].brightness_reduction=tree_brightness;
        }
    }
    const unsigned count = art_count + (state.trees_active ? LUFIA2_INTRO_TREE_JOB_IMAGES : 0u);
    memcpy(images, prepared_images, count * sizeof *images);
    memcpy(rectangles, prepared_rects, (art_count + tree_count) * sizeof *rectangles);
    *image_count = (uint16_t)count; *rectangle_count = (uint16_t)(art_count + tree_count);
    return SNESRECOMP_PPU_OK;
}
SnesRecompPpuResult Lufia2IntroPpuJobPrepare(Lufia2IntroPpuJob *c,
    const Ppu *ppu,const SnesRecompPpuFrame *frame,uint16_t margin,
    SnesRecompPpuJobImage images[LUFIA2_INTRO_JOB_IMAGES],uint16_t *image_count,
    SnesRecompPpuOverlay rectangles[LUFIA2_INTRO_JOB_RECTS],uint16_t *rectangle_count) {
    return Prepare(c,ppu,frame,margin,images,image_count,rectangles,rectangle_count,false);
}
SnesRecompPpuResult Lufia2IntroPpuJobPrepareUnshadedTrees(Lufia2IntroPpuJob *c,
    const Ppu *ppu,const SnesRecompPpuFrame *frame,uint16_t margin,
    SnesRecompPpuJobImage images[LUFIA2_INTRO_JOB_IMAGES],uint16_t *image_count,
    SnesRecompPpuOverlay rectangles[LUFIA2_INTRO_JOB_RECTS],uint16_t *rectangle_count) {
    return Prepare(c,ppu,frame,margin,images,image_count,rectangles,rectangle_count,true);
}
