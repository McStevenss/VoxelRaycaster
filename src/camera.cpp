#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "Camera.hpp"


Camera::Camera()
{
    //Default position
    mEye = glm::vec3(0.0f, 0.0f, 0.0f);  

    // Assume we're looking out into the world (negative -Z)
    mViewDirection = glm::vec3(0.0f,0.0f,1.0f);

    //Assume we start on a perfect plane and up means Y axis
    mUpVector    = glm::vec3(0.0f,1.0f,0.0f);
    mRightVector = glm::vec3(1.0f,0.0f,0.0f);
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(mEye, mEye + mViewDirection, mUpVector);
}

glm::vec3 Camera::GetViewDirection() const
{
    return mViewDirection;
}

void Camera::SetProjectionMatrix(float fovy, float aspect, float near, float far)
{
    mFovy = fovy;
    mFarPlane = far;
    mNearPlane = near;
    mAspectRatio = aspect;
    mProjectionMatrix = glm::perspective(glm::radians(mFovy), mAspectRatio, mNearPlane, mFarPlane);
};

glm::mat4 Camera::GetProjectionMatrix() const
{
    return mProjectionMatrix;
};

void Camera::MouseLook(int mouseX, int mouseY)
{
    glm::vec2 currentMouse = glm::vec2(mouseX,mouseY);

    static bool firstLook=true;
    if(firstLook){
        mOldMousePosition=currentMouse;
        firstLook=false;
    }
    glm::vec2 deltaMousePosition = (mOldMousePosition - currentMouse) * 0.1f;
    glm::vec3 localRightDirection = glm::cross(mViewDirection, mUpVector);

    //NOTE: Rotating each direction on its own to avoid gimbal-lock
    // Vertical rotation (pitch): Rotate around the localRightDirection
    float currentPitch = glm::degrees(glm::asin(mViewDirection.y)); // Calculate current pitch from mViewDirection
    float newPitch = glm::clamp(currentPitch + deltaMousePosition.y, -89.0f, 89.0f); // Clamp pitch between -89 and 89
    float deltaPitch = newPitch - currentPitch;
   
    mViewDirection = glm::rotate(mViewDirection, glm::radians(deltaPitch), localRightDirection);

    // Horizontal rotation (yaw): Rotate around the global up vector
    mViewDirection = glm::rotate(mViewDirection, glm::radians(deltaMousePosition.x), mUpVector);


    // Normalize mViewDirection to prevent floating-point drift / unessesary scaling vectors
    mViewDirection = glm::normalize(mViewDirection);

    mOldMousePosition = currentMouse;

}

void Camera::MoveForward(float speed)
{
    if(fpsControls){
        glm::vec3 forward = glm::normalize(glm::vec3(mViewDirection.x, 0.0f, mViewDirection.z));
        mEye += forward * speed;
    }
    else{
        mEye +=(mViewDirection * speed);
    }
}

void Camera::MoveBackward(float speed)
{
    if(fpsControls){
        glm::vec3 forward = glm::normalize(glm::vec3(mViewDirection.x, 0.0f, mViewDirection.z));
        mEye -= forward * speed;
    }
    else{
        mEye -= (mViewDirection * speed);
    }
}

void Camera::MoveLeft(float speed)
{
    if(fpsControls){
        glm::vec3 forward = glm::normalize(glm::vec3(mViewDirection.x, 0.0f, mViewDirection.z));
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        mEye -= right * speed;
    }
    else{
        glm::vec3 right = glm::normalize(glm::cross(mViewDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        mEye -= right * speed;
    }
}

void Camera::MoveRight(float speed)
{
    if(fpsControls){
        glm::vec3 forward = glm::normalize(glm::vec3(mViewDirection.x, 0.0f, mViewDirection.z));
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        mEye += right * speed;
    }
    else{
        glm::vec3 right = glm::normalize(glm::cross(mViewDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        mEye += right * speed;
    }
}

void Camera::MoveUp(float speed)
{
    mEye +=  (mUpVector * speed);
}

void Camera::MoveDown(float speed)
{
    mEye -=  (mUpVector * speed);
}
