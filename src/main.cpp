#include "Engine/Renderer.h"
#include "Engine/Scene.h"
#include "Engine/Components.h"
#include "Engine/Systems.h"
#include "Engine/Scripting.h"
#include "Engine/Editor.h"
// Temporarily comment out new system until compilation issues are resolved
// #include "Engine/GameplayActors.h"
// #include "Engine/Blueprint.h"

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

    GLFWwindow* window = glfwCreateWindow(1280, 720, "SproutEngine - Game Engine Foundation", nullptr, nullptr);
    if(!window){ std::cerr<<"Failed to create window\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    Renderer renderer;
    if(!renderer.init(window)){ std::cerr<<"Renderer init failed\n"; return -1; }

    Scene scene("MainLevel");
    
    std::cout << "=== SproutEngine - Unreal-like Game Engine Foundation ===" << std::endl;
    std::cout << "Foundation systems implemented:" << std::endl;
    std::cout << "- Core ECS system with EnTT" << std::endl;
    std::cout << "- Scene management" << std::endl;
    std::cout << "- Component system" << std::endl;
    std::cout << "- Lua scripting integration" << std::endl;
    std::cout << "- ImGui editor interface" << std::endl;
    std::cout << "- Asset management foundation" << std::endl;
    std::cout << "- Transform hierarchy system" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "Next phase: Actor/Component system like Unreal Engine" << std::endl;
    std::cout << "=============================================" << std::endl;

    // Create a cube entity
    auto cube = scene.createEntity("DemoCube");
    scene.registry.emplace<MeshCube>(cube);
    
    // Create multiple cubes to show the system working
    auto cube2 = scene.createEntity("DemoCube2");
    scene.registry.emplace<MeshCube>(cube2);
    auto& transform2 = scene.registry.get<Transform>(cube2);
    transform2.position = {3, 0, 0};
    
    auto cube3 = scene.createEntity("RotatingCube");
    scene.registry.emplace<MeshCube>(cube3);
    auto& transform3 = scene.registry.get<Transform>(cube3);
    transform3.position = {-3, 0, 0};
    
    // Create a HUD entity
    auto hudE = scene.createEntity("HUD");
    scene.registry.emplace<HUDComponent>(hudE, HUDComponent{85.0f, 60.0f, 420, "SproutEngine HUD"});

    auto& tr = scene.registry.get<Transform>(cube);
    tr.position = {0, 0, 0};
    tr.scale = {1,1,1};

    // Scripting
    Scripting scripting;
    scripting.init();
    scripting.attach(scene.registry);
    // Optional: attach script immediately
    scene.registry.emplace<Script>(cube3, Script{std::string("assets/scripts/Rotate.lua"), 0.0, false});
    scripting.loadScript(scene.registry, cube3, "assets/scripts/Rotate.lua");

    // Editor
    Editor editor;
    TinyImGui::Init(window);
    editor.init(window);

    bool playMode = true;
    auto last = std::chrono::high_resolution_clock::now();

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();

        int width, height; 
        glfwGetFramebufferSize(window, &width, &height);
        renderer.beginFrame(width, height);

        // Update
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        if(playMode){
            scripting.update(scene.registry, dt);
            Systems::UpdateTransform(scene.registry, dt);
            
            // Manually rotate one cube to show animation
            auto& rotatingTransform = scene.registry.get<Transform>(cube3);
            rotatingTransform.rotationEuler.y += 45.0f * dt; // 45 degrees per second
        }

        // Camera matrices
        glm::vec3 camPos = {5, 3, 8};
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

        // ---- ImGui Editor Interface ----
        TinyImGui::NewFrame();

        editor.drawDockspace();
        editor.drawPanels(scene.registry, renderer, scripting, playMode);
        
        // Add a development roadmap panel
        if (ImGui::Begin("SproutEngine Roadmap")) {
            ImGui::Text("🎮 Core Systems Progress");
            ImGui::Separator();
            
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "✅ Completed:");
            ImGui::BulletText("ECS System (EnTT)");
            ImGui::BulletText("Scene Management");
            ImGui::BulletText("Transform System");
            ImGui::BulletText("Lua Scripting");
            ImGui::BulletText("ImGui Editor");
            ImGui::BulletText("Basic Rendering");
            
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "🚧 In Progress:");
            ImGui::BulletText("Actor/Component System (like Unreal)");
            ImGui::BulletText("Blueprint Visual Scripting");
            ImGui::BulletText("Event System");
            ImGui::BulletText("Sprout Script (.sp) Language");
            
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "📋 Planned:");
            ImGui::BulletText("Scene Hierarchy & Outliner");
            ImGui::BulletText("Inspector/Details Panel");
            ImGui::BulletText("Viewport Manipulation Gizmos");
            ImGui::BulletText("Asset Browser");
            ImGui::BulletText("Material Editor");
            ImGui::BulletText("PBR Rendering Pipeline");
            ImGui::BulletText("Physics Integration (Bullet/PhysX)");
            ImGui::BulletText("3D Audio System");
            ImGui::BulletText("Animation System");
            ImGui::BulletText("AI Framework");
            ImGui::BulletText("Networking");
            ImGui::BulletText("Level Streaming");
            
            ImGui::Spacing();
            ImGui::Text("Current Status: Foundation systems are working!");
            ImGui::Text("Next: Implementing Actor system architecture");
        }
        ImGui::End();
        
        // Show example code panel
        if (ImGui::Begin("Example Sprout Script")) {
            ImGui::Text("Sprout Script (.sp) - Simplified C++ for game development");
            ImGui::Separator();
            
            ImGui::TextWrapped(R"(
actor MyCharacter extends Character {
    var health: float = 100.0
    var speed: float = 5.0
    
    fun beginPlay() {
        print("Character spawned!")
        setLocation(0, 0, 0)
    }
    
    fun tick(deltaTime: float) {
        moveForward(speed * deltaTime)
        if (health <= 0) {
            destroy()
        }
    }
    
    fun takeDamage(amount: float) {
        health -= amount
        print("Ouch! Health: " + health)
    }
}
            )");
            
            if (ImGui::Button("Learn More About Sprout Script")) {
                // TODO: Open documentation
            }
        }
        ImGui::End();

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
