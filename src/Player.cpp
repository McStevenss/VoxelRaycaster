#include "Player.h"
#include <SDL2/SDL.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl3.h"
#include "VoxelTerrain.h"


Player::Player(glm::vec3 position, int ScreenWidth, int ScreenHeight, SDL_Window* GraphicsApplicationWindow)
{
    mPosition = position;
    mScreenWidth = ScreenWidth;
    mScreenHeight = ScreenHeight;
    mGraphicsApplicationWindow = GraphicsApplicationWindow;
    mCamera.SetProjectionMatrix(90.0f,(float)ScreenWidth/(float)ScreenHeight,0.1,1000);
    mCamera.mEye = mPosition;

}

void Player::Update(float deltaTime, VoxelTerrain *terrain)
{
    mPosition = mCamera.mEye;
    glm::vec3 oldPos = mPosition;
        
    Input();

    if(shouldRemove){
        float voxelRGB[4];
        glReadPixels(mScreenWidth / 2, mScreenHeight / 2, 1, 1, GL_RGBA, GL_FLOAT, voxelRGB);
        glm::ivec3 voxelXYZ = glm::floor(glm::vec3(voxelRGB[0], voxelRGB[1], voxelRGB[2]) * float(terrain->VoxelWorldSize));
        terrain->setVoxel(voxelXYZ.x+1, voxelXYZ.y+1, voxelXYZ.z, 0);
        terrain->updateVoxelGPU(voxelXYZ.x+1, voxelXYZ.y+1, voxelXYZ.z);

        shouldRemove = false;
    }
    
    inAir = true;
    if(gravity)
    {
        mVelocity.y += gravityConstant * deltaTime;
        mCamera.mEye += mVelocity * deltaTime;
    }    

    if (collisionMode) 
    {
        glm::vec3 delta = mCamera.mEye - oldPos;
        mCamera.mEye = oldPos;
        glm::vec3 feetPos = mCamera.mEye - glm::vec3(0, PLAYER_HEIGHT, 0); // feet position at base

        
        // Check X-axis collision
        glm::vec3 posX = feetPos + glm::vec3(delta.x, 0, 0);
        if (!checkHorizontalCollision(posX, 0.0f, terrain) && !checkHorizontalCollision(posX, PLAYER_HEIGHT, terrain)) 
        {
            mCamera.mEye.x += delta.x;
        }

        // Check Z-axis collision
        glm::vec3 posZ = feetPos + glm::vec3(0, 0, delta.z);
        if (!checkHorizontalCollision(posZ, 0.0f, terrain) && !checkHorizontalCollision(posZ, PLAYER_HEIGHT, terrain)) 
        {
            mCamera.mEye.z += delta.z;
        }

        // Check Y-axis collision (vertical movement)
        glm::vec3 testY = feetPos + glm::vec3(0, delta.y, 0);

        float headOffset = PLAYER_HEIGHT;
        glm::vec3 headPos = testY + glm::vec3(0, PLAYER_HEIGHT, 0);

        bool feetCollision = false;
        bool headCollision = false;

        for (const auto& offset : offsets) {
            glm::vec3 checkFeet = testY + glm::vec3(offset.x, 0.0f, offset.y);
            glm::vec3 checkHead = headPos + glm::vec3(offset.x, 0.0f, offset.y);

            if (terrain->isVoxel(checkFeet))
                feetCollision = true;
            if (terrain->isVoxel(checkHead))
                headCollision = true;

            if (feetCollision || headCollision) break;
        }

        //We need to handle collisions differently for head/feet.
        if (delta.y > 0.0f) 
        { 
            if (headCollision) // Hit ceiling, stop
            {
                mVelocity.y = 0.0f;
                inAir = true;
                mCamera.mEye.y = oldPos.y; // snap to just below ceiling
            }
            else 
            {
                mCamera.mEye.y += delta.y;
                inAir = true;
            }
        } 
        else if (delta.y < 0.0f) 
        { 
           
            if (feetCollision)  // Landed
            {
                mVelocity.y = 0.0f;
                inAir = false;
                mCamera.mEye.y = oldPos.y; // snap to ground
            } 
            else
            {
                mCamera.mEye.y += delta.y;
                inAir = true;
            }
        }
    }

    mCamera.fpsControls = gravity;
}

bool Player::checkHorizontalCollision(const glm::vec3& centerPos, float yOffset, VoxelTerrain *terrain) 
{
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
        if (terrain->isVoxel(point)) return true;
    }
    return false;
}

void Player::Input()
{
    SDL_Event e;
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    
    static int mouseX = mScreenWidth/2;
    static int mouseY = mScreenHeight/2;
    
    
    while(SDL_PollEvent(&e) != 0)
    {
        if(e.type == SDL_QUIT)
        {
            mQuit = true;
        }
        else if(e.type == SDL_MOUSEMOTION && !lockMouse)
        {
            mouseX += e.motion.xrel;
            mouseY += e.motion.yrel;
            mCamera.MouseLook(mouseX,mouseY);
        }
        else if(e.type == SDL_MOUSEWHEEL)
        {
            if(e.wheel.y > 0)
            {
                if (state[SDL_SCANCODE_LSHIFT])
                    mCamera.speed += 0.1f;
                else
                    mCamera.speed += 0.025f;
            } 
            else if(e.wheel.y < 0) 
            {
                if (state[SDL_SCANCODE_LSHIFT])
                    mCamera.speed -= 0.1f;
                else
                    mCamera.speed -= 0.025f;

                if(mCamera.speed < 0.0)
                    mCamera.speed = 0.0;  
            }
        }
        else if(e.type == SDL_MOUSEBUTTONUP && !lockMouse)
        {
            shouldRemove = true;
        }

        // Feed events to ImGui
        ImGui_ImplSDL2_ProcessEvent(&e);
    }

    if (state[SDL_SCANCODE_ESCAPE])
    {
        mQuit = true;
    }
    if (state[SDL_SCANCODE_W])
    {
        mCamera.MoveForward(mCamera.speed);
    }
    if (state[SDL_SCANCODE_S])
    {
        mCamera.MoveBackward(mCamera.speed);
    }
    if (state[SDL_SCANCODE_A])
    {
        mCamera.MoveLeft(mCamera.speed);
    }
    if (state[SDL_SCANCODE_D])
    {
        mCamera.MoveRight(mCamera.speed);
    }
    if (state[SDL_SCANCODE_SPACE]) 
    {
        if (gravity) 
        {
            if (!inAir) 
            {
                mVelocity.y = jumpVelocity;
                inAir = true;
            }
        } 
        else 
        {
            mCamera.MoveUp(mCamera.speed); // Free-fly mode without gravity
        }
    }

    if (state[SDL_SCANCODE_LCTRL]){
        mCamera.MoveDown(mCamera.speed);
    }

    //Lock/Unlock mouse to interact with gui
    if (state[SDL_SCANCODE_F])
    {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        lockMouse = false;
    }
    if (state[SDL_SCANCODE_G])
    {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        lockMouse = true;
        SDL_WarpMouseInWindow(mGraphicsApplicationWindow, mScreenWidth/2,mScreenHeight/2);
    }
 }