#include "lufia2_party_limits.h"

#include <stdint.h>

#include "cpu_state.h"
#include "decomp_bridge/host_events.h"

enum {
    /* Member records packed by $85:C954 for a save. */
    MEMBER_FIRST = 0x0bad,
    MEMBER_SIZE = 0xbe,
    MEMBER_COUNT = 7,
    WRAM_BANK = 0x7e,
    /* $85:CB17 keeps two high bits of each. */
    MEMBER_HP = 0x11,
    MEMBER_MP = 0x13,
    MEMBER_MAX_HP = 0x25,
    MEMBER_MAX_MP = 0x27,
    /* Three menu digits; $82:91AB shows no more. */
    STAT_LIMIT = 999,
};

static const uint8_t kLimitedFields[] = {
    MEMBER_HP, MEMBER_MP, MEMBER_MAX_HP, MEMBER_MAX_MP};

static void LimitWord(CpuState *cpu, uint8_t bank, uint16_t address) {
    const uint16_t high = (uint16_t)(address + 1u);
    const uint16_t value = (uint16_t)(cpu_read8(cpu, bank, address) |
                                      cpu_read8(cpu, bank, high) << 8);
    if (value <= STAT_LIMIT)
        return;
    cpu_write8(cpu, bank, address, (uint8_t)(STAT_LIMIT & 0xff));
    cpu_write8(cpu, bank, high, (uint8_t)(STAT_LIMIT >> 8));
}

static void LimitRecord(CpuState *cpu, uint8_t bank, uint16_t record) {
    for (unsigned i = 0; i < sizeof(kLimitedFields); i++)
        LimitWord(cpu, bank, (uint16_t)(record + kLimitedFields[i]));
}

/* $81:F4ED adds the bonus to base HP and MP uncapped. */
static void LimitTotals(CpuState *cpu, uint32_t pc24) {
    (void)pc24;
    LimitRecord(cpu, cpu->DB, cpu->X);
}

/* Values past 1023 lose bits and corrupt MP in the save. */
static void LimitBeforeSave(CpuState *cpu, uint32_t pc24,
                            Lufia2DecompGameFileOperation operation) {
    (void)pc24;
    if (operation != LUFIA2_DECOMP_GAME_FILE_SAVE)
        return;
    for (unsigned member = 0; member < MEMBER_COUNT; member++)
        LimitRecord(cpu, WRAM_BANK,
                    (uint16_t)(MEMBER_FIRST + member * MEMBER_SIZE));
}

void Lufia2PartyLimitsInstall(void) {
    Lufia2DecompAddPartyStatTotalsEvent(LimitTotals);
    Lufia2DecompAddGameFileEvent(LimitBeforeSave);
}
