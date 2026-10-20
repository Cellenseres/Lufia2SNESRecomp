#include "lufia2_shrine_graphics.h"

#include <stdlib.h>

#include "common/ips_patch.h"
#include "lufia2_shrine_patch.h"

bool Lufia2ShrineGraphicsApply(uint8_t **rom, uint32_t *size) {
    uint8_t *repaired = NULL;
    size_t repaired_size = 0;
    if (!rom || !*rom || !size ||
        !bps_apply(*rom, *size, kLufia2ShrinePatch, sizeof kLufia2ShrinePatch,
                   &repaired, &repaired_size))
        return false;
    if (repaired_size < *size || repaired_size > *size + 65536u) {
        free(repaired);
        return false;
    }
    free(*rom);
    *rom = repaired;
    *size = (uint32_t)repaired_size;
    return true;
}
