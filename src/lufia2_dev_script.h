#ifndef LUFIA2_DEV_SCRIPT_H
#define LUFIA2_DEV_SCRIPT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Development automation, inactive unless LUFIA2_DEV_SCRIPT names a file:
 * scripted pad input, WRAM pokes, frame captures and a scripted exit, all
 * keyed on the guest frame counter. One command per line, `#` comments:
 *
 *   <frame> press <buttons> [frames]         hold, default 4 frames
 *   <frame> mash <buttons> <period> <until>  2-frame taps every <period>
 *   <frame> shot <path.png>                  composed SNES frame
 *   <frame> hdshot <path.png>                presenter output, see below
 *   <frame> layers <path-prefix>             battle layer dump, see below
 *   <frame> wram <path>                      all 128 KiB of work RAM
 *   <frame> ppu <path>                       PPU registers and memory
 *   <frame> poke <hex-address> <hex-value>   WRAM byte, every frame from here
 *   <frame> quit
 *
 * Buttons are A B X Y L R START SELECT UP DOWN LEFT RIGHT joined with `+`.
 * A frame written as `+N` is relative to the previous command's frame, and
 * `battle+N` to the frame the Battle scene last became visible. */

bool Lufia2DevScriptInit(void);
bool Lufia2DevScriptActive(void);
/* Battle anchor for `battle+N` frames. */
void Lufia2DevScriptNoteBattle(uint32_t frame, bool visible);
uint32_t Lufia2DevScriptInput(uint32_t frame, uint32_t host_input);
void Lufia2DevScriptPoke(uint32_t frame, uint8_t *wram, size_t size);
void Lufia2DevScriptFrame(uint32_t frame, const uint8_t *argb,
                         unsigned width, unsigned height);
/* Path requested for this frame by `hdshot`, `layers`, `wram` or `ppu`, else
 * NULL. */
const char *Lufia2DevScriptRequest(uint32_t frame, const char *command);
bool Lufia2DevScriptQuit(uint32_t frame);

/* Opaque 8-bit RGBA PNG from ARGB8888 rows; alpha is kept when asked. */
bool Lufia2DevWritePng(const char *path, const uint8_t *argb,
                      unsigned width, unsigned height, size_t pitch,
                      bool keep_alpha);

#endif
