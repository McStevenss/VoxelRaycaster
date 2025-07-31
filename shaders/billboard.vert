#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 uvOffset;
uniform vec2 uvScale;

out vec2 TexCoord;

void main()
{
    // Properly compute the UV based on tile offset and scale
    TexCoord = uvOffset + aTexCoord * uvScale;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}