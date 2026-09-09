#pragma once

#include "snesrecomp_platform/snes_ppu_capture.h"
#include <stddef.h>
#include <stdint.h>

#include "common_cpu_infra.h"
#include "snesrecomp_platform/host_boot.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const RtlGameInfo kLufia2GameInfo;
extern const SnesRecompHostGame kLufia2HostGame;

void Lufia2RunOneFrame(void);
void Lufia2DrawPpuFrame(void);
/* `wallclock_ms` is the host clock at the checkpoint. It turns the
 * diagnostic line into a throughput measurement as well as a state
 * fingerprint, which is what tells a slow build from a wrong one. */
void Lufia2PrintDiagnostics(void);
bool Lufia2GuestProgressed(void);

/* Renders the PPU line loop across two cores where the frame allows it.
 * Returns false when the helper could not be started; rendering then
 * stays on one core and remains correct. */
bool Lufia2PpuParallelInit(void);
/* Joins the bands of the frame that is still rendering. Must be called before
 * the frame is presented and before the next frame is snapshotted; both hold
 * if it is the first thing the frame loop does. No-op when nothing is in
 * flight. */
void Lufia2PpuWait(void);

bool Lufia2PpuPipelined(void);

/* Conservative description of enabled HDMA destinations for the frame most
 * recently advanced. Register-only HDMA is fully represented by the per-line
 * snapshots; writes through these memory ports are not, because VRAM/CGRAM/
 * OAM are currently captured once at frame end. */
enum {
    LUFIA2_PPU_RASTER_MEMORY_VRAM = SNES_PPU_RASTER_MEMORY_VRAM,
    LUFIA2_PPU_RASTER_MEMORY_CGRAM = SNES_PPU_RASTER_MEMORY_CGRAM,
    LUFIA2_PPU_RASTER_MEMORY_OAM = SNES_PPU_RASTER_MEMORY_OAM,
    LUFIA2_PPU_RASTER_MEMORY_UNKNOWN = SNES_PPU_RASTER_MEMORY_UNKNOWN,
};
uint32_t Lufia2PpuRasterMemoryFlags(void);

void Lufia2PpuParallelStats(uint64_t *parallel_frames,
                            uint64_t *serial_frames);

void Lufia2ReportGpuBackgrounds(void);

/* Visible scanlines. The PPU renders lines 1..224; line 0 draws nothing. */
enum { LUFIA2_PPU_VISIBLE_LINES = 224 };

/* The PPU register file as it stood when visible line `y` was rendered, or
 * NULL if `y` is out of range. This is the per-line HDMA walk the banded
 * software renderer already performs; a renderer that reads the frame-end
 * registers instead is drawing a frame the hardware never produced. */
const uint8_t *Lufia2LineRegisters(unsigned y);

/* Advances one frame of PPU raster state -- HDMA on every line, the per-line
 * register history, the helper snapshots -- without composing pixels. Returns
 * true when the pixels still have to be drawn by someone, which is when a
 * backend may claim the frame. False means the frame was rasterised as a side
 * effect and is already finished. */
bool Lufia2AdvancePpuFrame(void);

/* Composes the pixels for a frame the advance deferred. Skipping this is what
 * makes a GPU frame an offload rather than a duplicate. */
void Lufia2RasterisePpuFrame(void);

/* Installs the frame-buffer clear. It is called immediately before pixels are
 * composed, by whichever path composes them, and not at all when a backend
 * claims the frame. The caller cannot place this itself: only the runtime
 * knows whether the advance rasterised inline or deferred. */
void Lufia2SetFrameClearHook(void (*fn)(void));

/* Where two frames first disagree, and by how much. Alpha is excluded
 * throughout: the software PPU never writes it and a GPU target forces it, so
 * comparing it would report a difference that means nothing. */
typedef struct Lufia2FrameDiff {
    unsigned total;
    unsigned different;
    int first_x, first_y;
    uint32_t first_a, first_b;      /* 0x00RRGGBB */
    unsigned max_r, max_g, max_b;
} Lufia2FrameDiff;

struct SnesPpuTrace;

/* Renders a loaded trace with the software PPU. No guest execution, no HDMA,
 * no window, no audio, and nothing read from the running process: if the
 * picture depends on it, it is in the trace. Deterministic. */
bool Lufia2RenderPpuTraceCpu(const struct SnesPpuTrace *trace,
                             uint32_t *out_pixels,
                             size_t pitch_bytes);

unsigned Lufia2ComparePpuFrames(const uint32_t *a, const uint32_t *b,
                                unsigned width, unsigned height,
                                size_t pitch_bytes,
                                Lufia2FrameDiff *out);

unsigned Lufia2CountNonblackRgb(const uint32_t *pixels, unsigned width,
                                unsigned height, size_t pitch_bytes,
                                int *first_x, int *first_y);

/* Describes this frame in SNES terms, merging consecutive scanlines whose
 * state is identical into raster bands. Returns false if the frame cannot be
 * described, which is itself a reason to fall back. */
bool Lufia2CapturePpuFrame(SnesPpuFrameCapture *out,
                           unsigned canvas_width,
                           unsigned canvas_extra);

/* Portable Mode-1/Mode-7 eligibility, independent of whether this executable
 * has a native GPU backend. Production combines it with backend availability
 * before rendering, so desktop can validate captures without claiming an
 * OpenGL or GXM implementation. */
SnesPpuUnsupported Lufia2GpuPpuSupports(const SnesPpuFrameCapture *cap);

/* Optional source for widescreen margin tilemap cells. The trace format does
 * not yet carry the live widescreen shadow, so the dependency is explicit:
 * production supplies this resolver, while a self-contained trace without
 * SNES_PPU_TRACE_FLAG_NEEDS_WS_SHADOW leaves it NULL. */
typedef bool (*Lufia2GpuMarginTileLookup)(void *opaque, unsigned bg,
                                          uint32_t world_x,
                                          uint32_t world_y,
                                          uint16_t *entry);

typedef struct Lufia2GpuPpuInput {
    const SnesPpuFrameCapture *capture;
    Lufia2GpuMarginTileLookup margin_tile_lookup;
    void *margin_tile_opaque;

    /* Stable identity of any tile source outside VRAM. Zero is the normal
     * production source; trace replay uses the trace capture hash. This keeps
     * caches content/state keyed rather than keyed on transient pointers. */
    uint64_t tilemap_source_id;
    bool backdrop_only;

    /* Non-NULL selects the diagnostic 342x224 target and receives normalized
     * 0x00RRGGBB pixels. NULL is the normal display/present path. */
    uint32_t *readback_pixels;
    size_t readback_pitch_bytes;
} Lufia2GpuPpuInput;

/* Shared native renderer for production and trace replay. It reads only input. */
bool Lufia2GpuPpuRenderInput(const Lufia2GpuPpuInput *input);

/* Lets the multi-pass semantic compositor claim production frames. Off until
 * it measures faster than the software renderer; diagnostic replay always
 * uses it regardless. */
void Lufia2SetSemanticProductionEnabled(bool enabled);

/* Enables the separately guarded exact basic Mode 7 GXM path. Keeping this
 * independent from the Mode 1 semantic-production switch permits one focused
 * hardware validation without enabling the slower general compositor. */
void Lufia2SetMode7ProductionEnabled(bool enabled);

/* Invalidates derived trace state without recreating persistent GPU resources. */
void Lufia2GpuPpuResetDiagnosticState(void);

/* Production convenience wrapper. It supplies the live widescreen shadow as
 * an explicit margin source, but all SNES PPU state still comes from `cap`. */
bool Lufia2GpuPpuRender(const SnesPpuFrameCapture *cap, bool backdrop_only);

#ifdef __cplusplus
}
#endif

uint64_t Lufia2HostNowUs(void);
bool Lufia2TryGxmFrame(unsigned width, unsigned extra);
const char *Lufia2GxmFrameStatus(void);
bool Lufia2PreparePixelOffload(unsigned width, unsigned extra);
void Lufia2FinishPixelOffload(bool gpu_drawn);
bool Lufia2PixelFallbackWasParallel(void);
const char *Lufia2PixelOffloadStatus(void);
bool Lufia2GxmCanDefer(unsigned width, unsigned extra);
void Lufia2GuestPhaseTimes(uint64_t *nmi, uint64_t *scheduler);
