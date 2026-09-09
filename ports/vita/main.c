#ifdef LUFIA2_ENABLE_NATIVE_WAIT
#include "patches/native_patches.h"
#endif
#include "perf/lufia2_perf_audit.h"
/* Lufia II desktop host. */
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "desktop/sdl_compat.h"
#include "snesrecomp_platform/presenter.h"
#include "snesrecomp_platform/gpu_bg.h"
#include "snesrecomp_platform/msu_pack.h"
#include "snes/msu1.h"
#include "host_report.h"
#include "common_rtl.h"
#include "common_cpu_infra.h"
#include "snes/snes.h"
#include "snes/ppu.h"
#include "snes/ws_shadow.h"
#include "widescreen.h"
#include "desktop/display_aspect.h"


#include "snesrecomp_platform/host_boot.h"
#include "snesrecomp_platform/task.h"
#include "snesrecomp_platform/rom_verify.h"
extern const SnesRecompHostGame kLufia2HostGame;
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

uint64_t Lufia2HostNowUs(void) {
    return (uint64_t)((double)SDL_GetPerformanceCounter() * 1000000.0 /
                      (double)SDL_GetPerformanceFrequency());
}
static uint32_t ReadKeyboardInput(void) { return 0; }
static bool ResolveRomWithLauncher(int argc, char **argv, char *path, size_t size) {
    return snesrecomp_host_resolve_rom(&kLufia2HostGame, argc, argv, path, size);
}
static bool LoadVerifiedRom(const char *path, uint8_t **rom, uint32_t *size, bool quiet) {
    return snesrecomp_rom_load_verified(kLufia2HostGame.rom, path, rom, size, quiet, NULL);
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
    /* The SDL callback thread, not the thread that opens the stream. */
    static SDL_ThreadID pinned_audio_thread;
    const SDL_ThreadID callback_thread = SDL_GetCurrentThreadID();
    if (pinned_audio_thread != callback_thread) {
        snesrecomp_host_pin_audio_thread();
        pinned_audio_thread = callback_thread;
    }

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
    config.backend = SNESRECOMP_PRESENT_BACKEND_NATIVE;
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
    char error[256];
    if (!snesrecomp_presenter_create(&config, &s_presenter, error, sizeof error)) {
        fprintf(stderr, "Native presenter failed: %s\n", error);
        return false;
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

static bool HandleEvents(void) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) return false;
        if (e.type == SDL_EVENT_GAMEPAD_ADDED && !s_gamepad) TryOpenFirstGamepad();
        if (e.type == SDL_EVENT_GAMEPAD_REMOVED && s_gamepad &&
            SDL_GetGamepadID(s_gamepad) == e.gdevice.which) {
            SDL_CloseGamepad(s_gamepad); s_gamepad = NULL;
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
    snesrecomp_platform_task_shutdown();
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

    if (!snesrecomp_host_runtime_init(&kLufia2HostGame, 
#ifdef LUFIA2_ENABLE_PERF_AUDIT
        "clean-vita-gxm-perfaudit1"
#else
#ifdef LUFIA2_ENABLE_NATIVE_WAIT
        "clean-vita-gxm-native-wait-step1-ab"
#else
        "clean-vita-gxm-platform2"
#endif
#endif
)) return 1;
    snesrecomp_host_apply_performance_profile();
    L2PerfInit();
    fprintf(stderr, "[vita] main affinity core0=%s\n",
            snesrecomp_host_pin_main_thread() ? "set" : "unavailable");

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

    snesrecomp_host_rom_accepted(&kLufia2HostGame, rom_path);

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
        "Lufia II CleanMain + Vita GXM\n"
        "--------------------------------\n"
        "ROM:      %s\n"
        "Bytes:    %u\n"
        "Host:     Vita native\n",
        rom_path, (unsigned)rom_size);

    /* The native presenter owns GXM; SDL must not initialize another video owner. */
    if (!snesrecomp_sdl_init(SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
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

    /* Platform input and presentation settings. */
    snesrecomp_host_init_input(&kLufia2HostGame);
    s_player1_source = PLAYER_INPUT_GAMEPAD;
    s_vsync_enabled = snesrecomp_host_vsync_enabled();
    s_volume_percent = snesrecomp_host_volume_percent();

    char room_path[512];
    if (snesrecomp_host_resolve_data_file(&kLufia2HostGame,
            "lufia2_rooms.l2rooms", "data/widescreen/lufia2_rooms.l2rooms",
            room_path, sizeof room_path))
        snesrecomp_host_set_environment("LUFIA2_ROOM_DATA", room_path);
    Lufia2MapLoadInstallHooks();

    Snes *snes = SnesInit(rom_data, (int)rom_size);
    if (!snes) {
        fprintf(stderr, "SnesInit failed.\n");
        free(rom_data);
        ShutdownDesktop();
        return 1;
    }

    if (snesrecomp_host_saves_root()) RtlSetSaveRoot(snesrecomp_host_saves_root());
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

    Lufia2SetSemanticProductionEnabled(snesrecomp_host_data_file_exists(
        &kLufia2HostGame, "gpu_semantic_production.txt"));
    {
        const bool mode7_production = snesrecomp_host_data_file_exists(
            &kLufia2HostGame, "gpu_mode7_production.txt");
        Lufia2SetMode7ProductionEnabled(mode7_production);
        fprintf(stderr,
                "[vita] exact Mode 7 GXM production=%s backend=%s\n",
                mode7_production ? "ON" : "off (CPU fallback)",
                snesrecomp_gpu_mode7_available() ? "ready" : "unavailable");
    }
    if (snesrecomp_host_data_file_exists(&kLufia2HostGame,
                                         "no_poll_fastforward.txt"))
        (void)snesrecomp_host_set_environment("SNESRECOMP_POLL_FASTFWD", "0");
    fprintf(stderr, "[vita] Lufia APU wait fast-forward=%s\n",
            snesrecomp_host_data_file_exists(&kLufia2HostGame,
                                              "no_poll_fastforward.txt")
                ? "OFF" : "on");
    TryOpenFirstGamepad();

    /* Warm the task timer's lazy frequency cache before any worker can read
     * it. Tasks finish inside FinishPixelOffload, before presentation/guest. */
    (void)snesrecomp_platform_now_us();
    snesrecomp_platform_task_set_thread_hook(snesrecomp_host_pin_helper_thread);
    if (snesrecomp_host_parallel_workers_enabled() &&
        !snesrecomp_host_data_file_exists(&kLufia2HostGame, "no_ppu_parallel.txt"))
        (void)snesrecomp_platform_task_enable(true);
    fprintf(stderr, "[vita] platform workers=%u; one pixel helper; synchronous join\n",
            snesrecomp_platform_task_worker_count());

    fprintf(stderr, "[vita] entering CleanMain frame loop\n");

    bool running = true;
    uint32_t frame_counter = 0;
    uint64_t frame_deadline_ms = SDL_GetTicks();
    uint64_t perf_start = Lufia2HostNowUs();
    uint64_t perf_guest = 0, perf_ppu = 0, perf_gpu = 0, perf_present = 0;
    uint64_t perf_nmi = 0, perf_scheduler = 0;
    uint64_t perf_fallback = 0, perf_insns = 0;
    extern unsigned long long lufia2_poll_skipped_count(void);
    uint64_t perf_poll_start = lufia2_poll_skipped_count();
    uint32_t perf_parallel_count = 0;
    /* Existing unconditional core counter; no opcode tracing is enabled. */
    extern uint64_t interp816_insns_total(void);
    uint32_t perf_count = 0, perf_gpu_count = 0, perf_offload_count = 0;
    fprintf(stderr, "[vita-perf] GXM pixel offload, original CPU sprite evaluation; configured renderer=%s; AOT=-Os\n",
            g_config.new_renderer ? "scanline" : "legacy");

    while (running && !g_fail) {
        L2PerfFrameBegin(frame_counter + 1);
        { L2_SCOPE(audit_events, L2_EVENTS);
          running = HandleEvents(); }


        if (s_paused) {
            L2PerfFrameCancel();
            SDL_Delay(10);
            continue;
        }

        uint32_t input;
        { L2_SCOPE(audit_input, L2_INPUT);
          input = ReadKeyboardInput() | ReadGamepadInput(); }

        const uint64_t guest_start = Lufia2HostNowUs();
        const uint64_t insns_start = interp816_insns_total();
        { L2_SCOPE(audit_guest, L2_GUEST);
          RtlRunFrame(input); }
        perf_insns += interp816_insns_total() - insns_start;
        const uint64_t guest_end = Lufia2HostNowUs();
        uint64_t nmi_us, scheduler_us;
        Lufia2GuestPhaseTimes(&nmi_us, &scheduler_us);
        perf_nmi += nmi_us;
        perf_scheduler += scheduler_us;

        L2_SCOPE(audit_video, L2_VIDEO);
        PrepareVideoFrame();

        int ppu_flags = 0;
        if (g_config.new_renderer || g_ws_active)
            ppu_flags |= kPpuRenderFlags_NewRenderer;
        if (g_config.no_sprite_limits)
            ppu_flags |= kPpuRenderFlags_NoSpriteLimits;

        Lufia2BeginMapRenderOverlay(g_ppu);
        PpuBeginDrawing(
            g_ppu, s_pixels, (size_t)s_frame_width * 4, ppu_flags);
        L2PerfLeave(&audit_video);
        L2_SCOPE(audit_ppu, L2_PPU);
        const uint64_t ppu_start = Lufia2HostNowUs();
        const bool pixels_deferred = Lufia2PreparePixelOffload(
            (unsigned)s_frame_width, (unsigned)g_ws_extra);
        Lufia2DrawPpuFrame();
        const uint64_t ppu_end = Lufia2HostNowUs();
        L2PerfLeave(&audit_ppu);
        L2_SCOPE(audit_gxm, L2_GXM);
        bool gpu_drawn = pixels_deferred &&
            Lufia2TryGxmFrame((unsigned)s_frame_width, (unsigned)g_ws_extra);
        const uint64_t gpu_end = Lufia2HostNowUs();
        L2PerfLeave(&audit_gxm);
        L2_SCOPE(audit_fallback, L2_FALLBACK);
        Lufia2FinishPixelOffload(gpu_drawn);
        const uint64_t fallback_end = Lufia2HostNowUs();
        perf_parallel_count += Lufia2PixelFallbackWasParallel() ? 1u : 0u;
        L2PerfLeave(&audit_fallback);
        L2_SCOPE(audit_present, L2_PRESENT);
        Lufia2EndMapRenderOverlay(g_ppu);
        if (!gpu_drawn && !PresentFrame()) {
            g_fail = true;
            break;
        }
        L2PerfLeave(&audit_present);
        const uint64_t present_end = Lufia2HostNowUs();
        perf_guest += guest_end - guest_start;
        perf_ppu += ppu_end - ppu_start;
        perf_gpu += gpu_end - ppu_end;
        perf_fallback += fallback_end - gpu_end;
        perf_present += present_end - fallback_end;
        perf_count++;
        perf_gpu_count += gpu_drawn ? 1u : 0u;
        perf_offload_count += pixels_deferred && gpu_drawn ? 1u : 0u;

        frame_counter++;
        L2_SCOPE(audit_misc, L2_MISC);
        UpdatePerfTitle();

        if (frame_counter <= 10 || (frame_counter % 600) == 0)
            Lufia2PrintDiagnostics();

        L2PerfLeave(&audit_misc);
        L2_SCOPE(audit_pacing, L2_PACING);
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
        L2PerfLeave(&audit_pacing);
        L2PerfFrameEnd();
        const uint64_t perf_now = Lufia2HostNowUs();
        if (perf_now - perf_start >= 2000000u) {
            const double divisor = 1000.0 * (double)perf_count;
            fprintf(stderr,
                "[vita-perf] f=%u fps=%.2f avg_ms guest=%.2f ppu=%.2f "
                "gxm=%.2f present=%.2f other_wait=%.2f gpu=%u/%u offload=%u last=%s "
                "guest_split_ms nmi=%.2f scheduler=%.2f rtl_tail=%.2f "
                "fallback_ms=%.2f cpu_parallel=%u lle_insns_per_frame=%.0f "
                "pollskip_per_frame=%.0f\n",
                frame_counter, (double)perf_count * 1000000.0 / (double)(perf_now - perf_start),
                perf_guest / divisor, perf_ppu / divisor, perf_gpu / divisor,
                perf_present / divisor,
                ((double)(perf_now - perf_start) - (double)perf_guest - (double)perf_ppu -
                 (double)perf_gpu - (double)perf_present - (double)perf_fallback) / divisor,
                perf_gpu_count, perf_count, perf_offload_count,
                pixels_deferred ? Lufia2GxmFrameStatus() : Lufia2PixelOffloadStatus(),
                perf_nmi / divisor, perf_scheduler / divisor,
                ((double)perf_guest - (double)perf_nmi - (double)perf_scheduler) / divisor,
                perf_fallback / divisor, perf_parallel_count,
                (double)perf_insns / (double)perf_count,
                (double)(lufia2_poll_skipped_count() - perf_poll_start) /
                    (double)perf_count);
#ifdef LUFIA2_ENABLE_NATIVE_WAIT
            /* Same cadence as the line above, so no window straddles
             * an arm switch. The switch is between bridge calls. */
            Lufia2NativePatchesWindowEnd(perf_count,
                                         perf_now - perf_start,
                                         perf_scheduler);
#endif
            perf_start = Lufia2HostNowUs();
            perf_guest = perf_ppu = perf_gpu = perf_present = 0;
            perf_nmi = perf_scheduler = 0;
            perf_fallback = perf_insns = 0;
            perf_parallel_count = 0;
            perf_poll_start = lufia2_poll_skipped_count();
            perf_count = perf_gpu_count = perf_offload_count = 0;
        }
    }

    fprintf(stderr,
        "[desktop] leaving main loop: frames=%u fail=%d\n",
        frame_counter, g_fail ? 1 : 0);
#ifdef LUFIA2_ENABLE_NATIVE_WAIT
    Lufia2NativePatchesSummary();
#endif
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
