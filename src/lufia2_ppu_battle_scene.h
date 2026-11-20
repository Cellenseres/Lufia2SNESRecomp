#pragma once
#include "snes/ppu.h"
#include <string.h>
#include <stddef.h>

/* Synchronous, already-private battle PPU only. The registered scene recorder
 * may replace registers and qualified OBJ/status; the certified card draw
 * leaves other metadata untouched. BG/overlay scratch is disposable. Nothing
 * here is a frame snapshot, a guest loan or an asynchronous GPU reference. */
typedef struct Lufia2PpuBattleSceneState {
    uint8_t registers[PPU_SAVESTATE_REGS_SIZE];
    PpuPixelPrioBufs objects;
    const uint16_t *render_vram;
    bool sprites, range_over, time_over;
} Lufia2PpuBattleSceneState;

static inline void Lufia2PpuBattleSceneBegin(Lufia2PpuBattleSceneState *saved,
    Ppu *ppu) {
    memcpy(saved->registers,(const uint8_t *)ppu+offsetof(Ppu,inidisp),sizeof saved->registers);
    saved->objects=ppu->objBuffer;
    saved->render_vram=ppu->renderVram;
    saved->sprites=ppu->lineHasSprites;
    saved->range_over=ppu->rangeOver;saved->time_over=ppu->timeOver;
    /* Match LineClone's immutable memory view, including an existing borrow. */
    ppu->renderVram=PpuRenderVram(ppu);
}
static inline void Lufia2PpuBattleSceneEnd(Ppu *ppu,
    const Lufia2PpuBattleSceneState *saved) {
    memcpy((uint8_t *)ppu+offsetof(Ppu,inidisp),saved->registers,sizeof saved->registers);
    ppu->objBuffer=saved->objects;
    ppu->renderVram=saved->render_vram;
    ppu->lineHasSprites=saved->sprites;
    ppu->rangeOver=saved->range_over;ppu->timeOver=saved->time_over;
}
