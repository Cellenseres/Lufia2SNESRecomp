#pragma once
#include "snes/ppu.h"
#include "snesrecomp_platform/snes_obj_ranges.h"
#include <stdbool.h>
#include <stdint.h>

/* Sprite rows of a recorded frame, rasterized on a helper core. In battle
 * frames the helper also selects the sprites from a snapshot; elsewhere
 * selection stays on the guest thread. */

/* Set by the port before the first frame. */
extern bool lufia2_ppu_obj_defer_enabled;
/* The recording PPU while rows are deferred. */
extern Ppu *lufia2_ppu_obj_defer_ppu;

typedef struct Lufia2PpuObjDeferTarget {
    PpuPixelPrioBufs *objects;
    uint16_t *obj_pixels;
    unsigned width, extra;
    /* Optional battle ownership rows, in complete native scratch coordinates. */
    bool (*battle_masks)[kPpuBufWidth];
    /* Optional paired producer certificates for the packed rows. The owner
     * invalidates rows after any external packed/native/mask mutation or
     * layout/mode change. Worker owns published rows until Stop joins. */
    uint8_t *valid_rows;
    uint8_t *has_rows;
    uint32_t *reused_rows, *generated_rows;
    /* Optional: every packed row gets serial; see SnesPpuFrameCapture. */
    uint64_t *row_serials, serial;
} Lufia2PpuObjDeferTarget;

/* Starts a frame; false keeps every row on the guest thread. */
bool Lufia2PpuObjDeferStart(Ppu *ppu, const Lufia2PpuObjDeferTarget *target);
/* Joins the helper and hands the live row back to the PPU. */
void Lufia2PpuObjDeferStop(void);
/* Closes the rows without waiting; Join or the next Start/Stop finishes. */
bool Lufia2PpuObjDeferDetach(void);
/* Runs fn on the helper after the detached rows; snapshot gets the last row. */
bool Lufia2PpuObjDeferThen(void (*fn)(void *), void *arg, Ppu *snapshot);
void Lufia2PpuObjDeferJoin(void);

/* True when the next line can be deferred. */
bool Lufia2PpuObjDeferLineFits(unsigned line);
/* Called by the deferred evaluator for one line. */
void Lufia2PpuObjDeferBeginLine(unsigned line, unsigned clear_begin, unsigned clear_end,
                                const uint16_t *vram);
void Lufia2PpuObjDeferBeginSprites(void);
void Lufia2PpuObjDeferSliver(int x, unsigned vram_index, uint16_t z,
    bool flip, bool clip, bool effect);
/* Uses the original synchronous actor resolver, recording its final margin
 * pixels plus its ordered native-effect clear. No live state reaches worker. */
bool Lufia2PpuObjDeferBattleMargins(Ppu *ppu, const uint16_t *vram, unsigned line);

/* True once for a line the deferred evaluator took. */
bool Lufia2PpuObjDeferTakeLine(unsigned line);
/* Hands a recorded row to the helper. */
void Lufia2PpuObjDeferPublish(unsigned y, bool has_sprites);

/* Everything the sprite evaluator reads, with the PPU's field names, so the
 * helper can run the same generated evaluator. Per-line fields come first. */
typedef struct Lufia2PpuObjEval {
    uint8_t obsel, oamaddl, oamaddh, setini;
    uint16_t extraLeftCur, extraRightCur;
    bool marginSprites, rangeOver, timeOver;
    bool evenFrame;
    uint16_t extraLeftRight;
    uint32_t renderFlags;
    uint8_t wsHudSplitHeight, wsHudLeftEnd, wsHudRightStart;
    uint8_t wsHudOamFirstSlot, wsHudOamSlots, wsHudOamHeight;
    uint8_t wsHudOamFirstSlot2, wsHudOamSlots2;
    uint8_t wsOamMotionGraceOn, wsOamLeftHintStrict, wsOamRightHintStrict;
    uint8_t wsOamLeftHint[16], wsOamRightHint[16], wsOamMotionGrace[128];
    uint8_t effects[16];
    uint16_t oam[0x100];
    uint8_t highOam[0x20];
} Lufia2PpuObjEval;

/* True while this frame's deferred lines are evaluated on the helper. */
bool Lufia2PpuObjDeferQueueing(void);
/* Hands the open line's evaluation to the helper. */
void Lufia2PpuObjDeferQueue(const Ppu *ppu, unsigned line, bool margin_sprites);
/* Generated evaluator and its callbacks, run on the helper. */
bool Lufia2PpuObjEvalLine(Lufia2PpuObjEval *eval, int line);
void Lufia2PpuObjEvalBeginSprites(void);
void Lufia2PpuObjEvalSliver(int x, unsigned vram_index, uint16_t z,
    bool flip, bool clip, bool effect);
/* snesrecomp_obj_range_row for the evaluator's own snapshot. */
const uint32_t *Lufia2PpuObjEvalRangeRow(SnesRecompObjRangeCache *cache,
    const uint16_t oam[256], const uint8_t high_oam[32],
    unsigned size_mode, bool interlace, unsigned line);
static inline bool Lufia2PpuObjEvalEffect(const Lufia2PpuObjEval *eval, unsigned slot) {
    return (eval->effects[slot >> 3] >> (slot & 7u)) & 1u;
}
/* Before STAT77 is read: sprite overflow flags of evaluated lines. */
void Lufia2PpuObjDeferSyncFlags(Ppu *ppu);

/* Rows the helper produced; for diagnostics. */
uint64_t Lufia2PpuObjDeferRows(void);

#if defined(LUFIA2_OBJ_DEFER_CHECK)
/* Waits for the helper row and compares it with the serial one. */
void Lufia2PpuObjDeferCheck(unsigned y, bool has_sprites,
    const PpuPixelPrioBufs *expected, const bool *expected_mask);
void Lufia2PpuObjDeferCheckStats(uint64_t *checked, uint64_t *mismatched);
#endif
