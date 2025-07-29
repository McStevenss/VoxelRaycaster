#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;
out float RayAngle;

uniform vec3 cameraDir;
uniform float fov; // in degrees

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 0.0, 1.0);

    float baseAngle = atan(cameraDir.z, cameraDir.x);
    float screenX = aPos.x; // -1 to 1

    float rayAngleOffset = atan(screenX * tan(fov * 0.5));
    RayAngle = baseAngle + rayAngleOffset;
}