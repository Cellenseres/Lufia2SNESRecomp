#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Room visibility policy without a renderer/PPU dependency. */
bool Lufia2MapWidescreenIsActive(void);
bool Lufia2MapWidescreenWorldPointIsVisible(uint16_t x, uint16_t y);
