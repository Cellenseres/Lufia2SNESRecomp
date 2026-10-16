#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "snes_function_verify.h"
#include "decomp_bridge/field_resume.h"

int g_interp_apu_driving;
static unsigned checks;
static void check(Interp816 *cpu, uint32_t site, uint32_t expected) {
    const Interp816 before = *cpu;
    if (Lufia2FieldResumeReturn(cpu, site) != expected ||
        memcmp(cpu, &before, sizeof before)) {
        fprintf(stderr, "field gate %u site%06X PC%02X%04X S%04X DP%04X DB%02X\n",
                checks, site, cpu->k, cpu->pc, cpu->sp, cpu->dp, cpu->db);
        exit(1);
    }
    ++checks;
}
static Interp816 make_cpu(void) {
    Interp816 cpu = {0};
    cpu.k = cpu.db = 0x83u;
    cpu.pc = 0x8007u;
    cpu.sp = 0x1fffu;
    cpu.mf = true;
    return cpu;
}
static Interp816 flagged_cpu(unsigned flags, unsigned width) {
    Interp816 cpu = make_cpu();
    cpu.xf = width != 0u;
    cpu.c = (flags & 1u) != 0u;
    cpu.z = (flags & 2u) != 0u;
    cpu.n = (flags & 4u) != 0u;
    cpu.v = (flags & 8u) != 0u;
    cpu.i = (flags & 16u) != 0u;
    return cpu;
}
int main(int argc, char **argv) {
    for (unsigned width = 0; width < 2u; ++width) {
        for (unsigned value = 0; value < 65536u; ++value) {
            Interp816 cpu = make_cpu();
            cpu.xf = width != 0u;
            cpu.pc = (uint16_t)value;
            check(&cpu, 0x839012u, value == 0x8007u ? 0x838007u : 0u);
            cpu = make_cpu();
            cpu.xf = width != 0u;
            cpu.sp = (uint16_t)value;
            check(&cpu, 0x839012u, value == 0x1fffu ? 0x838007u : 0u);
            cpu = make_cpu();
            cpu.xf = width != 0u;
            cpu.dp = (uint16_t)value;
            check(&cpu, 0x839012u, value == 0u ? 0x838007u : 0u);
            cpu = make_cpu();
            cpu.xf = width != 0u;
            cpu.k = (uint8_t)(value >> 8);
            cpu.db = (uint8_t)value;
            check(&cpu, 0x839012u, value == 0x8383u ? 0x838007u : 0u);
            cpu = make_cpu();
            cpu.xf = width != 0u;
            check(&cpu, 0x830000u | value, value == 0x9012u ? 0x838007u : 0u);
        }
        for (unsigned bank = 0; bank < 256u; ++bank) {
            Interp816 cpu = make_cpu();
            cpu.xf = width != 0u;
            check(&cpu, (bank << 16) | 0x9012u, bank == 0x83u ? 0x838007u : 0u);
        }
        for (unsigned flags = 0; flags < 32u; ++flags) {
            Interp816 cpu = flagged_cpu(flags, width);
            cpu.mf = false;
            check(&cpu, 0x839012u, 0u);
            cpu = flagged_cpu(flags, width);
            cpu.d = true;
            check(&cpu, 0x839012u, 0u);
            cpu = flagged_cpu(flags, width);
            cpu.mf = cpu.xf = cpu.e = true;
            cpu.sp = 0x1ffu;
            check(&cpu, 0x839012u, 0u);
        }
    }
    uint8_t *rom;
    size_t size;
    SnesVerifyBus bus;
    if (argc != 2 || !SnesVerifyLoadFile(argv[1], &rom, &size) ||
        !SnesVerifyBusInit(&bus, rom, size))
        return 2;
    Interp816 *cpu = interp816_init(&bus, SnesVerifyBusRead, SnesVerifyBusWrite);
    for (unsigned status = 0; status < 256u; ++status) {
        cpu->k = cpu->db = 0x83u;
        cpu->pc = 0x9012u;
        cpu->sp = 0x1ffdu;
        cpu->dp = 0u;
        cpu->e = false;
        cpu->a = cpu->x = cpu->y = 0u;
        cpu->irqWanted = cpu->nmiWanted = cpu->waiting = cpu->stopped = false;
        interp816_setFlags(cpu, (uint8_t)status);
        bus.wram[0x1ffeu] = 6u;
        bus.wram[0x1fffu] = 0x80u;
        interp816_runOpcode(cpu);
        if (cpu->pc != 0x8007u || cpu->sp != 0x1fffu)
            return 1;
        check(cpu, 0x839012u, (status & 0x28u) == 0x20u ? 0x838007u : 0u);
    }
    printf(
        "PASS field gate%u readonly states;all PC/stack/DP/bank pairs/return sites;256 original RTS states\n",
        checks);
    interp816_free(cpu);
    SnesVerifyBusDestroy(&bus);
    free(rom);
    return 0;
}
