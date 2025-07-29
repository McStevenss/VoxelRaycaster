#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D heightmap;
uniform sampler2D colormap;

uniform vec3 cameraPos;
uniform vec3 cameraDir;
uniform float fov;
uniform float mapSize;
uniform float screenHeight;
uniform float scale;

void main()
{
    // Calculate ray angle based on FOV and screen X coordinate
    // vec3 cameraDirection = normalize(cameraDir);


    vec3 cameraDirection = normalize(cameraDir);
    float angle = atan(cameraDirection.z, cameraDirection.x);

    float new_fov = radians(95.0);
    float screenX = (TexCoords.x - 0.5) * 2.0;
    float rayAngleOffset = atan(screenX * tan(new_fov * 0.25));
    float rayAngle = angle + rayAngleOffset;
    
    float sinA = sin(rayAngle);
    float cosA = cos(rayAngle);
    float maxHeightOnScreen = screenHeight;

    float draw_distance = 1750.0;

    // Ray marching
    for (float t = 1; t < draw_distance; t += 0.25) 
    {
        float rayX = cameraPos.x + cosA * t;
        float rayZ = cameraPos.z + sinA * t;

        // Wrap coordinates for sampling textures
        float mapX = mod(rayX, mapSize) / mapSize;
        if (mapX < 0.0) mapX += 1.0;

        float mapZ = mod(rayZ, mapSize) / mapSize;
        if (mapZ < 0.0) mapZ += 1.0;

        float height = texture(heightmap, vec2(mapX, mapZ)).r * 255.0; // Assuming height stored in red channel scaled 0..255
        
        float pitchOffset = cameraDirection.y * t;
        float projectedHeight = (cameraPos.y - height + pitchOffset) / t * scale + screenHeight / 2.0;
        
        // float projectedHeight = (cameraPos.y - height) / t * scale + screenHeight / 2.0;

        if (projectedHeight < maxHeightOnScreen) {
            // Get color
            vec3 ground_color = texture(colormap, vec2(mapX, mapZ)).rgb;

            // Normalize y coordinate in screen space (0..1)
            float yNormStart = projectedHeight / screenHeight;

            if (TexCoords.y >= yNormStart && TexCoords.y <= maxHeightOnScreen / screenHeight) {
                FragColor = vec4(ground_color, 1.0);
                // FragColor = vec4(vec3(t/draw_distance), 1.0);
                return;
            }

            maxHeightOnScreen = projectedHeight;

            if (maxHeightOnScreen <= 0.0)
                break;
        }
    }

    // Sky color fallback
    FragColor = vec4(0.529, 0.808, 0.922, 1.0); // sky blue
    // FragColor = vec4(1.0, 0.0, 0.0, 1.0); // sky blue
}
