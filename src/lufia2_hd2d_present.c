#include "lufia2_hd2d_present.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

#include "lufia2_dev_script.h"
#include "lufia2_hd2d_gl.h"

enum {
    /* Guest frames the camera takes to settle after a scene appears. */
    INTRO_FRAMES = 66,
    /* A gap this long between scene frames is a new scene. */
    INTRO_GAP_FRAMES = 8,
};

static const char kPrefix[] = "hd2d:";

typedef struct Hd2dPreset {
    void *inner;
} Hd2dPreset;

static const SnesRecompShaderPresetInterface *s_inner;
static const Lufia2Hd2dFrame *s_pending;
static char s_path[1100];
static char s_shot[600];
static uint32_t s_last_serial, s_intro_serial;
static Lufia2Hd2dKind s_last_kind;
static bool s_in_scene;

static void *PresetCreate(const char *path, char *error, size_t error_size) {
    Hd2dPreset *preset = (Hd2dPreset *)calloc(1, sizeof *preset);
    if (!preset)
        return NULL;
    if (path && !strncmp(path, kPrefix, sizeof kPrefix - 1))
        path += sizeof kPrefix - 1;
    if (path && *path) {
        preset->inner = s_inner && s_inner->create
            ? s_inner->create(path, error, error_size) : NULL;
        if (!preset->inner) {
            free(preset);
            return NULL;
        }
    }
    return preset;
}

static void PresetDestroy(void *opaque) {
    Hd2dPreset *preset = (Hd2dPreset *)opaque;
    if (!preset)
        return;
    Lufia2Hd2dGlShutdown();
    if (preset->inner && s_inner && s_inner->destroy)
        s_inner->destroy(preset->inner);
    free(preset);
}

/* `scene` tells which picture this present drew: the HD-2D one, or the
 * authentic frame through the preset, read back from the window. */
static void WriteShot(bool scene, int x, int y, int width, int height) {
    uint8_t *argb = scene ? Lufia2Hd2dGlReadback(&width, &height)
                          : Lufia2Hd2dGlReadDefault(x, y, width, height);
    if (argb && Lufia2DevWritePng(s_shot, argb, (unsigned)width,
                                  (unsigned)height, (size_t)width * 4, false))
        fprintf(stderr, "[hd2d] %s %dx%d -> %s\n",
                scene ? "scene" : "frame", width, height, s_shot);
    else
        fprintf(stderr, "[hd2d] could not write %s\n", s_shot);
    free(argb);
    s_shot[0] = '\0';
}

static bool RenderScene(const Lufia2Hd2dFrame *frame, int viewport_x,
                        int viewport_y, int viewport_width,
                        int viewport_height) {
    SDL_Window *window = SDL_GL_GetCurrentWindow();
    Lufia2Hd2dView view;
    memset(&view, 0, sizeof view);
    if (!window || !SDL_GetWindowSizeInPixels(
            window, &view.drawable_width, &view.drawable_height))
        return false;

    if (!s_in_scene || frame->kind != s_last_kind ||
        frame->serial - s_last_serial > INTRO_GAP_FRAMES)
        s_intro_serial = frame->serial;
    s_in_scene = true;
    s_last_kind = frame->kind;
    s_last_serial = frame->serial;

    view.content_x = viewport_x;
    view.content_y = viewport_y;
    view.content_width = viewport_width;
    view.content_height = viewport_height;
    /* Guest time: the scene advances with the game, pauses with it, and a
     * recorded run reproduces. */
    view.time = (double)frame->serial / 60.0;
    view.intro =
        (float)(frame->serial - s_intro_serial) / (float)INTRO_FRAMES;

    Lufia2Hd2dScene scene;
    Lufia2Hd2dBuildScene(frame, &scene);
    return Lufia2Hd2dGlRender(&scene, &view, 0);
}

static void PresetRender(void *opaque, uint32_t source_texture,
                         int source_width, int source_height,
                         int viewport_x, int viewport_y,
                         int viewport_width, int viewport_height) {
    Hd2dPreset *preset = (Hd2dPreset *)opaque;
    const Lufia2Hd2dFrame *frame = s_pending;
    s_pending = NULL;

    if (frame && RenderScene(frame, viewport_x, viewport_y, viewport_width,
                             viewport_height)) {
        if (s_shot[0])
            WriteShot(true, 0, 0, 0, 0);
        return;
    }
    if (!frame)
        s_in_scene = false;

    if (preset && preset->inner)
        s_inner->render(preset->inner, source_texture, source_width,
                        source_height, viewport_x, viewport_y,
                        viewport_width, viewport_height);
    else
        Lufia2Hd2dGlBlit(source_texture, viewport_x, viewport_y,
                         viewport_width, viewport_height);
    if (s_shot[0])
        WriteShot(false, viewport_x, viewport_y, viewport_width,
                  viewport_height);
}

const SnesRecompShaderPresetInterface *Lufia2Hd2dPresetInterface(
    const SnesRecompShaderPresetInterface *inner) {
    static const SnesRecompShaderPresetInterface interface = {
        PresetCreate,
        PresetDestroy,
        PresetRender,
    };
    s_inner = inner;
    return &interface;
}

const char *Lufia2Hd2dPresetPath(const char *inner_path) {
    snprintf(s_path, sizeof s_path, "%s%s", kPrefix,
             inner_path ? inner_path : "");
    return s_path;
}

void Lufia2Hd2dPresentArm(const Lufia2Hd2dFrame *frame) {
    s_pending = frame;
}

void Lufia2Hd2dPresentShot(const char *path) {
    snprintf(s_shot, sizeof s_shot, "%s", path ? path : "");
}
