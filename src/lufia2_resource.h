#ifndef LUFIA2_RESOURCE_H
#define LUFIA2_RESOURCE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    LUFIA2_US_ROM_SIZE = 0x280000,
};

typedef struct Lufia2ResourceView {
    uint8_t *data;
    size_t size;
} Lufia2ResourceView;

bool Lufia2ResourceExtract(const uint8_t *rom,
                           size_t rom_size,
                           unsigned resource_id,
                           Lufia2ResourceView *out);

void Lufia2ResourceDestroy(Lufia2ResourceView *resource);

#endif
