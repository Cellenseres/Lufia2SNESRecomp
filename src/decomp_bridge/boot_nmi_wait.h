#ifndef LUFIA2_BOOT_NMI_WAIT_H
#define LUFIA2_BOOT_NMI_WAIT_H

#include <stdbool.h>
#include <stdint.h>

struct CpuState;

#ifdef __cplusplus
extern "C" {
#endif

bool Lufia2DecompBootNmiWaitHookOwned(uint32_t pc,
    void (*hook)(struct CpuState *, uint32_t));

#ifdef __cplusplus
}
#endif

#endif
