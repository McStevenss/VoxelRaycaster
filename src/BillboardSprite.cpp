#include "BillboardSprite.h"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "Shader.hpp"

//Initialize static/global variables for all sprites/billboards
GLuint BillboardSprite::VAO = 0;
GLuint BillboardSprite::VBO = 0;
Shader* BillboardSprite::sShader = nullptr;
GLuint BillboardSprite::sTexture = 0;
glm::vec2 BillboardSprite::sUvScale = glm::vec2(0.0f, 0.0f);

BillboardSprite::BillboardSprite(glm::vec3 pos, float s, glm::vec2 spriteOffset)
: position(glm::vec3(pos.x + 0.5f, pos.y, pos.z + 0.5f)), size(s), uvOffset(spriteOffset) {}


void BillboardSprite::InitBuffers() {
    if (VAO != 0) return; // already initialized

    float quadVertices[] = {
        // positions      // texcoords
        -0.5f, 0.0f, 0.0f,  0.0f, 0.0f,
         0.5f, 0.0f, 0.0f,  1.0f, 0.0f,
        -0.5f, 1.0f, 0.0f,  0.0f, 1.0f,

        -0.5f, 1.0f, 0.0f,  0.0f, 1.0f,
         0.5f, 0.0f, 0.0f,  1.0f, 0.0f,
         0.5f, 1.0f, 0.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texcoords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void BillboardSprite::SetShader(Shader* shader) {
    sShader = shader;
}

void BillboardSprite::SetTexture(GLuint texture, glm::vec2 uvScale) {
    sTexture = texture;
    sUvScale = uvScale;
}

glm::mat4 BillboardSprite::ComputeCylindricalBillboardMatrix(const glm::vec3& spritePos,
                                            const glm::vec3& cameraPos,
                                            float spriteSize)
{
    glm::mat4 model(1.0f);

    // 1. Translate to sprite position
    model = glm::translate(model, spritePos);

    // 2. Compute horizontal direction to camera
    glm::vec3 look = cameraPos - spritePos;
    look.y = 0.0f;
    if (glm::length(look) > 0.0001f)
        look = glm::normalize(look);

    // 3. Compute rotation about Y-axis
    float angle = atan2(look.x, look.z);
    model = glm::rotate(model, angle, glm::vec3(0, 1, 0));

    // 4. Scale
    model = glm::scale(model, glm::vec3(spriteSize));

    return model;
}

void BillboardSprite::Draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos) 
{
    if (!sShader) return; // safety check

    glm::mat4 model = ComputeCylindricalBillboardMatrix(position, cameraPos, size);

    sShader->use();
    sShader->setMat4("model", model);
    sShader->setMat4("view", view);
    sShader->setMat4("projection", projection);
    sShader->setVec2("uvOffset", uvOffset);
    sShader->setVec2("uvScale", sUvScale);
    sShader->setInt("spriteTexture", 0); // Texture bound to GL_TEXTURE0

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sTexture);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}