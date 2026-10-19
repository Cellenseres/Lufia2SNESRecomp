#ifndef LUFIA2_ROOM_INFLATE_H
#define LUFIA2_ROOM_INFLATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Exact, bounded zlib stream. Uses the output buffer as its history window. */
bool Lufia2RoomInflate(const uint8_t *input, size_t input_size,
                      uint8_t *output, size_t output_size);

#endif
