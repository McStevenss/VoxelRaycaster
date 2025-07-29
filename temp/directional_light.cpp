#include "directional_light.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>

DirectionalLight::DirectionalLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 direction)
{
    mPosition = position;
    mAmbient = ambient;
    mDiffuse = diffuse;
    mSpecular = specular;
    mDirection = direction;
    
    near_plane = 0.01f;
    far_plane = 30.5f;
    
    mLightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);  
    mLightView = glm::lookAt(mPosition, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = mLightProjection * mLightView; 
}
