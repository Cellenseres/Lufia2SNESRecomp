#if defined(LUFIA2_ENABLE_NATIVE_WAIT) || defined(LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD) || \
    defined(LUFIA2_ENABLE_ACTOR_EARLY_RETURN)
#include "patches/native_patches.h"
#endif
/* Lufia II frame and interrupt adapter. */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "lufia2_runtime.h"
#include "common_rtl.h"
#include "common_cpu_infra.h"
#include "cpu_state.h"
#include "snes/snes.h"
#include "snes/ppu.h"
#include "snes/dma.h"
#include "snes/interp_bridge.h"
#include "snesrecomp_platform/task.h"


#include <string.h>
#include "lufia2_abi_guard.h"
#include "lufia2_log.h"
static uint8_t s_clean_line_regs[225][PPU_SAVESTATE_REGS_SIZE];
static uint32_t s_clean_raster_flags;
const uint8_t *Lufia2LineRegisters(unsigned y) {
    return y < 224 ? s_clean_line_regs[y + 1] : NULL;
}
uint32_t Lufia2PpuRasterMemoryFlags(void) { return s_clean_raster_flags; }
bool lufia2_gxm_defer_pixels;
static Ppu s_pixel_fallback;
static Ppu s_pixel_helper;
static bool s_fallback_parallel;
static const char *s_pixel_status = "not prepared";
bool Lufia2PixelFallbackWasParallel(void) { return s_fallback_parallel; }
const char *Lufia2PixelOffloadStatus(void) { return s_pixel_status; }

/* Generated interrupt vectors. */
enum {
    LUFIA2_RESET_PC = 0x008000u,
    LUFIA2_NMI_PC   = 0x00862Du,
    LUFIA2_IRQ_PC   = 0x0086C0u,
};

/* Runner frame duration. */
#define LUFIA2_MASTER_CLOCKS_PER_FRAME 357368ull

extern uint8 g_ram[0x20000];
extern Ppu *g_ppu;
extern Dma *g_dma;
extern uint8 g_snesrecomp_last_hdmaen;
extern Snes *g_snes;
extern int snes_frame_counter;
extern bool g_fail;

static bool s_started;
static uint32_t s_resume_pc = LUFIA2_RESET_PC;
static bool s_last_boundary_was_wai;
static uint64_t s_boundaries;
static uint64_t s_nmis;
static uint64_t s_irqs;
static uint64_t s_guest_nmi_us, s_guest_scheduler_us;
void Lufia2GuestPhaseTimes(uint64_t *nmi, uint64_t *scheduler) {
    *nmi = s_guest_nmi_us;
    *scheduler = s_guest_scheduler_us;
}

static bool Lufia2RunToBoundary(uint32_t entry_pc) {
    const uint64_t deadline =
        g_cpu.master_cycles + LUFIA2_MASTER_CLOCKS_PER_FRAME;

#if defined(LUFIA2_ENABLE_NATIVE_WAIT) || defined(LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD) || \
    defined(LUFIA2_ENABLE_ACTOR_EARLY_RETURN)
    Lufia2NativeWaitBegin();
#endif
    interp_bridge_set_master_deadline(deadline);
    const int ok = interp_bridge_run_until_quiescent(&g_cpu, entry_pc);
    interp_bridge_set_master_deadline(0);
#if defined(LUFIA2_ENABLE_NATIVE_WAIT) || defined(LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD) || \
    defined(LUFIA2_ENABLE_ACTOR_EARLY_RETURN)
    Lufia2NativeWaitEnd();
#endif

    if (!ok) {
        fprintf(stderr,
                "[lufia2] LLE boundary run failed at/after $%06X "
                "(frame=%d, S=$%04X, M=%u, X=%u)\n",
                (unsigned)(entry_pc & 0xFFFFFFu),
                snes_frame_counter,
                g_cpu.S,
                (unsigned)(g_cpu.m_flag & 1),
                (unsigned)(g_cpu.x_flag & 1));
        g_fail = true;
        return false;
    }

    s_resume_pc = interp_bridge_lle_resume_pc() & 0xFFFFFFu;
    s_last_boundary_was_wai = interp_bridge_lle_took_wai() != 0;
    s_boundaries++;
    return true;
}

static bool Lufia2RunInterrupt(uint32_t vector_pc) {
    /* Preserve the guest resume PC across the interrupt. */
    cpu_push_interrupt_frame_at(&g_cpu, s_resume_pc);

    if (!interp_bridge_run_interrupt(&g_cpu, vector_pc)) {
        fprintf(stderr,
                "[lufia2] interrupt bridge failed: vector=$%06X "
                "resume=$%06X frame=%d\n",
                (unsigned)vector_pc,
                (unsigned)s_resume_pc,
                snes_frame_counter);
        g_fail = true;
        return false;
    }
    return true;
}

void Lufia2RunOneFrame(void) {
    s_guest_nmi_us = s_guest_scheduler_us = 0;
    if (!s_started) {
        /* Start from the ROM reset vector through the LLE scheduler. */
        cpu_state_init(&g_cpu, g_ram);
        s_started = true;
#if defined(LUFIA2_ENABLE_NATIVE_WAIT) || defined(LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD) || \
    defined(LUFIA2_ENABLE_ACTOR_EARLY_RETURN)
        Lufia2NativePatchesInit();
#endif

        LUFIA2_LOG(
                "[lufia2] starting hybrid LLE/AOT boot at $%06X\n",
                LUFIA2_RESET_PC);

        const uint64_t begin = Lufia2HostNowUs();
        (void)Lufia2RunToBoundary(LUFIA2_RESET_PC);
        s_guest_scheduler_us = Lufia2HostNowUs() - begin;
        return;
    }

    if (g_snes && g_snes->nmiEnabled) {
        const uint64_t begin = Lufia2HostNowUs();
        g_snes->inNmi = true;  /* makes $4210/RDNMI report the pending NMI */
        const bool ok = Lufia2RunInterrupt(LUFIA2_NMI_PC);
        s_guest_nmi_us = Lufia2HostNowUs() - begin;
        if (!ok)
            return;
        s_nmis++;
    }

    const uint64_t begin = Lufia2HostNowUs();
    (void)Lufia2RunToBoundary(s_resume_pc);
    s_guest_scheduler_us = Lufia2HostNowUs() - begin;
}

/* Match the pinned SimpleHdma_DoLine transfer order. Only admit writes whose
 * complete rendering effect is represented by the 64-byte register snapshot.
 * In particular MOSAIC has additional state outside that snapshot. */
static uint32_t Lufia2HdmaMemoryFlags(void) {
    static const uint8_t offsets[8][4] = {
        {0,0,0,0}, {0,1,0,1}, {0,0,0,0}, {0,0,1,1},
        {0,1,2,3}, {0,1,0,1}, {0,0,0,0}, {0,0,1,1},
    };
    static const uint8_t lengths[8] = {1,2,2,4,4,4,2,4};
    uint32_t flags = 0;
    for (unsigned ch = 0; ch < 8; ch++) {
        if (!(g_snesrecomp_last_hdmaen & (1u << ch))) continue;
        unsigned mode = g_dma->channel[ch].mode & 7u;
        for (unsigned j = 0; j < lengths[mode]; j++) {
            unsigned reg = (g_dma->channel[ch].bAdr + offsets[mode][j]) & 255u;
            if (reg == 0x18 || reg == 0x19)
                flags |= LUFIA2_PPU_RASTER_MEMORY_VRAM;
            else if (reg == 0x22)
                flags |= LUFIA2_PPU_RASTER_MEMORY_CGRAM;
            else if (reg == 0x04)
                flags |= LUFIA2_PPU_RASTER_MEMORY_OAM;
            else if (!(reg == 0x00 || (reg >= 0x0d && reg <= 0x14) ||
                       (reg >= 0x23 && reg <= 0x32)))
                flags |= LUFIA2_PPU_RASTER_MEMORY_UNKNOWN;
        }
    }
    return flags;
}

/* Reuse the development renderer's copy-hole optimization. Both capture
 * arrays are inaccessible while wsMode2CaptureLayer is zero (checked below). */
static void Lufia2CopyPixelPpu(Ppu *dst, const Ppu *src) {
    _Static_assert(offsetof(Ppu, wsMode2Bg1Palette) ==
                   offsetof(Ppu, wsMode2Capture) + sizeof(src->wsMode2Capture),
                   "PPU capture arrays must remain adjacent");
    const size_t begin = offsetof(Ppu, wsMode2Capture);
    const size_t end = offsetof(Ppu, wsMode2Bg1Palette) + sizeof(src->wsMode2Bg1Palette);
    memcpy(dst, src, begin);
    memcpy((char *)dst + end, (const char *)src + end, sizeof(*src) - end);
}

static bool Lufia2CanRenderBands(void) {
#if defined(SNESRECOMP_INTERP_PROFILE) || SNESRECOMP_REVERSE_DEBUG
    return false; /* Core diagnostics contain shared writable state. */
#else
    if (!snesrecomp_task_worker_count() ||
        PPU_mode(g_ppu) != 1 || g_ppu->mosaic ||
        g_ppu->extraLeftCur || g_ppu->extraRightCur)
        return false;
    /* With zero live margins WsShadowTile returns before its shared counters;
     * the extraLeft/Right check above therefore also makes shadow reads safe. */
    return true;
#endif
}

typedef struct Lufia2PixelBand {
    Ppu *ppu;
    int first, last;
} Lufia2PixelBand;

static void Lufia2RenderPixelBand(void *opaque) {
    const Lufia2PixelBand *band = opaque;
    for (int line = band->first; line <= band->last; line++) {
        memcpy(&band->ppu->inidisp, s_clean_line_regs[line], PPU_SAVESTATE_REGS_SIZE);
        ppu_runLine(band->ppu, line);
    }
}

/* Guest scheduling stays serial; only pixels can be deferred. */
bool Lufia2PreparePixelOffload(unsigned width, unsigned extra) {
    lufia2_gxm_defer_pixels = false;
    s_fallback_parallel = false;
    s_pixel_status = "not prepared";
    if (!g_ppu || !g_dma || !g_snes) {
        s_pixel_status = "missing PPU/DMA/SNES";
        return false;
    }
    if (g_snes->vIrqEnabled || g_snes->hIrqEnabled) {
        s_pixel_status = "IRQ frame";
        return false;
    }
    if (g_ppu->wsMode2CaptureLayer || g_ppu->widescreenLineEnhancer) {
        s_pixel_status = "capture/enhancer";
        return false;
    }
    if (!(g_ppu->renderFlags & kPpuRenderFlags_NewRenderer)) {
        s_pixel_status = "legacy renderer";
        return false;
    }
    for (unsigned i = 0;
         i < sizeof(g_ppu->overlayRenderBuffer) / sizeof(g_ppu->overlayRenderBuffer[0]); i++)
        if (g_ppu->overlayRenderBuffer[i]) {
            s_pixel_status = "overlay buffer";
            return false;
        }
    s_clean_raster_flags = Lufia2HdmaMemoryFlags();
    if (s_clean_raster_flags) {
        s_pixel_status = "HDMA memory/unsupported register";
        return false;
    }
    /* Preflight the initial state. The real per-line capture is checked again
     * after the original HDMA walk; unsupported rasters replay CPU pixels. */
    for (int line = 1; line <= 224; line++)
        memcpy(s_clean_line_regs[line], &g_ppu->inidisp, PPU_SAVESTATE_REGS_SIZE);
    if (!Lufia2CanRenderBands() && !Lufia2GxmCanDefer(width, extra)) {
        /* Lufia2GxmCanDefer records the exact fail-closed reason. It points at
         * static storage, so propagating it here adds no allocation or logging
         * to the per-frame hot path. */
        s_pixel_status = Lufia2GxmFrameStatus();
        return false;
    }
    /* Preserve line-0 state too: evenFrame and OAM history advance exactly
     * once in the live PPU, and independently in a failed-draw fallback. */
    Lufia2CopyPixelPpu(&s_pixel_fallback, g_ppu);
    lufia2_gxm_defer_pixels = true;
    s_pixel_status = "deferred";
    return true;
}

void Lufia2FinishPixelOffload(bool gpu_drawn) {
    const bool deferred = lufia2_gxm_defer_pixels;
    lufia2_gxm_defer_pixels = false;
    if (deferred && gpu_drawn) s_pixel_status = "GXM";
    if (deferred && !gpu_drawn) {
        /* Render to the same framebuffer, but never replay HDMA, interrupts,
         * or line-0/overflow state on the live PPU. */
        /* Initialize mosaic/evenFrame/OAM history once, before splitting. The
         * live line walk has already initialized the core's global OAM stamp. */
        memcpy(&s_pixel_fallback.inidisp, s_clean_line_regs[0], PPU_SAVESTATE_REGS_SIZE);
        ppu_runLine(&s_pixel_fallback, 0);
        if (Lufia2CanRenderBands()) {
            Lufia2CopyPixelPpu(&s_pixel_helper, &s_pixel_fallback);
            Lufia2PixelBand first = {&s_pixel_helper, 1, 112};
            Lufia2PixelBand last = {&s_pixel_fallback, 113, 224};
            if (snesrecomp_task_submit(0, Lufia2RenderPixelBand, &first)) {
                Lufia2RenderPixelBand(&last);
                snesrecomp_task_wait();
                s_fallback_parallel = true;
                s_pixel_status = "CPU parallel replay";
                return;
            }
        }
        Lufia2PixelBand all = {&s_pixel_fallback, 1, 224};
        Lufia2RenderPixelBand(&all);
        s_pixel_status = "CPU serial replay";
    }
}

void Lufia2DrawPpuFrame(void) {
    if (!g_ppu || !g_dma || !g_snes)
        return;

    s_clean_raster_flags = Lufia2HdmaMemoryFlags();
    if (g_snes->vIrqEnabled || g_snes->hIrqEnabled)
        s_clean_raster_flags |= LUFIA2_PPU_RASTER_MEMORY_UNKNOWN;

    /* Drive all HDMA channels. */
    SimpleHdma hdma[8];

    dma_startDma(g_dma, g_snesrecomp_last_hdmaen, true);
    for (int ch = 0; ch < 8; ch++)
        SimpleHdma_Init(&hdma[ch], &g_dma->channel[ch]);

    int trigger = g_snes->vIrqEnabled ? (int)g_snes->vTimer + 1 : -1;

    for (int line = 0; line <= 224; line++) {
        memcpy(s_clean_line_regs[line], &g_ppu->inidisp, PPU_SAVESTATE_REGS_SIZE);
        ppu_runLine(g_ppu, line);

        for (int ch = 0; ch < 8; ch++)
            SimpleHdma_DoLine(&hdma[ch]);

        /* H-only IRQ timing is not modeled yet. */
        if (line == trigger) {
            g_snes->inIrq = true;
            if (Lufia2RunInterrupt(LUFIA2_IRQ_PC))
                s_irqs++;

            trigger = g_snes->vIrqEnabled
                    ? (int)g_snes->vTimer + 1
                    : -1;
        }
    }
}

void Lufia2PrintDiagnostics(void) {
    long tier_hits = interp_tier_hit_count();

    LUFIA2_LOG(
            "[lufia2] f=%d resume=$%06X %s "
            "A=%04X X=%04X Y=%04X S=%04X D=%04X "
            "PB=%02X DB=%02X M=%u Xf=%u "
            "NMI=%llu IRQ=%llu boundaries=%llu tier2=%ld "
            "NMIen=%u VIrq=%u HIrq=%u\n",
            snes_frame_counter,
            (unsigned)s_resume_pc,
            s_last_boundary_was_wai ? "WAI" : "QUIET",
            g_cpu.A, g_cpu.X, g_cpu.Y, g_cpu.S, g_cpu.D,
            g_cpu.PB, g_cpu.DB,
            (unsigned)(g_cpu.m_flag & 1),
            (unsigned)(g_cpu.x_flag & 1),
            (unsigned long long)s_nmis,
            (unsigned long long)s_irqs,
            (unsigned long long)s_boundaries,
            tier_hits,
            g_snes ? (unsigned)g_snes->nmiEnabled : 0,
            g_snes ? (unsigned)g_snes->vIrqEnabled : 0,
            g_snes ? (unsigned)g_snes->hIrqEnabled : 0);
}
