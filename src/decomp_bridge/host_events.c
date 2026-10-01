#include "host_events.h"

#include "cpu_state.h"
#include "snes/interp_bridge.h"

static Lufia2DecompEquipmentListDrawEvent equipment_list_draw_event;
static Lufia2DecompPlayTimeTickEvent play_time_tick_event;
static Lufia2DecompMenuNumberEvent menu_number_event;
static Lufia2DecompMapLoadEvent map_load_begin_event;
static Lufia2DecompMapLoadEvent map_load_committed_event;
static Lufia2DecompGameFileEvent game_file_event;
static Lufia2DecompSongLoadEvent song_load_event;
static Lufia2DecompMusicFadeOutEvent music_fade_out_event;
static uint8_t interpreter_map_loading;
static uint16_t interpreter_map_return_stack;

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

void Lufia2DecompMapLoadBegin(CpuState *cpu, uint32_t pc) {
    if (map_load_begin_event)
        map_load_begin_event(cpu, pc);
}

void Lufia2DecompMapLoadCommitted(CpuState *cpu, uint32_t pc) {
    if (map_load_committed_event)
        map_load_committed_event(cpu, pc);
}

static void InterpreterMapEntered(CpuState *cpu, uint32_t pc) {
    (void)cpu;
    (void)pc;
    interpreter_map_loading = 0;
}

static void InterpreterMapBegin(CpuState *cpu, uint32_t pc) {
    Lufia2DecompMapLoadBegin(cpu, pc);
    interpreter_map_loading = 1;
    interpreter_map_return_stack = (uint16_t)(cpu->S + 1u);
    if (cpu->emulation)
        interpreter_map_return_stack =
            (uint16_t)(0x0100u | (interpreter_map_return_stack & 0x00ffu));
}

static void InterpreterMapCommitted(CpuState *cpu, uint32_t pc) {
    const uint8_t loaded = interpreter_map_loading;
    interpreter_map_loading = 0;
    if (loaded && cpu->S == interpreter_map_return_stack)
        Lufia2DecompMapLoadCommitted(cpu, pc);
}

static void InstallInterpreterMapEvents(void) {
    /* The skip path shares B580; a new entry also cancels stale loads. */
    interp_bridge_set_pre_opcode_hook(0x83b53bu, InterpreterMapEntered);
    interp_bridge_set_pre_opcode_hook(0x83b548u, InterpreterMapBegin);
    interp_bridge_set_pre_opcode_hook(0x83b580u, InterpreterMapCommitted);
}

void Lufia2DecompSetMapLoadBeginEvent(Lufia2DecompMapLoadEvent callback) {
    map_load_begin_event = callback;
    interpreter_map_loading = 0;
    InstallInterpreterMapEvents();
}

void Lufia2DecompSetMapLoadCommittedEvent(Lufia2DecompMapLoadEvent callback) {
    map_load_committed_event = callback;
    interpreter_map_loading = 0;
    InstallInterpreterMapEvents();
}

static uint16_t GameFileFrameByte(const CpuState *cpu, unsigned offset) {
    const uint16_t address = (uint16_t)(cpu->S + offset);
    return cpu->emulation ? (uint16_t)(0x0100u | (address & 0x00ffu)) : address;
}

void Lufia2DecompGameFile(CpuState *cpu, uint32_t pc) {
    Lufia2DecompGameFileOperation operation;
    if (!game_file_event)
        return;
    switch (pc & 0x7fffffu) {
    case 0x009099u:
        operation = LUFIA2_DECOMP_GAME_FILE_LOAD;
        break;
    case 0x0090c9u:
        operation = LUFIA2_DECOMP_GAME_FILE_SAVE;
        break;
    case 0x00914bu: {
        const uint8_t low = cpu_read8(cpu, 0u, GameFileFrameByte(cpu, 1u));
        const uint8_t high = cpu_read8(cpu, 0u, GameFileFrameByte(cpu, 2u));
        const uint8_t bank = cpu_read8(cpu, 0u, GameFileFrameByte(cpu, 3u));
        operation = low == 0xa8u && high == 0x90u && !(bank & 0x7fu)
            ? LUFIA2_DECOMP_GAME_FILE_LOAD_HEADER
            : LUFIA2_DECOMP_GAME_FILE_PREVIEW;
        break;
    }
    default:
        return;
    }
    game_file_event(cpu, pc, operation);
}

void Lufia2DecompSetGameFileEvent(Lufia2DecompGameFileEvent callback) {
    game_file_event = callback;
    interp_bridge_set_pre_opcode_hook(0x809099u, Lufia2DecompGameFile);
    interp_bridge_set_pre_opcode_hook(0x8090c9u, Lufia2DecompGameFile);
    interp_bridge_set_pre_opcode_hook(0x80914bu, Lufia2DecompGameFile);
}

uint32_t Lufia2DecompSongLoad(CpuState *cpu, uint32_t pc) {
    return song_load_event ? song_load_event(cpu, pc) : 0u;
}

static void InterpreterSongPush(CpuState *cpu, uint8_t value) {
    cpu_write8(cpu, 0u, cpu->S, value);
    cpu->S = (uint16_t)(cpu->S - 1u);
    if (cpu->emulation)
        cpu->S = (uint16_t)(0x0100u | (cpu->S & 0x00ffu));
}

static void InterpreterSongLoad(CpuState *cpu, uint32_t pc) {
    const uint32_t target = Lufia2DecompSongLoad(cpu, pc);
    if (target) {
        const uint16_t back = (uint16_t)(pc - 1u);
        InterpreterSongPush(cpu, (uint8_t)(pc >> 16));
        InterpreterSongPush(cpu, (uint8_t)(back >> 8));
        InterpreterSongPush(cpu, (uint8_t)back);
        interp_bridge_pre_opcode_redirect(target);
    }
}

void Lufia2DecompSetSongLoadEvent(Lufia2DecompSongLoadEvent callback) {
    song_load_event = callback;
    interp_bridge_set_pre_opcode_hook(0x80942eu, InterpreterSongLoad);
}

void Lufia2DecompMusicFadeOut(CpuState *cpu, uint32_t pc) {
    if (music_fade_out_event)
        music_fade_out_event(cpu, pc);
}

void Lufia2DecompSetMusicFadeOutEvent(Lufia2DecompMusicFadeOutEvent callback) {
    music_fade_out_event = callback;
    interp_bridge_set_pre_opcode_hook(0x809692u, Lufia2DecompMusicFadeOut);
}
