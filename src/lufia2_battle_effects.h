#pragma once

#include "snes/ppu.h"

/* Presentation only; guest state stays unchanged. */
bool Lufia2BattleEffectsPrepare(Ppu *ppu, bool wide,
                               unsigned width, unsigned height);
bool Lufia2BattleEffectsActive(const Ppu *ppu);
void Lufia2BattleEffectsBeginLine(const Ppu *ppu);
void Lufia2BattleEffectsMargin(Ppu *ppu, PpuPixelPrioBufs *background,
                              unsigned y, bool sub, int left, int right,
                              PpuZbufType low_priority, bool mosaic);
unsigned Lufia2BattleEffectsColour(const Ppu *ppu, unsigned pixel,
                                  unsigned index, bool sub);
int Lufia2BattleEffectsMosaic(int x, unsigned size);
bool Lufia2BattleEffectsOpaque(unsigned pixel, unsigned index, bool sub);
void Lufia2BattleEffectsShutdown(void);
bool Lufia2BattleEffectsWindows(const Ppu *ppu, unsigned layer,
                               int left, int right, int *w1_left, int *w1_right,
                               int *w2_left, int *w2_right, uint32_t *flags);
