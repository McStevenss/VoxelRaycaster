#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include <glad/glad.h>
#include <stdio.h>
#include <vector>

class BufferManager {
    public:
        BufferManager(int screenWidth, int screenHeight, std::vector<float>& ShadowCascades);
        unsigned int CreateBufferTexture(int width, int height, GLenum internalFormat, GLenum format, GLenum type, GLenum attachment);
        void CreateBuffers();
        
        //Gbuf
        unsigned int gBuffer;
        unsigned int gPosition, gNormal, gAlbedoSpec;
        unsigned int rboDepth;
        
        // Preview
        unsigned int previewBuffer;
        unsigned int previewPosition, previewNormals, previewDepth;

        // SSAO
        unsigned int ssaoFBO, ssaoBlurFBO;
        unsigned int ssaoColorBuffer, ssaoColorBufferBlur;
        unsigned int noiseTexture;
        unsigned int mAtachments[3];

        //Shadowmap
        unsigned int depthMapFBO;
        unsigned int depthMap;

        unsigned int lightFBO;
        unsigned int lightDepthMaps;
        unsigned int matricesUBO;
        int cameraFarPlane;

        GLenum drawBuffers[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};

        // float divideFactor = 1.0f;
        float divideFactor = 1.25f;
        unsigned int SHADOW_WIDTH = 4096;
        unsigned int SHADOW_HEIGHT= 4096;
        // unsigned int SHADOW_RESOLUTION= 4096;
        // unsigned int SHADOW_RESOLUTION= 2048;
        unsigned int SHADOW_RESOLUTION= 3072;
        
        std::vector<float> shadowCascades;

    private:
        void CreatePreviewBuffer();
        void CreateGBuffer();
        void CreateSSAOBuffer();
        void CreateShadowMapBuffer();
        void CreateCSMBuffers();

        int mScreenWidth;
        int mScreenHeight;

        int halfHeight;
        int halfWidth;

};


#endif