#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "snes/ppu.h"

typedef enum Lufia2MapWidescreenResult {
    LUFIA2_MAP_WIDESCREEN_DISABLED = 0,
    LUFIA2_MAP_WIDESCREEN_ACTIVE,
    LUFIA2_MAP_WIDESCREEN_LOADING,
} Lufia2MapWidescreenResult;

Lufia2MapWidescreenResult Lufia2PrepareMapWidescreen(
    Ppu *ppu,
    int margin_pixels);
bool Lufia2MapWidescreenIsActive(void);
void Lufia2FinalizeMapWidescreen(Ppu *ppu, int margin_pixels);
void Lufia2BeginMapRenderOverlay(Ppu *ppu);
void Lufia2EndMapRenderOverlay(Ppu *ppu);
void Lufia2DeactivateMapWidescreen(void);
bool Lufia2MapWidescreenWorldPointIsVisible(uint16_t x, uint16_t y);
