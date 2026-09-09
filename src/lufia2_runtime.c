#if defined(LUFIA2_ENABLE_NATIVE_WAIT) || defined(LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD) || \
    defined(LUFIA2_ENABLE_ACTOR_EARLY_RETURN)
#include "patches/native_patches.h"
#endif
/* Lufia II frame and interrupt adapter. */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "lufia2_runtime.h"
#include "common_rtl.h"
#include "common_cpu_infra.h"
#include "cpu_state.h"
#include "snes/snes.h"
#include "snes/ppu.h"
#include "snes/dma.h"
#include "snes/interp_bridge.h"
#include "lufia2_log.h"

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
static uint8_t s_line_regs[225][PPU_SAVESTATE_REGS_SIZE];
static bool s_line_regs_valid;
static uint32_t s_raster_memory_flags;

const uint8_t *Lufia2LineRegisters(unsigned y) {
    return y < LUFIA2_PPU_VISIBLE_LINES ? s_line_regs[y + 1u] : NULL;
}

bool Lufia2PpuRasterHistory(const uint8_t **rows, size_t *stride) {
    if (rows)
        *rows = s_line_regs[1];
    if (stride)
        *stride = sizeof s_line_regs[0];
    return s_line_regs_valid;
}

uint32_t Lufia2ResumePc(void) {
    return s_resume_pc & 0xFFFFFFu;
}

uint32_t Lufia2PpuRasterMemoryFlags(void) {
    return s_raster_memory_flags;
}

/* Match SimpleHdma_DoLine's register sequence. Register-only writes are
 * represented by s_line_regs; memory-port writes are not reconstructible from
 * the frame-end VRAM/CGRAM/OAM snapshots and therefore force fallback. */
static uint32_t Lufia2HdmaMemoryFlags(void) {
    static const uint8_t offsets[8][4] = {
        {0,0,0,0}, {0,1,0,1}, {0,0,0,0}, {0,0,1,1},
        {0,1,2,3}, {0,1,0,1}, {0,0,0,0}, {0,0,1,1},
    };
    static const uint8_t lengths[8] = {1,2,2,4,4,4,2,4};
    uint32_t flags = 0;

    for (unsigned ch = 0; ch < 8u; ch++) {
        unsigned mode;
        if (!(g_snesrecomp_last_hdmaen & (1u << ch)))
            continue;
        mode = g_dma->channel[ch].mode & 7u;
        for (unsigned j = 0; j < lengths[mode]; j++) {
            const unsigned reg =
                (g_dma->channel[ch].bAdr + offsets[mode][j]) & 255u;
            if (reg == 0x18u || reg == 0x19u)
                flags |= SNES_PPU_RASTER_MEMORY_VRAM;
            else if (reg == 0x22u)
                flags |= SNES_PPU_RASTER_MEMORY_CGRAM;
            else if (reg == 0x04u)
                flags |= SNES_PPU_RASTER_MEMORY_OAM;
            else if (!(reg == 0x00u ||
                       (reg >= 0x0du && reg <= 0x14u) ||
                       (reg >= 0x1au && reg <= 0x20u) ||
                       (reg >= 0x23u && reg <= 0x32u)))
                flags |= SNES_PPU_RASTER_MEMORY_UNKNOWN;
        }
    }
    return flags;
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

        (void)Lufia2RunToBoundary(LUFIA2_RESET_PC);
        return;
    }

    if (g_snes && g_snes->nmiEnabled) {
        g_snes->inNmi = true;  /* makes $4210/RDNMI report the pending NMI */
        if (!Lufia2RunInterrupt(LUFIA2_NMI_PC))
            return;
        s_nmis++;
    }

    (void)Lufia2RunToBoundary(s_resume_pc);
}

void Lufia2DrawPpuFrame(void) {
    if (!g_ppu || !g_dma || !g_snes)
        return;

    s_raster_memory_flags = Lufia2HdmaMemoryFlags();
    if (g_snes->vIrqEnabled || g_snes->hIrqEnabled)
        s_raster_memory_flags |= SNES_PPU_RASTER_MEMORY_UNKNOWN;

    /* Drive all HDMA channels. */
    SimpleHdma hdma[8];

    dma_startDma(g_dma, g_snesrecomp_last_hdmaen, true);
    for (int ch = 0; ch < 8; ch++)
        SimpleHdma_Init(&hdma[ch], &g_dma->channel[ch]);

    int trigger = g_snes->vIrqEnabled ? (int)g_snes->vTimer + 1 : -1;

    for (int line = 0; line <= 224; line++) {
        memcpy(s_line_regs[line], &g_ppu->inidisp,
               PPU_SAVESTATE_REGS_SIZE);
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
    s_line_regs_valid = true;
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
