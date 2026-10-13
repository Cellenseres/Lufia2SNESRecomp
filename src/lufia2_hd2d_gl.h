#ifndef LUFIA2_HD2D_GL_H
#define LUFIA2_HD2D_GL_H

#include <stdbool.h>
#include <stdint.h>

#include "lufia2_hd2d.h"

/* OpenGL 3.3 renderer for the HD-2D Battle scene. Every entry point needs
 * the presenter's context current; resources are created on first use. */

typedef struct Lufia2Hd2dView {
    /* Target size in pixels, and where the SNES picture sits inside it with
     * a bottom-left origin. The scene itself fills the whole target. */
    int drawable_width, drawable_height;
    int content_x, content_y, content_width, content_height;
    /* Seconds; drives the idle camera, the dust and the light shafts. */
    double time;
    /* 0 at the first Battle frame, 1 once the camera has settled. */
    float intro;
} Lufia2Hd2dView;

/* Draws the scene and copies it to `target_framebuffer`; a negative target
 * renders without presenting, for readback only. Very large targets are
 * rendered at a capped size and scaled up. False leaves the target untouched
 * so the caller can present the authentic frame instead. */
bool Lufia2Hd2dGlRender(const Lufia2Hd2dScene *scene,
                       const Lufia2Hd2dView *view, int target_framebuffer);

/* The last rendered image as top-down ARGB8888; the caller frees it. */
uint8_t *Lufia2Hd2dGlReadback(int *width, int *height);

/* The same for a rectangle of the default framebuffer, to capture a frame
 * that was not drawn as a scene. */
uint8_t *Lufia2Hd2dGlReadDefault(int x, int y, int width, int height);

/* Plain textured quad, for frames that are not Battle and have no preset. */
void Lufia2Hd2dGlBlit(uint32_t texture, int x, int y, int width, int height);

void Lufia2Hd2dGlShutdown(void);
const char *Lufia2Hd2dGlError(void);

#endif
