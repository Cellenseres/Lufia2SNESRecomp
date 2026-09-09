/* Pure desktop contract test for the portable frame-wait batch planner.
 * Invoked before ROM, SDL and game initialization. It never touches live
 * guest state and does not claim device-level equivalence. */
#include "patches/native_patches.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

typedef struct TestBus {
    uint8_t tick;
} TestBus;

static uint8_t ReadTest(void *opaque, uint32_t address) {
    TestBus *bus = (TestBus *)opaque;
    if (address == 0x000040u)
        return bus->tick;
    if (address == 0x83900eu)
        return 0xc5;
    if (address == 0x83900fu)
        return 0x40;
    if (address == 0x839010u)
        return 0xf0;
    if (address == 0x839011u)
        return 0xfc;
    if (address == 0x868b4eu)
        return 0xc5;
    if (address == 0x868b4fu)
        return 0x40;
    if (address == 0x868b50u)
        return 0xf0;
    if (address == 0x868b51u)
        return 0xfc;
    if (address == 0x85ec96u)
        return 0xc5;
    if (address == 0x85ec97u)
        return 0x40;
    if (address == 0x85ec98u)
        return 0xf0;
    if (address == 0x85ec99u)
        return 0xfc;
    if (address == 0x848d4fu)
        return 0xc5;
    if (address == 0x848d50u)
        return 0x40;
    if (address == 0x848d51u)
        return 0xf0;
    if (address == 0x848d52u)
        return 0xfc;
    if (address == 0x869752u)
        return 0xc5;
    if (address == 0x869753u)
        return 0x40;
    if (address == 0x869754u)
        return 0xf0;
    if (address == 0x869755u)
        return 0xfc;
    return 0;
}

static uint64_t ReferencePairs(uint64_t current, uint64_t deadline,
                               uint64_t pair_master,
                               uint64_t remaining_steps) {
    if (deadline <= current || pair_master == 0 || remaining_steps <= 2)
        return 0;

    /* This intentionally remains a step-by-step oracle, but it must obey the
     * caller's instruction budget while searching. Without this bound the
     * UINT64_MAX edge case would simulate roughly 2^64 / pair_master guest
     * iterations even though the production planner may use only the much
     * smaller remaining_steps budget. */
    const uint64_t step_pairs = (remaining_steps - 2u) / 2u;
    const uint64_t needed_reachable = step_pairs + 1u;
    uint64_t reachable = 0;
    uint64_t cursor = current;
    while (reachable < needed_reachable &&
           pair_master < deadline - cursor) {
        cursor += pair_master;
        ++reachable;
    }
    if (reachable)
        --reachable;
    return reachable < step_pairs ? reachable : step_pairs;
}

static int SameInterpreterState(const Interp816 *a, const Interp816 *b) {
#define FIELD(name) do { if (a->name != b->name) return 0; } while (0)
    FIELD(a); FIELD(x); FIELD(y); FIELD(sp); FIELD(pc); FIELD(dp);
    FIELD(k); FIELD(db); FIELD(c); FIELD(z); FIELD(v); FIELD(n);
    FIELD(i); FIELD(d); FIELD(xf); FIELD(mf); FIELD(e);
    FIELD(irqWanted); FIELD(nmiWanted); FIELD(waiting); FIELD(stopped);
    FIELD(brkHookEnabled); FIELD(cyclesUsed);
#undef FIELD
    return 1;
}

static int CheckPlanner(void) {
    uint64_t cases = 0;
    for (uint64_t current = 0; current < 24; ++current) {
        for (uint64_t deadline = 0; deadline < 192; ++deadline) {
            for (uint64_t pair = 1; pair <= 48; ++pair) {
                for (uint64_t steps = 0; steps <= 48; ++steps) {
                    const uint64_t actual = L2FrameWaitBatchPairs(
                        current, deadline, pair, steps);
                    const uint64_t expected = ReferencePairs(
                        current, deadline, pair, steps);
                    ++cases;
                    if (actual != expected) {
                        fprintf(stderr,
                            "[frame-wait-ff-test] planner mismatch "
                            "current=%llu deadline=%llu pair=%llu steps=%llu "
                            "actual=%llu expected=%llu\n",
                            (unsigned long long)current,
                            (unsigned long long)deadline,
                            (unsigned long long)pair,
                            (unsigned long long)steps,
                            (unsigned long long)actual,
                            (unsigned long long)expected);
                        return 0;
                    }
                }
            }
        }
    }

    static const struct {
        uint64_t current, deadline, pair, steps;
    } edge[] = {
        {0, UINT64_MAX, 38, 1000000},
        {UINT64_MAX - 1000, UINT64_MAX, 46, 1000},
        {UINT64_MAX - 1, UINT64_MAX, 38, 1000},
        {1234, 1234, 38, 1000},
        {1235, 1234, 38, 1000},
        {0, 357368, 38, 1000000},
        {0, 357368, 46, 1000000},
    };
    for (unsigned i = 0; i < sizeof(edge) / sizeof(edge[0]); ++i) {
        const uint64_t actual = L2FrameWaitBatchPairs(
            edge[i].current, edge[i].deadline, edge[i].pair, edge[i].steps);
        const uint64_t expected = ReferencePairs(
            edge[i].current, edge[i].deadline, edge[i].pair, edge[i].steps);
        ++cases;
        if (actual != expected)
            return 0;
        if (actual) {
            const uint64_t advanced = actual * edge[i].pair;
            if (advanced >= edge[i].deadline - edge[i].current)
                return 0;
        }
    }
    fprintf(stderr, "[frame-wait-ff-test] planner cases=%llu\n",
            (unsigned long long)cases);
    return 1;
}

static int CheckCpuState(void) {
    static const struct {
        uint8_t bank;
        uint16_t cmp_pc;
    } sites[] = {
        {0x83, 0x900e},
        {0x86, 0x8b4e},
        {0x85, 0xec96},
        {0x84, 0x8d4f},
        {0x86, 0x9752},
    };
    unsigned cases = 0;
    for (unsigned site = 0; site < sizeof(sites) / sizeof(sites[0]); ++site) {
        for (unsigned a = 0; a < 256; ++a) {
            for (unsigned p = 0; p < 256; ++p) {
                TestBus left_bus = {(uint8_t)a};
                TestBus right_bus = {(uint8_t)a};
                Interp816 reference;
                memset(&reference, 0, sizeof(reference));
                reference.a = (uint16_t)(0x5a00u | a);
                reference.x = (uint16_t)(0xa500u | (a ^ p));
                reference.y = (uint16_t)(0x3c00u | p);
                reference.sp = (uint16_t)(0x1f00u | a);
                reference.pc = sites[site].cmp_pc;
                reference.dp = 0;
                reference.k = sites[site].bank;
                reference.db = (uint8_t)(p ^ 0x7e);
                reference.read = ReadTest;
                reference.mem = &left_bus;
                reference.brkHookEnabled = true;
                interp816_setFlags(&reference, (uint8_t)(p | 0x20u));
                reference.e = false;
                reference.mf = true;

                Interp816 native = reference;
                native.mem = &right_bus;
                (void)interp816_runOpcode(&reference);
                (void)interp816_runOpcode(&reference);
                L2FrameWaitApplyEqualBatchState(&native, sites[site].cmp_pc);
                ++cases;
                if (!SameInterpreterState(&reference, &native)) {
                    fprintf(stderr,
                        "[frame-wait-ff-test] state mismatch "
                        "PC=%02X:%04X A=%02X P=%02X\n",
                        sites[site].bank, sites[site].cmp_pc, a, p);
                    return 0;
                }
            }
        }
    }
    fprintf(stderr, "[frame-wait-ff-test] state cases=%u\n", cases);
    return 1;
}

int Lufia2FrameWaitFastForwardSelfTest(void) {
    fprintf(stderr, "[frame-wait-ff-test] begin\n");
    fflush(stderr);
    const int ok = CheckPlanner() && CheckCpuState();
    fprintf(stderr,
        "[frame-wait-ff-test] %s planner=checked cpu_state=checked "
        "devices=guarded-not-simulated\n",
        ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
