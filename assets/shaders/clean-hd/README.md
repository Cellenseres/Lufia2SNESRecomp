# Clean-HD shader preset

`clean-hd.glslp` runs through the existing SNESRecomp GLSL preset runner:

1. a nearest-neighbour 2x prescale establishes a stable pixel grid;
2. a centre-biased fractional pass maps that grid to the viewport, with a
   0.65-output-pixel transition only at boundaries.

Integer scales snap fully to texel centres. The preset performs no gamma,
colour, scanline, mask, curvature or sharpening operation.

The shaders are project-owned; the sampling technique is informed by the
general sharp-bilinear approach, with no external source copied. Reference:
rsn8887, Sharp-Bilinear-Shaders,
<https://github.com/rsn8887/Sharp-Bilinear-Shaders>, GPL-2.0.
