#ifdef LUFIA2_ENABLE_QUIESCENCE_INDEX
#include "patches/quiescence_index.h"
#endif
#ifdef LUFIA2_ENABLE_BRIDGE_AUDIT
#include "src/diagnostics/lufia2_bridge_audit.h"
#endif
#if defined(LUFIA2_ENABLE_NATIVE_WAIT) || defined(LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD) || \
    defined(LUFIA2_ENABLE_ACTOR_EARLY_RETURN)
#include "patches/native_patches.h"
#endif
#if defined(LUFIA2_ENABLE_DMA_HOST_FASTFORWARD) || \
    defined(LUFIA2_ENABLE_DMA_DIRECT_SOURCE_READ)
#include "patches/dma_host_fastforward.h"
#endif
#include "diagnostics/lufia2_gameplay_capture.h"
/* Lufia II desktop host. */
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "desktop/sdl_compat.h"
#include "snesrecomp_platform/glsl_shader_adapter.h"
#include "snesrecomp_platform/present_timeline.h"
#include "snesrecomp_platform/presenter.h"
#include "snesrecomp_platform/snes_bg_upload.h"
#include "snesrecomp_platform/task.h"
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
#include "lufia2_overlay_ui.h"
#include "lufia2_ui_assets.h"

#include "recomp_launcher.h"
#include "launcher_profile.h"
#include "common/keybinds.h"
#include "common/sha1.h"
#include "common/launcher_binds.h"
#include "snes_osd.h"
#include "snes_rewind.h"

#include "config.h"
#include "lufia2_log.h"
#include "lufia2_map_load.h"
#include "lufia2_msu_driver.h"
#include "lufia2_map_widescreen.h"
#include "lufia2_intro_mode7_world.h"
#include "lufia2_mode7_substep.h"
#include "lufia2_intro_widescreen.h"
#include "lufia2_runtime.h"
#include "lufia2_splash_credit.h"
#include "lufia2_video_handoff.h"
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

    /* BG1 and BG2 carry the map; BG3 is its parallax backdrop. */
    LUFIA2_MAP_LAYER_MASK = 0x07,
    LUFIA2_MAP_BACKDROP_LAYER_MASK = 0x04,
    LUFIA2_MAP_WINDOW_LAYER_MASK = 0x33,
    LUFIA2_WORLD_WINDOW_LAYER_MASK = 0x31,
    LUFIA2_OUTDOOR_WINDOW_MASK = 0x03,
    LUFIA2_MODE7_LAYER_MASK = 0x01,
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
#if SNESRECOMP_PLATFORM_HAS_VULKAN
    LUFIA2_LAUNCHER_RENDERER_VULKAN,
#endif
    LUFIA2_LAUNCHER_RENDERER_COUNT,
};

static const char *const kLufia2LauncherRendererLabels[] = {
    "SDL Accelerated",
    "SDL Software",
    "OpenGL 3.3",
#if SNESRECOMP_PLATFORM_HAS_VULKAN
    "Vulkan",
#endif
};

/* The launcher config enum comes from the pinned snesrecomp source and has
 * no Vulkan entry, so the value is read here rather than by patching a
 * hash-pinned file. */
static bool s_output_vulkan;

extern Ppu *g_ppu;
extern bool g_fail;
extern uint8_t g_ram[0x20000];

static SnesRecompPresenter *s_presenter;
static SDL_AudioStream *s_audio_stream;
static SDL_Gamepad *s_gamepad;

static uint8_t s_pixels[SNES_WIDE_WIDTH * 4 * PPU_BUFFER_HEIGHT];
static uint8_t s_present_pixels[SNES_WIDE_WIDTH * 4 * SNES_HEIGHT];
static uint8_t s_map_handoff_pixels[SNES_WIDE_WIDTH * 4 * SNES_HEIGHT];
static uint16_t s_handoff_line_scroll_x[SNES_HEIGHT];
static uint16_t s_handoff_line_scroll_y[SNES_HEIGHT];
static uint8_t s_handoff_line_mosaic[SNES_HEIGHT];
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
static bool s_rewind_requested;
static uint64_t s_state_generation;
static uint32_t s_input_release_mask;
static Lufia2VideoLayout s_last_video_layout = LUFIA2_VIDEO_LAYOUT_COUNT;
static Lufia2VideoLayout s_held_video_layout = LUFIA2_VIDEO_CENTERED;
static Lufia2VideoLayout s_current_video_layout = LUFIA2_VIDEO_CENTERED;
static Lufia2VideoHandoff s_video_handoff;
static uint64_t s_last_intro_raster_reject_signature = UINT64_MAX;

typedef enum Lufia2VisualPreset {
    LUFIA2_VISUAL_ORIGINAL = 0,
    LUFIA2_VISUAL_CLEAN_HD,
} Lufia2VisualPreset;

static Lufia2VisualPreset s_visual_preset = LUFIA2_VISUAL_CLEAN_HD;
static unsigned s_hd_mode7_scale = 2;
/* The launcher's checkbox must not forget the configured scale. */
static unsigned s_hd_mode7_preferred_scale = 2;
/* Present on the display's clock, not the guest's. */
static bool s_high_refresh;
/* Overrides the reported refresh; a remote session misreports it. */
static unsigned s_present_rate_millihertz;
static SnesRecompPresentTimeline s_present_timeline;
/* The last picture can be shown again without recomposing. */
static bool s_frame_repeatable;
static bool s_hd_mode7_perspective;
static bool s_hd_mode7_filter;
static SnesRecompMode7Line s_hd_mode7_lines[SNES_HEIGHT];
static SnesRecompMode7Line s_intro_mode7_world_lines[SNES_HEIGHT];
static SnesRecompMode7MapSource s_intro_mode7_world_source;
static SnesRecompObjSliver
    s_hd_mode7_obj_slivers[SNESRECOMP_OBJ_MAX_SLIVERS];
static uint8_t s_hd_mode7_obj_line_priority[SNES_HEIGHT];
static SnesPpuUnsupported s_hd_mode7_last_reject = SNES_PPU_SUPPORTED;
static bool s_hd_mode7_present_error_reported;
static Lufia2IntroMode7WorldStatus s_last_intro_world_status =
    LUFIA2_INTRO_WORLD_INVALID_ARGUMENT;
static bool s_intro_mode7_world_active_reported;

static const char kCleanHdPresetPath[] =
    "assets/shaders/clean-hd/clean-hd.glslp";

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

static bool ReadIniText(const char *path, const char *wanted_section,
                        const char *wanted_key, char *out,
                        size_t out_capacity) {
    FILE *f;
    char section[64] = "";
    char line[512];

    if (!out || !out_capacity)
        return false;
    out[0] = '\0';
    f = fopen(path, "rb");
    if (!f)
        return false;

    while (fgets(line, sizeof(line), f)) {
        char *comment = strpbrk(line, "#;");
        char *text;
        size_t length;
        char *equals;
        if (comment)
            *comment = '\0';
        text = TrimAscii(line);
        length = strlen(text);
        if (length >= 2 && text[0] == '[' && text[length - 1] == ']') {
            text[length - 1] = '\0';
            snprintf(section, sizeof(section), "%s", TrimAscii(text + 1));
            continue;
        }
        if (!AsciiEqualsNoCase(section, wanted_section))
            continue;
        equals = strchr(text, '=');
        if (!equals)
            continue;
        *equals = '\0';
        if (!AsciiEqualsNoCase(TrimAscii(text), wanted_key))
            continue;
        snprintf(out, out_capacity, "%s", TrimAscii(equals + 1));
        fclose(f);
        return true;
    }
    fclose(f);
    return false;
}

/* `Off`, or a scale written as `2`, `2x` or `x2`. */
static unsigned Lufia2HdMode7ScaleFromText(const char *text,
                                           unsigned fallback) {
    unsigned scale = 0;

    if (!text || !text[0])
        return fallback;
    if (AsciiEqualsNoCase(text, "Off") || AsciiEqualsNoCase(text, "0"))
        return 0;
    if (*text == 'x' || *text == 'X')
        text++;
    while (*text >= '0' && *text <= '9')
        scale = scale * 10u + (unsigned)(*text++ - '0');
    if (*text == 'x' || *text == 'X')
        text++;
    if (*text || !scale || scale > SNESRECOMP_MODE7_MAX_SCALE)
        return fallback;
    return scale;
}

static const char *Lufia2HdMode7ScaleName(unsigned scale) {
    static const char *const kNames[SNESRECOMP_MODE7_MAX_SCALE + 1u] = {
        "Off", "1x", "2x", "3x", "4x", "5x", "6x", "7x", "8x",
    };
    return scale <= SNESRECOMP_MODE7_MAX_SCALE ? kNames[scale] : "?";
}

static void LoadVisualConfig(const char *path) {
    char value[64];

    s_output_vulkan = false;
#if SNESRECOMP_PLATFORM_HAS_VULKAN
    if (ReadIniText(path, "Graphics", "OutputMethod", value,
                    sizeof(value)) &&
        AsciiEqualsNoCase(value, "Vulkan"))
        s_output_vulkan = true;
#endif

    s_visual_preset = LUFIA2_VISUAL_CLEAN_HD;
    if (ReadIniText(path, "Graphics", "VisualPreset",
                    value, sizeof(value)) &&
        AsciiEqualsNoCase(value, "Original"))
        s_visual_preset = LUFIA2_VISUAL_ORIGINAL;

    s_hd_mode7_scale = 2;
    if (ReadIniText(path, "Graphics", "HDMode7", value, sizeof(value)))
        s_hd_mode7_scale = Lufia2HdMode7ScaleFromText(value, 2u);
    if (s_hd_mode7_scale)
        s_hd_mode7_preferred_scale = s_hd_mode7_scale;

    /* Both refine the HD pass only; the ordinary path stays exact. Filtering
     * is off by default: it softens a palette image more than it smooths it,
     * and the world map is meant to stay pixel exact. */
    s_hd_mode7_filter = false;
    if (ReadIniText(path, "Graphics", "HDMode7Filter", value,
                    sizeof(value)) &&
        (AsciiEqualsNoCase(value, "On") ||
         AsciiEqualsNoCase(value, "1")))
        s_hd_mode7_filter = true;

    s_high_refresh = false;
    if (ReadIniText(path, "Graphics", "PresentRate", value, sizeof(value)) &&
        (AsciiEqualsNoCase(value, "Display") ||
         AsciiEqualsNoCase(value, "On")))
        s_high_refresh = true;

    s_present_rate_millihertz = 0;
    if (ReadIniText(path, "Graphics", "PresentRateHz", value, sizeof(value))) {
        const double hz = atof(value);
        if (hz > 0.0 && hz < 1000.0)
            s_present_rate_millihertz = (unsigned)(hz * 1000.0 + 0.5);
    }

    s_hd_mode7_perspective = true;
    if (ReadIniText(path, "Graphics", "HDMode7Perspective",
                    value, sizeof(value)) &&
        (AsciiEqualsNoCase(value, "Off") ||
         AsciiEqualsNoCase(value, "0")))
        s_hd_mode7_perspective = false;
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
        "# OutputMethod: SDL, SDL-Software, OpenGL, Vulkan.\n"
        "# Vulkan needs a build configured with LUFIA2_ENABLE_VULKAN=ON;\n"
        "# it falls back to OpenGL when it cannot start.\n"
        "OutputMethod = OpenGL\n"
        "LinearFiltering = 0\n"
        "Shader =\n"
        "VisualPreset = CleanHD\n"
        "HDMode7 = 2x\n"
        "HDMode7Filter = Off\n"
        "HDMode7Perspective = On\n"
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
        "Load = F1, F2, F3, F4, F5, F6, F7, F8, F9, F10\n"
        "Save = Shift+F1, Shift+F2, Shift+F3, Shift+F4, Shift+F5, Shift+F6, Shift+F7, Shift+F8, Shift+F9, Shift+F10\n"
        "Reset =\n"
        "ToggleWidescreen =\n"
        "Fullscreen = Alt+Return\n"
        "Pause = Shift+p\n"
        "PauseDimmed = p\n"
        "Turbo = Tab\n"
        "DisplayPerf = f\n"
        "ToggleRenderer = r\n"
        "WindowBigger =\n"
        "WindowSmaller =\n"
        "VolumeUp =\n"
        "VolumeDown =\n"
        "Rewind = F12\n";

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

static void PersistText(const char *section, const char *key,
                        const char *value) {
    launcher_ini_kv_write(
        "config.ini", section, key, value ? value : "");
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
#if SNESRECOMP_PLATFORM_HAS_VULKAN
    if (s_output_vulkan)
        return LUFIA2_LAUNCHER_RENDERER_VULKAN;
#endif
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
#if SNESRECOMP_PLATFORM_HAS_VULKAN
    s_output_vulkan = renderer == LUFIA2_LAUNCHER_RENDERER_VULKAN;
    if (s_output_vulkan) {
        /* The stored enum keeps OpenGL, the fallback if Vulkan cannot start; the
         * Vulkan spelling is written over it afterwards. */
        return kOutputMethod_OpenGL;
    }
#endif
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
    LoadVisualConfig("config.ini");
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
    ls.sharp_filter =
        s_visual_preset == LUFIA2_VISUAL_CLEAN_HD ? 1 : 0;
    ls.affine_filter = s_hd_mode7_scale ? 1 : 0;
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
    gi.has_sharp_filter = 1;
    gi.has_affine_filter = 1;

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
    s_visual_preset = ls.sharp_filter
        ? LUFIA2_VISUAL_CLEAN_HD : LUFIA2_VISUAL_ORIGINAL;
    /* The checkbox toggles the enhancement, it does not pick the scale. */
    s_hd_mode7_scale = ls.affine_filter ? s_hd_mode7_preferred_scale : 0u;
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
#if SNESRECOMP_PLATFORM_HAS_VULKAN
    if (s_output_vulkan)
        PersistText("Graphics", "OutputMethod", "Vulkan");
#endif
    PersistInt("Graphics", "Fullscreen", g_config.fullscreen);
    PersistInt("Graphics", "IgnoreAspectRatio",
               g_config.ignore_aspect_ratio ? 1 : 0);
    PersistText("Graphics", "VisualPreset",
        s_visual_preset == LUFIA2_VISUAL_CLEAN_HD
            ? "CleanHD" : "Original");
    PersistText("Graphics", "HDMode7",
        Lufia2HdMode7ScaleName(s_hd_mode7_scale));

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

static bool RewindGesturePressed(void) {
    static bool was_held;
    const bool held = s_gamepad &&
        SDL_GetGamepadButton(s_gamepad, SDL_GAMEPAD_BUTTON_BACK) &&
        SDL_GetGamepadButton(s_gamepad, SDL_GAMEPAD_BUTTON_RIGHT_STICK);
    const bool pressed = held && !was_held;
    was_held = held;
    return pressed;
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
#if SNESRECOMP_PLATFORM_HAS_VULKAN
    if (s_output_vulkan) {
        config.backend = SNESRECOMP_PRESENT_BACKEND_VULKAN;
    } else
#endif
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
        config.shader_preset_path =
            s_visual_preset == LUFIA2_VISUAL_CLEAN_HD
                ? kCleanHdPresetPath : g_config.shader;
        config.shader_preset_interface =
            snesrecomp_glsl_shader_preset_interface();
    } else if (config.backend == SNESRECOMP_PRESENT_BACKEND_VULKAN) {
        /* Clean-HD reaches Vulkan as a scaling policy, not a preset file: the
         * backend runs the same two passes natively and parses no .glslp. */
        config.scaling = s_visual_preset == LUFIA2_VISUAL_CLEAN_HD
            ? SNESRECOMP_PRESENT_SCALING_SHARP_BILINEAR
            : SNESRECOMP_PRESENT_SCALING_DEFAULT;
    }

    char error[256];
    bool created = snesrecomp_presenter_create(
        &config, &s_presenter, error, sizeof(error));

    /* Each backend is tried once, in descending capability, so a machine with
     * no usable Vulkan driver keeps HD Mode 7 and Clean-HD. */
    if (!created && config.backend == SNESRECOMP_PRESENT_BACKEND_VULKAN) {
        fprintf(stderr,
            "[video] Vulkan initialization failed: %s\n"
            "[video] Falling back to OpenGL for this session.\n",
            error);
        host_report_breadcrumb(
            "Vulkan presenter failed; OpenGL fallback: %s", error);
        config.backend = SNESRECOMP_PRESENT_BACKEND_OPENGL;
        config.scaling = SNESRECOMP_PRESENT_SCALING_DEFAULT;
        config.shader_preset_path =
            s_visual_preset == LUFIA2_VISUAL_CLEAN_HD
                ? kCleanHdPresetPath : g_config.shader;
        config.shader_preset_interface =
            snesrecomp_glsl_shader_preset_interface();
        created = snesrecomp_presenter_create(
            &config, &s_presenter, error, sizeof(error));
    }

    if (!created) {
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
    /* Both mean the preset is doing something; an arbitrary Shader= entry is
     * still OpenGL only. */
    const SnesRecompPresentBackend active_backend =
        snesrecomp_presenter_backend(s_presenter);
    const bool preset_active =
        (active_backend == SNESRECOMP_PRESENT_BACKEND_OPENGL &&
         ((s_visual_preset == LUFIA2_VISUAL_CLEAN_HD) ||
          (g_config.shader && g_config.shader[0]))) ||
        (active_backend == SNESRECOMP_PRESENT_BACKEND_VULKAN &&
         s_visual_preset == LUFIA2_VISUAL_CLEAN_HD);
    fprintf(stderr,
        "[video] presenter ready: %s, capabilities=0x%x, VSync %s%s\n",
        snesrecomp_presenter_backend_name(s_presenter),
        (unsigned)snesrecomp_presenter_capabilities(s_presenter),
        snesrecomp_vsync_state_name(
            snesrecomp_presenter_vsync_state(s_presenter)),
        preset_active
            ? (active_backend == SNESRECOMP_PRESENT_BACKEND_VULKAN
                ? ", native Clean-HD active" : ", GLSL preset active")
            : "");
    /* Take the largest offered scale at or below the configured one. */
    if (s_hd_mode7_scale) {
        const uint32_t offered =
            snesrecomp_presenter_mode7_scales(s_presenter);
        unsigned granted = 0;
        for (unsigned candidate = s_hd_mode7_scale; candidate >= 1u;
             candidate--) {
            if (offered & (1u << candidate)) {
                granted = candidate;
                break;
            }
        }
        if (granted != s_hd_mode7_scale) {
            fprintf(stderr,
                "[video] HD Mode 7 %s is not available on %s "
                "(offers 0x%X); using %s.\n",
                Lufia2HdMode7ScaleName(s_hd_mode7_scale),
                snesrecomp_presenter_backend_name(s_presenter),
                (unsigned)offered,
                Lufia2HdMode7ScaleName(granted));
            s_hd_mode7_scale = granted;
        }
    }
    fprintf(stderr,
        "[video] visual preset=%s HD Mode 7=%s filter=%s perspective=%s\n",
        s_visual_preset == LUFIA2_VISUAL_CLEAN_HD ? "CleanHD" : "Original",
        Lufia2HdMode7ScaleName(s_hd_mode7_scale),
        s_hd_mode7_filter ? "On" : "Off",
        s_hd_mode7_perspective ? "On" : "Off");
    if (s_visual_preset == LUFIA2_VISUAL_CLEAN_HD && !preset_active) {
        fprintf(stderr,
            "[video] CleanHD requires OpenGL or Vulkan; using original "
            "presentation.\n");
    }
    if (active_backend == SNESRECOMP_PRESENT_BACKEND_VULKAN &&
        g_config.shader && g_config.shader[0]) {
        fprintf(stderr,
            "[video] Vulkan does not load GLSLP presets; `Shader = %s` is "
            "inactive.\n",
            g_config.shader);
    }
    if (s_high_refresh) {
        const unsigned reported =
            snesrecomp_presenter_display_millihertz(s_presenter);
        const unsigned display_mhz =
            s_present_rate_millihertz ? s_present_rate_millihertz : reported;
        snesrecomp_present_timeline_init(&s_present_timeline, 60u, 1u);
        snesrecomp_present_timeline_set_display(
            &s_present_timeline, display_mhz);
        snesrecomp_present_timeline_reset(
            &s_present_timeline, snesrecomp_now_us());
        fprintf(stderr,
            "[video] presentation timeline: display=%u.%03u Hz%s, %s\n",
            display_mhz / 1000u, display_mhz % 1000u,
            s_present_rate_millihertz ? " (configured)" : " (reported)",
            snesrecomp_present_timeline_is_decoupled(&s_present_timeline)
                ? "decoupled from the 60 Hz guest"
                : "one present per guest frame");
    }
    if (s_hd_mode7_scale &&
        !(snesrecomp_presenter_capabilities(s_presenter) &
          SNESRECOMP_PRESENT_CAP_HD_MODE7)) {
        fprintf(stderr,
            "[video] HD Mode 7 needs a backend with the HD capability; "
            "using the authentic renderer.\n");
    }
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

static const SnesRecompOverlayFrame *BuildPresentationOverlay(
    bool include_rewind) {
    return Lufia2OverlayUiBuild(
        s_presenter, s_frame_width, SNES_HEIGHT, include_rewind);
}

static bool PresentMode7Reference(
    const SnesPpuFrameCapture *capture,
    const SnesRecompMode7Line *lines,
    const SnesRecompObjFrame *obj,
    const SnesRecompMode7MapSource *map_source,
    const SnesRecompOverlayFrame *overlay) {
    if (!snesrecomp_ppu_mode7_render_reference_argb8888_with_map(
            capture, lines, capture->visible_height, obj, map_source, 1u,
            s_present_pixels, capture->canvas_width * 4u))
        return false;

    const SnesRecompVideoFrame frame = {
        .pixels = s_present_pixels,
        .pixel_format = SNESRECOMP_PIXEL_FORMAT_ARGB8888,
        .width = capture->canvas_width,
        .height = capture->visible_height,
        .pitch = capture->canvas_width * 4,
        .overlay = overlay,
    };
    return snesrecomp_presenter_present(s_presenter, &frame);
}

/* Frames in which the ordinary presentation path never ran. A frozen picture
 * with the guest still running shows up nowhere else in the log. */
static unsigned s_present_stall;
static bool s_present_stall_reported;

static Lufia2VideoHandoffScene VideoLayoutHandoffScene(
    Lufia2VideoLayout layout) {
    switch (layout) {
    case LUFIA2_VIDEO_REGULAR_MAP:
        /* The only layout whose scroll registers move the margin pixels. */
        return LUFIA2_VIDEO_HANDOFF_SCENE_WIDE_MAP;
    case LUFIA2_VIDEO_WORLD_MAP:
    case LUFIA2_VIDEO_INTRO_MODE7:
    case LUFIA2_VIDEO_PATTERN_MENU:
        return LUFIA2_VIDEO_HANDOFF_SCENE_WIDE;
    case LUFIA2_VIDEO_MAP_LOADING:
        return LUFIA2_VIDEO_HANDOFF_SCENE_MAP_LOADING;
    default:
        return LUFIA2_VIDEO_HANDOFF_SCENE_OWN_FRAME;
    }
}

/* Must run after Lufia2DrawPpuFrame(): the per-line history belongs to the
   frame the renderer has just walked. */
static void ObserveHandoffRaster(void) {
    if (!Lufia2CapturePpuRasterEffects(
            0,
            s_handoff_line_scroll_x,
            s_handoff_line_scroll_y,
            s_handoff_line_mosaic,
            SNES_HEIGHT)) {
        return;
    }
    for (size_t y = 0; y < SNES_HEIGHT; y++) {
        const uint8_t mosaic = s_handoff_line_mosaic[y];
        s_handoff_line_mosaic[y] = (mosaic & 0x03u)
            ? (uint8_t)((mosaic >> 4) + 1u)
            : 1u;
    }
    Lufia2VideoHandoffObserveRaster(
        &s_video_handoff,
        s_handoff_line_scroll_x,
        s_handoff_line_scroll_y,
        s_handoff_line_mosaic,
        SNES_HEIGHT);
}

/* Guest cadence: build the picture to be shown. */
static void ComposeFrame(bool include_rewind) {
    RtlWidescreenPresent(
        s_present_pixels,
        (size_t)s_frame_width * 4,
        s_pixels,
        s_frame_width,
        SNES_HEIGHT);
    Lufia2IntroWidescreenPaint(
        g_ppu, s_present_pixels, (size_t)s_frame_width, SNES_HEIGHT,
        g_ws_extra > 0 ? (unsigned)g_ws_extra : 0u);
    Lufia2VideoHandoffApply(
        &s_video_handoff,
        s_present_pixels,
        s_map_handoff_pixels,
        (size_t)s_frame_width,
        SNES_HEIGHT,
        g_ws_extra > 0 ? (size_t)g_ws_extra : 0u,
        LUFIA2_MAP_STREAM_GUARD_PIXELS);
    if (!include_rewind) {
        snes_rewind_note_framebuffer(
            (const uint32_t *)s_present_pixels,
            s_frame_width,
            SNES_HEIGHT);
    }
}

/* Display cadence: submit the composed picture. */
static bool SubmitFrame(bool include_rewind) {
    bool frame_presented = false;
    const SnesRecompOverlayFrame *overlay =
        BuildPresentationOverlay(include_rewind);
    const bool intro_world_requested =
        s_current_video_layout == LUFIA2_VIDEO_INTRO_MODE7;
    /* Only scenes the policy calls Mode 7 may be taken over. Otherwise the HD
     * path claims any capture that satisfies the supported subset and
     * suppresses the ordinary path for the whole scene. */
    const bool mode7_layout =
        s_current_video_layout == LUFIA2_VIDEO_INTRO_MODE7 ||
        s_current_video_layout == LUFIA2_VIDEO_WORLD_MAP;
    const bool hd_requested =
        mode7_layout && s_hd_mode7_scale != 0u &&
        snesrecomp_presenter_mode7_scale_supported(
            s_presenter, s_hd_mode7_scale);

    if (!intro_world_requested) {
        s_last_intro_world_status = LUFIA2_INTRO_WORLD_INVALID_ARGUMENT;
        s_intro_mode7_world_active_reported = false;
    }

    if (intro_world_requested || hd_requested) {
        SnesPpuFrameCapture capture;
        SnesRecompObjFrame obj;
        SnesRecompMode7HdFrame hd_frame;
        SnesPpuUnsupported support = SNES_PPU_UNSUPPORTED_RASTER_STATE;
        const SnesRecompMode7Line *render_lines = s_hd_mode7_lines;
        const SnesRecompMode7MapSource *map_source = NULL;
        bool semantic_ready = false;
        bool wants_obj = false;
        bool used_hd = false;

        memset(&obj, 0, sizeof obj);
        memset(&hd_frame, 0, sizeof hd_frame);
        if (Lufia2CapturePpuFrame(
                &capture, (unsigned)s_frame_width, (unsigned)g_ws_extra))
            support = snesrecomp_ppu_mode7_supports(&capture);
        if (support == SNES_PPU_SUPPORTED &&
            snesrecomp_ppu_mode7_compile_lines(
                &capture, s_hd_mode7_lines, SNES_HEIGHT)) {
            semantic_ready = true;
            /* Spend the sub-pixel and sub-step remainders the guest keeps but
             * cannot publish, before the world lift reads the lines. */
            Lufia2Mode7SubstepRefine(&capture, s_hd_mode7_lines,
                                     SNES_HEIGHT);
            if (intro_world_requested) {
                const Lufia2IntroMode7WorldStatus world_status =
                    Lufia2IntroMode7WorldPrepare(
                        &capture, s_hd_mode7_lines, capture.visible_height,
                        s_intro_mode7_world_lines, SNES_HEIGHT,
                        &s_intro_mode7_world_source);
                if (world_status != s_last_intro_world_status) {
                    LUFIA2_LOG("[video] Intro full-world Mode 7: %s\n",
                               Lufia2IntroMode7WorldStatusName(world_status));
                    s_last_intro_world_status = world_status;
                }
                /* Without a verified world, keep rendering through the
                 * captured ring: it repeats at the far edges, but dropping the
                 * semantic renderer leaves no widescreen shadow at all. */
                if (world_status == LUFIA2_INTRO_WORLD_READY) {
                    render_lines = s_intro_mode7_world_lines;
                    map_source = &s_intro_mode7_world_source;
                }
            }
            for (unsigned i = 0; semantic_ready && i < capture.band_count;
                 i++) {
                if (!capture.bands[i].forced_blank &&
                    ((capture.bands[i].main_enable |
                      capture.bands[i].sub_enable) & 0x10u)) {
                    wants_obj = true;
                    break;
                }
            }
            obj.slivers = s_hd_mode7_obj_slivers;
            obj.capacity = SNESRECOMP_OBJ_MAX_SLIVERS;
            obj.line_priority = s_hd_mode7_obj_line_priority;
            obj.line_capacity = SNES_HEIGHT;
            if (semantic_ready && wants_obj &&
                !snesrecomp_ppu_obj_evaluate(&capture, &obj)) {
                semantic_ready = false;
                support = SNES_PPU_UNSUPPORTED_OBJ;
            }
        } else if (support == SNES_PPU_SUPPORTED) {
            support = SNES_PPU_UNSUPPORTED_RASTER_STATE;
        }

        if (semantic_ready && hd_requested) {
            hd_frame.capture = &capture;
            hd_frame.lines = render_lines;
            hd_frame.line_count = capture.visible_height;
            hd_frame.obj = wants_obj ? &obj : NULL;
            hd_frame.map_source = map_source;
            hd_frame.scale = s_hd_mode7_scale;
            hd_frame.filter_bg = s_hd_mode7_filter;
            hd_frame.interpolate_lines = s_hd_mode7_perspective;
            hd_frame.overlay = overlay;
            frame_presented = snesrecomp_presenter_present_mode7_hd(
                s_presenter, &hd_frame);
            used_hd = frame_presented;
            if (!frame_presented && !s_hd_mode7_present_error_reported) {
                fprintf(stderr,
                    "[video] HD Mode 7 presentation failed; using "
                    "native semantic fallback: %s\n",
                    snesrecomp_presenter_last_error(s_presenter));
                s_hd_mode7_present_error_reported = true;
            }
        }
        if (semantic_ready && intro_world_requested && !frame_presented) {
            frame_presented = PresentMode7Reference(
                &capture, render_lines, wants_obj ? &obj : NULL, map_source,
                overlay);
        }
        if (frame_presented && map_source &&
            !s_intro_mode7_world_active_reported) {
            LUFIA2_LOG(
                "[video] Intro full-world Mode 7: active (%s%s)\n",
                used_hd ? "HD " : "native ",
                used_hd ? Lufia2HdMode7ScaleName(s_hd_mode7_scale) : "1x");
            s_intro_mode7_world_active_reported = true;
        }
        if (hd_requested && support != s_hd_mode7_last_reject) {
            if (support != SNES_PPU_SUPPORTED) {
                fprintf(stderr,
                    "[video] HD Mode 7 fallback: %s\n",
                    snes_ppu_unsupported_text(support));
                if (support == SNES_PPU_UNSUPPORTED_COLOUR_MATH) {
                    for (unsigned i = 0; i < capture.band_count; i++) {
                        const SnesPpuRasterBand *band = &capture.bands[i];
                        if (!band->forced_blank && (band->cgwsel & 0x01u)) {
                            LUFIA2_LOG(
                                "[video] HD Mode 7 direct-colour band: "
                                "lines=%u..%u CGWSEL=$%02X "
                                "CGADSUB=$%02X TS=$%02X\n",
                                band->y_begin, band->y_end,
                                band->cgwsel, band->cgadsub,
                                band->sub_enable);
                            break;
                        }
                    }
                }
            }
            s_hd_mode7_last_reject = support;
        }
    }

    if (!frame_presented) {
        const SnesRecompVideoFrame frame = {
            .pixels = s_present_pixels,
            .pixel_format = SNESRECOMP_PIXEL_FORMAT_ARGB8888,
            .width = s_frame_width,
            .height = SNES_HEIGHT,
            .pitch = s_frame_width * 4,
            .overlay = overlay,
        };
        if (!snesrecomp_presenter_present(s_presenter, &frame)) {
            fprintf(stderr, "Present failed: %s\n",
                    snesrecomp_presenter_last_error(s_presenter));
            return false;
        }
    }
    snes_osd_present_done();
    s_frame_repeatable = !frame_presented;
    if (!frame_presented || mode7_layout) {
        /* A Mode 7 scene is meant to hold every frame. */
        s_present_stall = 0;
    } else if (++s_present_stall == 240u && !s_present_stall_reported) {
        s_present_stall_reported = true;
        LUFIA2_LOG(
            "[video] ordinary presentation idle for %u frames "
            "(layout=%s); the Mode 7 path is holding every frame\n",
            s_present_stall, Lufia2VideoLayoutName(s_current_video_layout));
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

static bool PresentFrame(bool include_rewind) {
    ComposeFrame(include_rewind);
    return SubmitFrame(include_rewind);
}

static void PrepareVideoFrame(void) {
    g_ws_active = g_config.widescreen;
    g_ws_extra = g_ws_active ? SNES_WIDE_EXTRA : 0;
    s_frame_width = g_ws_active ? SNES_WIDE_WIDTH : SNES_WIDTH;

    memset(s_pixels, 0,
        (size_t)s_frame_width * 4 * PPU_BUFFER_HEIGHT);

    Lufia2MapLoadFrame();

    const uint8_t *raster_rows = NULL;
    size_t raster_stride = 0;
    const bool raster_valid =
        Lufia2PpuRasterHistory(&raster_rows, &raster_stride);
    const Lufia2IntroMode7RasterDetail intro_raster =
        Lufia2InspectIntroMode7Raster(
            raster_rows, raster_stride, LUFIA2_PPU_VISIBLE_LINES,
            raster_valid);
    const Lufia2VideoObservation observation = {
        g_ram[0x05ac],
        Lufia2ResumePc(),
        intro_raster.classification,
    };

    if (Lufia2IntroMode7Candidate(&observation) &&
        intro_raster.classification == LUFIA2_INTRO_RASTER_REJECTED) {
        const uint64_t signature =
            ((uint64_t)intro_raster.reason << 56) |
            ((uint64_t)(intro_raster.line & 0xffu) << 48) |
            ((uint64_t)intro_raster.bgmode << 24) |
            ((uint64_t)intro_raster.setini << 16) |
            ((uint64_t)intro_raster.main_enable << 8) |
            intro_raster.sub_enable;
        if (signature != s_last_intro_raster_reject_signature) {
            LUFIA2_LOG(
                "[video] Intro Mode 7 raster rejected: %s at line %u "
                "(INIDISP=$%02X BGMODE=$%02X SETINI=$%02X "
                "TM=$%02X TS=$%02X)\n",
                Lufia2IntroMode7RejectReasonName(intro_raster.reason),
                intro_raster.line,
                intro_raster.inidisp,
                intro_raster.bgmode,
                intro_raster.setini,
                intro_raster.main_enable,
                intro_raster.sub_enable);
            s_last_intro_raster_reject_signature = signature;
        }
    } else {
        s_last_intro_raster_reject_signature = UINT64_MAX;
    }

    /* PpuResetLayerPolicies() clears clamp, mirror, repeat and the window
       expansion, but not the widen mask, so a mask set for one scene
       survives into the next: the menu restricts the margins to BG2, and
       the Mode 7 overworld that follows is layer 0, which the stale mask
       then excludes from the margins entirely. Publish it per frame like
       every other policy; each branch sets its own afterwards. */
    if (g_ppu)
        PpuSetWidescreenLayerMask(g_ppu, 0);

    Lufia2VideoLayout layout =
        Lufia2SelectVideoLayoutObserved(
            g_ppu, g_ws_active, &observation);
    if (layout == LUFIA2_VIDEO_BLANK) {
        /* Nothing reaches the screen during a fade, so hold the last
           decision and keep preparing. The map source follows the
           player through the transition and the shadow stays keyed to
           the live camera, so the fade-in shows the destination room
           instead of whatever survived the blank. */
        layout = s_held_video_layout == LUFIA2_VIDEO_INTRO_MODE7 &&
                         !Lufia2IntroMode7Candidate(&observation)
                     ? LUFIA2_VIDEO_CENTERED
                     : s_held_video_layout;
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

    case LUFIA2_VIDEO_INTRO_MODE7:
        Lufia2DeactivateMapWidescreen();
        PpuSetExtraSpace(g_ppu, (uint8_t)g_ws_extra);
        PpuSetWidescreenLayerMask(g_ppu, LUFIA2_MODE7_LAYER_MASK);
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
            /* No map data extends the backdrop; cycle its scanline. */
            PpuSetWidescreenLayerRepeat(
                g_ppu, LUFIA2_MAP_BACKDROP_LAYER_MASK);
            PpuSetWidescreenWindowExpansion(
                g_ppu,
                LUFIA2_MAP_WINDOW_LAYER_MASK,
                LUFIA2_OUTDOOR_WINDOW_MASK);
            finalize_map_widescreen = true;
            break;
        case LUFIA2_MAP_WIDESCREEN_LOADING:
            /* Keep the PPU on its safe 256-pixel path; presentation holds
               the previous wide edges until the new map is readable. */
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
        /* The live center stays on the safe path behind bridged margins. */
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
        if (Lufia2IntroWidescreenPrepare(
                g_ppu, observation.runtime_map,
                g_ws_extra > 0 ? (unsigned)g_ws_extra : 0u)) {
            break;
        }
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

    /* Before WsShadowFrame: the widescreen shadow reads through
       PpuRenderVram() too, so the credit has to be bound by now or the
       margins would be sampled from a different picture than the centre. */
    Lufia2SplashCreditPrepare(g_ppu);

    if (layout != LUFIA2_VIDEO_INTRO_MODE7)
        WsShadowFrame(g_ppu);
    if (finalize_map_widescreen)
        Lufia2FinalizeMapWidescreen(g_ppu, g_ws_extra);

    if (layout != LUFIA2_VIDEO_CENTERED)
        Lufia2IntroWidescreenRelease(g_ppu);

    s_current_video_layout = layout;
    Lufia2VideoHandoffObserve(
        &s_video_handoff,
        VideoLayoutHandoffScene(layout),
        g_ppu && !PPU_forcedBlank(g_ppu)
            ? (uint8_t)PPU_brightness(g_ppu)
            : 0u,
        g_ppu && (g_ppu->mosaic & 0x03u)
            ? (uint8_t)PPU_mosaicSize(g_ppu)
            : 1u,
        g_ppu ? g_ppu->hScroll[0] : 0u,
        g_ppu ? g_ppu->vScroll[0] : 0u);

    if (layout != s_last_video_layout) {
        LUFIA2_LOG(
            "[video] layout: %s (%dx%d, PPU mode=%u, main=$%02X, "
            "sub=$%02X, tiles=$%04X, BG maps=%u/%u, "
            "scroll=%u,%u/%u,%u/%u,%u, map=$%02X, world=%u,%u)\n",
            Lufia2VideoLayoutName(layout),
            s_frame_width, SNES_HEIGHT,
            g_ppu ? (unsigned)PPU_mode(g_ppu) : 0,
            g_ppu ? (unsigned)g_ppu->screenEnabled[0] : 0,
            g_ppu ? (unsigned)g_ppu->screenEnabled[1] : 0,
            g_ppu ? (unsigned)g_ppu->bgTileAdr : 0,
            g_ppu ? (unsigned)(g_ppu->bgXsc[0] & 3) : 0,
            g_ppu ? (unsigned)(g_ppu->bgXsc[1] & 3) : 0,
            g_ppu ? (unsigned)g_ppu->hScroll[0] : 0,
            g_ppu ? (unsigned)g_ppu->vScroll[0] : 0,
            g_ppu ? (unsigned)g_ppu->hScroll[1] : 0,
            g_ppu ? (unsigned)g_ppu->vScroll[1] : 0,
            g_ppu ? (unsigned)g_ppu->hScroll[2] : 0,
            g_ppu ? (unsigned)g_ppu->vScroll[2] : 0,
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
        RtlAudioSetFastForward(s_turbo);
        snes_osd_set_turbo(s_turbo ? 1 : 0);
        return;
    }

    if (!pressed)
        return;

    if (cmd >= kKeys_Load && cmd <= kKeys_Load_Last) {
        const int slot = cmd - kKeys_Load;
        char path[256];
        RtlEnsureSaveDir();
        RtlSaveSlotPath(slot, path, sizeof(path));
        if (!FileExists(path))
            Lufia2OverlayUiPushSlotEmpty(slot);
        else if (RtlLoadSnapshot(path))
            Lufia2OverlayUiPushSlotLoaded(slot);
        else
            Lufia2OverlayUiPush("State load failed", 2000);
        return;
    }

    if (cmd >= kKeys_Save && cmd <= kKeys_Save_Last) {
        const int slot = cmd - kKeys_Save;
        char path[256];
        RtlEnsureSaveDir();
        RtlSaveSlotPath(slot, path, sizeof(path));
        if (RtlSaveSnapshot(path))
            Lufia2OverlayUiPushSlotSaved(slot);
        else
            Lufia2OverlayUiPush("State save failed", 2000);
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
        Lufia2OverlayUiNoteVolume(s_volume_percent);
        break;

    case kKeys_VolumeDown:
        s_volume_percent -= 5;
        if (s_volume_percent < 0) s_volume_percent = 0;
        fprintf(stderr, "[audio] volume %d%%\n", s_volume_percent);
        Lufia2OverlayUiNoteVolume(s_volume_percent);
        break;

    case kKeys_DisplayPerf:
        snes_osd_toggle_fps();
        break;

    case kKeys_Rewind:
        s_rewind_requested = true;
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
        /* The saved frame was cut for the old canvas width. */
        Lufia2VideoHandoffReset(&s_video_handoff);
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
#ifdef LUFIA2_ENABLE_BRIDGE_AUDIT
            if (e.key.key == SDLK_F9 && (e.key.mod & SDL_KMOD_CTRL) &&
                    (e.key.mod & SDL_KMOD_SHIFT)) {
                if (down && !e.key.repeat) L2BAToggle();
                continue;
            }
#endif
#ifdef LUFIA2_ENABLE_GAMEPLAY_CAPTURE
            if (e.key.key == SDLK_F10 && (e.key.mod & SDL_KMOD_CTRL) &&
                    (e.key.mod & SDL_KMOD_SHIFT)) {
                if (down && !e.key.repeat) L2CaptureToggle();
                continue;
            }
#endif
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

static void InvalidateDerivedHostState(bool reset_rewind) {
    if (s_audio_stream)
        (void)SDL_ClearAudioStream(s_audio_stream);
    /* The picture being paced no longer exists. */
    s_frame_repeatable = false;
    if (s_high_refresh)
        snesrecomp_present_timeline_reset(
            &s_present_timeline, snesrecomp_now_us());
    Lufia2MapLoadStateChanged();
    Lufia2MapWidescreenStateChanged();
    Lufia2Mode7SubstepStateChanged();
    Lufia2IntroMode7WorldStateChanged();
    s_last_video_layout = LUFIA2_VIDEO_LAYOUT_COUNT;
    s_held_video_layout = LUFIA2_VIDEO_CENTERED;
    s_current_video_layout = LUFIA2_VIDEO_CENTERED;
    Lufia2VideoHandoffReset(&s_video_handoff);
    s_last_intro_raster_reject_signature = UINT64_MAX;
    Lufia2IntroWidescreenRelease(g_ppu);
    s_hd_mode7_last_reject = SNES_PPU_SUPPORTED;
    s_hd_mode7_present_error_reported = false;
    s_last_intro_world_status = LUFIA2_INTRO_WORLD_INVALID_ARGUMENT;
    s_intro_mode7_world_active_reported = false;
    s_present_stall = 0;
    if (reset_rewind) {
        snes_rewind_shutdown();
        snes_rewind_configure();
    }
}

static void ObserveStateGeneration(bool reset_rewind) {
    const uint64_t generation = RtlStateGeneration();
    if (generation == s_state_generation)
        return;
    InvalidateDerivedHostState(reset_rewind);
    s_state_generation = generation;
}

static bool PresentFrozenRewind(void) {
    const SnesRecompOverlayFrame *overlay =
        BuildPresentationOverlay(true);
    const SnesRecompVideoFrame frame = {
        .pixels = s_present_pixels,
        .pixel_format = SNESRECOMP_PIXEL_FORMAT_ARGB8888,
        .width = s_frame_width,
        .height = SNES_HEIGHT,
        .pitch = s_frame_width * 4,
        .overlay = overlay,
    };
    if (!snesrecomp_presenter_present(s_presenter, &frame)) {
        fprintf(stderr, "Rewind present failed: %s\n",
            snesrecomp_presenter_last_error(s_presenter));
        return false;
    }
    snes_osd_present_done();
    return true;
}

static bool RunRewindLoop(void) {
    bool running = true;
    uint32_t previous_pad = ReadGamepadInput();
    uint64_t next_repeat = 0;
    uint32_t repeat_direction = 0;

    while (running && snes_rewind_is_open()) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                running = false;
                snes_rewind_close();
            } else if (e.type == SDL_EVENT_KEY_DOWN) {
                if (e.key.key == SDLK_LEFT)
                    snes_rewind_step(-1);
                else if (e.key.key == SDLK_RIGHT)
                    snes_rewind_step(+1);
                else if (e.key.key == SDLK_RETURN ||
                         e.key.key == SDLK_SPACE)
                    snes_rewind_commit();
                else if (e.key.key == SDLK_ESCAPE)
                    snes_rewind_close();
            } else if (e.type == SDL_EVENT_GAMEPAD_ADDED && !s_gamepad) {
                TryOpenFirstGamepad();
            } else if (e.type == SDL_EVENT_GAMEPAD_REMOVED && s_gamepad &&
                       SDL_GetGamepadID(s_gamepad) == e.gdevice.which) {
                SDL_CloseGamepad(s_gamepad);
                s_gamepad = NULL;
                TryOpenFirstGamepad();
            }
        }

        const uint32_t pad = ReadGamepadInput();
        const uint32_t pressed = pad & ~previous_pad;
        if (pressed & PAD_A)
            snes_rewind_commit();
        if (pressed & PAD_B)
            snes_rewind_close();

        const uint32_t direction = pad & (PAD_LEFT | PAD_RIGHT);
        const uint64_t now = SDL_GetTicks();
        if (direction == PAD_LEFT || direction == PAD_RIGHT) {
            if (direction != repeat_direction) {
                snes_rewind_step(direction == PAD_LEFT ? -1 : +1);
                repeat_direction = direction;
                next_repeat = now + 250u;
            } else if (now >= next_repeat) {
                snes_rewind_step(direction == PAD_LEFT ? -1 : +1);
                next_repeat = now + 80u;
            }
        } else {
            repeat_direction = 0;
        }
        previous_pad = pad;

        if (snes_rewind_is_open() && !PresentFrozenRewind()) {
            running = false;
            snes_rewind_close();
        }
        SDL_Delay(8);
    }

    s_input_release_mask |= ReadKeyboardInput() | ReadGamepadInput();
    ObserveStateGeneration(false);
    return running;
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
    L2CaptureShutdown();
    Lufia2OverlayUiShutdown();
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
#if defined(LUFIA2_ENABLE_QUIESCENCE_INDEX) && \
    defined(LUFIA2_ENABLE_PATCH_TESTS)
    if (argc == 2 && strcmp(argv[1], "--quiescence-index-selftest") == 0)
        return L2QIndexSelfTest();
#endif
#if defined(LUFIA2_ENABLE_BRIDGE_AUDIT) && \
    defined(LUFIA2_ENABLE_PATCH_TESTS)
    if (argc == 2 && strcmp(argv[1], "--bridge-audit-selftest") == 0)
        return L2BASelfTest();
#endif
#if defined(LUFIA2_ENABLE_NATIVE_WAIT) && \
    defined(LUFIA2_ENABLE_PATCH_TESTS)
    if (argc == 2 && strcmp(argv[1], "--native-wait-selftest") == 0)
        return Lufia2NativeWaitSelfTest();
#endif
#if defined(LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD) && \
    defined(LUFIA2_ENABLE_PATCH_TESTS)
    if (argc == 2 && strcmp(argv[1], "--frame-wait-fastforward-selftest") == 0)
        return Lufia2FrameWaitFastForwardSelfTest();
#endif
#if defined(LUFIA2_ENABLE_ACTOR_EARLY_RETURN) && \
    defined(LUFIA2_ENABLE_PATCH_TESTS)
    if (argc == 2 && strcmp(argv[1], "--actor-early-return-selftest") == 0)
        return Lufia2ActorEarlyReturnSelfTest();
    if (argc == 2 && strcmp(argv[1], "--actor-d508-early-return-selftest") == 0)
        return Lufia2ActorD508EarlyReturnSelfTest();
#endif
#if (defined(LUFIA2_ENABLE_DMA_HOST_FASTFORWARD) || \
     defined(LUFIA2_ENABLE_DMA_DIRECT_SOURCE_READ)) && \
    defined(LUFIA2_ENABLE_PATCH_TESTS)
    if (argc == 2 && strcmp(argv[1], "--dma-host-fastforward-selftest") == 0)
        return Lufia2DmaHostFastForwardSelfTest();
#endif
#if defined(LUFIA2_ENABLE_PATCH_TESTS)
    if (argc == 2 && strcmp(argv[1], "--mode7-substep-selftest") == 0)
        return Lufia2Mode7SubstepSelfTest();
#endif
    char rom_path[1024];
    rom_path[0] = '\0';

    /* Register early so Tier-2 reporting has the game identity. */
    RtlRegisterGame(&kLufia2GameInfo);

#ifdef LUFIA2_ENABLE_GAMEPLAY_CAPTURE
    host_report_init("lufia2", "desktop-gameplay-capture1");
    fprintf(stderr, "[gameplay-capture] ready; Ctrl+Shift+F10 starts/stops; original execution unchanged\n");
#else
#ifdef LUFIA2_ENABLE_NATIVE_WAIT
    host_report_init("lufia2", "desktop-native-wait-step1");
#elif defined(LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD)
    host_report_init("lufia2", "desktop-frame-wait-native-dma-step1");
#elif defined(LUFIA2_ENABLE_ACTOR_EARLY_RETURN)
    host_report_init("lufia2", "desktop-actor-d508-step1");
#else
    host_report_init("lufia2", "desktop-v2-launcher");
#endif
#endif

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

    {
        Lufia2UiPanelAsset rom_panel = {0};
        if (!Lufia2UiAssetsExtract(rom_data, rom_size, &rom_panel)) {
            fprintf(stderr,
                    "[Lufia2 UI] ROM panel extraction failed; "
                    "the neutral fallback remains available.\n");
        } else if (!Lufia2OverlayUiInstallRomPanel(&rom_panel)) {
            fprintf(stderr,
                    "[Lufia2 UI] Could not install the ROM panel; "
                    "the neutral fallback remains available.\n");
        }
        Lufia2UiAssetsDestroy(&rom_panel);
    }
    Lufia2IntroMode7WorldInit(rom_data, rom_size);

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
        Lufia2OverlayUiShutdown();
        free(rom_data);
        return 1;
    }

    if (!LufiaDesktopCreateAudioMutex()) {
        fprintf(stderr, "SDL_CreateMutex failed: %s\n", SDL_GetError());
        SDL_Quit();
        Lufia2OverlayUiShutdown();
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

    snes_rewind_configure();
    {
        const char *osd_fps = getenv("SNESRECOMP_OSD_FPS");
        if (osd_fps && atoi(osd_fps) != 0) {
            snes_osd_set_fps_visible(1);
        }
    }
    s_state_generation = RtlStateGeneration();

    fprintf(stderr,
        "[desktop] entering main loop\n"
        "[desktop] use --launcher to force the launcher on the next start\n");

    bool running = true;
    uint32_t frame_counter = 0;
    uint64_t frame_deadline_ms = SDL_GetTicks();

    while (running && !g_fail) {
        running = HandleEvents();
        ObserveStateGeneration(true);

        if (RewindGesturePressed())
            s_rewind_requested = true;

        if (s_rewind_requested) {
            s_rewind_requested = false;
            if (snes_rewind_open()) {
                running = RunRewindLoop() && running;
            } else {
                Lufia2OverlayUiPush("Rewind history unavailable", 1600);
            }
        }

        if (!running || g_fail)
            break;

        if (s_paused) {
            SDL_Delay(10);
            continue;
        }

        /* Turbo and a disabled limiter pace themselves. */
        const bool timeline_paced =
            s_high_refresh && !s_turbo && !g_config.disable_frame_delay;
        SnesRecompPresentStep step;
        step.run_guest = true;
        step.present = true;
        step.alpha = 0.0f;
        step.sleep_us = 0;
        if (timeline_paced)
            snesrecomp_present_timeline_step(
                &s_present_timeline, snesrecomp_now_us(), &step);

        if (step.run_guest) {
            const uint32_t raw_input =
                ReadKeyboardInput() | ReadGamepadInput();
            s_input_release_mask &= raw_input;
            const uint32_t input = raw_input & ~s_input_release_mask;

            L2CaptureFrameBegin(input);
            RtlRunFrame(input);
            L2CaptureGuestEnd();
            snes_osd_note_frame();
            snes_rewind_note_frame();

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
            L2CaptureFrameEnd();
            ObserveHandoffRaster();
            Lufia2IntroWidescreenObserve(g_ppu);
            ComposeFrame(false);

            frame_counter++;
            UpdatePerfTitle();
        }

        /* Mode 7 frames cannot repeat; those stay at guest cadence. */
        if (step.present && (step.run_guest || s_frame_repeatable)) {
            if (!SubmitFrame(false)) {
                g_fail = true;
                break;
            }
            if (timeline_paced)
                snesrecomp_present_timeline_note_present(
                    &s_present_timeline, snesrecomp_now_us());
        }

#ifdef LUFIA2_ENABLE_RUNTIME_LOG
        if (frame_counter <= 10 || (frame_counter % 600) == 0)
            Lufia2PrintDiagnostics();
#endif

        if (timeline_paced) {
            if (step.sleep_us)
                SDL_DelayNS(step.sleep_us * 1000ull);
        } else if (!s_turbo && !g_config.disable_frame_delay) {
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
    if (s_high_refresh) {
        SnesRecompPresentStats stats;
        snesrecomp_present_timeline_stats(&s_present_timeline, &stats);
        fprintf(stderr,
            "[video] timeline: guest=%llu presents=%llu repeats=%llu "
            "late=%llu worst_late=%llums resyncs=%llu "
            "last_guest=%lluus last_present=%lluus\n",
            (unsigned long long)stats.guest_frames,
            (unsigned long long)stats.presents,
            (unsigned long long)stats.extra_presents,
            (unsigned long long)stats.late_guest_frames,
            (unsigned long long)(stats.worst_guest_late_us / 1000u),
            (unsigned long long)stats.resyncs,
            (unsigned long long)stats.last_guest_interval_us,
            (unsigned long long)stats.last_present_interval_us);
    }
#ifdef LUFIA2_ENABLE_NATIVE_WAIT
    Lufia2NativePatchesSummary();
#endif
#ifdef LUFIA2_ENABLE_RUNTIME_LOG
    Lufia2PrintDiagnostics();
#endif

    RtlWriteSram();
    snes_rewind_shutdown();

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
