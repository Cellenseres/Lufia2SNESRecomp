#pragma once

#include "snes/ppu.h"

/* Keeps BG3 text windows out of map margins. */
void Lufia2UiMarginsMap(Ppu *ppu);

/* Keeps the BG2 member highlight out of menu margins. */
void Lufia2UiMarginsMenu(Ppu *ppu);
