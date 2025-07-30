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
        int VoxelWorldSize = 256;

    private:
        int mMapSize;
        std::vector<GLubyte> voxels;

    };