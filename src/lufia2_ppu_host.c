#include "lufia2_ppu_host.h"
#include <stdlib.h>

#if defined(LUFIA2_PPU_HOST_PATCH) && LUFIA2_PPU_HOST_PATCH
#define LUFIA2_HOST_COSTS_AVAILABLE true
#else
#define LUFIA2_HOST_COSTS_AVAILABLE false
#endif
bool lufia2_ppu_host_costs_enabled = LUFIA2_HOST_COSTS_AVAILABLE;
void Lufia2PpuHostConfigure(void) {
    lufia2_ppu_host_costs_enabled = LUFIA2_HOST_COSTS_AVAILABLE && getenv("LUFIA2_PPU_HOST_REFERENCE") == NULL;
}
