#include "terrain.h"
#include <stdio.h>
#include <iostream>
#include <stb_image.h>

std::vector<float> Terrain::GenerateGridVertices(int width, int height, float cellSize) {
    std::vector<float> vertices;

    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            float posX = x * cellSize;
            float posZ = z * cellSize;
           

            float u = x / (float)(width-1);    // Normalize x position across the entire grid
            float v = z / (float)(height-1);  // Normalize z position across the entire grid


            vertices.push_back(posX); // x
            vertices.push_back(0.0f); // y (initial height)
            vertices.push_back(posZ); // z
            vertices.push_back(u); // z
            vertices.push_back(v); // z
        }
    }

    return vertices;
}

std::vector<unsigned int> Terrain::GenerateGridIndices(int width, int height) {
    std::vector<unsigned int> indices;

    for (int z = 0; z < height-1; ++z) {
        for (int x = 0; x < width-1; ++x) {
            int topLeft = z * width + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * width + x;
            int bottomRight = bottomLeft + 1;

            // First triangle
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Second triangle
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);

            mIndexCount += 6;

        }
    }

    return indices;
}

std::vector<float> Terrain::ApplyHeightmapToGrid(const char* heightmapPath, std::vector<float>& vertices, int gridWidth, int gridHeight) {
    int width, height, channels;
    unsigned char* heightmap = stbi_load(heightmapPath, &width, &height, &channels, 1); // Load as grayscale

    if (!heightmap) {
        std::cout << "Failed to load heightmap" << std::endl;
        return vertices;
    }

    for (int z = 0; z < gridHeight; ++z) {
        for (int x = 0; x < gridWidth; ++x) {
            int index = (z * gridWidth + x) * 5; // Each vertex has 5 components (x, y, z, u, v)

            int heightmapIndex = (z * width + x); // Assume heightmap matches grid size

            float heightValue = heightmap[heightmapIndex] / 255.0f; // Normalize height
            vertices[index + 1] = heightValue * 20.0f; // Set y value (scale height)
        }
    }

    stbi_image_free(heightmap);
    return vertices;
}


void Terrain::Render(Shader& shader)
{
    shader.setMat4("model", model);
    shader.use();

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Terrain::Terrain(const char* heightmapPath, int width, int height, float cellsize, glm::vec3 position)
{
    mHeightmapPath = heightmapPath;
    mWidth = width;
    mHeight = height;
    mCellSize = cellsize;
    mName = "[Terrain]";
    mIndexCount = 0;
    mPosition = position;

    model = glm::translate(model,mPosition);

    std::cout << mName << " Creating Grid Vertices..." << std::endl;
    mVertices = GenerateGridVertices(mWidth, mHeight, mCellSize);
    
    std::cout << mName << " Creating Grid Indices..." << std::endl;
    mIndices = GenerateGridIndices(mWidth, mHeight);

    mVertices = ApplyHeightmapToGrid(mHeightmapPath, mVertices, mWidth, mHeight);


    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(float), &mVertices[0], GL_STATIC_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(unsigned int), &mIndices[0], GL_STATIC_DRAW);

    // Define vertex attribute layout
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

        //Give specification for location 1, colors
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}