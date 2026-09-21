#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu_state.h"
#include "interp816.h"
#include "snes_function_verify.h"

extern RecompReturn Lufia2DecompBridge_BBF3(CpuState *cpu);

enum {
    LUFIA2_ROM_SIZE = 0x280000,
    BBF3_PC = 0x83bbf3,
    BBF3_CHILD_SPECIAL = 0x83bc28,
    BBF3_CHILD_STANDARD = 0x83c1b4,
    BBF3_SPECIAL_SITE = 0x83bc1d,
    BBF3_STANDARD_SITE = 0x83bc22,
    BBF3_RETURN_SENTINEL = 0x837fff,
    BBF3_STACK = 0x01f0,
    BBF3_GATE_COUNT = 6,
    BRIDGE_RANDOM_CASES = 4096,
};

static const uint16_t kGateAddresses[BBF3_GATE_COUNT] = {
    0x09a8, 0x05b5, 0x05b7, 0x0622, 0x099b, 0x09a7,
};

typedef struct VerifyLog {
    FILE *file;
} VerifyLog;

typedef struct BridgeCapture {
    bool called;
    uint32_t target;
    uint32_t site;
    CpuState child_entry;
} BridgeCapture;

static uint8_t g_ram[SNES_VERIFY_WRAM_SIZE];
static BridgeCapture g_capture;
static uint64_t s_rng = UINT64_C(0xd1b54a32d192ed03);

static void Logf(VerifyLog *log, const char *format, ...) {
    va_list args;
    va_list copy;

    va_start(args, format);
    va_copy(copy, args);
    vprintf(format, args);
    if (log->file)
        vfprintf(log->file, format, copy);
    va_end(copy);
    va_end(args);
}

static uint32_t Random32(void) {
    s_rng ^= s_rng << 13;
    s_rng ^= s_rng >> 7;
    s_rng ^= s_rng << 17;
    return (uint32_t)(s_rng >> 32);
}

static uint8_t ReadWram(uint8_t bank, uint16_t address) {
    const int32_t offset = cpu_wram_offset(bank, address);
    return offset >= 0 ? g_ram[offset] : 0xffu;
}

uint8 cpu_read8(CpuState *cpu, uint8 bank, uint16 address) {
    const uint8 value = ReadWram(bank, address);
    cpu->open_bus = value;
    return value;
}

void cpu_write8(CpuState *cpu, uint8 bank, uint16 address, uint8 value) {
    const int32_t offset = cpu_wram_offset(bank, address);
    if (offset >= 0)
        g_ram[offset] = value;
    cpu->open_bus = value;
}

RecompReturn cpu_dispatch_call_pc(
    CpuState *cpu, uint32 target, uint32 source_pc24) {
    const uint16_t return_minus_one =
        (uint16_t)((source_pc24 + 2u) & 0xffffu);

    cpu_write8(
        cpu, 0x00, cpu->S, (uint8_t)(return_minus_one >> 8));
    cpu->S = (uint16_t)(cpu->S - 1);
    cpu_write8(
        cpu, 0x00, cpu->S, (uint8_t)return_minus_one);
    cpu->S = (uint16_t)(cpu->S - 1);
    cpu->host_return_valid = 2;

    memset(&g_capture, 0, sizeof(g_capture));
    g_capture.called = true;
    g_capture.target = target & 0xffffffu;
    g_capture.site = source_pc24 & 0xffffffu;
    g_capture.child_entry = *cpu;

    /* Stop at the child boundary. The bridge must propagate one level. */
    return RECOMP_RETURN_SKIP_2;
}

RecompReturn interp_tier_dispatch_rewritten_return(
    CpuState *cpu, uint32 target_pc24, uint32 site_pc24) {
    (void)cpu;
    (void)target_pc24;
    (void)site_pc24;
    return RECOMP_RETURN_LLE_UNWIND_BASE;
}

RecompReturn cpu_dispatch_pc_from(
    CpuState *cpu, uint32 pc24, uint16 miss_restore_s, uint32 source_pc24) {
    (void)cpu;
    (void)pc24;
    (void)miss_restore_s;
    (void)source_pc24;
    return RECOMP_RETURN_LLE_UNWIND_BASE;
}

int interp816_opcode_hook(uint32_t address) {
    (void)address;
    return 0;
}

static void SeedStack(uint8_t *ram) {
    ram[BBF3_STACK + 1] = 0xfeu;
    ram[BBF3_STACK + 2] = 0x7fu;
}

static void SeedGates(uint8_t *ram, const uint8_t gate[BBF3_GATE_COUNT]) {
    for (int i = 0; i < BBF3_GATE_COUNT; ++i)
        ram[kGateAddresses[i]] = gate[i];
}

static void InitCpuState(CpuState *cpu, const uint8_t gate[BBF3_GATE_COUNT]) {
    memset(cpu, 0, sizeof(*cpu));
    memset(g_ram, 0, sizeof(g_ram));
    SeedGates(g_ram, gate);
    SeedStack(g_ram);

    cpu->A = (uint16_t)Random32();
    cpu->X = (uint8_t)Random32();
    cpu->Y = (uint8_t)Random32();
    cpu->S = BBF3_STACK;
    cpu->D = (uint16_t)Random32();
    cpu->DB = 0x00;
    cpu->PB = 0x83;
    cpu->host_return_valid = 2;
    cpu->m_flag = 1;
    cpu->x_flag = 1;
    cpu->emulation = 0;
    cpu->_flag_C = Random32() & 1u;
    cpu->_flag_Z = Random32() & 1u;
    cpu->_flag_V = Random32() & 1u;
    cpu->_flag_N = Random32() & 1u;
    cpu->_flag_I = Random32() & 1u;
    cpu->_flag_D = Random32() & 1u;
    cpu->ram = g_ram;
    cpu_mirrors_to_p(cpu);
}

static void InitInterp(
    Interp816 *cpu,
    SnesVerifyBus *bus,
    const CpuState *input,
    const uint8_t gate[BBF3_GATE_COUNT]) {
    memset(bus->wram, 0, SNES_VERIFY_WRAM_SIZE);
    SeedGates(bus->wram, gate);
    SeedStack(bus->wram);

    cpu->a = input->A;
    cpu->x = input->X;
    cpu->y = input->Y;
    cpu->sp = input->S;
    cpu->pc = (uint16_t)BBF3_PC;
    cpu->dp = input->D;
    cpu->k = input->PB;
    cpu->db = input->DB;
    cpu->c = input->_flag_C != 0;
    cpu->z = input->_flag_Z != 0;
    cpu->v = input->_flag_V != 0;
    cpu->n = input->_flag_N != 0;
    cpu->i = input->_flag_I != 0;
    cpu->d = input->_flag_D != 0;
    cpu->mf = true;
    cpu->xf = true;
    cpu->e = false;
    cpu->irqWanted = false;
    cpu->nmiWanted = false;
    cpu->waiting = false;
    cpu->stopped = false;
    interp816_set_brk_hook_enabled(cpu, false);
}

static bool SameArchitecturalState(
    const CpuState *native, const Interp816 *reference) {
    return native->A == reference->a &&
           native->X == reference->x &&
           native->Y == reference->y &&
           native->S == reference->sp &&
           native->D == reference->dp &&
           native->DB == reference->db &&
           native->PB == reference->k &&
           native->m_flag == (uint8_t)reference->mf &&
           native->x_flag == (uint8_t)reference->xf &&
           native->_flag_C == (uint8_t)reference->c &&
           native->_flag_Z == (uint8_t)reference->z &&
           native->_flag_V == (uint8_t)reference->v &&
           native->_flag_N == (uint8_t)reference->n &&
           native->_flag_I == (uint8_t)reference->i &&
           native->_flag_D == (uint8_t)reference->d;
}

static bool RunNoChildCase(
    VerifyLog *log,
    SnesVerifyBus *bus,
    Interp816 *reference,
    const uint8_t gate[BBF3_GATE_COUNT],
    unsigned index) {
    CpuState native;
    uint32_t stops[] = {BBF3_RETURN_SENTINEL};
    unsigned instructions = 0;

    InitCpuState(&native, gate);
    InitInterp(reference, bus, &native, gate);
    memset(&g_capture, 0, sizeof(g_capture));

    const RecompReturn native_return = Lufia2DecompBridge_BBF3(&native);
    const int stop = SnesVerifyRunUntil(
        reference, stops, 1, 128, &instructions);

    if (native_return != RECOMP_RETURN_NORMAL || g_capture.called ||
        stop != 0 || !SameArchitecturalState(&native, reference)) {
        Logf(log,
            "FAIL no-child bridge case %u: ret=%d child=%d stop=%d "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X "
            "S=%04X/%04X flags N%d/%d Z%d/%d C%d/%d V%d/%d "
            "I%d/%d D%d/%d M%d/%d X%d/%d\n",
            index, (int)native_return, g_capture.called ? 1 : 0, stop,
            native.A, reference->a, native.X, reference->x,
            native.Y, reference->y, native.S, reference->sp,
            native._flag_N, reference->n, native._flag_Z, reference->z,
            native._flag_C, reference->c, native._flag_V, reference->v,
            native._flag_I, reference->i, native._flag_D, reference->d,
            native.m_flag, reference->mf, native.x_flag, reference->xf);
        return false;
    }
    return true;
}

static bool RunChildBoundaryCase(
    VerifyLog *log,
    SnesVerifyBus *bus,
    Interp816 *reference,
    const uint8_t gate[BBF3_GATE_COUNT],
    uint32_t expected_target,
    uint32_t expected_site,
    unsigned index) {
    CpuState native;
    uint32_t stops[] = {expected_target};
    unsigned instructions = 0;

    InitCpuState(&native, gate);
    InitInterp(reference, bus, &native, gate);
    memset(&g_capture, 0, sizeof(g_capture));

    const RecompReturn native_return = Lufia2DecompBridge_BBF3(&native);
    const int stop = SnesVerifyRunUntil(
        reference, stops, 1, 128, &instructions);

    if (native_return != RECOMP_RETURN_SKIP_1 ||
        !g_capture.called ||
        g_capture.target != expected_target ||
        g_capture.site != expected_site ||
        stop != 0 ||
        !SameArchitecturalState(&g_capture.child_entry, reference)) {
        Logf(log,
            "FAIL child bridge case %u: ret=%d called=%d "
            "target=%06X/%06X site=%06X/%06X stop=%d "
            "S=%04X/%04X Xflag=%u/%u\n",
            index, (int)native_return, g_capture.called ? 1 : 0,
            g_capture.target, expected_target,
            g_capture.site, expected_site, stop,
            g_capture.child_entry.S, reference->sp,
            g_capture.child_entry.x_flag, reference->xf);
        return false;
    }
    return true;
}

int main(int argc, char **argv) {
    const char *rom_path;
    const char *report_path = NULL;
    uint8_t *rom = NULL;
    size_t rom_size = 0;
    SnesVerifyBus bus;
    Interp816 *reference = NULL;
    VerifyLog log = {0};
    unsigned passed = 0;
    unsigned failed = 0;
    bool bus_initialized = false;

    if (argc < 2) {
        fprintf(stderr,
            "usage: %s <lufia2.sfc> [--report path]\n", argv[0]);
        return 2;
    }
    rom_path = argv[1];
    for (int i = 2; i < argc; ++i) {
        if (!strcmp(argv[i], "--report") && i + 1 < argc)
            report_path = argv[++i];
        else {
            fprintf(stderr, "unknown argument: %s\n", argv[i]);
            return 2;
        }
    }

    if (report_path) {
        log.file = fopen(report_path, "wb");
        if (!log.file) {
            fprintf(stderr, "cannot open report: %s\n", report_path);
            return 2;
        }
    }

    Logf(&log, "Lufia II decomp bridge verifier\n");
    Logf(&log, "================================\n\n");
    Logf(&log, "$83:BBF3 bridge vs original 65816 call boundary\n");

    if (!SnesVerifyLoadFile(rom_path, &rom, &rom_size) ||
        rom_size != LUFIA2_ROM_SIZE ||
        !SnesVerifyBusInit(&bus, rom, rom_size)) {
        Logf(&log, "ERROR: reference ROM/bus initialization failed\n");
        failed = 1;
        goto done;
    }
    bus_initialized = true;
    reference = interp816_init(&bus, SnesVerifyBusRead, SnesVerifyBusWrite);
    if (!reference) {
        Logf(&log, "ERROR: interp816 initialization failed\n");
        failed = 1;
        goto done;
    }

    for (unsigned i = 0; i < BRIDGE_RANDOM_CASES; ++i) {
        uint8_t no_child[BBF3_GATE_COUNT] = {0};
        no_child[i % 5] =
            (i % 5 == 0) ? 0x08u :
            (i % 5 == 1) ? 0x02u :
            (i % 5 == 2) ? 0x01u :
            0x80u;
        if (RunNoChildCase(
                &log, &bus, reference, no_child, i))
            ++passed;
        else if (++failed >= 20)
            break;

        {
            uint8_t special[BBF3_GATE_COUNT] = {0};
            special[5] = (uint8_t)(Random32() | 1u);
            if (RunChildBoundaryCase(
                    &log, &bus, reference, special,
                    BBF3_CHILD_SPECIAL, BBF3_SPECIAL_SITE, i))
                ++passed;
            else if (++failed >= 20)
                break;
        }

        {
            uint8_t standard[BBF3_GATE_COUNT] = {0};
            standard[5] = (uint8_t)(Random32() & 0xfeu);
            if (RunChildBoundaryCase(
                    &log, &bus, reference, standard,
                    BBF3_CHILD_STANDARD, BBF3_STANDARD_SITE, i))
                ++passed;
            else if (++failed >= 20)
                break;
        }
    }

    Logf(&log, "\nSummary\n-------\n");
    Logf(&log, "bridge cases passed: %u\n", passed);
    Logf(&log, "bridge cases failed: %u\n", failed);
    Logf(&log, failed
        ? "RESULT: FAIL - $83:BBF3 bridge mismatch found\n"
        : "RESULT: PASS - $83:BBF3 bridge/stack boundary matched\n");

done:
    if (reference)
        interp816_free(reference);
    if (bus_initialized)
        SnesVerifyBusDestroy(&bus);
    free(rom);
    if (log.file)
        fclose(log.file);
    return failed ? 1 : 0;
}
