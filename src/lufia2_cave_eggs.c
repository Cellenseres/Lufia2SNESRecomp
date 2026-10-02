#include "lufia2_cave_eggs.h"

#include <stdint.h>

#include "cpu_state.h"
#include "decomp_bridge/host_events.h"

enum {
    /* Item words: id | count << 9. */
    INVENTORY = 0x0a8d,
    INVENTORY_BYTES = 0xc0,
    ITEM_ID_MASK = 0x01ff,
    ITEM_COUNT_SHIFT = 9,
    ITEM_COUNT_MAX = 0x7f,
    DRAGON_EGG = 0x02b,
    /* Eggs found, bumped by $8E:C1C5. */
    EGGS_FOUND = 0x0b61,
    /* $FF after a cave defeat; the exit then returns $7F:BF00. */
    CAVE_DEFEATED = 0x1e759,
    RETURN_LIST = 0x1bf00,
    RETURN_LIST_BYTES = 0x100,
    /* Carry list the exit fills with Iris items. */
    CARRY_COUNT = 0xe200,
    CARRY_ENTRIES = 0xe202,
};

extern uint8_t g_ram[0x20000];

static unsigned s_eggs;
static unsigned s_defeat_eggs;

static uint16_t Word(uint32_t address) {
    return (uint16_t)(g_ram[address] | g_ram[address + 1] << 8);
}

static void SetWord(uint32_t address, uint16_t value) {
    g_ram[address] = (uint8_t)value;
    g_ram[address + 1] = (uint8_t)(value >> 8);
}

static uint16_t EggWord(unsigned count) {
    return (uint16_t)(DRAGON_EGG | count << ITEM_COUNT_SHIFT);
}

static unsigned InventoryEggs(void) {
    unsigned eggs = 0;
    for (unsigned slot = 0; slot < INVENTORY_BYTES; slot += 2) {
        const uint16_t item = Word(INVENTORY + slot);
        if ((item & ITEM_ID_MASK) == DRAGON_EGG)
            eggs += item >> ITEM_COUNT_SHIFT;
    }
    return eggs > ITEM_COUNT_MAX ? ITEM_COUNT_MAX : eggs;
}

/* The cave inventory is still the one the party carried. */
static void CountCaveEggs(CpuState *cpu, uint32_t pc24) {
    (void)cpu;
    (void)pc24;
    s_eggs = InventoryEggs();
}

/* $84:8888 is about to clear the cave inventory. */
static void CountEggsBeforeDefeat(CpuState *cpu, uint32_t pc24) {
    (void)cpu;
    (void)pc24;
    s_defeat_eggs = InventoryEggs();
}

/* Back into the cleared inventory, so the exit finds them. */
static void KeepEggsAfterDefeat(CpuState *cpu, uint32_t pc24) {
    (void)cpu;
    (void)pc24;
    if (s_defeat_eggs == 0)
        return;
    for (unsigned slot = 0; slot < INVENTORY_BYTES; slot += 2) {
        if (Word(INVENTORY + slot) != 0)
            continue;
        SetWord(INVENTORY + slot, EggWord(s_defeat_eggs));
        break;
    }
    s_defeat_eggs = 0;
}

static void AppendToReturnList(void) {
    for (uint32_t at = RETURN_LIST; at + 4 <= RETURN_LIST + RETURN_LIST_BYTES;
         at += 2) {
        if (Word(at) != 0)
            continue;
        SetWord(at, EggWord(s_eggs));
        SetWord(at + 2, 0);
        return;
    }
}

static void AppendToCarryList(void) {
    const uint16_t count = Word(CARRY_COUNT);
    SetWord(CARRY_ENTRIES + 2u * count, EggWord(s_eggs));
    SetWord(CARRY_COUNT, (uint16_t)(count + 1));
}

/* The restore dropped them; hand them over like Iris items. */
static void CarryEggsOut(CpuState *cpu, uint32_t pc24) {
    (void)cpu;
    (void)pc24;
    if (s_eggs == 0)
        return;
    if (g_ram[CAVE_DEFEATED])
        AppendToReturnList();
    else
        AppendToCarryList();
    const uint32_t found = Word(EGGS_FOUND) + s_eggs;
    SetWord(EGGS_FOUND, (uint16_t)(found > 0xffff ? 0xffff : found));
    s_eggs = 0;
}

void Lufia2CaveEggsInstall(void) {
    Lufia2DecompAddCaveDefeatBeginEvent(CountEggsBeforeDefeat);
    Lufia2DecompAddCaveDefeatResetEvent(KeepEggsAfterDefeat);
    Lufia2DecompAddCaveExitBeginEvent(CountCaveEggs);
    Lufia2DecompAddCaveExitRestoredEvent(CarryEggsOut);
}
