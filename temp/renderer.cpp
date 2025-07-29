#include "renderer.h"
#include <SDL2/SDL.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl3.h"
#include <stdio.h>
#include <iostream>
#include <random>
#include "Shader.hpp"
#include "scene.h"
#include "BufferManager.h"



Renderer::Renderer(int ScreenWidth, int ScreenHeight, Camera* EngineCamera)
{
  
    try {
        std::cout << "[Renderer] Created!" << std::endl;
        screenWidth = ScreenWidth;
        screenHeight = ScreenHeight;

        //Point to camera in engine
        camera = EngineCamera;
        //Initialize camera, set FOV, Aspect Ratio, near-, far-plane
        // camera->SetProjectionMatrix(90.0f,(float)screenWidth/(float)screenHeight,0.1f,1000.0f);
        camera->SetProjectionMatrix(90.0f,(float)screenWidth/(float)screenHeight,cameraNearPlane,cameraFarPlane);
        PrepareForRender();

    } catch (const std::exception& e) {
        std::cout << "[Renderer] Error: " << e.what() << std::endl;
    }

}

Renderer::~Renderer()
{
    std::cout << "[Renderer] Cleaning up ShaderManager...";
    delete shaderManager;
    std::cout << "Done!" << std::endl;
    
    std::cout << "[Renderer] Cleaning up BufferManager...";
    delete bufferManager;
    std::cout << "Done!" << std::endl;
}

void Renderer::RenderFrame(Scene& scene)
{
    //Clear screen before rendering
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    Node& sceneRoot = scene.GetRoot();
    glm::mat4 projection = camera->GetProjectionMatrix();
    glm::mat4 view = camera->GetViewMatrix();

    // -----------------------------------------------------------------
    // 0.0 Pre-pass ShadowMapping
    // -----------------------------------------------------------------
    RenderShadowPass(scene, projection, view);
    
    // -----------------------------------------------------------------
    // 1.0 Geometry pass    
    // -----------------------------------------------------------------
    RenderGeometryPass(sceneRoot, projection, view);

    // -----------------------------------------------------------------
    // 1.5 (Optional render preview frame)
    // -----------------------------------------------------------------
    RenderPreviewFrames();

    // -----------------------------------------------------------------
    // 2.0 SSAO pass on G-buffer
    // -----------------------------------------------------------------
    RenderSSAOPass(projection, view);

    // -----------------------------------------------------------------
    // 3.0 Lighting pass
    // -----------------------------------------------------------------
    RenderLightingPass(scene);

    // -----------------------------------------------------------------
    // 3.5 Copy finished deferred render to gBuffer 
    // -----------------------------------------------------------------
    CopyDepthToDefaultFramebuffer();

    // -----------------------------------------------------------------
    // 4.0 Render light cube models post deferred stage
    // -----------------------------------------------------------------
    RenderLightCubes(projection, view);

    // -----------------------------------------------------------------
    // 4.5 Render bounding boxes
    // -----------------------------------------------------------------
    RenderBoundingBoxes(sceneRoot,projection,view);

    // -----------------------------------------------------------------
    // 5.0 Render Skybox
    // -----------------------------------------------------------------
    RenderSkyBox(projection, view);
}

void Renderer::PrepareForRender()
{
    shaderManager = new ShaderManager();
    bufferManager = new BufferManager(screenWidth,screenHeight, shadowCascadeLevels);
    
    std::cout << "[Renderer] Configuring OpenGL and SDL settings..." << std::endl;    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    // stbi_set_flip_vertically_on_load(true);
    stbi_set_flip_vertically_on_load(false);
   
    ConfigureShaders();
    CreateSSAONoise();
    SetupBoundingBoxes();

    bufferManager->CreateBuffers();    
}

void Renderer::ConfigureShaders()
{   
    shaderManager->get("Preview").use();
    shaderManager->get("Preview").setInt("PreviewPosition",  0);
    shaderManager->get("Preview").setInt("PreviewNormals",  1);
    shaderManager->get("Preview").setInt("PreviewDepth",  2);

    shaderManager->get("SSAO").use();
    shaderManager->get("SSAO").setFloat("screenW", (float)screenWidth);
    shaderManager->get("SSAO").setFloat("screenH", (float)screenHeight);
    shaderManager->get("SSAO").setInt("gPosition", 0);
    shaderManager->get("SSAO").setInt("gNormal",   1);
    shaderManager->get("SSAO").setInt("texNoise",  2);
    shaderManager->get("SSAO").setFloat("scaleFactor",glm::pow(bufferManager->divideFactor,2)); // -- SSAO buffer is divide factor size full res is always cubed
    shaderManager->get("SSAO").setFloat("radius", ssao_radius);
    shaderManager->get("SSAO").setFloat("bias", ssao_bias);
    
    //Configure SSAO-blur shader
    shaderManager->get("SSAOBlur").use();
    shaderManager->get("SSAOBlur").setInt("ssaoInput", 0);

    //Configure light shader
    shaderManager->get("DeferredLight").use();
    shaderManager->get("DeferredLight").setInt("gPosition",  0);
    shaderManager->get("DeferredLight").setInt("gNormal",    1);
    shaderManager->get("DeferredLight").setInt("gAlbedoSpec",2);
    shaderManager->get("DeferredLight").setInt("ssao",       3);
    shaderManager->get("DeferredLight").setInt("shadowMap",  4);
    shaderManager->get("DeferredLight").setBool("useFog",    useFog);

    //Configure depth shader
    shaderManager->get("DepthQuad").use();
    shaderManager->get("DepthQuad").setFloat("near_plane", shadow_near_plane);
    shaderManager->get("DepthQuad").setFloat("far_plane", shadow_far_plane);
    shaderManager->get("DepthQuad").setInt("depthMap", 0);
}

void Renderer::CreateSSAONoise()
{
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    for (unsigned int i = 0; i < ssao_kernel_size; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / (float)ssao_kernel_size;

        // scale samples s.t. they're more aligned to center of kernel
        scale = Lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    // ---------------------------
    // generate SSAO noise texture
    // ---------------------------
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(glm::normalize(noise));
    }
    // bufferManager->noiseTexture = CreateBufferTexture(4,4,GL_RGBA32F, GL_RGB, GL_FLOAT, GL_NONE);

    glGenTextures(1, &bufferManager->noiseTexture);
    glBindTexture(GL_TEXTURE_2D, bufferManager->noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Renderer::SetupBoundingBoxes()
{
    
    float vertices[24];
    unsigned int indices[] = {
        0,1, 1,2, 2,3, 3,0, // bottom
        4,5, 5,6, 6,7, 7,4, // top
        0,4, 1,5, 2,6, 3,7  // verticals
    };

    glGenVertexArrays(1, &bboxVAO);
    glGenBuffers(1, &bboxVBO);
    glGenBuffers(1, &bboxEBO);

    glBindVertexArray(bboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bboxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    
}

unsigned int Renderer::CreateBufferTexture(int width, int height, GLenum internalFormat, GLenum format, GLenum type, GLenum attachment)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (attachment != GL_NONE)
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, 0);

    return texture;
}

unsigned int Renderer::LoadTexture(const char *path)
{

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    return texture;

}

float Renderer::Lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

void Renderer::RenderShadowPass(Scene& scene, glm::mat4 &projection, glm::mat4 &view)
{
    //---------------------------------------------------
    //Calculates the lightspace matrix from the suns POV
    //---------------------------------------------------
    Node& sceneRoot = scene.GetRoot();
    shaderManager->get("Depth").use();
        
    glm::vec3 tempLightDir = glm::normalize(scene.lightSettings.sunLightPos);
    lightDir = tempLightDir;

    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    
    if (fabs(lightDir.x) < 0.001f && fabs(lightDir.z) < 0.001f) {
        up = glm::vec3(0.0f, 0.0f, 1.0f); // Use Z-up instead of Y-up
    }
        
    //Setup UBO for CSM
    const auto lightMatrices = getLightSpaceMatrices();
    glBindBuffer(GL_UNIFORM_BUFFER, bufferManager->matricesUBO);
    for (size_t i = 0; i < lightMatrices.size(); ++i)
    {
        glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightMatrices[i]);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    // render scene from light's point of view
    glBindFramebuffer(GL_FRAMEBUFFER, bufferManager->lightFBO);
    glViewport(0, 0, bufferManager->SHADOW_RESOLUTION, bufferManager->SHADOW_RESOLUTION);
    glClear(GL_DEPTH_BUFFER_BIT);
        // glCullFace(GL_FRONT);
        glDisable(GL_CULL_FACE); // No culling during shadow pass        
            glEnable(GL_POLYGON_OFFSET_FILL); // ← Prevent acne
                glPolygonOffset(2.0f, 4.0f);
                RenderScene(shaderManager->get("Depth"),sceneRoot,projection, view);
            glDisable(GL_POLYGON_OFFSET_FILL);
        glEnable(GL_CULL_FACE);  // Restore culling for main pass
        glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
    glViewport(0, 0, screenWidth, screenHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void Renderer::RenderGeometryPass(Node& sceneRoot, glm::mat4& projection, glm::mat4& view)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glEnable(GL_DEPTH_TEST); 
    glBindFramebuffer(GL_FRAMEBUFFER, bufferManager->gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderManager->get("DeferredGeometry").use();
        shaderManager->get("DeferredGeometry").setBool("invertedNormals", false);
        // RenderScene(shaderManager->get("DeferredGeometry"), sceneRoot, projection, view); 
        RenderScene(shaderManager->get("DeferredGeometry"), sceneRoot, projection, view); 
}

void Renderer::RenderSSAOPass(glm::mat4& projection, glm::mat4& view)
{
    glBindFramebuffer(GL_FRAMEBUFFER, bufferManager->ssaoFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    shaderManager->get("SSAO").use();
    shaderManager->get("SSAO").setMat4("view", view);
    shaderManager->get("SSAO").setMat4("projection", projection);
    shaderManager->get("SSAO").setFloat("radius", ssao_radius);
    shaderManager->get("SSAO").setFloat("bias", ssao_bias);
    // Send kernel + rotation 
    for (unsigned int i = 0; i < 64; ++i)
    shaderManager->get("SSAO").setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bufferManager->gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bufferManager->gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, bufferManager->noiseTexture);
    RenderToFrame();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // -----------------------------------------------------------------
    // SSAO blur pass to remove noise
    // -----------------------------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, bufferManager->ssaoBlurFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        shaderManager->get("SSAOBlur").use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bufferManager->ssaoColorBuffer);
        RenderToFrame();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::RenderPreviewFrames()
{
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, bufferManager->previewBuffer);

        glClear(GL_COLOR_BUFFER_BIT);
        shaderManager->get("Preview").use();
        shaderManager->get("Preview").setInt("layer",0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bufferManager->gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, bufferManager->gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D_ARRAY, bufferManager->lightDepthMaps);
        RenderToFrame();
        
        glEnable(GL_DEPTH_TEST);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::RenderLightingPass(Scene& scene)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaderManager->get("DeferredLight").use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bufferManager->gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bufferManager->gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, bufferManager->gAlbedoSpec);
    glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
    glBindTexture(GL_TEXTURE_2D, bufferManager->ssaoColorBufferBlur);
    glActiveTexture(GL_TEXTURE4);
    // glBindTexture(GL_TEXTURE_2D, bufferManager->depthMap);
    glBindTexture(GL_TEXTURE_2D_ARRAY, bufferManager->lightDepthMaps);
    
    shaderManager->get("DeferredLight").setBool("useSSAO", useSSAO);
    shaderManager->get("DeferredLight").setBool("useFog", useFog);
    shaderManager->get("DeferredLight").setFloat("ambientLevel", scene.lightSettings.ambientLevel);
    // shaderManager->get("DeferredLight").setMat4("lightSpaceMatrix", lightSpaceMatrix);
    shaderManager->get("DeferredLight").setVec3("lightDir", lightDir);
    shaderManager->get("DeferredLight").setMat4("view",camera->GetViewMatrix());
    shaderManager->get("DeferredLight").setFloat("farPlane", cameraFarPlane);
    shaderManager->get("DeferredLight").setInt("cascadeCount", shadowCascadeLevels.size());
    for (size_t i = 0; i < shadowCascadeLevels.size(); ++i)
    {
        shaderManager->get("DeferredLight").setFloat("cascadePlaneDistances[" + std::to_string(i) + "]", shadowCascadeLevels[i]);
    }

    for (unsigned int i = 0; i < lightPositions.size(); i++)
    {
        glm::vec3 basePos = lightPositions[i];
        shaderManager->get("DeferredLight").setVec3("lights[" + std::to_string(i) + "].Position", basePos);
        shaderManager->get("DeferredLight").setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
        shaderManager->get("DeferredLight").setFloat("lights[" + std::to_string(i) + "].Linear", scene.lightSettings.linear);
        shaderManager->get("DeferredLight").setFloat("lights[" + std::to_string(i) + "].Quadratic", scene.lightSettings.quadratic);
    }
    shaderManager->get("DeferredLight").setVec3("viewPos", camera->mEye);
    shaderManager->get("DeferredLight").setInt("renderMode", renderMode+1);
    RenderToFrame();
}

void Renderer::CopyDepthToDefaultFramebuffer()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, bufferManager->gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
    glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::RenderLightCubes(const glm::mat4& projection, const glm::mat4& view)
{
    shaderManager->get("LightCube").use();
    shaderManager->get("LightCube").setMat4("projection", projection);
    shaderManager->get("LightCube").setMat4("view", view);
    glm::mat4 model = glm::mat4(1.0f);
    for (unsigned int i = 0; i < lightPositions.size(); i++)
    {
        model = glm::mat4(1.0f);
        
        glm::vec3 basePos = lightPositions[i];
        model = glm::translate(model, basePos);
        model = glm::scale(model, glm::vec3(0.125f));
        shaderManager->get("LightCube").setMat4("model", model);
        shaderManager->get("LightCube").setVec3("lightColor", lightColors[i]);
        RenderCube();
    }
}

void Renderer::RenderBoundingBoxes(Node& sceneRoot, const glm::mat4& projection, const glm::mat4& view)
{
    glLineWidth(2.0f);
    shaderManager->get("BBox").use();
    shaderManager->get("BBox").setMat4("projection", projection);
    shaderManager->get("BBox").setMat4("view", view);
    shaderManager->get("BBox").setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));
    std::list<Node*> nodesToProcess;

    glBindVertexArray(bboxVAO);        
    nodesToProcess.push_back(&sceneRoot);
    
    while (!nodesToProcess.empty())
    {
        Node* currentNode = nodesToProcess.front();
        nodesToProcess.pop_front();
    
        if (Entity* entity = dynamic_cast<Entity*>(currentNode))
        {
            if(shouldRenderBoundingBoxes)
            {
                auto corners = entity->getWorldBoundingBoxCorners();
                float cornersArray[24];
                for (int i = 0; i < 8; ++i)
                {
                    cornersArray[i * 3 + 0] = corners[i].x;
                    cornersArray[i * 3 + 1] = corners[i].y;
                    cornersArray[i * 3 + 2] = corners[i].z;
                }
        
                glBindBuffer(GL_ARRAY_BUFFER, bboxVBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cornersArray), cornersArray);
                shaderManager->get("BBox").setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f)); // red for entity bbox
                
                glBindVertexArray(bboxVAO);
                glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
            }
            
            //------------------
            // Draw sub-entity bboxes
            //------------------
            if(entity->isSelected)
            {
                shaderManager->get("BBox").setVec3("color", glm::vec3(0.0f, 1.0f, 0.0f)); // green for mesh bboxes  
                for (auto& mesh : entity->meshes) {
                    auto min = mesh.aabbMin;
                    auto max = mesh.aabbMax;
                    glm::mat4 modelMatrix = entity->transform.getModelMatrix();
                    std::array<glm::vec3, 8> meshCorners = {
                        glm::vec3(modelMatrix * glm::vec4(min.x, min.y, min.z, 1.0f)),
                        glm::vec3(modelMatrix * glm::vec4(max.x, min.y, min.z, 1.0f)),
                        glm::vec3(modelMatrix * glm::vec4(max.x, min.y, max.z, 1.0f)),
                        glm::vec3(modelMatrix * glm::vec4(min.x, min.y, max.z, 1.0f)),
                        glm::vec3(modelMatrix * glm::vec4(min.x, max.y, min.z, 1.0f)),
                        glm::vec3(modelMatrix * glm::vec4(max.x, max.y, min.z, 1.0f)),
                        glm::vec3(modelMatrix * glm::vec4(max.x, max.y, max.z, 1.0f)),
                        glm::vec3(modelMatrix * glm::vec4(min.x, max.y, max.z, 1.0f))
                    };
                    
                    //Bind buffer so we can still draw sub bboxes even if we dont want to draw the main big one
                    if(!shouldRenderBoundingBoxes)
                        glBindBuffer(GL_ARRAY_BUFFER, bboxVBO); 

                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(meshCorners), meshCorners.data());
                    glBindVertexArray(bboxVAO);
                    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
                }
            }
        }

        for (auto& child : currentNode->children)
            nodesToProcess.push_back(child.get());

    }
    sceneRoot.updateSelfAndChild();
    glBindVertexArray(0);
}



void Renderer::RenderSkyBox(const glm::mat4& projection, const glm::mat4& view)
{
    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    
    shaderManager->get("Skybox").use();
    shaderManager->get("Skybox").setMat4("projection", projection);
    shaderManager->get("Skybox").setMat4("view", glm::mat4(glm::mat3(view)));

    glBindVertexArray(skyBox.VAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void Renderer::RenderScene(Shader &shader, Node &sceneRoot, glm::mat4 &projection, glm::mat4 &view)
{
    //This renders the scene with the default projection,view,model, any other uniforms need to be set before this function is called
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    std::list<Node*> nodesToProcess;

    nodesToProcess.push_back(&sceneRoot); // Start with root

    while (!nodesToProcess.empty())
    {
        Node* currentNode = nodesToProcess.front();
        nodesToProcess.pop_front();

        // Try casting to Entity (We dont want to attempt to draw scene root)
        if (Entity* entity = dynamic_cast<Entity*>(currentNode))
        {
            if (entity->hidden){
                continue;
            }
            shader.setBool("useSkinning",entity->m_HasAnimation);
            
            if(entity->m_HasAnimation){
                entity->animator->UpdateAnimation(GetDeltaTime());
                auto transforms = entity->animator->GetFinalBoneMatrices();
                for (int i = 0; i < transforms.size(); ++i)
                {
                    shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
                }
            
            }

            // It's a visible Entity, so draw it
            shader.setMat4("model", entity->transform.mModelMatrix);
            entity->Draw(shader);

        }
            // Add children to the list
        for (auto& child : currentNode->children)
        {
            nodesToProcess.push_back(child.get());
        }
    }
    sceneRoot.updateSelfAndChild();
}

float Renderer::GetDeltaTime()
{
    static uint64_t lastTime = SDL_GetTicks64();
    uint64_t currentTime = SDL_GetTicks64();
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    // float deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    return deltaTime;
}

void Renderer::RenderToFrame()
{ 
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    
}

void Renderer::RenderCube()
{
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            // X     Y      Z      Nx     Ny     Nz    U     V
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
//--------------------------------------------------------------------------------------------------------
// Calls for the shader manager to reload all shaders and apply the same configurations to them as default
//--------------------------------------------------------------------------------------------------------
void Renderer::ReloadShaders()
{
    shaderManager->reloadShaders();
    ConfigureShaders();
}

std::tuple<unsigned int, unsigned int, unsigned int, unsigned int> Renderer::getPreviewValues()
{
    return {bufferManager->previewPosition, bufferManager->previewNormals, bufferManager->previewDepth, bufferManager->ssaoColorBufferBlur};
}

std::vector<glm::vec4> Renderer::getFrustumCornersWorldSpace(const glm::mat4& projview)
{
    const auto inv = glm::inverse(projview);

    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x)
    {
        for (unsigned int y = 0; y < 2; ++y)
        {
            for (unsigned int z = 0; z < 2; ++z)
            {
                const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}


std::vector<glm::vec4> Renderer::getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
    return getFrustumCornersWorldSpace(proj * view);
}

// glm::mat4 Renderer::getLightSpaceMatrix(const float nearPlane, const float farPlane)
// {
//     const auto proj = glm::perspective(
//         glm::radians(camera->mFovy), (float)screenWidth / (float)screenHeight, nearPlane,
//         farPlane);
//     const auto corners = getFrustumCornersWorldSpace(proj, camera->GetViewMatrix());

//     glm::vec3 center = glm::vec3(0, 0, 0);
//     for (const auto& v : corners)
//     {
//         center += glm::vec3(v);
//     }
//     center /= corners.size();

//     const auto lightView = glm::lookAt(center + lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

//     float minX = std::numeric_limits<float>::max();
//     float maxX = std::numeric_limits<float>::lowest();
//     float minY = std::numeric_limits<float>::max();
//     float maxY = std::numeric_limits<float>::lowest();
//     float minZ = std::numeric_limits<float>::max();
//     float maxZ = std::numeric_limits<float>::lowest();
//     for (const auto& v : corners)
//     {
//         const auto trf = lightView * v;
//         minX = std::min(minX, trf.x);
//         maxX = std::max(maxX, trf.x);
//         minY = std::min(minY, trf.y);
//         maxY = std::max(maxY, trf.y);
//         minZ = std::min(minZ, trf.z);
//         maxZ = std::max(maxZ, trf.z);
//     }

//     float width = maxX - minX;
//     float height = maxY - minY;
//     float maxSide = std::max(width, height);

//     float centerX = (maxX + minX) * 0.5f;
//     float centerY = (maxY + minY) * 0.5f;

//     // Snap to texel grid
//     const float shadowResolution = static_cast<float>(bufferManager->SHADOW_RESOLUTION);
//     float worldUnitsPerTexel = maxSide / shadowResolution;

//     minX = std::floor(minX / worldUnitsPerTexel) * worldUnitsPerTexel;
//     minY = std::floor(minY / worldUnitsPerTexel) * worldUnitsPerTexel;
//     maxX = minX + maxSide;
//     maxY = minY + maxSide;

//     // Tune this parameter according to the scene
//     constexpr float zMult = 5.0f;
//     if (minZ < 0)
//     {
//         minZ *= zMult;
//     }
//     else
//     {
//         minZ /= zMult;
//     }
//     if (maxZ < 0)
//     {
//         maxZ /= zMult;
//     }
//     else
//     {
//         maxZ *= zMult;
//     }

//     const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
//     return lightProjection * lightView;
// }


glm::mat4 Renderer::getLightSpaceMatrix(const float nearPlane, const float farPlane)
{
    const auto proj = glm::perspective(
        glm::radians(camera->mFovy), (float)screenWidth / (float)screenHeight, nearPlane,
        farPlane);
    const auto corners = getFrustumCornersWorldSpace(proj, camera->GetViewMatrix());

    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : corners)
    {
        center += glm::vec3(v);
    }
    center /= corners.size();

    const auto lightView = glm::lookAt(center + lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    for (const auto& v : corners)
    {
        const auto trf = lightView * v;
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }

    // Make ortho bounds square
    float width = maxX - minX;
    float height = maxY - minY;
    float maxSide = std::max(width, height);

    float centerX = (maxX + minX) * 0.5f;
    float centerY = (maxY + minY) * 0.5f;

    // Snap to texel grid
    const float shadowResolution = static_cast<float>(bufferManager->SHADOW_RESOLUTION);
    float worldUnitsPerTexel = maxSide / shadowResolution;

    minX = std::floor(minX / worldUnitsPerTexel) * worldUnitsPerTexel;
    minY = std::floor(minY / worldUnitsPerTexel) * worldUnitsPerTexel;
    maxX = minX + maxSide;
    maxY = minY + maxSide;
    minZ = std::min(minZ, -cameraFarPlane);
    maxZ = std::max(maxZ, cameraFarPlane);


    const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
    return lightProjection * lightView;
}


// glm::mat4 Renderer::getLightSpaceMatrix(const float nearPlane, const float farPlane)
// {
//     // Step 1: Create a projection matrix for the current cascade slice
//     glm::mat4 cascadeProj = glm::perspective(
//         glm::radians(camera->mFovy),
//         static_cast<float>(screenWidth) / static_cast<float>(screenHeight),
//         nearPlane,
//         farPlane
//     );

//     // Step 2: Get the inverse view-projection matrix for the slice
//     glm::mat4 invViewProj = glm::inverse(cascadeProj * camera->GetViewMatrix());

//     // Step 3: Generate 8 frustum corners in NDC space and transform to world space
//     std::vector<glm::vec4> frustumCorners;
//     for (int x = 0; x <= 1; ++x)
//     {
//         for (int y = 0; y <= 1; ++y)
//         {
//             for (int z = 0; z <= 1; ++z)
//             {
//                 glm::vec4 cornerNDC(
//                     2.0f * x - 1.0f,
//                     2.0f * y - 1.0f,
//                     2.0f * z - 1.0f,
//                     1.0f
//                 );
//                 glm::vec4 worldCorner = invViewProj * cornerNDC;
//                 worldCorner /= worldCorner.w;
//                 frustumCorners.push_back(worldCorner);
//             }
//         }
//     }

//     // Step 4: Compute the center of the frustum slice
//     glm::vec3 center(0.0f);
//     for (const auto& corner : frustumCorners)
//         center += glm::vec3(corner);
//     center /= static_cast<float>(frustumCorners.size());

//     // Step 5: Create the light view matrix (fixed lightDir)
//     glm::mat4 lightView = glm::lookAt(
//         center + lightDir,
//         center,
//         glm::vec3(0.0f, 1.0f, 0.0f)
//     );



//     // Step 6: Find AABB bounds of frustum corners in light space
//     float minX = std::numeric_limits<float>::max();
//     float maxX = std::numeric_limits<float>::lowest();
//     float minY = std::numeric_limits<float>::max();
//     float maxY = std::numeric_limits<float>::lowest();
//     float minZ = std::numeric_limits<float>::max();
//     float maxZ = std::numeric_limits<float>::lowest();

//     for (const auto& corner : frustumCorners)
//     {
//         glm::vec4 cornerLS = lightView * corner;
//         minX = std::min(minX, cornerLS.x);
//         maxX = std::max(maxX, cornerLS.x);
//         minY = std::min(minY, cornerLS.y);
//         maxY = std::max(maxY, cornerLS.y);
//         minZ = std::min(minZ, cornerLS.z);
//         maxZ = std::max(maxZ, cornerLS.z);
//     }

//     // Step 7: Make ortho bounds square
//     float width = maxX - minX;
//     float height = maxY - minY;
//     float maxSide = std::max(width, height);

//     float centerX = (minX + maxX) * 0.5f;
//     float centerY = (minY + maxY) * 0.5f;

//     minX = centerX - maxSide / 2.0f;
//     maxX = centerX + maxSide / 2.0f;
//     minY = centerY - maxSide / 2.0f;
//     maxY = centerY + maxSide / 2.0f;

//     // Step 8: Snap to texel grid (stabilization)
//     const float shadowResolution = static_cast<float>(bufferManager->SHADOW_RESOLUTION);
//     float worldUnitsPerTexel = maxSide / shadowResolution;

//     minX = std::floor(minX / worldUnitsPerTexel) * worldUnitsPerTexel;
//     minY = std::floor(minY / worldUnitsPerTexel) * worldUnitsPerTexel;
//     maxX = minX + maxSide;
//     maxY = minY + maxSide;

//     // Optional: Adjust depth bounds slightly or clamp
//     minZ = std::min(minZ, -cameraFarPlane);
//     maxZ = std::max(maxZ, cameraFarPlane);

//     // Step 9: Ortho projection
//     glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

//     return lightProjection * lightView;
// }

std::vector<glm::mat4> Renderer::getLightSpaceMatrices()
{
    std::vector<glm::mat4> ret;
    for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
    {
        if (i == 0)
        {
            ret.push_back(getLightSpaceMatrix(cameraNearPlane, shadowCascadeLevels[i]));
        }
        else if (i < shadowCascadeLevels.size())
        {
            ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
        }
        else
        {
            ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], cameraFarPlane));
        }
    }
    return ret;
}
