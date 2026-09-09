#ifndef LUFIA2_DMA_HOST_FASTFORWARD_H
#define LUFIA2_DMA_HOST_FASTFORWARD_H

#include <stdint.h>

#include "dma.h"
#include "cart.h"

extern uint64_t lufia2_dma_host_idle_ticks_skipped;

/* $420B runs DMA synchronously in this core.  Between transfer bytes,
 * dma_doDma() only subtracts two from dmaTimer and returns; the surrounding
 * loop advances no PPU, APU, beam, CPU or interrupt state. Collapse those
 * host-only calls, then execute the unchanged transfer function once.
 *
 * Odd timers are outside the reviewed invariant, so retain the upstream
 * dma_cycle() behavior instead of trying to reinterpret them. */
static inline bool L2DmaHostFastForwardCycle(Dma *dma,
                                              uint64_t *idle_ticks) {
    if (!dma || !dma->dmaBusy)
        return false;
    if (dma->dmaTimer & 1u)
        return dma_cycle(dma);

    if (idle_ticks)
        *idle_ticks += dma->dmaTimer / 2u;
    dma->dmaTimer = 0;
    dma_doDma(dma);

    /* Match dma_cycle(): a call that entered while busy returns true even if
     * dma_doDma() just cleared dmaBusy. */
    return true;
}

/* A-to-B DMA source reads are side-effect-free only for WRAM (including its
 * low mirror) and mapped ROM on a plain LoROM cartridge. Mirror the pinned
 * core's address mapping exactly and reject every other address, so snes_read
 * retains MMIO, SRAM, open-bus and special-chip behavior. */
static inline bool L2DmaHostDirectSourceRead(Snes *snes, uint8_t bank,
                                             uint16_t address,
                                             uint8_t *value) {
    if (!snes || !snes->ram || !value)
        return false;

    if (bank == 0x7eu || bank == 0x7fu) {
        *value = snes->ram[((uint32_t)(bank & 1u) << 16) | address];
        return true;
    }
    if ((bank < 0x40u || (bank >= 0x80u && bank < 0xc0u)) &&
        address < 0x2000u) {
        *value = snes->ram[address];
        return true;
    }

    Cart *cart = snes->cart;
    if (!cart || cart->type != CART_LOROM || !cart->rom ||
        cart->romSize == 0u)
        return false;
    if (((bank >= 0x70u && bank < 0x7eu) || bank >= 0xf0u) &&
        address < 0x8000u && cart->ramSize > 0u)
        return false;

    const uint8_t canonical = bank & 0x7fu;
    if (address < 0x8000u && canonical < 0x40u)
        return false;

    const uint32_t offset = ((uint32_t)canonical << 15) |
                            (address & 0x7fffu);
    *value = cart->rom[offset % cart->romSize];
    return true;
}

uint64_t Lufia2DmaHostFastForwardIdleTicks(void);
int Lufia2DmaHostFastForwardSelfTest(void);

#endif
