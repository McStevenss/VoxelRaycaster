#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler3D voxelTexture;
uniform sampler2D voxelSurfaceTextureWall;
uniform sampler2D voxelSurfaceTextureFloor;

uniform vec3 cameraPos;
uniform mat4 invProjection;
uniform mat4 invView;
uniform int voxelWorldSize;

const int MAX_STEPS = 512;
// const vec3 lightPos = vec3(230,243, 25);
// const vec3 lightColor = vec3(0.5,0.5,0.5);
float voxelSize = 1.0;

vec3 generateRay(vec2 uv) {
    vec4 clip = vec4(uv * 2.0 - 1.0, 1.0, 1.0);
    vec4 view = invProjection * clip;
    view /= view.w;
    vec4 world = invView * view;
    return normalize(world.xyz - cameraPos);
}

bool intersectBox(vec3 rayOrigin, vec3 rayDir, out float tmin, out float tmax) {
    vec3 boxMin = vec3(0.0);
    vec3 boxMax = vec3(1.0);

    vec3 invDir = 1.0 / rayDir;
    vec3 t0s = (boxMin - rayOrigin) * invDir;
    vec3 t1s = (boxMax - rayOrigin) * invDir;

    vec3 tsmaller = min(t0s, t1s);
    vec3 tbigger = max(t0s, t1s);

    tmin = max(max(tsmaller.x, tsmaller.y), tsmaller.z);
    tmax = min(min(tbigger.x, tbigger.y), tbigger.z);

    return tmax >= max(tmin, 0.0);
}

void main() {
    float STEP_SIZE = 1.0 / voxelWorldSize;

    vec3 rayOrigin = cameraPos;
    vec3 rayDir = generateRay(TexCoords);

    float tmin, tmax;
    if (!intersectBox(cameraPos / voxelWorldSize, rayDir / voxelWorldSize, tmin, tmax)) {
        FragColor = vec4(0.5, 0.5, 0.5, 1.0); // Background color
        return;
    }

    tmin = max(tmin, 0.0);
    float tCurrent = tmin;  // <-- Declare and initialize here

    vec3 pos = rayOrigin + rayDir * tCurrent;

    ivec3 voxel = ivec3(floor(pos / voxelSize));
    vec3 rayStep = sign(rayDir);

    vec3 tDelta = abs(voxelSize / rayDir);
    vec3 voxelWorldPos = vec3(voxel) * voxelSize;

    vec3 tMax;
    tMax.x = ((rayDir.x > 0.0 ? voxelWorldPos.x + voxelSize : voxelWorldPos.x) - pos.x) / rayDir.x;
    tMax.y = ((rayDir.y > 0.0 ? voxelWorldPos.y + voxelSize : voxelWorldPos.y) - pos.y) / rayDir.y;
    tMax.z = ((rayDir.z > 0.0 ? voxelWorldPos.z + voxelSize : voxelWorldPos.z) - pos.z) / rayDir.z;

    int face = -1;
    int faceDir = 0;
    ivec3 lastVoxel = voxel;
    vec3 normal = vec3(0.0);

    for (int i = 0; i < MAX_STEPS; ++i) {
        vec3 texCoord = vec3(voxel) / float(voxelWorldSize);
        if (any(lessThan(texCoord, vec3(0.0))) || any(greaterThanEqual(texCoord, vec3(1.0))))
            break;

        float density = texture(voxelTexture, texCoord).r;
        if (density > 0.1) {
            // Hit detected

            voxelWorldPos = vec3(lastVoxel) * voxelSize;
            vec3 localPos = pos - voxelWorldPos;
            vec3 local = clamp(localPos / voxelSize, 0.0, 1.0);

            vec2 voxelUV = vec2(0.0);
            if (face == 0) { // X face
                voxelUV = faceDir < 0 ? vec2(local.z, local.y) : vec2(1.0 - local.z, local.y);
            } else if (face == 1) { // Y face
                voxelUV = faceDir < 0 ? vec2(local.x, local.z) : vec2(local.x, 1.0 - local.z);
            } else if (face == 2) { // Z face
                voxelUV = faceDir < 0 ? vec2(local.x, local.y) : vec2(1.0 - local.x, local.y);
            }

            if (face == 0)      normal = vec3(-faceDir, 0.0, 0.0);  // X face
            else if (face == 1) normal = vec3(0.0, -faceDir, 0.0);  // Y face
            else if (face == 2) normal = vec3(0.0, 0.0, -faceDir);  // Z face

            voxelUV = clamp(voxelUV, 0.0, 1.0);

            if (face == 1) {
                // FragColor = vec4(0.0, 1.0, 0.0, 1.0); // Green for floor
                FragColor = texture(voxelSurfaceTextureFloor, voxelUV);
            } else {
                // FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red for walls
                FragColor = texture(voxelSurfaceTextureWall, voxelUV);
            }
            return;
        }

        lastVoxel = voxel;


        // Determine which face is next hit and step accordingly
        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            face = 0;
            faceDir = int(rayStep.x);
            voxel.x += faceDir;
            tCurrent = tMax.x;
            tMax.x += tDelta.x;
        } else if (tMax.y < tMax.z) {
            face = 1;
            faceDir = int(rayStep.y);
            voxel.y += faceDir;
            tCurrent = tMax.y;
            tMax.y += tDelta.y;
        } else {
            face = 2;
            faceDir = int(rayStep.z);
            voxel.z += faceDir;
            tCurrent = tMax.z;
            tMax.z += tDelta.z;
        }

        if (tCurrent > tmax)
            break;

        pos = rayOrigin + rayDir * tCurrent;
    }

    // FragColor = vec4(0.5, 0.5, 0.5, 1.0); // Background
    FragColor = vec4(0.529, 0.808, 0.922, 1.0); // Background
    
}
