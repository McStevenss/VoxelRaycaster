#ifndef GUI_H
#define GUI_H
#include "scene.h"
#include "renderer.h"
class Gui {
    public:

        void render(Scene& scene, Renderer* renderer, Camera &camera);
        bool mMouseActive = true;

    private:
    
        float GetDeltaTime();
        std::vector<float> frameTimes;
        const int maxSamples = 100;

        void AddTitle(const char* title);


};


#endif