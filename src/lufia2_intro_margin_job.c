#include "lufia2_intro_margin_job.h"
#include "lufia2_intro_margins.h"
#include "snesrecomp_platform/snes_ppu_tile_plan.h"
#include <stdlib.h>
#include <string.h>

struct Lufia2IntroMarginJob {
    uint32_t *source,*pixels[2];
    uint16_t max_margin,max_height,margin,height,width;
    unsigned active,brightness;
    bool valid;
};
SnesRecompPpuResult Lufia2IntroMarginJobPrepareFrame(Lufia2IntroMarginJob *c,
    const SnesRecompPpuFrame *frame,uint16_t margin,unsigned brightness,
    SnesRecompPpuJobImage images[2],SnesRecompPpuOverlay rectangles[2],uint16_t *count) {
    const SnesPpuFrameCapture *capture=snesrecomp_ppu_frame_capture(frame);
    if(!c || !capture || !images || !rectangles || !count) return SNESRECOMP_PPU_INVALID;
    if(!margin || !brightness) {*count=0;return SNESRECOMP_PPU_OK;}
    const uint32_t *art=Lufia2IntroMarginsBorrowArt(capture->canvas_width,capture->visible_height);
    if(!art) {*count=0;return SNESRECOMP_PPU_OK;}
    uint32_t backdrop;
    const SnesRecompPpuResult result=snesrecomp_ppu_mode1_sample(frame,0,0,&backdrop);
    if(result!=SNESRECOMP_PPU_OK) return result;
    return Lufia2IntroMarginJobPrepare(c,art,capture->canvas_width,capture->canvas_width,
        capture->visible_height,margin,brightness,backdrop,images,rectangles,count);
}
Lufia2IntroMarginJob *Lufia2IntroMarginJobCreate(uint16_t max_margin,uint16_t max_height) {
    if(!max_margin || max_margin>128 || !max_height || max_height>SNES_PPU_MAX_BANDS) return NULL;
    Lufia2IntroMarginJob *c=calloc(1,sizeof(*c));
    if(!c) return NULL;
    c->max_margin=max_margin;c->max_height=max_height;
    const size_t bytes=(size_t)max_margin*max_height*2u*sizeof(uint32_t);
    c->source=malloc(bytes);c->pixels[0]=malloc(bytes);c->pixels[1]=malloc(bytes);
    if(!c->source || !c->pixels[0] || !c->pixels[1]) {Lufia2IntroMarginJobDestroy(c);return NULL;}
    return c;
}
void Lufia2IntroMarginJobDestroy(Lufia2IntroMarginJob *c) {
    if(c) {free(c->source);free(c->pixels[0]);free(c->pixels[1]);free(c);}
}
static uint32_t Dim(uint32_t source,unsigned brightness) {
    uint32_t out=source&0xff000000u;
    for(unsigned shift=0;shift<24;shift+=8)
        out|=(((source>>shift)&255u)*brightness/15u)<<shift;
    return out;
}
static bool Overlap(uintptr_t source,size_t bytes,const uint32_t *owned,size_t capacity) {
    uintptr_t begin=(uintptr_t)owned;
    return source<begin+capacity && begin<source+bytes;
}
SnesRecompPpuResult Lufia2IntroMarginJobPrepare(Lufia2IntroMarginJob *c,
    const uint32_t *art,uint32_t pitch,uint16_t width,uint16_t height,
    uint16_t margin,unsigned brightness,uint32_t backdrop,
    SnesRecompPpuJobImage images[2],SnesRecompPpuOverlay rectangles[2],uint16_t *count) {
    if(!c || !images || !rectangles || !count) return SNESRECOMP_PPU_INVALID;
    if(!art || !margin || !brightness) {*count=0;return SNESRECOMP_PPU_OK;}
    if(!width || width>SNESRECOMP_PPU_MAX_WIDTH || !height || pitch<width ||
       pitch>SIZE_MAX/sizeof(uint32_t)/height || (uint32_t)margin*2u>=width || backdrop>>24!=255u)
        return SNESRECOMP_PPU_INVALID;
    if(margin>c->max_margin || height>c->max_height) return SNESRECOMP_PPU_RESOURCE_LIMIT;
    const size_t source_bytes=((size_t)pitch*(height-1u)+width)*sizeof(uint32_t);
    const size_t capacity=(size_t)c->max_margin*c->max_height*2u*sizeof(uint32_t);
    const uintptr_t address=(uintptr_t)art;
    if(address>UINTPTR_MAX-source_bytes || Overlap(address,source_bytes,c->source,capacity) ||
       Overlap(address,source_bytes,c->pixels[0],capacity) || Overlap(address,source_bytes,c->pixels[1],capacity))
        return SNESRECOMP_PPU_INVALID;
    if(brightness>15u) brightness=15u;
    const size_t area=(size_t)margin*height;
    bool same=c->valid && c->width==width && c->height==height && c->margin==margin;
    for(unsigned side=0;same && side<2;++side) for(unsigned y=0;same && y<height;++y)
        same=!memcmp(c->source+side*area+(size_t)y*margin,
            art+(size_t)y*pitch+(side?width-margin:0),margin*sizeof(uint32_t));
    if(!same || c->brightness!=brightness) {
        const unsigned next=c->active^1u;
        for(unsigned side=0;side<2;++side) for(unsigned y=0;y<height;++y) for(unsigned x=0;x<margin;++x) {
            const uint32_t pixel=Dim(art[(size_t)y*pitch+(side?width-margin:0)+x],brightness);
            const unsigned alpha=pixel>>24;
            if((pixel&255u)>alpha || ((pixel>>8)&255u)>alpha || ((pixel>>16)&255u)>alpha)
                return SNESRECOMP_PPU_UNSUPPORTED;
            c->pixels[next][side*area+(size_t)y*margin+x]=pixel;
        }
        if(!same) for(unsigned side=0;side<2;++side) for(unsigned y=0;y<height;++y)
            memcpy(c->source+side*area+(size_t)y*margin,
                art+(size_t)y*pitch+(side?width-margin:0),margin*sizeof(uint32_t));
        c->active=next;c->width=width;c->height=height;c->margin=margin;c->brightness=brightness;c->valid=true;
    }
    for(unsigned side=0;side<2;++side) {
        images[side]=(SnesRecompPpuJobImage){c->pixels[c->active]+side*area,margin,margin,height};
        rectangles[side]=(SnesRecompPpuOverlay){.x=side?(uint16_t)(width-margin):0,
            .width=margin,.height=height,.image=(uint8_t)side,
            .blend=SNESRECOMP_PPU_OVERLAY_PREMULTIPLIED,
            .condition=SNESRECOMP_PPU_OVERLAY_KEY_OR_BLACK,.key_argb=backdrop};
    }
    *count=2;return SNESRECOMP_PPU_OK;
}
