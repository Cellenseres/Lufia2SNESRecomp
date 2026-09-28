#include "lufia2_resource.h"

#include "lufia2/resource_format.h"

#include <stdlib.h>

static size_t RomOffset(uint32_t address) {
    return (size_t)((address >> 16) & 0x7fu) * 0x8000u +
           (size_t)(address & 0x7fffu);
}

static bool ResourcePointer(const uint8_t *rom, size_t rom_size,
                            unsigned resource_id, size_t *offset_out) {
    const size_t table = RomOffset(LUFIA2_RESOURCE_TABLE_ADDRESS);
    size_t entry;

    if (!rom || !offset_out || resource_id >= LUFIA2_RESOURCE_COUNT ||
        rom_size != LUFIA2_US_ROM_SIZE)
        return false;
    if (table > rom_size ||
        (size_t)resource_id > (rom_size - table) / LUFIA2_RESOURCE_ENTRY_SIZE)
        return false;
    entry = table + (size_t)resource_id * LUFIA2_RESOURCE_ENTRY_SIZE;
    if (rom_size - entry < LUFIA2_RESOURCE_ENTRY_SIZE)
        return false;

    *offset_out = RomOffset(Lufia2ResourceStreamAddress(rom + entry));
    return *offset_out <= rom_size;
}

static bool ResourceLocate(const uint8_t *rom, size_t rom_size,
                           unsigned resource_id, size_t *start_out,
                           size_t *end_out) {
    size_t start;
    size_t end = rom_size;

    if (!start_out || !end_out ||
        !ResourcePointer(rom, rom_size, resource_id, &start))
        return false;
    if (resource_id + 1u < LUFIA2_RESOURCE_COUNT &&
        !ResourcePointer(rom, rom_size, resource_id + 1u, &end))
        return false;
    if (start > end)
        return false;

    *start_out = start;
    *end_out = end;
    return true;
}

bool Lufia2ResourceExtract(const uint8_t *rom, size_t rom_size,
                           unsigned resource_id, Lufia2ResourceView *out) {
    size_t start;
    size_t end;
    size_t decoded_size;
    uint8_t *data;

    if (!out)
        return false;
    *out = (Lufia2ResourceView){0};
    if (!ResourceLocate(rom, rom_size, resource_id, &start, &end) ||
        Lufia2ResourceDecodedSize(rom + start, end - start, &decoded_size) !=
            LUFIA2_RESOURCE_OK)
        return false;
    if (decoded_size == 0u)
        return true;

    data = (uint8_t *)malloc(decoded_size);
    if (!data)
        return false;
    if (Lufia2ResourceDecode(rom + start, end - start, data, decoded_size,
                             NULL) != LUFIA2_RESOURCE_OK) {
        free(data);
        return false;
    }
    out->data = data;
    out->size = decoded_size;
    return true;
}

void Lufia2ResourceDestroy(Lufia2ResourceView *resource) {
    if (!resource)
        return;
    free(resource->data);
    *resource = (Lufia2ResourceView){0};
}
