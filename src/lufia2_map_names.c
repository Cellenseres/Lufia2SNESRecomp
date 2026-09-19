/* Lufia II location names, decoded from the ROM's own table. */
#include "lufia2_map_names.h"

#include <string.h>

#include "lufia2_resource.h"

enum {
    /* NUL-terminated ASCII in bank $07. Entry N+1 names map N;
     * the first two are the world map and the seafloor. */
    NAME_TABLE = 0x038810,
    NAME_COUNT = 242,

    /* $0A starts a back-reference: 12 address bits and a length. */
    NAME_REFERENCE = 0x0A,
    REFERENCE_DEPTH_LIMIT = 8,

    NAME_CAPACITY = 32,
};

static char s_names[NAME_COUNT][NAME_CAPACITY];
static bool s_ready;

static size_t decode_entry(const uint8_t *rom, size_t rom_size, size_t offset,
                           char *out, size_t cap, int depth);

/* Append at most `limit` characters of the entry at `source`. */
static void append_reference(const uint8_t *rom, size_t rom_size,
                             size_t source, size_t limit,
                             char *out, size_t cap, size_t *used, int depth) {
    char quoted[NAME_CAPACITY];
    decode_entry(rom, rom_size, source, quoted, sizeof(quoted), depth + 1);
    for (size_t i = 0; i < limit && quoted[i]; i++) {
        if (*used + 1 >= cap) return;
        out[(*used)++] = quoted[i];
    }
}

static size_t decode_entry(const uint8_t *rom, size_t rom_size, size_t offset,
                           char *out, size_t cap, int depth) {
    size_t used = 0;

    if (cap) out[0] = '\0';
    while (offset < rom_size) {
        const uint8_t byte = rom[offset];

        if (byte == 0x00) {
            offset++;
            break;
        }
        if (byte == NAME_REFERENCE && depth < REFERENCE_DEPTH_LIMIT &&
                offset + 2 < rom_size) {
            const uint8_t low = rom[offset + 1];
            const uint8_t high = rom[offset + 2];
            size_t source = (offset & ~(size_t)0xFFF) |
                            ((size_t)(high & 0x0Fu) << 8) | low;
            /* References only point back; that supplies the missing bits. */
            if (source > offset) source -= 0x1000u;
            append_reference(rom, rom_size, source, (size_t)(high >> 4) + 2u,
                             out, cap, &used, depth);
            offset += 3;
            continue;
        }
        if (byte >= 0x20 && byte < 0x7F && used + 1 < cap)
            out[used++] = (char)byte;
        offset++;
    }

    if (cap) out[used < cap ? used : cap - 1] = '\0';
    return offset;
}

bool Lufia2MapNamesInit(const uint8_t *rom, size_t rom_size) {
    if (s_ready) return true;
    if (!rom || rom_size != LUFIA2_US_ROM_SIZE || NAME_TABLE >= rom_size)
        return false;

    size_t offset = NAME_TABLE;
    for (int i = 0; i < NAME_COUNT; i++)
        offset = decode_entry(rom, rom_size, offset, s_names[i],
                              NAME_CAPACITY, 0);

    s_ready = true;
    return true;
}

const char *Lufia2MapName(unsigned map_id) {
    /* $00 is not a regular map; the ROM's entry 1 would say
     * "Map of Seafloor", which is wrong for the overworld. */
    if (map_id == 0u) return "Overworld";
    if (!s_ready || map_id + 1u >= (unsigned)NAME_COUNT)
        return NULL;
    const char *name = s_names[map_id + 1u];
    return name[0] ? name : NULL;
}
