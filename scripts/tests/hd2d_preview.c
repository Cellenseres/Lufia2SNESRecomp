/* Offline replay of a recorded HD-2D frame through the renderer:
 *
 *   Lufia2Hd2dPreview <frame.l2hd> <out.png> [width height [time [intro
 *                     [bench-frames]]]]
 *
 * The frame comes from a `layers` line in a LUFIA2_DEV_SCRIPT. The picture
 * is rendered off screen, so the window never has to be shown. A bench count
 * renders that many extra frames and reports the time each took. */
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL.h>

#include "gl_core_3_1.h"
#include "lufia2_dev_script.h"
#include "lufia2_hd2d.h"
#include "lufia2_hd2d_gl.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <frame.l2hd> <out.png> "
                        "[width height [time [intro [bench-frames]]]]\n",
                argv[0]);
        return 2;
    }
    const int width = argc > 4 ? atoi(argv[3]) : 1920;
    const int height = argc > 4 ? atoi(argv[4]) : 1080;
    const double time = argc > 5 ? atof(argv[5]) : 4.0;
    const float intro = argc > 6 ? (float)atof(argv[6]) : 1.0f;
    const int bench = argc > 7 ? atoi(argv[7]) : 0;

    Lufia2Hd2dFrame frame;
    if (!Lufia2Hd2dLoadFrame(argv[1], &frame)) {
        fprintf(stderr, "cannot read %s\n", argv[1]);
        return 1;
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_SetHint(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, "0");
    SDL_Window *window = SDL_CreateWindow(
        "hd2d preview", 64, 64, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    SDL_GLContext context = window ? SDL_GL_CreateContext(window) : NULL;
    if (!context || ogl_LoadFunctions() == ogl_LOAD_FAILED) {
        fprintf(stderr, "no OpenGL 3.3 context: %s\n", SDL_GetError());
        return 1;
    }

    /* LUFIA2_HD2D_AT=x,y looks at another part of a recorded world map:
     * the plane point, in pixels, to put at the picture's top left. */
    const char *at = getenv("LUFIA2_HD2D_AT");
    if (at)
        sscanf(at, "%f,%f", &frame.plane.origin_x, &frame.plane.origin_y);

    Lufia2Hd2dScene scene;
    Lufia2Hd2dBuildScene(&frame, &scene);

    /* LUFIA2_HD2D_RELIEF=<out.png> writes a field's heights as a picture:
     * brighter is higher, red is drawn over the sprites. */
    const char *relief_path = getenv("LUFIA2_HD2D_RELIEF");
    if (relief_path && scene.ground.relief) {
        const Lufia2Hd2dGround *ground = &scene.ground;
        const unsigned wide = ground->tiles_wide * 8u;
        const unsigned high = ground->tiles_high * 8u;
        uint8_t *argb = (uint8_t *)malloc((size_t)wide * high * 4u);
        for (size_t i = 0; argb && i < (size_t)wide * high; i++) {
            const uint8_t *relief = ground->relief + i * 4u;
            const unsigned height =
                ground->upper[i * 4u + 3u] ? relief[0] : relief[1];
            const unsigned grey = height > 84u ? 255u : height * 3u;
            argb[i * 4 + 0] = (uint8_t)(height ? grey : 40u);
            argb[i * 4 + 1] = (uint8_t)grey;
            argb[i * 4 + 2] = (uint8_t)(relief[2] ? 255u : grey);
            argb[i * 4 + 3] = 255;
        }
        if (argb)
            Lufia2DevWritePng(relief_path, argb, wide, high,
                              (size_t)wide * 4, false);
        free(argb);
    }

    /* The picture keeps the presenter's shape: 4:3 for the native frame,
     * proportionally wider for a widescreen capture. */
    Lufia2Hd2dView view = {0};
    view.drawable_width = width;
    view.drawable_height = height;
    view.content_height = height;
    view.content_width = (int)((long long)height * 4 * (int)scene.width /
                               (3 * LUFIA2_HD2D_NATIVE_WIDTH));
    if (view.content_width > width) {
        view.content_width = width;
        view.content_height = (int)((long long)width * 3 *
            LUFIA2_HD2D_NATIVE_WIDTH / (4 * (int)scene.width));
    }
    view.content_x = (width - view.content_width) / 2;
    view.content_y = (height - view.content_height) / 2;
    view.time = time;
    view.intro = intro;

    int status = 1;
    if (!Lufia2Hd2dGlRender(&scene, &view, -1)) {
        fprintf(stderr, "render failed: %s\n", Lufia2Hd2dGlError());
    } else {
        int out_width = 0, out_height = 0;
        uint8_t *argb = Lufia2Hd2dGlReadback(&out_width, &out_height);
        if (argb && Lufia2DevWritePng(argv[2], argb, (unsigned)out_width,
                                      (unsigned)out_height,
                                      (size_t)out_width * 4, false)) {
            printf("%s %dx%d actors=%u\n", argv[2], out_width, out_height,
                   scene.actor_count);
            status = 0;
        } else {
            fprintf(stderr, "cannot write %s\n", argv[2]);
        }
        free(argb);
    }

    if (bench > 0 && status == 0) {
        glFinish();
        Uint64 building = 0;
        const Uint64 start = SDL_GetTicksNS();
        for (int i = 0; i < bench; i++) {
            view.time = time + (double)i / 60.0;
            const Uint64 before = SDL_GetTicksNS();
            Lufia2Hd2dBuildScene(&frame, &scene);
            building += SDL_GetTicksNS() - before;
            Lufia2Hd2dGlRender(&scene, &view, -1);
        }
        glFinish();
        printf("%d frames at %dx%d: %.2f ms each, %.2f of it building the "
               "scene\n",
               bench, width, height,
               (double)(SDL_GetTicksNS() - start) / 1e6 / bench,
               (double)building / 1e6 / bench);
    }

    Lufia2Hd2dGlShutdown();
    Lufia2Hd2dFreeFrame(&frame);
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return status;
}
