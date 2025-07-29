#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler3D voxelTexture;
uniform vec3 cameraPos;
uniform mat4 invProjection;
uniform mat4 invView;
uniform int voxelWorldSize;

const int MAX_STEPS = 128;

float voxelSize = 1;

vec3 generateRay(vec2 uv) {
    // Convert UV to NDC [-1,1]
    vec4 clip = vec4(uv * 2.0 - 1.0, 1.0, 1.0); // z = 1.0 means far plane

    // Transform to view space
    vec4 view = invProjection * clip;
    view /= view.w;

    // Transform to world space
    vec4 world = invView * view;

    // Ray direction = point on far plane - camera position
    return normalize(world.xyz - cameraPos);
}


bool intersectBox(vec3 rayOrigin, vec3 rayDir, out float tmin, out float tmax) {
    vec3 boxMin = vec3(0.0);
    vec3 boxMax = vec3(1.0); // normalized voxel texture coords

    vec3 invDir = 1.0 / rayDir;
    vec3 t0s = (boxMin - rayOrigin) * invDir;
    vec3 t1s = (boxMax - rayOrigin) * invDir;

    vec3 tsmaller = min(t0s, t1s);
    vec3 tbigger = max(t0s, t1s);

    tmin = max(max(tsmaller.x, tsmaller.y), tsmaller.z);
    tmax = min(min(tbigger.x, tbigger.y), tbigger.z);

    return tmax >= max(tmin, 0.0);
}

// World position to texture coordinates [0,1]
vec3 worldToTex(vec3 p) {
    return p / 128.0; // map world [0,128] → texture [0,1]
}

void main() {
    float STEP_SIZE = 1.0 / voxelWorldSize;
    vec3 rayOrigin = cameraPos;
    vec3 rayDir = generateRay(TexCoords); // world-space ray direction

    // Normalize voxel space to [0, VOXEL_RES]
    float tmin, tmax;
    if (!intersectBox(cameraPos / voxelWorldSize, rayDir / voxelWorldSize, tmin, tmax)) {
        FragColor = vec4(0.5, 0.5, 0.5, 1.0); // No intersection
        return;
    }

    tmin = max(tmin, 0.0);
    vec3 pos = rayOrigin + rayDir * tmin;

    // Start in voxel grid
    ivec3 voxel = ivec3(floor(pos / voxelSize));
    vec3 voxelPos = vec3(voxel);

    vec3 rayStep = sign(rayDir);
    // vec3 tDelta = abs(1.0 / rayDir); // distance to cross one voxel along each axis
    vec3 tDelta = abs(voxelSize / rayDir);
    vec3 tMax;

    vec3 voxelWorldPos = vec3(voxel) * voxelSize;

    
    tMax.x = ((rayDir.x > 0.0 ? voxelWorldPos.x + voxelSize : voxelWorldPos.x) - pos.x) / rayDir.x;
    tMax.y = ((rayDir.y > 0.0 ? voxelWorldPos.y + voxelSize : voxelWorldPos.y) - pos.y) / rayDir.y;
    tMax.z = ((rayDir.z > 0.0 ? voxelWorldPos.z + voxelSize : voxelWorldPos.z) - pos.z) / rayDir.z;


    for (int i = 0; i < voxelWorldSize; ++i) {
        // Convert voxel index to [0,1] texture coordinate
        vec3 texCoord = vec3(voxel) / voxelWorldSize;

        // Bounds check
        if (any(lessThan(texCoord, vec3(0.0))) || any(greaterThanEqual(texCoord, vec3(1.0))))
            break;

        float density = texture(voxelTexture, texCoord).r;

        if (density > 0.1) {
            // Color based on voxel coordinates (debug coloring)
            vec3 color = vec3(
                mod(float(voxel.x), 3.0) / 3.0,
                mod(float(voxel.y), 3.0) / 3.0,
                mod(float(voxel.z), 3.0) / 3.0
            );
            FragColor = vec4(color, 1.0);
            return;
        }

        // Move to next voxel along ray
        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            voxel.x += int(rayStep.x);
            tMax.x += tDelta.x;
        } else if (tMax.y < tMax.z) {
            voxel.y += int(rayStep.y);
            tMax.y += tDelta.y;
        } else {
            voxel.z += int(rayStep.z);
            tMax.z += tDelta.z;
        }

        // Optional exit condition based on tmax distance
        float traveled = min(min(tMax.x, tMax.y), tMax.z);
        if (traveled > tmax)
            break;
    }

    FragColor = vec4(0.5, 0.5, 0.5, 1.0); // Background color if nothing hit
}
