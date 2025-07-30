#include "VoxelTerrain.h"
#include <cstdlib> // for rand()

VoxelTerrain::VoxelTerrain(unsigned int seed)
{
    srand(seed);

    //Generate random map for now
    const int VOXEL_PADDING = 32;
    voxels.resize(VOXEL_WORLD_SIZE * VOXEL_WORLD_SIZE * VOXEL_WORLD_SIZE, 0);

    for (int z = VOXEL_PADDING; z < VOXEL_WORLD_SIZE-VOXEL_PADDING; ++z)
        for (int y = VOXEL_PADDING; y < VOXEL_WORLD_SIZE-VOXEL_PADDING; ++y)
            for (int x = VOXEL_PADDING; x < VOXEL_WORLD_SIZE-VOXEL_PADDING; ++x)
                voxels[x + y * VOXEL_WORLD_SIZE + z * VOXEL_WORLD_SIZE * VOXEL_WORLD_SIZE] = (rand() % 100 < 50) ? 255 : 0;
}

bool VoxelTerrain::isVoxel(glm::vec3 pos)
{
    int x = (int)pos.x;
    int y = (int)pos.y;
    int z = (int)pos.z;
    
    if (x < 0 || y < 0 || z < 0 || x >= VOXEL_WORLD_SIZE || y >= VOXEL_WORLD_SIZE || z >= VOXEL_WORLD_SIZE)
        return false;

    int index = x + y * VOXEL_WORLD_SIZE + z * VOXEL_WORLD_SIZE * VOXEL_WORLD_SIZE;
    return voxels[index] != 0;
}

std::vector<GLubyte> VoxelTerrain::getVoxels()
{
    return voxels;
}
