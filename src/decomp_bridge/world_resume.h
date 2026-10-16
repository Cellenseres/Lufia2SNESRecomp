#ifndef LUFIA2_SCENE_RESUME_H
#define LUFIA2_SCENE_RESUME_H
#include <stdint.h>
#include "interp816.h"

static inline uint32_t Lufia2SceneResumeReturn(const Interp816 *cpu,
                                               uint32_t return_site,
                                               const uint8_t *ram) {
    if (return_site != 0x869757u || cpu->e || cpu->d || !cpu->mf || cpu->xf ||
        cpu->k != 0x86u || cpu->db != 0x86u || cpu->dp || cpu->sp < 0x1f20u ||
        cpu->sp > 0x1ffcu || !ram)
        return 0u;
    switch (cpu->pc) {
    case 0xc1e3u:
    case 0xc237u:
    case 0xb38eu:
    case 0xb391u:
    case 0xb394u:
    case 0xb397u:
        return 0x860000u |
               (uint16_t)((ram[cpu->sp + 1u] | (uint16_t)ram[cpu->sp + 2u] << 8) + 1u);
    default:
        return 0u;
    }
}
#endif
