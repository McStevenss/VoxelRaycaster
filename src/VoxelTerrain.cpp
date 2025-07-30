#include "VoxelTerrain.h"
#include <cstdlib> // for rand()

VoxelTerrain::VoxelTerrain(unsigned int seed)
{
    srand(seed);

    //Generate random map for now
    const int VoxelPadding = 32;
    voxels.resize(VoxelWorldSize * VoxelWorldSize * VoxelWorldSize, 0);

    for (int z = VoxelPadding; z < VoxelWorldSize-VoxelPadding; ++z)
        for (int y = VoxelPadding; y < VoxelWorldSize-VoxelPadding; ++y)
            for (int x = VoxelPadding; x < VoxelWorldSize-VoxelPadding; ++x)
                voxels[x + y * VoxelWorldSize + z * VoxelWorldSize * VoxelWorldSize] = (rand() % 100 < 50) ? 255 : 0;
}

bool VoxelTerrain::isVoxel(glm::vec3 pos)
{
    int x = (int)pos.x;
    int y = (int)pos.y;
    int z = (int)pos.z;
    
    if (x < 0 || y < 0 || z < 0 || x >= VoxelWorldSize || y >= VoxelWorldSize || z >= VoxelWorldSize)
        return false;

    int index = x + y * VoxelWorldSize + z * VoxelWorldSize * VoxelWorldSize;
    return voxels[index] != 0;
}

std::vector<GLubyte> VoxelTerrain::getVoxels()
{
    return voxels;
}
