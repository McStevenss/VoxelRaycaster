#ifndef SIMPLECUBE_H
#define SIMPLECUBE_H
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>
#include <Shader.hpp>
class SimpleCube{
    public:
        float vertices[216]; 
        
        SimpleCube(glm::vec3 position, glm::vec3 scale);
        void Render(Shader &shader);

        glm::mat4 GetModelMatrix();
        glm::vec3 GetPosition();

        unsigned int VBO, VAO, EBO;
    private:
        glm::mat4 model;
        glm::vec3 mPosition;
       
};


#endif