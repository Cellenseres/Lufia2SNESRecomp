#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu_state.h"
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/decomp.h"

extern RecompReturn Lufia2DecompBridge_D350(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_F9D4(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_FB12(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_FB71(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_C7F8(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_D508(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_C1B4(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_BB93(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_81C6(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_E03E(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_9FA9(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_BD77(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_80CD(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_8682(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_AEB5(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_9C72(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_CBAE(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_8DC5(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_CEF6(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_ECDB(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_99BF(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_83A0(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_867B(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_8103(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_E746(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_85DC(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_939C(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_81A9(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_92A4(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_B452(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_8B4B(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_9313(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_C627(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_B66E(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_B711(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_B747(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_86C1(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_9EDD(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_9CB8(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_8A2F(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_ECF0(CpuState *cpu);

enum {
    LUFIA2_ROM_SIZE = 0x280000,
    RETURN_WORD = 0x7ffe,
    CASES_PER_MODE = 2048,
    UNSUPPORTED_CASES = 512,
    WHOLE_CASES = 8192,
    STUB_SENTINEL = RECOMP_RETURN_LLE_UNWIND_BASE,
};

typedef enum StubKind {
    STUB_NONE = 0,
    STUB_DISPATCH,
    STUB_REWRITTEN,
    STUB_TAIL,
} StubKind;

typedef struct StubCall {
    StubKind kind;
    unsigned count;
    uint32_t target;
    uint32_t site;
    uint16_t restore_s;
    uint16_t entry_s;
    uint8_t hrv;
} StubCall;

typedef struct BridgeTarget {
    const char *name;
    RecompReturn (*bridge)(CpuState *);
    uint32_t entry;
    uint8_t frame;
    bool allow_x8;
    bool dp_page0;
} BridgeTarget;

static const BridgeTarget kTargets[] = {
    {"D350", Lufia2DecompBridge_D350, 0x83d350u, 3, false, true},
    {"F9D4", Lufia2DecompBridge_F9D4, 0x83f9d4u, 2, false, false},
    {"FB12", Lufia2DecompBridge_FB12, 0x83fb12u, 3, true, false},
    {"FB71", Lufia2DecompBridge_FB71, 0x83fb71u, 3, false, false},
};

static SnesVerifyBus g_bus;
static StubCall g_stub;
static uint8_t g_seed[SNES_VERIFY_WRAM_SIZE];
static uint8_t g_native[SNES_VERIFY_WRAM_SIZE];
static uint64_t s_rng = UINT64_C(0x9e3779b97f4a7c15);

static uint32_t Random32(void) {
    s_rng ^= s_rng << 13;
    s_rng ^= s_rng >> 7;
    s_rng ^= s_rng << 17;
    return (uint32_t)(s_rng >> 32);
}

uint8 cpu_read8(CpuState *cpu, uint8 bank, uint16 address) {
    const uint8 value =
        SnesVerifyBusRead(&g_bus, ((uint32_t)bank << 16) | address);
    cpu->open_bus = value;
    return value;
}

void cpu_write8(CpuState *cpu, uint8 bank, uint16 address, uint8 value) {
    SnesVerifyBusWrite(&g_bus, ((uint32_t)bank << 16) | address, value);
    cpu->open_bus = value;
}

int cpu_dispatch_has_entry(CpuState *cpu, uint32 pc24) {
    (void)cpu;
    (void)pc24;
    return 0;
}

static RecompReturn RecordStub(
    StubKind kind, uint32_t target, uint32_t site,
    uint16_t restore_s, uint16_t entry_s, uint8_t hrv) {
    ++g_stub.count;
    g_stub.kind = kind;
    g_stub.target = target & 0xffffffu;
    g_stub.site = site & 0xffffffu;
    g_stub.restore_s = restore_s;
    g_stub.entry_s = entry_s;
    g_stub.hrv = hrv;
    return (RecompReturn)STUB_SENTINEL;
}

RecompReturn cpu_dispatch_pc_from(
    CpuState *cpu, uint32 pc24, uint16 miss_restore_s, uint32 source_pc24) {
    (void)cpu;
    return RecordStub(
        STUB_DISPATCH, pc24, source_pc24, miss_restore_s, 0, 0);
}

RecompReturn interp_tier_dispatch_rewritten_return(
    CpuState *cpu, uint32 target_pc24, uint32 site_pc24) {
    (void)cpu;
    return RecordStub(STUB_REWRITTEN, target_pc24, site_pc24, 0, 0, 0);
}

RecompReturn interp_tier_dispatch_tail(
    CpuState *cpu, uint32 target_pc24, uint32 site_pc24,
    uint16 entry_s, uint8 hrv) {
    (void)cpu;
    return RecordStub(
        STUB_TAIL, target_pc24, site_pc24, 0, entry_s, hrv);
}

int interp816_opcode_hook(uint32_t address) {
    (void)address;
    return 0;
}

static void RandomFill(uint8_t *ram) {
    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = Random32();
        memcpy(ram + i, &word, 4);
    }
}

static void Poke16(uint8_t *ram, uint32_t offset, uint16_t value) {
    ram[offset] = (uint8_t)value;
    ram[offset + 1] = (uint8_t)(value >> 8);
}

/* Random machine plus a sane actor slot and return frame. */
static void SeedCase(
    const BridgeTarget *target, CpuState *cpu, uint8_t return_bank) {
    static const uint16_t dps[4] = {0x0000u, 0x0020u, 0x0400u, 0x0a00u};
    static const uint16_t page0_dps[4] = {0x0000u, 0x0020u, 0x0080u, 0x00c0u};
    static const uint16_t stacks[3] = {0x1ff0u, 0x1d80u, 0x13f0u};
    static const uint8_t banks[6] = {0x00u, 0x7eu, 0x7fu, 0x80u, 0x83u, 0x91u};
    const uint16_t dp = target->dp_page0 ? page0_dps[Random32() & 3u]
                                         : dps[Random32() & 3u];
    const uint16_t s = stacks[Random32() % 3u];
    const uint8_t slot = (uint8_t)(Random32() % 40u);

    RandomFill(g_bus.wram);
    Poke16(g_bus.wram, dp + 0xa7u, slot);
    Poke16(g_bus.wram, dp + 0xabu,
        (Random32() & 1u) ? (uint16_t)(slot * 3u)
                          : (uint16_t)(Random32() & 0xffu));
    if (Random32() & 1u)
        g_bus.wram[0x0622u + slot] &= (uint8_t)~0x28u;
    Poke16(g_bus.wram, 0x05aau, (uint16_t)(Random32() & 0x3eu));
    g_bus.wram[s + 1u] = (uint8_t)RETURN_WORD;
    g_bus.wram[s + 2u] = (uint8_t)(RETURN_WORD >> 8);
    if (target->frame == 3)
        g_bus.wram[s + 3u] = return_bank;

    memset(cpu, 0, sizeof(*cpu));
    cpu->A = (uint16_t)Random32();
    cpu->X = (uint16_t)Random32();
    cpu->Y = (uint16_t)Random32();
    cpu->S = s;
    cpu->D = dp;
    cpu->DB = banks[Random32() % 6u];
    cpu->PB = 0x83;
    cpu->m_flag = 1;
    cpu->x_flag = target->allow_x8 ? (uint8_t)(Random32() & 1u) : 0;
    cpu->emulation = 0;
    cpu->_flag_C = Random32() & 1u;
    cpu->_flag_Z = Random32() & 1u;
    cpu->_flag_V = Random32() & 1u;
    cpu->_flag_N = Random32() & 1u;
    cpu->_flag_I = Random32() & 1u;
    cpu->_flag_D = 0;
    cpu->ram = g_bus.wram;
    if (cpu->x_flag) {
        cpu->X &= 0x00ffu;
        cpu->Y &= 0x00ffu;
    }
    cpu_mirrors_to_p(cpu);
}

static void InitReference(
    Interp816 *ref, const CpuState *input, uint32_t pc24) {
    ref->a = input->A;
    ref->x = input->X;
    ref->y = input->Y;
    ref->sp = input->S;
    ref->pc = (uint16_t)pc24;
    ref->dp = input->D;
    ref->k = (uint8_t)(pc24 >> 16);
    ref->db = input->DB;
    ref->c = input->_flag_C != 0;
    ref->z = input->_flag_Z != 0;
    ref->v = input->_flag_V != 0;
    ref->n = input->_flag_N != 0;
    ref->i = input->_flag_I != 0;
    ref->d = input->_flag_D != 0;
    ref->mf = input->m_flag != 0;
    ref->xf = input->x_flag != 0;
    ref->e = false;
    ref->irqWanted = false;
    ref->nmiWanted = false;
    ref->waiting = false;
    ref->stopped = false;
    interp816_set_brk_hook_enabled(ref, false);
}

static uint8_t PackP(const Interp816 *ref) {
    return (uint8_t)((ref->n ? 0x80u : 0) | (ref->v ? 0x40u : 0) |
                     (ref->mf ? 0x20u : 0) | (ref->xf ? 0x10u : 0) |
                     (ref->d ? 0x08u : 0) | (ref->i ? 0x04u : 0) |
                     (ref->z ? 0x02u : 0) | (ref->c ? 0x01u : 0));
}

/* PB is excluded: generated RTL leaves it to the caller. */
static bool SameState(const CpuState *native, const Interp816 *ref) {
    return native->A == ref->a && native->X == ref->x &&
           native->Y == ref->y && native->S == ref->sp &&
           native->D == ref->dp && native->DB == ref->db &&
           native->m_flag == (uint8_t)ref->mf &&
           native->x_flag == (uint8_t)ref->xf &&
           native->_flag_C == (uint8_t)ref->c &&
           native->_flag_Z == (uint8_t)ref->z &&
           native->_flag_V == (uint8_t)ref->v &&
           native->_flag_N == (uint8_t)ref->n &&
           native->_flag_I == (uint8_t)ref->i &&
           native->_flag_D == (uint8_t)ref->d &&
           native->P == PackP(ref) && native->emulation == 0;
}

static void Report(
    const char *name, const char *mode, unsigned index,
    const CpuState *native, const Interp816 *ref, RecompReturn ret,
    int stop, const char *why) {
    fprintf(stderr,
        "FAIL %s %s case %u: %s ret=%d stop=%d stub=%u/%u "
        "target=%06X site=%06X restore=%04X "
        "A=%04X/%04X X=%04X/%04X Y=%04X/%04X S=%04X/%04X DB=%02X/%02X "
        "P=%02X/%02X\n",
        name, mode, index, why, (int)ret, stop, (unsigned)g_stub.kind,
        g_stub.count, g_stub.target, g_stub.site, g_stub.restore_s,
        native->A, ref->a, native->X, ref->x, native->Y, ref->y,
        native->S, ref->sp, native->DB, ref->db, native->P, PackP(ref));
}

static uint32_t Fb12RtlSite(uint8_t direction) {
    static const uint32_t sites[4] = {
        0x83fb24u, 0x83fb27u, 0x83fb2au, 0x83fb2du};
    return sites[(direction >> 1) & 3u];
}

/* hrv: frame size, 0 or the wrong size. */
static bool RunBoundaryCase(
    Interp816 *ref, const BridgeTarget *target, unsigned index,
    unsigned hrv_mode) {
    static const uint8_t return_banks[3] = {0x80u, 0x83u, 0x85u};
    const uint8_t return_bank = return_banks[Random32() % 3u];
    const uint32_t sentinel = ((uint32_t)(
        target->frame == 3 ? return_bank : 0x83u) << 16) |
        (uint16_t)(RETURN_WORD + 1u);
    const char *mode = hrv_mode == 0 ? "paired" :
                       hrv_mode == 1 ? "dispatched" : "mismatched";
    CpuState native;
    CpuState input;
    RecompReturn ret;
    unsigned instructions = 0;
    uint32_t site;
    int stop;

    SeedCase(target, &native, return_bank);
    if (!strcmp(target->name, "FB12")) {
        native.A = (uint16_t)((native.A & 0xff00u) | ((Random32() & 3u) * 2u));
    }
    native.host_return_valid = hrv_mode == 0 ? target->frame :
        hrv_mode == 1 ? 0 : (uint8_t)(5u - target->frame);
    input = native;
    memcpy(g_seed, g_bus.wram, SNES_VERIFY_WRAM_SIZE);

    memset(&g_stub, 0, sizeof(g_stub));
    ret = target->bridge(&native);
    memcpy(g_native, g_bus.wram, SNES_VERIFY_WRAM_SIZE);

    memcpy(g_bus.wram, g_seed, SNES_VERIFY_WRAM_SIZE);
    InitReference(ref, &input, target->entry);
    stop = SnesVerifyRunUntil(ref, &sentinel, 1, 4096, &instructions);

    site = !strcmp(target->name, "FB12")
        ? Fb12RtlSite((uint8_t)input.A)
        : target->entry == 0x83d350u ? 0x83d3aeu
        : target->entry == 0x83f9d4u ? 0x83f9edu : 0x83fb8au;

    if (stop != 0 || !SameState(&native, ref) ||
        memcmp(g_native, g_bus.wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        Report(target->name, mode, index, &native, ref, ret, stop,
            "state/memory");
        return false;
    }
    if (hrv_mode == 0) {
        if (ret != RECOMP_RETURN_NORMAL || g_stub.count != 0 ||
            native.PB != input.PB) {
            Report(target->name, mode, index, &native, ref, ret, stop,
                "host return");
            return false;
        }
    } else if (ret != (RecompReturn)STUB_SENTINEL || g_stub.count != 1 ||
               g_stub.kind != STUB_DISPATCH || g_stub.target != sentinel ||
               g_stub.site != site ||
               g_stub.restore_s != (uint16_t)(input.S + target->frame)) {
        Report(target->name, mode, index, &native, ref, ret, stop,
            "dispatch");
        return false;
    }
    return true;
}

/* M0, X8, D=1 and emulation must reach LLE untouched. */
static bool RunUnsupportedCase(
    const BridgeTarget *target, unsigned index, unsigned kind) {
    CpuState native;
    CpuState input;
    RecompReturn ret;

    SeedCase(target, &native, 0x83u);
    switch (kind) {
    case 0: native.m_flag = 0; break;
    case 1: native._flag_D = 1; break;
    case 2: native.emulation = 1; break;
    case 4: native.D = (uint16_t)(native.D | 0x0400u); break;
    default:
        native.x_flag = 1;
        native.X &= 0x00ffu;
        native.Y &= 0x00ffu;
        break;
    }
    cpu_mirrors_to_p(&native);
    native.host_return_valid = (uint8_t)((index & 1u) ? target->frame : 0);
    input = native;
    memcpy(g_seed, g_bus.wram, SNES_VERIFY_WRAM_SIZE);

    memset(&g_stub, 0, sizeof(g_stub));
    ret = target->bridge(&native);
    /* The paired-frame peek updates open bus, like generated prologues. */
    native.open_bus = input.open_bus;

    if (ret != (RecompReturn)STUB_SENTINEL || g_stub.count != 1 ||
        g_stub.kind != STUB_TAIL || g_stub.target != target->entry ||
        g_stub.site != target->entry || g_stub.entry_s != input.S ||
        g_stub.hrv != input.host_return_valid ||
        memcmp(&native, &input, sizeof(native)) != 0 ||
        memcmp(g_seed, g_bus.wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        fprintf(stderr,
            "FAIL %s unsupported case %u kind %u: ret=%d stub=%u/%u "
            "target=%06X site=%06X\n",
            target->name, index, kind, (int)ret, (unsigned)g_stub.kind,
            g_stub.count, g_stub.target, g_stub.site);
        return false;
    }
    return true;
}

/* FB12 with an odd or out-of-range index. */
static bool RunFb12OutOfRangeCase(Interp816 *ref, unsigned index) {
    const BridgeTarget *target = &kTargets[2];
    CpuState native;
    CpuState input;
    RecompReturn ret;
    unsigned instructions = 0;
    uint16_t pointer;
    uint32_t jump;
    uint8_t direction;
    int stop;

    SeedCase(target, &native, 0x83u);
    do {
        direction = (uint8_t)Random32();
    } while (!(direction & 1u) && direction < 8u);
    native.A = (uint16_t)((native.A & 0xff00u) | direction);
    native.host_return_valid = (uint8_t)((index & 1u) ? 3u : 0u);
    input = native;
    memcpy(g_seed, g_bus.wram, SNES_VERIFY_WRAM_SIZE);
    pointer = (uint16_t)(
        SnesVerifyBusRead(&g_bus, 0x830000u | (uint16_t)(0xfb1au + direction)) |
        (SnesVerifyBusRead(
             &g_bus, 0x830000u | (uint16_t)(0xfb1bu + direction)) << 8));
    jump = 0x830000u | pointer;

    memset(&g_stub, 0, sizeof(g_stub));
    ret = target->bridge(&native);
    memcpy(g_native, g_bus.wram, SNES_VERIFY_WRAM_SIZE);

    memcpy(g_bus.wram, g_seed, SNES_VERIFY_WRAM_SIZE);
    InitReference(ref, &input, target->entry);
    stop = SnesVerifyRunUntil(ref, &jump, 1, 64, &instructions);

    if (stop != 0 || !SameState(&native, ref) ||
        memcmp(g_native, g_bus.wram, SNES_VERIFY_WRAM_SIZE) != 0 ||
        ret != (RecompReturn)STUB_SENTINEL || g_stub.count != 1 ||
        g_stub.kind != STUB_TAIL || g_stub.target != jump ||
        g_stub.site != 0x83fb17u || g_stub.entry_s != input.S ||
        g_stub.hrv != input.host_return_valid) {
        Report(target->name, "out-of-range", index, &native, ref, ret,
            stop, "tail");
        return false;
    }
    return true;
}

static uint8_t FrontRead(void *context, uint32_t address) {
    (void)context;
    return SnesVerifyBusRead(&g_bus, address);
}

static void FrontWrite(void *context, uint32_t address, uint8_t value) {
    (void)context;
    SnesVerifyBusWrite(&g_bus, address, value);
}

typedef struct WholeStats {
    unsigned host_return;
    unsigned dispatch_return;
    unsigned lle_boundary;
    unsigned lle_entry;
    unsigned unterminated;
} WholeStats;

/* Random actor, script and machine around the BB93 JSR. */
static void SeedC7F8(CpuState *cpu) {
    static const uint16_t dps[4] = {0x0000u, 0x0020u, 0x0000u, 0x0400u};
    static const uint8_t banks[4] = {0x00u, 0x7eu, 0x80u, 0x83u};
    static const uint8_t chain[] = {
        0x0c, 0x0d, 0x0e, 0x1a, 0x21, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x2a, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x39,
        0x3a, 0x3d, 0x40, 0x42};
    const uint16_t dp = dps[Random32() & 3u];
    const uint8_t slot = (uint8_t)(Random32() % 40u);
    const uint16_t record = (uint16_t)(slot * 3u);
    const bool chained = Random32() & 1u;

    RandomFill(g_bus.wram);
    for (uint16_t i = 0x1800u; i < 0x1f00u; ++i) {
        const unsigned roll = Random32() % 100u;
        if (chained)
            g_bus.wram[i] = roll < 95u ? chain[Random32() % sizeof(chain)]
                                       : (uint8_t)(Random32() % 0x50u);
        else
            g_bus.wram[i] = roll < 85u ? (uint8_t)(Random32() % 0x50u)
                                       : (uint8_t)Random32();
    }
    Poke16(g_bus.wram, dp + 0xa7u, slot);
    Poke16(g_bus.wram, dp + 0xa9u, (uint16_t)(slot * 2u));
    Poke16(g_bus.wram, dp + 0xabu, record);
    Poke16(g_bus.wram, 0x1e506u + record,
        (uint16_t)(0x1800u + (Random32() & 0x3ffu)));
    g_bus.wram[0x1e508u + record] = (Random32() & 3u) ? 0x7eu : 0x00u;
    if ((Random32() & 7u) != 0)
        g_bus.wram[0x0622u + slot] &= 0x7fu;
    if (Random32() & 3u)
        g_bus.wram[0x1291u + slot] &= 0xf8u;
    g_bus.wram[0x1e3c6u + slot] = (uint8_t)(Random32() % 3u);
    Poke16(g_bus.wram, 0x1724u, (uint16_t)(Random32() & 0x3fu));
    g_bus.wram[0x1ff1u] = (uint8_t)RETURN_WORD;
    g_bus.wram[0x1ff2u] = (uint8_t)(RETURN_WORD >> 8);

    memset(cpu, 0, sizeof(*cpu));
    cpu->A = (uint16_t)Random32();
    cpu->X = (uint16_t)Random32();
    cpu->Y = (uint16_t)Random32();
    cpu->S = 0x1ff0u;
    cpu->D = dp;
    cpu->DB = banks[Random32() & 3u];
    cpu->PB = 0x83;
    cpu->m_flag = 1;
    cpu->x_flag = (Random32() & 3u) ? 1u : 0u;
    cpu->_flag_C = Random32() & 1u;
    cpu->_flag_Z = Random32() & 1u;
    cpu->_flag_V = Random32() & 1u;
    cpu->_flag_N = Random32() & 1u;
    cpu->_flag_I = Random32() & 1u;
    cpu->ram = g_bus.wram;
    if (cpu->x_flag) {
        cpu->X &= 0x00ffu;
        cpu->Y &= 0x00ffu;
    }
    cpu_mirrors_to_p(cpu);
}

/* Random actor, secondary script and walk state. */
static void SeedD508(CpuState *cpu) {
    static const uint16_t dps[4] = {0x0000u, 0x0020u, 0x0000u, 0x0400u};
    static const uint8_t banks[4] = {0x00u, 0x7eu, 0x80u, 0x83u};
    static const uint8_t known[] = {
        0xf0, 0xf0, 0xf0, 0xf0, 0xf1, 0xf9, 0xfa, 0xfb, 0xfc, 0xfe,
        0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe9, 0xea, 0xec,
        0xed, 0xee, 0xef, 0xd0, 0xd1, 0xd5, 0xd6, 0xd7, 0x13, 0x22,
        0x31, 0x55, 0x74, 0x92, 0x93, 0xa1, 0xb2, 0x00, 0x43, 0x01,
        0x83, 0xc1, 0xc2, 0xf2, 0xf5, 0xf6, 0xf7, 0xf8, 0xfd, 0xff,
        0xeb, 0xd4, 0x61, 0xf3, 0xf4, 0xe7, 0xe8, 0xd2};
    const uint16_t dp = dps[Random32() & 3u];
    const uint8_t slot = (uint8_t)(Random32() % 40u);
    const uint16_t record = (uint16_t)(slot * 3u);

    RandomFill(g_bus.wram);
    for (uint16_t i = 0x1800u; i < 0x1f00u; ++i)
        g_bus.wram[i] = (Random32() % 100u) < 80u
            ? known[Random32() % sizeof(known)] : (uint8_t)Random32();
    for (unsigned i = 0; i < 0x5000u; ++i)
        if (Random32() & 3u)
            g_bus.wram[0x4000u + i] = 0;
    g_bus.wram[0x05b9u] = (uint8_t)(0x20u + (Random32() & 0x10u));
    Poke16(g_bus.wram, 0x05aau, 0);
    Poke16(g_bus.wram, 0x1d008u, 0);
    Poke16(g_bus.wram, dp + 0xa7u, slot);
    Poke16(g_bus.wram, dp + 0xa9u, (uint16_t)(slot * 2u));
    Poke16(g_bus.wram, dp + 0xabu, record);
    Poke16(g_bus.wram, 0x1e3eeu + record,
        (uint16_t)(0x1800u + (Random32() & 0x3ffu)));
    g_bus.wram[0x1e3f0u + record] = (Random32() & 3u) ? 0x7eu : 0x00u;
    if (Random32() & 3u)
        g_bus.wram[0x0622u + slot] |= 0x80u;
    if (Random32() & 1u)
        g_bus.wram[0x0622u + slot] &= (uint8_t)~0x0au;
    if (Random32() & 1u)
        g_bus.wram[0x1e48eu + slot] = (uint8_t)(Random32() & 0x0fu);
    g_bus.wram[0x1e4deu + slot] = (uint8_t)(1u << (Random32() & 3u));
    g_bus.wram[0x1ff1u] = (uint8_t)RETURN_WORD;
    g_bus.wram[0x1ff2u] = (uint8_t)(RETURN_WORD >> 8);

    memset(cpu, 0, sizeof(*cpu));
    cpu->A = (uint16_t)Random32();
    cpu->X = (uint16_t)Random32();
    cpu->Y = (uint16_t)Random32();
    cpu->S = 0x1ff0u;
    cpu->D = dp;
    cpu->DB = banks[Random32() & 3u];
    cpu->PB = 0x83;
    cpu->m_flag = 1;
    cpu->x_flag = (Random32() & 3u) ? 1u : 0u;
    cpu->_flag_C = Random32() & 1u;
    cpu->_flag_Z = Random32() & 1u;
    cpu->_flag_V = Random32() & 1u;
    cpu->_flag_N = Random32() & 1u;
    cpu->_flag_I = Random32() & 1u;
    cpu->ram = g_bus.wram;
    if (cpu->x_flag) {
        cpu->X &= 0x00ffu;
        cpu->Y &= 0x00ffu;
    }
    cpu_mirrors_to_p(cpu);
}

/* Leader, controller, map and door table for C1B4. */
static void SeedC1B4(CpuState *cpu) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[8] = {
        0x83u, 0x83u, 0x83u, 0x83u, 0x83u, 0x00u, 0x80u, 0x7eu};
    static const uint8_t pads[4] = {1, 2, 4, 8};
    const uint16_t dp = dps[Random32() & 7u];
    const uint8_t px = (uint8_t)(4u + Random32() % 8u);
    const uint8_t py = (uint8_t)(4u + Random32() % 8u);
    const unsigned doors = Random32() % 4u;
    uint32_t entry = 0xf010u;

    RandomFill(g_bus.wram);
    if (Random32() & 7u)
        g_bus.wram[0x099bu] &= 0x7fu;
    g_bus.wram[dp + 0x46u] &= (Random32() & 3u) ? 0x5fu : 0xffu;
    g_bus.wram[dp + 0x47u] = (uint8_t)(
        (Random32() & 0xe0u & ((Random32() & 3u) ? 0xdfu : 0xffu)) |
        ((Random32() & 7u) ? pads[Random32() & 3u]
                           : (uint8_t)(Random32() & 0x0fu)));
    if (Random32() & 1u)
        g_bus.wram[0x057cu] = 0;
    Poke16(g_bus.wram, dp + 0xa7u,
        (Random32() & 7u) ? 0u : (uint16_t)(Random32() % 40u));
    g_bus.wram[0x06bau] = px;
    g_bus.wram[0x06e2u] = py;
    if (Random32() & 7u)
        g_bus.wram[0x0692u] = (uint8_t)((Random32() & 3u) * 2u);
    for (unsigned slot = 8; slot < 40u; ++slot) {
        g_bus.wram[0x06bau + slot] = (uint8_t)(px + Random32() % 7u - 3u);
        g_bus.wram[0x06e2u + slot] = (uint8_t)(py + Random32() % 7u - 3u);
        if (Random32() & 3u)
            g_bus.wram[0x0622u + slot] &= 0xfbu;
        g_bus.wram[0x1e216u + slot] = (uint8_t)(1u + (Random32() & 1u));
    }
    g_bus.wram[0x05b9u] = (uint8_t)(0x20u + (Random32() & 0x10u));
    Poke16(g_bus.wram, 0x05aau, 0);
    Poke16(g_bus.wram, 0x1d008u, 0);
    Poke16(g_bus.wram, 0x1d03eu, 0x8000u);
    for (unsigned i = 0; i < 0x2000u; ++i)
        if (Random32() & 1u)
            g_bus.wram[0x4000u + i] &= 0x31u;
    for (unsigned i = 0; i < 0x400u; ++i) {
        const unsigned roll = Random32() % 6u;
        g_bus.wram[0x18000u + i] = roll < 2u ? 6u : roll == 2u ? 7u :
            roll == 3u ? 0u : (uint8_t)Random32();
    }
    Poke16(g_bus.wram, 0xf002u, 0x0010u);
    for (unsigned i = 0; i < doors; ++i, entry += 15u) {
        for (unsigned r = 0; r < 2u; ++r) {
            const uint32_t rect = entry + 5u + 4u * r;
            g_bus.wram[rect + 0u] = (uint8_t)(px + 2u - Random32() % 5u);
            g_bus.wram[rect + 1u] = (uint8_t)(py + 2u - Random32() % 5u);
            g_bus.wram[rect + 2u] = (uint8_t)(px + Random32() % 5u - 1u);
            g_bus.wram[rect + 3u] = (uint8_t)(py + Random32() % 5u - 1u);
        }
        g_bus.wram[entry] = (uint8_t)(Random32() % 0xffu);
        g_bus.wram[entry + 2u] = (uint8_t)(py + Random32() % 5u - 2u);
    }
    g_bus.wram[entry] = 0xffu;
    g_bus.wram[0x1ff1u] = (uint8_t)RETURN_WORD;
    g_bus.wram[0x1ff2u] = (uint8_t)(RETURN_WORD >> 8);

    memset(cpu, 0, sizeof(*cpu));
    cpu->A = (uint16_t)Random32();
    cpu->X = (uint16_t)Random32();
    cpu->Y = (uint16_t)Random32();
    cpu->S = 0x1ff0u;
    cpu->D = dp;
    cpu->DB = banks[Random32() & 7u];
    cpu->PB = 0x83;
    cpu->m_flag = 1;
    cpu->x_flag = (Random32() & 3u) ? 0u : 1u;
    cpu->_flag_C = Random32() & 1u;
    cpu->_flag_Z = Random32() & 1u;
    cpu->_flag_V = Random32() & 1u;
    cpu->_flag_N = Random32() & 1u;
    cpu->_flag_I = Random32() & 1u;
    cpu->ram = g_bus.wram;
    if (cpu->x_flag) {
        cpu->X &= 0x00ffu;
        cpu->Y &= 0x00ffu;
    }
    cpu_mirrors_to_p(cpu);
}

/* Children of BB93 run in their own interpreter. */
static Interp816 *g_child;
static bool g_child_stuck;

static void InterpToCpu(const Interp816 *ref, CpuState *cpu) {
    cpu->A = ref->a;
    cpu->X = ref->x;
    cpu->Y = ref->y;
    cpu->S = ref->sp;
    cpu->D = ref->dp;
    cpu->DB = ref->db;
    cpu->PB = ref->k;
    cpu->_flag_C = ref->c;
    cpu->_flag_Z = ref->z;
    cpu->_flag_N = ref->n;
    cpu->_flag_V = ref->v;
    cpu->_flag_D = ref->d;
    cpu->_flag_I = ref->i;
    cpu->m_flag = ref->mf;
    cpu->x_flag = ref->xf;
    cpu_mirrors_to_p(cpu);
}

/* Run pc24 in g_child until RTS/RTL back to S. */
static bool RunChildToReturn(
    CpuState *cpu, uint32_t pc24, uint32_t back, uint16_t entry_s) {
    InitReference(g_child, cpu, pc24);
    for (unsigned n = 0; n < 1000000u; ++n) {
        if (SnesVerifyPc24(g_child) == back && g_child->sp == entry_s) {
            InterpToCpu(g_child, cpu);
            return true;
        }
        interp816_runOpcode(g_child);
    }
    g_child_stuck = true;
    return false;
}

static bool g_bad_site;

RecompReturn cpu_dispatch_call_pc(
    CpuState *cpu, uint32 target, uint32 source_pc24) {
    const uint16_t entry_s = cpu->S;
    const uint16_t pushed = (uint16_t)(source_pc24 + 2u);

    /* source_pc24 must hold JSR target. */
    if (SnesVerifyBusRead(&g_bus, source_pc24) != 0x20u ||
        (SnesVerifyBusRead(&g_bus, source_pc24 + 1u) |
         ((uint32_t)SnesVerifyBusRead(&g_bus, source_pc24 + 2u) << 8)) !=
            (target & 0xffffu))
        g_bad_site = true;

    cpu_write8(cpu, 0x00, cpu->S, (uint8_t)(pushed >> 8));
    cpu->S = (uint16_t)(cpu->S - 1u);
    cpu_write8(cpu, 0x00, cpu->S, (uint8_t)pushed);
    cpu->S = (uint16_t)(cpu->S - 1u);
    cpu->host_return_valid = 2;
    if (!RunChildToReturn(
            cpu, target,
            (source_pc24 & 0xff0000u) | (uint16_t)(source_pc24 + 3u),
            entry_s))
        return RECOMP_RETURN_SKIP_2;
    return RECOMP_RETURN_NORMAL;
}

/* Verifier-side child for the portable BB93. */
static uint8_t FrontSlotChild(
    void *context,
    Lufia2CpuState *state,
    uint32_t target,
    uint32_t site) {
    CpuState cpu;

    (void)context;
    memset(&cpu, 0, sizeof(cpu));
    cpu.A = state->accumulator;
    cpu.X = state->x;
    cpu.Y = state->y;
    cpu.S = state->stack;
    cpu.D = state->direct_page;
    cpu.DB = state->data_bank;
    cpu.PB = (uint8_t)(target >> 16);
    cpu._flag_C = state->carry;
    cpu._flag_Z = state->zero;
    cpu._flag_N = state->negative;
    cpu._flag_V = state->overflow;
    cpu._flag_D = state->decimal;
    cpu._flag_I = state->irq_disable;
    cpu.m_flag = state->accumulator_is_8_bit;
    cpu.x_flag = state->index_is_8_bit;
    cpu.ram = g_bus.wram;
    cpu_mirrors_to_p(&cpu);
    if (cpu_dispatch_call_pc(&cpu, target, site) != RECOMP_RETURN_NORMAL)
        return 0;
    state->accumulator = cpu.A;
    state->x = cpu.X;
    state->y = cpu.Y;
    state->stack = cpu.S;
    state->direct_page = cpu.D;
    state->data_bank = cpu.DB;
    state->program_bank = cpu.PB;
    state->carry = cpu._flag_C;
    state->zero = cpu._flag_Z;
    state->negative = cpu._flag_N;
    state->overflow = cpu._flag_V;
    state->decimal = cpu._flag_D;
    state->irq_disable = cpu._flag_I;
    state->accumulator_is_8_bit = cpu.m_flag;
    state->index_is_8_bit = cpu.x_flag;
    return 1;
}

/* Iterations before the BBA1 handoff, as counted at BBA5. */
static Lufia2ExecutionResult DecompBB93(
    const Lufia2Memory *memory, Lufia2CpuState *cpu) {
    Lufia2ExecutionResult result =
        Lufia2UpdateActorSlots(memory, cpu, FrontSlotChild, NULL);
    if (result.dispatches)
        --result.dispatches;
    return result;
}

/* Field-loop state for BB93: slots, scripts, flags. */
static void SeedBB93(CpuState *cpu) {
    static const uint8_t ops[] = {
        0xf0, 0xf0, 0x00, 0x43, 0xfa, 0x13, 0x22, 0x92, 0xe9, 0xd0,
        0x0c, 0x0d, 0x0e, 0x21, 0x23, 0x30, 0x31, 0x3a, 0x3d, 0x40};
    const uint16_t dp = (Random32() & 7u) == 7u ? 0x0020u : 0x0000u;

    RandomFill(g_bus.wram);
    for (uint16_t i = 0x1800u; i < 0x1f00u; ++i)
        g_bus.wram[i] = ops[Random32() % sizeof(ops)];
    for (unsigned slot = 0; slot < 40u; ++slot) {
        const uint16_t record = (uint16_t)(slot * 3u);
        if (Random32() % 3u)
            g_bus.wram[0x0622u + slot] |= 0x04u;
        Poke16(g_bus.wram, 0x1e3eeu + record,
            (uint16_t)(0x1800u + (Random32() & 0x3ffu)));
        g_bus.wram[0x1e3f0u + record] = (Random32() & 1u) ? 0x7eu : 0x00u;
        Poke16(g_bus.wram, 0x1e506u + record,
            (uint16_t)(0x1800u + (Random32() & 0x3ffu)));
        g_bus.wram[0x1e508u + record] = (Random32() & 1u) ? 0x7eu : 0x00u;
        g_bus.wram[0x1e4deu + slot] = (uint8_t)(1u << (Random32() & 3u));
        g_bus.wram[0x1e216u + slot] = (uint8_t)(1u + (Random32() & 1u));
    }
    g_bus.wram[0x05b9u] = 0x20u;
    Poke16(g_bus.wram, 0x05aau, 0);
    Poke16(g_bus.wram, 0x1d008u, 0);
    for (unsigned i = 0; i < 0x2000u; ++i)
        if (Random32() & 1u)
            g_bus.wram[0x4000u + i] = 0;
    if (Random32() & 1u)
        g_bus.wram[0x1d0feu] = 0;
    if (Random32() & 3u)
        g_bus.wram[0x09a1u] = 0xffu;
    g_bus.wram[0x1ff1u] = (uint8_t)RETURN_WORD;
    g_bus.wram[0x1ff2u] = (uint8_t)(RETURN_WORD >> 8);
    g_bus.wram[0x1ff3u] = 0x83u;

    memset(cpu, 0, sizeof(*cpu));
    cpu->A = (uint16_t)Random32();
    cpu->X = (uint16_t)(Random32() & 0xffu);
    cpu->Y = (uint16_t)(Random32() & 0xffu);
    cpu->S = 0x1ff0u;
    cpu->D = dp;
    cpu->DB = (Random32() & 3u) ? 0x83u : 0x00u;
    cpu->PB = 0x83;
    cpu->m_flag = 1;
    cpu->x_flag = (Random32() & 7u) ? 1u : 0u;
    cpu->_flag_C = Random32() & 1u;
    cpu->_flag_Z = Random32() & 1u;
    cpu->_flag_V = Random32() & 1u;
    cpu->_flag_N = Random32() & 1u;
    cpu->_flag_I = Random32() & 1u;
    cpu->ram = g_bus.wram;
    if (!cpu->x_flag)
        cpu->X = (uint16_t)Random32();
    cpu_mirrors_to_p(cpu);
}

/* Field loop state around the 81C6 JSR. */
static void Seed81C6(CpuState *cpu) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[8] = {
        0x83u, 0x83u, 0x83u, 0x83u, 0x83u, 0x00u, 0x80u, 0x7eu};
    const uint16_t dp = dps[Random32() & 7u];
    const uint8_t px = (uint8_t)(4u + Random32() % 8u);
    const uint8_t py = (uint8_t)(4u + Random32() % 8u);

    RandomFill(g_bus.wram);
    for (unsigned t = 0; t < 8u; ++t) {
        const unsigned roll = Random32() % 16u;
        g_bus.wram[0x1d18cu + t] = roll < 10u ? (uint8_t)(Random32() & 0x7fu)
            : roll < 15u || (Random32() & 3u)
                ? (uint8_t)(0x82u + (Random32() % 0x7du)) : 0x81u;
    }
    if (Random32() & 3u)
        g_bus.wram[0x09a7u] |= 0x01u;
    if (Random32() & 7u) {
        g_bus.wram[0x09a8u] &= 0xf7u;
        g_bus.wram[0x0622u] &= 0x77u;
        g_bus.wram[0x05b7u] &= 0xf8u;
        g_bus.wram[0x05b5u] &= 0x5du;
        g_bus.wram[0x17aau] = 0;
        g_bus.wram[0x099bu] &= 0x7fu;
        for (unsigned i = 0; i < 8u; ++i)
            if (Random32() % 16u)
                g_bus.wram[0x1d057u + i] &= 0x7fu;
    }
    if (Random32() & 3u)
        g_bus.wram[0x1d0a1u] = 0;
    if (Random32() & 3u)
        g_bus.wram[0x05b5u] &= 0xefu;
    if (Random32() & 1u)
        g_bus.wram[0x057cu] = 0;
    g_bus.wram[0x06bau] = px;
    g_bus.wram[0x06e2u] = py;
    for (unsigned slot = 8; slot < 40u; ++slot) {
        g_bus.wram[0x06bau + slot] = (uint8_t)(px + Random32() % 5u - 2u);
        g_bus.wram[0x06e2u + slot] = (uint8_t)(py + Random32() % 5u - 2u);
        if (Random32() & 3u) {
            g_bus.wram[0x0622u + slot] &= 0x7bu;
            g_bus.wram[0x0736u + slot] &= 0xebu;
        }
        g_bus.wram[0x1e216u + slot] = (uint8_t)(1u + (Random32() & 1u));
    }
    g_bus.wram[0x05b9u] = (uint8_t)(0x20u + (Random32() & 0x10u));
    Poke16(g_bus.wram, 0x05aau, 0);
    Poke16(g_bus.wram, 0x1d008u, 0);
    for (unsigned i = 0; i < 0x2000u; ++i)
        if (Random32() & 1u)
            g_bus.wram[0x4000u + i] &= 0x31u;
    g_bus.wram[0x1ff1u] = (uint8_t)RETURN_WORD;
    g_bus.wram[0x1ff2u] = (uint8_t)(RETURN_WORD >> 8);

    memset(cpu, 0, sizeof(*cpu));
    cpu->A = (uint16_t)Random32();
    cpu->X = (uint16_t)Random32();
    cpu->Y = (uint16_t)Random32();
    cpu->S = 0x1ff0u;
    cpu->D = dp;
    cpu->DB = banks[Random32() & 7u];
    cpu->PB = 0x83;
    cpu->m_flag = 1;
    cpu->x_flag = (Random32() & 7u) ? 1u : 0u;
    cpu->_flag_C = Random32() & 1u;
    cpu->_flag_Z = Random32() & 1u;
    cpu->_flag_V = Random32() & 1u;
    cpu->_flag_N = Random32() & 1u;
    cpu->_flag_I = Random32() & 1u;
    cpu->ram = g_bus.wram;
    if (cpu->x_flag) {
        cpu->X &= 0x00ffu;
        cpu->Y &= 0x00ffu;
    }
    cpu_mirrors_to_p(cpu);
}

/* Object slots around the field-loop E03E JSR. */
static void SeedE03E(CpuState *cpu) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[8] = {
        0x83u, 0x83u, 0x83u, 0x83u, 0x83u, 0x00u, 0x80u, 0x7eu};
    const uint16_t dp = dps[Random32() & 7u];

    static const uint8_t ops[] = {
        0x41, 0x43, 0x4f, 0xf6, 0xf6, 0xf2, 0xf2, 0x1a, 0x1a, 0x80,
        0xf6, 0xf2, 0x1a, 0x00, 0x33, 0x9a, 0x2f, 0x2e, 0x1d, 0x12,
        0x13, 0x22, 0x23, 0x8d, 0x72, 0x29, 0x2b, 0xc0, 0xc3, 0x84,
        0x89, 0xea, 0xf8, 0xf9, 0xe2, 0x82, 0x83, 0xeb, 0xec, 0xed,
        0x35, 0x2c, 0x25, 0xfd, 0x8f, 0x01, 0x80, 0x53, 0x28, 0xd0,
        0xfb, 0xfc, 0xe3, 0xf7, 0x20, 0xf1, 0x19, 0x8e, 0x26, 0xa0,
        0x60, 0x85, 0x8a, 0xee};

    RandomFill(g_bus.wram);
    for (uint16_t i = 0x1800u; i < 0x1f00u; ++i)
        g_bus.wram[i] = (Random32() % 100u) < 85u
            ? ops[Random32() % sizeof(ops)] : (uint8_t)Random32();
    for (unsigned slot = 0; slot < 32u; ++slot) {
        const uint16_t record = (uint16_t)(slot * 3u);

        if (Random32() & 1u)
            g_bus.wram[0x064au + slot] &= 0x7fu;
        g_bus.wram[0x1dfaeu + slot] = (Random32() & 3u)
            ? (uint8_t)(2u + Random32() % 200u) : 1u;
        Poke16(g_bus.wram, 0x1deeeu + record,
            (uint16_t)(0x1800u + (Random32() & 0x3ffu)));
        g_bus.wram[0x1def0u + record] = (Random32() & 3u) ? 0x7eu : 0x00u;
        Poke16(g_bus.wram, 0x1dfceu + record,
            (uint16_t)(0x1800u + (Random32() & 0x3ffu)));
        g_bus.wram[0x1dfd0u + record] = 0x7eu;
        g_bus.wram[0x1e08eu + slot] = (uint8_t)(Random32() % 4u);
        if (Random32() & 1u)
            g_bus.wram[0x1e386u + slot] =
                (uint8_t)((Random32() & 0xf0u) | 1u);
        if (Random32() & 1u)
            g_bus.wram[0x1daecu + slot] = 0x1fu;
        g_bus.wram[0x1e23eu + slot] &= 0x0fu;
    }
    if (Random32() & 3u)
        g_bus.wram[0x09a7u] |= 0x01u;
    g_bus.wram[0x1ff1u] = (uint8_t)RETURN_WORD;
    g_bus.wram[0x1ff2u] = (uint8_t)(RETURN_WORD >> 8);

    memset(cpu, 0, sizeof(*cpu));
    cpu->A = (uint16_t)Random32();
    cpu->X = (uint16_t)Random32();
    cpu->Y = (uint16_t)Random32();
    cpu->S = 0x1ff0u;
    cpu->D = dp;
    cpu->DB = banks[Random32() & 7u];
    cpu->PB = 0x83;
    cpu->m_flag = 1;
    cpu->x_flag = (Random32() & 7u) ? 1u : 0u;
    cpu->_flag_C = Random32() & 1u;
    cpu->_flag_Z = Random32() & 1u;
    cpu->_flag_V = Random32() & 1u;
    cpu->_flag_N = Random32() & 1u;
    cpu->_flag_I = Random32() & 1u;
    cpu->ram = g_bus.wram;
    if (cpu->x_flag) {
        cpu->X &= 0x00ffu;
        cpu->Y &= 0x00ffu;
    }
    cpu_mirrors_to_p(cpu);
}

/* Field NMI queues around the $00:0067 JSL. */
static void Seed9FA9(CpuState *cpu) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    const uint16_t dp = dps[Random32() & 7u];

    RandomFill(g_bus.wram);
    for (unsigned i = 0; i < 8u; ++i) {
        if (Random32() & 3u)
            Poke16(g_bus.wram, 0x1246u + 2u * i, 0);
        if (Random32() & 3u)
            Poke16(g_bus.wram, 0x1236u + 2u * i, 0);
        if (Random32() & 3u)
            Poke16(g_bus.wram, 0x05c2u + 2u * i, 0);
    }
    if (Random32() & 1u) {
        const uint8_t count = (uint8_t)(2u * (1u + Random32() % 4u));
        g_bus.wram[0x1d4f8u] = count;
        for (unsigned i = 2; i <= count; i += 2)
            g_bus.wram[0x1d538u + i] = (uint8_t)(1u + Random32() % 3u);
    } else {
        g_bus.wram[0x1d4f8u] = 0;
    }
    g_bus.wram[0x1ff1u] = (uint8_t)RETURN_WORD;
    g_bus.wram[0x1ff2u] = (uint8_t)(RETURN_WORD >> 8);
    g_bus.wram[0x1ff3u] = 0x83u;

    memset(cpu, 0, sizeof(*cpu));
    cpu->A = (uint16_t)Random32();
    cpu->X = (uint16_t)Random32();
    cpu->Y = (uint16_t)Random32();
    cpu->S = 0x1ff0u;
    cpu->D = dp;
    cpu->DB = (Random32() & 1u) ? 0x80u : 0x00u;
    cpu->PB = 0x83;
    cpu->m_flag = 1;
    cpu->x_flag = (Random32() & 3u) ? 1u : 0u;
    cpu->_flag_C = Random32() & 1u;
    cpu->_flag_Z = Random32() & 1u;
    cpu->_flag_V = Random32() & 1u;
    cpu->_flag_N = Random32() & 1u;
    cpu->_flag_I = Random32() & 1u;
    cpu->ram = g_bus.wram;
    if (cpu->x_flag) {
        cpu->X &= 0x00ffu;
        cpu->Y &= 0x00ffu;
    }
    cpu_mirrors_to_p(cpu);
}

/* Field-loop children: open idle gates, sparse slots. */
static void SeedFieldChild(CpuState *cpu) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[4] = {0x83u, 0x83u, 0x80u, 0x00u};
    static const uint16_t gates[6] = {
        0x09a8u, 0x0622u, 0x05b7u, 0x05b5u, 0x17aau, 0x099bu};
    static const uint8_t masks[6] = {0x08u, 0x88u, 0x07u, 0xa2u, 0xffu, 0x80u};
    const uint16_t dp = dps[Random32() & 7u];

    RandomFill(g_bus.wram);
    for (unsigned i = 0; i < 6u; ++i)
        if (Random32() & 7u)
            g_bus.wram[gates[i]] &= (uint8_t)~masks[i];
    for (unsigned i = 0; i < 8u; ++i)
        if (Random32() & 7u)
            g_bus.wram[0x1d057u + i] &= 0x7fu;
    g_bus.wram[0x1ff1u] = (uint8_t)RETURN_WORD;
    g_bus.wram[0x1ff2u] = (uint8_t)(RETURN_WORD >> 8);

    memset(cpu, 0, sizeof(*cpu));
    cpu->A = (uint16_t)Random32();
    cpu->X = (uint16_t)Random32();
    cpu->Y = (uint16_t)Random32();
    cpu->S = 0x1ff0u;
    cpu->D = dp;
    cpu->DB = banks[Random32() & 3u];
    cpu->PB = 0x83;
    cpu->m_flag = 1;
    cpu->x_flag = (Random32() & 7u) ? 1u : 0u;
    cpu->_flag_C = Random32() & 1u;
    cpu->_flag_Z = Random32() & 1u;
    cpu->_flag_V = Random32() & 1u;
    cpu->_flag_N = Random32() & 1u;
    cpu->_flag_I = Random32() & 1u;
    cpu->ram = g_bus.wram;
    if (cpu->x_flag) {
        cpu->X &= 0x00ffu;
        cpu->Y &= 0x00ffu;
    }
    cpu_mirrors_to_p(cpu);
}

/* Palette cycles on, wave table live. */
static void SeedAEB5(CpuState *cpu) {
    SeedFieldChild(cpu);
    g_bus.wram[0x09a9u] &= 0xf8u;
    if (Random32() & 3u)
        g_bus.wram[0x09a9u] |= 0x01u;
    if (!(Random32() & 7u))
        g_bus.wram[0x09a9u] |= 0x02u;
    if (!(Random32() & 7u))
        g_bus.wram[0x09a9u] |= 0x04u;
    g_bus.wram[0x1d0f7u] = (uint8_t)(Random32() % 6u);
    for (unsigned i = 0; i < 12u; i += 2u)
        g_bus.wram[0x1ed01u + i] = (uint8_t)(1u + Random32() % 3u);
    g_bus.wram[0x1d0cau] = (Random32() & 3u) ? 0xc0u : 0xffu;
    g_bus.wram[0x1d0c9u] = (uint8_t)(Random32() & 0x7eu);
}

/* Idle effects for the field loop's JSL $80:9C72. */
static void Seed9C72(CpuState *cpu) {
    SeedFieldChild(cpu);
    if (Random32() & 1u)
        g_bus.wram[0x1261u] &= 0x48u;
    if (Random32() & 3u)
        g_bus.wram[0x1262u] &= 0xfeu;
    g_bus.wram[0x1d0fau] = (uint8_t)(Random32() % 0x70u * 2u);
    g_bus.wram[0x1d0fbu] = 0;
    for (unsigned i = 0; i < 3u; ++i)
        if (Random32() & 1u)
            g_bus.wram[0x1274u + 2u * i] = g_bus.wram[0x1275u + 2u * i] = 0;
    g_bus.wram[0x1d0c1u] = (Random32() & 1u)
        ? 0xffu : (uint8_t)(1u + Random32() % 3u);
    if (Random32() & 3u)
        g_bus.wram[0x099bu] &= 0x75u;
    if (Random32() & 1u)
        g_bus.wram[0x1265u] = 0;
    g_bus.wram[0x1266u] =
        (uint8_t)(g_bus.wram[0x1265u] - 1u - (Random32() & 1u));
    g_bus.wram[0x09b2u] = (uint8_t)(0x20u + (Random32() & 0x3fu));
    g_bus.wram[0x109b2u] = (uint8_t)(0x20u + (Random32() & 0x3fu));
    g_bus.wram[0x1ff3u] = 0x83u;
    cpu->PB = 0x80;
}

/* Text step with printable WRAM text. */
static void Seed9CB8(CpuState *cpu) {
    Seed9C72(cpu);
    g_bus.wram[0x099bu] |= 0x81u;
    g_bus.wram[0x1d0ffu] = 0;
    g_bus.wram[0x1259u] = 0;
    g_bus.wram[0x09b9u] = 0x7eu;
    Poke16(g_bus.wram, 0x09b7u, 0x3000u);
    g_bus.wram[0x3000u] = (uint8_t)(0x20u + Random32() % 0x60u);
    if (Random32() & 1u) {
        const uint16_t text = (uint16_t)(0x8000u + (Random32() & 0x7ff0u));
        const uint8_t actor = (uint8_t)(Random32() & 0x3fu);

        g_bus.wram[0x099bu] &= 0xfeu;
        Poke16(g_bus.wram, 0x09b7u, text);
        g_bus.wram[text] = 0x33u;
        g_bus.wram[(uint16_t)(text + 3u)] =
            (Random32() & 1u) ? 0x33u : (uint8_t)Random32();
        g_bus.wram[0x1269u] = (Random32() & 3u) ? actor : (uint8_t)Random32();
        g_bus.wram[0x0622u + actor] &= (Random32() & 1u) ? 0x77u : 0xffu;
        if (Random32() & 1u)
            g_bus.wram[0x099cu] &= 0xfeu;
        {
            static const uint8_t ops[64] = {
                0x33u, 0x03u, 0x37u, 0x3cu, 0x00u, 0x42u, 0x68u, 0x05u,
                0x06u, 0x0fu, 0x15u, 0x1au, 0x1bu, 0x1cu, 0x1du, 0x37u,
                0x3eu, 0x3fu, 0x4fu, 0x5fu, 0x27u, 0x2au, 0x50u, 0xc1u,
                0x5au, 0x8au, 0xc5u, 0xb5u, 0x57u, 0x71u, 0x76u, 0xaau,
                0xccu, 0x60u, 0x1eu, 0x09u, 0x22u, 0x26u, 0x95u, 0x94u,
                0x96u, 0x47u, 0x49u, 0x4au, 0x52u, 0x7cu, 0x7du, 0x7eu,
                0x7fu, 0x80u, 0xcbu, 0x74u, 0x0cu, 0x0du, 0x0eu, 0x33u,
                0x03u, 0x05u, 0x1cu, 0x42u, 0x00u, 0x37u, 0x3cu, 0x68u};
            const uint16_t back = (uint16_t)(0x8000u + (Random32() & 0x7ff0u));

            if (Random32() & 1u)
                g_bus.wram[text] = ops[Random32() & 63u];
            if (Random32() & 1u)
                g_bus.wram[(uint16_t)(text + 1u)] = (uint8_t)(Random32() & 0x0fu);
            g_bus.wram[0x1254u] = (Random32() & 1u) ? 0x7eu : 0x00u;
            g_bus.wram[0x1252u] = (uint8_t)back;
            g_bus.wram[0x1253u] = (uint8_t)(back >> 8);
            g_bus.wram[back] = ops[Random32() & 63u];
            if (Random32() & 1u)
                for (unsigned i = 0; i < 5u; ++i)
                    g_bus.wram[0x0622u + i] &= 0xf7u;
            if (Random32() & 1u)
                g_bus.wram[0x0581u] = 0;
            if (Random32() & 1u)
                g_bus.wram[0x0b62u] = 0;
        }
    }
}

/* Battle timers mostly on handler 1, queues mostly idle. */
static void Seed8DC5(CpuState *cpu) {
    SeedFieldChild(cpu);
    for (unsigned i = 0; i < 16u; ++i)
        if (Random32() & 3u)
            Poke16(g_bus.wram, 0x1a8fu + 6u * i, 0);
    for (unsigned slot = 0; slot < 8u; ++slot) {
        const uint16_t base = (uint16_t)(0x1b17u + 8u * slot);

        if (Random32() & 1u)
            g_bus.wram[base] = 0;
        g_bus.wram[base + 1u] = (uint8_t)(1u + (Random32() & 1u));
        g_bus.wram[base + 2u] = (Random32() & 7u)
            ? 1u : (uint8_t)(Random32() & 0x0fu);
    }
    if (Random32() & 7u)
        g_bus.wram[0x12e3u] |= 0x80u;
    g_bus.wram[0x1ff3u] = 0x83u;
    cpu->PB = 0x85;
}

/* World map NMI: small upload records, palette cycles. */
static void SeedCEF6(CpuState *cpu) {
    SeedFieldChild(cpu);
    g_bus.wram[0x1365u] = (Random32() & 7u) ? (uint8_t)(1u + (Random32() & 3u)) : 0;
    for (unsigned i = 0; i < 4u; ++i) {
        const uint16_t record = (uint16_t)(0x0200u + (Random32() & 0x0dffu));

        Poke16(g_bus.wram, 0x1367u + 2u * i, record);
        g_bus.wram[record] = (uint8_t)((Random32() & 0x80u) | (Random32() % 3u));
        g_bus.wram[(uint16_t)(record + 1u)] = (uint8_t)(Random32() & 0x3fu);
        g_bus.wram[(uint16_t)(record + 2u)] = (uint8_t)(1u + (Random32() & 3u));
    }
    g_bus.wram[0x16e7u] = (uint8_t)(Random32() & 3u);
    for (unsigned i = 0; i < 3u; ++i) {
        const uint16_t base = (uint16_t)(0x16e8u + 5u * i);
        const uint8_t length = (uint8_t)(1u + (Random32() & 15u));

        g_bus.wram[base + 1u] = length;
        g_bus.wram[base + 3u] = (uint8_t)(Random32() % (length + 1u));
        g_bus.wram[base + 2u] = (uint8_t)(Random32() & 3u);
    }
    g_bus.wram[0x1ff3u] = 0x83u;
    cpu->PB = 0x86;
}

/* VRAM queue with a free slot most of the time, X=0. */
static void SeedECDB(CpuState *cpu) {
    SeedFieldChild(cpu);
    for (unsigned slot = 0; slot < 16u; ++slot)
        if (!(Random32() % 12u))
            Poke16(g_bus.wram, 0x1a8fu + 6u * slot, 0);
    g_bus.wram[0x1ff3u] = 0x83u;
    cpu->PB = 0x85;
    cpu->x_flag = (Random32() & 7u) ? 0u : 1u;
    if (cpu->x_flag) {
        cpu->X &= 0x00ffu;
        cpu->Y &= 0x00ffu;
    }
    cpu_mirrors_to_p(cpu);
}

/* World map camera one step from the last streamed edge. */
static void Seed99BF(CpuState *cpu) {
    SeedFieldChild(cpu);
    if (Random32() & 1u)
        g_bus.wram[0x11f6u] = g_bus.wram[0x11f2u];
    if (Random32() & 1u)
        g_bus.wram[0x11f7u] = g_bus.wram[0x11f4u];
    cpu->PB = 0x86;
    cpu->x_flag = (Random32() & 7u) ? 0u : 1u;
    if (cpu->x_flag) {
        cpu->X &= 0x00ffu;
        cpu->Y &= 0x00ffu;
    }
    cpu_mirrors_to_p(cpu);
}

/* JSL targets entered with X=0 most of the time. */
static void SeedJslX16(CpuState *cpu, uint8_t bank) {
    SeedFieldChild(cpu);
    g_bus.wram[0x1ff3u] = 0x83u;
    cpu->PB = bank;
    cpu->x_flag = (Random32() & 7u) ? 0u : 1u;
    if (cpu->x_flag) {
        cpu->X &= 0x00ffu;
        cpu->Y &= 0x00ffu;
    }
    cpu_mirrors_to_p(cpu);
}

static void SeedE746(CpuState *cpu) {
    SeedJslX16(cpu, 0x82);
    g_bus.wram[(uint16_t)(cpu->D + 0x30u)] = (uint8_t)(Random32() % 9u);
}

static void Seed85DC(CpuState *cpu) {
    SeedJslX16(cpu, 0x83);
}

/* Forward-only event scripts of the native opcodes, in WRAM. */
static void SeedEventScripts(uint8_t *wram, uint32_t (*random)(void)) {
    static const uint8_t kOps[71] = {
        0x01u, 0x0cu, 0x08u, 0x09u, 0x0au, 0x0du, 0x71u,
        0x19u, 0x1eu, 0x2bu, 0x1bu, 0x1cu, 0x57u, 0x2fu,
        0x30u, 0x31u, 0x32u, 0x33u, 0x34u, 0x35u, 0x36u, 0x37u,
        0x38u, 0x39u, 0x3au, 0x3bu, 0x3cu, 0x3du, 0x3eu, 0x3fu, 0x40u,
        0x79u, 0x83u, 0x84u, 0x86u, 0x9du, 0x9eu, 0xa2u, 0xabu, 0xb5u,
        0xb8u, 0x5fu, 0x68u, 0x6bu, 0x58u, 0x24u, 0x25u, 0x29u,
        0xa9u, 0xaau, 0x55u, 0x69u, 0x85u,
        0x64u, 0x65u, 0x66u, 0x67u, 0x7cu, 0x7du, 0x7eu, 0x7fu, 0x80u,
        0x81u, 0xa3u, 0xa4u, 0xa5u, 0xa6u, 0xafu, 0xb0u, 0xb1u, 0xb2u};
    uint8_t script[256];
    uint16_t starts[48];
    uint16_t words[96];
    uint8_t owner[96];
    unsigned n = 0, len = 0, patches = 0;
    const uint8_t bank = (random() & 1u) ? 0x7eu : 0x7fu;
    const uint16_t base = bank == 0x7eu && !(random() & 7u)
        ? (uint16_t)(0xff00u + (random() & 0xf0u))
        : (uint16_t)(0x9000u + (random() & 0x3fffu));
    const uint32_t map_0692 = bank == 0x7eu ? 0x0692u : 0x10692u;

    while (len < 220u && n < 47u) {
        const unsigned roll = random() % 32u;
        uint8_t op;

        starts[n++] = (uint16_t)len;
        if (roll < 2u) {
            script[len++] = 0x00u;
            continue;
        }
        if (roll < 4u) {
            script[len++] = 0x11u;
            script[len++] = (uint8_t)random();
            continue;
        }
        if (roll < 5u) {
            script[len++] = (uint8_t)random();
            continue;
        }
        op = kOps[random() % 71u];
        script[len++] = op;
        switch (op) {
        case 0x01u: case 0x0cu: case 0x08u: case 0x09u:
            script[len++] = (random() & 3u) ? (uint8_t)random()
                                            : (uint8_t)(0xfbu + random() % 5u);
            break;
        case 0x0du: case 0x71u:
            script[len++] = (random() & 1u) ? wram[map_0692]
                                            : (uint8_t)random();
            break;
        case 0x1bu: case 0x1cu:
            script[len++] = (uint8_t)random();
            script[len++] = (uint8_t)random();
            break;
        case 0x0au: case 0x19u: case 0x1eu: case 0x2bu: case 0x57u:
        case 0x86u: case 0xa2u: case 0xabu: case 0xb5u: case 0xb8u:
            break;
        case 0x79u:
            script[len++] = (uint8_t)random();
            break;
        case 0x6bu: case 0x29u: case 0xaau:
            break;
        case 0xa9u:
            /* Up to three arguments, $FF, then the target word. */
            for (unsigned a = random() % 4u; a > 0u; --a)
                script[len++] = (random() & 3u) ? (uint8_t)(random() % 0xfbu)
                                                : (uint8_t)(0xfbu + random() % 4u);
            script[len++] = 0xffu;
            break;
        case 0x24u:
            /* Tile x, tile y, spawn id. */
            script[len++] = (uint8_t)(random() & 0x7fu);
            script[len++] = (uint8_t)(random() & 0x7fu);
            script[len++] = (uint8_t)random();
            break;
        case 0x55u: case 0x69u: {
            /* Point, then own slot, point, entity or actor. */
            static const uint8_t kArea[4] = {0xfbu, 0xe0u, 0x60u, 0x20u};
            const unsigned kind = random() & 3u;

            script[len++] = (op == 0x69u && !(random() & 3u))
                ? (uint8_t)(0xa0u + (random() & 0x1fu))
                : (uint8_t)(0xe0u + (random() & 0x1fu));
            script[len++] = kind == 0u ? 0xfbu
                : (uint8_t)(kArea[kind] + ((random() & 15u) ? (random() & (kind == 1u ? 0x1fu : 7u)) : (random() & 0x1fu)));
            break;
        }
        case 0x64u: case 0x65u: case 0x66u: case 0x67u:
        case 0x7cu: case 0x7du: case 0x7eu: case 0x7fu: case 0x80u:
        case 0x81u: case 0xa3u: case 0xa4u: case 0xa5u: case 0xa6u:
        case 0xafu: case 0xb0u: case 0xb1u: case 0xb2u: {
            /* Target (mostly a point), object or actor, bytes. */
            const unsigned tail = op >= 0xafu ? 1u
                : (op <= 0x67u || op == 0x81u) ? 1u : 2u;

            script[len++] = (random() & 3u) ? (uint8_t)(0xe0u + (random() & 0x1fu))
                : (random() & 1u) ? 0xfbu : (uint8_t)(0x20u + (random() & 7u));
            script[len++] = op >= 0xafu ? (uint8_t)(random() % 0x28u)
                                        : (uint8_t)(random() & 7u);
            for (unsigned b = 0; b < tail; ++b)
                script[len++] = (uint8_t)random();
            break;
        }
        case 0x85u:
            script[len++] = (random() & 1u) ? (uint8_t)(0x20u + (random() & 7u))
                                            : (uint8_t)(0xe0u + (random() & 0x1fu));
            script[len++] = (uint8_t)random();
            break;
        case 0x25u:
            /* Own position, a point, rarely an actor; spawn id. */
            script[len++] = (random() & 3u) == 0 ? 0xfbu
                : (random() & 15u) ? (uint8_t)(0xe0u + (random() & 0x1fu))
                                   : (uint8_t)random();
            script[len++] = (uint8_t)random();
            break;
        case 0x5fu: case 0x68u:
            /* Actor slot, or a variable $A0-$BF. */
            script[len++] = (random() & 3u) ? (uint8_t)(random() % 0x28u)
                                            : (uint8_t)(0xa0u + (random() & 0x1fu));
            break;
        case 0x58u:
            /* Layer, dx, dy, x speed, y speed. */
            script[len++] = (uint8_t)(random() & 3u);
            for (unsigned b = 0; b < 4u; ++b)
                script[len++] = (uint8_t)random();
            break;
        case 0x83u: case 0x84u:
            /* Point $E0-$FF, then an offset. */
            script[len++] = (random() & 7u) ? (uint8_t)(0xe0u + (random() & 0x1fu))
                                            : (uint8_t)random();
            script[len++] = (uint8_t)random();
            break;
        case 0x9du: case 0x9eu:
            /* Variable, point value operand, point offset. */
            script[len++] = (uint8_t)(random() & 0x3fu);
            script[len++] = (random() & 7u) ? (uint8_t)(0xe0u + (random() & 0x1fu))
                                            : (uint8_t)random();
            script[len++] = (uint8_t)(random() & 0x1fu);
            break;
        default: {
            /* $2F-$40: variable, then a byte or a value operand. */
            const unsigned roll = random() % 8u;

            script[len++] = (random() & 3u)
                ? (uint8_t)(random() & 0x3fu)
                : (uint8_t)(0xfbu + random() % 5u);
            if (op == 0x32u || op == 0x33u)
                break;
            script[len++] = roll < 2u ? (uint8_t)(0xa0u + (random() & 0x1fu))
                : roll < 3u ? 0xfbu
                : roll < 4u ? (uint8_t)(0xfcu + random() % 4u)
                : roll < 5u ? (uint8_t)(0xc0u + (random() & 0x3fu))
                            : (uint8_t)random();
            break;
        }
        }
        if (op == 0x08u || op == 0x09u || op == 0x1bu || op == 0x1cu ||
            op == 0x57u || (op >= 0x2fu && op <= 0x34u) ||
            (op >= 0x79u && op != 0xa9u) ||
            op == 0x5fu || op == 0x68u || op == 0x6bu || op == 0x58u ||
            op == 0x24u || op == 0x25u || op == 0x29u || op == 0xaau ||
            op == 0x55u || op == 0x69u || op == 0x85u ||
            (op >= 0x64u && op <= 0x67u))
            continue;
        for (unsigned w = op == 0x1eu ? 2u : 1u; w > 0u; --w) {
            owner[patches] = (uint8_t)(n - 1u);
            words[patches++] = (uint16_t)len;
            len += 2u;
        }
    }
    starts[n] = (uint16_t)len;
    script[len++] = 0x00u;
    /* Gotos jump to a later opcode, rarely anywhere. */
    for (unsigned p = 0; p < patches; ++p) {
        const unsigned later = owner[p] + 1u + random() % (n - owner[p]);
        const uint16_t target = (random() % 64u)
            ? starts[later] : (uint16_t)random();

        script[words[p]] = (uint8_t)target;
        script[words[p] + 1u] = (uint8_t)(target >> 8);
    }
    for (unsigned i = 0; i < len; ++i) {
        uint32_t at = (uint32_t)base + i;
        uint32_t offset = bank == 0x7eu ? 0u : 0x10000u;

        if (at > 0xffffu) {
            at = 0x8000u + (at - 0x10000u);
            offset += 0x10000u;
        }
        if (offset < 0x20000u)
            wram[offset + at] = script[i];
    }
    /* Map actor list ($7E:F022, stride 3) and entity list ($F024,
       stride 5) with keys 0-7 and an $FF end, rarely left random. */
    for (unsigned l = 0; l < 3u; ++l) {
        const unsigned stride = l == 2u ? 10u : l ? 5u : 3u;
        const uint16_t list = (uint16_t)(0x0100u + l * 0x80u + (random() & 0x1fu));
        const unsigned entries = random() % 8u;
        uint16_t at = (uint16_t)(0xf000u + list);

        const uint16_t head = l == 2u ? 0xf016u : (uint16_t)(0xf022u + 2u * l);

        wram[head] = (uint8_t)list;
        wram[head + 1u] = (uint8_t)(list >> 8);
        for (unsigned e = 0; e < entries; ++e, at = (uint16_t)(at + stride)) {
            wram[at] = (uint8_t)(random() & 7u);
            for (unsigned b = 1; b < stride; ++b)
                wram[(uint16_t)(at + b)] = (uint8_t)random();
        }
        if (random() & 15u)
            wram[at] = 0xffu;
    }
    wram[0x1d194u] = (uint8_t)base;
    wram[0x1d195u] = (uint8_t)(base >> 8);
    wram[0x1d196u] = bank;
    if (random() & 1u)
        wram[0x11273u] = 0;
    for (unsigned t = 0; t < 8u; ++t) {
        uint32_t at = (uint32_t)base + starts[random() % n];
        uint8_t at_bank = bank;

        if (!(random() % 8u))
            continue;
        if (at > 0xffffu) {
            at = 0x8000u + (at - 0x10000u);
            ++at_bank;
        }
        wram[0x1d134u + 3u * t] = (uint8_t)at;
        wram[0x1d135u + 3u * t] = (uint8_t)(at >> 8);
        wram[0x1d136u + 3u * t] = at_bank;
    }
}

/* Event slot timers; DB $C0 reads $1273 from ROM. */
static void SeedCBAE(CpuState *cpu) {
    static const uint8_t banks[8] = {
        0x83u, 0x83u, 0x80u, 0x7eu, 0x7fu, 0x00u, 0x8eu, 0xc0u};
    const unsigned mode = Random32() & 7u;

    SeedJslX16(cpu, 0x80);
    for (unsigned t = 0; t < 8u; ++t) {
        const unsigned roll = Random32() % 16u;

        g_bus.wram[0x1d18cu + t] = roll < 6u ? (uint8_t)(Random32() & 0x7fu)
            : roll < 11u ? (uint8_t)(0x82u + Random32() % 0x7eu)
            : roll < 13u ? 0x80u : 0x81u;
    }
    if (mode < 2u)
        for (unsigned t = 0; t < 8u; ++t)
            g_bus.wram[0x1d18cu + t] &= 0x7fu;
    SeedEventScripts(g_bus.wram, Random32);
    cpu->DB = banks[Random32() & 7u];
}

/* Menu routines in bank 82. */
static void SeedMenu(CpuState *cpu) {
    SeedJslX16(cpu, 0x82);
}

/* HDMA tables, window rows and upload flags in range. */
static void SeedNmiTables(uint16_t dp) {
    const uint16_t source = (uint16_t)(0x2000u + (Random32() & 0x0ff0u));
    const uint16_t target = (uint16_t)(0x4000u + (Random32() & 0x0ff0u));
    const unsigned rows = Random32() & 7u;

    g_bus.wram[(uint16_t)(dp + 0xf2u)] = (uint8_t)Random32();
    g_bus.wram[(uint16_t)(dp + 0xf3u)] = (Random32() & 7u)
        ? (uint8_t)((Random32() & 7u) << 4) : (uint8_t)Random32();
    g_bus.wram[(uint16_t)(dp + 0xf4u)] = (uint8_t)source;
    g_bus.wram[(uint16_t)(dp + 0xf5u)] = (uint8_t)(source >> 8);
    g_bus.wram[(uint16_t)(dp + 0xf6u)] = 0x7eu;
    g_bus.wram[(uint16_t)(dp + 0xf7u)] = (uint8_t)target;
    g_bus.wram[(uint16_t)(dp + 0xf8u)] = (uint8_t)(target >> 8);
    g_bus.wram[(uint16_t)(dp + 0xf9u)] = 0x7eu;
    for (unsigned i = 0; i < rows; ++i)
        g_bus.wram[source + 3u * i] |= 0x01u;
    g_bus.wram[source + 3u * rows] = 0;
    g_bus.wram[0x1530u] = (uint8_t)(1u + (Random32() & 7u));
    g_bus.wram[0x1531u] = (Random32() & 15u) ? 0 : (uint8_t)(Random32() & 1u);
    g_bus.wram[0x1539u] = (uint8_t)(Random32() & 7u);
    g_bus.wram[0x153bu] = (uint8_t)(Random32() & 7u);
    for (unsigned t = 0; t < 3u; ++t) {
        const uint16_t table = (uint16_t)(0x80c0u + 0x100u * t);
        const unsigned count = Random32() & 7u;

        for (unsigned i = 0; i < count; ++i)
            g_bus.wram[table + 3u * i] |= 0x01u;
        g_bus.wram[table + 3u * count] = 0;
    }
    if (Random32() & 3u)
        g_bus.wram[0x1565u] &= 0x07u;
    if (Random32() & 3u)
        g_bus.wram[0x1566u] = (uint8_t)(1u << (Random32() % 5u));
    if (Random32() & 3u)
        g_bus.wram[0x1568u] &= (uint8_t)~0x03u;
}

static void SeedMenuNmiBridge(CpuState *cpu) {
    SeedFieldChild(cpu);
    SeedNmiTables(cpu->D);
    if (!(Random32() & 3u))
        memset(g_bus.wram + 0x11e8u, 0, 0x20u);
    g_bus.wram[0x1ff3u] = 0x83u;
    cpu->PB = 0x82;
    if (Random32() & 1u)
        g_bus.wram[0x1565u] = g_bus.wram[0x1566u] =
            g_bus.wram[0x1567u] = 0;
}

/* Battle script of known opcodes, forward jumps, then an end. */
static void SeedBattleScriptWram(uint16_t dp) {
    static const uint8_t ops[52] = {
        0x03u, 0x05u, 0x06u, 0x07u, 0x0au, 0x0bu, 0x0cu, 0x0du,
        0x0eu, 0x16u, 0x17u, 0x18u, 0x19u, 0x1au, 0x1bu, 0x1cu,
        0x1fu, 0x20u, 0x42u, 0x43u, 0x0cu, 0x06u, 0x0fu, 0x21u,
        0x04u, 0x08u, 0x09u, 0x23u, 0x24u, 0x25u, 0x26u, 0x27u,
        0x28u, 0x29u, 0x2au, 0x2bu, 0x2eu, 0x2fu, 0x30u, 0x35u,
        0x36u, 0x3eu, 0x41u, 0x56u, 0x10u, 0x11u, 0x12u, 0x22u,
        0x37u, 0x3cu, 0x4du, 0x13u};
    static const uint8_t sizes[0x57] = {
        [0x10] = 5, [0x11] = 3, [0x12] = 5, [0x13] = 3, [0x22] = 6,
        [0x37] = 1, [0x3c] = 1, [0x4d] = 1,
        [0x03] = 3, [0x04] = 3, [0x05] = 4, [0x06] = 6, [0x07] = 6,
        [0x08] = 6, [0x09] = 6, [0x0a] = 6, [0x0b] = 6, [0x0c] = 4,
        [0x0d] = 5, [0x0e] = 5, [0x0f] = 5, [0x16] = 5, [0x17] = 5,
        [0x18] = 5, [0x19] = 3, [0x1a] = 3, [0x1b] = 3, [0x1c] = 2,
        [0x1f] = 3, [0x20] = 3, [0x21] = 6, [0x23] = 3, [0x24] = 5,
        [0x25] = 3, [0x26] = 3, [0x27] = 3, [0x28] = 1, [0x29] = 1,
        [0x2a] = 1, [0x2b] = 3, [0x2e] = 1, [0x2f] = 2, [0x30] = 2,
        [0x35] = 2, [0x36] = 2, [0x3e] = 2, [0x41] = 1, [0x42] = 3,
        [0x43] = 1, [0x56] = 3};
    const uint16_t base = (uint16_t)(0x2000u + (Random32() & 0x1ff0u));
    unsigned at = 0;

    for (unsigned i = 0; i < 128u; ++i)
        g_bus.wram[base + i] = (uint8_t)Random32();
    for (;;) {
        const uint8_t op = ops[Random32() % 52u];
        const unsigned size = sizes[op];

        if (at + size > 120u)
            break;
        g_bus.wram[base + at] = op;
        if (op >= 0x03u && op <= 0x0bu) {
            const uint16_t jump = (Random32() & 7u) ? 120u : (uint16_t)(Random32() % 120u);

            g_bus.wram[base + at + size - 2u] = (uint8_t)jump;
            g_bus.wram[base + at + size - 1u] = (uint8_t)(jump >> 8);
        }
        if (op >= 0x06u && op <= 0x0bu && (Random32() & 1u)) {
            const uint8_t n = (uint8_t)(Random32() & 0x7fu);
            const uint16_t value = (uint16_t)(Random32() & 0x7fffu);
            const uint16_t near = (uint16_t)(value + Random32() % 3u - 1u);

            g_bus.wram[base + at + 1u] = n;
            g_bus.wram[base + at + 2u] = (uint8_t)value;
            g_bus.wram[base + at + 3u] = (uint8_t)(value >> 8);
            g_bus.wram[0x1f40eu + 2u * n] = (uint8_t)near;
            g_bus.wram[0x1f40fu + 2u * n] = (uint8_t)(near >> 8);
        }
        if (op == 0x42u)
            g_bus.wram[base + at + 2u] = 0;
        at += size;
    }
    while (at < 120u)
        g_bus.wram[base + at++] = 0x00u;
    g_bus.wram[base + 120u] = (Random32() & 1u) ? 0x00u : 0x4fu;
    g_bus.wram[(uint16_t)(dp + 0xbbu)] = (uint8_t)base;
    g_bus.wram[(uint16_t)(dp + 0xbcu)] = (uint8_t)(base >> 8);
    g_bus.wram[(uint16_t)(dp + 0xbdu)] = 0x7eu;
    g_bus.wram[0x0a42u] = (uint8_t)base;
    g_bus.wram[0x0a43u] = (uint8_t)(base >> 8);
    g_bus.wram[0x0a44u] = 0x7eu;
    for (unsigned i = 0x0a64u; i < 0x0c00u; i += 2u)
        g_bus.wram[i + 1u] &= 0x07u;
    if (Random32() & 1u)
        g_bus.wram[0x1f462u] = g_bus.wram[0x1f463u] = 0;
}

static void SeedBattleScriptBridge(CpuState *cpu) {
    SeedFieldChild(cpu);
    SeedBattleScriptWram(cpu->D);
    g_bus.wram[0x1ff3u] = 0x83u;
    cpu->PB = 0x85;
}

static void SeedIntroNmiBridge(CpuState *cpu) {
    const uint16_t dp = cpu->D;
    static const uint8_t timers[8] = {
        0x00u, 0x01u, 0x1fu, 0x20u, 0x77u, 0x78u, 0x80u, 0xffu};

    g_bus.wram[(uint16_t)(dp + 0x50u)] = (Random32() & 7u)
        ? (uint8_t)(Random32() % 7u) : (uint8_t)Random32();
    if (Random32() & 1u)
        g_bus.wram[(uint16_t)(dp + 0x4eu)] = timers[Random32() & 7u];
}

static void SeedIntroBridge(CpuState *cpu) {
    SeedFieldChild(cpu);
    SeedIntroNmiBridge(cpu);
    g_bus.wram[0x1ff3u] = 0x83u;
    cpu->PB = 0x80;
}

static void SeedSelectNmiBridge(CpuState *cpu) {
    SeedFieldChild(cpu);
    SeedNmiTables(cpu->D);
    g_bus.wram[0x1ff3u] = 0x83u;
    cpu->PB = 0x86;
}

/* Field rectangle checks, DB $7E like the caller. */
static void SeedRects(CpuState *cpu) {
    SeedJslX16(cpu, 0x83);
    for (unsigned list = 0; list < 4u; ++list) {
        static const uint16_t heads[4] = {2u, 10u, 12u, 6u};
        static const uint8_t strides[4] = {15u, 5u, 5u, 9u};
        const uint16_t base = (uint16_t)(0x0100u + 0x0200u * list);

        Poke16(g_bus.wram, 0xf000u + heads[list], base);
        g_bus.wram[0xf000u + base + strides[list] * (Random32() & 3u)] = 0xffu;
    }
    cpu->DB = 0x7e;
}

/* Fade active half the time. */
static void Seed86C1(CpuState *cpu) {
    SeedJslX16(cpu, 0x80);
    if (Random32() & 1u)
        g_bus.wram[0x0581u] |= 0x80u;
}

/* World map region index in range, DB $86. */
static void Seed9EDD(CpuState *cpu) {
    SeedJslX16(cpu, 0x86);
    Poke16(g_bus.wram, 0x09ebu, (uint16_t)((Random32() % 16u) * 2u));
    cpu->DB = 0x86;
}

/* Battle records, small party blocks, X=0 like the battle loop. */
static void SeedBattleFrame(CpuState *cpu) {
    static const uint8_t layouts[8] = {1, 1, 1, 2, 2, 2, 0, 3};
    static const uint8_t banks[4] = {0x81u, 0x85u, 0x00u, 0x7eu};

    SeedFieldChild(cpu);
    g_bus.wram[0x15abu] = (Random32() & 15u) ? layouts[Random32() & 7u]
                                             : (uint8_t)Random32();
    if (Random32() & 1u)
        g_bus.wram[0x11deu] = 0;
    if (Random32() & 1u)
        g_bus.wram[0x125fu] = 0;
    g_bus.wram[0x153cu] = (uint8_t)(Random32() % 9u);
    g_bus.wram[0x154eu] = (uint8_t)(Random32() % 7u);
    Poke16(g_bus.wram, 0x15c8u, (uint16_t)(0x4800u + (Random32() & 0x03ffu)));
    Poke16(g_bus.wram, 0x15ccu, (uint16_t)(0x4800u + (Random32() & 0x03ffu)));
    Poke16(g_bus.wram, 0x15d4u, (uint16_t)(0x4800u + (Random32() & 0x03ffu)));
    for (unsigned i = 0; i < 11u; ++i)
        Poke16(g_bus.wram, 0x0a64u + 2u * i, (Random32() & 3u)
            ? (uint16_t)(0x0800u + (Random32() & 0x0fffu)) : 0);
    for (unsigned i = 0; i < 6u; ++i) {
        g_bus.wram[0x13e7u + 15u * i] = (uint8_t)(1u + (Random32() & 3u));
        g_bus.wram[0x13e8u + 15u * i] = (uint8_t)(1u + (Random32() & 3u));
    }
    g_bus.wram[0x1ff3u] = 0x83u;
    cpu->PB = 0x85;
    cpu->DB = banks[Random32() & 3u];
    cpu->x_flag = (Random32() & 7u) ? 0u : 1u;
    if (cpu->x_flag) {
        cpu->X &= 0x00ffu;
        cpu->Y &= 0x00ffu;
    }
    cpu_mirrors_to_p(cpu);
}

/* Camera layers for the field loop's JSL $8E:BD77. */
static void SeedBD77(CpuState *cpu) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[4] = {0x83u, 0x83u, 0x80u, 0x7eu};
    static const uint8_t modes[16] = {
        0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 4, 4, 0x80u, 0x85u, 0x05u};
    const uint16_t dp = dps[Random32() & 7u];

    RandomFill(g_bus.wram);
    for (unsigned layer = 0; layer < 6u; layer += 2u) {
        const uint16_t x = (Random32() & 7u)
            ? (uint16_t)((Random32() & 0x07f0u) |
                  ((Random32() & 3u) ? (1u + Random32() % 15u) : 0u))
            : (Random32() & 1u) ? (uint16_t)(0xeff0u | (Random32() & 0x0fu))
            : (uint16_t)Random32();
        const uint16_t y = (Random32() & 7u)
            ? (uint16_t)((Random32() & 0x07f0u) |
                  ((Random32() & 3u) ? (1u + Random32() % 15u) : 0u))
            : (Random32() & 1u) ? (uint16_t)(0xeff0u | (Random32() & 0x0fu))
            : (uint16_t)Random32();

        g_bus.wram[0x1d020u + layer] = modes[Random32() & 15u];
        Poke16(g_bus.wram, 0x121eu + layer, x);
        Poke16(g_bus.wram, 0x1226u + layer, y);
        if (Random32() & 1u) {
            Poke16(g_bus.wram, 0x1d0ceu + layer, x);
            Poke16(g_bus.wram, 0x1d0d6u + layer, y);
        }
        Poke16(g_bus.wram, 0x1d0deu + layer, (uint16_t)(Random32() % 9u));
        Poke16(g_bus.wram, 0x1d0e6u + layer, (uint16_t)(Random32() % 9u));
        /* Map size in cells; $0100 divides by zero. */
        Poke16(g_bus.wram, 0x1d010u + layer, (Random32() & 7u)
            ? (uint16_t)(Random32() & 0x7fu) : 0x0100u);
        Poke16(g_bus.wram, 0x1d018u + layer, (Random32() & 7u)
            ? (uint16_t)(Random32() & 0x7fu) : 0x0100u);
        if (Random32() & 1u) {
            Poke16(g_bus.wram, 0x05a4u, x);
            Poke16(g_bus.wram, 0x05a6u, y);
        }
    }
    if (Random32() & 1u)
        g_bus.wram[0x1261u] &= 0xf7u;
    if (Random32() & 1u)
        Poke16(g_bus.wram, 0x05a8u, 0);
    g_bus.wram[0x1ff1u] = (uint8_t)RETURN_WORD;
    g_bus.wram[0x1ff2u] = (uint8_t)(RETURN_WORD >> 8);
    g_bus.wram[0x1ff3u] = 0x83u;

    memset(cpu, 0, sizeof(*cpu));
    cpu->A = (uint16_t)Random32();
    cpu->X = (uint16_t)Random32();
    cpu->Y = (uint16_t)Random32();
    cpu->S = 0x1ff0u;
    cpu->D = dp;
    cpu->DB = banks[Random32() & 3u];
    cpu->PB = 0x8e;
    cpu->m_flag = 1;
    cpu->x_flag = (Random32() & 7u) ? 1u : 0u;
    cpu->_flag_C = Random32() & 1u;
    cpu->_flag_Z = Random32() & 1u;
    cpu->_flag_V = Random32() & 1u;
    cpu->_flag_N = Random32() & 1u;
    cpu->_flag_I = Random32() & 1u;
    cpu->ram = g_bus.wram;
    if (cpu->x_flag) {
        cpu->X &= 0x00ffu;
        cpu->Y &= 0x00ffu;
    }
    cpu_mirrors_to_p(cpu);
}

typedef Lufia2ExecutionResult (*WholeDecomp)(
    const Lufia2Memory *memory, Lufia2CpuState *cpu);

typedef struct WholeTarget {
    const char *name;
    uint32_t entry;
    uint32_t dispatch_pc;
    void (*seed)(CpuState *cpu);
    WholeDecomp decomp;
    RecompReturn (*bridge)(CpuState *cpu);
    uint8_t frame;
    unsigned limit;
} WholeTarget;

static const WholeTarget kWholeTargets[] = {
    {"C7F8", 0x83c7f8u, 0x83c864u, SeedC7F8, Lufia2ActorPrimaryUpdate,
     Lufia2DecompBridge_C7F8, 2, 4000000u},
    {"D508", 0x83d508u, 0x83d5d1u, SeedD508, Lufia2ActorSecondaryUpdate,
     Lufia2DecompBridge_D508, 2, 4000000u},
    {"C1B4", 0x83c1b4u, 0u, SeedC1B4, Lufia2PlayerSlotStandardUpdate,
     Lufia2DecompBridge_C1B4, 2, 4000000u},
    {"BB93", 0x83bb93u, 0x83bba5u, SeedBB93, DecompBB93,
     Lufia2DecompBridge_BB93, 3, 100000000u},
    {"81C6", 0x8381c6u, 0u, Seed81C6, Lufia2FieldTriggerUpdate,
     Lufia2DecompBridge_81C6, 2, 4000000u},
    {"E03E", 0x83e03eu, 0x83e10fu, SeedE03E, Lufia2ObjectSlotsUpdate,
     Lufia2DecompBridge_E03E, 2, 64000000u},
    {"9FA9", 0x839fa9u, 0u, Seed9FA9, Lufia2FieldNmiUploads,
     Lufia2DecompBridge_9FA9, 3, 4000000u},
    {"BD77", 0x8ebd77u, 0x8ebdd7u, SeedBD77, Lufia2FieldScrollUpdate,
     Lufia2DecompBridge_BD77, 3, 4000000u},
    {"80CD", 0x8380cdu, 0u, SeedFieldChild, Lufia2FieldIdleTest,
     Lufia2DecompBridge_80CD, 2, 4000000u},
    {"8682", 0x838682u, 0u, SeedFieldChild, Lufia2FieldAnimationTicks,
     Lufia2DecompBridge_8682, 2, 4000000u},
    {"AEB5", 0x83aeb5u, 0x83af11u, SeedAEB5, Lufia2FieldColourEffects,
     Lufia2DecompBridge_AEB5, 2, 4000000u},
    {"9C72", 0x809c72u, 0u, Seed9C72, Lufia2FieldEventTick,
     Lufia2DecompBridge_9C72, 3, 4000000u},
    {"8DC5", 0x858dc5u, 0u, Seed8DC5, Lufia2BattleNmiUploads,
     Lufia2DecompBridge_8DC5, 3, 4000000u},
    {"CEF6", 0x86cef6u, 0u, SeedCEF6, Lufia2WorldMapNmiUploads,
     Lufia2DecompBridge_CEF6, 3, 4000000u},
    {"ECDB", 0x85ecdbu, 0u, SeedECDB, Lufia2BattleVramQueueSlot,
     Lufia2DecompBridge_ECDB, 3, 4000000u},
    {"99BF", 0x8699bfu, 0u, Seed99BF, Lufia2WorldMapStreamEdges,
     Lufia2DecompBridge_99BF, 2, 4000000u},
    {"83A0", 0x8383a0u, 0u, SeedFieldChild, Lufia2FieldMenuRequest,
     Lufia2DecompBridge_83A0, 2, 4000000u},
    {"867B", 0x83867bu, 0u, SeedFieldChild, Lufia2FieldTakeButtons,
     Lufia2DecompBridge_867B, 2, 4000000u},
    {"8103", 0x838103u, 0u, SeedFieldChild, Lufia2FieldStatusRequests,
     Lufia2DecompBridge_8103, 2, 4000000u},
    {"E746", 0x82e746u, 0u, SeedE746, Lufia2TitleStateDispatch,
     Lufia2DecompBridge_E746, 3, 4000000u},
    {"85DC", 0x8385dcu, 0u, Seed85DC, Lufia2FieldReloadSetup,
     Lufia2DecompBridge_85DC, 3, 4000000u},
    {"939C", 0x82939cu, 0u, SeedMenuNmiBridge, Lufia2MenuNmi,
     Lufia2DecompBridge_939C, 3, 4000000u},
    {"81A9", 0x8681a9u, 0u, SeedSelectNmiBridge, Lufia2SelectScreenNmi,
     Lufia2DecompBridge_81A9, 3, 4000000u},
    {"92A4", 0x8092a4u, 0u, SeedIntroBridge, Lufia2IntroNmi,
     Lufia2DecompBridge_92A4, 3, 4000000u},
    {"B452", 0x85b452u, 0u, SeedBattleScriptBridge, Lufia2BattleScript,
     Lufia2DecompBridge_B452, 3, 4000000u},
    {"8B4B", 0x828b4bu, 0u, SeedMenu, Lufia2MenuButtons,
     Lufia2DecompBridge_8B4B, 3, 4000000u},
    {"9313", 0x829313u, 0u, SeedMenu, Lufia2MenuWindowRequest,
     Lufia2DecompBridge_9313, 2, 4000000u},
    {"C627", 0x82c627u, 0u, SeedMenu, Lufia2MenuCursorBlink,
     Lufia2DecompBridge_C627, 2, 4000000u},
    {"B66E", 0x83b66eu, 0u, SeedRects, Lufia2FieldStairRects,
     Lufia2DecompBridge_B66E, 2, 4000000u},
    {"B711", 0x83b711u, 0u, SeedRects, Lufia2FieldEventRects,
     Lufia2DecompBridge_B711, 2, 4000000u},
    {"B747", 0x83b747u, 0u, SeedRects, Lufia2FieldAreaRects,
     Lufia2DecompBridge_B747, 2, 4000000u},
    {"86C1", 0x8086c1u, 0u, Seed86C1, Lufia2ScreenFade,
     Lufia2DecompBridge_86C1, 2, 4000000u},
    {"9EDD", 0x869eddu, 0u, Seed9EDD, Lufia2WorldMapRegionSearch,
     Lufia2DecompBridge_9EDD, 2, 4000000u},
    {"9CB8", 0x809cb8u, 0u, Seed9CB8, Lufia2TextEngineStep,
     Lufia2DecompBridge_9CB8, 3, 4000000u},
    {"8A2F", 0x858a2fu, 0u, SeedBattleFrame, Lufia2BattleSprites,
     Lufia2DecompBridge_8A2F, 3, 4000000u},
    {"ECF0", 0x85ecf0u, 0u, SeedBattleFrame, Lufia2BattleFrameUpkeep,
     Lufia2DecompBridge_ECF0, 3, 4000000u},
    {"CBAE", 0x80cbaeu, 0u, SeedCBAE, Lufia2FieldEventTimerTick,
     Lufia2DecompBridge_CBAE, 3, 4000000u},
};

enum { WHOLE_TARGETS = sizeof(kWholeTargets) / sizeof(kWholeTargets[0]) };

/* Multiplier latches carry across runs. */
typedef struct BusRegisters {
    uint8_t multiply_a;
    uint8_t multiply_b;
    uint16_t multiply_result;
    uint16_t divide_a;
    uint16_t divide_result;
    uint8_t m7_latch;
    uint16_t m7_a;
    uint8_t m7_b;
    uint32_t wm_address;
} BusRegisters;

static BusRegisters SaveRegisters(void) {
    BusRegisters saved;
    saved.multiply_a = g_bus.multiply_a;
    saved.multiply_b = g_bus.multiply_b;
    saved.multiply_result = g_bus.multiply_result;
    saved.divide_a = g_bus.divide_a;
    saved.divide_result = g_bus.divide_result;
    saved.m7_latch = g_bus.m7_latch;
    saved.m7_a = g_bus.m7_a;
    saved.m7_b = g_bus.m7_b;
    saved.wm_address = g_bus.wm_address;
    return saved;
}

static void RestoreRegisters(const BusRegisters *saved) {
    g_bus.multiply_a = saved->multiply_a;
    g_bus.multiply_b = saved->multiply_b;
    g_bus.multiply_result = saved->multiply_result;
    g_bus.divide_a = saved->divide_a;
    g_bus.divide_result = saved->divide_result;
    g_bus.m7_latch = saved->m7_latch;
    g_bus.m7_a = saved->m7_a;
    g_bus.m7_b = saved->m7_b;
    g_bus.wm_address = saved->wm_address;
}

static bool RunWholeCase(
    Interp816 *ref, const WholeTarget *target, unsigned index,
    unsigned hrv_mode, WholeStats *stats) {
    /* RTS stays in the entry bank; seeds push bank 83 for RTL. */
    const uint32_t sentinel =
        (target->frame == 2 ? target->entry & 0xff0000u : 0x830000u) |
        (uint16_t)(RETURN_WORD + 1u);
    const Lufia2Memory front = {FrontRead, FrontWrite, NULL};
    CpuState native;
    CpuState input;
    Lufia2CpuState probe;
    Lufia2ExecutionResult expected;
    RecompReturn ret;
    unsigned instructions = 0;
    unsigned dispatches = 0;
    unsigned visits = 0;
    uint32_t last_pc = 0;
    BusRegisters registers;
    bool stopped = false;

    target->seed(&native);
    native.host_return_valid = (uint8_t)(
        hrv_mode == 0 ? target->frame : hrv_mode == 1 ? 0u :
        target->frame == 2 ? 3u : 2u);
    input = native;
    memcpy(g_seed, g_bus.wram, SNES_VERIFY_WRAM_SIZE);
    registers = SaveRegisters();
    g_child_stuck = false;
    g_bad_site = false;

    memset(&probe, 0, sizeof(probe));
    probe.accumulator = input.A;
    probe.x = input.X;
    probe.y = input.Y;
    probe.stack = input.S;
    probe.direct_page = input.D;
    probe.data_bank = input.DB;
    probe.program_bank = input.PB;
    probe.carry = input._flag_C;
    probe.zero = input._flag_Z;
    probe.negative = input._flag_N;
    probe.overflow = input._flag_V;
    probe.irq_disable = input._flag_I;
    probe.accumulator_is_8_bit = 1;
    probe.index_is_8_bit = input.x_flag;
    expected = target->decomp(&front, &probe);
    memcpy(g_bus.wram, g_seed, SNES_VERIFY_WRAM_SIZE);
    if (expected.flow == LUFIA2_EXECUTION_CHILD_UNWOUND) {
        /* Child stub unwinds SKIP_2; the bridge passes SKIP_1 up. */
        RestoreRegisters(&registers);
        memset(&g_stub, 0, sizeof(g_stub));
        ret = target->bridge(&native);
        memcpy(g_bus.wram, g_seed, SNES_VERIFY_WRAM_SIZE);
        if (ret != RECOMP_RETURN_SKIP_1 || g_stub.count != 0) {
            Report(target->name, "unwind", index, &native, ref, ret, 0,
                "child unwind");
            return false;
        }
        ++stats->unterminated;
        return true;
    }
    RestoreRegisters(&registers);

    memset(&g_stub, 0, sizeof(g_stub));
    ret = target->bridge(&native);
    memcpy(g_native, g_bus.wram, SNES_VERIFY_WRAM_SIZE);

    memcpy(g_bus.wram, g_seed, SNES_VERIFY_WRAM_SIZE);
    RestoreRegisters(&registers);
    InitReference(ref, &input, target->entry);
    while (instructions < target->limit) {
        const uint32_t pc = SnesVerifyPc24(ref);
        if (expected.flow == LUFIA2_EXECUTION_RETURNED) {
            if (pc == sentinel) {
                stopped = true;
                break;
            }
        } else if (pc == expected.pc && ref->sp == native.S &&
                   (target->dispatch_pc
                        ? dispatches == expected.dispatches
                        : visits++ == expected.dispatches)) {
            stopped = true;
            break;
        }
        if (pc == target->dispatch_pc)
            ++dispatches;
        last_pc = pc;
        interp816_runOpcode(ref);
        ++instructions;
    }

    if (!stopped || g_bad_site || !SameState(&native, ref) ||
        memcmp(g_native, g_bus.wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        Report(target->name, "state", index, &native, ref, ret, stopped ? 0 : -1,
            g_bad_site ? "child site" : "state/memory");
        return false;
    }
    if (expected.flow == LUFIA2_EXECUTION_BOUNDARY) {
        if (ret != (RecompReturn)STUB_SENTINEL || g_stub.count != 1 ||
            g_stub.kind != STUB_TAIL || g_stub.target != expected.pc ||
            g_stub.entry_s != input.S ||
            g_stub.hrv != input.host_return_valid) {
            Report(target->name, "boundary", index, &native, ref, ret, 0, "tail");
            return false;
        }
        ++stats->lle_boundary;
    } else if (hrv_mode == 0) {
        if (ret != RECOMP_RETURN_NORMAL || g_stub.count != 0) {
            Report(target->name, "paired", index, &native, ref, ret, 0, "return");
            return false;
        }
        ++stats->host_return;
    } else {
        if (ret != (RecompReturn)STUB_SENTINEL || g_stub.count != 1 ||
            g_stub.kind != STUB_DISPATCH || g_stub.target != sentinel ||
            g_stub.site != last_pc ||
            g_stub.restore_s != (uint16_t)(input.S + target->frame)) {
            Report(target->name, "dispatched", index, &native, ref, ret, 0,
                "dispatch");
            return false;
        }
        ++stats->dispatch_return;
    }
    return true;
}

static bool RunWholeUnsupportedCase(
    const WholeTarget *target, unsigned index, WholeStats *stats) {
    CpuState native;
    CpuState input;
    RecompReturn ret;

    target->seed(&native);
    switch (index % 3u) {
    case 0: native.m_flag = 0; break;
    case 1: native._flag_D = 1; break;
    default: native.emulation = 1; break;
    }
    cpu_mirrors_to_p(&native);
    native.host_return_valid = (uint8_t)((index & 1u) ? 2u : 0u);
    input = native;
    memcpy(g_seed, g_bus.wram, SNES_VERIFY_WRAM_SIZE);

    memset(&g_stub, 0, sizeof(g_stub));
    ret = target->bridge(&native);
    native.open_bus = input.open_bus;
    if (ret != (RecompReturn)STUB_SENTINEL || g_stub.count != 1 ||
        g_stub.kind != STUB_TAIL || g_stub.target != target->entry ||
        g_stub.site != target->entry || g_stub.entry_s != input.S ||
        g_stub.hrv != input.host_return_valid ||
        memcmp(&native, &input, sizeof(native)) != 0 ||
        memcmp(g_seed, g_bus.wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        fprintf(stderr, "FAIL %s unsupported case %u\n",
            target->name, index);
        return false;
    }
    ++stats->lle_entry;
    return true;
}

int main(int argc, char **argv) {
    const char *report_path = NULL;
    FILE *report = NULL;
    uint8_t *rom = NULL;
    size_t rom_size = 0;
    Interp816 *ref = NULL;
    unsigned passed[4][3] = {{0}};
    unsigned unsupported[4] = {0};
    unsigned fb12_oob = 0;
    unsigned whole_passed[WHOLE_TARGETS] = {0};
    WholeStats whole_stats[WHOLE_TARGETS];
    unsigned failed = 0;
    bool bus_ready = false;

    if (argc < 2) {
        fprintf(stderr, "usage: %s <lufia2.sfc> [--report path]\n", argv[0]);
        return 2;
    }
    for (int i = 2; i < argc; ++i) {
        if (!strcmp(argv[i], "--report") && i + 1 < argc)
            report_path = argv[++i];
        else {
            fprintf(stderr, "unknown argument: %s\n", argv[i]);
            return 2;
        }
    }

    if (!SnesVerifyLoadFile(argv[1], &rom, &rom_size) ||
        rom_size != LUFIA2_ROM_SIZE ||
        !SnesVerifyBusInit(&g_bus, rom, rom_size)) {
        fprintf(stderr, "ERROR: reference ROM/bus initialization failed\n");
        failed = 1;
        goto done;
    }
    bus_ready = true;
    ref = interp816_init(&g_bus, SnesVerifyBusRead, SnesVerifyBusWrite);
    g_child = interp816_init(&g_bus, SnesVerifyBusRead, SnesVerifyBusWrite);
    if (!ref) {
        fprintf(stderr, "ERROR: interp816 initialization failed\n");
        failed = 1;
        goto done;
    }

    for (unsigned t = 0; t < 4u && failed < 20; ++t) {
        for (unsigned mode = 0; mode < 3u && failed < 20; ++mode) {
            for (unsigned i = 0; i < CASES_PER_MODE && failed < 20; ++i) {
                if (RunBoundaryCase(ref, &kTargets[t], i, mode))
                    ++passed[t][mode];
                else
                    ++failed;
            }
        }
        for (unsigned i = 0; i < UNSUPPORTED_CASES && failed < 20; ++i) {
            const unsigned kinds = kTargets[t].allow_x8 ? 3u :
                                   kTargets[t].dp_page0 ? 5u : 4u;
            if (RunUnsupportedCase(&kTargets[t], i, i % kinds))
                ++unsupported[t];
            else
                ++failed;
        }
    }
    for (unsigned i = 0; i < UNSUPPORTED_CASES && failed < 20; ++i) {
        if (RunFb12OutOfRangeCase(ref, i))
            ++fb12_oob;
        else
            ++failed;
    }

    memset(whole_stats, 0, sizeof(whole_stats));
    for (unsigned t = 0; t < WHOLE_TARGETS && failed < 20; ++t) {
        const WholeTarget *target = &kWholeTargets[t];

        const unsigned cases = t == 3u ? WHOLE_CASES / 4u : WHOLE_CASES;

        for (unsigned i = 0; i < cases && failed < 20; ++i) {
            if (RunWholeCase(ref, target, i, i % 3u, &whole_stats[t]))
                ++whole_passed[t];
            else
                ++failed;
        }
        for (unsigned i = 0; i < UNSUPPORTED_CASES && failed < 20; ++i) {
            if (RunWholeUnsupportedCase(target, i, &whole_stats[t]))
                ++whole_passed[t];
            else
                ++failed;
        }
    }

    if (report_path)
        report = fopen(report_path, "wb");
    for (int pass = 0; pass < 2; ++pass) {
        FILE *out = pass ? report : stdout;
        if (!out)
            continue;
        fprintf(out, "Lufia II actor bridge verifier\n");
        fprintf(out, "==============================\n\n");
        for (unsigned t = 0; t < 4u; ++t)
            fprintf(out,
                "$83:%s paired %u/%u dispatched %u/%u mismatched %u/%u "
                "LLE-entry %u/%u\n",
                kTargets[t].name,
                passed[t][0], CASES_PER_MODE, passed[t][1], CASES_PER_MODE,
                passed[t][2], CASES_PER_MODE,
                unsupported[t], UNSUPPORTED_CASES);
        fprintf(out, "$83:FB12 out-of-range tail %u/%u\n",
            fb12_oob, UNSUPPORTED_CASES);
        for (unsigned t = 0; t < WHOLE_TARGETS; ++t)
            fprintf(out,
                "$%02X:%s %u/%u (host return %u, dispatch return %u, "
                "LLE boundary %u, LLE entry %u, child never returned %u)\n",
                kWholeTargets[t].entry >> 16, kWholeTargets[t].name,
                whole_passed[t],
                (t == 3u ? WHOLE_CASES / 4u : WHOLE_CASES) +
                    UNSUPPORTED_CASES,
                whole_stats[t].host_return, whole_stats[t].dispatch_return,
                whole_stats[t].lle_boundary, whole_stats[t].lle_entry,
                whole_stats[t].unterminated);
        fprintf(out, "failures: %u\n", failed);
        fprintf(out, failed
            ? "RESULT: FAIL - actor bridge mismatch found\n"
            : "RESULT: PASS - actor bridges matched original call boundaries\n");
    }

done:
    if (report)
        fclose(report);
    if (ref)
        interp816_free(ref);
    if (bus_ready)
        SnesVerifyBusDestroy(&g_bus);
    free(rom);
    return failed ? 1 : 0;
}
