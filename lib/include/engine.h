#ifndef ENGINE_H
#define ENGINE_H
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <SDL2/SDL.h>
#include <glad/glad.h>


//---- Engine libs ----
#include "skybox.h"
#include "VoxelRenderer.hpp"
#include "Player.h"

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

        // OpenGL context
        SDL_GLContext mOpenGLContext;

        VoxelTerrain *terrain;
        VoxelRenderer *renderer;
        Player *mPlayer;

        bool drawWireframe;
        bool drawLightDepth;
        void MainLoop();
        float GetDeltaTime();

    private:

        std::vector<float> frameTimes;
        const int maxSamples = 100;

        void Input();
        void InitializeProgram();

};

#endif
