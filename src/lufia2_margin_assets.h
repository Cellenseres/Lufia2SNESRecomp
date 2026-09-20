#ifndef LUFIA2_MARGIN_ASSETS_H
#define LUFIA2_MARGIN_ASSETS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum Lufia2MarginScene {
    LUFIA2_MARGIN_SCENE_INTRO = 0,
    LUFIA2_MARGIN_SCENE_BATTLE = 1,
} Lufia2MarginScene;

bool Lufia2MarginAssetApply(
    Lufia2MarginScene scene, uint8_t scene_id,
    uint8_t *frame, size_t width, size_t height,
    unsigned brightness);

void Lufia2MarginAssetsShutdown(void);

#endif
