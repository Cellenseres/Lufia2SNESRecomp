#ifndef LUFIA2_NMI_OWNER_H
#define LUFIA2_NMI_OWNER_H

#include <stdbool.h>

bool Lufia2DecompNmiHasInterruptOwner(void);
void Lufia2DecompNmiInstallTickHook(void);

#endif
