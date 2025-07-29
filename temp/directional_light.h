#ifndef DIRECTIONALLIGHT_H
#define DIRECTIONALLIGHT_H
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>

class DirectionalLight{
    public:
        
        DirectionalLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 direction);

        glm::vec3 mPosition;
        glm::vec3 mAmbient;
        glm::vec3 mDiffuse;
        glm::vec3 mSpecular;
        glm::vec3 mDirection;

        float near_plane, far_plane;
        glm::mat4 mLightProjection;
        glm::mat4 mLightView;
        glm::mat4 lightSpaceMatrix;
    private:

       
};


#endif