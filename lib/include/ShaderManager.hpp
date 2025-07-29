#ifndef SHADERMANAGER_HPP
#define SHADERMANAGER_HPP
#include <unordered_map>
#include <string>
#include "Shader.hpp"

class ShaderManager {
    public:

        ShaderManager() {
            //---------------------------------
            //Load and compile Forward shaders
            //---------------------------------
            load("Skybox",            "shaders/forward/skybox.vs",            "shaders/forward/skybox.fs");
            load("Depth",             "shaders/forward/simpleDepthShader.vs", "shaders/forward/simpleDepthShader.fs", "shaders/forward/simpleDepthShader.gs");
            // load("Depth",             "shaders/forward/simpleDepthShader.vs", "shaders/forward/simpleDepthShader.fs");
            load("DepthQuad",         "shaders/forward/debug_depth.vs",       "shaders/forward/debug_depth.fs");
            //---------------------------------
            //Load and compile Deferred shaders
            //---------------------------------    
            load("DeferredLight",     "shaders/deferred/light.vs",            "shaders/deferred/light.fs");
            load("SSAO",              "shaders/deferred/ssao.vs",             "shaders/deferred/ssao.fs");
            load("SSAOBlur",          "shaders/deferred/ssao.vs",             "shaders/deferred/ssao_blur.fs");
            load("DeferredGeometry",  "shaders/deferred/geometry.vs",         "shaders/deferred/geometry.fs");
            load("Preview",           "shaders/deferred/preview.vs",          "shaders/deferred/preview.fs");
            load("BBox",              "shaders/deferred/bbox.vs",             "shaders/deferred/bbox.fs");
            load("LightCube",         "shaders/deferred/cube.vs",             "shaders/deferred/cube.fs");
        }

        void load(const std::string& name, const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr) {
            if (geometryPath == nullptr)
            {
                shaders.emplace(name, Shader(vertexPath, fragmentPath));
            }
            else{
                shaders.emplace(name, Shader(vertexPath, fragmentPath, geometryPath));
            }
        }
    
        Shader& get(const std::string& name) {
            try {
                return shaders.at(name); // throws if not found
            } catch (const std::out_of_range& e) {
                std::cerr << "[Renderer][ShaderManager] Error: Shader '" << name << "' not found.\n";
                throw; 
            }
        }
    
        bool has(const std::string& name) const {
            return shaders.find(name) != shaders.end();
        }

        void reloadShaders(){
            int shaderCount = 0;
            // std::cout << "[ShaderManager] Reloading shaders..." << std::endl;
            for (auto& [name,shader] : shaders){
                shader.reload();
                shaderCount++;
            }
            std::cout << "[Renderer][ShaderManager] Reloaded " << shaderCount << " shaders!" << std::endl;
        }
    
    private:
        std::unordered_map<std::string, Shader> shaders;
    };
#endif