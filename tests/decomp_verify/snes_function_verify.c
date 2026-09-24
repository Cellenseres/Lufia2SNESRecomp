#include "snes_function_verify.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int32_t WramOffset(uint32_t address) {
    const uint8_t bank = (uint8_t)(address >> 16);
    const uint16_t offset = (uint16_t)address;

    if (bank == 0x7e)
        return (int32_t)offset;
    if (bank == 0x7f)
        return 0x10000 + (int32_t)offset;
    if (offset < 0x2000 &&
        (bank <= 0x3f || (bank >= 0x80 && bank <= 0xbf)))
        return (int32_t)offset;
    return -1;
}

/* CPU registers mirror into every system bank. */
static bool IsCpuRegister(uint32_t address, uint16_t reg) {
    const uint8_t bank = (uint8_t)(address >> 16);

    return (uint16_t)address == reg &&
           (bank <= 0x3f || (bank >= 0x80 && bank <= 0xbf));
}

static bool LoRomOffset(
    const SnesVerifyBus *bus, uint32_t address, size_t *offset_out) {
    const uint8_t bank = (uint8_t)(address >> 16);
    const uint16_t offset = (uint16_t)address;
    size_t rom_offset;

    if (bank == 0x7e || bank == 0x7f || offset < 0x8000)
        return false;

    rom_offset =
        ((size_t)(bank & 0x7fu) << 15) | (size_t)(offset & 0x7fffu);
    if (rom_offset >= bus->rom_size)
        return false;

    *offset_out = rom_offset;
    return true;
}

static void Trace(
    SnesVerifyBus *bus, uint32_t address, uint8_t value, bool write) {
    if (bus->event_count < SNES_VERIFY_MAX_BUS_EVENTS) {
        SnesVerifyBusEvent *event = &bus->events[bus->event_count++];
        event->address = address & 0xffffffu;
        event->value = value;
        event->write = write ? 1u : 0u;
    } else {
        bus->event_overflow = true;
    }
}

bool SnesVerifyLoadFile(
    const char *path, uint8_t **data_out, size_t *size_out) {
    FILE *file;
    long length;
    uint8_t *data;

    if (!path || !data_out || !size_out)
        return false;

    file = fopen(path, "rb");
    if (!file)
        return false;
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return false;
    }
    length = ftell(file);
    if (length <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return false;
    }

    data = (uint8_t *)malloc((size_t)length);
    if (!data) {
        fclose(file);
        return false;
    }
    if (fread(data, 1, (size_t)length, file) != (size_t)length) {
        free(data);
        fclose(file);
        return false;
    }
    fclose(file);

    *data_out = data;
    *size_out = (size_t)length;
    return true;
}

bool SnesVerifyBusInit(
    SnesVerifyBus *bus, const uint8_t *rom, size_t rom_size) {
    if (!bus || !rom || !rom_size)
        return false;

    memset(bus, 0, sizeof(*bus));
    bus->wram = (uint8_t *)calloc(1, SNES_VERIFY_WRAM_SIZE);
    if (!bus->wram)
        return false;

    bus->rom = rom;
    bus->rom_size = rom_size;
    return true;
}

void SnesVerifyBusDestroy(SnesVerifyBus *bus) {
    if (!bus)
        return;
    free(bus->wram);
    memset(bus, 0, sizeof(*bus));
}

void SnesVerifyBusResetTrace(SnesVerifyBus *bus) {
    if (!bus)
        return;
    bus->event_count = 0;
    bus->event_overflow = false;
}

/* Signed 16 x signed 8, 24-bit result. */
static uint32_t Mode7Product(const SnesVerifyBus *bus) {
    const int32_t product =
        (int32_t)(int16_t)bus->m7_a * (int32_t)(int8_t)bus->m7_b;
    return (uint32_t)product & 0x00ffffffu;
}

void SnesVerifyBusResetMmio(SnesVerifyBus *bus) {
    if (!bus)
        return;
    bus->mmio_count = 0;
    bus->mmio_overflow = false;
}

uint8_t SnesVerifyBusRead(void *opaque, uint32_t address) {
    SnesVerifyBus *bus = (SnesVerifyBus *)opaque;
    const int32_t wram_offset = WramOffset(address);
    size_t rom_offset;
    uint8_t value = 0xffu;

    if (IsCpuRegister(address, 0x4214u))
        value = (uint8_t)bus->divide_result;
    else if (IsCpuRegister(address, 0x4215u))
        value = (uint8_t)(bus->divide_result >> 8);
    else if (IsCpuRegister(address, 0x4216u))
        value = (uint8_t)bus->multiply_result;
    else if (IsCpuRegister(address, 0x2134u) ||
             IsCpuRegister(address, 0x2135u) ||
             IsCpuRegister(address, 0x2136u))
        value = (uint8_t)(
            Mode7Product(bus) >> (8u * ((uint16_t)address - 0x2134u)));
    else if (IsCpuRegister(address, 0x4217u))
        value = (uint8_t)(bus->multiply_result >> 8);
    else if (wram_offset >= 0)
        value = bus->wram[wram_offset];
    else if (LoRomOffset(bus, address, &rom_offset))
        value = bus->rom[rom_offset];

    Trace(bus, address, value, false);
    return value;
}

void SnesVerifyBusWrite(void *opaque, uint32_t address, uint8_t value) {
    SnesVerifyBus *bus = (SnesVerifyBus *)opaque;
    const int32_t wram_offset = WramOffset(address);

    if (IsCpuRegister(address, 0x4202u)) {
        bus->multiply_a = value;
    } else if (IsCpuRegister(address, 0x4203u)) {
        bus->multiply_b = value;
        bus->multiply_result =
            (uint16_t)((uint16_t)bus->multiply_a * value);
    } else if (IsCpuRegister(address, 0x4204u)) {
        bus->divide_a = (uint16_t)((bus->divide_a & 0xff00u) | value);
    } else if (IsCpuRegister(address, 0x4205u)) {
        bus->divide_a = (uint16_t)((bus->divide_a & 0x00ffu) | (value << 8));
    } else if (IsCpuRegister(address, 0x4206u)) {
        /* Same as the runtime core: instant. */
        if (value == 0) {
            bus->divide_result = 0xffffu;
            bus->multiply_result = bus->divide_a;
        } else {
            bus->divide_result = (uint16_t)(bus->divide_a / value);
            bus->multiply_result = (uint16_t)(bus->divide_a % value);
        }
    } else if (IsCpuRegister(address, 0x211bu)) {
        bus->m7_a = (uint16_t)((value << 8) | bus->m7_latch);
        bus->m7_latch = value;
    } else if (IsCpuRegister(address, 0x211cu)) {
        bus->m7_b = value;
        bus->m7_latch = value;
    } else if (wram_offset >= 0) {
        bus->wram[wram_offset] = value;
    }
    if (wram_offset < 0) {
        if (bus->mmio_count < SNES_VERIFY_MAX_MMIO) {
            SnesVerifyBusEvent *event = &bus->mmio[bus->mmio_count++];
            event->address = address & 0xffffffu;
            event->value = value;
            event->write = 1u;
        } else {
            bus->mmio_overflow = true;
        }
    }
    Trace(bus, address, value, true);
}

bool SnesVerifyBusPoke(
    SnesVerifyBus *bus, uint32_t address, uint8_t value) {
    const int32_t wram_offset = WramOffset(address);
    if (!bus || wram_offset < 0)
        return false;
    bus->wram[wram_offset] = value;
    return true;
}

uint32_t SnesVerifyPc24(const Interp816 *cpu) {
    return ((uint32_t)cpu->k << 16) | cpu->pc;
}

int SnesVerifyRunUntil(
    Interp816 *cpu,
    const uint32_t *stop_pc24,
    size_t stop_count,
    unsigned instruction_limit,
    unsigned *instructions_run) {
    unsigned executed = 0;

    while (executed < instruction_limit) {
        const uint32_t pc24 = SnesVerifyPc24(cpu);
        for (size_t i = 0; i < stop_count; ++i) {
            if (pc24 == (stop_pc24[i] & 0xffffffu)) {
                if (instructions_run)
                    *instructions_run = executed;
                return (int)i;
            }
        }
        interp816_runOpcode(cpu);
        ++executed;
    }

    if (instructions_run)
        *instructions_run = executed;
    return -1;
}
