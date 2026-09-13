#include "lufia2_msu_driver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu_state.h"
#include "snes/interp_bridge.h"
#include "snes/msu1.h"
#include "snes/saveload.h"
#include "lufia2_log.h"

/* The SPC driver scales voices 0-7 by the music group volume at $08CC and
 * voices 8-15 by $08CD; command $0B behind $00:9601 sets the first. Zeroing it
 * keeps the sequencer running, which the intro waits on -- the fade-out at
 * $00:9692 ends in the driver's full music stop ($3645) and strands it. The
 * volume reaches a voice at its next note-on ($3FFD), so it is set during the
 * load, before the upload. */
enum {
    SONG_LOAD    = 0x00942Eu, /* STA $54, song id live in A */
    MUSIC_VOLUME = 0x009601u, /* JSL: command $0B, A = music group volume */
    FADE_OUT     = 0x009692u, /* JSL: command $06, the game stopping music */

    VOLUME_FULL = 0xFFu,
    NO_SONG     = 0xFFFFu,
};

enum {
    MSU_STATUS   = 0x2000,
    MSU_TRACK_LO = 0x2004,
    MSU_TRACK_HI = 0x2005,
    MSU_VOLUME   = 0x2006,
    MSU_CONTROL  = 0x2007,

    MSU_ST_AUDIO_ERROR = 0x08,

    MSU_CTL_PLAY   = 0x01,
    MSU_CTL_REPEAT = 0x02,
};

static bool     s_installed;
static bool     s_playing;
static unsigned s_song = NO_SONG;
static unsigned s_loading_song;
static bool     s_volume_sent;

enum {
    LUFIA2_MSU_STATE_MAGIC = 0x3255534du, /* MSU2 */
    LUFIA2_MSU_STATE_VERSION = 1u,
};

typedef struct Lufia2MsuState {
    uint32_t magic;
    uint32_t version;
    uint16_t song;
    uint16_t loading_song;
    uint8_t playing;
    uint8_t volume_sent;
    uint8_t reserved[2];
} Lufia2MsuState;

static Lufia2MsuState s_loaded_state;
static bool s_loaded_state_valid;

/* LUFIA2_MSU_HUSH=off leaves the music group at full volume. */
static bool HushEnabled(void) {
    static bool s_resolved, s_enabled = true;
    if (!s_resolved) {
        const char *choice = getenv("LUFIA2_MSU_HUSH");
        s_resolved = true;
        if (choice && strcmp(choice, "off") == 0) {
            s_enabled = false;
            fprintf(stderr,
                    "[msu] SPC hush disabled; both tracks will sound\n");
        }
    }
    return s_enabled;
}

bool Lufia2MsuDriverPlaying(void) {
    return s_playing;
}

static void MsuSilence(void) {
    msu1_write(MSU_CONTROL, 0);
    s_playing = false;
    s_song = NO_SONG;
}

/* Selecting stops playback by spec, so this also cuts a track short. */
static bool MsuSelectTrack(unsigned song) {
    msu1_write(MSU_TRACK_LO, (uint8_t)song);
    msu1_write(MSU_TRACK_HI, 0);
    return (msu1_read(MSU_STATUS) & MSU_ST_AUDIO_ERROR) == 0;
}

static void GuestPush8(CpuState *cpu, uint8_t value) {
    cpu_write8(cpu, 0x00, cpu->S, value);
    cpu->S = (uint16)(cpu->S - 1u);
}

/* RTL continues one past the address it pops; push order as a real JSL. */
static void GuestCall(CpuState *cpu, uint32_t entry, uint32_t resume) {
    const uint32_t bank = resume & 0xFF0000u;
    const uint32_t back = bank | ((resume - 1u) & 0xFFFFu);
    GuestPush8(cpu, (uint8_t)(back >> 16));
    GuestPush8(cpu, (uint8_t)(back >> 8));
    GuestPush8(cpu, (uint8_t)back);
    interp_bridge_pre_opcode_redirect(bank | entry);
}

static void StartTrack(unsigned song) {
    if (!MsuSelectTrack(song)) {
        MsuSilence();
        LUFIA2_LOG("[msu] song $%02X: no track, SPC plays it\n", song);
        LUFIA2_LOG_FLUSH();
        return;
    }
    msu1_write(MSU_VOLUME, 0xFF);
    /* Repeat everything; the one-shot list is not established yet. */
    msu1_write(MSU_CONTROL, MSU_CTL_PLAY | MSU_CTL_REPEAT);
    s_playing = true;
    s_song = song;
    LUFIA2_LOG("[msu] song $%02X -> track %u\n", song, song);
    LUFIA2_LOG_FLUSH();
}

static void SongLoad(CpuState *cpu, uint32_t pc24) {
    if (s_volume_sent) {
        s_volume_sent = false;   /* $9601 clobbered A; STA $54 wants the id */
        cpu->A = (uint16)((cpu->A & 0xFF00u) | s_loading_song);
        return;
    }

    const unsigned song = cpu->A & 0xFFu;
    s_loading_song = song;
    /* Restarting the track already playing would stutter. */
    if (!s_playing || song != s_song) StartTrack(song);

    if (!HushEnabled()) return;
    cpu->A = (uint16)((cpu->A & 0xFF00u) | (s_playing ? 0u : VOLUME_FULL));
    s_volume_sent = true;
    GuestCall(cpu, MUSIC_VOLUME, pc24);
}

static void MusicFadeOut(CpuState *cpu, uint32_t pc24) {
    (void)cpu;
    (void)pc24;
    MsuSilence();
}

void Lufia2MsuDriverInstall(void) {
    if (s_installed || !msu1_enabled()) return;
    interp_bridge_set_pre_opcode_hook(SONG_LOAD, SongLoad);
    interp_bridge_set_pre_opcode_hook(FADE_OUT, MusicFadeOut);
    s_installed = true;
    LUFIA2_LOG("[msu] driver: load $%06X, fade $%06X\n", SONG_LOAD, FADE_OUT);
    LUFIA2_LOG_FLUSH();
}

void Lufia2MsuSaveState(SaveLoadInfo *sli) {
    Lufia2MsuState state;
    memset(&state, 0, sizeof(state));
    state.magic = LUFIA2_MSU_STATE_MAGIC;
    state.version = LUFIA2_MSU_STATE_VERSION;
    state.song = (uint16_t)s_song;
    state.loading_song = (uint16_t)s_loading_song;
    state.playing = s_playing ? 1u : 0u;
    state.volume_sent = s_volume_sent ? 1u : 0u;
    sli->func(sli, &state, sizeof(state));
}

bool Lufia2MsuLoadState(SaveLoadInfo *sli) {
    memset(&s_loaded_state, 0, sizeof(s_loaded_state));
    s_loaded_state_valid = false;
    sli->func(sli, &s_loaded_state, sizeof(s_loaded_state));
    s_loaded_state_valid =
        s_loaded_state.magic == LUFIA2_MSU_STATE_MAGIC &&
        s_loaded_state.version == LUFIA2_MSU_STATE_VERSION;
    return s_loaded_state_valid;
}

void Lufia2MsuApplyLoadedState(void) {
    if (!s_loaded_state_valid) {
        MsuSilence();
        s_loading_song = 0;
        s_volume_sent = false;
        return;
    }

    const Lufia2MsuState state = s_loaded_state;
    s_loaded_state_valid = false;
    MsuSilence();
    s_loading_song = state.loading_song;
    s_volume_sent = state.volume_sent != 0;
    if (s_installed && state.playing && state.song != NO_SONG)
        StartTrack(state.song);
}
