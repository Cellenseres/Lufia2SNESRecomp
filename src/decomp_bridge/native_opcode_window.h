#ifndef LUFIA2_NATIVE_OPCODE_WINDOW_H
#define LUFIA2_NATIVE_OPCODE_WINDOW_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Valid only within the SDK's current pre-opcode callback. */
bool Lufia2DecompOpcodeWindowReady(void);

#ifdef __cplusplus
}
#endif

#endif
