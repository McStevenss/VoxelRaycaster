#ifndef POINTLIGHT_H
#define POINTLIGHT_H
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>

class PointLight{
    public:
        
        PointLight(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 position);

        glm::vec3 mPosition;  
        glm::vec3 mAmbient;
        glm::vec3 mDiffuse;
        glm::vec3 mSpecular;
        
        float constant;
        float linear;
        float quadratic;

    private:

       
};


#endif