#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "camera.hpp"
#include "Shader.hpp"

class VoxelRenderer {
public:
    VoxelRenderer(int width, int height);
    void Init();
    float GetTerrainHeight(float worldX, float worldZ) const;
    void RenderVoxels(const Camera& camera);

    bool isVoxel(glm::vec3 pos);
    void loadTexture(const std::string &path, GLuint &textureRef);


private:
    int mScreenWidth, mScreenHeight;
    int mMapSize;

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

    void InitRenderTexture();
    void DrawToScreen();
    void InitFullscreenQuad();
};