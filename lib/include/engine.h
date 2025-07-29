#ifndef ENGINE_H
#define ENGINE_H
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include "camera.hpp"
#include "Shader.hpp"
// #include "point_light.h"
// #include "directional_light.h"
#include "skybox.h"
#include "ShaderManager.hpp"
#include "VoxelRenderer.hpp"

class Engine{
    public:

        //Constructor
        Engine();

        ~Engine()
        {
            SDL_GL_DeleteContext(mOpenGLContext);
            SDL_DestroyWindow(mGraphicsApplicationWindow);
            SDL_Quit();
        }

        // Window properties
        int mScreenWidth;
        int mScreenHeight;
        SDL_Window* mGraphicsApplicationWindow;

        // GameLoop
        bool mQuit = false;
        Camera camera;
        // OpenGL context
        SDL_GLContext mOpenGLContext;


        VoxelRenderer *renderer;

        bool drawWireframe;
        bool drawLightDepth;
        void MainLoop();
        float GetDeltaTime();
        bool checkHorizontalCollision(const glm::vec3& centerPos, float yOffset);
        bool checkCollisionAt(const glm::vec3& pos);

    private:

        bool lockMouse = false;
        bool gravity = false;
        bool collisionMode = true;
        //Light cubes
        std::vector<glm::vec3> lightPositions;
        std::vector<glm::vec3> lightColors;

        const float PLAYER_HEIGHT = 0.5f;
        const float PLAYER_RADIUS = 0.25f;

        std::vector<float> frameTimes;
        const int maxSamples = 100;

        void Input();
        void InitializeProgram();

};

#endif
