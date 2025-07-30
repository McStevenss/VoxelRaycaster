
//For windows needs to be defined first of all
#define SDL_MAIN_HANDLED
// #define SDL_VIDEO_DRIVER_X11 1
#define SDL_VIDEO_DRIVER_WINDOWS 1

#include <Engine.h>

//############### For Linux #########################
//[DEPR]c++ src/*.cpp  -I lib/include/ -o bin/Game -lSDL2 -ldl -lassimp


//c++ src/*.cpp lib/build/*.o -I lib/include/ -o bin/Game -lSDL2 -ldl -lassimp


//**NEW**
//c++ src/*.cpp lib/build/*.o -I lib/include -Wl,-rpath=$PWD/lib/build lib/build/libassimp.so -lSDL2 -ldl -o bin/Game



//################## For Windows #########################
//[DEPR]g++ src/*.cpp -I lib/include/ -o bin/Game.exe -lSDL2 -lopengl32 -lgdi32 -lwinmm -luser32 -mwindows -lassimp

//g++ src/*.cpp lib/include/imgui/*.cpp -I lib/include/ -o bin/Game.exe -lSDL2 -lopengl32 -lgdi32 -lwinmm -luser32 -mwindows -lassimp

// For windows WITH console output
// g++ src/*.cpp lib/include/imgui/*.cpp -I lib/include/ -o bin/Game.exe -lSDL2 -lopengl32 -lgdi32 -lwinmm -luser32 -lassimp

// Console output + "Release Mode"
// g++ src/*.cpp lib/include/imgui/*.cpp -I lib/include/ -o bin/Game.exe -lSDL2 -lopengl32 -lgdi32 -lwinmm -luser32 -lassimp -O2 -DNDEBUG

//################################################################
//######## This is the entrypoint of this render engine ##########
//################################################################

int main(){

    // Create Engine
    Engine gameEngine;

    // Start Loop
    gameEngine.MainLoop();    

    return 0;
}