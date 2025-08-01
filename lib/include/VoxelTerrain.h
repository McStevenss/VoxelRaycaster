#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Camera.hpp"
#include "Shader.hpp"

struct Ray {
    glm::vec3 origin;
    glm::vec3 dir; // Must be normalized
};

struct VoxelHit {
    bool valid;
    glm::ivec3 voxel;
    glm::vec3 position;
    int face; // Optional: which face was hit
};

class VoxelTerrain {
    public:
        VoxelTerrain(unsigned int seed);
        bool isVoxel(glm::vec3 pos);
        std::vector<GLubyte> getVoxels();
        void setVoxel(int x, int y, int z, uint8_t value);
        void updateVoxelGPU(int x, int y, int z);
        int VoxelWorldSize = 256;
        Ray computeCameraRay(glm::vec3 viewDir, glm::vec3 cameraPos, glm::mat4 invProjection, glm::mat4 invView);
        VoxelHit raycastVoxels(Ray ray, int maxSteps);

        GLuint VoxelTexture;

    private:
        int mMapSize;
        std::vector<GLubyte> voxels;

    };