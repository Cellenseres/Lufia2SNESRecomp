#include "lufia2_equip_names.h"

#include <stdint.h>
#include <string.h>

#include "cpu_state.h"
#include "snes/interp_bridge.h"

enum {
    /* LDY #$C37D ahead of both equipment list draws. */
    EQUIP_LIST_FIRST_DRAW = 0x82a3b4,
    EQUIP_LIST_REDRAW = 0x82a3f0,
    /* Six slots of icon plus twelve characters at $7E:30E2. */
    EQUIP_LIST_ORIGIN = 0x30e2,
    EQUIP_LIST_COLUMNS = 13,
    EQUIP_LIST_ROWS = 12,
    TILEMAP_ENTRY_BYTES = 2,
    TILEMAP_ROW_BYTES = 0x40,
};

extern uint8_t g_ram[0x20000];

/* $81:F2A9 trims names; the list is never cleared. */
static void ClearEquipList(CpuState *cpu, uint32_t pc24) {
    (void)cpu;
    (void)pc24;
    for (unsigned row = 0; row < EQUIP_LIST_ROWS; row++) {
        memset(&g_ram[EQUIP_LIST_ORIGIN + row * TILEMAP_ROW_BYTES], 0,
               EQUIP_LIST_COLUMNS * TILEMAP_ENTRY_BYTES);
    }
}

void Lufia2EquipNamesInstallHooks(void) {
    interp_bridge_set_pre_opcode_hook(EQUIP_LIST_FIRST_DRAW, ClearEquipList);
    interp_bridge_set_pre_opcode_hook(EQUIP_LIST_REDRAW, ClearEquipList);
}
