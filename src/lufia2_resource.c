#include "lufia2_resource.h"

#include <stddef.h>
#include <stdlib.h>

enum {
    LUFIA2_RESOURCE_COUNT = 680,
    LUFIA2_RESOURCE_TABLE_FILE_OFFSET = 0x138000,
    LUFIA2_RESOURCE_MAX_DECODED_SIZE = 0x10000,
};

static uint16_t ReadU16Le(const uint8_t *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static size_t ReadU24Le(const uint8_t *p) {
    return (size_t)p[0] |
           ((size_t)p[1] << 8) |
           ((size_t)p[2] << 16);
}

static bool ResourcePointer(const uint8_t *rom,
                            size_t rom_size,
                            unsigned resource_id,
                            size_t *address_out) {
    size_t entry;
    size_t relative;

    if (!rom || !address_out || resource_id >= LUFIA2_RESOURCE_COUNT ||
        rom_size != LUFIA2_US_ROM_SIZE)
        return false;

    if ((size_t)resource_id >
        (SIZE_MAX - LUFIA2_RESOURCE_TABLE_FILE_OFFSET) / 3u)
        return false;
    entry = LUFIA2_RESOURCE_TABLE_FILE_OFFSET + (size_t)resource_id * 3u;
    if (entry > rom_size || rom_size - entry < 3u)
        return false;

    relative = ReadU24Le(rom + entry);
    if (relative > rom_size - LUFIA2_RESOURCE_TABLE_FILE_OFFSET)
        return false;
    *address_out = LUFIA2_RESOURCE_TABLE_FILE_OFFSET + relative;
    return true;
}

static bool ResourceLocate(const uint8_t *rom,
                           size_t rom_size,
                           unsigned resource_id,
                           size_t *start_out,
                           size_t *end_out) {
    size_t start;
    size_t end = rom_size;

    if (start_out) *start_out = 0;
    if (end_out) *end_out = 0;
    if (!start_out || !end_out ||
        !ResourcePointer(rom, rom_size, resource_id, &start))
        return false;
    if (resource_id + 1u < LUFIA2_RESOURCE_COUNT &&
        !ResourcePointer(rom, rom_size, resource_id + 1u, &end))
        return false;
    if (start > end || end > rom_size)
        return false;

    *start_out = start;
    *end_out = end;
    return true;
}

static bool ResourceDecompress(const uint8_t *compressed,
                               size_t compressed_size,
                               size_t max_decoded_size,
                               Lufia2ResourceView *out) {
    uint8_t *data = NULL;
    size_t position = 0;
    size_t written = 0;
    size_t decoded_size;
    uint8_t flags;
    unsigned flag_bits = 8;

    if (!out)
        return false;
    *out = (Lufia2ResourceView){0};
    if (!compressed || compressed_size < 2u)
        return false;

    decoded_size = ReadU16Le(compressed);
    position = 2u;
    if (decoded_size > max_decoded_size ||
        decoded_size > LUFIA2_RESOURCE_MAX_DECODED_SIZE)
        return false;
    if (decoded_size == 0u)
        return true;
    if (position >= compressed_size)
        return false;

    data = (uint8_t *)malloc(decoded_size);
    if (!data)
        return false;
    flags = compressed[position++];

    while (written < decoded_size) {
        uint8_t value;

        if (position >= compressed_size)
            goto fail;
        value = compressed[position++];
        if ((value & 0x80u) == 0u) {
            data[written++] = value;
            continue;
        }

        {
            const bool is_copy = (flags & 0x80u) != 0u;
            flags = (uint8_t)(flags << 1);
            flag_bits--;

            if (!is_copy) {
                data[written++] = value;
            } else {
                uint16_t packed;
                uint16_t encoded_offset;
                unsigned copies;
                ptrdiff_t signed_offset;
                ptrdiff_t source;

                if (position >= compressed_size)
                    goto fail;
                packed = (uint16_t)(((uint16_t)value << 8) |
                                    compressed[position++]);
                if ((packed & 0x0Fu) != 0u) {
                    copies = (packed & 0x0Fu) + 2u;
                    encoded_offset =
                        (uint16_t)((packed >> 4) | 0xF000u);
                } else {
                    uint8_t extra;
                    if (position >= compressed_size)
                        goto fail;
                    extra = compressed[position++];
                    copies = (extra & 0x3Fu) + 3u;
                    encoded_offset = (uint16_t)(
                        (((uint32_t)((packed >> 4) | 0xF000u) << 2) |
                         (extra >> 6)) & 0xFFFFu);
                }

                signed_offset = (encoded_offset & 0x8000u)
                    ? (ptrdiff_t)encoded_offset - 0x10000
                    : (ptrdiff_t)encoded_offset;
                source = (ptrdiff_t)written + signed_offset;
                for (unsigned i = 0;
                     i < copies && written < decoded_size;
                     i++, source++) {
                    if (source < 0 || (size_t)source >= written)
                        goto fail;
                    data[written++] = data[(size_t)source];
                }
            }
        }

        /* LEdit eagerly fetched here. Do not cross the resource boundary once
         * the declared output has been completed. */
        if (flag_bits == 0u && written < decoded_size) {
            if (position >= compressed_size)
                goto fail;
            flags = compressed[position++];
            flag_bits = 8u;
        }
    }

    out->data = data;
    out->size = decoded_size;
    return true;

fail:
    free(data);
    return false;
}

bool Lufia2ResourceExtract(const uint8_t *rom,
                           size_t rom_size,
                           unsigned resource_id,
                           Lufia2ResourceView *out) {
    size_t start;
    size_t end;

    if (!out)
        return false;
    *out = (Lufia2ResourceView){0};
    if (!ResourceLocate(
            rom, rom_size, resource_id, &start, &end))
        return false;
    return ResourceDecompress(
        rom + start, end - start, LUFIA2_RESOURCE_MAX_DECODED_SIZE, out);
}

void Lufia2ResourceDestroy(Lufia2ResourceView *resource) {
    if (!resource)
        return;
    free(resource->data);
    *resource = (Lufia2ResourceView){0};
}
