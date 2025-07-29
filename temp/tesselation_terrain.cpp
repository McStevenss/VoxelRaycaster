#include "tesselation_terrain.h"
#include <stdio.h>
#include <iostream>
#include <stb_image.h>
#include <Shader.hpp>

TesselationTerrain::TesselationTerrain(const char *HeightmapPath, Shader &shader)
{
    int channels;

    mHeightmapPath = HeightmapPath;

    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char *data = stbi_load("textures/iceland_heightmap.png", &mWidth, &mHeight, &channels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        shader.setInt("heightMap", 0);
        std::cout << "Loaded heightmap of size " << mHeight << " x " << mWidth << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture in tessellation" << std::endl;
    }
    stbi_image_free(data);

    GenerateGridVertices();

    std::cout << "got texture id" << texture << std::endl;

    // first, configure the cube's VAO (and terrainVBO)
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mVertices.size(), &mVertices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texCoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glPatchParameteri(GL_PATCH_VERTICES, NUM_PATCH_PTS);


}

void TesselationTerrain::GenerateGridVertices()
{
    for(unsigned i = 0; i <= rez-1; i++)
    {
        for(unsigned j = 0; j <= rez-1; j++)
        {
            mVertices.push_back(-mHeight/2.0f + mHeight*j/(float)rez); // v.z
            mVertices.push_back(0.0f); // v.y
            mVertices.push_back(-mWidth/2.0f + mWidth*i/(float)rez); // v.x
            mVertices.push_back(i / (float)rez); // u
            mVertices.push_back(j / (float)rez); // v

            mVertices.push_back(-mHeight/2.0f + mHeight*j/(float)rez); // v.z
            mVertices.push_back(0.0f); // v.y
            mVertices.push_back(-mWidth/2.0f + mWidth*(i+1)/(float)rez); // v.x
            mVertices.push_back((i+1) / (float)rez); // u
            mVertices.push_back(j / (float)rez); // v

            mVertices.push_back(-mHeight/2.0f + mHeight*(j+1)/(float)rez); // v.z
            mVertices.push_back(0.0f); // v.y
            mVertices.push_back(-mWidth/2.0f + mWidth*i/(float)rez); // v.x
            mVertices.push_back(i / (float)rez); // u
            mVertices.push_back((j+1) / (float)rez); // v

            mVertices.push_back(-mHeight/2.0f + mHeight*(j+1)/(float)rez); // v.z
            mVertices.push_back(0.0f); // v.y
            mVertices.push_back(-mWidth/2.0f + mWidth*(i+1)/(float)rez); // v.x
            mVertices.push_back((i+1) / (float)rez); // u
            mVertices.push_back((j+1) / (float)rez); // v
        }
    }
    std::cout << "Loaded " << rez*rez << " patches of 4 control points each" << std::endl;
    std::cout << "Processing " << rez*rez*4 << " vertices in vertex shader" << std::endl;
}

void TesselationTerrain::Render(Shader &shader)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);  // Bind the heightmap texture to GL_TEXTURE0

    glBindVertexArray(VAO);
    // std::cout << "patches" << NUM_PATCH_PTS << " rez "<< rez << std::endl;

    glDrawArrays(GL_PATCHES, 0, NUM_PATCH_PTS * rez * rez);

}
