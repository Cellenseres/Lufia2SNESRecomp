#include "lufia2_play_time.h"

#include <stdbool.h>
#include <stdint.h>

#include "cpu_state.h"
#include "decomp_bridge/host_events.h"

enum {
    /* Play clock kept by the $80:8638 NMI. */
    PLAY_TIME_HOURS = 0x0b4d,
    PLAY_TIME_MINUTES = 0x0b4e,
    PLAY_TIME_SECONDS = 0x0b4f,
    PLAY_TIME_FRAMES = 0x0b50,
    PLAY_TIME_STOPPED = 0x80,
    /* The saved hour byte, saturated by $84:8AD3 as well. */
    PLAY_TIME_MAX_HOURS = 255,
    PLAY_TIME_MAX_MINUTES = 59,
    PLAY_TIME_MAX_SECONDS = 59,
    /* Menu text state of op $01 at $80:8922. */
    MENU_STRING = 0x5d,
    MENU_STRING_BANK = 0x5f,
    NUMBER_START = 0x54,
    NUMBER_POINTER = 0x56,
    MENU_CURSOR = 0x0575,
    MENU_TILE_ROW = 0x40,
    MENU_TILE_BYTES = 2,
    MENU_SPACE = 0x20,
    /* Longest label moved: "TIME " plus one. */
    MENU_LABEL_SCAN = 8,
    /* Tilemap rows are 32 tiles. */
    MENU_ROW_MASK = 0xffc0,
    HOUR_TEXT_BANK = 0x8e,
    TWO_DIGITS = 6,
    THREE_DIGITS = 5,
};

/* End of each hour field's op $01 in bank $8E. */
static const uint16_t kHourFields[] = {
    0xca6a, /* file select */
    0xd50d, /* menu and save TIME box */
    0xd6e5, /* records play time */
    0xd7e9, /* records challenge time */
};

extern uint8_t g_ram[0x20000];

static uint16_t Word(uint16_t address) {
    return (uint16_t)(g_ram[address] | g_ram[(uint16_t)(address + 1u)] << 8);
}

static void SetWord(uint16_t address, uint16_t value) {
    g_ram[address] = (uint8_t)value;
    g_ram[(uint16_t)(address + 1u)] = (uint8_t)(value >> 8);
}

static uint16_t DirectWord(const CpuState *cpu, uint8_t offset) {
    return Word((uint16_t)(cpu->D + offset));
}

/* Holding the frame count stops the clock at 255:59:59. */
static void HoldPlayTime(CpuState *cpu, uint32_t pc24) {
    (void)cpu;
    (void)pc24;
    if (g_ram[PLAY_TIME_HOURS] == PLAY_TIME_MAX_HOURS &&
        g_ram[PLAY_TIME_MINUTES] == PLAY_TIME_MAX_MINUTES &&
        g_ram[PLAY_TIME_SECONDS] == PLAY_TIME_MAX_SECONDS)
        g_ram[PLAY_TIME_FRAMES] &= PLAY_TIME_STOPPED;
}

static bool IsHourField(const CpuState *cpu) {
    if (g_ram[(uint16_t)(cpu->D + MENU_STRING_BANK)] != HOUR_TEXT_BANK)
        return false;
    const uint16_t field = (uint16_t)(DirectWord(cpu, MENU_STRING) + cpu->Y);
    for (unsigned i = 0; i < sizeof(kHourFields) / sizeof(kHourFields[0]); i++) {
        if (kHourFields[i] == field)
            return true;
    }
    return false;
}

static uint16_t TileAt(uint16_t cursor) {
    return (uint16_t)(cursor + MENU_TILE_ROW);
}

/* A space, or a tile the menu cleared. */
static bool TileIsBlank(uint16_t tile) {
    return g_ram[tile] == MENU_SPACE || Word(tile) == 0;
}

/* Third digit grows left; the label moves along. */
static void MakeRoomOnTheLeft(void) {
    const uint16_t cursor = Word(MENU_CURSOR);
    const uint16_t last = (uint16_t)(cursor - MENU_TILE_BYTES);
    uint16_t blank = last;

    for (unsigned tiles = 2; tiles <= MENU_LABEL_SCAN; tiles++) {
        const uint16_t tile = (uint16_t)(cursor - tiles * MENU_TILE_BYTES);
        if ((tile & MENU_ROW_MASK) != (last & MENU_ROW_MASK))
            return;
        if (TileIsBlank(TileAt(tile))) {
            blank = tile;
            break;
        }
    }
    if (blank == last)
        return;
    for (uint16_t tile = blank; tile != last; tile += MENU_TILE_BYTES)
        SetWord(TileAt(tile), Word(TileAt((uint16_t)(tile + MENU_TILE_BYTES))));
    SetWord(MENU_CURSOR, last);
}

/* Hours past 99 printed as 00 in the original. */
static void WidenHourDigits(CpuState *cpu, uint32_t pc24) {
    (void)pc24;
    uint8_t *start = &g_ram[(uint16_t)(cpu->D + NUMBER_START)];
    if (*start != TWO_DIGITS || !IsHourField(cpu) ||
        g_ram[DirectWord(cpu, NUMBER_POINTER)] < 100)
        return;
    *start = THREE_DIGITS;
    MakeRoomOnTheLeft();
}

void Lufia2PlayTimeInstall(void) {
    Lufia2DecompSetPlayTimeTickEvent(HoldPlayTime);
    Lufia2DecompSetMenuNumberEvent(WidenHourDigits);
}
