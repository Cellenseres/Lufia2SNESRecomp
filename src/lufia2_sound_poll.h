#pragma once
#include <stdbool.h>
#include <stdint.h>

/* Consumer patch: enable only when the host owns all APU execution on its
 * game thread. The unmodified semantic library retains its original polls. */
void Lufia2HostSoundPollConfigure(bool single_threaded);
uint32_t Lufia2HostSoundPollLimit(uint32_t original_limit);
extern bool lufia2_sound_poll_kernel_enabled;
extern uint64_t lufia2_sound_poll_kernel_steps;
extern bool lufia2_frame_poll_kernel_enabled;
extern uint64_t lufia2_frame_poll_kernel_steps;
