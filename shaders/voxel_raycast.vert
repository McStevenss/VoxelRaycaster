#version 330 core

const vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

out vec2 TexCoords;

void main() {
    vec2 pos = positions[gl_VertexID];
    TexCoords = pos * 0.5 + 0.5;
    gl_Position = vec4(pos, 0.0, 1.0);
}