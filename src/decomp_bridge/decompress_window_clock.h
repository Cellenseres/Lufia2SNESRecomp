#ifndef LUFIA2_DECOMPRESS_WINDOW_CLOCK_H
#define LUFIA2_DECOMPRESS_WINDOW_CLOCK_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Valid only within the SDK's current pre-opcode callback. */
bool Lufia2DecompDecompressWindowReady(void);

#ifdef __cplusplus
}
#endif

#endif
