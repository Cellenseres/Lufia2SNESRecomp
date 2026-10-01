#include "lufia2_spell_price.h"

#include <stdint.h>

#include "cpu_state.h"
#include "snes/interp_bridge.h"

enum {
    /* Right after STX $09BA in the spell shop's price setup. */
    SPELL_PRICE_STORED = 0x82d922,
    /* 24-bit purchase price; item totals reach this byte. */
    PURCHASE_PRICE_HIGH = 0x09bc,
};

extern uint8_t g_ram[0x20000];

/* $82:D905 misses the STZ $09BC every other price setter has. */
static void ClearSpellPriceHigh(CpuState *cpu, uint32_t pc24) {
    (void)cpu;
    (void)pc24;
    g_ram[PURCHASE_PRICE_HIGH] = 0;
}

void Lufia2SpellPriceInstallHooks(void) {
    interp_bridge_set_pre_opcode_hook(SPELL_PRICE_STORED, ClearSpellPriceHigh);
}
