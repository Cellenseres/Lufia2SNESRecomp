#include "host_events.h"

#include "snes/interp_bridge.h"

static Lufia2DecompEquipmentListDrawEvent equipment_list_draw_event;
static Lufia2DecompPlayTimeTickEvent play_time_tick_event;
static Lufia2DecompMenuNumberEvent menu_number_event;

void Lufia2DecompEquipmentListDraw(CpuState *cpu, uint32_t pc) {
    if (equipment_list_draw_event)
        equipment_list_draw_event(cpu, pc);
}

void Lufia2DecompSetEquipmentListDrawEvent(
    Lufia2DecompEquipmentListDrawEvent callback) {
    equipment_list_draw_event = callback;
    /* NULL clears the entire interpreter table; retain no-op forwarders. */
    interp_bridge_set_pre_opcode_hook(0x82a3b4u, Lufia2DecompEquipmentListDraw);
    interp_bridge_set_pre_opcode_hook(0x82a3f0u, Lufia2DecompEquipmentListDraw);
}

void Lufia2DecompPlayTimeTick(CpuState *cpu, uint32_t pc) {
    if (play_time_tick_event)
        play_time_tick_event(cpu, pc);
}

void Lufia2DecompSetPlayTimeTickEvent(Lufia2DecompPlayTimeTickEvent callback) {
    play_time_tick_event = callback;
    interp_bridge_set_pre_opcode_hook(0x808699u, Lufia2DecompPlayTimeTick);
}

void Lufia2DecompMenuNumber(CpuState *cpu, uint32_t pc) {
    if (menu_number_event)
        menu_number_event(cpu, pc);
}

void Lufia2DecompSetMenuNumberEvent(Lufia2DecompMenuNumberEvent callback) {
    menu_number_event = callback;
    interp_bridge_set_pre_opcode_hook(0x808922u, Lufia2DecompMenuNumber);
}
