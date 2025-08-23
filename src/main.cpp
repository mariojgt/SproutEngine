#include "Engine/Renderer.h"
#include "Engine/Scene.h"
#include "Engine/Components.h"
#include "Engine/Systems.h"
#include "Engine/Scripting.h"
#include "Engine/Editor.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "Engine/TinyImGui.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <chrono>

static glm::mat4 ComposeTRS(const Transform& t){
    glm::mat4 M(1.0f);
    M = glm::translate(M, t.position);
    M = glm::rotate(M, glm::radians(t.rotationEuler.x), glm::vec3(1,0,0));
    M = glm::rotate(M, glm::radians(t.rotationEuler.y), glm::vec3(0,1,0));
    M = glm::rotate(M, glm::radians(t.rotationEuler.z), glm::vec3(0,0,1));
    M = glm::scale(M, t.scale);
    return M;
}

int main(){
    if(!glfwInit()){ std::cerr<<"Failed to init GLFW\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "SproutEngine", nullptr, nullptr);
    if(!window){ std::cerr<<"Failed to create window\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    Renderer renderer;
    if(!renderer.init(window)){ std::cerr<<"Renderer init failed\n"; return -1; }

    Scene scene;
    // Create a cube entity
    auto cube = scene.createEntity("Cube");
    scene.registry.emplace<MeshCube>(cube);
    // Create a HUD entity
    auto hudE = scene.createEntity("HUD");
    scene.registry.emplace<HUDComponent>(hudE, HUDComponent{85.0f, 60.0f, 420, "RPG Rush HUD"});

    auto& tr = scene.registry.get<Transform>(cube);
    tr.position = {0, 0, 0};
    tr.scale = {1,1,1};

    // Scripting
    Scripting scripting;
    scripting.init();
    scripting.attach(scene.registry);
    // Optional: attach script immediately
    scene.registry.emplace<Script>(cube, Script{std::string("assets/scripts/Rotate.lua"), 0.0, false});
    scripting.loadScript(scene.registry, cube, "assets/scripts/Rotate.lua");

    // Editor
    Editor editor;
    TinyImGui::Init(window);

    editor.init(window);

    bool playMode = true;

    auto last = std::chrono::high_resolution_clock::now();

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();

        int width, height; glfwGetFramebufferSize(window, &width, &height);
        renderer.beginFrame(width, height);

        // Update
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        if(playMode){
            scripting.update(scene.registry, dt);
            Systems::UpdateTransform(scene.registry, dt);
        }

        // Camera matrices
        glm::vec3 camPos = {3, 2, 5};
        glm::mat4 V = glm::lookAt(camPos, glm::vec3(0,0,0), glm::vec3(0,1,0));
        glm::mat4 P = glm::perspective(glm::radians(60.0f), width > 0 ? (float)width/height : 16.0f/9.0f, 0.1f, 100.0f);

        // Draw cubes
        auto view = scene.registry.view<Transform, MeshCube>();
        for(auto e : view){
            auto& t = view.get<Transform>(e);
            glm::mat4 M = ComposeTRS(t);
            glm::mat4 MVP = P * V * M;
            renderer.drawCube(MVP);
        }

        // ---- ImGui pass ----
        Tiny
        
        

        editor.drawDockspace();
        editor.drawPanels(scene.registry, renderer, scripting, playMode);

        ImGui::Render();
        TinyImGui::RenderDrawData(ImGui::GetDrawData());
        // ---- end ImGui ----

        renderer.endFrame();
        glfwSwapBuffers(window);
    }

    editor.shutdown(window);
    scripting.shutdown();
    renderer.shutdown();
    TinyImGui::Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
