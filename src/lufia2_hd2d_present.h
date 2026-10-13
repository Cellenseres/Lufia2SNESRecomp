#ifndef LUFIA2_HD2D_PRESENT_H
#define LUFIA2_HD2D_PRESENT_H

#include "lufia2_hd2d.h"
#include "snesrecomp_platform/presenter.h"

/* Seam into the shared presenter.
 *
 * The OpenGL presenter hands every frame to a shader-preset hook with its
 * context current. Wrapping that hook lets a staged frame be drawn as its
 * HD-2D scene while every other frame takes the preset it always took, and
 * leaves the platform library untouched. */

/* Preset interface and path to give the presenter in place of `inner` and
 * `inner_path`; an empty inner path means no preset. Both stay valid for the
 * lifetime of the process. */
const SnesRecompShaderPresetInterface *Lufia2Hd2dPresetInterface(
    const SnesRecompShaderPresetInterface *inner);
const char *Lufia2Hd2dPresetPath(const char *inner_path);

/* The frame the next present should draw as a scene, or NULL for the
 * authentic picture. Consumed by that present. */
void Lufia2Hd2dPresentArm(const Lufia2Hd2dFrame *frame);

/* Development: write the next scene the presenter draws to a PNG. */
void Lufia2Hd2dPresentShot(const char *path);

#endif
