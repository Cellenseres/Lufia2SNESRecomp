#include "patches/native_patches.h"

#include <stdio.h>
#include <string.h>

enum { EVENT_CAP = 64, WRAM_SIZE = 0x20000 };

typedef struct ReadEvent {
    uint32_t address;
    uint8_t value;
} ReadEvent;

typedef struct TestBus {
    uint8_t ram[WRAM_SIZE];
    ReadEvent event[EVENT_CAP];
    unsigned count;
    unsigned unsupported_reads;
    unsigned writes;
} TestBus;

static const uint8_t kEntry[] = {
    0xA6,0xA7, 0xBF,0x36,0x07,0x00, 0x89,0x80, 0xF0,0x27,
};
static const uint8_t kMiddle[] = {
    0xBD,0x22,0x06, 0x29,0x80, 0xD0,0x5A,
    0xAD,0xA7,0x09, 0x89,0x01, 0xF0,0x37,
    0x8A, 0xD0,0x08, 0xAF,0xFE,0xD0,0x7F, 0xF0,0x2E,
};
static const uint8_t kExitPath[] = {
    0xBD,0x36,0x07, 0x89,0x04, 0xF0,0x14,
};

static int ReadRomByte(uint16_t offset, uint8_t *value) {
    if (offset >= 0xD508u && offset < 0xD508u + sizeof(kEntry)) {
        *value = kEntry[offset - 0xD508u];
        return 1;
    }
    if (offset >= 0xD539u && offset < 0xD539u + sizeof(kMiddle)) {
        *value = kMiddle[offset - 0xD539u];
        return 1;
    }
    if (offset >= 0xD57Eu && offset < 0xD57Eu + sizeof(kExitPath)) {
        *value = kExitPath[offset - 0xD57Eu];
        return 1;
    }
    return 0;
}

static uint8_t ReadTest(void *opaque, uint32_t address) {
    TestBus *bus = (TestBus *)opaque;
    const uint8_t bank = (uint8_t)(address >> 16);
    const uint16_t offset = (uint16_t)address;
    uint8_t value = 0;
    if (bank == 0x83u && ReadRomByte(offset, &value)) {
        /* ROM byte supplied above. */
    } else if ((bank == 0 || bank == 0x83u) && offset < 0x2000u) {
        value = bus->ram[offset];
    } else if (bank == 0x7Eu || bank == 0x7Fu) {
        value = bus->ram[((unsigned)(bank - 0x7Eu) << 16) | offset];
    } else {
        ++bus->unsupported_reads;
    }
    if (bus->count < EVENT_CAP) {
        bus->event[bus->count].address = address & 0xFFFFFFu;
        bus->event[bus->count].value = value;
    }
    ++bus->count;
    return value;
}

static void WriteTest(void *opaque, uint32_t address, uint8_t value) {
    TestBus *bus = (TestBus *)opaque;
    const uint8_t bank = (uint8_t)(address >> 16);
    const uint16_t offset = (uint16_t)address;
    ++bus->writes;
    if ((bank == 0 || bank == 0x83u) && offset < 0x2000u)
        bus->ram[offset] = value;
    else if (bank == 0x7Eu || bank == 0x7Fu)
        bus->ram[((unsigned)(bank - 0x7Eu) << 16) | offset] = value;
}

static int SameState(const Interp816 *a, const Interp816 *b) {
#define SAME(field) do { if (a->field != b->field) return 0; } while (0)
    SAME(a); SAME(x); SAME(y); SAME(sp); SAME(pc); SAME(dp);
    SAME(k); SAME(db); SAME(c); SAME(z); SAME(v); SAME(n);
    SAME(i); SAME(d); SAME(xf); SAME(mf); SAME(e);
    SAME(irqWanted); SAME(nmiWanted); SAME(waiting); SAME(stopped);
    SAME(brkHookEnabled); SAME(cyclesUsed);
#undef SAME
    return 1;
}

static void InitCase(Interp816 *in, TestBus *bus, uint8_t actor_state,
                     uint8_t flags, uint8_t global, unsigned seed) {
    memset(in, 0, sizeof(*in));
    memset(bus, 0, sizeof(*bus));
    bus->ram[0x00A7u] = 0;
    bus->ram[0x0736u] = actor_state;
    bus->ram[0x0622u] = flags;
    bus->ram[0x09A7u] = global;
    bus->ram[0x1D0FEu] = 0;
    in->mem = bus;
    in->read = ReadTest;
    in->write = WriteTest;
    in->a = (uint16_t)(0xA500u | (seed & 0xFFu));
    in->x = (uint16_t)(0x5A00u | ((seed * 3u) & 0xFFu));
    in->y = (uint16_t)(0x3300u | ((seed * 5u) & 0xFFu));
    in->sp = (uint16_t)(0x1F00u | ((seed * 7u) & 0xFFu));
    in->pc = L2_ACTOR_D508_EARLY_RETURN_PC;
    in->k = 0x83u;
    in->db = 0x83u;
    in->c = (seed & 1u) != 0;
    in->z = (seed & 2u) != 0;
    in->v = (seed & 4u) != 0;
    in->n = (seed & 8u) != 0;
    in->i = (seed & 16u) != 0;
    in->d = (seed & 32u) != 0;
    in->mf = true;
    in->xf = true;
}

static int CheckPositiveCases(void) {
    static const uint8_t actor_states[] = {
        0x00,0x01,0x02,0x03,0x08,0x10,0x20,0x40,0x78,
    };
    static const uint8_t flags[] = {0x00,0x01,0x7F};
    static const uint8_t globals[] = {0x01,0x03,0x11,0xFF};
    static const unsigned seeds[] = {0,1,2,4,8,16,32,63};
    unsigned cases = 0;
    for (unsigned ai = 0; ai < sizeof(actor_states); ++ai) {
        for (unsigned fi = 0; fi < sizeof(flags); ++fi) {
            for (unsigned gi = 0; gi < sizeof(globals); ++gi) {
                for (unsigned si = 0; si < sizeof(seeds) / sizeof(seeds[0]); ++si) {
                    TestBus reference_bus, native_bus;
                    Interp816 reference, native;
                    InitCase(&reference, &reference_bus, actor_states[ai],
                             flags[fi], globals[gi], seeds[si]);
                    InitCase(&native, &native_bus, actor_states[ai],
                             flags[fi], globals[gi], seeds[si]);
                    CpuState cpu;
                    memset(&cpu, 0, sizeof(cpu));
                    cpu.ram = native_bus.ram;
                    if (!L2ActorD508EarlyReturnEligible(&cpu, &native, 1))
                        return 0;

                    unsigned reference_cycles = 0;
                    for (unsigned op = 0; op < L2_ACTOR_D508_EARLY_RETURN_OPCODES; ++op)
                        reference_cycles += (unsigned)interp816_runOpcode(&reference);
                    const unsigned native_cycles =
                        L2ActorD508EarlyReturnStep(&native);
                    ++cases;
                    if (reference.pc != L2_ACTOR_D508_EARLY_RETURN_NEXT_PC ||
                        reference_cycles != L2_ACTOR_D508_EARLY_RETURN_CYCLES ||
                        native_cycles != reference_cycles ||
                        !SameState(&reference, &native) ||
                        reference_bus.unsupported_reads != 0 ||
                        native_bus.unsupported_reads != 0 ||
                        reference_bus.writes != 0 || native_bus.writes != 0 ||
                        reference_bus.count != native_bus.count ||
                        reference_bus.count != 46u ||
                        memcmp(reference_bus.event, native_bus.event,
                               sizeof(ReadEvent) * reference_bus.count) != 0) {
                        fprintf(stderr,
                            "[actor-d508-test] mismatch state=%02X flags=%02X "
                            "global=%02X seed=%u cycles=%u/%u reads=%u/%u "
                            "unsupported=%u/%u writes=%u/%u\n",
                            actor_states[ai], flags[fi], globals[gi], seeds[si],
                            reference_cycles, native_cycles,
                            reference_bus.count, native_bus.count,
                            reference_bus.unsupported_reads,
                            native_bus.unsupported_reads,
                            reference_bus.writes, native_bus.writes);
                        return 0;
                    }
                }
            }
        }
    }
    fprintf(stderr, "[actor-d508-test] positive cases=%u\n", cases);
    return 1;
}

static int CheckGuards(void) {
    TestBus bus;
    Interp816 in;
    CpuState cpu;
    InitCase(&in, &bus, 0, 0, 1, 0);
    memset(&cpu, 0, sizeof(cpu));
    cpu.ram = bus.ram;
    if (!L2ActorD508EarlyReturnEligible(&cpu, &in, 1)) return 0;
    if (!L2ActorD508EarlyReturnFitsStepCap(0, 17) ||
        !L2ActorD508EarlyReturnFitsStepCap(983, 1000) ||
        L2ActorD508EarlyReturnFitsStepCap(0, 16) ||
        L2ActorD508EarlyReturnFitsStepCap(1000, 1000) ||
        L2ActorD508EarlyReturnFitsStepCap(1001, 1000))
        return 0;
    static const uint16_t opcode_pc[L2_ACTOR_D508_EARLY_RETURN_OPCODES] = {
        0xD508,0xD50A,0xD50E,0xD510,0xD539,0xD53C,0xD53E,
        0xD540,0xD543,0xD545,0xD547,0xD548,0xD54A,0xD54E,
        0xD57E,0xD581,0xD583,
    };
    for (unsigned j = 0; j < L2_ACTOR_D508_EARLY_RETURN_OPCODES; ++j) {
        if (!L2ActorD508EarlyReturnContainsOpcodePc(
                0x830000u | opcode_pc[j]) ||
            !L2ActorD508EarlyReturnContainsOpcodePc(
                0x030000u | opcode_pc[j]))
            return 0;
    }
    if (L2ActorD508EarlyReturnContainsOpcodePc(0x00D508u) ||
        L2ActorD508EarlyReturnContainsOpcodePc(0x83D509u) ||
        L2ActorD508EarlyReturnContainsOpcodePc(0x83D599u))
        return 0;

#define REJECT(statement) do { \
    InitCase(&in, &bus, 0, 0, 1, 0); cpu.ram = bus.ram; \
    statement; \
    if (L2ActorD508EarlyReturnEligible(&cpu, &in, 1) || bus.count != 0) \
        return 0; \
} while (0)
    if (L2ActorD508EarlyReturnEligible(&cpu, &in, 0)) return 0;
    REJECT(in.pc++);
    REJECT(in.k = 0x82);
    REJECT(in.db = 0x7E);
    REJECT(in.dp = 1);
    REJECT(in.mf = false);
    REJECT(in.xf = false);
    REJECT(in.e = true);
    REJECT(in.nmiWanted = true);
    REJECT(bus.ram[0x00A7u] = 8);
    REJECT(bus.ram[0x0736u] = 0x80);
    REJECT(bus.ram[0x0736u] = 0x04);
    REJECT(bus.ram[0x0622u] = 0x80);
    REJECT(bus.ram[0x09A7u] = 0);
    REJECT(bus.ram[0x1D0FEu] = 1);
#undef REJECT

    static uint8_t rom[0x1D57Eu + sizeof(kExitPath)];
    memset(rom, 0, sizeof(rom));
    memcpy(rom + 0x1D508u, kEntry, sizeof(kEntry));
    memcpy(rom + 0x1D539u, kMiddle, sizeof(kMiddle));
    memcpy(rom + 0x1D57Eu, kExitPath, sizeof(kExitPath));
    if (!L2ActorD508EarlyReturnBytesMatch(rom, sizeof(rom))) return 0;
    rom[0x1D539u + 5u] ^= 1u;
    if (L2ActorD508EarlyReturnBytesMatch(rom, sizeof(rom))) return 0;
    fprintf(stderr,
        "[actor-d508-test] guards=checked signature=checked "
        "step_cap=exact hooks=local-only\n");
    return 1;
}

int Lufia2ActorD508EarlyReturnSelfTest(void) {
    fprintf(stderr, "[actor-d508-test] begin\n");
    fflush(stderr);
    const int ok = CheckGuards() && CheckPositiveCases();
    fprintf(stderr,
        "[actor-d508-test] %s state=exact bus=exact cycles=exact "
        "interrupts/deadline=guarded\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
