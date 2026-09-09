#version 330

/* Project-owned sharp-bilinear implementation. Most output pixels sample the
 * exact centre of one prescaled texel. A narrow, symmetric transition at a
 * texel boundary absorbs fractional-scale wobble without making the whole
 * image look bilinear-filtered. No colour transform or sharpening is used. */
#if defined(VERTEX)
in vec2 VertexCoord;
in vec2 TexCoord;
out vec2 tex_coord;
uniform mat4 MVPMatrix;

void main(void) {
    gl_Position = MVPMatrix * vec4(VertexCoord, 0.0, 1.0);
    tex_coord = TexCoord;
}
#elif defined(FRAGMENT)
in vec2 tex_coord;
out vec4 color;
uniform sampler2D Texture;
uniform vec2 InputSize;
uniform vec2 TextureSize;
uniform vec2 OutputSize;

void main(void) {
    vec2 texel = tex_coord * TextureSize;
    vec2 output_scale = max(OutputSize / InputSize, vec2(1.0));

    /* Keep the blend to 0.65 output pixels across each boundary. The former
     * mapping started its transition at the texel's left edge and therefore
     * softened/asymmetrically shifted far more of the image. */
    const vec2 blend_pixels = vec2(0.65);
    vec2 half_blend = 0.5 * blend_pixels / output_scale;
    vec2 centre_distance = fract(texel) - 0.5;
    vec2 hold_region = max(vec2(0.0), vec2(0.5) - half_blend);
    vec2 transition =
        (centre_distance -
         clamp(centre_distance, -hold_region, hold_region)) /
        (2.0 * half_blend) + 0.5;

    /* At exact integer scale there is no uneven-pixel problem to hide. Snap
     * completely to texel centres for a byte-stable nearest presentation. */
    vec2 integer_distance =
        abs(output_scale - floor(output_scale + vec2(0.5)));
    vec2 integer_axis =
        vec2(1.0) - step(vec2(0.0001), integer_distance);
    transition = mix(transition, vec2(0.5), integer_axis);

    vec2 sample_uv =
        (floor(texel) + clamp(transition, 0.0, 1.0)) / TextureSize;
    color = texture(Texture, sample_uv);
}
#endif
