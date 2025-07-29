
//ImGui + SDL
#include <SDL2/SDL.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl3.h"

// Engine libs
#include "engine.h"
#include "VoxelRenderer.hpp"

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
    mQuit                      = false; 
    bool mMouseActive          = true;
    InitializeProgram();
    // camera.SetProjectionMatrix
    camera.SetProjectionMatrix(90.0f,(float)mScreenWidth/(float)mScreenHeight,0.1,1000);
    std::cout << "[Engine][Camera] Fovy:" << camera.mFovy << std::endl;
    renderer = new VoxelRenderer(mScreenWidth,mScreenHeight);
    

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
 
    const float gravityConstant = -9.81f; // meters per second squared
    bool inAir = true;
    glm::vec3 mVelocity = glm::vec3(0.0f); // Add this to your Camera class

    const glm::vec2 offsets[] = {
        { PLAYER_RADIUS,  0.0f },
        {-PLAYER_RADIUS,  0.0f },
        { 0.0f,  PLAYER_RADIUS },
        { 0.0f, -PLAYER_RADIUS },
        { PLAYER_RADIUS * 0.707f,  PLAYER_RADIUS * 0.707f },
        {-PLAYER_RADIUS * 0.707f,  PLAYER_RADIUS * 0.707f },
        { PLAYER_RADIUS * 0.707f, -PLAYER_RADIUS * 0.707f },
        {-PLAYER_RADIUS * 0.707f, -PLAYER_RADIUS * 0.707f }
    };

     // GUI
     while(!mQuit){
         
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        
        
        glm::vec3 oldPos = camera.mEye;
        
        float deltaTime = GetDeltaTime();

        if(gravity && inAir){

            // Apply gravity to vertical velocity
            mVelocity.y += gravityConstant * deltaTime;
            // Apply velocity to camera position
            camera.mEye += mVelocity * deltaTime;
        }
        
        Input();
        ImGui::Begin("Info panel");
        

        if (collisionMode) {
            glm::vec3 delta = camera.mEye - oldPos;

            camera.mEye = oldPos;

            glm::vec3 feetPos = camera.mEye - glm::vec3(0, PLAYER_HEIGHT, 0); // feet position at base

            float feetHeight = 0.0f;
            float headHeight = PLAYER_HEIGHT * 0.9f;

            // Check X-axis collision
            glm::vec3 posX = feetPos + glm::vec3(delta.x, 0, 0);

            if (!checkHorizontalCollision(posX, 0.0f) && !checkHorizontalCollision(posX, headHeight)) {
                camera.mEye.x += delta.x;
            }

            // Check Y-axis collision (vertical movement)
            glm::vec3 testY = feetPos + glm::vec3(0, delta.y, 0);
            ImGui::Text("Feet Position: (%.1f, %.1f, %.1f)", feetPos.x, feetPos.y, feetPos.z);

            float headOffset = PLAYER_HEIGHT;
            glm::vec3 headPos = testY + glm::vec3(0, headOffset, 0);
            bool collision = false;
            for (const auto& offset : offsets) {
                // Feet check
                glm::vec3 checkFeet = testY + glm::vec3(offset.x, 0.0f, offset.y);
                // Head check
                glm::vec3 checkHead = headPos + glm::vec3(offset.x, 0.0f, offset.y);

                if (checkCollisionAt(checkFeet) || checkCollisionAt(checkHead)) {
                    collision = true;
                    break;
                }
            }

            if (!collision) {
                camera.mEye.y += delta.y;
                inAir = true;
            } 
            else {
                mVelocity.y = 0.0f;
                inAir = false;
                camera.mEye.y = oldPos.y; // snap to surface
            }

            // Check Z-axis collision
            glm::vec3 posZ = feetPos + glm::vec3(0, 0, delta.z);
            if (!checkHorizontalCollision(posZ, 0.0f) && !checkHorizontalCollision(posZ, headHeight)) {
                camera.mEye.z += delta.z;
            }
        }
            
        renderer->RenderVoxels(camera);
        
        camera.fpsControls = gravity;
        
        
        // Begin the Info panel window
            float fps = 1.0f / deltaTime;
            ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", camera.mEye.x, camera.mEye.y, camera.mEye.z);
            ImGui::Text("Camera Speed: %.3f", camera.speed);
            ImGui::Text("FPS: %.3f", fps);
            ImGui::Text("Collision Mode: %s", collisionMode ? "true" : "false");
            ImGui::Checkbox("Toggle collision mode:", &collisionMode);
            ImGui::Checkbox("Toggle gravity:", &gravity);
        ImGui::End();
        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        
        SDL_GL_SwapWindow(mGraphicsApplicationWindow);

    }
}

bool Engine::checkHorizontalCollision(const glm::vec3& centerPos, float yOffset) {
    // Positions to check around the player on XZ plane at height yOffset
    glm::vec3 pointsToCheck[8] = {
        centerPos + glm::vec3( PLAYER_RADIUS, yOffset,  0.0f),
        centerPos + glm::vec3(-PLAYER_RADIUS, yOffset,  0.0f),
        centerPos + glm::vec3(0.0f, yOffset,  PLAYER_RADIUS),
        centerPos + glm::vec3(0.0f, yOffset, -PLAYER_RADIUS),
        centerPos + glm::vec3( PLAYER_RADIUS * 0.7f, yOffset,  PLAYER_RADIUS * 0.7f),
        centerPos + glm::vec3( PLAYER_RADIUS * 0.7f, yOffset, -PLAYER_RADIUS * 0.7f),
        centerPos + glm::vec3(-PLAYER_RADIUS * 0.7f, yOffset,  PLAYER_RADIUS * 0.7f),
        centerPos + glm::vec3(-PLAYER_RADIUS * 0.7f, yOffset, -PLAYER_RADIUS * 0.7f),
    };

    for (auto& point : pointsToCheck) {
        if (checkCollisionAt(point)) return true;
    }
    return false;
}

bool Engine::checkCollisionAt(const glm::vec3& pos) {
    return renderer->isVoxel(pos);
}

float Engine::GetDeltaTime()
{
    static uint64_t lastTime = SDL_GetTicks64();
    uint64_t currentTime = SDL_GetTicks64();
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    return deltaTime;
}

void Engine::Input(){
    SDL_Event e;
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    
    static int mouseX = mScreenWidth/2;
    static int mouseY = mScreenHeight/2;
    
    
    while(SDL_PollEvent(&e) != 0){
        if(e.type == SDL_QUIT){
            mQuit = true;
        }
    
        else if(e.type == SDL_MOUSEMOTION && !lockMouse){
    
            mouseX += e.motion.xrel;
            mouseY += e.motion.yrel;
            camera.MouseLook(mouseX,mouseY);
        
        }
        else if(e.type == SDL_MOUSEWHEEL){
            if(e.wheel.y > 0) {

                if (state[SDL_SCANCODE_LSHIFT])
                    camera.speed += 0.1f;  // Increase the value
                else
                    camera.speed += 0.025f;  // Increase the value
            } 
            else if(e.wheel.y < 0) {
                if (state[SDL_SCANCODE_LSHIFT])
                    camera.speed -= 0.1f;  // Increase the value
                else
                    camera.speed -= 0.025f;  // Increase the value

                if(camera.speed < 0.0)
                    camera.speed = 0.0;  
            }
        }

        // Feed events to ImGui
        ImGui_ImplSDL2_ProcessEvent(&e);
    }

    if (state[SDL_SCANCODE_ESCAPE]){
        mQuit = true;
    }
    if (state[SDL_SCANCODE_W]){
        camera.MoveForward(camera.speed);
    }
    if (state[SDL_SCANCODE_S]){
        camera.MoveBackward(camera.speed);
    }
    if (state[SDL_SCANCODE_A]){
        camera.MoveLeft(camera.speed);
    }
    if (state[SDL_SCANCODE_D]){
        camera.MoveRight(camera.speed);
    }
    if (state[SDL_SCANCODE_SPACE]){
        camera.MoveUp(camera.speed);
    }
    if (state[SDL_SCANCODE_LCTRL]){
        camera.MoveDown(camera.speed);
    }

    //Lock/Unlock mouse to interact with gui
    if (state[SDL_SCANCODE_F]){
        SDL_SetRelativeMouseMode(SDL_TRUE);
        lockMouse = false;
    }
    if (state[SDL_SCANCODE_G]){
        SDL_SetRelativeMouseMode(SDL_FALSE);
        lockMouse = true;
        SDL_WarpMouseInWindow(mGraphicsApplicationWindow, mScreenWidth/2,mScreenHeight/2);
    }
}


