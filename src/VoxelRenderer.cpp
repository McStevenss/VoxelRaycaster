#include "VoxelRenderer.hpp"
#include <stb_image.h>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <cstdlib> // for rand()
#include "VoxelTerrain.h"
#include "BillboardSprite.h"

VoxelRenderer::VoxelRenderer(int width, int height, VoxelTerrain *terrain)
    : mScreenWidth(width), mScreenHeight(height)
{
    mTerrain = terrain;
    Init();
    loadTexture("textures/voxels/FloorTexture.png", voxelSurfaceTexture_floor,false);
    loadTexture("textures/voxels/WallTexture.png", voxelSurfaceTexture_wall,false);
    // loadTexture("textures/sprite.png", billboardSpriteTexture,true,true);
    loadTexture("textures/sprites/spritesheet.png", billboardSpriteTexture,true,true);

    int tilesPerRow = 10;
    int tilesPerCol = 10;
    glm::vec2 spriteScale(1.0f / tilesPerRow, 1.0f / tilesPerCol);

    //Initialize buffers (Static)
    BillboardSprite::InitBuffers();
    BillboardSprite::SetTexture(billboardSpriteTexture, spriteScale);
    BillboardSprite::SetShader(mBillboardShader);

    int tileX = 3;
    int tileY = 5; // zero-indexed
    // float billboardSize = 1.0f; 
    float billboardSize = 0.8f; 
    
    glm::vec2 spriteOffset(tileX * spriteScale.x, (tilesPerCol - 1 - tileY) * spriteScale.y);

    mSprites.emplace_back(glm::vec3(204.0f,224.0f,75.0f), billboardSize, spriteOffset);
    mSprites.emplace_back(glm::vec3(200.0f,224.0f,76.0f), billboardSize, spriteOffset);


    // tileY = 4;
    // spriteOffset = glm::vec2(tileX * spriteScale.x, (tilesPerCol - 1 - tileY) * spriteScale.y);

    // mSprites.emplace_back(glm::vec3(205.0f,218.0f,76.0f), billboardSize, spriteOffset);
    
}

void VoxelRenderer::Init() {
    mShader = new Shader("shaders/voxel_raycast.vert", "shaders/voxel_raycast.frag");
    mBillboardShader = new Shader("shaders/billboard.vert", "shaders/billboard.frag");
    mSkyboxShader = new Shader("shaders/skybox.vert","shaders/skybox.frag");

    InitFullscreenQuad();

    //Create an explicit framebuffer instead of using the default one so we can attach a depthbuffer.
    //We will write to it in the voxelrenderer fragmentshader
    glGenFramebuffers(1, &mFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

    glGenTextures(1, &mColorTexture);
    glBindTexture(GL_TEXTURE_2D, mColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mScreenWidth, mScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorTexture, 0);

    glGenTextures(1, &mDepthTexture);
    glBindTexture(GL_TEXTURE_2D, mDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, mScreenWidth, mScreenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthTexture, 0);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "[VoxelRenderer] FBO incomplete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    voxels = mTerrain->getVoxels();
    glGenTextures(1, &voxelTexture);
    glBindTexture(GL_TEXTURE_3D, voxelTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, mTerrain->VoxelWorldSize, mTerrain->VoxelWorldSize, mTerrain->VoxelWorldSize, 0, GL_RED, GL_UNSIGNED_BYTE, voxels.data());
    
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);      
    
    
    mTerrain->VoxelTexture = voxelTexture;
}

void VoxelRenderer::loadTexture(const std::string &path, GLuint &textureRef, bool flipVertically, bool isRGBA){

    int w, h, n;
    stbi_uc* colorData;
    stbi_set_flip_vertically_on_load(flipVertically);

    int desiredChannels = isRGBA ? 4 : 3;
    colorData = stbi_load(path.c_str(), &w, &h, &n, desiredChannels);
     
    if (!colorData) {
        std::cerr << "[VoxelRenderer] Error loading:" << path.c_str() << std::endl;
        exit(1);
    }
    
    GLenum format = isRGBA ? GL_RGBA : GL_RGB;
    GLenum internalFormat = isRGBA ? GL_RGBA8 : GL_RGB8;
    GLenum min_filter = isRGBA ? GL_NEAREST : GL_LINEAR_MIPMAP_LINEAR;

    glGenTextures(1, &textureRef);
    glBindTexture(GL_TEXTURE_2D, textureRef);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, colorData);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter); // Or GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    stbi_image_free(colorData);
}


void VoxelRenderer::RenderVoxels(const Camera& camera) 
{
    glClearColor(0.529, 0.808, 0.922, 1.0);
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glViewport(0, 0, mScreenWidth, mScreenHeight);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    mShader->setFloat("nearPlane", camera.mNearPlane);
    mShader->setFloat("farPlane", camera.mFarPlane);
    mShader->setMat4("invProjection",invProj);
    mShader->setMat4("projectionMatrix",projection);
    mShader->setMat4("invView",invView);
    mShader->setMat4("viewMatrix",view);
    mShader->setInt("voxelWorldSize",mTerrain->VoxelWorldSize);


    //############## VOXELTERRAIN 3D TEXTURE ###########
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, voxelTexture);
    //##################################################

    //################ WALL/FLOOR TEXTURES #############
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, voxelSurfaceTexture_wall);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, voxelSurfaceTexture_floor);
    //#######################################################

    glBindVertexArray(mQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);



    //################ DRAW BILLBOARDS/SPRITES #############
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (auto& sprite : mSprites) {  // store your sprites as a member
        sprite.Draw(view, projection, camera.mEye);
    }
    glDisable(GL_BLEND);

    
    RenderSkyBox(projection,view);
    

    // TEMP copy raycast buffer to main to see
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, mScreenWidth, mScreenHeight,
                      0, 0, mScreenWidth, mScreenHeight,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                      GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default framebuffer
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


void VoxelRenderer::RenderSkyBox(const glm::mat4& projection, const glm::mat4& view)
{
    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    
    mSkyboxShader->use();
    mSkyboxShader->setMat4("projection", projection);
    mSkyboxShader->setMat4("view", glm::mat4(glm::mat3(view)));

    glBindVertexArray(skyBox.VAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
