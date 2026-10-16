#include <stdio.h>
#include <string.h>
#include "decomp_bridge/world_resume.h"

static uint8_t ram[0x20000];
static unsigned checks;

static int check(Interp816 *cpu, uint32_t site, uint32_t expected) {
    Interp816 before = *cpu;
    uint32_t actual = Lufia2SceneResumeReturn(cpu, site, ram);
    ++checks;
    if (actual != expected || memcmp(&before, cpu, sizeof before)) {
        fprintf(stderr, "resume gate site%06X target%04X expected%06X actual%06X\n",
                site, cpu->pc, expected, actual);
        return 1;
    }
    return 0;
}

int main(void) {
    static const uint16_t entries[] = {0xc1e3, 0xc237, 0xb38e, 0xb391, 0xb394, 0xb397};
    Interp816 cpu = {0};
    cpu.mf = 1;
    cpu.k = cpu.db = 0x86;
    cpu.sp = 0x1f20;
    ram[0x1f21] = 0x67;
    ram[0x1f22] = 0x45;
    for (unsigned pc = 0; pc < 65536u; ++pc) {
        unsigned valid = 0;
        for (unsigned j = 0; j < 6u; ++j)
            valid |= pc == entries[j];
        cpu.pc = (uint16_t)pc;
        if (check(&cpu, 0x869757, valid ? 0x864568u : 0))
            return 1;
    }
    for (unsigned j = 0; j < 6u; ++j) {
        cpu.pc = entries[j];
        for (unsigned pc = 0; pc < 65536u; ++pc)
            if (check(&cpu, 0x860000u | pc, pc == 0x9757 ? 0x864568u : 0))
                return 1;
        for (unsigned flag = 0; flag < 8u; ++flag) {
            Interp816 bad = cpu;
            switch (flag) {
            case 0:
                bad.e = 1;
                break;
            case 1:
                bad.d = 1;
                break;
            case 2:
                bad.mf = 0;
                break;
            case 3:
                bad.xf = 1;
                break;
            case 4:
                bad.k = 0x85;
                break;
            case 5:
                bad.db = 0x7f;
                break;
            case 6:
                bad.dp = 1;
                break;
            case 7:
                bad.dp = 0xffff;
                break;
            }
            if (check(&bad, 0x869757, 0))
                return 1;
        }
        for (unsigned sp = 0; sp < 65536u; ++sp) {
            cpu.sp = (uint16_t)sp;
            if (sp >= 0x1f20 && sp <= 0x1ffc) {
                ram[sp + 1u] = 0xff;
                ram[sp + 2u] = 0xff;
            }
            if (check(&cpu, 0x869757, sp >= 0x1f20 && sp <= 0x1ffc ? 0x860000u : 0))
                return 1;
        }
        cpu.sp = 0x1f20;
        ram[0x1f21] = 0x67;
        ram[0x1f22] = 0x45;
        if (Lufia2SceneResumeReturn(&cpu, 0x869757, NULL))
            return 1;
    }
    printf("Resume gate PASS %u read-only cases\n", checks);
    return 0;
}
