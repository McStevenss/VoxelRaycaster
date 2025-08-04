#ifndef CAMERA_HPP
#define CAMERA_HPP
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Camera{
    public:

    //Constructor
    Camera();
    void SetProjectionMatrix(float fovy, float aspect, float near, float far);
    glm::mat4 GetProjectionMatrix() const;
    std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view);
    //The final view matrix we return
    glm::mat4 GetViewMatrix() const;
    glm::vec3 GetViewDirection() const;
    float GetHorizontalFov(int screenW, int screenH) const;
    void MoveForward(float speed);
    void MoveBackward(float speed);
    void MoveLeft(float speed);
    void MoveRight(float speed);
    void MoveUp(float speed);
    void MoveDown(float speed);
    void MouseLook(int mouseX, int mouseY);
    glm::vec3 mEye;
    float speed = 0.025f;
    float mFovy;
    bool fpsControls = false;
    glm::vec3 mUpVector;
    glm::vec3 mRightVector;
    glm::vec3 mViewDirection;

    float mNearPlane;
    float mFarPlane;
    float mAspectRatio;

    private:
        glm::mat4 mProjectionMatrix;
        glm::vec2 mOldMousePosition;
        float pitch = 0.0f;

};


#endif