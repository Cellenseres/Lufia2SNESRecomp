#include "lufia2_party_limits.h"

#include <stdint.h>

#include "cpu_state.h"
#include "decomp_bridge/host_events.h"

enum {
    /* Member records packed by $85:C954 for a save. */
    MEMBER_FIRST = 0x0bad,
    MEMBER_SIZE = 0xbe,
    MEMBER_COUNT = 7,
    /* $85:CB17 keeps two high bits of each. */
    MEMBER_HP = 0x11,
    MEMBER_MP = 0x13,
    MEMBER_MAX_HP = 0x25,
    MEMBER_MAX_MP = 0x27,
    /* Three menu digits; $82:91AB shows no more. */
    STAT_LIMIT = 999,
};

extern uint8_t g_ram[0x20000];

static void LimitWord(uint16_t address) {
    const uint16_t value = (uint16_t)(g_ram[address] | g_ram[address + 1u] << 8);
    if (value <= STAT_LIMIT)
        return;
    g_ram[address] = (uint8_t)(STAT_LIMIT & 0xff);
    g_ram[address + 1u] = (uint8_t)(STAT_LIMIT >> 8);
}

/* Values past 1023 lose bits and corrupt MP in the save. */
static void LimitBeforeSave(CpuState *cpu, uint32_t pc24,
                            Lufia2DecompGameFileOperation operation) {
    (void)cpu;
    (void)pc24;
    if (operation != LUFIA2_DECOMP_GAME_FILE_SAVE)
        return;
    static const uint8_t kFields[] = {
        MEMBER_HP, MEMBER_MP, MEMBER_MAX_HP, MEMBER_MAX_MP};
    for (unsigned member = 0; member < MEMBER_COUNT; member++) {
        const uint16_t record = (uint16_t)(MEMBER_FIRST + member * MEMBER_SIZE);
        for (unsigned i = 0; i < sizeof(kFields); i++)
            LimitWord((uint16_t)(record + kFields[i]));
    }
}

void Lufia2PartyLimitsInstall(void) {
    Lufia2DecompSetGameFileEvent(LimitBeforeSave);
}
