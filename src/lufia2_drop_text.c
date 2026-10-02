#include "lufia2_drop_text.h"

#include <stdint.h>

#include "cpu_state.h"
#include "decomp_bridge/host_events.h"

enum {
    /* Item word id | count << 9; the remainder after adding. */
    RECEIVED_ITEM = 0x0a06,
    ITEM_ID_HIGH = 0x0100,
};

extern uint8_t g_ram[0x20000];

/* $81:F0F3 clears only the low byte of a stacked item. */
static void ClearStackedRemainder(CpuState *cpu, uint32_t pc24) {
    (void)cpu;
    (void)pc24;
    const uint16_t remainder =
        (uint16_t)(g_ram[RECEIVED_ITEM] | g_ram[RECEIVED_ITEM + 1] << 8);
    if (remainder == ITEM_ID_HIGH)
        g_ram[RECEIVED_ITEM + 1] = 0;
}

void Lufia2DropTextInstall(void) {
    Lufia2DecompAddItemReceivedEvent(ClearStackedRemainder);
}
