#include "transform.h"

glm::mat4 Transform::getLocalModelMatrix()
{
    const glm::mat4 transformX = glm::rotate(glm::mat4(1.0f),
                        glm::radians(Rotation.x),
                        glm::vec3(1.0f, 0.0f, 0.0f));
    const glm::mat4 transformY = glm::rotate(glm::mat4(1.0f),
                        glm::radians(Rotation.y),
                        glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 transformZ = glm::rotate(glm::mat4(1.0f),
                        glm::radians(Rotation.z),
                        glm::vec3(0.0f, 0.0f, 1.0f));

    // Y * X * Z
    const glm::mat4 roationMatrix = transformY * transformX * transformZ;

    // translation * rotation * scale (also know as TRS matrix)
    return glm::translate(glm::mat4(1.0f), Position) *
                roationMatrix *
                glm::scale(glm::mat4(1.0f), Scale);
}

void Transform::computeModelMatrix()
{
    mModelMatrix = getLocalModelMatrix();
    mIsDirty = false;
}

void Transform::computeModelMatrix(const glm::mat4& parentGlobalModelMatrix)
{
    mModelMatrix = parentGlobalModelMatrix * getLocalModelMatrix();
    mIsDirty = false;
}

void Transform::setLocalPosition(const glm::vec3& newPosition, bool isInitial)
{
    Position = newPosition;
    mIsDirty = true;
    if (isInitial)
        initialPosition = newPosition;
}

void Transform::setLocalRotation(const glm::vec3& newRotation, bool isInitial)
{
    Rotation = newRotation;
    mIsDirty = true;
    if(isInitial)
        initialRotation = newRotation;
}

void Transform::setLocalScale(const glm::vec3& newScale, bool isInitial)
{
    Scale = newScale;
    mIsDirty = true;

    if(isInitial)
        initialScale = newScale;
}


void Transform::resetLocalPosition()
{
    Position = initialPosition;
    mIsDirty = true;
}
void Transform::resetLocalRotation()
{
    Rotation = initialRotation;
    mIsDirty = true;
}
void Transform::resetLocalScale()
{
    Scale = initialScale;
    mIsDirty = true;
}


//| ModelMatrix[0] | → Right vector (X-axis direction)
//| ModelMatrix[1] | → Up vector (Y-axis direction)
//| ModelMatrix[2] | → Forward vector (Z-axis direction)
//| ModelMatrix[3] | → Translation (Position in world space)

const glm::vec3& Transform::getLocalPosition() const
{
    return Position;
}

const glm::vec3& Transform::getLocalRotation() const
{
    return Rotation;
}

const glm::vec3& Transform::getLocalScale() const
{
    return Scale;
}

const glm::mat4& Transform::getModelMatrix() const
{
    return mModelMatrix;
}

glm::vec3 Transform::getRight() const
{
    return mModelMatrix[0];
}


glm::vec3 Transform::getUp() const
{
    return mModelMatrix[1];
}

glm::vec3 Transform::getBackward() const
{
    return mModelMatrix[2];
}

glm::vec3 Transform::getForward() const
{
    return -mModelMatrix[2];
}

glm::vec3 Transform::getGlobalScale() const
{
    return { glm::length(getRight()), glm::length(getUp()), glm::length(getBackward()) };
}

bool Transform::isDirty()
{
    return mIsDirty;
}