#include "BufferManager.h"
#include <stdio.h>
#include <iostream>
#include <vector>
#include "glm/glm.hpp"

BufferManager::BufferManager(int screenWidth, int screenHeight, std::vector<float>& ShadowCascades)
{
    mScreenWidth = screenWidth;
    mScreenHeight = screenHeight;
    shadowCascades = ShadowCascades;
}

unsigned int BufferManager::CreateBufferTexture(int width, int height, GLenum internalFormat, GLenum format, GLenum type, GLenum attachment)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (attachment != GL_NONE)
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, 0);

    return texture;
}

void BufferManager::CreateBuffers()
{
    CreatePreviewBuffer();
    CreateGBuffer();
    CreateSSAOBuffer();
    CreateCSMBuffers();
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}

void BufferManager::CreatePreviewBuffer()
{
    glGenFramebuffers(1, &previewBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, previewBuffer);
    previewPosition = CreateBufferTexture(mScreenWidth,mScreenHeight, GL_RGBA,GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT0);
    previewNormals = CreateBufferTexture(mScreenWidth,mScreenHeight, GL_RGBA,GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT1);
    previewDepth = CreateBufferTexture(mScreenWidth,mScreenHeight, GL_RGBA,GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT2);
    glDrawBuffers(3, drawBuffers);

    // Check framebuffer completion
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "[!][Renderer][Deferred] Preview framebuffer not complete!" << std::endl;
    else
        std::cout << "[Renderer] Preview framebuffer complete!" << std::endl;
}

void BufferManager::CreateGBuffer()
{
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    gPosition = CreateBufferTexture(mScreenWidth,mScreenHeight,GL_RGBA16F,GL_RGBA,GL_FLOAT, GL_COLOR_ATTACHMENT0);
    gNormal = CreateBufferTexture(mScreenWidth,mScreenHeight,GL_RGBA16F,GL_RGBA,GL_FLOAT, GL_COLOR_ATTACHMENT1);
    gAlbedoSpec = CreateBufferTexture(mScreenWidth,mScreenHeight,GL_RGBA,GL_RGBA,GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT2);
    glDrawBuffers(3, drawBuffers);

    //Create and attach depth buffer
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, mScreenWidth, mScreenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "[!][Renderer][Deferred] Framebuffer not complete!" << std::endl;
    else
        std::cout << "[Renderer] Framebuffer complete!" << std::endl;
}

void BufferManager::CreateSSAOBuffer()
{
    halfWidth = mScreenWidth / divideFactor;
    halfHeight = mScreenHeight / divideFactor;
    glGenFramebuffers(1, &ssaoFBO); 
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    
    ssaoColorBuffer = CreateBufferTexture(halfWidth,halfHeight,GL_RED, GL_RED, GL_FLOAT, GL_COLOR_ATTACHMENT0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "[Renderer][Deferred] SSAO Framebuffer not complete!" << std::endl;
    else
    std::cout << "[Renderer] SSAO Framebuffer complete!" << std::endl;
    
    glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    ssaoColorBufferBlur = CreateBufferTexture(halfWidth, halfHeight, GL_RED, GL_RED, GL_FLOAT, GL_COLOR_ATTACHMENT0);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "[Renderer][Deferred] SSAO Blur Framebuffer not complete!" << std::endl;
    else
        std::cout << "[Renderer] SSAO Blur Framebuffer complete!" << std::endl;
    
}

void BufferManager::CreateShadowMapBuffer()
{
    glGenFramebuffers(1, &depthMapFBO);  
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    // depthMap = CreateBufferTexture(SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT,GL_DEPTH_COMPONENT,GL_FLOAT, GL_DEPTH_ATTACHMENT);
    depthMap = CreateBufferTexture(SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT32F,GL_DEPTH_COMPONENT,GL_FLOAT, GL_DEPTH_ATTACHMENT);
    

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Check framebuffer completion
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "[!][Renderer][Deferred] Shadow Depth framebuffer not complete!" << std::endl;
    else 
        std::cout << "[Renderer] Shadow Depth framebuffer complete!" << std::endl;
}

void BufferManager::CreateCSMBuffers()
{
    // configure light FBO
    // -----------------------
    glGenFramebuffers(1, &lightFBO);

    glGenTextures(1, &lightDepthMaps);
    glBindTexture(GL_TEXTURE_2D_ARRAY, lightDepthMaps);
    glTexImage3D(
        GL_TEXTURE_2D_ARRAY, 
        0, 
        GL_DEPTH_COMPONENT32F, 
        SHADOW_RESOLUTION, 
        SHADOW_RESOLUTION, 
        int(shadowCascades.size()) + 1,
        0, 
        GL_DEPTH_COMPONENT, 
        GL_FLOAT, 
        nullptr);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

    glBindFramebuffer(GL_FRAMEBUFFER, lightFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, lightDepthMaps, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
        throw 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenBuffers(1, &matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

}
