#include "lufia2_runtime.h"

#include "snesrecomp_platform/host_boot.h"

#if defined(LUFIA2_ENABLE_NATIVE_SPC)
#include "lufia2_spc_native.h"
#include "snes/snes.h"

extern Snes *g_snes;

/* Native sound driver progress; nothing else is host state. */
static void Lufia2VitaSaveState(SaveLoadInfo *sli) {
    Lufia2SpcNativeSaveState(sli, g_snes->apu);
}

static void Lufia2VitaLoadState(SaveLoadInfo *sli, uint32_t version) {
    (void)version;
    Lufia2SpcNativeLoadState(sli, g_snes->apu);
}
#define LUFIA2_VITA_STATE_SAVE (&Lufia2VitaSaveState)
#define LUFIA2_VITA_STATE_LOAD (&Lufia2VitaLoadState)
#else
#define LUFIA2_VITA_STATE_SAVE NULL
#define LUFIA2_VITA_STATE_LOAD NULL
#endif

const RtlGameInfo kLufia2GameInfo = {
    .title = "lufia2",
    .initialize = NULL,
    .run_frame = &Lufia2RunOneFrame,
    .draw_ppu_frame = &Lufia2DrawPpuFrame,
    .save_name_prefix = "lufia2",
    .state_save_extra = LUFIA2_VITA_STATE_SAVE,
    .state_load_extra = LUFIA2_VITA_STATE_LOAD,
    .on_state_loaded = NULL,
    .session_reset = NULL,
};

/* The clean USA release, hashed after any copier header is stripped. */
static const char *const kLufia2AcceptedSha1[] = {
    "a89931c1f29b161b8be717dfab4a4adb54b42b84",
};

static const SnesRecompRomSpec kLufia2Rom = {
    .display_name = "Lufia II",
    .payload_size = 2621440,
    .copier_header_size = 512,
    .accepted_sha1_hex = kLufia2AcceptedSha1,
    .accepted_count = 1,
};

const SnesRecompHostGame kLufia2HostGame = {
    .game_id = "lufia2",
    .display_name = "Lufia II: Rise of the Sinistrals",
    .region = "(USA)",
    .short_name = "Lufia II Recompiled",
    .launcher_profile = "snes",
    .rom = &kLufia2Rom,
    .config_path = "config.ini",
    .widescreen_supported = 1,
    .num_players = 1,
    .data_directory_name = "Lufia2Recomp",
    .rom_filename = "lufia2.sfc",
};
