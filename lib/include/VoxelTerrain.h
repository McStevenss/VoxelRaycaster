#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Camera.hpp"
#include "Shader.hpp"

class VoxelTerrain {
    public:
        VoxelTerrain(unsigned int seed);
        bool isVoxel(glm::vec3 pos);
        std::vector<GLubyte> getVoxels();

    private:
        int mMapSize;
        int VOXEL_WORLD_SIZE = 256;
        std::vector<GLubyte> voxels;

    };