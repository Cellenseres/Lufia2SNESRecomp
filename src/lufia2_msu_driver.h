#pragma once

#include <stdbool.h>

/* MSU-1 music driver. Hooks the game's own song-start routine and drives the
 * registers directly, so no ROM is patched. No-op unless a pack was armed. */
void Lufia2MsuDriverInstall(void);

bool Lufia2MsuDriverPlaying(void);
