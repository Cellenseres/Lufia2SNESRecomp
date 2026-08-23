/* Lufia II desktop host. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "desktop/sdl_compat.h"
#include "host_report.h"
#include "host_paths.h"
#include "launcher_cache.h"
#include "common_rtl.h"
#include "common_cpu_infra.h"
#include "snes/snes.h"
#include "snes/ppu.h"

#include "recomp_launcher.h"
#include "launcher_profile.h"
#include "common/keybinds.h"
#include "common/sha1.h"
#include "common/launcher_binds.h"

#include "config.h"
#include "lufia2_runtime.h"
#include "desktop_glue.h"

enum {
    SNES_WIDTH  = 256,
    SNES_HEIGHT = 224,
    PPU_BUFFER_HEIGHT = 240,
    DEFAULT_WINDOW_SCALE = 3,

    PAD_B      = 0x001,
    PAD_Y      = 0x002,
    PAD_SELECT = 0x004,
    PAD_START  = 0x008,
    PAD_UP     = 0x010,
    PAD_DOWN   = 0x020,
    PAD_LEFT   = 0x040,
    PAD_RIGHT  = 0x080,
    PAD_A      = 0x100,
    PAD_X      = 0x200,
    PAD_L      = 0x400,
    PAD_R      = 0x800,

    LUFIA2_ROM_SIZE = 2621440,
    LUFIA2_COPIER_HEADER = 512,
};

static const char kLufia2Sha1[] =
    "a89931c1f29b161b8be717dfab4a4adb54b42b84";

static const char *const kLufia2KnownSha1[] = {
    "a89931c1f29b161b8be717dfab4a4adb54b42b84",
};

extern Ppu *g_ppu;
extern bool g_fail;

static SDL_Window *s_window;
static SDL_Renderer *s_renderer;
static SDL_Texture *s_texture;
static SDL_AudioStream *s_audio_stream;
static SDL_Gamepad *s_gamepad;

static uint8_t s_pixels[SNES_WIDTH * 4 * PPU_BUFFER_HEIGHT];
static uint8_t *s_audio_scratch;
static size_t s_audio_scratch_size;

static bool s_paused;
static bool s_turbo;
static bool s_fullscreen;
static bool s_display_perf;
static int s_current_window_scale = DEFAULT_WINDOW_SCALE;
static int s_volume_percent = 100;
static int s_player1_source = 1; /* 0 none, 1 keyboard, 2 gamepad */

static uint64_t s_perf_last_ms;
static uint32_t s_perf_frames;

static void PrintUsage(const char *exe) {
    fprintf(stderr,
        "Lufia II Recompiled\n\n"
        "Usage:\n"
        "  %s                         Open launcher\n"
        "  %s <rom.sfc>               Boot ROM directly (developer path)\n"
        "  %s --launcher              Force launcher even with SkipLauncher=1\n\n"
        "Runtime hotkeys are editable in Launcher -> Settings -> Hotkeys.\n",
        exe, exe, exe);
}

static bool FileExists(const char *path) {
    if (!path || !*path)
        return false;
    FILE *f = fopen(path, "rb");
    if (!f)
        return false;
    fclose(f);
    return true;
}

static bool EnsureDefaultConfig(const char *path) {
    if (FileExists(path))
        return true;

    FILE *f = fopen(path, "wb");
    if (!f)
        return false;

    static const char kDefaultConfig[] =
        "# Lufia II Recompiled desktop configuration\n"
        "# The launcher edits the supported settings in-place.\n"
        "\n"
        "[General]\n"
        "SkipLauncher = 0\n"
        "DisplayPerfInTitle = 0\n"
        "DisableFrameDelay = 0\n"
        "\n"
        "[Graphics]\n"
        "WindowScale = 3\n"
        "Fullscreen = 0\n"
        "IgnoreAspectRatio = 0\n"
        "DisplayAspect = 4:3\n"
        "OutputMethod = SDL\n"
        "LinearFiltering = 0\n"
        "NewRenderer = 0\n"
        "NoSpriteLimits = 0\n"
        "Widescreen = 0\n"
        "\n"
        "[Sound]\n"
        "EnableAudio = 1\n"
        "AudioFreq = 32040\n"
        "AudioChannels = 2\n"
        "AudioSamples = 512\n"
        "\n"
        "[GamepadMap]\n"
        "EnableGamepad1 = true\n"
        "EnableGamepad2 = false\n"
        "GamepadDeadzone = 10000\n"
        "\n"
        "[KeyMap]\n"
        "# Controller buttons are stored separately in keybinds.ini.\n"
        "# Unsafe/unfinished host actions begin unbound.\n"
        "Load =\n"
        "Save =\n"
        "Reset =\n"
        "ToggleWidescreen =\n"
        "Fullscreen = Alt+Return\n"
        "Pause = Shift+p\n"
        "PauseDimmed = p\n"
        "Turbo = Tab\n"
        "DisplayPerf = f\n"
        "ToggleRenderer = F8\n"
        "WindowBigger =\n"
        "WindowSmaller =\n"
        "VolumeUp =\n"
        "VolumeDown =\n";

    const size_t n = sizeof(kDefaultConfig) - 1;
    const bool ok = fwrite(kDefaultConfig, 1, n, f) == n;
    fclose(f);

    if (ok)
        fprintf(stderr, "[config] created %s\n", path);
    return ok;
}

static void PersistInt(const char *section, const char *key, int value) {
    char text[64];
    snprintf(text, sizeof(text), "%d", value);
    launcher_ini_kv_write("config.ini", section, key, text);
}

static bool LoadVerifiedRom(const char *path,
                            uint8_t **rom_out,
                            uint32_t *size_out,
                            bool quiet) {
    if (rom_out) *rom_out = NULL;
    if (size_out) *size_out = 0;

    FILE *f = fopen(path, "rb");
    if (!f) {
        if (!quiet)
            fprintf(stderr, "Could not open ROM: %s\n", path);
        return false;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return false;
    }

    long file_size_long = ftell(f);
    if (file_size_long <= 0) {
        fclose(f);
        return false;
    }
    rewind(f);

    const size_t file_size = (size_t)file_size_long;
    uint8_t *file_data = (uint8_t *)malloc(file_size);
    if (!file_data) {
        fclose(f);
        return false;
    }

    if (fread(file_data, 1, file_size, f) != file_size) {
        free(file_data);
        fclose(f);
        return false;
    }
    fclose(f);

    size_t payload_offset = 0;
    size_t payload_size = file_size;

    /* Accept legacy 512-byte copier headers. */
    if (file_size == (size_t)LUFIA2_ROM_SIZE + LUFIA2_COPIER_HEADER) {
        payload_offset = LUFIA2_COPIER_HEADER;
        payload_size = LUFIA2_ROM_SIZE;
    }

    if (payload_size != LUFIA2_ROM_SIZE) {
        if (!quiet) {
            fprintf(stderr,
                "Wrong Lufia II ROM size: %zu bytes "
                "(expected %d unheadered, or %d with copier header)\n",
                file_size, LUFIA2_ROM_SIZE,
                LUFIA2_ROM_SIZE + LUFIA2_COPIER_HEADER);
        }
        free(file_data);
        return false;
    }

    uint8_t digest[20];
    char sha1_hex[41];
    recompui_sha1_compute(file_data + payload_offset, payload_size, digest);
    recompui_sha1_hex(digest, sha1_hex);

    if (strcmp(sha1_hex, kLufia2Sha1) != 0) {
        if (!quiet) {
            fprintf(stderr,
                "ROM SHA-1 mismatch.\n"
                "Expected: %s\n"
                "Actual:   %s\n",
                kLufia2Sha1, sha1_hex);
        }
        free(file_data);
        return false;
    }

    uint8_t *payload = file_data;
    if (payload_offset != 0) {
        payload = (uint8_t *)malloc(payload_size);
        if (!payload) {
            free(file_data);
            return false;
        }
        memcpy(payload, file_data + payload_offset, payload_size);
        free(file_data);
        fprintf(stderr,
            "[rom] stripped 512-byte copier header before boot/hash\n");
    }

    if (rom_out) *rom_out = payload;
    else free(payload);
    if (size_out) *size_out = (uint32_t)payload_size;
    return true;
}

static bool CachedRomIsValid(const char *path) {
    uint8_t *data = NULL;
    uint32_t size = 0;
    if (!LoadVerifiedRom(path, &data, &size, true))
        return false;
    free(data);
    return true;
}

static bool ResolveRomWithLauncher(int argc, char **argv,
                                   char *rom_path, size_t rom_cap) {
    char positional_abs[1024];
    positional_abs[0] = '\0';

    bool force_launcher = false;
    const char *positional = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--launcher") == 0) {
            force_launcher = true;
        } else if (strcmp(argv[i], "--help") == 0 ||
                   strcmp(argv[i], "-h") == 0) {
            PrintUsage(argv[0]);
            return false;
        } else if (argv[i][0] != '-' && positional == NULL) {
            positional = argv[i];
        }
    }

    /* Resolve relative ROM paths before changing cwd. */
    if (positional) {
        if (!snesrecomp_abspath(positional,
                                positional_abs,
                                sizeof(positional_abs))) {
            fprintf(stderr, "Could not resolve ROM path: %s\n", positional);
            return false;
        }
    }

    if (!snesrecomp_anchor_to_exe_dir()) {
        fprintf(stderr,
            "[host] warning: could not anchor cwd to executable directory\n");
    }
    RtlMigrateLegacySram("lufia2");


    if (!EnsureDefaultConfig("config.ini")) {
        fprintf(stderr,
            "[config] warning: could not create default config.ini\n");
    }

    ParseConfigFile("config.ini");

    if (positional_abs[0]) {
        snprintf(rom_path, rom_cap, "%s", positional_abs);
        return true;
    }

    char cached[1024];
    cached[0] = '\0';
    snesrecomp_rom_cache_read(cached, sizeof(cached));

    const char *no_launcher = getenv("SNESRECOMP_NO_LAUNCHER");
    bool want_launcher = !(no_launcher && *no_launcher);

    if (want_launcher && g_config.skip_launcher && !force_launcher &&
        cached[0] && CachedRomIsValid(cached)) {
        snprintf(rom_path, rom_cap, "%s", cached);
        host_report_breadcrumb(
            "launcher skipped (SkipLauncher=1, verified cached ROM)");
        return true;
    }

    if (!want_launcher) {
        if (cached[0] && CachedRomIsValid(cached)) {
            snprintf(rom_path, rom_cap, "%s", cached);
            return true;
        }
        fprintf(stderr,
            "SNESRECOMP_NO_LAUNCHER is set but no verified cached ROM exists.\n");
        return false;
    }

    RecompLauncherCSettings ls;
    memset(&ls, 0, sizeof(ls));
    ls.output_method = g_config.output_method;
    ls.window_scale = g_config.window_scale
        ? g_config.window_scale : DEFAULT_WINDOW_SCALE;
    ls.fullscreen = g_config.fullscreen;
    ls.ignore_aspect = g_config.ignore_aspect_ratio ? 1 : 0;
    ls.linear_filter = g_config.linear_filtering ? 1 : 0;
    ls.widescreen = 0;
    ls.enable_audio = g_config.enable_audio ? 1 : 0;
    ls.audio_freq = g_config.audio_freq ? g_config.audio_freq : 32040;
    ls.volume = 100;
    ls.player_src[0] = g_config.enable_gamepad[0] ? 2 : 1;
    ls.player_src[1] = 0;
    ls.deadzone[0] = g_config.gamepad_deadzone * 100 / 32767;
    if (ls.deadzone[0] < 0) ls.deadzone[0] = 0;
    if (ls.deadzone[0] > 100) ls.deadzone[0] = 100;
    ls.deadzone[1] = ls.deadzone[0];
    ls.skip_launcher = g_config.skip_launcher ? 1 : 0;
    ls.msu1_enabled = 0;

    RecompLauncherCGameInfo gi;
    memset(&gi, 0, sizeof(gi));
    launcher_profile_apply("snes", &gi);

    gi.name = "Lufia II: Rise of the Sinistrals";
    gi.region = "(USA)";
    gi.known_sha1_hex = kLufia2KnownSha1;
    gi.num_known_sha1 = 1;
    gi.sram_path = "saves/save.srm";
    gi.widescreen_supported = 0;
    gi.num_players = 1;
    gi.msu1_supported = 0;
    gi.config_path = "config.ini";
    gi.keybinds_path = "keybinds.ini";

    host_report_breadcrumb("launcher: opening recomp-ui");

    const int action = recomp_launcher_run_window(
        "Lufia II: Rise of the Sinistrals — Launcher",
        &ls,
        &gi,
        ".",
        cached[0] ? cached : NULL,
        rom_path,
        rom_cap);

    host_report_breadcrumb("launcher: action=%d rom=%s",
        action, rom_path[0] ? rom_path : "(none)");

    if (action == RECOMP_LAUNCHER_RESULT_QUIT)
        return false;

    if (action == RECOMP_LAUNCHER_RESULT_UNAVAILABLE) {
        fprintf(stderr,
            "[launcher] GUI unavailable; trying verified cached ROM.\n");
        if (cached[0] && CachedRomIsValid(cached)) {
            snprintf(rom_path, rom_cap, "%s", cached);
            return true;
        }
        return false;
    }

    if (action != RECOMP_LAUNCHER_RESULT_LAUNCH || !rom_path[0])
        return false;

    /* Game output currently uses SDL_Renderer. */
    if (ls.output_method == kOutputMethod_OpenGL) {
        fprintf(stderr,
            "[video] OpenGL game renderer is not enabled in Lufia Desktop v2; "
            "falling back to SDL.\n");
        ls.output_method = kOutputMethod_SDL;
    }

    g_config.output_method = (uint8)ls.output_method;
    g_config.window_scale =
        (uint8)(ls.window_scale > 0 ? ls.window_scale : DEFAULT_WINDOW_SCALE);
    g_config.fullscreen = (uint8)ls.fullscreen;
    g_config.ignore_aspect_ratio = ls.ignore_aspect != 0;
    g_config.linear_filtering = ls.linear_filter != 0;
    g_config.widescreen = false;
    g_config.enable_audio = ls.enable_audio != 0;
    g_config.audio_freq = (uint16)ls.audio_freq;
    g_config.enable_gamepad[0] = ls.player_src[0] == 2;
    g_config.enable_gamepad[1] = false;
    g_config.gamepad_deadzone = ls.deadzone[0] * 32767 / 100;
    g_config.skip_launcher = ls.skip_launcher != 0;

    s_player1_source = ls.player_src[0];
    s_volume_percent = ls.volume;
    if (s_volume_percent < 0) s_volume_percent = 0;
    if (s_volume_percent > 100) s_volume_percent = 100;

    WriteConfigFile("config.ini");

    /* These fields are not persisted by mmx_config.c at this revision. */
    PersistInt("Graphics", "Fullscreen", g_config.fullscreen);
    PersistInt("Graphics", "IgnoreAspectRatio",
               g_config.ignore_aspect_ratio ? 1 : 0);

    ConfigReloadKeyMap("config.ini");

    snesrecomp_rom_cache_write(rom_path);
    return true;
}

static bool KeyDown(const uint8_t *keys, SDL_Scancode sc) {
    return keys && sc != SDL_SCANCODE_UNKNOWN && keys[sc] != 0;
}

static uint32_t ReadKeyboardInput(void) {
    if (s_player1_source != 1)
        return 0;

    const uint8_t *keys = snesrecomp_sdl_get_keyboard_state();
    const KeyBinds *binds = recompui_keybinds_get();
    if (!keys || !binds)
        return 0;

    const PlayerBinds *b = &binds->p1;
    uint32_t p = 0;

    if (KeyDown(keys, b->up))     p |= PAD_UP;
    if (KeyDown(keys, b->down))   p |= PAD_DOWN;
    if (KeyDown(keys, b->left))   p |= PAD_LEFT;
    if (KeyDown(keys, b->right))  p |= PAD_RIGHT;
    if (KeyDown(keys, b->b))      p |= PAD_B;
    if (KeyDown(keys, b->a))      p |= PAD_A;
    if (KeyDown(keys, b->y))      p |= PAD_Y;
    if (KeyDown(keys, b->x))      p |= PAD_X;
    if (KeyDown(keys, b->l))      p |= PAD_L;
    if (KeyDown(keys, b->r))      p |= PAD_R;
    if (KeyDown(keys, b->start))  p |= PAD_START;
    if (KeyDown(keys, b->select)) p |= PAD_SELECT;

    return p;
}

static uint32_t ReadGamepadInput(void) {
    if (s_player1_source != 2 || !s_gamepad)
        return 0;

    uint32_t p = 0;

    if (SDL_GetGamepadButton(s_gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP))
        p |= PAD_UP;
    if (SDL_GetGamepadButton(s_gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN))
        p |= PAD_DOWN;
    if (SDL_GetGamepadButton(s_gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT))
        p |= PAD_LEFT;
    if (SDL_GetGamepadButton(s_gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT))
        p |= PAD_RIGHT;

    /* Standard SNES pad mapping. */
    if (SDL_GetGamepadButton(s_gamepad, SDL_GAMEPAD_BUTTON_SOUTH))
        p |= PAD_B;
    if (SDL_GetGamepadButton(s_gamepad, SDL_GAMEPAD_BUTTON_EAST))
        p |= PAD_A;
    if (SDL_GetGamepadButton(s_gamepad, SDL_GAMEPAD_BUTTON_WEST))
        p |= PAD_Y;
    if (SDL_GetGamepadButton(s_gamepad, SDL_GAMEPAD_BUTTON_NORTH))
        p |= PAD_X;
    if (SDL_GetGamepadButton(s_gamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER))
        p |= PAD_L;
    if (SDL_GetGamepadButton(s_gamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER))
        p |= PAD_R;
    if (SDL_GetGamepadButton(s_gamepad, SDL_GAMEPAD_BUTTON_START))
        p |= PAD_START;
    if (SDL_GetGamepadButton(s_gamepad, SDL_GAMEPAD_BUTTON_BACK))
        p |= PAD_SELECT;

    int deadzone = g_config.gamepad_deadzone;
    if (deadzone < 0 || deadzone > 32767)
        deadzone = 10000;

    const Sint16 ax = SDL_GetGamepadAxis(s_gamepad, SDL_GAMEPAD_AXIS_LEFTX);
    const Sint16 ay = SDL_GetGamepadAxis(s_gamepad, SDL_GAMEPAD_AXIS_LEFTY);

    if (ax < -deadzone) p |= PAD_LEFT;
    if (ax >  deadzone) p |= PAD_RIGHT;
    if (ay < -deadzone) p |= PAD_UP;
    if (ay >  deadzone) p |= PAD_DOWN;

    return p;
}

static void TryOpenFirstGamepad(void) {
    if (s_player1_source != 2 || s_gamepad)
        return;

    int count = 0;
    SDL_JoystickID *ids = SDL_GetGamepads(&count);
    if (!ids)
        return;

    if (count > 0) {
        s_gamepad = SDL_OpenGamepad(ids[0]);
        if (s_gamepad) {
            const char *name = SDL_GetGamepadName(s_gamepad);
            fprintf(stderr, "[input] gamepad: %s\n",
                    name ? name : "(unknown)");
        }
    }

    SDL_free(ids);
}

static void ApplyVolume(int16_t *samples, int sample_count) {
    if (s_volume_percent >= 100)
        return;

    for (int i = 0; i < sample_count; i++) {
        int32_t v = (int32_t)samples[i] * s_volume_percent / 100;
        if (v > 32767) v = 32767;
        if (v < -32768) v = -32768;
        samples[i] = (int16_t)v;
    }
}

static void SDLCALL AudioStreamCallback(void *userdata,
                                        SDL_AudioStream *stream,
                                        int additional_amount,
                                        int total_amount) {
    (void)userdata;
    (void)total_amount;

    if (additional_amount <= 0)
        return;

    if ((size_t)additional_amount > s_audio_scratch_size) {
        uint8_t *p = (uint8_t *)realloc(
            s_audio_scratch, (size_t)additional_amount);
        if (!p)
            return;
        s_audio_scratch = p;
        s_audio_scratch_size = (size_t)additional_amount;
    }

    memset(s_audio_scratch, 0, (size_t)additional_amount);

    const int frames = additional_amount / (2 * (int)sizeof(int16_t));
    if (frames > 0) {
        RtlRenderAudio((int16 *)s_audio_scratch, frames, 2);
        ApplyVolume((int16_t *)s_audio_scratch, frames * 2);
    }

    SDL_PutAudioStreamData(stream, s_audio_scratch, additional_amount);
}

static bool InitVideo(void) {
    int scale = g_config.window_scale
        ? g_config.window_scale : DEFAULT_WINDOW_SCALE;
    if (scale < 1) scale = 1;
    if (scale > 10) scale = 10;
    s_current_window_scale = scale;

    s_window = snesrecomp_sdl_create_window(
        "Lufia II: Rise of the Sinistrals (Recompiled)",
        SNES_WIDTH * scale,
        SNES_HEIGHT * scale,
        SDL_WINDOW_RESIZABLE);

    if (!s_window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    const bool software =
        g_config.output_method == kOutputMethod_SDLSoftware;

    /* VSync prevents tearing; frame pacing still caps guest speed. */
    s_renderer = snesrecomp_sdl_create_renderer(
        s_window, software, true);

    if (!s_renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return false;
    }

    if (!g_config.ignore_aspect_ratio) {
        snesrecomp_sdl_set_render_logical_size(
            s_renderer, SNES_WIDTH, SNES_HEIGHT);
    }

    s_texture = SDL_CreateTexture(
        s_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SNES_WIDTH,
        SNES_HEIGHT);

    if (!s_texture) {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        return false;
    }

    snesrecomp_sdl_set_texture_opaque(s_texture);
    snesrecomp_sdl_set_texture_linear(
        s_texture, g_config.linear_filtering);

    if (g_config.fullscreen != 0) {
        s_fullscreen = true;
        snesrecomp_sdl_set_fullscreen(s_window, true);
    }

    host_report_breadcrumb(
        "video ready: %dx%d scale=%d method=%s fullscreen=%d linear=%d",
        SNES_WIDTH, SNES_HEIGHT, scale,
        software ? "SDL-software" : "SDL",
        g_config.fullscreen,
        g_config.linear_filtering ? 1 : 0);
    return true;
}

static bool InitAudio(void) {
    if (!g_config.enable_audio)
        return true;

    SDL_AudioSpec want;
    SDL_AudioSpec have;
    SDL_zero(want);
    SDL_zero(have);

    int freq = g_config.audio_freq;
    if (freq < 11025 || freq > 48000)
        freq = 32040;

    want.freq = freq;
    want.format = SDL_AUDIO_S16;
    want.channels = 2;

    s_audio_stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &want,
        AudioStreamCallback,
        NULL);

    if (!s_audio_stream) {
        fprintf(stderr,
            "SDL_OpenAudioDeviceStream failed: %s\n",
            SDL_GetError());
        return false;
    }

    have = want;
    SDL_GetAudioStreamFormat(s_audio_stream, &have, NULL);
    RtlSetAudioOutputRate(have.freq);

    if (!SDL_ResumeAudioStreamDevice(s_audio_stream)) {
        fprintf(stderr,
            "SDL_ResumeAudioStreamDevice failed: %s\n",
            SDL_GetError());
        return false;
    }

    host_report_breadcrumb(
        "audio ready: %d Hz, %d channel(s), volume=%d%%",
        have.freq, have.channels, s_volume_percent);
    return true;
}

static void PresentFrame(void) {
    SDL_UpdateTexture(s_texture, NULL, s_pixels, SNES_WIDTH * 4);
    SDL_SetRenderDrawColor(s_renderer, 0, 0, 0, 255);
    SDL_RenderClear(s_renderer);
    snesrecomp_sdl_render_texture(
        s_renderer, s_texture, NULL, NULL);
    SDL_RenderPresent(s_renderer);
}

static void ToggleFullscreen(void) {
    s_fullscreen = !s_fullscreen;
    g_config.fullscreen = s_fullscreen ? 1 : 0;
    snesrecomp_sdl_set_fullscreen(s_window, s_fullscreen);
    PersistInt("Graphics", "Fullscreen", g_config.fullscreen);
}

static void ResizeWindow(int delta) {
    s_current_window_scale += delta;
    if (s_current_window_scale < 1)
        s_current_window_scale = 1;
    if (s_current_window_scale > 10)
        s_current_window_scale = 10;

    SDL_SetWindowSize(
        s_window,
        SNES_WIDTH * s_current_window_scale,
        SNES_HEIGHT * s_current_window_scale);

    g_config.window_scale = (uint8)s_current_window_scale;
    WriteConfigFile("config.ini");
}

static void HandleHostCommand(int cmd, bool pressed) {
    if (cmd == kKeys_Turbo) {
        s_turbo = pressed;
        return;
    }

    if (!pressed)
        return;

    if (cmd >= kKeys_Load && cmd <= kKeys_Load_Last) {
        fprintf(stderr,
            "[savestate] load slots are intentionally disabled until "
            "Lufia's hybrid CpuState/resume context is serialized safely.\n");
        return;
    }

    if (cmd >= kKeys_Save && cmd <= kKeys_Save_Last) {
        fprintf(stderr,
            "[savestate] save slots are intentionally disabled until "
            "Lufia's hybrid CpuState/resume context is serialized safely.\n");
        return;
    }

    switch (cmd) {
    case kKeys_Fullscreen:
        ToggleFullscreen();
        break;

    case kKeys_Reset:
        /* RtlReset requires an HLE SpcPlayer at this runner revision. */
        fprintf(stderr,
            "[reset] not enabled yet for the real-SPC Lufia runtime.\n");
        break;

    case kKeys_Pause:
    case kKeys_PauseDimmed:
        s_paused = !s_paused;
        break;

    case kKeys_WindowBigger:
        ResizeWindow(+1);
        break;

    case kKeys_WindowSmaller:
        ResizeWindow(-1);
        break;

    case kKeys_VolumeUp:
        s_volume_percent += 5;
        if (s_volume_percent > 100) s_volume_percent = 100;
        fprintf(stderr, "[audio] volume %d%%\n", s_volume_percent);
        break;

    case kKeys_VolumeDown:
        s_volume_percent -= 5;
        if (s_volume_percent < 0) s_volume_percent = 0;
        fprintf(stderr, "[audio] volume %d%%\n", s_volume_percent);
        break;

    case kKeys_DisplayPerf:
        s_display_perf = !s_display_perf;
        s_perf_last_ms = SDL_GetTicks();
        s_perf_frames = 0;
        break;

    case kKeys_ToggleRenderer:
        g_config.new_renderer = !g_config.new_renderer;
        PersistInt("Graphics", "NewRenderer",
                   g_config.new_renderer ? 1 : 0);
        fprintf(stderr,
            "[video] PPU renderer: %s\n",
            g_config.new_renderer ? "new" : "legacy");
        break;

    case kKeys_ToggleWidescreen:
        fprintf(stderr,
            "[video] widescreen is not enabled for Lufia II yet.\n");
        break;

    default:
        break;
    }
}

static bool HandleEvents(void) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT)
            return false;

        if (e.type == SDL_EVENT_KEY_DOWN ||
            e.type == SDL_EVENT_KEY_UP) {
            const bool down = e.type == SDL_EVENT_KEY_DOWN;
            if (down && !e.key.repeat &&
                e.key.key == SDLK_ESCAPE) {
                return false;
            }

            const int cmd =
                FindCmdForSdlKey(e.key.key, e.key.mod);
            if (!e.key.repeat)
                HandleHostCommand(cmd, down);
        }

        if (e.type == SDL_EVENT_GAMEPAD_ADDED && !s_gamepad)
            TryOpenFirstGamepad();

        if (e.type == SDL_EVENT_GAMEPAD_REMOVED && s_gamepad) {
            const SDL_JoystickID open_id =
                SDL_GetGamepadID(s_gamepad);
            if (open_id == e.gdevice.which) {
                SDL_CloseGamepad(s_gamepad);
                s_gamepad = NULL;
                TryOpenFirstGamepad();
            }
        }
    }

    return true;
}

static void UpdatePerfTitle(void) {
    if (!s_display_perf || !s_window)
        return;

    s_perf_frames++;
    const uint64_t now = SDL_GetTicks();

    if (s_perf_last_ms == 0)
        s_perf_last_ms = now;

    const uint64_t elapsed = now - s_perf_last_ms;
    if (elapsed >= 1000) {
        const double fps =
            (double)s_perf_frames * 1000.0 / (double)elapsed;
        char title[256];
        snprintf(title, sizeof(title),
            "Lufia II: Rise of the Sinistrals (Recompiled) — %.1f FPS",
            fps);
        SDL_SetWindowTitle(s_window, title);
        s_perf_frames = 0;
        s_perf_last_ms = now;
    }
}

static void ShutdownDesktop(void) {
    if (s_audio_stream) {
        SDL_PauseAudioStreamDevice(s_audio_stream);
        SDL_DestroyAudioStream(s_audio_stream);
        s_audio_stream = NULL;
    }

    free(s_audio_scratch);
    s_audio_scratch = NULL;
    s_audio_scratch_size = 0;

    if (s_gamepad) {
        SDL_CloseGamepad(s_gamepad);
        s_gamepad = NULL;
    }

    if (s_texture) {
        SDL_DestroyTexture(s_texture);
        s_texture = NULL;
    }
    if (s_renderer) {
        SDL_DestroyRenderer(s_renderer);
        s_renderer = NULL;
    }
    if (s_window) {
        SDL_DestroyWindow(s_window);
        s_window = NULL;
    }

    LufiaDesktopDestroyAudioMutex();
    SDL_Quit();
}

int main(int argc, char **argv) {
    char rom_path[1024];
    rom_path[0] = '\0';

    /* Register early so Tier-2 reporting has the game identity. */
    RtlRegisterGame(&kLufia2GameInfo);

    host_report_init("lufia2", "desktop-v2-launcher");

    if (!ResolveRomWithLauncher(
            argc, argv, rom_path, sizeof(rom_path))) {
        return 0;
    }


    s_player1_source = g_config.enable_gamepad[0] ? 2 : 1;
    s_display_perf = g_config.display_perf_title;

    uint8_t *rom_data = NULL;
    uint32_t rom_size = 0;
    if (!LoadVerifiedRom(
            rom_path, &rom_data, &rom_size, false)) {
        fprintf(stderr,
            "\nThis build accepts the verified USA Lufia II ROM only.\n"
            "Expected SHA-1: %s\n", kLufia2Sha1);
        return 1;
    }

    snesrecomp_rom_cache_write(rom_path);

    fprintf(stderr,
        "Lufia II SNESRecomp Desktop v2\n"
        "--------------------------------\n"
        "ROM:      %s\n"
        "Bytes:    %u\n"
        "Launcher: recomp-ui / SNES profile\n",
        rom_path, (unsigned)rom_size);

    if (!snesrecomp_sdl_init(
            SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        free(rom_data);
        return 1;
    }

    if (!LufiaDesktopCreateAudioMutex()) {
        fprintf(stderr, "SDL_CreateMutex failed: %s\n", SDL_GetError());
        SDL_Quit();
        free(rom_data);
        return 1;
    }

    /* Apply bindings changed in the launcher. */
    recompui_keybinds_init(NULL);

    Snes *snes = SnesInit(rom_data, (int)rom_size);
    if (!snes) {
        fprintf(stderr, "SnesInit failed.\n");
        free(rom_data);
        ShutdownDesktop();
        return 1;
    }

    RtlEnsureSaveDir();
    RtlReadSram();

    memset(s_pixels, 0, sizeof(s_pixels));

    if (!InitVideo()) {
        snes_free(snes);
        free(rom_data);
        ShutdownDesktop();
        return 1;
    }

    if (!InitAudio()) {
        fprintf(stderr,
            "[audio] continuing without host playback; "
            "emulated APU still runs.\n");
    }

    TryOpenFirstGamepad();

    fprintf(stderr,
        "[desktop] entering main loop\n"
        "[desktop] use --launcher to force the launcher on the next start\n");

    bool running = true;
    uint32_t frame_counter = 0;
    uint64_t frame_deadline_ms = SDL_GetTicks();

    while (running && !g_fail) {
        running = HandleEvents();

        if (s_paused) {
            SDL_Delay(10);
            continue;
        }

        const uint32_t input =
            ReadKeyboardInput() |
            ReadGamepadInput();

        RtlRunFrame(input);


        int ppu_flags = 0;
        if (g_config.new_renderer)
            ppu_flags |= kPpuRenderFlags_NewRenderer;
        if (g_config.no_sprite_limits)
            ppu_flags |= kPpuRenderFlags_NoSpriteLimits;

        PpuBeginDrawing(
            g_ppu, s_pixels, SNES_WIDTH * 4, ppu_flags);
        Lufia2DrawPpuFrame();
        PresentFrame();

        frame_counter++;
        UpdatePerfTitle();

        if (frame_counter <= 10 || (frame_counter % 600) == 0)
            Lufia2PrintDiagnostics();

        if (!s_turbo && !g_config.disable_frame_delay) {
            static const uint8_t delays[3] = {17, 17, 16};
            frame_deadline_ms += delays[frame_counter % 3];

            const uint64_t now = SDL_GetTicks();
            if (frame_deadline_ms > now) {
                uint64_t wait = frame_deadline_ms - now;
                if (wait > 100)
                    wait = 100;
                SDL_Delay((Uint32)wait);
            } else if (now - frame_deadline_ms > 500) {
                frame_deadline_ms = now;
            }
        } else {
            frame_deadline_ms = SDL_GetTicks();
        }
    }

    fprintf(stderr,
        "[desktop] leaving main loop: frames=%u fail=%d\n",
        frame_counter, g_fail ? 1 : 0);
    Lufia2PrintDiagnostics();

    RtlWriteSram();


    if (s_audio_stream) {
        SDL_PauseAudioStreamDevice(s_audio_stream);
        SDL_DestroyAudioStream(s_audio_stream);
        s_audio_stream = NULL;
    }

    snes_free(snes);
    free(rom_data);
    ShutdownDesktop();

    return g_fail ? 1 : 0;
}
