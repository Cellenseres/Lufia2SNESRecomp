#include "patches/native_patches.h"

#include <stdio.h>
#include <string.h>

enum { EVENT_CAP = 64 };

typedef struct ReadEvent {
    uint32_t address;
    uint8_t value;
} ReadEvent;

typedef struct TestBus {
    uint8_t ram[0x2000];
    ReadEvent event[EVENT_CAP];
    unsigned count;
    unsigned unsupported_reads;
    unsigned writes;
} TestBus;

static const uint8_t kPrefix[] = {
    0xA6,0xA7, 0xBD,0x22,0x06, 0x29,0x80, 0xD0,0x3A,
    0xBD,0x91,0x12, 0x89,0x07, 0xF0,0x06,
    0x3A, 0x9D,0x91,0x12, 0x80,0x2E,
    0xAD,0xA7,0x09, 0x89,0x01, 0xF0,0x19,
    0xAD,0x69,0x12, 0x10,0x14,
    0xA5,0xA7, 0xF0,0x1E,
    0xBD,0x22,0x06, 0x89,0x08, 0xD0,0x09,
    0x89,0x40, 0xF0,0x12,
};

static uint8_t ReadTest(void *opaque, uint32_t address) {
    TestBus *bus = (TestBus *)opaque;
    uint8_t value = 0;
    const uint8_t bank = (uint8_t)(address >> 16);
    const uint16_t offset = (uint16_t)address;
    if (bank == 0x83u && offset >= L2_ACTOR_EARLY_RETURN_PC &&
        offset < L2_ACTOR_EARLY_RETURN_PC + sizeof(kPrefix)) {
        value = kPrefix[offset - L2_ACTOR_EARLY_RETURN_PC];
    } else if ((bank == 0 || bank == 0x83u) && offset < 0x2000u) {
        value = bus->ram[offset];
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

static void InitCase(Interp816 *in, TestBus *bus, uint8_t actor,
                     uint8_t flags, uint8_t state, uint8_t global,
                     uint8_t direction, unsigned seed) {
    memset(in, 0, sizeof(*in));
    memset(bus, 0, sizeof(*bus));
    bus->ram[0x00A7u] = actor;
    bus->ram[0x0622u + actor] = flags;
    bus->ram[0x1291u + actor] = state;
    bus->ram[0x09A7u] = global;
    bus->ram[0x1269u] = direction;
    in->mem = bus;
    in->read = ReadTest;
    in->write = WriteTest;
    in->a = (uint16_t)(0xA500u | (seed & 0xFFu));
    in->x = (uint16_t)(0x5A00u | ((seed * 3u) & 0xFFu));
    in->y = (uint16_t)(0x3300u | ((seed * 5u) & 0xFFu));
    in->sp = (uint16_t)(0x1F00u | ((seed * 7u) & 0xFFu));
    in->pc = L2_ACTOR_EARLY_RETURN_PC;
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
    static const unsigned seeds[] = {0,1,2,4,8,16,32,63};
    unsigned cases = 0;
    for (unsigned actor = 8; actor <= 0x78; actor += 8) {
        for (unsigned flags = 0; flags <= 0x30; flags += 0x10) {
            for (unsigned state = 0; state <= 0xF8; state += 0x78) {
                for (unsigned si = 0; si < sizeof(seeds) / sizeof(seeds[0]); ++si) {
                    const unsigned seed = seeds[si];
                    TestBus reference_bus, native_bus;
                    Interp816 reference, native;
                    InitCase(&reference, &reference_bus, (uint8_t)actor,
                             (uint8_t)flags, (uint8_t)state, 0x11u, 0x80u,
                             seed);
                    InitCase(&native, &native_bus, (uint8_t)actor,
                             (uint8_t)flags, (uint8_t)state, 0x11u, 0x80u,
                             seed);
                    CpuState cpu;
                    memset(&cpu, 0, sizeof(cpu));
                    cpu.ram = native_bus.ram;
                    if (!L2ActorEarlyReturnEligible(&cpu, &native, 1))
                        return 0;

                    unsigned reference_cycles = 0;
                    for (unsigned op = 0; op < 19; ++op)
                        reference_cycles += (unsigned)interp816_runOpcode(&reference);
                    const unsigned native_cycles = L2ActorEarlyReturnStep(&native);
                    ++cases;
                    if (reference.pc != L2_ACTOR_EARLY_RETURN_NEXT_PC ||
                        reference_cycles != L2ActorEarlyReturnCycles(actor) ||
                        native_cycles != reference_cycles ||
                        !SameState(&reference, &native) ||
                        reference_bus.unsupported_reads != 0 ||
                        native_bus.unsupported_reads != 0 ||
                        reference_bus.writes != 0 || native_bus.writes != 0 ||
                        reference_bus.count != native_bus.count ||
                        reference_bus.count != 50u ||
                        memcmp(reference_bus.event, native_bus.event,
                               sizeof(ReadEvent) * reference_bus.count) != 0) {
                        fprintf(stderr,
                            "[actor-fastpath-test] mismatch actor=%02X flags=%02X "
                            "state=%02X seed=%u cycles=%u/%u reads=%u/%u "
                            "unsupported=%u/%u writes=%u/%u\n",
                            actor, flags, state, seed, reference_cycles,
                            native_cycles, reference_bus.count, native_bus.count,
                            reference_bus.unsupported_reads,
                            native_bus.unsupported_reads, reference_bus.writes,
                            native_bus.writes);
                        return 0;
                    }
                }
            }
        }
    }
    fprintf(stderr, "[actor-fastpath-test] positive cases=%u\n", cases);
    return 1;
}

static int CheckGuards(void) {
    TestBus bus;
    Interp816 in;
    CpuState cpu;
    InitCase(&in, &bus, 8, 0, 8, 1, 0x80, 0);
    memset(&cpu, 0, sizeof(cpu));
    cpu.ram = bus.ram;
    if (!L2ActorEarlyReturnEligible(&cpu, &in, 1)) return 0;
    if (!L2ActorEarlyReturnFitsStepCap(0, 19) ||
        !L2ActorEarlyReturnFitsStepCap(981, 1000) ||
        L2ActorEarlyReturnFitsStepCap(0, 18) ||
        L2ActorEarlyReturnFitsStepCap(1000, 1000) ||
        L2ActorEarlyReturnFitsStepCap(1001, 1000))
        return 0;
    static const uint16_t opcode_pc[L2_ACTOR_EARLY_RETURN_OPCODES] = {
        0xC7F8,0xC7FA,0xC7FD,0xC7FF,0xC801,0xC804,0xC806,
        0xC80E,0xC811,0xC813,0xC815,0xC818,0xC81A,0xC81C,
        0xC81E,0xC821,0xC823,0xC825,0xC827,
    };
    for (unsigned j = 0; j < L2_ACTOR_EARLY_RETURN_OPCODES; ++j) {
        if (!L2ActorEarlyReturnContainsOpcodePc(0x830000u | opcode_pc[j]) ||
            !L2ActorEarlyReturnContainsOpcodePc(0x030000u | opcode_pc[j]))
            return 0;
    }
    if (L2ActorEarlyReturnContainsOpcodePc(0x00C7F8u) ||
        L2ActorEarlyReturnContainsOpcodePc(0x83C7F9u) ||
        L2ActorEarlyReturnContainsOpcodePc(0x00942Eu))
        return 0;

#define REJECT(statement) do { \
    InitCase(&in, &bus, 8, 0, 8, 1, 0x80, 0); cpu.ram = bus.ram; \
    statement; \
    if (L2ActorEarlyReturnEligible(&cpu, &in, 1) || bus.count != 0) return 0; \
} while (0)
    if (L2ActorEarlyReturnEligible(&cpu, &in, 0)) return 0;
    REJECT(in.pc++);
    REJECT(in.k = 0x82);
    REJECT(in.db = 0x7E);
    REJECT(in.dp = 1);
    REJECT(in.mf = false);
    REJECT(in.xf = false);
    REJECT(in.e = true);
    REJECT(in.nmiWanted = true);
    REJECT(bus.ram[0x00A7] = 0);
    REJECT(bus.ram[0x00A7] = 9);
    REJECT(bus.ram[0x062A] = 0x80);
    REJECT(bus.ram[0x062A] = 0x08);
    REJECT(bus.ram[0x062A] = 0x40);
    REJECT(bus.ram[0x1299] = 1);
    REJECT(bus.ram[0x09A7] = 0);
    REJECT(bus.ram[0x1269] = 0);
#undef REJECT

    static uint8_t rom[0x1C7F8u + sizeof(kPrefix)];
    memset(rom, 0, sizeof(rom));
    memcpy(rom + 0x1C7F8u, kPrefix, sizeof(kPrefix));
    if (!L2ActorEarlyReturnBytesMatch(rom, sizeof(rom))) return 0;
    rom[0x1C7F8u + 7u] ^= 1u;
    if (L2ActorEarlyReturnBytesMatch(rom, sizeof(rom))) return 0;
    fprintf(stderr,
        "[actor-fastpath-test] guards=checked signature=checked "
        "step_cap=exact hooks=local-only\n");
    return 1;
}

int Lufia2ActorEarlyReturnSelfTest(void) {
    fprintf(stderr, "[actor-fastpath-test] begin\n");
    fflush(stderr);
    const int ok = CheckGuards() && CheckPositiveCases();
    fprintf(stderr,
        "[actor-fastpath-test] %s state=exact bus=exact cycles=exact "
        "interrupts/deadline=guarded\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
