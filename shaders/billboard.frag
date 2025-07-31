#version 330 core

in vec2 TexCoord;

uniform sampler2D spriteTexture;

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(spriteTexture, TexCoord);
    if (texColor.a < 0.1)
        discard;  // discard transparent pixels for alpha cutout

    FragColor = texColor;
}