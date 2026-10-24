#include "lufia2_ppu_obj_capture.h"
#include <string.h>
Lufia2PpuObjCapture *lufia2_ppu_obj_capture;
void Lufia2PpuObjCaptureBind(Lufia2PpuObjCapture *c,const struct Ppu *p) {
    if(c) {c->ppu=p;c->count=0;memset(c->lines,0,sizeof c->lines);
        memset(c->completed_lines,0,sizeof c->completed_lines);
        c->valid=p && c->runs && c->capacity && c->capacity<=SNESRECOMP_OBJ_MAX_SLIVERS;}
    lufia2_ppu_obj_capture=c;
}
void Lufia2PpuObjCaptureLine(const struct Ppu *p,int line) {
    Lufia2PpuObjCapture *c=lufia2_ppu_obj_capture;
    if(!c || c->ppu!=p || !c->valid) return;
    if(line<0 || line>=256 || (c->lines[(unsigned)line>>5]&(1u<<(line&31)))) {c->valid=false;return;}
    c->lines[(unsigned)line>>5]|=1u<<(line&31);
    c->line_first[line]=c->count;
}
void Lufia2PpuObjCaptureEndLine(const struct Ppu *p,int line,bool unrecorded_margin) {
    Lufia2PpuObjCapture *c=lufia2_ppu_obj_capture;
    if(!c || c->ppu!=p || !c->valid) return;
    if(line<0 || line>=256 || unrecorded_margin ||
        !(c->lines[(unsigned)line>>5]&(1u<<(line&31))) ||
        (c->completed_lines[(unsigned)line>>5]&(1u<<(line&31)))) {c->valid=false;return;}
    c->line_end[line]=c->count;c->completed_lines[(unsigned)line>>5]|=1u<<(line&31);
}
void Lufia2PpuObjCaptureRun(const struct Ppu *p,int line,int x,int tile,int row,
    int palette,int priority,int flip,unsigned slot,int begin,int end,
    int overlay_begin,int overlay_end,unsigned flags,unsigned owner) {
    Lufia2PpuObjCapture *c=lufia2_ppu_obj_capture;
    if(!c || c->ppu!=p || !c->valid) return;
    if(line<0 || line>=256 || x<INT16_MIN || x>INT16_MAX || tile<0 || tile>=2048 ||
        row<0 || row>7 || palette<128 || palette>240 || (palette&15) || priority<0 || priority>3 ||
        flip<0 || flip>1 || slot>=128 || begin<0 || begin>end || end>8 || overlay_begin<begin ||
        overlay_begin>overlay_end || overlay_end>end || flags>7 || owner>UINT16_MAX ||
        !(c->lines[(unsigned)line>>5]&(1u<<(line&31))) || c->count==c->capacity) {
        c->valid=false;return;
    }
    c->runs[c->count++]=(SnesRecompPpuObjRun){
        .sliver={(int16_t)x,(uint16_t)line,(uint16_t)tile,(uint8_t)row,(uint8_t)palette,
            (uint8_t)priority,(uint8_t)flip,(uint8_t)slot,(uint8_t)(palette>=192)},
        .owner=(uint16_t)owner,.begin=(uint8_t)begin,.end=(uint8_t)end,
        .overlay_begin=(uint8_t)overlay_begin,.overlay_end=(uint8_t)overlay_end,.flags=(uint8_t)flags};
}
