#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP


#include "glm/glm.hpp"
#include "model.hpp"
#include <stdio.h>
#include <iostream>
#include <list>
#include <memory>

class Transform{
    public:
    /*SPACE INFORMATION*/
    //Local space information
    glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
    //Euler rotation
    glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
    glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

    //Global space information concatenate in matrix
    glm::mat4 mModelMatrix = glm::mat4(1.0f);

    bool mIsDirty = true;

        glm::mat4 getLocalModelMatrix();
        void computeModelMatrix();
        void computeModelMatrix(const glm::mat4& parentGlobalModelMatrix);
        void setLocalPosition(const glm::vec3& newPosition, bool isInitial=false);
        void setLocalRotation(const glm::vec3& newRotation, bool isInitial=false);
        void setLocalScale(const glm::vec3& newScale, bool isInitial=false);

        void resetLocalPosition();
        void resetLocalRotation();
        void resetLocalScale();
        // void setLocalScale(const glm::vec3& newScale);
        // const glm::vec3& getGlobalPosition();
        const glm::vec3& getLocalPosition() const;
        const glm::vec3& getLocalRotation() const;
        const glm::vec3& getLocalScale() const;

        const glm::mat4& getModelMatrix() const;
        glm::vec3 getRight() const;
        glm::vec3 getUp() const;
        glm::vec3 getBackward() const;
        glm::vec3 getForward() const;
        glm::vec3 getGlobalScale() const;
        bool isDirty();

    private:

        //Save initial in case we want to reset an object
        glm::vec3 initialPosition = { 0.0f, 0.0f, 0.0f };
        //Euler rotation
        glm::vec3 initialRotation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 initialScale = { 1.0f, 1.0f, 1.0f };
};

#endif