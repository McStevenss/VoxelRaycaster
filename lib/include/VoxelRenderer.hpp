#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Camera.hpp"
#include "Shader.hpp"
#include "VoxelTerrain.h"
#include "BillboardSprite.h"
#include "skybox.h"




class VoxelRenderer {
public:
    // VoxelRenderer(int width, int height);
    VoxelRenderer(int width, int height, VoxelTerrain *terrain);
    void Init();
    void RenderVoxels(const Camera& camera);

    bool isVoxel(glm::vec3 pos);
    // void loadTexture(const std::string &path, GLuint &textureRef);
    void loadTexture(const std::string &path, GLuint &textureRef, bool flipVertically, bool isRGBA=false);
    void RenderSkyBox(const glm::mat4& projection, const glm::mat4& view);
 


private:
    int mScreenWidth, mScreenHeight;
    
    VoxelTerrain *mTerrain;
    std::vector<uint8_t> mFrameBuffer;
    GLuint mTextureID;
    
    GLuint voxelTexture;
    GLuint voxelSurfaceTexture_wall;
    GLuint voxelSurfaceTexture_floor;
    GLuint billboardSpriteTexture;
    
    std::vector<BillboardSprite> mSprites;

    SkyBox skyBox;
    Shader* mShader = nullptr;
    Shader* mBillboardShader = nullptr;
    Shader* mSkyboxShader = nullptr;
    unsigned int mQuadVAO = 0;
    unsigned int mQuadVBO = 0;
    
    unsigned int mFBO = 0;
    unsigned int mColorTexture = 0;
    unsigned int mDepthTexture = 0;

    std::vector<GLubyte> voxels;

    void InitFullscreenQuad();
};