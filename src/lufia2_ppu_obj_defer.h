#pragma once
#include "snes/ppu.h"
#include <stdbool.h>
#include <stdint.h>

/* Sprite rows of a recorded frame, rasterized on a helper core.
 * Selection, flags and placement stay on the guest thread; the helper
 * only decodes the selected tiles into the rows the capture keeps. */

/* Set by the port before the first frame. */
extern bool lufia2_ppu_obj_defer_enabled;
/* The recording PPU while rows are deferred. */
extern Ppu *lufia2_ppu_obj_defer_ppu;

typedef struct Lufia2PpuObjDeferTarget {
    PpuPixelPrioBufs *objects;
    uint16_t *obj_pixels;
    unsigned width, extra;
} Lufia2PpuObjDeferTarget;

/* Starts a frame; false keeps every row on the guest thread. */
bool Lufia2PpuObjDeferStart(Ppu *ppu, const Lufia2PpuObjDeferTarget *target);
/* Joins the helper and hands the live row back to the PPU. */
void Lufia2PpuObjDeferStop(void);

/* True when the next line can be deferred. */
bool Lufia2PpuObjDeferLineFits(unsigned line);
/* Called by the deferred evaluator for one line. */
void Lufia2PpuObjDeferBeginLine(unsigned line, unsigned clear_begin, unsigned clear_end,
                                const uint16_t *vram);
void Lufia2PpuObjDeferSliver(int x, unsigned vram_index, uint16_t z, bool flip, bool clip);

/* True once for a line the deferred evaluator took. */
bool Lufia2PpuObjDeferTakeLine(unsigned line);
/* Hands a recorded row to the helper. */
void Lufia2PpuObjDeferPublish(unsigned y, bool has_sprites);

/* Rows the helper produced; for diagnostics. */
uint64_t Lufia2PpuObjDeferRows(void);

#if defined(LUFIA2_OBJ_DEFER_CHECK)
/* Waits for the helper row and compares it with the serial one. */
void Lufia2PpuObjDeferCheck(unsigned y, bool has_sprites, const PpuPixelPrioBufs *expected);
void Lufia2PpuObjDeferCheckStats(uint64_t *checked, uint64_t *mismatched);
#endif
