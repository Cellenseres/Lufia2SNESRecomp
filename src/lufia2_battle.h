#ifndef LUFIA2_BATTLE_H
#define LUFIA2_BATTLE_H

#include <stdbool.h>
#include <stdint.h>

enum {
    LUFIA2_BATTLE_BACKGROUND_MIN = 0x00,
    LUFIA2_BATTLE_BACKGROUND_MAX = 0x18,
    LUFIA2_WRAM_SCENE_NMI = 0x0067,
    LUFIA2_WRAM_BATTLE_BACKGROUND_ID = 0x11E1,
    LUFIA2_WRAM_DISPLAY_SHADOW = 0x0583,
};

typedef enum Lufia2NmiScene {
    LUFIA2_NMI_UNKNOWN = 0,
    LUFIA2_NMI_FIELD,
    LUFIA2_NMI_BATTLE,
    LUFIA2_NMI_WORLD_MAP,
    LUFIA2_NMI_MENU,
    LUFIA2_NMI_INTRO,
} Lufia2NmiScene;

typedef struct Lufia2BattleState {
    /* Installed scene callback, not the still-unported encounter lifecycle. */
    Lufia2NmiScene nmi_scene;
    bool active;
    bool background_valid;
    uint8_t background_id;
    /* Battle NMI's pending INIDISP value; readiness only, never identity. */
    bool display_ready;
} Lufia2BattleState;

Lufia2BattleState Lufia2BattleInspect(const uint8_t *wram);

#endif
