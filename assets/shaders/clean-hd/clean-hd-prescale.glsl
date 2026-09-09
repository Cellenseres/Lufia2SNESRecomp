#version 330

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

void main(void) {
    color = texture(Texture, tex_coord);
}
#endif

