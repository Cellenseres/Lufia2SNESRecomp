#pragma once
#include "lufia2/actor.h"
#include <stddef.h>

/* Consumer ABI: SDK enum defaults must never change the result structures
 * shared with the standalone semantic provider. The semantic code stays SDK
 * independent; each console selects this ABI at its compilation boundary. */
#if defined(__cplusplus)
#define LUFIA2_ABI_ASSERT(condition, message) static_assert(condition, message)
#else
#define LUFIA2_ABI_ASSERT(condition, message) _Static_assert(condition, message)
#endif
LUFIA2_ABI_ASSERT(sizeof(Lufia2ExecutionFlow) == 4, "decomp execution enum ABI must be int-sized");
LUFIA2_ABI_ASSERT(sizeof(Lufia2ActorPrimaryScriptStepFlow) == 4, "decomp actor enum ABI must be int-sized");
LUFIA2_ABI_ASSERT(offsetof(Lufia2ExecutionResult, pc) == 4 &&
    sizeof(Lufia2ExecutionResult) == 12, "decomp execution result ABI changed");
LUFIA2_ABI_ASSERT(offsetof(Lufia2ActorPrimaryScriptStepResult, opcode) == 4 &&
    offsetof(Lufia2ActorPrimaryScriptStepResult, handler_pc) == 8 &&
    sizeof(Lufia2ActorPrimaryScriptStepResult) == 12, "decomp actor result ABI changed");
#undef LUFIA2_ABI_ASSERT
