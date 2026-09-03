/* Lufia II desktop host. */
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "desktop/sdl_compat.h"
#include "snesrecomp_platform/glsl_shader_adapter.h"
#include "snesrecomp_platform/presenter.h"
#include "snesrecomp_platform/msu_pack.h"
#include "snes/msu1.h"
#include "host_report.h"
#include "host_paths.h"
#include "launcher_cache.h"
#include "common_rtl.h"
#include "common_cpu_infra.h"
#include "snes/snes.h"
#include "snes/ppu.h"
#include "snes/ws_shadow.h"
#include "widescreen.h"
#include "desktop/display_aspect.h"

#include "recomp_launcher.h"
#include "launcher_profile.h"
#include "common/keybinds.h"
#include "common/sha1.h"
#include "common/launcher_binds.h"

#include "config.h"
#include "lufia2_map_load.h"
#include "lufia2_msu_driver.h"
#include "lufia2_map_widescreen.h"
#include "lufia2_runtime.h"
#include "lufia2_video_policy.h"
#include "desktop_glue.h"

enum {
    SNES_WIDTH = 256,
    SNES_WIDE_WIDTH = 342,
    SNES_WIDE_EXTRA = (SNES_WIDE_WIDTH - SNES_WIDTH) / 2,
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

    LUFIA2_MAP_LAYER_MASK = 0x03,
    LUFIA2_MAP_WINDOW_LAYER_MASK = 0x33,
    LUFIA2_WORLD_WINDOW_LAYER_MASK = 0x31,
    LUFIA2_OUTDOOR_WINDOW_MASK = 0x03,
    LUFIA2_MENU_REPEAT_LAYER_MASK = 0x02,
    LUFIA2_MENU_CLAMP_LAYER_MASK = 0x0d,
};

typedef enum PlayerInputSource {
    PLAYER_INPUT_NONE = 0,
    PLAYER_INPUT_KEYBOARD,
    PLAYER_INPUT_GAMEPAD,
} PlayerInputSource;

static const char kLufia2Sha1[] =
    "a89931c1f29b161b8be717dfab4a4adb54b42b84";

static const char *const kLufia2KnownSha1[] = {
    "a89931c1f29b161b8be717dfab4a4adb54b42b84",
};

enum {
    LUFIA2_LAUNCHER_RENDERER_SDL = 0,
    LUFIA2_LAUNCHER_RENDERER_SDL_SOFTWARE,
    LUFIA2_LAUNCHER_RENDERER_OPENGL,
    LUFIA2_LAUNCHER_RENDERER_COUNT,
};

static const char *const kLufia2LauncherRendererLabels[] = {
    "SDL Accelerated",
    "SDL Software",
    "OpenGL 3.3",
};

extern Ppu *g_ppu;
extern bool g_fail;
extern uint8_t g_ram[0x20000];

static SnesRecompPresenter *s_presenter;
static SDL_AudioStream *s_audio_stream;
static SDL_Gamepad *s_gamepad;

static uint8_t s_pixels[SNES_WIDE_WIDTH * 4 * PPU_BUFFER_HEIGHT];
static uint8_t s_present_pixels[SNES_WIDE_WIDTH * 4 * SNES_HEIGHT];
static uint8_t *s_audio_scratch;
static size_t s_audio_scratch_size;

static bool s_paused;
static bool s_turbo;
static bool s_fullscreen;
static bool s_display_perf;
static int s_current_window_scale = DEFAULT_WINDOW_SCALE;
static int s_volume_percent = 100;
static PlayerInputSource s_player1_source = PLAYER_INPUT_KEYBOARD;
static int s_frame_width = SNES_WIDTH;
static bool s_vsync_enabled = true;
static bool s_window_resize_pending;
static Lufia2VideoLayout s_last_video_layout = LUFIA2_VIDEO_LAYOUT_COUNT;
static Lufia2VideoLayout s_held_video_layout = LUFIA2_VIDEO_CENTERED;

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

static char *TrimAscii(char *text) {
    while (*text && isspace((unsigned char)*text))
        text++;
    char *end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1]))
        *--end = '\0';
    return text;
}

static bool AsciiEqualsNoCase(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return false;
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

static bool ReadIniBool(
    const char *path,
    const char *wanted_section,
    const char *wanted_key,
    bool fallback) {
    FILE *f = fopen(path, "rb");
    if (!f)
        return fallback;

    char section[64] = "";
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *comment = strpbrk(line, "#;");
        if (comment)
            *comment = '\0';
        char *text = TrimAscii(line);
        const size_t length = strlen(text);
        if (length >= 2 && text[0] == '[' && text[length - 1] == ']') {
            text[length - 1] = '\0';
            snprintf(section, sizeof(section), "%s", TrimAscii(text + 1));
            continue;
        }
        if (!AsciiEqualsNoCase(section, wanted_section))
            continue;

        char *equals = strchr(text, '=');
        if (!equals)
            continue;
        *equals = '\0';
        if (!AsciiEqualsNoCase(TrimAscii(text), wanted_key))
            continue;

        char *value = TrimAscii(equals + 1);
        fclose(f);
        if (AsciiEqualsNoCase(value, "1") ||
            AsciiEqualsNoCase(value, "true") ||
            AsciiEqualsNoCase(value, "on")) {
            return true;
        }
        if (AsciiEqualsNoCase(value, "0") ||
            AsciiEqualsNoCase(value, "false") ||
            AsciiEqualsNoCase(value, "off")) {
            return false;
        }
        return fallback;
    }

    fclose(f);
    return fallback;
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
        "OutputMethod = OpenGL\n"
        "LinearFiltering = 0\n"
        "Shader =\n"
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

static bool EnsureDefaultPlatformConfig(const char *path) {
    if (FileExists(path))
        return true;

    FILE *f = fopen(path, "wb");
    if (!f)
        return false;

    static const char kDefaultPlatformConfig[] =
        "[Video]\n"
        "VSync = 1\n";
    const size_t n = sizeof(kDefaultPlatformConfig) - 1;
    const bool ok = fwrite(kDefaultPlatformConfig, 1, n, f) == n;
    fclose(f);
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

static int LauncherRendererFromOutputMethod(int output_method) {
    switch (output_method) {
    case kOutputMethod_SDLSoftware:
        return LUFIA2_LAUNCHER_RENDERER_SDL_SOFTWARE;
    case kOutputMethod_OpenGL:
        return LUFIA2_LAUNCHER_RENDERER_OPENGL;
    case kOutputMethod_SDL:
    default:
        return LUFIA2_LAUNCHER_RENDERER_SDL;
    }
}

static uint8 OutputMethodFromLauncherRenderer(int renderer) {
    switch (renderer) {
    case LUFIA2_LAUNCHER_RENDERER_SDL_SOFTWARE:
        return kOutputMethod_SDLSoftware;
    case LUFIA2_LAUNCHER_RENDERER_OPENGL:
        return kOutputMethod_OpenGL;
    case LUFIA2_LAUNCHER_RENDERER_SDL:
    default:
        return kOutputMethod_SDL;
    }
}

static PlayerInputSource PlayerInputSourceFromLauncher(int source) {
    switch (source) {
    case PLAYER_INPUT_NONE:
        return PLAYER_INPUT_NONE;
    case PLAYER_INPUT_GAMEPAD:
        return PLAYER_INPUT_GAMEPAD;
    case PLAYER_INPUT_KEYBOARD:
    default:
        return PLAYER_INPUT_KEYBOARD;
    }
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
    if (!EnsureDefaultPlatformConfig("platform.ini")) {
        fprintf(stderr,
            "[config] warning: could not create platform.ini\n");
    }
    s_vsync_enabled = ReadIniBool(
        "platform.ini", "Video", "VSync", true);

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
    ls.renderer =
        LauncherRendererFromOutputMethod(g_config.output_method);
    ls.window_scale = g_config.window_scale
        ? g_config.window_scale : DEFAULT_WINDOW_SCALE;
    ls.fullscreen = g_config.fullscreen;
    ls.ignore_aspect = g_config.ignore_aspect_ratio ? 1 : 0;
    ls.linear_filter = g_config.linear_filtering ? 1 : 0;
    ls.widescreen = g_config.widescreen ? 1 : 0;
    ls.enable_audio = g_config.enable_audio ? 1 : 0;
    ls.audio_freq = g_config.audio_freq ? g_config.audio_freq : 32040;
    ls.volume = 100;
    ls.player_src[0] = g_config.enable_gamepad[0]
        ? PLAYER_INPUT_GAMEPAD
        : PLAYER_INPUT_KEYBOARD;
    ls.player_src[1] = PLAYER_INPUT_NONE;
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
    gi.widescreen_supported = 1;
    gi.num_players = 1;
    gi.msu1_supported = 0;
    gi.config_path = "config.ini";
    gi.keybinds_path = "keybinds.ini";
    gi.has_renderer = 1;
    gi.renderer_labels = kLufia2LauncherRendererLabels;
    gi.num_renderers = LUFIA2_LAUNCHER_RENDERER_COUNT;

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

    g_config.output_method =
        OutputMethodFromLauncherRenderer(ls.renderer);
    g_config.window_scale =
        (uint8)(ls.window_scale > 0 ? ls.window_scale : DEFAULT_WINDOW_SCALE);
    g_config.fullscreen = (uint8)ls.fullscreen;
    g_config.ignore_aspect_ratio = ls.ignore_aspect != 0;
    g_config.linear_filtering = ls.linear_filter != 0;
    g_config.widescreen = ls.widescreen != 0;
    g_config.enable_audio = ls.enable_audio != 0;
    g_config.audio_freq = (uint16)ls.audio_freq;
    g_config.enable_gamepad[0] =
        ls.player_src[0] == PLAYER_INPUT_GAMEPAD;
    g_config.enable_gamepad[1] = false;
    g_config.gamepad_deadzone = ls.deadzone[0] * 32767 / 100;
    g_config.skip_launcher = ls.skip_launcher != 0;

    s_player1_source = PlayerInputSourceFromLauncher(ls.player_src[0]);
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
    if (s_player1_source != PLAYER_INPUT_KEYBOARD)
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
    if (s_player1_source != PLAYER_INPUT_GAMEPAD || !s_gamepad)
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
    if (s_player1_source != PLAYER_INPUT_GAMEPAD || s_gamepad)
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
    s_frame_width = g_config.widescreen
        ? SNES_WIDE_WIDTH : SNES_WIDTH;
    g_ws_active = g_config.widescreen;
    g_ws_extra = g_ws_active ? SNES_WIDE_EXTRA : 0;

    SnesRecompPresentConfig config;
    memset(&config, 0, sizeof(config));
    config.window_title =
        "Lufia II: Rise of the Sinistrals (Recompiled)";
    if (g_config.output_method == kOutputMethod_OpenGL) {
        config.backend = SNESRECOMP_PRESENT_BACKEND_OPENGL;
    } else if (g_config.output_method == kOutputMethod_SDLSoftware) {
        config.backend = SNESRECOMP_PRESENT_BACKEND_SDL_SOFTWARE;
    } else {
        config.backend = SNESRECOMP_PRESENT_BACKEND_SDL;
    }
    config.pixel_format = SNESRECOMP_PIXEL_FORMAT_ARGB8888;
    config.frame_width = s_frame_width;
    config.frame_height = SNES_HEIGHT;
    config.window_scale = scale;
    config.vsync = s_vsync_enabled;
    SnesDisplayAspect_GetPixelAspect(
        SnesDisplayAspect_Clamp(g_config.display_aspect),
        &config.pixel_aspect_numerator,
        &config.pixel_aspect_denominator);
    config.preserve_aspect = !g_config.ignore_aspect_ratio;
    config.linear_filtering = g_config.linear_filtering;
    config.fullscreen = g_config.fullscreen != 0;
    if (config.backend == SNESRECOMP_PRESENT_BACKEND_OPENGL) {
        config.shader_preset_path = g_config.shader;
        config.shader_preset_interface =
            snesrecomp_glsl_shader_preset_interface();
    }

    char error[256];
    if (!snesrecomp_presenter_create(
            &config, &s_presenter, error, sizeof(error))) {
        if (config.backend != SNESRECOMP_PRESENT_BACKEND_OPENGL) {
            fprintf(stderr, "Presenter creation failed: %s\n", error);
            return false;
        }

        fprintf(stderr,
            "[video] OpenGL initialization failed: %s\n"
            "[video] Falling back to the SDL renderer for this session.\n",
            error);
        host_report_breadcrumb(
            "OpenGL presenter failed; SDL fallback: %s", error);
        config.backend = SNESRECOMP_PRESENT_BACKEND_SDL;
        config.shader_preset_path = NULL;
        config.shader_preset_interface = NULL;
        if (!snesrecomp_presenter_create(
                &config, &s_presenter, error, sizeof(error))) {
            fprintf(stderr,
                "SDL fallback presenter creation failed: %s\n", error);
            return false;
        }
    }

    s_fullscreen = config.fullscreen;

    host_report_breadcrumb(
        "video ready: %dx%d scale=%d method=%s fullscreen=%d linear=%d vsync=%s caps=0x%x",
        s_frame_width, SNES_HEIGHT, scale,
        snesrecomp_presenter_backend_name(s_presenter),
        g_config.fullscreen,
        g_config.linear_filtering ? 1 : 0,
        snesrecomp_vsync_state_name(
            snesrecomp_presenter_vsync_state(s_presenter)),
        (unsigned)snesrecomp_presenter_capabilities(s_presenter));
    const bool preset_active =
        snesrecomp_presenter_backend(s_presenter) ==
            SNESRECOMP_PRESENT_BACKEND_OPENGL &&
        g_config.shader && g_config.shader[0];
    fprintf(stderr,
        "[video] presenter ready: %s, capabilities=0x%x, VSync %s%s\n",
        snesrecomp_presenter_backend_name(s_presenter),
        (unsigned)snesrecomp_presenter_capabilities(s_presenter),
        snesrecomp_vsync_state_name(
            snesrecomp_presenter_vsync_state(s_presenter)),
        preset_active ? ", GLSL preset active" : "");
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

static bool PresentFrame(void) {
    RtlWidescreenPresent(
        s_present_pixels,
        (size_t)s_frame_width * 4,
        s_pixels,
        s_frame_width,
        SNES_HEIGHT);
    const SnesRecompVideoFrame frame = {
        .pixels = s_present_pixels,
        .pixel_format = SNESRECOMP_PIXEL_FORMAT_ARGB8888,
        .width = s_frame_width,
        .height = SNES_HEIGHT,
        .pitch = s_frame_width * 4,
    };
    if (!snesrecomp_presenter_present(s_presenter, &frame)) {
        fprintf(stderr, "Present failed: %s\n",
                snesrecomp_presenter_last_error(s_presenter));
        return false;
    }
    if (s_window_resize_pending) {
        if (!snesrecomp_presenter_set_window_scale(
                s_presenter, s_current_window_scale)) {
            fprintf(stderr, "Window resize failed: %s\n",
                snesrecomp_presenter_last_error(s_presenter));
        }
        s_window_resize_pending = false;
    }
    return true;
}

static void PrepareVideoFrame(void) {
    g_ws_active = g_config.widescreen;
    g_ws_extra = g_ws_active ? SNES_WIDE_EXTRA : 0;
    s_frame_width = g_ws_active ? SNES_WIDE_WIDTH : SNES_WIDTH;

    memset(s_pixels, 0,
        (size_t)s_frame_width * 4 * PPU_BUFFER_HEIGHT);

    Lufia2MapLoadFrame();

    /* PpuResetLayerPolicies() clears clamp, mirror, repeat and the window
       expansion, but not the widen mask, so a mask set for one scene
       survives into the next: the menu restricts the margins to BG2, and
       the Mode 7 overworld that follows is layer 0, which the stale mask
       then excludes from the margins entirely. Publish it per frame like
       every other policy; each branch sets its own afterwards. */
    if (g_ppu)
        PpuSetWidescreenLayerMask(g_ppu, 0);

    Lufia2VideoLayout layout =
        Lufia2SelectVideoLayout(g_ppu, g_ws_active);
    if (layout == LUFIA2_VIDEO_BLANK) {
        /* Nothing reaches the screen during a fade, so hold the last
           decision and keep preparing. The map source follows the
           player through the transition and the shadow stays keyed to
           the live camera, so the fade-in shows the destination room
           instead of whatever survived the blank. */
        layout = s_held_video_layout;
    } else {
        s_held_video_layout = layout;
    }
    bool finalize_map_widescreen = false;
    switch (layout) {
    case LUFIA2_VIDEO_WORLD_MAP:
        Lufia2DeactivateMapWidescreen();
        PpuSetExtraSpace(g_ppu, (uint8_t)g_ws_extra);
        PpuSetWidescreenWindowExpansion(
            g_ppu,
            LUFIA2_WORLD_WINDOW_LAYER_MASK,
            LUFIA2_OUTDOOR_WINDOW_MASK);
        break;

    case LUFIA2_VIDEO_REGULAR_MAP:
        switch (Lufia2PrepareMapWidescreen(g_ppu, g_ws_extra)) {
        case LUFIA2_MAP_WIDESCREEN_ACTIVE:
            PpuSetExtraSpace(g_ppu, (uint8_t)g_ws_extra);
            PpuSetWidescreenLayerMask(g_ppu, LUFIA2_MAP_LAYER_MASK);
            PpuSetWidescreenWindowExpansion(
                g_ppu,
                LUFIA2_MAP_WINDOW_LAYER_MASK,
                LUFIA2_OUTDOOR_WINDOW_MASK);
            finalize_map_widescreen = true;
            break;
        case LUFIA2_MAP_WIDESCREEN_LOADING:
            /* Keep the 16:9 canvas but show only the authentic 256 pixels
               until the new map is readable. */
            layout = LUFIA2_VIDEO_MAP_LOADING;
            PpuSetExtraSpaceCentered(g_ppu, (uint8_t)g_ws_extra);
            break;
        case LUFIA2_MAP_WIDESCREEN_DISABLED:
        default:
            layout = LUFIA2_VIDEO_CENTERED;
            PpuSetExtraSpaceCentered(g_ppu, (uint8_t)g_ws_extra);
            break;
        }
        break;

    case LUFIA2_VIDEO_MAP_LOADING:
        /* Only reached as a relabelled centered frame. */
        PpuSetExtraSpaceCentered(g_ppu, (uint8_t)g_ws_extra);
        break;

    case LUFIA2_VIDEO_PATTERN_MENU:
        Lufia2DeactivateMapWidescreen();
        PpuSetExtraSpace(g_ppu, (uint8_t)g_ws_extra);
        PpuSetWidescreenLayerMask(
            g_ppu, LUFIA2_MENU_REPEAT_LAYER_MASK);
        PpuSetWidescreenLayerRepeat(
            g_ppu, LUFIA2_MENU_REPEAT_LAYER_MASK);
        PpuSetWidescreenLayerClamp(
            g_ppu, LUFIA2_MENU_CLAMP_LAYER_MASK);
        break;

    case LUFIA2_VIDEO_CENTERED:
        Lufia2DeactivateMapWidescreen();
        /* A blanked or reconfigured screen during a map load is the load,
           not an unsupported scene. */
        if (Lufia2MapLoadInProgress())
            layout = LUFIA2_VIDEO_MAP_LOADING;
        PpuSetExtraSpaceCentered(g_ppu, (uint8_t)g_ws_extra);
        break;

    case LUFIA2_VIDEO_BLANK:
    case LUFIA2_VIDEO_NATIVE:
    default:
        Lufia2DeactivateMapWidescreen();
        PpuSetExtraSpace(g_ppu, 0);
        break;
    }

    WsShadowFrame(g_ppu);
    if (finalize_map_widescreen)
        Lufia2FinalizeMapWidescreen(g_ppu, g_ws_extra);

    if (layout != s_last_video_layout) {
        fprintf(stderr,
            "[video] layout: %s (%dx%d, PPU mode=%u, main=$%02X, "
            "BG maps=%u/%u, scroll=%u,%u, map=$%02X, world=%u,%u)\n",
            Lufia2VideoLayoutName(layout),
            s_frame_width, SNES_HEIGHT,
            g_ppu ? (unsigned)PPU_mode(g_ppu) : 0,
            g_ppu ? (unsigned)g_ppu->screenEnabled[0] : 0,
            g_ppu ? (unsigned)(g_ppu->bgXsc[0] & 3) : 0,
            g_ppu ? (unsigned)(g_ppu->bgXsc[1] & 3) : 0,
            g_ppu ? (unsigned)g_ppu->hScroll[0] : 0,
            g_ppu ? (unsigned)g_ppu->vScroll[0] : 0,
            (unsigned)g_ram[0x05ac],
            (unsigned)(g_ram[0x0594] | (g_ram[0x0595] << 8)),
            (unsigned)(g_ram[0x0596] | (g_ram[0x0597] << 8)));
        s_last_video_layout = layout;
    }
}

static void ToggleFullscreen(void) {
    const bool requested = !s_fullscreen;
    if (!snesrecomp_presenter_set_fullscreen(
            s_presenter, requested)) {
        fprintf(stderr, "Fullscreen change failed: %s\n",
                snesrecomp_presenter_last_error(s_presenter));
        return;
    }
    s_fullscreen = requested;
    g_config.fullscreen = s_fullscreen ? 1 : 0;
    PersistInt("Graphics", "Fullscreen", g_config.fullscreen);
}

static void ResizeWindow(int delta) {
    int requested_scale = s_current_window_scale + delta;
    if (requested_scale < 1)
        requested_scale = 1;
    if (requested_scale > 10)
        requested_scale = 10;

    if (!snesrecomp_presenter_set_window_scale(
            s_presenter, requested_scale)) {
        fprintf(stderr, "Window resize failed: %s\n",
                snesrecomp_presenter_last_error(s_presenter));
        return;
    }

    s_current_window_scale = requested_scale;
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
        g_config.widescreen = !g_config.widescreen;
        PersistInt("Graphics", "Widescreen",
                   g_config.widescreen ? 1 : 0);
        s_window_resize_pending = true;
        fprintf(stderr, "[video] widescreen: %s\n",
            g_config.widescreen ? "enabled" : "disabled");
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
    if (!s_display_perf || !s_presenter)
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
        if (!snesrecomp_presenter_set_window_title(
                s_presenter, title)) {
            fprintf(stderr, "Window title update failed: %s\n",
                    snesrecomp_presenter_last_error(s_presenter));
        }
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

    snesrecomp_presenter_destroy(s_presenter);
    s_presenter = NULL;

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


    s_player1_source = g_config.enable_gamepad[0]
        ? PLAYER_INPUT_GAMEPAD
        : PLAYER_INPUT_KEYBOARD;
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

    /* RtlRegisterGame already ran msu1_init() against an empty
     * environment, so re-run it once the pack is known. */
    {
        SnesRecompMsuRequest msu;
        memset(&msu, 0, sizeof(msu));
        msu.ini_path = "config.ini";
        msu.driver_present = true;
        const SnesRecompMsuStatus *m = snesrecomp_msu_resolve(&msu);
        fprintf(stderr, "[msu] directory: %s\n", m->directory);
        if (m->pack_found)
            fprintf(stderr, "[msu] pack: base=%s tracks=%d\n",
                    m->pack_base, m->track_count);
        else
            fprintf(stderr, "[msu] pack: none\n");
        fprintf(stderr, "[msu] runtime: %s; %s\n",
                m->armed ? "enabled" : "inactive", m->reason);
        if (m->armed) {
            msu1_init();
            Lufia2MsuDriverInstall();
        }
    }

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

    Lufia2MapLoadInstallHooks();

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
            ReadKeyboardInput() | ReadGamepadInput();

        RtlRunFrame(input);

        PrepareVideoFrame();

        int ppu_flags = 0;
        if (g_config.new_renderer || g_ws_active)
            ppu_flags |= kPpuRenderFlags_NewRenderer;
        if (g_config.no_sprite_limits)
            ppu_flags |= kPpuRenderFlags_NoSpriteLimits;

        Lufia2BeginMapRenderOverlay(g_ppu);
        PpuBeginDrawing(
            g_ppu, s_pixels, (size_t)s_frame_width * 4, ppu_flags);
        Lufia2DrawPpuFrame();
        Lufia2EndMapRenderOverlay(g_ppu);
        if (!PresentFrame()) {
            g_fail = true;
            break;
        }

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
