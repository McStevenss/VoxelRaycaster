#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Shader.hpp"


class BillboardSprite {
public:
    glm::vec3 position;
    float size;

    BillboardSprite(glm::vec3 pos, float s, glm::vec2 spriteOffset);
    glm::mat4 ComputeCylindricalBillboardMatrix(const glm::vec3& spritePos, const glm::vec3& cameraPos, float spriteSize);

    
    static void InitBuffers(); // Initializes VAO/VBO for a simple quad if not already
    static void SetShader(Shader* shader); // Set global shader for all billboards/sprites
    // static void SetTexture(GLuint texture); // Set tilemap/spritesheet for all billboards/sprites
    static void SetTexture(GLuint texture, glm::vec2 uvScale); // Set tilemap/spritesheet for all billboards/sprites

    glm::vec2 uvOffset; // Bottom-left of tile in normalized UV
    // glm::vec2 uvScale;  // Tile size in normalized UV

    // Draw the sprite given a view/projection matrix
    void Draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos);


private:
    static GLuint VAO, VBO;
    static Shader* sShader;
    static GLuint sTexture; // Shared spritesheet texture
    static glm::vec2 sUvScale;  // Tile size in normalized UV
};