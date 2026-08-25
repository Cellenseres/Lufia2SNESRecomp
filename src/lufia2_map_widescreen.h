#pragma once

#include <stdbool.h>

#include "snes/ppu.h"

bool Lufia2PrepareMapWidescreen(Ppu *ppu, int margin_pixels);
bool Lufia2MapWidescreenIsActive(void);
void Lufia2FinalizeMapWidescreen(Ppu *ppu, int margin_pixels);
void Lufia2BeginMapRenderOverlay(Ppu *ppu);
void Lufia2EndMapRenderOverlay(Ppu *ppu);
void Lufia2DeactivateMapWidescreen(void);
