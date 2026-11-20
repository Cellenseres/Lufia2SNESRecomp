#pragma once

#include "snes/ppu.h"
#include "snesrecomp_platform/snes_ppu_job_types.h"
#include "snesrecomp_platform/snes_ppu_capture.h"
typedef struct Lufia2PpuRowPolicy Lufia2PpuRowPolicy;

void Lufia2BattleEffectsInit(const uint8_t *rom, size_t size);
/* Presentation only; guest state stays unchanged. */
bool Lufia2BattleEffectsPrepare(Ppu *ppu, bool wide,
                               unsigned width, unsigned height);
bool Lufia2BattleEffectsActive(const Ppu *ppu);
/* Zero declines reuse; never wraps or identifies an asset by a freed pointer. */
uint64_t Lufia2BattleEffectsArtGeneration(const Ppu *ppu);
bool Lufia2BattleEffectsPlane(const Ppu *ppu, unsigned layer);
bool Lufia2BattleEffectsPlaneCleared(const Ppu *ppu, unsigned layer);
bool Lufia2BattleEffectsBackground(const Ppu *ppu);
bool Lufia2BattleEffectsSprite(const Ppu *ppu, unsigned slot);
void Lufia2BattleEffectsBeginSprites(void);
/* Host-only pixel policy. Capture before deferring pixels; primary OBJ
 * qualification continues to read the live game state exactly once. */
typedef struct Lufia2BattleEffectsPixelPolicy {
    /* Host-only storage, never a save/wire record. Match the low address
     * bits of the three WRAM ranges so ARM11 can compare aligned words after
     * at most three prefix bytes. Explicit padding is not guest state. */
#if defined(__cplusplus)
    alignas(4) uint8_t scripts;
#else
    _Alignas(4) uint8_t scripts;
#endif
    uint8_t selector;
    uint8_t alignment_padding[2];
    uint8_t groups[0x60], oam[0x220];
    uint8_t actor_padding[3], actors[64 * 0x2d];
} Lufia2BattleEffectsPixelPolicy;
void Lufia2BattleEffectsCapturePixelPolicy(Lufia2BattleEffectsPixelPolicy *policy);
/* Exact semantic comparison; explicit alignment padding is not an input. */
bool Lufia2BattleEffectsPixelPoliciesEqual(const Lufia2BattleEffectsPixelPolicy *a,
    const Lufia2BattleEffectsPixelPolicy *b);
/* Rendering read set only; script/VM state still belongs to the full policy. */
bool Lufia2BattleEffectsPixelRenderPoliciesEqual(const Lufia2BattleEffectsPixelPolicy *a,
    const Lufia2BattleEffectsPixelPolicy *b);
bool Lufia2BattleEffectsPixelPolicyMatches(const Lufia2BattleEffectsPixelPolicy *policy);
const Lufia2BattleEffectsPixelPolicy *Lufia2BattleEffectsBindPixelPolicy(
    const Lufia2BattleEffectsPixelPolicy *policy);
void Lufia2BattleEffectsCaptureSpriteRow(bool pixels[kPpuBufWidth]);
void Lufia2BattleEffectsRestoreSpriteRow(const bool pixels[kPpuBufWidth]);
/* Borrow only within the current synchronous row operation. Qualification
 * or RestoreSpriteRow invalidates these views; never retain them in a job.
 * Centre pixels remain visible independently of the visible-row mask. */
const bool *Lufia2BattleEffectsSpriteRow(void);
const bool *Lufia2BattleEffectsVisibleSpriteRow(const Ppu *ppu);
/* Restart pure preparation without reclassifying the frame from live RAM. */
void Lufia2BattleEffectsRestartPixels(unsigned width, unsigned height);
typedef bool Lufia2BattleEffectsSceneRecorder(void *context, Ppu *scene, unsigned line);
bool Lufia2BattleEffectsBeginGpuScene(Lufia2BattleEffectsSceneRecorder *recorder,void *context);
/* Opt-in for a synchronous private PPU: recorder may mutate ONLY registers,
 * qualified OBJ, lineHasSprites and range/time flags. No retained PPU pointer.
 * Source draws must also satisfy GpuPartyLineScoped's metadata contract.
 * Rejection materializes an isolated clone before unrestricted CPU fallback. */
bool Lufia2BattleEffectsBeginGpuSceneScoped(Lufia2BattleEffectsSceneRecorder *recorder,void *context);
void Lufia2BattleEffectsEndGpuScene(void);
const Lufia2PpuRowPolicy *Lufia2BattleEffectsBindRowPolicy(const Lufia2PpuRowPolicy *policy);
void Lufia2BattleEffectsSpritePixel(int x, bool effect);
bool Lufia2BattleEffectsSpriteVisible(const Ppu *ppu, int x);
bool Lufia2BattleEffectsSpriteMargins(Ppu *ppu, const uint16_t *vram, unsigned line);
/* Synchronous pixel recording only: the original actor ordering, clipping and
 * transparent occupancy decisions stay on the caller. Callbacks must consume
 * values immediately and must not retain live PPU/WRAM pointers. Clearing
 * applies only to winning native effect pixels in the two margin intervals. */
typedef struct Lufia2BattleEffectsMarginWriter {
    void *context;
    void (*clear_effects)(void *context, int left, int right);
    void (*pixel)(void *context, int x, uint16_t native);
} Lufia2BattleEffectsMarginWriter;
bool Lufia2BattleEffectsSpriteMarginsWithWriter(Ppu *ppu, const uint16_t *vram,
    unsigned line, const Lufia2BattleEffectsMarginWriter *writer);
typedef void Lufia2BattleEffectsLineRenderer(Ppu *ppu, unsigned line);
void Lufia2BattleEffectsHudLine(Ppu *ppu, unsigned line,
                               Lufia2BattleEffectsLineRenderer *draw);
void Lufia2BattleEffectsBeginLine(const Ppu *ppu);
void Lufia2BattleEffectsBeginIndexLine(void);
void Lufia2BattleEffectsEndIndexLine(void);
void Lufia2BattleEffectsMargin(Ppu *ppu, PpuPixelPrioBufs *background,
                              unsigned y, bool sub, int left, int right,
                              PpuZbufType low_priority, bool mosaic);
unsigned Lufia2BattleEffectsColour(const Ppu *ppu, unsigned pixel,
                                  unsigned index, bool sub);
int Lufia2BattleEffectsMosaic(int x, unsigned size);
bool Lufia2BattleEffectsOpaque(unsigned pixel, unsigned index, bool sub);
void Lufia2BattleEffectsShutdown(void);
void Lufia2BattleEffectsCacheStats(uint64_t *hits, uint64_t *misses);
bool Lufia2BattleEffectsReadRow(Ppu *ppu, unsigned line);
void Lufia2BattleEffectsStoreRow(const Ppu *ppu, unsigned line);
/* Prepared host sources, independent of the console SDK. Complex unrecorded
 * battle effects remain serial until their complete source contract exists. */
bool Lufia2BattleEffectsGpuCompatible(const Ppu *ppu);
bool Lufia2BattleEffectsGpuArt(const Ppu *ppu,SnesRecompPpuJobImage *image,
    unsigned *center_left,unsigned *center_right);
/* NULL palette borrows the unchanged art for separately resolved raster rows.
 * Otherwise colours use the captured BG1 palette, never end-of-frame CGRAM. */
bool Lufia2BattleEffectsGpuArtPalette(const Ppu *ppu,const uint16_t *palette,
    SnesRecompPpuJobImage *image,unsigned *center_left,unsigned *center_right);
bool Lufia2BattleEffectsGpuArtRow(const uint16_t palette[32],unsigned source_x,
    unsigned source_y,unsigned count,bool repeat,uint32_t *pixels);
/* 1: isolated modern card source plus owned GPU scene, 0: ordinary path,
 * -1: preparation started but must be abandoned. The caller must require a
 * complete ReadCards result before submitting any job that used this path. */
int Lufia2BattleEffectsGpuPartyLine(Ppu *ppu,unsigned line,
    Lufia2BattleEffectsLineRenderer *draw);
/* As GpuPartyLine, but the callback certifies that a NATIVE_SOURCE BG2/3
 * draw preserves scalar policy, memory, qualified OBJ and capture descriptors.
 * BG scratch is disposable. The consumer may then reuse its private scene
 * temporarily instead of copying another Ppu for every card source row. */
int Lufia2BattleEffectsGpuPartyLineScoped(Ppu *ppu,unsigned line,
    Lufia2BattleEffectsLineRenderer *draw);
/* Conservative preflight for a complete immutable GPU job. False means no
 * relocated title/party consumer; already pending/saved scenes retain replay. */
bool Lufia2BattleEffectsGpuHudMayRelocateLine(const Ppu *ppu,unsigned line);
bool Lufia2BattleEffectsGpuHudLine(Ppu *ppu,unsigned line,
    Lufia2BattleEffectsLineRenderer *draw,Lufia2BattleEffectsLineRenderer *indices);
/* Materialize the current immutable row only before a pixel consumer. The
 * callback is synchronous, invoked at most once, and never retained. A true
 * return still asks the caller to prepare inputs before rendering that row. */
typedef void Lufia2BattleEffectsLineInputs(void *context,Ppu *ppu,unsigned line);
bool Lufia2BattleEffectsGpuHudLineWithInputs(Ppu *ppu,unsigned line,
    Lufia2BattleEffectsLineRenderer *draw,Lufia2BattleEffectsLineRenderer *indices,
    Lufia2BattleEffectsLineInputs *inputs,void *context);
void Lufia2BattleEffectsGpuWindows(const Ppu *ppu,unsigned line,SnesPpuResolvedWindow out[6]);
bool Lufia2BattleEffectsWindows(const Ppu *ppu, unsigned layer,
                               int left, int right, int *w1_left, int *w1_right,
                               int *w2_left, int *w2_right, uint32_t *flags);
