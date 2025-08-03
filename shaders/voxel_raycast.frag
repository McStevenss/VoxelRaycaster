#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler3D voxelTexture;
uniform sampler2D voxelSpriteSheet;

uniform vec3 cameraPos;
uniform float nearPlane;
uniform float farPlane;
uniform mat4 invProjection;
uniform mat4 projectionMatrix;
uniform mat4 invView;
uniform mat4 viewMatrix;
uniform int voxelWorldSize;
uniform float VoxelScaleX;
uniform float VoxelScaleY;
uniform int tilesPerCol;

const int MAX_STEPS          = 256;
const int MAX_LIGHT_STEPS    = 8;
const int MAX_RAYTRACE_RANGE = 64;
const float SHADOW_STRENGHT  = 0.4;

const float pointLightVoxelRadius = 3.0; // e.g., 6.0 voxels
const float pointLightIntensity   = 1.0;  // e.g., 0.5
const vec3 pointLightColor        = vec3(1.0,1.0,1.0); // e.g., vec3(1.0, 0.95, 0.8)
const vec3 lightDir               = normalize(vec3(0.2, 1.0, 0.2));
float voxelSize                   = 1.0;


#define CAMERA_POINTLIGHT 1
#define RAYTRACED_SHADOWS 1

bool isSkyLight(ivec3 voxel, vec3 lightDir) {
    vec3 pos = vec3(voxel) + 0.5; // center of voxel
    for (int i = 0; i < voxelWorldSize; i++) {
        pos += lightDir;
        if (any(lessThan(pos, vec3(0.0))) || any(greaterThanEqual(pos, vec3(voxelWorldSize))))
            break;

        float density = texture(voxelTexture, pos / float(voxelWorldSize)).r;
        if (density != 0.0)
            return false; // blocked
    }
    return true;
}

void WriteDepth(vec3 hitPos)
{
    vec4 clipPos = projectionMatrix * viewMatrix * vec4(hitPos, 1.0);
    clipPos /= clipPos.w;
    float ndcDepth = clipPos.z;
    gl_FragDepth = ndcDepth * 0.5 + 0.5;
}

vec4 EncodeVoxel(ivec3 voxel, vec3 normal){
    vec3 encodedVoxel = vec3(voxel) / float(voxelWorldSize);
    vec3 encodedNormal = normal * 0.5 + 0.5; // Map [-1,1] → [0,1]

    int faceIndex = 0;
    if (normal.x > 0.5)      faceIndex = 0; // +X
    else if (normal.x < -0.5) faceIndex = 1; // -X
    else if (normal.y > 0.5)  faceIndex = 2; // +Y
    else if (normal.y < -0.5) faceIndex = 3; // -Y
    else if (normal.z > 0.5)  faceIndex = 4; // +Z
    else                      faceIndex = 5; // -Z

    float encodedFace = float(faceIndex) / 5.0;

    return vec4(encodedVoxel, encodedFace);
}

float SkyLight(ivec3 voxel, vec3 lightDir) {
    vec3 pos = vec3(voxel) + 0.5; // center of voxel
    for (int i = 0; i < voxelWorldSize; i++) {
        pos += lightDir;
        if (any(lessThan(pos, vec3(0.0))) || any(greaterThanEqual(pos, vec3(voxelWorldSize))))
            break;

        float density = texture(voxelTexture, pos / float(voxelWorldSize)).r;
        if (density != 0.0)
            return SHADOW_STRENGHT; // blocked
    }
    return 1.0;
}

float voxelShadow(vec3 startPos, vec3 dir) {
    vec3 pos = startPos;
    ivec3 voxel = ivec3(floor(pos * voxelWorldSize));

    vec3 rayStep = sign(dir);
    vec3 deltaT = abs(1.0 / (dir * float(voxelWorldSize)));

    vec3 voxelF = vec3(voxel);
    bvec3 stepPositive = greaterThan(rayStep, vec3(0.0));
    vec3 nextVoxelBorder = mix(voxelF, voxelF + vec3(1.0), stepPositive);
    vec3 nextT = (nextVoxelBorder / float(voxelWorldSize) - pos) / dir;

    int face = -1;       // 0 = X, 1 = Y, 2 = Z, Default invalid
    float faceDir = 0.0;

    for (int i = 0; i < MAX_LIGHT_STEPS * voxelWorldSize; i++) {
        if (any(lessThan(voxel, ivec3(0))) || any(greaterThanEqual(voxel, ivec3(voxelWorldSize))))
            break;

        float density = texelFetch(voxelTexture, voxel, 0).r;
        if (density != 0.0) {
            // Compute hit distance (t) along ray
            float hitT;
            if (face == 0) {
                hitT = nextT.x - deltaT.x;
            } else if (face == 1) {
                hitT = nextT.y - deltaT.y;
            } else if (face == 2) {
                hitT = nextT.z - deltaT.z;
            } else {
                hitT = 0.0;
            }

            vec3 hitPos = startPos + dir * hitT;
            vec3 voxelOrigin = vec3(voxel) / float(voxelWorldSize);
            vec3 localPos = (hitPos - voxelOrigin) * float(voxelWorldSize);
            vec3 local = clamp(localPos, 0.01, 0.99);
            vec2 voxelUV;
            if (face == 0) {            // X face
                voxelUV = vec2(local.z, local.y);
            } else if (face == 1) {     // Y face
                voxelUV = vec2(local.x, local.z);
            } else {                    // Z face
                voxelUV = vec2(local.x, local.y);
            }
            voxelUV = clamp(voxelUV, 0.0, 1.0);

            float voxelID = floor(density * 255.0);
            float xOffset = (face == 1) ? 1.0 : 0.0; 
            float yOffset = (tilesPerCol - 1 - voxelID) * VoxelScaleY;
            voxelUV.x = voxelUV.x * VoxelScaleX + (xOffset * VoxelScaleX); 
            voxelUV.y = voxelUV.y * VoxelScaleY + yOffset;

            vec4 textureColor = texture(voxelSpriteSheet, voxelUV);

            if (textureColor.a < 0.1) {
                // Transparent pixel, ignore voxel and continue marching
                // Do nothing here (just proceed with loop)
            } else {
                return SHADOW_STRENGHT;
            }
        }

        // Step the voxel along the ray
        if (nextT.x < nextT.y && nextT.x < nextT.z) {
            voxel.x += int(rayStep.x);
            nextT.x += deltaT.x;
            face = 0;
            faceDir = rayStep.x;
        } else if (nextT.y < nextT.z) {
            voxel.y += int(rayStep.y);
            nextT.y += deltaT.y;
            face = 1;
            faceDir = rayStep.y;
        } else {
            voxel.z += int(rayStep.z);
            nextT.z += deltaT.z;
            face = 2;
            faceDir = rayStep.z;
        }
    }

    return 1.0;
}


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
    ivec3 voxel = ivec3(floor(pos.x), floor(pos.y), floor(pos.z));
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
    bool isCenter = abs(TexCoords.x - 0.5) < 0.001 && abs(TexCoords.y - 0.5) < 0.001;
    ivec3 centerVoxel = ivec3(-1); // invalid initially

    for (int i = 0; i < MAX_STEPS; ++i) {
        vec3 texCoord = vec3(voxel) / float(voxelWorldSize);
        if (any(lessThan(texCoord, vec3(0.0))) || any(greaterThanEqual(texCoord, vec3(1.0))))
            break;

        if (isCenter && centerVoxel.x < 0) {
            // Save the first voxel the center ray is in
            centerVoxel = voxel;
        }

        float density = texture(voxelTexture, texCoord).r;
        // Hit detected
        if (density != 0.0) {

            //Calculate depth to populate the depthbuffer
            vec3 hitPos = rayOrigin + rayDir * tCurrent;
            WriteDepth(hitPos);

            voxelWorldPos = vec3(lastVoxel) * voxelSize;
            vec3 localPos = pos - voxelWorldPos;
            // vec3 local = clamp(localPos / voxelSize, 0.0, 1.0);
            vec3 local = clamp(vec3(pos - voxelWorldPos) / voxelSize, 0.01, 1.0- 0.01);
            vec2 voxelUV = vec2(0.0);

            if (face == 0) // X face
            { 
                // voxelUV = faceDir < 0 ? vec2(local.z, local.y) : vec2(1.0 - local.z, local.y);
                voxelUV = vec2(local.z, local.y);
                normal = vec3(-faceDir, 0.0, 0.0);  // X face
            } 
            else if (face == 1) // Y face
            { 
                voxelUV = vec2(local.x, local.z);
                normal = vec3(0.0, -faceDir, 0.0);  // Y face
            } 
            else if (face == 2) // Z face
            { 
                voxelUV = vec2(local.x, local.y);
                normal = vec3(0.0, 0.0, -faceDir);  // Z face
            }

            voxelUV = clamp(voxelUV, 0.0, 1.0);
           
            float voxelID = floor(density * 255.0);
            float xOffset = (face == 1) ? 1.0 : 0.0; 
            float yOffset = (tilesPerCol - 1 - voxelID) * VoxelScaleY;
            voxelUV.x = voxelUV.x * VoxelScaleX + (xOffset * VoxelScaleX); 
            voxelUV.y = voxelUV.y * VoxelScaleY + yOffset;
            vec4 textureColor = texture(voxelSpriteSheet, voxelUV);

            if (textureColor.a < 0.9) {
                // Step to next voxel and continue loop
                lastVoxel = voxel;

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
                continue; // skip lighting and move to the next voxel
            }

            FragColor = textureColor;
            vec3 baseColor = textureColor.rgb;


            float voxelWorldSizeF = float(voxelWorldSize);
            vec3 startShadowPos = (hitPos) / voxelWorldSizeF;
            #if RAYTRACED_SHADOWS
                float light = 0.7;
                if(distance(cameraPos,hitPos) < MAX_RAYTRACE_RANGE)
                {
                    light = voxelShadow(startShadowPos, lightDir);

                    if (light == 1.0) {
                        vec3 viewDir = normalize(cameraPos - hitPos);
                        vec3 halfDir = normalize(lightDir + viewDir);
                        float spec = pow(max(dot(normal, halfDir), 0.0), 32.0);
                        FragColor.rgb += spec * vec3(0.5,0.5,0.5);
                    }
                }
                else{
                    light = SkyLight(voxel,lightDir);
                }
          
                FragColor.rgb *= light;

            #endif

            vec3 voxelHit = hitPos / voxelSize; // convert hit position to voxel space
            float voxelDist = distance(voxelHit, cameraPos / voxelSize);

            #if CAMERA_POINTLIGHT
                if (voxelDist <= pointLightVoxelRadius) {
                    // Normalized direction to the camera
                    vec3 lightDirToCamera = normalize(cameraPos - hitPos);
                    // Simple linear falloff for performance
                    float attenuation = 1.0 - (voxelDist / pointLightVoxelRadius);
                    attenuation = max(attenuation, 0.0);
                    // Lambert term
                    float NdotL = max(dot(normal, lightDirToCamera), 0.0);
                    // Final point light contribution
                    vec3 pointLight = pointLightColor * pointLightIntensity * NdotL * attenuation;
                    FragColor.rgb += baseColor * pointLight;
                }
            #endif

            if (isCenter) 
            {    
                FragColor = EncodeVoxel(voxel,normal);
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

    gl_FragDepth = 1.0; // Far plane
    FragColor = vec4(0.529, 0.808, 0.922, 1.0); // Background
}