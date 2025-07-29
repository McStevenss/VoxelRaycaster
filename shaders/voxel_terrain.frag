#version 330 core

out vec4 FragColor;
in vec2 TexCoords;     // Screen UV coordinates
in float RayAngle;     // Precomputed in vertex shader

uniform sampler2D heightmap;
uniform sampler2D colormap;

uniform vec3 cameraPos;
uniform vec3 cameraDir; // Used only for vertical offset (Y)
uniform float mapSize;
uniform float screenHeight;
uniform float scale;
// uniform float draw_distance;

// Optional noise toggle
#define ENABLE_DITHER 1

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}


void main()
{
    float sinA = sin(RayAngle);
    float cosA = cos(RayAngle);
    float maxHeightOnScreen = screenHeight;
    float draw_distance = 1750;

    // float jitter = ENABLE_DITHER != 0 ? rand(TexCoords) * 0.5 : 0.0;
    float t = 1.0;

    float fogStart = draw_distance-100.0;
    float fogLength = draw_distance - fogStart;
    vec3 skyColor = vec3(0.529, 0.808, 0.922);

    while (t < draw_distance)
    {
        if(t >= draw_distance/10)
        {
            float jitter = ENABLE_DITHER != 0 ? rand(TexCoords) * 0.5 : 0.0;

            t = t+jitter;
        }
        float rayX = cameraPos.x + cosA * t;
        float rayZ = cameraPos.z + sinA * t;

        float mapX = mod(rayX, mapSize) / mapSize;
        float mapZ = mod(rayZ, mapSize) / mapSize;


        float height = texture(heightmap, vec2(mapX, mapZ)).r * 255.0;
        float pitchOffset = cameraDir.y * t;
        float projectedHeight = (cameraPos.y - height + pitchOffset) / t * scale + screenHeight * 0.5;

        if (projectedHeight < maxHeightOnScreen)
        {
            vec3 groundColor = texture(colormap, vec2(mapX, mapZ)).rgb;
            float yNormStart = projectedHeight / screenHeight;
            float yNormEnd = maxHeightOnScreen / screenHeight;

            if (TexCoords.y >= yNormStart && TexCoords.y <= yNormEnd)
            {
                FragColor = vec4(groundColor, 1.0);
                float fogFactor = clamp((draw_distance - t) / fogLength, 0.0, 1.0);
                FragColor.rgb = mix(skyColor, FragColor.rgb, fogFactor);
              
                return;
            }
            maxHeightOnScreen = projectedHeight;
            if (maxHeightOnScreen <= 0.0)
                break;
        }
        float stepSize = mix(0.05, 10.0, t / draw_distance);
        t += stepSize;
        // t += 0.25;
    }
    
    float fogFactor = clamp((draw_distance - draw_distance) / fogLength, 0.0, 1.0); // = 0
    FragColor = vec4(skyColor, 1.0);

    // FragColor = vec4(0.529, 0.808, 0.922, 1.0); // sky color fallback
}
