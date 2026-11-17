#ifndef LUFIA2_BATTLE_OUTPUT_RANGE_H
#define LUFIA2_BATTLE_OUTPUT_RANGE_H

#include <stdbool.h>

static bool BattleWindowOutputSupported(unsigned start, unsigned end) {
    return end <= 0x10000u && (end <= 0x09f2u || start >= 0x0a00u) &&
           (end <= 0x0a3cu || start >= 0x0a3eu) && (end <= 0x1efcu || start >= 0x2000u);
}

#endif
