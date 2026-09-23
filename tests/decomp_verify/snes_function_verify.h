#ifndef SNES_FUNCTION_VERIFY_H
#define SNES_FUNCTION_VERIFY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "interp816.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    SNES_VERIFY_WRAM_SIZE = 0x20000,
    SNES_VERIFY_MAX_BUS_EVENTS = 512,
};

typedef struct SnesVerifyBusEvent {
    uint32_t address;
    uint8_t value;
    uint8_t write;
} SnesVerifyBusEvent;

typedef struct SnesVerifyBus {
    const uint8_t *rom;
    size_t rom_size;
    uint8_t *wram;
    SnesVerifyBusEvent events[SNES_VERIFY_MAX_BUS_EVENTS];
    size_t event_count;
    bool event_overflow;
    uint8_t multiply_a;
    uint8_t multiply_b;
    uint16_t multiply_result;
    /* PPU Mode 7 multiplier. */
    uint8_t m7_latch;
    uint16_t m7_a;
    uint8_t m7_b;
} SnesVerifyBus;

bool SnesVerifyLoadFile(
    const char *path, uint8_t **data_out, size_t *size_out);
bool SnesVerifyBusInit(
    SnesVerifyBus *bus, const uint8_t *rom, size_t rom_size);
void SnesVerifyBusDestroy(SnesVerifyBus *bus);
void SnesVerifyBusResetTrace(SnesVerifyBus *bus);

uint8_t SnesVerifyBusRead(void *opaque, uint32_t address);
void SnesVerifyBusWrite(void *opaque, uint32_t address, uint8_t value);
bool SnesVerifyBusPoke(
    SnesVerifyBus *bus, uint32_t address, uint8_t value);

uint32_t SnesVerifyPc24(const Interp816 *cpu);

int SnesVerifyRunUntil(
    Interp816 *cpu,
    const uint32_t *stop_pc24,
    size_t stop_count,
    unsigned instruction_limit,
    unsigned *instructions_run);

#ifdef __cplusplus
}
#endif

#endif
