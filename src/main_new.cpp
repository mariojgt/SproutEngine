#include "Engine/Renderer.h"
#include "Engine/Scene.h"
#include "Engine/Components.h"
#include "Engine/Systems.h"
#include "Engine/Scripting.h"
#include "Engine/Editor.h"
#include "Engine/GameplayActors.h"
#include "Engine/Blueprint.h"

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

void SetupExampleBlueprints() {
    // Create a simple rotating cube blueprint
    auto rotatingCubeBP = std::make_unique<BlueprintClass>("RotatingCube_BP");
    rotatingCubeBP->AddDefaultComponent("MeshRenderer");
    rotatingCubeBP->AddDefaultComponent("Collision");
    rotatingCubeBP->AddProperty("RotationSpeed", "float", "90.0");
    rotatingCubeBP->AddProperty("RotationAxis", "vector3", "0,1,0");

    BlueprintManager::Get().RegisterBlueprint("RotatingCube_BP", std::move(rotatingCubeBP));

    // Create a character blueprint
    auto characterBP = std::make_unique<BlueprintClass>("MyCharacter_BP");
    characterBP->AddDefaultComponent("MeshRenderer");
    characterBP->AddDefaultComponent("CapsuleCollision");
    characterBP->AddDefaultComponent("Camera");
    characterBP->AddProperty("WalkSpeed", "float", "600.0");
    characterBP->AddProperty("JumpHeight", "float", "420.0");

    BlueprintManager::Get().RegisterBlueprint("MyCharacter_BP", std::move(characterBP));

    std::cout << "Example blueprints registered!" << std::endl;
}

int main(){
    if(!glfwInit()){ std::cerr<<"Failed to init GLFW\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);

    // Much larger window for better editor experience - 1920x1200 (good for editor layouts)
    GLFWwindow* window = glfwCreateWindow(1920, 1200, "SproutEngine - Unreal-like Game Engine", nullptr, nullptr);
    if(!window){ std::cerr<<"Failed to create window\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    Renderer renderer;
    if(!renderer.init(window)){ std::cerr<<"Renderer init failed\n"; return -1; }

    // Create scene with new Actor system
    Scene scene("MainLevel");
    World* world = scene.GetWorld();

    std::cout << "=== SproutEngine - Unreal-like Game Engine Demo ===" << std::endl;
    std::cout << "Features implemented:" << std::endl;
    std::cout << "- Actor/Component system (like Unreal's AActor)" << std::endl;
    std::cout << "- World management and scene hierarchy" << std::endl;
    std::cout << "- Blueprint foundation and event system" << std::endl;
    std::cout << "- Core components (Mesh, Camera, Light, Audio, Collision)" << std::endl;
    std::cout << "- Pawn/Character/Controller system" << std::endl;
    std::cout << "- GameMode for game flow management" << std::endl;
    std::cout << "- Sprout Script (.sp) language foundation" << std::endl;
    std::cout << "=========================================" << std::endl;

    // Setup example blueprints
    SetupExampleBlueprints();

    // Spawn a GameMode to manage the game
    GameMode* gameMode = world->SpawnActor<GameMode>("MainGameMode");

    // Create some example actors using the new system

    // 1. Rotating cube actor (demonstrates custom actor behavior)
    RotatingCube* rotatingCube = world->SpawnActor<RotatingCube>("DemoCube");
    rotatingCube->SetActorLocation(glm::vec3(2, 0, 0));
    rotatingCube->RotationSpeed = 45.0f; // 45 degrees per second

    // 2. Another cube with different rotation
    RotatingCube* rotatingCube2 = world->SpawnActor<RotatingCube>("DemoCube2");
    rotatingCube2->SetActorLocation(glm::vec3(-2, 0, 0));
    rotatingCube2->RotationSpeed = -60.0f;
    rotatingCube2->RotationAxis = glm::vec3(1, 0, 1); // Rotate around X+Z axis

    // 3. Create a character (controlled by GameMode's PlayerController)
    // The GameMode will automatically create this, but we can also create additional ones
    Character* npcCharacter = world->SpawnActor<Character>("NPCCharacter");
    npcCharacter->SetActorLocation(glm::vec3(0, 0, 3));

    // 4. Create some lights
    auto lightActor = world->SpawnActor<Actor>("MainLight");
    auto lightComponent = lightActor->CreateComponent<LightComponent>(LightComponent::LightType::Directional);
    lightComponent->SetColor(glm::vec3(1.0f, 0.9f, 0.8f));
    lightComponent->SetIntensity(2.0f);
    lightActor->SetActorRotation(glm::vec3(-45, 30, 0));

    // Create some legacy entities for compatibility testing
    auto legacyCube = scene.createEntity("LegacyCube");
    scene.registry.emplace<MeshCube>(legacyCube);
    auto& legacyTransform = scene.registry.get<Transform>(legacyCube);
    legacyTransform.position = {0, 0, -3};

    // Create a HUD entity (legacy system)
    auto hudE = scene.createEntity("HUD");
    scene.registry.emplace<HUDComponent>(hudE, HUDComponent{85.0f, 60.0f, 420, "SproutEngine HUD"});

    // Scripting for legacy system
    Scripting scripting;
    scripting.init();
    scripting.attach(scene.registry);
    scene.registry.emplace<Script>(legacyCube, Script{std::string("assets/scripts/Rotate.lua"), 0.0, false});
    scripting.loadScript(scene.registry, legacyCube, "assets/scripts/Rotate.lua");

    // Editor
    Editor editor;
    TinyImGui::Init(window);
    editor.init(window);

    bool playMode = true;
    auto last = std::chrono::high_resolution_clock::now();

    // Begin play for all actors
    scene.BeginPlay();

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
            // Update new Actor system
            scene.Tick(dt);

            // Update legacy systems for compatibility
            scripting.update(scene.registry, dt);
            Systems::UpdateTransform(scene.registry, dt);
        }

        // Camera setup - use the character's camera if available
        glm::vec3 camPos = {5, 3, 8};
        glm::mat4 V = glm::lookAt(camPos, glm::vec3(0,0,0), glm::vec3(0,1,0));
        glm::mat4 P = glm::perspective(glm::radians(60.0f), width > 0 ? (float)width/height : 16.0f/9.0f, 0.1f, 100.0f);

        // Render actors with mesh components
        if (world) {
            const auto& allActors = world->GetAllActors();
            for (const auto& actor : allActors) {
                if (auto* meshComp = actor->GetComponent<MeshRendererComponent>()) {
                    if (meshComp->IsVisible()) {
                        glm::mat4 M = glm::mat4(1.0f);

                        // Get transform from actor
                        glm::vec3 pos = actor->GetActorLocation();
                        glm::vec3 rot = actor->GetActorRotation();
                        glm::vec3 scale = actor->GetActorScale();

                        M = glm::translate(M, pos);
                        M = glm::rotate(M, glm::radians(rot.x), glm::vec3(1, 0, 0));
                        M = glm::rotate(M, glm::radians(rot.y), glm::vec3(0, 1, 0));
                        M = glm::rotate(M, glm::radians(rot.z), glm::vec3(0, 0, 1));
                        M = glm::scale(M, scale);

                        glm::mat4 MVP = P * V * M;
                        renderer.drawCube(MVP); // For now, all meshes render as cubes
                    }
                }
            }
        }

        // Draw legacy cubes for compatibility
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

        // Add a new panel showing our Actor system
        if (ImGui::Begin("Actor System")) {
            ImGui::Text("SproutEngine Actor System");
            ImGui::Separator();

            if (world) {
                ImGui::Text("Total Actors: %zu", world->GetActorCount());
                ImGui::Spacing();

                if (ImGui::CollapsingHeader("Actors in World")) {
                    const auto& actors = world->GetAllActors();
                    for (const auto& actor : actors) {
                        ImGui::PushID(actor.get());

                        if (ImGui::Selectable(actor->GetName().c_str())) {
                            editor.selected = actor->GetEntity();
                        }

                        // Show actor details
                        if (ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            ImGui::Text("ID: %llu", actor->GetActorID());
                            glm::vec3 pos = actor->GetActorLocation();
                            ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
                            ImGui::EndTooltip();
                        }

                        ImGui::PopID();
                    }
                }

                if (ImGui::CollapsingHeader("Spawn New Actors")) {
                    if (ImGui::Button("Spawn Rotating Cube")) {
                        RotatingCube* newCube = world->SpawnActor<RotatingCube>("NewRotatingCube");
                        newCube->SetActorLocation(glm::vec3(
                            (rand() % 10) - 5,
                            (rand() % 5),
                            (rand() % 10) - 5
                        ));
                    }

                    if (ImGui::Button("Spawn Character")) {
                        Character* newChar = world->SpawnActor<Character>("NewCharacter");
                        newChar->SetActorLocation(glm::vec3(
                            (rand() % 6) - 3,
                            0,
                            (rand() % 6) - 3
                        ));
                    }

                    if (ImGui::Button("Spawn Light")) {
                        auto lightActor = world->SpawnActor<Actor>("NewLight");
                        auto lightComp = lightActor->CreateComponent<LightComponent>(LightComponent::LightType::Point);
                        lightComp->SetColor(glm::vec3(
                            (rand() % 100) / 100.0f,
                            (rand() % 100) / 100.0f,
                            (rand() % 100) / 100.0f
                        ));
                        lightActor->SetActorLocation(glm::vec3(
                            (rand() % 10) - 5,
                            (rand() % 5) + 2,
                            (rand() % 10) - 5
                        ));
                    }
                }
            }
        }
        ImGui::End();

        // Blueprint System Panel
        if (ImGui::Begin("Blueprint System")) {
            ImGui::Text("Blueprint Manager");
            ImGui::Separator();

            if (ImGui::Button("Create RotatingCube from Blueprint")) {
                Actor* blueprintActor = BlueprintManager::Get().CreateBlueprintInstance("RotatingCube_BP", world);
                if (blueprintActor) {
                    blueprintActor->SetActorLocation(glm::vec3(
                        (rand() % 8) - 4,
                        (rand() % 3),
                        (rand() % 8) - 4
                    ));
                }
            }

            if (ImGui::Button("Create Character from Blueprint")) {
                Actor* blueprintActor = BlueprintManager::Get().CreateBlueprintInstance("MyCharacter_BP", world);
                if (blueprintActor) {
                    blueprintActor->SetActorLocation(glm::vec3(
                        (rand() % 6) - 3,
                        0,
                        (rand() % 6) - 3
                    ));
                }
            }
        }
        ImGui::End();

        ImGui::Render();
        TinyImGui::RenderDrawData(ImGui::GetDrawData());
        // ---- end ImGui ----

        renderer.endFrame();
        glfwSwapBuffers(window);
    }

    // Clean shutdown
    scene.EndPlay();

    editor.shutdown(window);
    scripting.shutdown();
    renderer.shutdown();
    TinyImGui::Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
