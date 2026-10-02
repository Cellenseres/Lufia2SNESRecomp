#include "lufia2_msu_driver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu_state.h"
#include "decomp_bridge/host_events.h"
#include "snes/msu1.h"
#include "snes/saveload.h"
#include "snes/snes.h"
#include "lufia2_log.h"

/* The SPC keeps playing at zero music volume; MSU follows it. */
enum {
    VOLUME_FULL = 0xFFu,
    NO_SONG     = 0xFFFFu,
};

/* SPC driver RAM. */
enum {
    SPC_MUSIC_STATE = 0x009B,
    SPC_FADE_LEVEL  = 0x08CA, /* scales music voices, $FF = full */
    /* Command $0B only, which the game never sends. */
    SPC_MUSIC_VOLUME = 0x08CC,

    SPC_MUSIC_PLAYING = 0x01,
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

extern Snes *g_snes;

static bool     s_installed;
static unsigned s_song = NO_SONG;  /* selected track, NO_SONG if missing */
static bool     s_rewound;
static bool     s_playing;
static int      s_volume = -1;
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

/* Selecting stops and rewinds by spec. */
static bool MsuSelectTrack(unsigned song) {
    msu1_write(MSU_TRACK_LO, (uint8_t)song);
    msu1_write(MSU_TRACK_HI, 0);
    return (msu1_read(MSU_STATUS) & MSU_ST_AUDIO_ERROR) == 0;
}

static void MsuStop(void) {
    msu1_write(MSU_CONTROL, 0);
    s_playing = false;
    s_rewound = false;
}

static void MsuForget(void) {
    MsuStop();
    s_song = NO_SONG;
}

static void SelectSong(unsigned song) {
    MsuForget();
    if (!MsuSelectTrack(song)) {
        LUFIA2_LOG("[msu] song $%02X: no track, SPC plays it\n", song);
        LUFIA2_LOG_FLUSH();
        return;
    }
    s_song = song;
    s_rewound = true;
    LUFIA2_LOG("[msu] song $%02X -> track %u\n", song, song);
    LUFIA2_LOG_FLUSH();
}

/* The original stops the SPC before every load, so restart. */
static uint32_t SongLoad(CpuState *cpu, uint32_t pc24) {
    (void)pc24;
    if (s_volume_sent) {
        /* Savestate from the guest-call driver; restore the id. */
        s_volume_sent = false;
        cpu->A = (uint16)((cpu->A & 0xFF00u) | s_loading_song);
        return 0;
    }

    s_loading_song = cpu->A & 0xFFu;
    SelectSong(s_loading_song);

    /* Read at note-on, so set before the upload. */
    if (HushEnabled() && g_snes && g_snes->apu)
        g_snes->apu->ram[SPC_MUSIC_VOLUME] =
            (uint8_t)(s_song != NO_SONG ? 0u : VOLUME_FULL);
    return 0;
}

static void MsuPlay(void) {
    if (!s_rewound && !MsuSelectTrack(s_song)) {
        MsuForget();
        return;
    }
    msu1_write(MSU_CONTROL, MSU_CTL_PLAY | MSU_CTL_REPEAT);
    s_playing = true;
    s_rewound = false;
}

/* Play, stop and fade exactly as the SPC music does. */
void Lufia2MsuDriverFrame(void) {
    if (!s_installed || s_song == NO_SONG || !g_snes || !g_snes->apu)
        return;
    const uint8_t *spc = g_snes->apu->ram;

    if (!(spc[SPC_MUSIC_STATE] & SPC_MUSIC_PLAYING)) {
        if (s_playing) MsuStop();
        return;
    }
    const int volume = spc[SPC_FADE_LEVEL];
    if (volume != s_volume) {
        msu1_write(MSU_VOLUME, (uint8_t)volume);
        s_volume = volume;
    }
    if (!s_playing) MsuPlay();
}

void Lufia2MsuDriverInstall(void) {
    if (s_installed || !msu1_enabled()) return;
    Lufia2DecompSetSongLoadEvent(SongLoad);
    s_installed = true;
    LUFIA2_LOG("[msu] driver: following the SPC music\n");
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

/* The next frame resumes from the restored SPC state. */
void Lufia2MsuApplyLoadedState(void) {
    const bool valid = s_loaded_state_valid;
    const Lufia2MsuState state = s_loaded_state;
    s_loaded_state_valid = false;

    MsuForget();
    s_loading_song = valid ? state.loading_song : 0u;
    s_volume_sent = valid && state.volume_sent != 0;
    if (s_installed && valid && state.song != NO_SONG)
        SelectSong(state.song);
}
