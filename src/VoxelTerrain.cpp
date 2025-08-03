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
                voxels[x + y * VoxelWorldSize + z * VoxelWorldSize * VoxelWorldSize] = 1;
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

void VoxelTerrain::setVoxel(int x, int y, int z, uint8_t value)
{
    if (x < 0 || x >= VoxelWorldSize ||
        y < 0 || y >= VoxelWorldSize ||
        z < 0 || z >= VoxelWorldSize) return;

    voxels[x + y * VoxelWorldSize + z * VoxelWorldSize * VoxelWorldSize] = value;
}

void VoxelTerrain::updateVoxelGPU(int x, int y, int z)
{
    glBindTexture(GL_TEXTURE_3D, VoxelTexture);
    uint8_t value = voxels[x + y * VoxelWorldSize + z * VoxelWorldSize * VoxelWorldSize];
    glTexSubImage3D(GL_TEXTURE_3D, 0, x, y, z, 1, 1, 1, GL_RED, GL_UNSIGNED_BYTE, &value);
}

glm::ivec3 VoxelTerrain::decodeVoxel(int mScreenWidth, int mScreenHeight, bool addBlock)
{
    float voxelRGBA[4];
    glReadPixels(mScreenWidth / 2, mScreenHeight / 2, 1, 1, GL_RGBA, GL_FLOAT, voxelRGBA);
    glm::ivec3 voxelXYZ = glm::floor(glm::vec3(voxelRGBA[0], voxelRGBA[1], voxelRGBA[2]) * float(VoxelWorldSize));
        
    voxelXYZ = glm::ivec3(voxelXYZ.x+1, voxelXYZ.y+1, voxelXYZ.z);

    int faceIndex = int(round(voxelRGBA[3] * 5.0f));
        
    glm::ivec3 faceNormal;
    switch(faceIndex) {
        case 0: faceNormal = { 1, 0, 0 }; break; // +X
        case 1: faceNormal = {-1, 0, 0 }; break; // -X
        case 2: faceNormal = { 0, 1, 0 }; break; // +Y
        case 3: faceNormal = { 0,-1, 0 }; break; // -Y
        case 4: faceNormal = { 0, 0, 1 }; break; // +Z
        case 5: faceNormal = { 0, 0,-1 }; break; // -Z
    }

        glm::ivec3 targetVoxel = voxelXYZ + (addBlock ? faceNormal : glm::ivec3(0));

    return targetVoxel;
}