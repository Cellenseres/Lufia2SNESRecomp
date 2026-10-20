#include "lufia2_sprite_visibility.h"
#include "lufia2_map_visibility.h"
#include "widescreen.h"

enum { ACTOR_CULL_PADDING = 64, FIELD_OAM_CAPACITY = 128 };

static uint8_t AcceptWorldPoint(void *context, uint16_t x, uint16_t y) {
    (void)context;
    return Lufia2MapWidescreenWorldPointIsVisible(x, y);
}

Lufia2ExecutionResult Lufia2PatchedFieldActorSprites(
    const Lufia2Memory *memory, Lufia2CpuState *cpu) {
    Lufia2FieldActorVisibility visibility = {0};

    if (!g_ws_active || !Lufia2MapWidescreenIsActive())
        return Lufia2FieldActorSprites(memory, cpu);
    /* Actor anchors are not sprite bounds; keep whole figures past the edge. */
    visibility.horizontal_padding = (uint16_t)(g_ws_extra + ACTOR_CULL_PADDING);
    visibility.vertical_padding = ACTOR_CULL_PADDING;
    visibility.oam_capacity = FIELD_OAM_CAPACITY;
    visibility.accept = AcceptWorldPoint;
    return Lufia2FieldActorSpritesWithVisibility(memory, cpu, &visibility);
}
