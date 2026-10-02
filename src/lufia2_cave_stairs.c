#include "lufia2_cave_stairs.h"

#include <stdint.h>

#include "cpu_state.h"
#include "decomp_bridge/host_events.h"

enum {
    FIELD_FLAGS = 0x05b5,
    /* Set by $83:DA84 after a step; $83:823B checks the tile. */
    STEP_CHECK_PENDING = 0x10,
};

extern uint8_t g_ram[0x20000];

/* A step check left over from the stairs would override the restart. */
static void DropPendingStepCheck(CpuState *cpu, uint32_t pc24) {
    (void)cpu;
    (void)pc24;
    g_ram[FIELD_FLAGS] &= (uint8_t)~STEP_CHECK_PENDING;
}

void Lufia2CaveStairsInstall(void) {
    Lufia2DecompAddCaveDefeatResetEvent(DropPendingStepCheck);
}
