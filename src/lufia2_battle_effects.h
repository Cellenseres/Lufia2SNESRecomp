#pragma once

#include "snes/ppu.h"

void Lufia2BattleEffectsInit(const uint8_t *rom, size_t size);
/* Presentation only; guest state stays unchanged. */
bool Lufia2BattleEffectsPrepare(Ppu *ppu, bool wide,
                               unsigned width, unsigned height);
bool Lufia2BattleEffectsActive(const Ppu *ppu);
bool Lufia2BattleEffectsPlane(const Ppu *ppu, unsigned layer);
bool Lufia2BattleEffectsBackground(const Ppu *ppu);
bool Lufia2BattleEffectsSprite(const Ppu *ppu, unsigned slot);
void Lufia2BattleEffectsBeginSprites(void);
void Lufia2BattleEffectsSpritePixel(int x, bool effect);
bool Lufia2BattleEffectsSpriteVisible(const Ppu *ppu, int x);
bool Lufia2BattleEffectsSpriteMargins(Ppu *ppu, const uint16_t *vram, unsigned line);
typedef void Lufia2BattleEffectsLineRenderer(Ppu *ppu, unsigned line);
void Lufia2BattleEffectsHudLine(Ppu *ppu, unsigned line,
                               Lufia2BattleEffectsLineRenderer *draw);
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
