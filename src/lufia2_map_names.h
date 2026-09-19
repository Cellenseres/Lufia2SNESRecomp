#ifndef LUFIA2_MAP_NAMES_H
#define LUFIA2_MAP_NAMES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Decode the location names the game shows, from the verified US ROM.
 * Safe to call more than once; a second call is a no-op. */
bool Lufia2MapNamesInit(const uint8_t *headerless_rom, size_t rom_size);

/* The name for a map id ($05AC), or NULL when it has none. 21 of the 240
 * ids are blank in the ROM. $00 is not a regular map and reads "Overworld". */
const char *Lufia2MapName(unsigned map_id);

#endif
