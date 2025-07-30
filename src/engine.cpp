
//ImGui + SDL
#include <SDL2/SDL.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl3.h"

// Engine libs
#include "Engine.h"
#include "VoxelRenderer.hpp"
#include "Player.h"
#include "VoxelTerrain.h"

// STD libs + GLM
#include <stdio.h>
#include <iostream>
#include <random>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

Engine::Engine()
{
    std::cout << "Engine Created!" << std::endl;
    mScreenWidth  = 1920;
    mScreenHeight = 1080;
    mGraphicsApplicationWindow = nullptr;
    mOpenGLContext             = nullptr;
    bool mMouseActive          = true;
    int terrainSeed            = 69;
    InitializeProgram();

    mPlayer = new Player(glm::vec3(199.0f, 228.0f, 68.0f),mScreenWidth,mScreenHeight,mGraphicsApplicationWindow);
    terrain = new VoxelTerrain(terrainSeed);
    renderer = new VoxelRenderer(mScreenWidth,mScreenHeight, terrain);
    
}


void Engine::InitializeProgram(){
    //Initialize SDL
    std::cout << "[Engine] Initializing SDL...";
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cout << "[!] SDL could not initialize" << std::endl;
        exit(1);
    }
    std::cout << "Done!" << std::endl;

    //Set context to OpenGL version 4.1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    std::cout << "[Engine] Creating Window...";
    mGraphicsApplicationWindow = SDL_CreateWindow(
        "OpenGL Window",         // Window title
        SDL_WINDOWPOS_CENTERED,  // X position: centered
        SDL_WINDOWPOS_CENTERED,  // Y position: centered
        mScreenWidth,            // Window width
        mScreenHeight,           // Window height
        SDL_WINDOW_OPENGL        // Window flags
    );

    if(mGraphicsApplicationWindow == nullptr){
        std::cout << "[!][Engine] Could not create application window." << std::endl;
        exit(1);
    }
    std::cout << "Done!" << std::endl;

    std::cout << "[Engine] Creating OpenGL context...";
    mOpenGLContext = SDL_GL_CreateContext(mGraphicsApplicationWindow);
    if(mOpenGLContext == nullptr){
        std::cout << "[!][Engine] Could not create OpenGL context" << std::endl;
        exit(1);
    }
    SDL_GL_MakeCurrent(mGraphicsApplicationWindow, mOpenGLContext);
    std::cout << "Done!" << std::endl;

    //Initialize GLAD lib
    std::cout << "[Engine] Initializing GLAD...";  
    if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){
        std::cout << "[!][Engine] Could not initialize GLAD-lib" << std::endl;
        exit(1);
    }
    std::cout << "Done!"<< std::endl;  

    //Initialize ImGUI
    std::cout << "[Engine] Initializing ImGUI...";  
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

     // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(mGraphicsApplicationWindow, mOpenGLContext);
    ImGui_ImplOpenGL3_Init("#version 410");
    std::cout << "Done!" << std::endl;  


    std::cout << "[Engine] System info:" << std::endl;
    std::cout << "\n------------------------<OpenGL Info>-----------------------" << std::endl;
    std::cout << "Vendor:  " << glGetString(GL_VENDOR)<< std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Shading language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "------------------------------------------------------------\n" << std::endl;


    SDL_WarpMouseInWindow(mGraphicsApplicationWindow, mScreenWidth/2,mScreenHeight/2);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    // SDL_GL_SetSwapInterval(0);
    SDL_GL_SetSwapInterval(-1);
}

void Engine::MainLoop()
{
 
    while(!mPlayer->mQuit){
         
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        
        float deltaTime = GetDeltaTime();
        mPlayer->Update(deltaTime, terrain);

        renderer->RenderVoxels(mPlayer->mCamera);
        
        ImGui::Begin("Info panel");
        // Begin the Info panel window
            float fps = 1.0f / deltaTime;
            ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", mPlayer->mPosition.x, mPlayer->mPosition.y, mPlayer->mPosition.z);
            ImGui::Text("Camera Speed: %.3f", mPlayer->mCamera.speed);
            ImGui::Text("FPS: %.3f", fps);
            ImGui::Text("Collision Mode: %s", mPlayer->collisionMode ? "true" : "false");
            ImGui::Text("In Air: %s", mPlayer->inAir ? "true" : "false");
            ImGui::Checkbox("Toggle collision mode:", &mPlayer->collisionMode);
            ImGui::Checkbox("Toggle gravity:", &mPlayer->gravity);
        ImGui::End();
        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        
        SDL_GL_SwapWindow(mGraphicsApplicationWindow);

    }
}

float Engine::GetDeltaTime()
{
    static uint64_t lastTime = SDL_GetTicks64();
    uint64_t currentTime = SDL_GetTicks64();
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    return deltaTime;
}
