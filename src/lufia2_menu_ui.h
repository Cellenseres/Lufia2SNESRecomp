#ifndef LUFIA2_MENU_UI_H
#define LUFIA2_MENU_UI_H

#include "snes/ppu.h"
#include "snesrecomp_platform/ppu_blit.h"
#include "snesrecomp_platform/snes_ppu_job.h"
#include "snesrecomp_platform/snes_ppu_repeat_axis.h"

enum { LUFIA2_MENU_UI_MAX_OBJECTS = 2048, LUFIA2_MENU_UI_RECORD = 32 };

typedef struct Lufia2MenuUiObject {
    uint32_t id, parent;
    int16_t x, y;
    uint16_t width, height;
    uint8_t kind, slot;
} Lufia2MenuUiObject;

void Lufia2MenuUiInit(const uint8_t *rom, size_t size,
                     const char *layout, const char *preview);
/* Freeze optional layout in RAM; Init restores live reload. */
void Lufia2MenuUiPreload(void);
void Lufia2MenuUiPrepare(Ppu *ppu, bool enabled, unsigned width);
/* Native index consumers may defer unused RGB initialization until the
 * backend selects its path. BeforePixelLine must run before any RGB writes;
 * CPU recovery initializes its already recorded rows explicitly. */
void Lufia2MenuUiPrepareCapture(Ppu *ppu, bool enabled, unsigned width, bool prefer_indices);
void Lufia2MenuUiBeforePixelLine(const Ppu *ppu);
void Lufia2MenuUiLatchSprites(const Ppu *ppu);
void Lufia2MenuUiSpritePixel(int x, unsigned oam);
uint16_t Lufia2MenuUiSpriteOwner(unsigned oam);
void Lufia2MenuUiPlane(Ppu *ppu, unsigned layer, unsigned line);
/* Copy a certified index-cache row directly into the consumer capture.
 * Borrowed inputs end at this call; the matching line consumes the marker. */
bool Lufia2MenuUiCachedIndexRow(Ppu *ppu,unsigned line,const uint16_t *bg1,
    const uint16_t *bg3,const uint16_t *background);
typedef void Lufia2MenuUiRenderer(Ppu *ppu, unsigned line);
/* The isolated callback reads only the supplied PPU and immutable picture
 * memory. External battle/shadow renderers are excluded from row reuse. */
void Lufia2MenuUiLine(Ppu *ppu, unsigned line, Lufia2MenuUiRenderer *draw);
/* Known full-line callbacks overwrite BG/overlay scratch before reading it. */
void Lufia2MenuUiReplayLine(Ppu *ppu, unsigned line, Lufia2MenuUiRenderer *draw);
/* The isolated background pass replaces the complete primary RGB row.
 * Capture/priority work still runs; the discarded primary colour conversion
 * can be skipped before Line performs that exact replacement. */
bool Lufia2MenuUiBackgroundRedrawn(const Ppu *ppu,unsigned line);
/* Consume only freshly completed primary indices. Unrecorded clone callbacks
 * and an early retained-RGB return keep the original isolated renderer. */
typedef struct Lufia2MenuUiPrimitiveRows {
    const uint32_t *pixels[3];
    const uint16_t *priority[2];
} Lufia2MenuUiPrimitiveRows;
bool Lufia2MenuUiPrimitiveRowsForLine(const Ppu *ppu,unsigned line,
                                    Lufia2MenuUiPrimitiveRows *rows);
typedef bool Lufia2MenuUiColourRenderer(Ppu *ppu,unsigned line,unsigned pass,uint32_t *row);
/* Borrow fresh indices for this line; cache hits never invoke draw. */
void Lufia2MenuUiPrimitiveRenderer(const Ppu *ppu,unsigned line,
                                  Lufia2MenuUiColourRenderer *draw);
/* Admit only the exact menu-owned captures with a serial pixel callback.
 * Qualified OBJ and UI primitive capture still run on the original timeline;
 * only the BG2 RGB base is deferred to an immutable native PPU job. */
bool Lufia2MenuUiNativeBaseEligible(const Ppu *ppu);
bool Lufia2MenuUiBeginNativeBase(const Ppu *ppu);
bool Lufia2MenuUiBeginNativePrimitives(const Ppu *ppu);
bool Lufia2MenuUiPrimitiveCaptureActive(const Ppu *ppu);
/* Indexed jobs need row ownership, not CPU background indices or colours. */
bool Lufia2MenuUiRecordIndexedLine(const Ppu *ppu,unsigned line);
/* Rebuild omitted indices from the original recorded row and qualified OBJ.
 * Neither callback qualifies sprites or advances the guest timeline. */
void Lufia2MenuUiRecoverIndexedRow(Ppu *ppu,unsigned line,
    Lufia2MenuUiRenderer *indices,Lufia2MenuUiRenderer *pixels);
/* Restore only omitted colour work from saved indices and a recorded PPU row.
 * No sprite qualification, guest flags or row-count advancement occurs. */
void Lufia2MenuUiRecoverPrimitiveRow(Ppu *ppu,unsigned line,Lufia2MenuUiRenderer *draw);
/* Optional cleaned highlight tilemap, stable until native submission/fallback.
 * Guest VRAM and qualified OBJ rows remain the original authoritative data. */
const uint16_t *Lufia2MenuUiNativeBaseVram(void);
void Lufia2MenuUiEndNativeBase(void);
/* Pure BG2 fallback from a captured PPU row. Suppresses UI capture callbacks;
 * previously captured primitive images, owners and row counts stay intact. */
void Lufia2MenuUiReplayBaseLine(Ppu *ppu,unsigned line,Lufia2MenuUiRenderer *draw);
typedef struct Lufia2MenuUiCacheStats {
    uint64_t hits, misses;
    uint32_t bytes;
    uint64_t geometry_hits, geometry_misses, geometry_rows, geometry_rejects;
    uint64_t prepare_us, geometry_check_us, geometry_resolve_us, geometry_compile_us;
    uint64_t highlight_hits, highlight_misses;
} Lufia2MenuUiCacheStats;
/* Optional monotonic microsecond clock; SDK types stay in the host. Timing
 * occurs at frame stages, never per pixel or guest instruction. */
void Lufia2MenuUiSetClock(uint64_t (*clock_us)(void));
void Lufia2MenuUiGetCacheStats(Lufia2MenuUiCacheStats *stats);
/* Host-resolved primitive sampling for native opaque composition. The caller
 * owns the map; borrowed image pixels remain unchanged until the next PPU
 * frame. Editor previews decline explicitly.
 * No guest execution, sprite qualification or destination pixel work. */
bool Lufia2MenuUiResolveBlits(uint32_t *map, uint32_t pitch, size_t capacity,
                            SnesRecompPpuJobImage images[4], uint16_t *image_count);
/* Context-owned geometry; immutable until the next call/frame. Captured colour
 * data is borrowed only for synchronous presentation. No allocation on hits. */
const SnesRecompBlitPlan *Lufia2MenuUiBlitPlan(unsigned width);
typedef struct Lufia2MenuUiPrimitivePlan {
    const SnesRecompPpuPrimitive *primitives;
    const SnesRecompPpuMaterial *materials;
    uint32_t count;
    uint16_t material_count;
} Lufia2MenuUiPrimitivePlan;
/* Original source context must remain owned until the resulting job freezes.
 * Produces geometry from completed native indices, never RGB surfaces or a
 * destination visibility map. Rejection leaves the legacy renderer available. */
const Lufia2MenuUiPrimitivePlan *Lufia2MenuUiPrimitives(const SnesRecompPpuFrame *graphics);
/* Compact layout for the index-atlas path; no CPU index/RGB/visibility planes.
 * Claims preserve native coverage and ordered text owners. Sprite owners are
 * the already resolved menu slots, never another OAM selection. Axis runs are
 * relative to each object's destination/source. Materials remain source-band
 * decisions in the eventual immutable frame adapter, not layout decisions.
 * This view alone does not admit a frame or suppress its legacy capture. */
typedef SnesRecompPpuIndexedClaim Lufia2MenuUiIndexedClaim;
typedef SnesRecompPpuIndexedObject Lufia2MenuUiIndexedObject;
SNESRECOMP_STATIC_ASSERT(sizeof(Lufia2MenuUiIndexedClaim)==12,"menu indexed claim ABI");
SNESRECOMP_STATIC_ASSERT(sizeof(Lufia2MenuUiIndexedObject)==28,"menu indexed object ABI");
typedef struct Lufia2MenuUiIndexedLayout {
    const Lufia2MenuUiIndexedClaim *claims;
    const Lufia2MenuUiIndexedObject *objects;
    const SnesRecompPpuAxisRun *axes;
    const uint8_t *oam_owners;
    uint32_t claim_count,object_count,axis_count;
    uint64_t generation;
    uint16_t width,height;
} Lufia2MenuUiIndexedLayout;
/* Borrowed until the next call/reset. Allocates bounded staging buffers once;
 * subsequent hits/misses allocate nothing. Capacity failure publishes nothing.
 * The GPU job must own this view before the next preparation. */
const Lufia2MenuUiIndexedLayout *Lufia2MenuUiIndexedLayoutPlan(void);
/* Reserve mandatory owned layout buffers before optional rendering payloads.
 * This performs no scene lookup, guest access or pixel generation. */
bool Lufia2MenuUiReserveIndexedLayout(void);
/* Freeze the compact layout into a shared, owned job. The caller's original
 * timeline capture supplies both immutable frames; this never qualifies OBJ. */
SnesRecompPpuResult Lufia2MenuUiFreezeIndexedLayout(SnesRecompPpuJob *job,
    const SnesRecompPpuFrame *base,const SnesRecompPpuFrame *graphics,
    const SnesRecompPpuObjRun *runs,uint32_t run_count,const uint32_t qualified_lines[8],uint64_t sequence);
void Lufia2MenuUiCompose(uint8_t *pixels, unsigned width,
                        unsigned height, bool authoritative);
void Lufia2MenuUiReset(Ppu *ppu);
bool Lufia2MenuUiLoad(const uint8_t *bytes, size_t size);
unsigned Lufia2MenuUiObjects(const Lufia2MenuUiObject **objects, uint32_t *scene);

#endif
