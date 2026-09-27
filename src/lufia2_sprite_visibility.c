#include "lufia2_sprite_visibility.h"
#include "lufia2_map_visibility.h"
#include "widescreen.h"

static uint8_t AcceptWorldPoint(void *context, uint16_t x, uint16_t y) {
    (void)context;
    return Lufia2MapWidescreenWorldPointIsVisible(x, y);
}

Lufia2ExecutionResult Lufia2PatchedFieldActorSprites(
    const Lufia2Memory *memory, Lufia2CpuState *cpu) {
    Lufia2FieldActorVisibility visibility = {0, AcceptWorldPoint, 0};

    if (!g_ws_active || !Lufia2MapWidescreenIsActive())
        return Lufia2FieldActorSprites(memory, cpu);
    if (g_ws_extra > 0)
        visibility.horizontal_padding = (uint16_t)g_ws_extra;
    return Lufia2FieldActorSpritesWithVisibility(memory, cpu, &visibility);
}
