#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Camera.hpp"
#include "Shader.hpp"
#include "VoxelTerrain.h"

class VoxelRenderer {
public:
    // VoxelRenderer(int width, int height);
    VoxelRenderer(int width, int height, VoxelTerrain *terrain);
    void Init();
    void RenderVoxels(const Camera& camera);

    bool isVoxel(glm::vec3 pos);
    void loadTexture(const std::string &path, GLuint &textureRef);
    

private:
    int mScreenWidth, mScreenHeight;
    
    VoxelTerrain *mTerrain;
    std::vector<uint8_t> mFrameBuffer;
    GLuint mTextureID;
    
    GLuint voxelTexture;
    GLuint voxelSurfaceTexture_wall;
    GLuint voxelSurfaceTexture_floor;
    
    Shader* mShader = nullptr;
    unsigned int mHeightmapTex = 0;
    unsigned int mColormapTex = 0;
    unsigned int mQuadVAO = 0;
    unsigned int mQuadVBO = 0;
    int VOXEL_WORLD_SIZE = 256;
    std::vector<GLubyte> voxels;

    void InitFullscreenQuad();
};