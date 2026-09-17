#ifndef LUFIA2_SPLASH_CREDIT_H
#define LUFIA2_SPLASH_CREDIT_H

struct Ppu;

/* Draws a recompilation credit under "LICENSED BY NINTENDO" on the title
 * splash, into host picture memory. Guest VRAM, the CPU ports and
 * savestates are untouched.
 *
 * Call once per frame, before anything reads PpuRenderVram(). */
void Lufia2SplashCreditPrepare(struct Ppu *ppu);

/* Unbinds the host picture memory. Safe when nothing is bound. */
void Lufia2SplashCreditRelease(struct Ppu *ppu);

#endif
