#ifndef LUFIA2_SOUND_MENU_H
#define LUFIA2_SOUND_MENU_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct CpuState;

bool Lufia2SoundMenuFixEnabled(void);
void Lufia2SoundMenuSetFixEnabled(bool enabled);
bool Lufia2SoundMenuInstall(const uint8_t *rom, size_t size);
void Lufia2SoundMenuCorrectTarget(struct CpuState *cpu, uint32_t pc24);

#ifdef __cplusplus
}
#endif

#endif
