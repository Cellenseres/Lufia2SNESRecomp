#ifndef LUFIA2_SHRINE_GRAPHICS_H
#define LUFIA2_SHRINE_GRAPHICS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Replace the verified ROM buffer; the source file is never written. */
bool Lufia2ShrineGraphicsApply(uint8_t **rom, uint32_t *size);

#ifdef __cplusplus
}
#endif

#endif
