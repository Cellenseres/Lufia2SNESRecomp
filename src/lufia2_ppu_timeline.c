#include "lufia2_ppu_timeline.h"
#include "common_rtl.h"
#include "snes/dma.h"
#include "snes/ppu.h"
#include "snes/snes.h"
#include <string.h>

static uint32_t s_policy_epoch, s_next_policy_epoch;
static bool s_drawing;

uint32_t Lufia2PpuTimelineReadOnlyEpoch(void) { return s_policy_epoch; }

static void ResumeReadOnlyInterval(void) {
    if (!++s_next_policy_epoch) ++s_next_policy_epoch;
    s_policy_epoch = s_next_policy_epoch;
}

bool Lufia2PpuTimelineDraw(Ppu *ppu, Dma *dma, Snes *snes,
                          uint8_t hdma_enable, const Lufia2PpuTimeline *timeline) {
    if (s_drawing || !ppu || !dma || !snes || !timeline || !timeline->registers ||
        timeline->register_stride < PPU_SAVESTATE_REGS_SIZE ||
        timeline->register_stride > SIZE_MAX / LUFIA2_PPU_TIMELINE_ROWS ||
        (timeline->palettes && (timeline->palette_stride < sizeof ppu->cgram ||
            timeline->palette_stride > SIZE_MAX / LUFIA2_PPU_TIMELINE_ROWS)) ||
        !timeline->interrupt) return false;

    s_drawing = true;
    /* The host owns HDMA for this frame. Reset can re-enable the core's beam,
     * so set this every time. Otherwise interrupt guest cycles run HDMA twice. */
    snes_set_hdma_beam_enabled(snes, false);
    dma_startDma(dma, hdma_enable, true);
    /* No core beam consumes the priming slot on the host's line timeline. */
    dma_hdma_pending_init_set(dma, 0);
    SimpleHdma hdma[8];
    SimpleHdma *active[8];
    unsigned active_count = 0;
    for (unsigned channel = 0; channel < 8; ++channel) {
        SimpleHdma_Init(&hdma[channel], &dma->channel[channel]);
        if (hdma[channel].table) active[active_count++] = &hdma[channel];
    }

    ResumeReadOnlyInterval();
    int trigger = snes->vIrqEnabled ? (int)snes->vTimer + 1 : -1;
    for (unsigned line = 0; line < LUFIA2_PPU_TIMELINE_ROWS; ++line) {
        memcpy(timeline->registers + line * timeline->register_stride,
               &ppu->inidisp, PPU_SAVESTATE_REGS_SIZE);
        if (timeline->palettes)
            memcpy((uint8_t *)timeline->palettes + line * timeline->palette_stride,
                   ppu->cgram, sizeof ppu->cgram);
        ppu_runLine(ppu, (int)line);
        /* A null table is an absorbing state in SimpleHdma_DoLine: it has no
         * bus access, counter update or reinitialization. Keep the original
         * channel order, and retire only tables the authoritative walker has
         * ended. IRQ changes to DMA registers cannot restart these local
         * cursors; that still happens at next frame's original initialization. */
        unsigned remaining = 0;
        for (unsigned channel = 0; channel < active_count; ++channel) {
            SimpleHdma *cursor = active[channel];
            SimpleHdma_DoLine(cursor);
            if (cursor->table) active[remaining++] = cursor;
        }
        active_count = remaining;
        if ((int)line == trigger) {
            s_policy_epoch = 0;
            snes->inIrq = true;
            timeline->interrupt(timeline->interrupt_context);
            ResumeReadOnlyInterval();
            trigger = snes->vIrqEnabled ? (int)snes->vTimer + 1 : -1;
        }
    }
    s_policy_epoch = 0;
    s_drawing = false;
    return true;
}
