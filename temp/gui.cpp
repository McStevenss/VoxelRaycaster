#include "gui.h"
#include <SDL2/SDL.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl3.h"
#include "renderer.h"


void Gui::render(Scene &scene, Renderer* renderer, Camera &camera)
{
    Node& sceneRoot = scene.GetRoot();
    static bool entityPrecisionMode = false;

    // Update frametimes for graph
    float deltaTime = GetDeltaTime();
    float fps = 1.0f / deltaTime;

    frameTimes.push_back(deltaTime * 1000.0f);
    if (frameTimes.size() > maxSamples) {
        frameTimes.erase(frameTimes.begin());
    }

    // Prevents/Allows ImGui from reacting to mouse input
    ImGuiIO& io = ImGui::GetIO();
    if (mMouseActive) {
        io.MouseDrawCursor = false;
    } else {
        io.MouseDrawCursor = true;
    }

    // GUI
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Begin the Info panel window
    ImGui::Begin("Info panel");

    // Create a tab bar within the Info panel window
    if (ImGui::BeginTabBar("Tabs")) {

        // First tab: Info
        if (ImGui::BeginTabItem("Info")) {

            //General ---------------------------------
                AddTitle("General Settings");            
                ImGui::Text("FPS: %.1f", fps); // Display FPS with one decimal place
                ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", camera.mEye.x, camera.mEye.y, camera.mEye.z);
                ImGui::Text("Camera Speed: %.3f", camera.speed);
                
                ImGui::Checkbox("Toggle SSAO ", &renderer->useSSAO);
                ImGui::Checkbox("Toggle Fog ", &renderer->useFog);
                ImGui::Checkbox("Toggle Bounding Boxes ", &renderer->shouldRenderBoundingBoxes);
                
                // Rendering Mode Dropdown
                const char* renderModes[] = {"1. Default", "2. Position", "3. Normal", "4. SSAO"}; // 5 options
                if (ImGui::Combo("##Render Mode", &renderer->renderMode, renderModes, IM_ARRAYSIZE(renderModes))) {}
            //-----------------------------------------

            // Shaders---------------------------------
                AddTitle("Shaders");
                if (ImGui::Button("Reload Shaders")) {
                    renderer->ReloadShaders();
                }
            //----------------------------------------- 
            
            // Frame times---------------------------------
                AddTitle("Frame Times");
                ImGui::Text("Frame Time: %.3f ms", deltaTime * 1000.0f);
                ImGui::PlotLines("Frame Time", frameTimes.data(), frameTimes.size(), 0, NULL, 0.0f, 20.0f, ImVec2(0, 80));
            //-----------------------------------------

            // Light Settings---------------------------------
                AddTitle("Light Settings");
                ImGui::SliderFloat("Sun Pos", &scene.lightSettings.sunLightPos.z, -15.0f, 15.0f, "%.2f");
                ImGui::SliderFloat("Ambient", &scene.lightSettings.ambientLevel, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Linear", &scene.lightSettings.linear, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Quadratic", &scene.lightSettings.quadratic, 0.0f, 1.0f, "%.2f");

                ImGui::Text("SSAO Settings");
                ImGui::SliderFloat("Bias", &renderer->ssao_bias, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Radius", &renderer->ssao_radius, 0.0f, 5.0f, "%.2f");
                ImGui::EndTabItem();
            //-----------------------------------------
        }

        // Second tab: Scene Graph
        if (ImGui::BeginTabItem("Scene Graph")) {
            std::list<std::pair<Node*, int>> nodesToProcess;
            nodesToProcess.push_back({&sceneRoot, 0});

            static Node* selectedEntity = nullptr;

            while (!nodesToProcess.empty()) {
                Node* node = nodesToProcess.front().first;
                int currentDepth = nodesToProcess.front().second;
                nodesToProcess.pop_front();

                if (currentDepth > 0) {
                    ImGui::Indent(currentDepth * 20);  // Indent based on depth (20px per level)
                }

                if (Node* entity = dynamic_cast<Node*>(node)) {
                    bool isSelected = (entity == selectedEntity);
                    std::string nodeName = entity->name;

                    if (entity->isSelected)
                        nodeName = nodeName + " +bbox";
                    if (entity->hidden)
                        nodeName = nodeName + " -hidden";

                    if (ImGui::Selectable(nodeName.c_str(), isSelected))
                        selectedEntity = entity;
                        
                }

                if (currentDepth > 0) {
                    ImGui::Unindent(currentDepth * 20);
                }

                // Add children of the currently processed node
                for (auto& child : node->children) {
                    nodesToProcess.push_back({child.get(), currentDepth + 1});
                }
            }

            if (selectedEntity) {
                

                AddTitle("Node Settings");
                ImGui::Text("Selected Node: %s", selectedEntity->name.c_str());
                ImGui::Checkbox("Hidden: ", &selectedEntity->hidden);
                ImGui::Checkbox("Precision mode: ", &entityPrecisionMode);
                ImGui::Checkbox("Show Sub-bboxes: ", &selectedEntity->isSelected);       
                if (ImGui::Button("Reset Position"))
                    selectedEntity->transform.resetLocalPosition();
                ImGui::SameLine();
                if (ImGui::Button("Reset Rotation"))
                    selectedEntity->transform.resetLocalRotation();
                ImGui::SameLine();
                if (ImGui::Button("Reset Scale"))
                    selectedEntity->transform.resetLocalScale();

                glm::vec3 position = selectedEntity->transform.getLocalPosition();
                float speed = entityPrecisionMode ? 0.01f : 0.1f;
                if (ImGui::DragFloat3("Position", &position.x, speed, -FLT_MAX, FLT_MAX, "%.2f"))
                    selectedEntity->transform.setLocalPosition(position);

                glm::vec3 rotation = selectedEntity->transform.getLocalRotation();
                if (ImGui::DragFloat3("Rotation", &rotation.x, speed * 2, -180.0f, 180.0f, "%.2f"))
                    selectedEntity->transform.setLocalRotation(rotation);

                glm::vec3 scale = selectedEntity->transform.getLocalScale();
                if (ImGui::DragFloat3("Scale", &scale.x, speed, 0.01, 10.0f, "%.2f"))
                    selectedEntity->transform.setLocalScale(scale);
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Preview renders")) {
            auto [previewPosition, previewNormals, previewDepth, ssaoColorBufferBlur] = renderer->getPreviewValues();
            ImVec2 previewSize = ImVec2(renderer->screenWidth / 8, renderer->screenHeight / 8);
            ImGui::Image((void*)(intptr_t)previewPosition, previewSize, ImVec2(0, 1), ImVec2(1, 0));
            ImGui::SameLine();
            ImGui::Image((void*)(intptr_t)previewNormals, previewSize, ImVec2(0, 1), ImVec2(1, 0));
            ImGui::Image((void*)(intptr_t)ssaoColorBufferBlur, previewSize, ImVec2(0, 1), ImVec2(1, 0));
            ImGui::SameLine();
            ImGui::Image((void*)(intptr_t)previewDepth, previewSize, ImVec2(0, 1), ImVec2(1, 0));

        ImGui::EndTabItem();
    }

        // End the tab bar
        ImGui::EndTabBar();
    }

    // End Info panel window
    ImGui::End();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

float Gui::GetDeltaTime()
{
    static uint64_t lastTime = SDL_GetTicks64();
    uint64_t currentTime = SDL_GetTicks64();
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    return deltaTime;
}

void Gui::AddTitle(const char *title)
{
    float windowWidth = ImGui::GetWindowSize().x;
    float textWidth = ImGui::CalcTextSize(title).x;
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator(); // Add a separator below the title
    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::Text("%s",title);
    ImGui::Separator(); // Add a separator below the title
    ImGui::Spacing();
    ImGui::Spacing();
}
