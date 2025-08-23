#include "Engine/Renderer.h"
#include "Engine/Scene.h"
#include "Engine/Components.h"
#include "Engine/Systems.h"
#include "Engine/Scripting.h"
#include "Engine/Editor.h"
#include "Engine/UnrealEditorSimple.h"
// Temporarily comment out new system until compilation issues are resolved
// #include "Engine/GameplayActors.h"
// #include "Engine/Blueprint.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <chrono>
#include <algorithm>

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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);

    // Get monitor resolution for adaptive sizing
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    // Handle high DPI displays (4K/Retina) better
    float contentScale;
    glfwGetMonitorContentScale(monitor, &contentScale, nullptr);

    // For 4K displays, we want a good balance between usability and content visibility
    int windowWidth, windowHeight;
    if (contentScale >= 2.0f) {
        // High DPI (4K/Retina) - use larger base size but not too large
        windowWidth = std::max(1800, (int)(mode->width * 0.7));
        windowHeight = std::max(1200, (int)(mode->height * 0.7));
    } else {
        // Standard DPI - use 80% of screen size
        windowWidth = std::max(1600, (int)(mode->width * 0.8));
        windowHeight = std::max(1000, (int)(mode->height * 0.8));
    }

    // Cap maximum size for very large monitors
    windowWidth = std::min(windowWidth, 2800);
    windowHeight = std::min(windowHeight, 1800);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "SproutEngine - Game Engine Foundation", nullptr, nullptr);
    if(!window){ std::cerr<<"Failed to create window\n"; glfwTerminate(); return -1; }

    // Center the window on the primary monitor
    if (monitor && mode) {
        glfwSetWindowPos(window,
            (mode->width - windowWidth) / 2,
            (mode->height - windowHeight) / 2);
    }

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

    // Editor - Use new Unreal-like editor with standard ImGui backends
    UnrealEditor unrealEditor;

    // Setup Dear ImGui context with proper 4K support
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char* glsl_version = "#version 330";
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Configure font for 4K displays - more reasonable scaling
    float baseFontSize = 16.0f;
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    float xscale, yscale;
    glfwGetMonitorContentScale(primaryMonitor, &xscale, &yscale);
    if (xscale >= 2.0f) {
        baseFontSize = 20.0f; // Reasonable size for 4K
    }

    io.Fonts->AddFontDefault();
    io.FontGlobalScale = baseFontSize / 13.0f; // 13.0f is ImGui default font size

    unrealEditor.Init(window);

    bool playMode = true;
    auto last = std::chrono::high_resolution_clock::now();

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // Only update viewport if size is valid
        if (width > 0 && height > 0) {
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

            // Update editor
            unrealEditor.Update(dt);

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
                // If this entity is the selected one in the editor, tint it
                glm::vec3 tint(1.0f, 1.0f, 1.0f);
                if (unrealEditor.GetSelectedEntity() == e) {
                    tint = glm::vec3(1.0f, 0.6f, 0.2f); // orange highlight
                }
                renderer.drawCube(MVP, tint);
            }

            // ---- Unreal-like Editor Interface ----
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            unrealEditor.Render(scene.registry, renderer, scripting, playMode);

            // Rendering
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            // ---- end ImGui ----

            renderer.endFrame();
            glfwSwapBuffers(window);
        }
    }

    unrealEditor.Shutdown(window);
    scripting.shutdown();
    renderer.shutdown();

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
