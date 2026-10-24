#pragma once
#include "snesrecomp_platform/snes_ppu_job.h"
struct Ppu;
struct Lufia2BattleEffectsPixelPolicy;
struct Lufia2PpuOffload;
typedef struct Lufia2BattlePpuJob Lufia2BattlePpuJob;
enum { LUFIA2_BATTLE_JOB_IMAGES=3, LUFIA2_BATTLE_JOB_OVERLAYS=32 };
Lufia2BattlePpuJob *Lufia2BattlePpuJobCreate(void);
void Lufia2BattlePpuJobDestroy(Lufia2BattlePpuJob *context);
const SnesRecompPpuFrame *Lufia2BattlePpuJobFrame(const Lufia2BattlePpuJob *context);
/* Pure pixels from an immutable capture. Guest execution, HDMA and primary
 * sprite qualification are never repeated. Alternate HUD views qualify only
 * their private clones and cannot publish guest flags. Outputs remain valid until the
 * next preparation/destruction and must be frozen by the caller. */
SnesRecompPpuResult Lufia2BattlePpuJobPrepare(Lufia2BattlePpuJob *context,
    const struct Ppu *live,const SnesRecompPpuFrame *frame,
    SnesRecompPpuJobImage *images,uint16_t *image_count,
    SnesRecompPpuOverlay *overlays,uint16_t *overlay_count,
    const SnesRecompPpuSource **sources,uint16_t *source_count);
/* Shared console consumer: prepare from recorded policy/qualification and
 * submit the derived frame. Declining resets only host HUD scratch state;
 * the caller then replays the original capture through the CPU fallback. */
bool Lufia2BattlePpuJobSubmit(Lufia2BattlePpuJob *context,
    struct Lufia2PpuOffload *offload,const struct Ppu *live,
    const SnesRecompPpuFrame *frame);
/* Submit in two steps: Freeze here, then Lufia2PpuOffloadPrepareFrozen
 * and the presenter submit; Refused when those decline. */
bool Lufia2BattlePpuJobFreeze(Lufia2BattlePpuJob *context,
    struct Lufia2PpuOffload *offload,const struct Ppu *live,
    const SnesRecompPpuFrame *frame);
void Lufia2BattlePpuJobRefused(const struct Ppu *live,const SnesRecompPpuFrame *frame);
SnesRecompPpuResult Lufia2BattlePpuJobPrepareRecorded(Lufia2BattlePpuJob *context,
    const struct Ppu *live,const SnesRecompPpuFrame *frame,
    const struct Lufia2BattleEffectsPixelPolicy *policy,const bool *sprite_rows,
    SnesRecompPpuJobImage *images,uint16_t *image_count,
    SnesRecompPpuOverlay *overlays,uint16_t *overlay_count,
    const SnesRecompPpuSource **sources,uint16_t *source_count);
/* Qualified rows describe the actual OBJ sample line of capture row zero.
 * Alternate scene views may borrow a matching row only with identical OBJ
 * registers; missing rows or changed selection retain private qualification.
 * UINT16_MAX preserves the legacy preparation contract. */
SnesRecompPpuResult Lufia2BattlePpuJobPrepareRecordedRows(Lufia2BattlePpuJob *context,
    const struct Ppu *live,const SnesRecompPpuFrame *frame,
    const struct Lufia2BattleEffectsPixelPolicy *policy,const bool *sprite_rows,
    uint16_t first_sprite_line,SnesRecompPpuJobImage *images,uint16_t *image_count,
    SnesRecompPpuOverlay *overlays,uint16_t *overlay_count,
    const SnesRecompPpuSource **sources,uint16_t *source_count);
