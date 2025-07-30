#ifndef RENDERER_H
#define RENDERER_H
#include "Camera.hpp"
#include "ShaderManager.hpp"
#include "BufferManager.h"
#include "skybox.h"
#include <SDL2/SDL.h>
#include "Shader.hpp"
#include "scene.h"

class Renderer {
    public:
        Renderer(int ScreenWidth, int ScreenHeight, Camera* camera);
        ~Renderer();    
        // void FinalizeFrame();
        void ReloadShaders();

        std::vector<glm::vec3> lightPositions;
        std::vector<glm::vec3> lightColors;

        std::tuple<unsigned int, unsigned int, unsigned int,unsigned int> getPreviewValues();
    
        int screenWidth, screenHeight;

        //---------------------        
        //Light cubes
        //---------------------
        const unsigned int NR_LIGHTS = 64;
        int light_cube_spread = 25;
        float amplitude = 10.0f;
        float cube_speed = 0.02f;
        
        //---------------------
        //Renderer settings
        //---------------------
        bool useSSAO = true;
        bool useFog = false;
        bool shouldRenderBoundingBoxes = false;
        int renderMode = 0;

        //---------------------
        //SSAO
        //---------------------
        std::vector<glm::vec3> ssaoNoise;
        std::vector<glm::vec3> ssaoKernel;
        int ssao_kernel_size = 128;
        float ssao_radius = 2.17;
        float ssao_bias = 0.09;

        unsigned int bboxVAO, bboxVBO, bboxEBO;

    private:
        //---------------------
        //Preparation functions
        //---------------------
        void PrepareForRender();
        void CreateBuffers();
        void ConfigureShaders();
        void CreateSSAONoise();
        void SetupBoundingBoxes();
        unsigned int CreateBufferTexture(int width, int height, GLenum internalFormat, GLenum format, GLenum type, GLenum attachment);
        
        glm::vec3 lightDir;
        unsigned int LoadTexture(const char* path);
        float Lerp(float a, float b, float f);
        
        std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview);
        std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view);
        glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane);
        std::vector<glm::mat4> getLightSpaceMatrices();
        
        // glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane, const glm::mat4& lightView);
        // std::vector<glm::mat4> getLightSpaceMatrices(const glm::mat4& lightView);
        
        
     

        //---------------------
        //Render Passes
        //---------------------
        void RenderShadowPass(Scene& scene, glm::mat4& projection, glm::mat4& view);
        void RenderGeometryPass(Node& sceneRoot, glm::mat4& projection, glm::mat4& view);
        void RenderSSAOPass(glm::mat4& projection, glm::mat4& view);
        void RenderPreviewFrames();
        void RenderLightingPass(Scene& scene);
        void CopyDepthToDefaultFramebuffer();
        void RenderLightCubes(const glm::mat4&, const glm::mat4&);
        void RenderBoundingBoxes(Node&, const glm::mat4&, const glm::mat4&);
        void RenderSkyBox(const glm::mat4& projection, const glm::mat4& view);
        void RenderScene(Shader &shader, Node &sceneRoot, glm::mat4 &projection, glm::mat4 &view);
        float GetDeltaTime();
        void RenderToFrame();
        void RenderCube();
        
        //---------------------
        //Window settings
        //---------------------
        ShaderManager* shaderManager;
        BufferManager* bufferManager;
    
        unsigned int quadVAO = 0;
        unsigned int quadVBO = 0;
        unsigned int cubeVAO = 0;
        unsigned int cubeVBO = 0;

        //---------------------
        //Misc settings
        //---------------------
        SkyBox skyBox;
        float divideFactor = 1.0f;
        Camera* camera;
        
        //---------------------        
        //Shadow settings
        //---------------------
        float shadow_near_plane =13.0f, shadow_far_plane = 50.0f;
        float cameraNearPlane = 0.1f;
        float cameraFarPlane = 500.0f;
        // float cameraFarPlane = 250.0f;
        std::vector<float> shadowCascadeLevels{ cameraFarPlane / 50.0f, cameraFarPlane / 25.0f, cameraFarPlane / 10.0f, cameraFarPlane / 1.0f };
    };

#endif