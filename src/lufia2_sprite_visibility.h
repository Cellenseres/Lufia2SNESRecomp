#pragma once

#include "lufia2/field.h"

/* Consumer-owned widescreen policy over the shared semantic sprite builder. */
Lufia2ExecutionResult Lufia2PatchedFieldActorSprites(
    const Lufia2Memory *memory, Lufia2CpuState *cpu);
