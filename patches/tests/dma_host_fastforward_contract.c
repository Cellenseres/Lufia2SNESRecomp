#include "patches/dma_host_fastforward.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    L2_DMA_TEST_RAM = 0x20000,
    L2_DMA_TEST_ROM = 0x280000
};

static void FillRam(uint8_t *ram, unsigned seed) {
    for (unsigned i = 0; i < L2_DMA_TEST_RAM; ++i)
        ram[i] = (uint8_t)((i * 37u + seed * 73u + (i >> 8)) & 0xffu);
}

static void Configure(Dma *dma, Snes *snes, uint8_t *ram, unsigned mask,
                      unsigned size, unsigned variant, unsigned timer) {
    memset(snes, 0, sizeof(*snes));
    memset(dma, 0, sizeof(*dma));
    snes->ram = ram;
    snes->dma = dma;
    snes->ramAdr = 0x12000u + variant * 0x200u;
    dma->snes = snes;
    dma->dmaTimer = timer;

    for (unsigned channel = 0; channel < 8; ++channel) {
        DmaChannel *c = &dma->channel[channel];
        c->bAdr = 0x80u; /* WRAM data port: no global PPU/APU dependency. */
        c->aBank = (uint8_t)(0x7eu + ((channel + variant) & 1u));
        c->aAdr = (uint16_t)(0x4000u + channel * 0x400u +
                             (variant == 1u ? size + 4u : 0u));
        c->size = (uint16_t)size;
        c->fixed = variant == 2u;
        c->decrement = variant == 1u;
        c->mode = 0;
    }
    dma_startDma(dma, (uint8_t)mask, false);
}

static int SameDma(const Dma *a, const Dma *b) {
    return a->dmaTimer == b->dmaTimer && a->dmaBusy == b->dmaBusy &&
           memcmp(a->channel, b->channel, sizeof(a->channel)) == 0;
}

static int TestDirectSourceReads(unsigned *checked) {
    uint8_t *ram = malloc(L2_DMA_TEST_RAM);
    uint8_t *rom = malloc(L2_DMA_TEST_ROM);
    uint8_t *sram = malloc(0x8000u);
    static const uint16_t boundaries[] = {
        0x0000u, 0x0001u, 0x1ffeu, 0x1fffu, 0x2000u,
        0x20ffu, 0x2100u, 0x21ffu, 0x2200u, 0x3fffu,
        0x4000u, 0x5fffu, 0x6000u, 0x6fffu, 0x7000u,
        0x7fffu, 0x8000u, 0x8001u, 0xbfffu, 0xfffeu, 0xffffu
    };
    Snes snes;
    Cart cart;
    unsigned cases = 0;
    int result = 1;

    if (!ram || !rom || !sram) {
        fprintf(stderr, "[dma-host-ff-test] direct test allocation failed\n");
        goto cleanup;
    }

    memset(&snes, 0, sizeof(snes));
    memset(&cart, 0, sizeof(cart));
    FillRam(ram, 0x51u);
    for (unsigned i = 0; i < L2_DMA_TEST_ROM; ++i)
        rom[i] = (uint8_t)((i * 29u + (i >> 13) + 0xa7u) & 0xffu);
    snes.ram = ram;
    snes.cart = &cart;
    cart.snes = &snes;
    cart.type = CART_LOROM;
    cart.rom = rom;
    cart.romSize = L2_DMA_TEST_ROM;
    cart.ram = sram;
    cart.ramSize = 0x8000u;

    for (unsigned bank = 0; bank < 0x100u; ++bank) {
        for (unsigned ai = 0;
             ai < sizeof(boundaries) / sizeof(boundaries[0]); ++ai) {
            uint8_t direct = 0;
            const uint16_t address = boundaries[ai];
            if (L2DmaHostDirectSourceRead(&snes, (uint8_t)bank, address,
                                          &direct)) {
                const uint8_t reference = snes_read(
                    &snes, ((uint32_t)bank << 16) | address);
                if (direct != reference) {
                    fprintf(stderr,
                        "[dma-host-ff-test] direct mismatch src=%02X:%04X "
                        "value=%02X/%02X\n",
                        bank, address, direct, reference);
                    goto cleanup;
                }
                ++cases;
            }
        }
        for (unsigned sample = 0; sample < 0x100u; ++sample) {
            uint8_t direct = 0;
            const uint16_t address = (uint16_t)(
                sample * 257u + bank * 131u + 0x5a3du);
            if (L2DmaHostDirectSourceRead(&snes, (uint8_t)bank, address,
                                          &direct)) {
                const uint8_t reference = snes_read(
                    &snes, ((uint32_t)bank << 16) | address);
                if (direct != reference) {
                    fprintf(stderr,
                        "[dma-host-ff-test] direct mismatch src=%02X:%04X "
                        "value=%02X/%02X\n",
                        bank, address, direct, reference);
                    goto cleanup;
                }
                ++cases;
            }
        }
    }

    {
        uint8_t ignored = 0;
        if (L2DmaHostDirectSourceRead(&snes, 0x00u, 0x2100u, &ignored) ||
            L2DmaHostDirectSourceRead(&snes, 0x70u, 0x1000u, &ignored) ||
            L2DmaHostDirectSourceRead(&snes, 0xf0u, 0x7fffu, &ignored) ||
            L2DmaHostDirectSourceRead(&snes, 0x00u, 0x4000u, &ignored) ||
            L2DmaHostDirectSourceRead(NULL, 0x7eu, 0x0000u, &ignored) ||
            L2DmaHostDirectSourceRead(&snes, 0x7eu, 0x0000u, NULL)) {
            fprintf(stderr,
                "[dma-host-ff-test] direct guard accepted side-effectful "
                "or invalid source\n");
            goto cleanup;
        }
        cart.type = CART_CX4;
        if (L2DmaHostDirectSourceRead(&snes, 0x80u, 0x8000u,
                                      &ignored)) {
            fprintf(stderr,
                "[dma-host-ff-test] direct guard accepted special cart\n");
            goto cleanup;
        }
        cart.type = CART_LOROM;
    }

    *checked = cases;
    result = 0;

cleanup:
    free(sram);
    free(rom);
    free(ram);
    return result;
}

int Lufia2DmaHostFastForwardSelfTest(void) {
    static uint8_t reference_ram[L2_DMA_TEST_RAM];
    static uint8_t native_ram[L2_DMA_TEST_RAM];
    static const unsigned masks[] = { 0x01u, 0x03u, 0x81u };
    static const unsigned sizes[] = { 1u, 2u, 3u, 16u, 257u };
    static const unsigned timers[] = { 0u, 2u, 10u };
    unsigned cases = 0;
    unsigned direct_reads = 0;

    fprintf(stderr, "[dma-host-ff-test] begin\n");
    if (TestDirectSourceReads(&direct_reads) != 0)
        return 1;
    for (unsigned mi = 0; mi < sizeof(masks) / sizeof(masks[0]); ++mi) {
        for (unsigned si = 0; si < sizeof(sizes) / sizeof(sizes[0]); ++si) {
            for (unsigned ti = 0; ti < sizeof(timers) / sizeof(timers[0]); ++ti) {
                for (unsigned variant = 0; variant < 3u; ++variant) {
                    Snes reference_snes, native_snes;
                    Dma reference_dma, native_dma;
                    uint64_t expected_idle_ticks = 0;
                    uint64_t actual_idle_ticks = 0;

                    FillRam(reference_ram, cases + 1u);
                    memcpy(native_ram, reference_ram, sizeof(reference_ram));
                    Configure(&reference_dma, &reference_snes, reference_ram,
                              masks[mi], sizes[si], variant, timers[ti]);
                    Configure(&native_dma, &native_snes, native_ram,
                              masks[mi], sizes[si], variant, timers[ti]);

                    while (reference_dma.dmaBusy) {
                        if (reference_dma.dmaTimer > 0u &&
                            !(reference_dma.dmaTimer & 1u))
                            ++expected_idle_ticks;
                        dma_cycle(&reference_dma);
                    }
                    while (L2DmaHostFastForwardCycle(&native_dma,
                                                     &actual_idle_ticks)) {}

                    ++cases;
                    if (!SameDma(&reference_dma, &native_dma) ||
                        reference_snes.ramAdr != native_snes.ramAdr ||
                        memcmp(reference_ram, native_ram,
                               sizeof(reference_ram)) != 0 ||
                        expected_idle_ticks != actual_idle_ticks) {
                        fprintf(stderr,
                            "[dma-host-ff-test] mismatch case=%u mask=%02X "
                            "size=%u variant=%u timer=%u idle=%llu/%llu\n",
                            cases, masks[mi], sizes[si], variant, timers[ti],
                            (unsigned long long)expected_idle_ticks,
                            (unsigned long long)actual_idle_ticks);
                        return 1;
                    }
                }
            }
        }
    }

    fprintf(stderr,
        "[dma-host-ff-test] PASS cases=%u state=exact transfers=exact "
        "timer=exact bbus=wram-port direct_reads=%u mapping=exact\n",
        cases, direct_reads);
    return 0;
}
