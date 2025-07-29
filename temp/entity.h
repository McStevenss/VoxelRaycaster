#ifndef ENTITY_HPP
#define ENTITY_HPP
// #include "model.h"
#include <glm/glm.hpp> //glm::mat4
#include <stdio.h>
#include <iostream>
#include <list>
#include <array> //std::array
#include <memory> //std::unique_ptr
#include "transform.h"
#include "scene_node.h"
#include "BoneAnimation.hpp"
#include "BoneAnimator.hpp"


class Entity : public Node, public Model
{
    public:

    Animator* animator = nullptr;
    Animation* animation = nullptr;
    // Constructor with given name
    Entity(std::string const& name, std::string const& path, bool gamma = false, bool flipuvs = true, bool is_animated = false) 
    : Node(name), Model(path, gamma, flipuvs, is_animated) 
    {
        if(is_animated){
            animation = new Animation(animation_path,this); 
            animator = new Animator(animation);
        }
    }
    
    // Overloaded constructor where the name will use the default constructor from Node
    Entity(std::string const& path, bool gamma = false, bool flipuvs = true) 
    : Node(), Model(path, gamma, flipuvs, false) {}
    
    
    std::array<glm::vec3, 8> getWorldBoundingBoxCorners()
        {
            glm::vec3 min = aabbMin;
            glm::vec3 max = aabbMax;
        
            glm::mat4 modelMatrix = transform.getModelMatrix();

            return {
                glm::vec3(modelMatrix * glm::vec4(min.x, min.y, min.z, 1.0f)),
                glm::vec3(modelMatrix * glm::vec4(max.x, min.y, min.z, 1.0f)),
                glm::vec3(modelMatrix * glm::vec4(max.x, min.y, max.z, 1.0f)),
                glm::vec3(modelMatrix * glm::vec4(min.x, min.y, max.z, 1.0f)),
        
                glm::vec3(modelMatrix * glm::vec4(min.x, max.y, min.z, 1.0f)),
                glm::vec3(modelMatrix * glm::vec4(max.x, max.y, min.z, 1.0f)),
                glm::vec3(modelMatrix * glm::vec4(max.x, max.y, max.z, 1.0f)),
                glm::vec3(modelMatrix * glm::vec4(min.x, max.y, max.z, 1.0f))
            };
        }

};
#endif