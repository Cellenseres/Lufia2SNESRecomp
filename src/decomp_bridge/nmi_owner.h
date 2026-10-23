#ifndef LUFIA2_NMI_OWNER_H
#define LUFIA2_NMI_OWNER_H

#include <stdbool.h>

enum {
    LUFIA2_NMI_PRIMARY_WAVE_OPCODES = 555u,
    LUFIA2_NMI_RIPPLE_ROW_OPCODES = 277u,
    LUFIA2_NMI_WAVE_FILL_WORD_OPCODES = 15u
};

bool Lufia2DecompNmiHasInterruptOwner(void);
void Lufia2DecompNmiInstallTickHook(void);

#endif
