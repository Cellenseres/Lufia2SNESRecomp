#ifndef LUFIA2_FIELD_RESUME_H
#define LUFIA2_FIELD_RESUME_H
#include <stdint.h>
#include "interp816.h"

static inline uint32_t Lufia2FieldResumeReturn(const Interp816 *cpu,
                                               uint32_t return_site) {
    if (return_site != 0x839012u || cpu->e || cpu->d || !cpu->mf || cpu->k != 0x83u ||
        cpu->db != 0x83u || cpu->dp || cpu->sp != 0x1fffu || cpu->pc != 0x8007u)
        return 0u;
    return 0x838007u;
}
#endif
