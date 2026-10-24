#pragma once
#include "snes/ppu.h"
#include <stddef.h>
#include <string.h>

/* Both Ppu objects have the same required alignment. Peel the exact short
 * prefix of an interior span before copying its aligned body. The 3DS libc
 * otherwise copies the whole large span byte by byte; excluded scratch bytes
 * must still remain untouched. No overlapping objects are supported. */
static inline void Lufia2PpuCopyRange(Ppu *destination,const Ppu *source,
    size_t offset,size_t bytes) {
    uint8_t *out=(uint8_t *)destination+offset;
    const uint8_t *in=(const uint8_t *)source+offset;
    size_t prefix=(0u-offset)&(sizeof(uint32_t)-1u);
    if(prefix>bytes)prefix=bytes;
    for(size_t i=0;i<prefix;++i)out[i]=in[i];
    if(bytes>prefix)memcpy(out+prefix,in+prefix,bytes-prefix);
}

/* A GPU forced-blank band needs no CPU RGB row. Advance only the same host
 * brightness tables as pixel replay, so a later nonblank HUD view is ready. */
static inline void Lufia2PpuUpdateBrightness(Ppu *ppu) {
    const uint8_t brightness=PPU_brightness(ppu);
    if(ppu->lastBrightnessMult==brightness) return;
    ppu->lastBrightnessMult=brightness;
    for(unsigned i=0;i<32;++i) {
        const uint8_t value=(uint8_t)(((i<<3)|(i>>2))*brightness/15u);
        ppu->brightnessMult[i]=value;
        ppu->brightnessMultHalf[i*2]=ppu->brightnessMultHalf[i*2+1]=value;
    }
    memset(ppu->brightnessMult+32,ppu->brightnessMult[31],31);
}

/* Synchronous drawing only; never a guest or deferred snapshot. */
static inline void Lufia2PpuRenderClone(Ppu *destination, const Ppu *source) {
    if (source->wsMode2CaptureLayer) {
        memcpy(destination, source, sizeof(*source));
        return;
    }
    _Static_assert(offsetof(Ppu, wsMode2Bg1Palette) ==
        offsetof(Ppu, wsMode2Capture) + sizeof(source->wsMode2Capture),
        "PPU capture storage moved");
    const size_t first = offsetof(Ppu, wsMode2Capture);
    const size_t last = offsetof(Ppu, wsMode2Bg1Palette) +
                        sizeof(source->wsMode2Bg1Palette);
    const size_t vram_first = offsetof(Ppu, vram);
    const size_t vram_last = offsetof(Ppu, vram) + sizeof(source->vram);
    _Static_assert(offsetof(Ppu, vram) > offsetof(Ppu, wsMode2Bg1Palette) +
        sizeof(source->wsMode2Bg1Palette), "PPU memory layout moved");
    Lufia2PpuCopyRange(destination,source,0,first);
    Lufia2PpuCopyRange(destination,source,last,vram_first-last);
    Lufia2PpuCopyRange(destination,source,vram_last,sizeof(*source)-vram_last);
    /* Both line renderers read through this existing immutable view. */
    destination->renderVram = PpuRenderVram(source);
}

/* Full-line pixel callbacks overwrite BG and overlay scratch before reading.
 * Retain qualified OBJ, registers, palettes, OAM and every host policy; omit
 * only scratch rows and disabled capture/borrowed VRAM storage. Never use for
 * arbitrary callbacks or a consumer which reads inherited BG priority rows. */
static inline void Lufia2PpuLineClone(Ppu *destination,const Ppu *source) {
    if(source->wsMode2CaptureLayer) {
        memcpy(destination,source,sizeof *source);return;
    }
    const size_t capture=offsetof(Ppu,wsMode2Capture);
    const size_t after_capture=offsetof(Ppu,wsMode2Bg1Palette)+sizeof source->wsMode2Bg1Palette;
    const size_t background=offsetof(Ppu,bgBuffers);
    const size_t object=offsetof(Ppu,objBuffer);
    const size_t after_overlay=offsetof(Ppu,overlayCaptures);
    const size_t vram=offsetof(Ppu,vram);
    const size_t after_vram=vram+sizeof source->vram;
    _Static_assert(offsetof(Ppu,objBuffer)==offsetof(Ppu,bgBuffers)+sizeof source->bgBuffers,
        "PPU scratch layout moved");
    _Static_assert(offsetof(Ppu,overlayBuffers)==offsetof(Ppu,objBuffer)+sizeof source->objBuffer,
        "PPU OBJ layout moved");
    _Static_assert(offsetof(Ppu,overlayCaptures)==offsetof(Ppu,overlayBuffers)+sizeof source->overlayBuffers,
        "PPU overlay layout moved");
    Lufia2PpuCopyRange(destination,source,0,capture);
    Lufia2PpuCopyRange(destination,source,after_capture,background-after_capture);
    Lufia2PpuCopyRange(destination,source,object,sizeof source->objBuffer);
    Lufia2PpuCopyRange(destination,source,after_overlay,vram-after_overlay);
    Lufia2PpuCopyRange(destination,source,after_vram,sizeof *source-after_vram);
    destination->renderVram=PpuRenderVram(source);
}
