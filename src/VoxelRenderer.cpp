#include "VoxelRenderer.hpp"
#include <stb_image.h>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <cstdlib> // for rand()
#include "VoxelTerrain.h"

VoxelRenderer::VoxelRenderer(int width, int height, VoxelTerrain *terrain)
    : mScreenWidth(width), mScreenHeight(height)
{
    mTerrain = terrain;
    Init();
    loadTexture("textures/voxels/FloorTexture.png", voxelSurfaceTexture_floor);
    loadTexture("textures/voxels/WallTexture.png", voxelSurfaceTexture_wall);
}

void VoxelRenderer::Init() {
    mShader = new Shader("shaders/voxel_raycast.vert", "shaders/voxel_raycast.frag");

    InitFullscreenQuad();
    voxels = mTerrain->getVoxels();
    
    glGenTextures(1, &voxelTexture);
    glBindTexture(GL_TEXTURE_3D, voxelTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, mTerrain->VoxelWorldSize, mTerrain->VoxelWorldSize, mTerrain->VoxelWorldSize, 0, GL_RED, GL_UNSIGNED_BYTE, voxels.data());
    
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);           
}

void VoxelRenderer::loadTexture(const std::string &path, GLuint &textureRef){
    int w, h, n;
    stbi_uc* colorData = stbi_load(path.c_str(), &w, &h, &n, 3);
    if (!colorData) {
        std::cerr << "[VoxelRenderer] Error loading:" << path.c_str() << std::endl;
        exit(1);
    }
    
    glGenTextures(1, &textureRef);
    glBindTexture(GL_TEXTURE_2D, textureRef);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, colorData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Or GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    stbi_image_free(colorData);
}


void VoxelRenderer::RenderVoxels(const Camera& camera) {

    glm::mat4 projection = camera.GetProjectionMatrix();
    glm::mat4 view = camera.GetViewMatrix();

    glm::mat4 invProj = glm::inverse(projection);
    glm::mat4 invView = glm::inverse(view);

    mShader->use();

    mShader->setInt("voxelTexture",0);
    //####
    mShader->setInt("voxelSurfaceTextureWall",1);
    mShader->setInt("voxelSurfaceTextureFloor",2);
    //####
    
    mShader->setVec3("cameraPos",camera.mEye);
    mShader->setMat4("invProjection",invProj);
    mShader->setMat4("invView",invView);
    mShader->setInt("voxelWorldSize",mTerrain->VoxelWorldSize);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, voxelTexture);

    //################ TEMP WALL/FLOOR TEXTURES #############
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, voxelSurfaceTexture_wall);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, voxelSurfaceTexture_floor);
    //#######################################################

    glBindVertexArray(mQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindVertexArray(0);

}

void VoxelRenderer::InitFullscreenQuad() 
{
    float quadVertices[] = {
    // positions   // texCoords (flipped vertically)
    -1.0f, -1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 1.0f,
     1.0f,  1.0f,  1.0f, 0.0f,

    -1.0f, -1.0f,  0.0f, 1.0f,
     1.0f,  1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f,  0.0f, 0.0f
    };

    glGenVertexArrays(1, &mQuadVAO);
    glGenBuffers(1, &mQuadVBO);

    glBindVertexArray(mQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1); // texCoords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}
