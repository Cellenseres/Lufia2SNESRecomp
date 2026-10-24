#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct Ppu;
struct Dma;
struct Snes;

enum { LUFIA2_PPU_TIMELINE_ROWS = 225 };
typedef void (*Lufia2PpuTimelineInterrupt)(void *context);

/* Caller-owned raster history, including the original line-zero state. Strides
 * are bytes. Palette history is optional when no raster CGRAM writes occur.
 * This is the sole live line walk, for both CPU pixels and deferred GPU frames.
 * Pixel fallback must never call it a second time. */
typedef struct Lufia2PpuTimeline {
    uint8_t *registers;
    size_t register_stride;
    uint16_t *palettes;
    size_t palette_stride;
    Lufia2PpuTimelineInterrupt interrupt;
    void *interrupt_context;
} Lufia2PpuTimeline;

/* Validate storage before any guest-visible work. The caller supplies enough
 * storage for all ROWS. Preserves the host's existing V-IRQ trigger rule;
 * H-only IRQ timing is not newly modeled here. */
bool Lufia2PpuTimelineDraw(struct Ppu *ppu, struct Dma *dma, struct Snes *snes,
                          uint8_t hdma_enable, const Lufia2PpuTimeline *timeline);
