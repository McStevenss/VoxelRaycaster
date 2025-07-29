#ifndef QUAD_HPP
#define QUAD_HPP
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>
#include <Shader.hpp>
#include <stb_image.h>

class Quad{
    public:
        float vertices[32]; 
        unsigned int indices[6]; // 2 triangles, 3 indices each
        unsigned int VBO, VAO, EBO;
        
        glm::vec3 mPosition;

        Quad(glm::vec3 position);
        Quad(glm::vec3 position, float advanced);

        void Render(Shader &shader, bool useParallax);
        int LoadTexture(const char *texture_Path);
        glm::mat4 GetModelMatrix();

    private:
        int mNormal;
        int mDisplacement;
        int mDiffuse;
        glm::mat4 model;
       
};


#endif