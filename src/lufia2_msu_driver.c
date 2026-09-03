#include "lufia2_msu_driver.h"

#include <stdio.h>

#include "cpu_state.h"
#include "snes/interp_bridge.h"
#include "snes/msu1.h"

/*   $00:942A  CMP #$64
 *   $00:942C  BCS $944C      out of range -> give up
 *   $00:942E  STA $54        hook: id is live in A
 *   ...       the SPC upload
 *   $00:944C  SEC / RTS      the game's "cannot play this" exit
 *
 * Redirecting to $944C skips the SPC upload, so the two never sound at once.
 * $00:9693 is the fade/stop command; we stop MSU there and let it run. */
enum {
    LUFIA2_MSU_SONG_START = 0x00942E,
    LUFIA2_MSU_SONG_SKIP  = 0x00944C,
    LUFIA2_MSU_MUSIC_STOP = 0x009693,

    LUFIA2_MSU_SONG_LIMIT = 0x64,   /* the game's own CMP #$64 */
    LUFIA2_MSU_NO_SONG    = 0xFFFF,
};

enum {
    MSU_STATUS   = 0x2000,
    MSU_TRACK_LO = 0x2004,
    MSU_TRACK_HI = 0x2005,
    MSU_VOLUME   = 0x2006,
    MSU_CONTROL  = 0x2007,

    MSU_ST_AUDIO_BUSY  = 0x40,
    MSU_ST_AUDIO_ERROR = 0x08,

    MSU_CTL_PLAY   = 0x01,
    MSU_CTL_REPEAT = 0x02,
};

static bool     s_installed;
static bool     s_playing;
static unsigned s_song = LUFIA2_MSU_NO_SONG;

bool Lufia2MsuDriverPlaying(void) {
    return s_playing;
}

static void MsuStop(void) {
    if (!s_playing) return;
    msu1_write(MSU_CONTROL, 0);
    s_playing = false;
    s_song = LUFIA2_MSU_NO_SONG;
}

/* Selecting stops playback by spec, so this also cuts a track short. */
static bool MsuSelectTrack(unsigned song) {
    msu1_write(MSU_TRACK_LO, (uint8_t)song);
    msu1_write(MSU_TRACK_HI, 0);
    return (msu1_read(MSU_STATUS) & MSU_ST_AUDIO_ERROR) == 0;
}

static void SongStart(CpuState *cpu, uint32_t pc24) {
    (void)pc24;
    const unsigned song = cpu->A & 0xFFu;
    if (song >= LUFIA2_MSU_SONG_LIMIT) return;

    /* Runs again for the song already playing; restarting would stutter. */
    if (s_playing && song == s_song) {
        interp_bridge_pre_opcode_redirect(LUFIA2_MSU_SONG_SKIP);
        return;
    }

    if (!MsuSelectTrack(song)) {
        /* No track for this song: let the SPC have it. */
        fprintf(stderr, "[msu] song $%02X: no track, SPC plays it\n", song);
        fflush(stderr);
        s_playing = false;
        s_song = LUFIA2_MSU_NO_SONG;
        return;
    }

    msu1_write(MSU_VOLUME, 0xFF);
    /* Repeat everything; the one-shot list is not established yet. */
    msu1_write(MSU_CONTROL, MSU_CTL_PLAY | MSU_CTL_REPEAT);
    s_playing = true;
    s_song = song;
    fprintf(stderr, "[msu] song $%02X -> track %u\n", song, song);
    fflush(stderr);

    interp_bridge_pre_opcode_redirect(LUFIA2_MSU_SONG_SKIP);
}

static void MusicStop(CpuState *cpu, uint32_t pc24) {
    (void)cpu;
    (void)pc24;
    MsuStop();
}

void Lufia2MsuDriverInstall(void) {
    if (s_installed || !msu1_enabled()) return;
    interp_bridge_set_pre_opcode_hook(LUFIA2_MSU_SONG_START, SongStart);
    interp_bridge_set_pre_opcode_hook(LUFIA2_MSU_MUSIC_STOP, MusicStop);
    s_installed = true;
    fprintf(stderr, "[msu] driver: hooks at $%06X and $%06X\n",
            LUFIA2_MSU_SONG_START, LUFIA2_MSU_MUSIC_STOP);
    fflush(stderr);
}
