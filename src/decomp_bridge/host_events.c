#include "host_events.h"

#include "cpu_state.h"
#include "snes/interp_bridge.h"

enum { EVENT_SUBSCRIBER_CAPACITY = 8 };
typedef void (*CpuEventCallback)(CpuState *, uint32_t);
typedef struct CpuEventList {
    CpuEventCallback callbacks[EVENT_SUBSCRIBER_CAPACITY];
    unsigned count;
} CpuEventList;

static int CpuEventAdd(CpuEventList *event, CpuEventCallback callback) {
    if (!callback)
        return 0;
    for (unsigned i = 0; i < event->count; ++i)
        if (event->callbacks[i] == callback)
            return 1;
    if (event->count == EVENT_SUBSCRIBER_CAPACITY)
        return 0;
    event->callbacks[event->count++] = callback;
    return 1;
}

static void CpuEventSet(CpuEventList *event, CpuEventCallback callback) {
    event->count = 0;
    (void)CpuEventAdd(event, callback);
}

static void CpuEventRemove(CpuEventList *event, CpuEventCallback callback) {
    for (unsigned i = 0; i < event->count; ++i) {
        if (event->callbacks[i] != callback)
            continue;
        --event->count;
        for (; i < event->count; ++i)
            event->callbacks[i] = event->callbacks[i + 1u];
        return;
    }
}

static void CpuEventDispatch(
    const CpuEventList *event, CpuState *cpu, uint32_t pc) {
    const CpuEventList snapshot = *event;
    for (unsigned i = 0; i < snapshot.count; ++i)
        snapshot.callbacks[i](cpu, pc);
}

typedef struct GameFileEventList {
    Lufia2DecompGameFileEvent callbacks[EVENT_SUBSCRIBER_CAPACITY];
    unsigned count;
} GameFileEventList;

static int GameFileEventAdd(
    GameFileEventList *event, Lufia2DecompGameFileEvent callback) {
    if (!callback)
        return 0;
    for (unsigned i = 0; i < event->count; ++i)
        if (event->callbacks[i] == callback)
            return 1;
    if (event->count == EVENT_SUBSCRIBER_CAPACITY)
        return 0;
    event->callbacks[event->count++] = callback;
    return 1;
}

static void GameFileEventRemove(
    GameFileEventList *event, Lufia2DecompGameFileEvent callback) {
    for (unsigned i = 0; i < event->count; ++i) {
        if (event->callbacks[i] != callback)
            continue;
        --event->count;
        for (; i < event->count; ++i)
            event->callbacks[i] = event->callbacks[i + 1u];
        return;
    }
}

static CpuEventList equipment_list_draw_event;
static CpuEventList play_time_tick_event;
static CpuEventList menu_number_event;
static CpuEventList map_load_begin_event;
static CpuEventList map_load_committed_event;
static GameFileEventList game_file_event;
static CpuEventList song_load_observers;
static Lufia2DecompSongLoadEvent song_load_event;
static CpuEventList music_fade_out_event;
static CpuEventList spell_price_stored_event;
static CpuEventList party_stats_derived_event;
static CpuEventList party_stat_totals_event;
static CpuEventList item_received_event;
static CpuEventList cave_exit_begin_event;
static CpuEventList cave_exit_restored_event;
static uint8_t interpreter_map_loading;
static uint16_t interpreter_map_return_stack;

void Lufia2DecompEquipmentListDraw(CpuState *cpu, uint32_t pc) {
    CpuEventDispatch(&equipment_list_draw_event, cpu, pc);
}

void Lufia2DecompSetEquipmentListDrawEvent(
    Lufia2DecompEquipmentListDrawEvent callback) {
    CpuEventSet(&equipment_list_draw_event, callback);
    /* NULL clears the entire interpreter table; retain no-op forwarders. */
    interp_bridge_set_pre_opcode_hook(0x82a3b4u, Lufia2DecompEquipmentListDraw);
    interp_bridge_set_pre_opcode_hook(0x82a3f0u, Lufia2DecompEquipmentListDraw);
}

void Lufia2DecompPlayTimeTick(CpuState *cpu, uint32_t pc) {
    CpuEventDispatch(&play_time_tick_event, cpu, pc);
}

void Lufia2DecompSetPlayTimeTickEvent(Lufia2DecompPlayTimeTickEvent callback) {
    CpuEventSet(&play_time_tick_event, callback);
    interp_bridge_set_pre_opcode_hook(0x808699u, Lufia2DecompPlayTimeTick);
}

void Lufia2DecompMenuNumber(CpuState *cpu, uint32_t pc) {
    CpuEventDispatch(&menu_number_event, cpu, pc);
}

void Lufia2DecompSetMenuNumberEvent(Lufia2DecompMenuNumberEvent callback) {
    CpuEventSet(&menu_number_event, callback);
    interp_bridge_set_pre_opcode_hook(0x808922u, Lufia2DecompMenuNumber);
}

/* A native load that unwinds finishes in the interpreter. */
void Lufia2DecompMapLoadBegin(CpuState *cpu, uint32_t pc) {
    CpuEventDispatch(&map_load_begin_event, cpu, pc);
    interpreter_map_loading = 1;
    interpreter_map_return_stack = (uint16_t)(cpu->S + 1u);
    if (cpu->emulation)
        interpreter_map_return_stack =
            (uint16_t)(0x0100u | (interpreter_map_return_stack & 0x00ffu));
}

void Lufia2DecompMapLoadCommitted(CpuState *cpu, uint32_t pc) {
    interpreter_map_loading = 0;
    CpuEventDispatch(&map_load_committed_event, cpu, pc);
}

static void InterpreterMapEntered(CpuState *cpu, uint32_t pc) {
    (void)cpu;
    (void)pc;
    interpreter_map_loading = 0;
}

static void InterpreterMapBegin(CpuState *cpu, uint32_t pc) {
    Lufia2DecompMapLoadBegin(cpu, pc);
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
    CpuEventSet(&map_load_begin_event, callback);
    interpreter_map_loading = 0;
    InstallInterpreterMapEvents();
}

void Lufia2DecompSetMapLoadCommittedEvent(Lufia2DecompMapLoadEvent callback) {
    CpuEventSet(&map_load_committed_event, callback);
    interpreter_map_loading = 0;
    InstallInterpreterMapEvents();
}

static uint16_t GameFileFrameByte(const CpuState *cpu, unsigned offset) {
    const uint16_t address = (uint16_t)(cpu->S + offset);
    return cpu->emulation ? (uint16_t)(0x0100u | (address & 0x00ffu)) : address;
}

void Lufia2DecompGameFile(CpuState *cpu, uint32_t pc) {
    Lufia2DecompGameFileOperation operation;
    if (!game_file_event.count)
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
    const GameFileEventList snapshot = game_file_event;
    for (unsigned i = 0; i < snapshot.count; ++i)
        snapshot.callbacks[i](cpu, pc, operation);
}

void Lufia2DecompSetGameFileEvent(Lufia2DecompGameFileEvent callback) {
    game_file_event.count = 0;
    (void)GameFileEventAdd(&game_file_event, callback);
    interp_bridge_set_pre_opcode_hook(0x809099u, Lufia2DecompGameFile);
    interp_bridge_set_pre_opcode_hook(0x8090c9u, Lufia2DecompGameFile);
    interp_bridge_set_pre_opcode_hook(0x80914bu, Lufia2DecompGameFile);
}

uint32_t Lufia2DecompSongLoad(CpuState *cpu, uint32_t pc) {
    CpuEventDispatch(&song_load_observers, cpu, pc);
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
    CpuEventDispatch(&music_fade_out_event, cpu, pc);
}

void Lufia2DecompSetMusicFadeOutEvent(Lufia2DecompMusicFadeOutEvent callback) {
    CpuEventSet(&music_fade_out_event, callback);
    interp_bridge_set_pre_opcode_hook(0x809692u, Lufia2DecompMusicFadeOut);
}

void Lufia2DecompSpellPriceStored(CpuState *cpu, uint32_t pc) {
    CpuEventDispatch(&spell_price_stored_event, cpu, pc);
}

void Lufia2DecompSetSpellPriceStoredEvent(
    Lufia2DecompSpellPriceStoredEvent callback) {
    CpuEventSet(&spell_price_stored_event, callback);
    interp_bridge_set_pre_opcode_hook(0x82d922u, Lufia2DecompSpellPriceStored);
}

void Lufia2DecompPartyStatsDerived(CpuState *cpu, uint32_t pc) {
    CpuEventDispatch(&party_stats_derived_event, cpu, pc);
}

void Lufia2DecompSetPartyStatsDerivedEvent(
    Lufia2DecompPartyStatsDerivedEvent callback) {
    CpuEventSet(&party_stats_derived_event, callback);
    interp_bridge_set_pre_opcode_hook(0x81f4e2u, Lufia2DecompPartyStatsDerived);
}

void Lufia2DecompItemReceived(CpuState *cpu, uint32_t pc) {
    CpuEventDispatch(&item_received_event, cpu, pc);
}

void Lufia2DecompSetItemReceivedEvent(Lufia2DecompItemReceivedEvent callback) {
    CpuEventSet(&item_received_event, callback);
    interp_bridge_set_pre_opcode_hook(0x81f099u, Lufia2DecompItemReceived);
}

int Lufia2DecompAddEquipmentListDrawEvent(Lufia2DecompEquipmentListDrawEvent callback) {
    const int added = CpuEventAdd(&equipment_list_draw_event, callback);
    interp_bridge_set_pre_opcode_hook(0x82a3b4u, Lufia2DecompEquipmentListDraw);
    interp_bridge_set_pre_opcode_hook(0x82a3f0u, Lufia2DecompEquipmentListDraw);
    return added;
}

void Lufia2DecompRemoveEquipmentListDrawEvent(Lufia2DecompEquipmentListDrawEvent callback) {
    CpuEventRemove(&equipment_list_draw_event, callback);
}

int Lufia2DecompAddPlayTimeTickEvent(Lufia2DecompPlayTimeTickEvent callback) {
    const int added = CpuEventAdd(&play_time_tick_event, callback);
    interp_bridge_set_pre_opcode_hook(0x808699u, Lufia2DecompPlayTimeTick);
    return added;
}

void Lufia2DecompRemovePlayTimeTickEvent(Lufia2DecompPlayTimeTickEvent callback) {
    CpuEventRemove(&play_time_tick_event, callback);
}

int Lufia2DecompAddMenuNumberEvent(Lufia2DecompMenuNumberEvent callback) {
    const int added = CpuEventAdd(&menu_number_event, callback);
    interp_bridge_set_pre_opcode_hook(0x808922u, Lufia2DecompMenuNumber);
    return added;
}

void Lufia2DecompRemoveMenuNumberEvent(Lufia2DecompMenuNumberEvent callback) {
    CpuEventRemove(&menu_number_event, callback);
}

int Lufia2DecompAddMapLoadBeginEvent(Lufia2DecompMapLoadEvent callback) {
    const int added = CpuEventAdd(&map_load_begin_event, callback);
    InstallInterpreterMapEvents();
    return added;
}

void Lufia2DecompRemoveMapLoadBeginEvent(Lufia2DecompMapLoadEvent callback) {
    CpuEventRemove(&map_load_begin_event, callback);
}

int Lufia2DecompAddMapLoadCommittedEvent(Lufia2DecompMapLoadEvent callback) {
    const int added = CpuEventAdd(&map_load_committed_event, callback);
    InstallInterpreterMapEvents();
    return added;
}

void Lufia2DecompRemoveMapLoadCommittedEvent(Lufia2DecompMapLoadEvent callback) {
    CpuEventRemove(&map_load_committed_event, callback);
}

int Lufia2DecompAddMusicFadeOutEvent(Lufia2DecompMusicFadeOutEvent callback) {
    const int added = CpuEventAdd(&music_fade_out_event, callback);
    interp_bridge_set_pre_opcode_hook(0x809692u, Lufia2DecompMusicFadeOut);
    return added;
}

void Lufia2DecompRemoveMusicFadeOutEvent(Lufia2DecompMusicFadeOutEvent callback) {
    CpuEventRemove(&music_fade_out_event, callback);
}

int Lufia2DecompAddSpellPriceStoredEvent(Lufia2DecompSpellPriceStoredEvent callback) {
    const int added = CpuEventAdd(&spell_price_stored_event, callback);
    interp_bridge_set_pre_opcode_hook(0x82d922u, Lufia2DecompSpellPriceStored);
    return added;
}

void Lufia2DecompRemoveSpellPriceStoredEvent(Lufia2DecompSpellPriceStoredEvent callback) {
    CpuEventRemove(&spell_price_stored_event, callback);
}

int Lufia2DecompAddPartyStatsDerivedEvent(Lufia2DecompPartyStatsDerivedEvent callback) {
    const int added = CpuEventAdd(&party_stats_derived_event, callback);
    interp_bridge_set_pre_opcode_hook(0x81f4e2u, Lufia2DecompPartyStatsDerived);
    return added;
}

void Lufia2DecompRemovePartyStatsDerivedEvent(Lufia2DecompPartyStatsDerivedEvent callback) {
    CpuEventRemove(&party_stats_derived_event, callback);
}

int Lufia2DecompAddItemReceivedEvent(Lufia2DecompItemReceivedEvent callback) {
    const int added = CpuEventAdd(&item_received_event, callback);
    interp_bridge_set_pre_opcode_hook(0x81f099u, Lufia2DecompItemReceived);
    return added;
}

void Lufia2DecompRemoveItemReceivedEvent(Lufia2DecompItemReceivedEvent callback) {
    CpuEventRemove(&item_received_event, callback);
}

int Lufia2DecompAddGameFileEvent(Lufia2DecompGameFileEvent callback) {
    const int added = GameFileEventAdd(&game_file_event, callback);
    interp_bridge_set_pre_opcode_hook(0x809099u, Lufia2DecompGameFile);
    interp_bridge_set_pre_opcode_hook(0x8090c9u, Lufia2DecompGameFile);
    interp_bridge_set_pre_opcode_hook(0x80914bu, Lufia2DecompGameFile);
    return added;
}

void Lufia2DecompRemoveGameFileEvent(Lufia2DecompGameFileEvent callback) {
    GameFileEventRemove(&game_file_event, callback);
}

int Lufia2DecompAddSongLoadObserver(Lufia2DecompSongLoadObserver callback) {
    const int added = CpuEventAdd(&song_load_observers, callback);
    interp_bridge_set_pre_opcode_hook(0x80942eu, InterpreterSongLoad);
    return added;
}

void Lufia2DecompRemoveSongLoadObserver(Lufia2DecompSongLoadObserver callback) {
    CpuEventRemove(&song_load_observers, callback);
}

void Lufia2DecompPartyStatTotals(CpuState *cpu, uint32_t pc) {
    CpuEventDispatch(&party_stat_totals_event, cpu, pc);
}

void Lufia2DecompSetPartyStatTotalsEvent(Lufia2DecompPartyStatTotalsEvent callback) {
    CpuEventSet(&party_stat_totals_event, callback);
    interp_bridge_set_pre_opcode_hook(0x81f576u, Lufia2DecompPartyStatTotals);
}

int Lufia2DecompAddPartyStatTotalsEvent(Lufia2DecompPartyStatTotalsEvent callback) {
    const int added = CpuEventAdd(&party_stat_totals_event, callback);
    interp_bridge_set_pre_opcode_hook(0x81f576u, Lufia2DecompPartyStatTotals);
    return added;
}

void Lufia2DecompRemovePartyStatTotalsEvent(Lufia2DecompPartyStatTotalsEvent callback) {
    CpuEventRemove(&party_stat_totals_event, callback);
}

void Lufia2DecompCaveExitBegin(CpuState *cpu, uint32_t pc) {
    CpuEventDispatch(&cave_exit_begin_event, cpu, pc);
}

void Lufia2DecompSetCaveExitBeginEvent(Lufia2DecompCaveExitEvent callback) {
    CpuEventSet(&cave_exit_begin_event, callback);
    interp_bridge_set_pre_opcode_hook(0x84890bu, Lufia2DecompCaveExitBegin);
}

int Lufia2DecompAddCaveExitBeginEvent(Lufia2DecompCaveExitEvent callback) {
    const int added = CpuEventAdd(&cave_exit_begin_event, callback);
    interp_bridge_set_pre_opcode_hook(0x84890bu, Lufia2DecompCaveExitBegin);
    return added;
}

void Lufia2DecompRemoveCaveExitBeginEvent(Lufia2DecompCaveExitEvent callback) {
    CpuEventRemove(&cave_exit_begin_event, callback);
}

void Lufia2DecompCaveExitRestored(CpuState *cpu, uint32_t pc) {
    CpuEventDispatch(&cave_exit_restored_event, cpu, pc);
}

void Lufia2DecompSetCaveExitRestoredEvent(Lufia2DecompCaveExitEvent callback) {
    CpuEventSet(&cave_exit_restored_event, callback);
    interp_bridge_set_pre_opcode_hook(0x848a38u, Lufia2DecompCaveExitRestored);
}

int Lufia2DecompAddCaveExitRestoredEvent(Lufia2DecompCaveExitEvent callback) {
    const int added = CpuEventAdd(&cave_exit_restored_event, callback);
    interp_bridge_set_pre_opcode_hook(0x848a38u, Lufia2DecompCaveExitRestored);
    return added;
}

void Lufia2DecompRemoveCaveExitRestoredEvent(Lufia2DecompCaveExitEvent callback) {
    CpuEventRemove(&cave_exit_restored_event, callback);
}
