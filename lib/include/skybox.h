#ifndef SKYBOX_H
#define SKYBOX_H
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <stdio.h>
#include <iostream>
// #include <vector>

class SkyBox{
    public:

        float vertices[108];

        SkyBox();
        unsigned int loadCubemap(std::vector<std::string> faces);

        unsigned int cubemapTexture;
        unsigned int VAO, VBO;
    private:
};


#endif