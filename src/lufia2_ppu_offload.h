#pragma once
#include "snesrecomp_platform/presenter.h"
struct Ppu;
struct Lufia2BattleEffectsPixelPolicy;
typedef struct Lufia2PpuOffload Lufia2PpuOffload;
Lufia2PpuOffload *Lufia2PpuOffloadCreate(SnesRecompPresenter *presenter);
void Lufia2PpuOffloadDestroy(Lufia2PpuOffload *context);
bool Lufia2PpuOffloadBegin(Lufia2PpuOffload *context, struct Ppu *ppu,
                         unsigned width, unsigned extra, bool eligible);
/* After successful Begin, defer clearing the unused CPU destination until
 * an actual fallback. Consumers must clear serial frames themselves. */
void Lufia2PpuOffloadDeferDestinationClear(Lufia2PpuOffload *context);
/* Called once after authoritative sprite selection, before pixel drawing. */
bool Lufia2PpuOffloadRecordLine(struct Ppu *ppu, unsigned line);
/* The write adapter calls this before mutating unrecorded raster memory.
 * A cheap guard keeps inactive guest writes free of a function call. */
extern bool lufia2_ppu_offload_recording;
void Lufia2PpuOffloadBeforeMemoryWrite(struct Ppu *ppu, uint8_t port);
/* Finish while the consumer's render-memory overlay remains bound. */
bool Lufia2PpuOffloadEnd(Lufia2PpuOffload *context);
/* Separate capture from pure consumer preparation. The returned immutable
 * frame is borrowed until the next Begin or destruction; retain it explicitly
 * for longer use. Capture does not execute the guest or queue GPU work.
 * PrepareOverlays is required before Submit, or Fallback on any failure. */
const SnesRecompPpuFrame *Lufia2PpuOffloadEndCapture(Lufia2PpuOffload *context);
bool Lufia2PpuOffloadPrepareOverlays(Lufia2PpuOffload *context,
    const SnesRecompPpuJobImage *images, uint16_t image_count,
    const SnesRecompPpuOverlay *overlays, uint16_t overlay_count);
/* Complete menu base + resolved UI. Only a context that admitted the known
 * menu capture can use this path. Failure retains its exact CPU fallback. */
bool Lufia2PpuOffloadMenuBase(const Lufia2PpuOffload *context);
bool Lufia2PpuOffloadMenuPrimitives(const Lufia2PpuOffload *context);
bool Lufia2PpuOffloadPrepareMenu(Lufia2PpuOffload *context);
/* PrepareMenu in two steps: consumer freeze, then presenter work. */
bool Lufia2PpuOffloadFreezeMenu(Lufia2PpuOffload *context);
/* Presenter step of either freeze; safe on the helper core. */
bool Lufia2PpuOffloadPrepareFrozen(Lufia2PpuOffload *context);
bool Lufia2PpuOffloadPrepareSources(Lufia2PpuOffload *context,
    const SnesRecompPpuJobImage *images, uint16_t image_count,
    const SnesRecompPpuOverlay *overlays, uint16_t overlay_count,
    const SnesRecompPpuSource *sources, uint16_t source_count);
bool Lufia2PpuOffloadPrepareResolvedSources(Lufia2PpuOffload *context,
    const SnesRecompPpuFrame *resolved_frame,
    const SnesRecompPpuJobImage *images, uint16_t image_count,
    const SnesRecompPpuOverlay *overlays, uint16_t overlay_count,
    const SnesRecompPpuSource *sources, uint16_t source_count);
/* PrepareResolvedSources without its presenter step. */
bool Lufia2PpuOffloadFreezeResolvedSources(Lufia2PpuOffload *context,
    const SnesRecompPpuFrame *resolved_frame,
    const SnesRecompPpuJobImage *images, uint16_t image_count,
    const SnesRecompPpuOverlay *overlays, uint16_t overlay_count,
    const SnesRecompPpuSource *sources, uint16_t source_count);
/* Consumer-resolved overlays, in their original postprocessing order. Source
 * art is copied before submission; no guest callbacks or destination readback.
 * On rejection the caller uses Fallback and its ordinary CPU postprocessing.
 * Never omit the CPU postprocessing merely because capture succeeded. */
bool Lufia2PpuOffloadEndWithOverlays(Lufia2PpuOffload *context,
    const SnesRecompPpuJobImage *images, uint16_t image_count,
    const SnesRecompPpuOverlay *overlays, uint16_t overlay_count);
bool Lufia2PpuOffloadSubmit(Lufia2PpuOffload *context);
/* Submit in two steps: presenter work, then menu state on the guest thread. */
bool Lufia2PpuOffloadSubmitPresenter(Lufia2PpuOffload *context);
void Lufia2PpuOffloadSubmitted(Lufia2PpuOffload *context);
void Lufia2PpuOffloadFallback(Lufia2PpuOffload *context);
const struct Lufia2BattleEffectsPixelPolicy *Lufia2PpuOffloadBattlePolicy(
    const Lufia2PpuOffload *context, const bool **sprite_rows);
void Lufia2PpuOffloadStats(const Lufia2PpuOffload *context, SnesRecompPpuStats *out);
/* Generated core adapter: pure pixels, no guest or sprite advancement. */
void Lufia2PpuReplayPixelLine(struct Ppu *ppu, unsigned line);
void Lufia2PpuReplayMenuIndexLine(struct Ppu *ppu,unsigned line);
/* Isolated alternate battle view, using captured OAM on a private PPU clone.
 * Its OBJ flags never reach the guest; the primary row is not qualified again. */
void Lufia2PpuReplayBattleSourceLine(struct Ppu *ppu, unsigned line);
/* Main-screen priority attribution only; no RGB image, subscreen, callbacks,
 * guest flags or primary OBJ qualification. */
void Lufia2PpuReplayBattleIndexLine(struct Ppu *ppu, unsigned line);
/* Alternate visual clone only; qualified OBJ is recorded for the GPU scene. */
void Lufia2PpuQualifyBattleSourceLine(struct Ppu *ppu, unsigned line);
/* Exact maintained-CPU source lookup; map has 64*64 packed RGBA entries and
 * known has one byte per entry. Incompatible aliases require CPU fallback. */
bool Lufia2PpuResolveShadowLine(struct Ppu *ppu, unsigned line, unsigned layer,
                              uint8_t *map, uint8_t *known);
/* Per-frame source-row memo. Memory barriers abandon capture before any
 * unrecorded VRAM/OAM write; palette writes do not alter tilemap entries. */
typedef struct Lufia2PpuShadowRowKey {
    uint8_t regs[SNES_PPU_REG_BLOCK_BYTES];
    uint32_t native_row, history_row, world_row, world_x, world_y, scroll_y;
    uint16_t extra_left, extra_right, margin_left, margin_right;
    uint8_t expand_layers, expand_windows;
} Lufia2PpuShadowRowKey;
typedef struct Lufia2PpuShadowMemo {
    Lufia2PpuShadowRowKey key;
    uint32_t resolved, reused;
    bool valid;
} Lufia2PpuShadowMemo;
bool Lufia2PpuResolveShadowLineCached(struct Ppu *ppu, unsigned line, unsigned layer,
    uint8_t *map, uint8_t *known, Lufia2PpuShadowMemo *memo);
