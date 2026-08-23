#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "desktop/sdl_compat.h"
#include "host_report.h"
#include "types.h"
#include "spc_player.h"
#include "desktop_glue.h"

/* Shared APU/audio lock. */
static SDL_mutex *s_audio_mutex;

bool LufiaDesktopCreateAudioMutex(void) {
    if (s_audio_mutex)
        return true;
    s_audio_mutex = SDL_CreateMutex();
    return s_audio_mutex != NULL;
}

void LufiaDesktopDestroyAudioMutex(void) {
    if (s_audio_mutex) {
        SDL_DestroyMutex(s_audio_mutex);
        s_audio_mutex = NULL;
    }
}

void RtlApuLock(void) {
    if (s_audio_mutex)
        SDL_LockMutex(s_audio_mutex);
}

void RtlApuUnlock(void) {
    if (s_audio_mutex)
        SDL_UnlockMutex(s_audio_mutex);
}

/* Lufia uses the emulated SPC700/DSP path. */
SpcPlayer *g_spc_player = NULL;

bool g_ws_active = false;
int g_ws_extra = 0;

void debug_on_wram_write_byte(uint32_t addr, uint8_t old_val, uint8_t new_val) {
    (void)addr; (void)old_val; (void)new_val;
}

void debug_on_wram_write_word(uint32_t addr, uint16_t old_val, uint16_t new_val) {
    (void)addr; (void)old_val; (void)new_val;
}

void NORETURN Die(const char *error) {
    host_report_fatal(error ? error : "(null)");
    fprintf(stderr, "Lufia2RecompDesktop FATAL: %s\n",
            error ? error : "(null)");
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                             "Lufia II Recompiled",
                             error ? error : "Fatal error",
                             NULL);
    exit(1);
}
