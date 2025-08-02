#pragma once

#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Camera.hpp"
#include "Shader.hpp"
#include "VoxelTerrain.h"

class Player {
    public:
        Player(glm::vec3 position, int ScreenWidth, int ScreenHeight, SDL_Window* GraphicsApplicationWindow);
        // void Update(float deltaTime, VoxelTerrain &terrain);
        void Update(float deltaTime, VoxelTerrain *terrain);
        bool checkHorizontalCollision(const glm::vec3& centerPos, float yOffset, VoxelTerrain *terrain);
        int getChosenBlock();
        void Input();
        
        // UI 
        bool mQuit = false; 
        bool lockMouse = false;
        bool mGravity = false;
        bool inAir = true;
        bool collisionMode = true;
        Camera mCamera;
        glm::vec3 mPosition;
              
    private:
        SDL_Window* mGraphicsApplicationWindow;
        const float PLAYER_HEIGHT = 0.65f;
        const float PLAYER_RADIUS = 0.25f;

        int mScreenWidth;
        int mScreenHeight;

        bool removeBlock = false;
        bool addBlock = false;
        int mChosenBlock = 0;

        const glm::vec2 offsets[8] = {
            { PLAYER_RADIUS,  0.0f },
            {-PLAYER_RADIUS,  0.0f },
            { 0.0f,  PLAYER_RADIUS },
            { 0.0f, -PLAYER_RADIUS },
            { PLAYER_RADIUS * 0.707f,  PLAYER_RADIUS * 0.707f },
            {-PLAYER_RADIUS * 0.707f,  PLAYER_RADIUS * 0.707f },
            { PLAYER_RADIUS * 0.707f, -PLAYER_RADIUS * 0.707f },
            {-PLAYER_RADIUS * 0.707f, -PLAYER_RADIUS * 0.707f }
        };

        const float gravityConstant = -9.81f; // meters per second squared
        glm::vec3 mVelocity = glm::vec3(0.0f); // Add this to your Camera class
        int jumpVelocity = 5;
        

        bool gKeyPreviouslyPressed = false;
        bool mb_one_PreviouslyPressed = false;
        bool mb_two_PreviouslyPressed = false;

};