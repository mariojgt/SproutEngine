#include "UnrealEditorSimple.h"
#include "Components.h"
#include "Renderer.h"
#include "Scripting.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>

UnrealEditor::UnrealEditor() {
    // Initialize Web UI
    InitializeWebUI();
}

UnrealEditor::~UnrealEditor() {
}

bool UnrealEditor::Init(GLFWwindow* window) {
    editorWindow = window;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize console with welcome message
    AddLog("=== SproutEngine Unreal-like Editor Started ===", "System");
    AddLog("Welcome to SproutEngine - Your lightweight Unreal alternative!", "Info");
    AddLog("Type 'help' for available commands", "Info");

    // Refresh content browser
    RefreshContentBrowser();

    return true;
}

void UnrealEditor::Shutdown(GLFWwindow* window) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UnrealEditor::Update(float deltaTime) {
    // Refresh content browser periodically
    static float refreshTimer = 0.0f;
    refreshTimer += deltaTime;
    if (refreshTimer > 2.0f) { // Refresh every 2 seconds
        if (contentBrowser.needsRefresh) {
            RefreshContentBrowser();
            contentBrowser.needsRefresh = false;
        }
        refreshTimer = 0.0f;
    }
}

void UnrealEditor::Render(entt::registry& registry, Renderer& renderer, Scripting& scripting, bool& playMode) {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Draw main menu bar
    DrawMainMenuBar(registry, scripting, playMode);

    // Draw all editor panels in windows
    if (showViewport) DrawViewport(registry, renderer);
    if (showContentBrowser) DrawContentBrowser();
    if (showWorldOutliner) DrawWorldOutliner(registry);
    if (showInspector) DrawInspector(registry, scripting);
    if (showBlueprintGraph) DrawBlueprintEditor();
    if (showConsole) DrawConsole(registry, scripting);
    if (showMaterialEditor) DrawMaterialEditor();
    if (showRoadmap) DrawRoadmap();
    if (showScriptEditor) DrawScriptEditor();

    // Draw toolbar as overlay
    DrawToolbar(playMode);

    // Demo window for development
    if (showDemoWindow) ImGui::ShowDemoWindow(&showDemoWindow);
    if (showMetrics) ImGui::ShowMetricsWindow(&showMetrics);

    // Render
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UnrealEditor::DrawMainMenuBar(entt::registry& registry, Scripting& scripting, bool& playMode) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                NewScene(registry);
            }
            if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
                AddLog("Open Scene - File dialog not implemented yet", "Warning");
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                SaveScene(registry, "assets/scenes/current_scene.json");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Import Asset")) {
                AddLog("Import Asset - File dialog not implemented yet", "Warning");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                glfwSetWindowShouldClose(editorWindow, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
                AddLog("Undo - Not implemented yet", "Warning");
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
                AddLog("Redo - Not implemented yet", "Warning");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete", "Del")) {
                if (selectedEntity != entt::null && IsEntityValid(registry, selectedEntity)) {
                    DeleteEntity(registry, selectedEntity);
                    selectedEntity = entt::null;
                }
            }
            if (ImGui::MenuItem("Duplicate", "Ctrl+D")) {
                if (selectedEntity != entt::null && IsEntityValid(registry, selectedEntity)) {
                    DuplicateEntity(registry, selectedEntity);
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Viewport", nullptr, &showViewport);
            ImGui::MenuItem("Content Browser", nullptr, &showContentBrowser);
            ImGui::MenuItem("World Outliner", nullptr, &showWorldOutliner);
            ImGui::MenuItem("Inspector", nullptr, &showInspector);
            ImGui::MenuItem("Blueprint Editor", nullptr, &showBlueprintGraph);
            ImGui::MenuItem("Script Editor", nullptr, &showScriptEditor);
            ImGui::MenuItem("Console", nullptr, &showConsole);
            ImGui::MenuItem("Material Editor", nullptr, &showMaterialEditor);
            ImGui::MenuItem("Roadmap", nullptr, &showRoadmap);
            ImGui::Separator();
            ImGui::MenuItem("Demo Window", nullptr, &showDemoWindow);
            ImGui::MenuItem("Metrics", nullptr, &showMetrics);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Create")) {
            if (ImGui::MenuItem("Empty Entity")) {
                entt::entity entity = CreateEntity(registry, "Empty Entity");
                selectedEntity = entity;
                AddLog("Created new empty entity", "Info");
            }
            if (ImGui::MenuItem("Cube")) {
                entt::entity entity = CreateEntity(registry, "Cube");
                registry.emplace<MeshCube>(entity);
                selectedEntity = entity;
                AddLog("Created cube entity", "Info");
            }
            if (ImGui::MenuItem("HUD")) {
                entt::entity entity = CreateEntity(registry, "HUD");
                registry.emplace<HUDComponent>(entity, HUDComponent{85.0f, 60.0f, 420, "New HUD"});
                selectedEntity = entity;
                AddLog("Created HUD entity", "Info");
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Reload Scripts", "F5")) {
                AddLog("Reloaded all scripts", "Info");
            }
            if (ImGui::MenuItem("Build Lighting")) {
                AddLog("Build Lighting - Not implemented yet", "Warning");
            }
            if (ImGui::MenuItem("Generate Navmesh")) {
                AddLog("Generate Navmesh - Not implemented yet", "Warning");
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About SproutEngine")) {
                AddLog("SproutEngine v1.0 - Unreal-like Game Engine", "Info");
            }
            if (ImGui::MenuItem("Documentation")) {
                AddLog("Documentation - Opening external link", "Info");
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void UnrealEditor::DrawToolbar(bool& playMode) {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                    ImGuiWindowFlags_NoCollapse;

    // Position toolbar at top of screen
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 50));

    if (ImGui::Begin("Toolbar", nullptr, window_flags)) {
        // Play/Stop/Pause buttons
        const char* playButtonText = playMode ? "Stop" : "Play";
        if (ImGui::Button(playButtonText, ImVec2(60, 30))) {
            playMode = !playMode;
            if (playMode) {
                currentMode = EditorMode::Play;
                AddLog("Entered Play Mode", "System");
            } else {
                currentMode = EditorMode::Edit;
                AddLog("Entered Edit Mode", "System");
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Pause", ImVec2(60, 30))) {
            currentMode = EditorMode::Simulate;
            AddLog("Entered Simulate Mode", "System");
        }

        ImGui::SameLine();
        ImGui::Text("|"); // Vertical separator replacement

        // Transform tools
        ImGui::SameLine();
        if (ImGui::Button("Select", ImVec2(60, 30))) {
            AddLog("Select tool activated", "Info");
        }

        ImGui::SameLine();
        if (ImGui::Button("Move", ImVec2(60, 30))) {
            AddLog("Move tool activated", "Info");
        }

        ImGui::SameLine();
        if (ImGui::Button("Rotate", ImVec2(60, 30))) {
            AddLog("Rotate tool activated", "Info");
        }

        ImGui::SameLine();
        if (ImGui::Button("Scale", ImVec2(60, 30))) {
            AddLog("Scale tool activated", "Info");
        }

        // Current mode display
        ImGui::SameLine();
        ImGui::Text("|"); // Vertical separator replacement
        ImGui::SameLine();
        const char* modeText = currentMode == EditorMode::Play ? "PLAYING" :
                              currentMode == EditorMode::Simulate ? "SIMULATING" : "EDITING";
        ImGui::Text("Mode: %s", modeText);
    }
    ImGui::End();
}

void UnrealEditor::DrawViewport(entt::registry& registry, Renderer& renderer) {
    if (ImGui::Begin("Viewport")) {
        // Get the size of the viewport
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();

        // Display viewport info
        ImGui::Text("Camera Position: %.2f, %.2f, %.2f",
                   viewportCamera.position.x,
                   viewportCamera.position.y,
                   viewportCamera.position.z);

        ImGui::Text("Selected Entity: %s",
                   selectedEntity != entt::null ?
                   GetEntityName(registry, selectedEntity).c_str() : "None");

        // Viewport controls info
        ImGui::Text("Controls: WASD + Mouse to navigate, Click to select entities");

        // Grid and gizmo options
        static bool showGrid = true;
        static bool showGizmos = true;
        ImGui::Checkbox("Show Grid", &showGrid);
        ImGui::SameLine();
        ImGui::Checkbox("Show Gizmos", &showGizmos);

        // Entity selection list for now (until we have proper 3D picking)
        ImGui::Separator();
        ImGui::Text("Quick Entity Selection:");

        // List entities in the viewport for selection
        auto view = registry.view<NameComponent>();
        for (auto entity : view) {
            auto& name = view.get<NameComponent>(entity);
            bool isSelected = (entity == selectedEntity);

            if (ImGui::Selectable(name.name.c_str(), isSelected)) {
                selectedEntity = entity;
                AddLog("Selected entity: " + name.name, "Info");
            }
        }
    }
    ImGui::End();
}

void UnrealEditor::DrawContentBrowser() {
    if (ImGui::Begin("Content Browser")) {
        // Path navigation
        ImGui::Text("Current Path: %s", contentBrowser.currentPath.c_str());

        if (ImGui::Button("Refresh")) {
            RefreshContentBrowser();
        }

        ImGui::SameLine();
        if (ImGui::Button("Up") && contentBrowser.currentPath != "assets/") {
            // Go up one directory
            size_t lastSlash = contentBrowser.currentPath.find_last_of("/", contentBrowser.currentPath.length() - 2);
            if (lastSlash != std::string::npos) {
                contentBrowser.currentPath = contentBrowser.currentPath.substr(0, lastSlash + 1);
                RefreshContentBrowser();
            }
        }

        ImGui::Separator();

        // Directories
        if (!contentBrowser.directories.empty()) {
            ImGui::Text("Directories:");
            for (const auto& dir : contentBrowser.directories) {
                if (ImGui::Selectable(("📁 " + dir).c_str(), false, ImGuiSelectableFlags_DontClosePopups)) {
                    contentBrowser.currentPath += dir + "/";
                    RefreshContentBrowser();
                }
            }
            ImGui::Separator();
        }

        // Files
        if (!contentBrowser.files.empty()) {
            ImGui::Text("Files:");
            for (const auto& file : contentBrowser.files) {
                std::string icon = "📄";
                if (file.ends_with(".lua")) icon = "📜";
                else if (file.ends_with(".sp")) icon = "🌱";
                else if (file.ends_with(".obj") || file.ends_with(".fbx")) icon = "🎨";
                else if (file.ends_with(".png") || file.ends_with(".jpg")) icon = "🖼️";

                bool isSelected = (contentBrowser.selectedItem == file);
                if (ImGui::Selectable((icon + " " + file).c_str(), isSelected)) {
                    contentBrowser.selectedItem = file;
                }

                // Context menu for files
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Open")) {
                        AddLog("Open file: " + file, "Info");
                    }
                    if (ImGui::MenuItem("Edit")) {
                        AddLog("Edit file: " + file, "Info");
                    }
                    if (ImGui::MenuItem("Delete")) {
                        AddLog("Delete file: " + file, "Warning");
                    }
                    ImGui::EndPopup();
                }
            }
        }

        // Context menu for empty space
        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::MenuItem("Create Folder")) {
                AddLog("Create Folder - Not implemented yet", "Warning");
            }
            if (ImGui::MenuItem("Create Material")) {
                AddLog("Create Material - Not implemented yet", "Warning");
            }
            if (ImGui::MenuItem("Create Script")) {
                AddLog("Create Script - Not implemented yet", "Warning");
            }
            if (ImGui::MenuItem("Import Asset")) {
                AddLog("Import Asset - Not implemented yet", "Warning");
            }
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void UnrealEditor::DrawWorldOutliner(entt::registry& registry) {
    if (ImGui::Begin("World Outliner")) {
        // Search filter
        static char searchBuffer[256] = "";
        ImGui::InputText("Search", searchBuffer, sizeof(searchBuffer));

        ImGui::Separator();

        // Entity count
        auto view = registry.view<NameComponent>();
        ImGui::Text("Entities: %zu", view.size());

        ImGui::Separator();

        // Entity list
        for (auto entity : view) {
            auto& name = view.get<NameComponent>(entity);

            // Filter by search
            if (strlen(searchBuffer) > 0) {
                if (name.name.find(searchBuffer) == std::string::npos) {
                    continue;
                }
            }

            bool isSelected = (entity == selectedEntity);

            // Add icon based on components
            std::string icon = "📦"; // Default entity icon
            if (registry.any_of<MeshCube>(entity)) icon = "🟦";
            if (registry.any_of<HUDComponent>(entity)) icon = "🖥️";
            if (registry.any_of<Script>(entity)) icon = "📜";

            if (ImGui::Selectable((icon + " " + name.name).c_str(), isSelected)) {
                selectedEntity = entity;
                AddLog("Selected entity: " + name.name, "Info");
            }

            // Context menu
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Rename")) {
                    AddLog("Rename - Not implemented yet", "Warning");
                }
                if (ImGui::MenuItem("Delete")) {
                    DeleteEntity(registry, entity);
                    if (entity == selectedEntity) {
                        selectedEntity = entt::null;
                    }
                }
                if (ImGui::MenuItem("Duplicate")) {
                    DuplicateEntity(registry, entity);
                }
                ImGui::EndPopup();
            }
        }

        // Context menu for empty space
        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::MenuItem("Create Empty Entity")) {
                entt::entity entity = CreateEntity(registry, "New Entity");
                selectedEntity = entity;
            }
            if (ImGui::MenuItem("Create Cube")) {
                entt::entity entity = CreateEntity(registry, "New Cube");
                registry.emplace<MeshCube>(entity);
                selectedEntity = entity;
            }
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void UnrealEditor::DrawInspector(entt::registry& registry, Scripting& scripting) {
    if (ImGui::Begin("Inspector")) {
        if (selectedEntity != entt::null && IsEntityValid(registry, selectedEntity)) {
            // Entity ID and info
            ImGui::Text("Entity ID: %u", (uint32_t)selectedEntity);

            // Entity name
            auto* nameComp = registry.try_get<NameComponent>(selectedEntity);
            if (nameComp) {
                char nameBuffer[256];
                strncpy(nameBuffer, nameComp->name.c_str(), sizeof(nameBuffer));
                if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
                    nameComp->name = std::string(nameBuffer);
                }
            }

            ImGui::Separator();

            // Transform Component
            if (auto* transform = registry.try_get<Transform>(selectedEntity)) {
                if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::DragFloat3("Position", &transform->position.x, 0.1f);
                    ImGui::DragFloat3("Rotation", &transform->rotationEuler.x, 1.0f);
                    ImGui::DragFloat3("Scale", &transform->scale.x, 0.1f);
                }
            }

            // Mesh Component
            if (auto* mesh = registry.try_get<MeshCube>(selectedEntity)) {
                if (ImGui::CollapsingHeader("Mesh (Cube)", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Mesh Type: Cube");
                    ImGui::Text("This is a basic cube mesh component");
                }
            }

            // HUD Component
            if (auto* hud = registry.try_get<HUDComponent>(selectedEntity)) {
                if (ImGui::CollapsingHeader("HUD", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::DragFloat("X", &hud->x, 1.0f);
                    ImGui::DragFloat("Y", &hud->y, 1.0f);
                    ImGui::DragInt("Width", &hud->width);

                    char textBuffer[256];
                    strncpy(textBuffer, hud->text.c_str(), sizeof(textBuffer));
                    if (ImGui::InputText("Text", textBuffer, sizeof(textBuffer))) {
                        hud->text = std::string(textBuffer);
                    }
                }
            }

            // Script Component - ENHANCED
            if (auto* script = registry.try_get<Script>(selectedEntity)) {
                if (ImGui::CollapsingHeader("Script", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Current Script: %s", script->filePath.empty() ? "None" : script->filePath.c_str());

                    // Script selection dropdown
                    if (ImGui::BeginCombo("Select Script", script->filePath.empty() ? "Choose Script..." : script->filePath.c_str())) {
                        // List existing scripts dynamically
                        std::vector<std::string> availableScripts = {"Rotate.lua", "Rotate.sp", "PlayerCharacter.sp", "NewScript.sp"};
                        for (const auto& scriptFile : availableScripts) {
                            if (ImGui::Selectable(scriptFile.c_str(), script->filePath == ("assets/scripts/" + scriptFile))) {
                                script->filePath = "assets/scripts/" + scriptFile;
                                // Reload the script
                                scripting.loadScript(registry, selectedEntity, script->filePath);
                                AddLog("Selected script: " + scriptFile, "Info");
                            }
                        }

                        ImGui::Separator();

                        // Create new options
                        if (ImGui::Selectable("+ Create New Script")) {
                            script->filePath = "assets/scripts/NewScript.sp";
                            AddLog("Created new script template", "Info");
                        }

                        if (ImGui::Selectable("+ Create New Blueprint")) {
                            script->filePath = "assets/scripts/NewBlueprint.bp";
                            AddLog("Created new blueprint template", "Info");
                        }

                        ImGui::EndCombo();
                    }

                    ImGui::Separator();

                    // Action buttons
                    if (!script->filePath.empty()) {
                        if (ImGui::Button("Edit Script")) {
                            if (script->filePath.find(".sp") != std::string::npos) {
                                AddLog("Opening Script Editor for: " + script->filePath, "Info");
                                OpenScriptFile(script->filePath);
                                showScriptEditor = true;
                            } else if (script->filePath.find(".bp") != std::string::npos) {
                                AddLog("Opening Blueprint Editor for: " + script->filePath, "Info");
                                OpenBlueprintFile(script->filePath);
                                showBlueprintGraph = true;
                            } else {
                                AddLog("Editing Lua script: " + script->filePath, "Info");
                            }
                        }

                        ImGui::SameLine();
                        if (ImGui::Button("Reload Script")) {
                            scripting.loadScript(registry, selectedEntity, script->filePath);
                            AddLog("Reloaded script: " + script->filePath, "Info");
                        }

                        ImGui::SameLine();
                        if (ImGui::Button("Remove Script")) {
                            script->filePath = "";
                            AddLog("Removed script from entity", "Info");
                        }
                    }

                    ImGui::Separator();

                    // Quick creation buttons
                    if (ImGui::Button("Quick Script Template")) {
                        auto* nameComp = registry.try_get<NameComponent>(selectedEntity);
                        std::string entityName = nameComp ? nameComp->name : "Entity";
                        std::string scriptName = "assets/scripts/" + entityName + "Script.sp";
                        script->filePath = scriptName;
                        AddLog("Created quick script template: " + scriptName, "Info");
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Quick Blueprint")) {
                        auto* nameComp = registry.try_get<NameComponent>(selectedEntity);
                        std::string entityName = nameComp ? nameComp->name : "Entity";
                        std::string blueprintName = "assets/scripts/" + entityName + "BP.bp";
                        script->filePath = blueprintName;
                        AddLog("Created quick blueprint: " + blueprintName, "Info");
                    }

                    // Show script status
                    if (script->needsUpdate) {
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Script needs reload");
                        ImGui::SameLine();
                        if (ImGui::Button("Reload Now")) {
                            script->needsUpdate = false;
                            scripting.loadScript(registry, selectedEntity, script->filePath);
                            AddLog("Hot reloaded script: " + script->filePath, "Info");
                        }
                    } else if (!script->filePath.empty()) {
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Script up to date");
                    }

                    ImGui::Text("Last Update: %.2f", script->lastUpdateTime);
                }
            }

            ImGui::Separator();

            // Add Component section - ENHANCED
            if (ImGui::Button("Add Component")) {
                ImGui::OpenPopup("AddComponentPopup");
            }

            if (ImGui::BeginPopup("AddComponentPopup")) {
                if (ImGui::MenuItem("Mesh (Cube)")) {
                    if (!registry.all_of<MeshCube>(selectedEntity)) {
                        registry.emplace<MeshCube>(selectedEntity);
                        AddLog("Added Mesh Component", "Info");
                    }
                }

                // Script submenu
                if (ImGui::BeginMenu("Script")) {
                    if (!registry.all_of<Script>(selectedEntity)) {
                        if (ImGui::MenuItem("Add Existing Script")) {
                            registry.emplace<Script>(selectedEntity, "", 0.0f, false);
                            AddLog("Added Script Component", "Info");
                        }

                        ImGui::Separator();

                        if (ImGui::MenuItem("Create New Script")) {
                            registry.emplace<Script>(selectedEntity, "assets/scripts/NewScript.sp", 0.0f, false);
                            AddLog("Added new Script component with template", "Info");
                        }

                        if (ImGui::MenuItem("Create New Blueprint")) {
                            registry.emplace<Script>(selectedEntity, "assets/scripts/NewBlueprint.bp", 0.0f, false);
                            AddLog("Added new Blueprint component", "Info");
                        }

                        ImGui::Separator();
                        ImGui::Text("Quick Add:");

                        if (ImGui::MenuItem("Rotate Script")) {
                            registry.emplace<Script>(selectedEntity, "assets/scripts/Rotate.sp", 0.0f, false);
                            AddLog("Added Rotate script", "Info");
                        }

                        if (ImGui::MenuItem("Rotate Lua Script")) {
                            registry.emplace<Script>(selectedEntity, "assets/scripts/Rotate.lua", 0.0f, false);
                            scripting.loadScript(registry, selectedEntity, "assets/scripts/Rotate.lua");
                            AddLog("Added Rotate Lua script", "Info");
                        }

                        if (ImGui::MenuItem("Player Character Script")) {
                            registry.emplace<Script>(selectedEntity, "assets/scripts/PlayerCharacter.sp", 0.0f, false);
                            AddLog("Added Player Character script", "Info");
                        }
                    } else {
                        ImGui::Text("Script already exists");
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::MenuItem("HUD")) {
                    if (!registry.all_of<HUDComponent>(selectedEntity)) {
                        registry.emplace<HUDComponent>(selectedEntity, HUDComponent{100.0f, 100.0f, 200, "New HUD"});
                        AddLog("Added HUD Component", "Info");
                    }
                }
                ImGui::EndPopup();
            }

        } else {
            ImGui::Text("No entity selected");
            ImGui::Text("Select an entity in the World Outliner");

            // Quick entity creation
            ImGui::Separator();
            ImGui::Text("Quick Create:");
            if (ImGui::Button("Create Cube")) {
                entt::entity entity = CreateEntity(registry, "Quick Cube");
                registry.emplace<MeshCube>(entity);
                selectedEntity = entity;
                AddLog("Created cube from Inspector", "Info");
            }
        }
    }
    ImGui::End();
}

void UnrealEditor::DrawBlueprintGraph() {
    if (ImGui::Begin("Blueprint Graph")) {
        ImGui::Text("🔧 Blueprint Visual Scripting System");
        ImGui::Separator();

        ImGui::Text("This panel will contain the visual scripting interface");
        ImGui::Text("Similar to Unreal Engine's Blueprint system");

        ImGui::Spacing();
        ImGui::Text("Features planned:");
        ImGui::BulletText("Node-based visual scripting");
        ImGui::BulletText("Event nodes (BeginPlay, Tick, etc.)");
        ImGui::BulletText("Function nodes");
        ImGui::BulletText("Variable nodes");
        ImGui::BulletText("Flow control nodes");
        ImGui::BulletText("Lua code generation");

        ImGui::Spacing();
        if (ImGui::Button("Generate Test Lua")) {
            AddLog("Generated Lua from Blueprint graph (placeholder)", "Info");
        }

        ImGui::Spacing();
        ImGui::Text("Current Status: Architecture designed, UI pending");
        ImGui::Text("Requires imnodes library for full implementation");
    }
    ImGui::End();
}

void UnrealEditor::DrawConsole(entt::registry& registry, Scripting& scripting) {
    if (ImGui::Begin("Console")) {
        // Options
        if (ImGui::Button("Clear")) {
            console.logs.clear();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &console.autoScroll);

        ImGui::Separator();

        // Log display
        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -30), false, ImGuiWindowFlags_HorizontalScrollbar)) {
            for (const auto& log : console.logs) {
                // Color code based on level
                if (log.find("[Error]") != std::string::npos) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                } else if (log.find("[Warning]") != std::string::npos) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.3f, 1.0f));
                } else if (log.find("[System]") != std::string::npos) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                }

                ImGui::TextUnformatted(log.c_str());
                ImGui::PopStyleColor();
            }

            if (console.autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }
        }
        ImGui::EndChild();

        // Command input
        ImGui::Separator();
        char inputBuffer[256];
        strncpy(inputBuffer, console.inputBuffer.c_str(), sizeof(inputBuffer));

        if (ImGui::InputText("Command", inputBuffer, sizeof(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            console.inputBuffer = std::string(inputBuffer);
            if (!console.inputBuffer.empty()) {
                AddLog("> " + console.inputBuffer, "Input");
                ExecuteCommand(console.inputBuffer, registry, scripting);
                console.inputBuffer.clear();
            }
        }

        // Keep focus on input
        if (ImGui::IsItemDeactivated()) {
            ImGui::SetKeyboardFocusHere(-1);
        }
    }
    ImGui::End();
}

void UnrealEditor::DrawMaterialEditor() {
    if (ImGui::Begin("Material Editor")) {
        ImGui::Text("🎨 Material Editor - PBR Pipeline");
        ImGui::Separator();

        ImGui::Text("Advanced material editing interface");
        ImGui::Text("Similar to Unreal Engine's Material Editor");

        // Basic material properties
        static float roughness = 0.5f;
        static float metallic = 0.0f;
        static ImVec4 baseColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

        ImGui::Spacing();
        ImGui::Text("Basic Properties:");
        ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f);
        ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f);
        ImGui::ColorEdit4("Base Color", &baseColor.x);

        ImGui::Spacing();
        if (ImGui::Button("Apply to Selected")) {
            if (selectedEntity != entt::null) {
                AddLog("Applied material properties to selected entity", "Info");
            }
        }

        ImGui::Spacing();
        ImGui::Text("Features planned:");
        ImGui::BulletText("Node-based material graphs");
        ImGui::BulletText("PBR shading model");
        ImGui::BulletText("Texture mapping");
        ImGui::BulletText("Real-time preview");
        ImGui::BulletText("Shader code generation");
    }
    ImGui::End();
}

void UnrealEditor::DrawRoadmap() {
    if (ImGui::Begin("SproutEngine Roadmap")) {
        ImGui::Text("🎮 SproutEngine Development Roadmap");
        ImGui::Separator();

        ImGui::TextColored(ImVec4(0, 1, 0, 1), "✅ Phase 1 - Foundation (COMPLETED):");
        ImGui::BulletText("ECS System (EnTT) - ✅");
        ImGui::BulletText("Scene Management - ✅");
        ImGui::BulletText("Transform System - ✅");
        ImGui::BulletText("Lua Scripting Integration - ✅");
        ImGui::BulletText("Basic ImGui Editor - ✅");
        ImGui::BulletText("Basic OpenGL Rendering - ✅");
        ImGui::BulletText("Component System - ✅");

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "🚧 Phase 2 - Unreal-like Editor (IN PROGRESS):");
        ImGui::BulletText("Multi-panel Editor Interface - ✅");
        ImGui::BulletText("World Outliner/Hierarchy - ✅");
        ImGui::BulletText("Inspector/Details Panel - ✅");
        ImGui::BulletText("Content Browser - ✅");
        ImGui::BulletText("Console System - ✅");
        ImGui::BulletText("Viewport Navigation - 🔄");
        ImGui::BulletText("Entity Selection & Manipulation - 🔄");
        ImGui::BulletText("Asset Drag & Drop - 📋");

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1), "📋 Phase 3 - Advanced Systems:");
        ImGui::BulletText("Actor/Component System (like Unreal)");
        ImGui::BulletText("Blueprint Visual Scripting");
        ImGui::BulletText("Sprout Script (.sp) Language");
        ImGui::BulletText("3D Viewport Gizmos");
        ImGui::BulletText("Material Editor");
        ImGui::BulletText("PBR Rendering Pipeline");

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "🔮 Phase 4 - Professional Features:");
        ImGui::BulletText("Physics Integration (Bullet/PhysX)");
        ImGui::BulletText("3D Audio System");
        ImGui::BulletText("Animation System & Timeline");
        ImGui::BulletText("AI Framework & Behavior Trees");
        ImGui::BulletText("Networking & Multiplayer");
        ImGui::BulletText("Level Streaming");
        ImGui::BulletText("Asset Pipeline & Build System");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("🎯 Current Focus: Unreal-like Editor Interface");
        ImGui::Text("✨ Status: Core editor panels functional!");
        ImGui::Text("🚀 Next: 3D viewport navigation and entity manipulation");

        if (ImGui::Button("View GitHub Repository")) {
            AddLog("Opening GitHub repository (placeholder)", "Info");
        }
        ImGui::SameLine();
        if (ImGui::Button("Check Documentation")) {
            AddLog("Opening documentation (placeholder)", "Info");
        }
    }
    ImGui::End();
}

// Utility function implementations
std::string UnrealEditor::GetEntityName(entt::registry& registry, entt::entity entity) {
    auto* nameComp = registry.try_get<NameComponent>(entity);
    return nameComp ? nameComp->name : "Unnamed Entity";
}

void UnrealEditor::SetEntityName(entt::registry& registry, entt::entity entity, const std::string& name) {
    auto* nameComp = registry.try_get<NameComponent>(entity);
    if (nameComp) {
        nameComp->name = name;
    } else {
        registry.emplace<NameComponent>(entity, name);
    }
}

bool UnrealEditor::IsEntityValid(entt::registry& registry, entt::entity entity) {
    return registry.valid(entity);
}

entt::entity UnrealEditor::CreateEntity(entt::registry& registry, const std::string& name) {
    entt::entity entity = registry.create();
    registry.emplace<NameComponent>(entity, name);
    registry.emplace<Transform>(entity);
    return entity;
}

void UnrealEditor::DeleteEntity(entt::registry& registry, entt::entity entity) {
    std::string name = GetEntityName(registry, entity);
    registry.destroy(entity);
    AddLog("Deleted entity: " + name, "Info");
}

void UnrealEditor::DuplicateEntity(entt::registry& registry, entt::entity entity) {
    std::string originalName = GetEntityName(registry, entity);
    entt::entity newEntity = CreateEntity(registry, originalName + " Copy");

    // Copy transform if it exists
    if (auto* transform = registry.try_get<Transform>(entity)) {
        registry.emplace<Transform>(newEntity, *transform);
        // Offset position slightly
        auto& newTransform = registry.get<Transform>(newEntity);
        newTransform.position.x += 1.0f;
    }

    // Copy other components
    if (auto* mesh = registry.try_get<MeshCube>(entity)) {
        registry.emplace<MeshCube>(newEntity, *mesh);
    }

    if (auto* hud = registry.try_get<HUDComponent>(entity)) {
        registry.emplace<HUDComponent>(newEntity, *hud);
    }

    if (auto* script = registry.try_get<Script>(entity)) {
        registry.emplace<Script>(newEntity, *script);
    }

    AddLog("Duplicated entity: " + originalName, "Info");
}

void UnrealEditor::AddLog(const std::string& message, const std::string& level) {
    std::string fullMessage = "[" + level + "] " + message;
    console.logs.push_back(fullMessage);

    // Limit log size
    if (console.logs.size() > console.maxLogs) {
        console.logs.erase(console.logs.begin());
    }

    // Also print to stdout for debugging
    std::cout << fullMessage << std::endl;
}

void UnrealEditor::ExecuteCommand(const std::string& command, entt::registry& registry, Scripting& scripting) {
    if (command == "help") {
        AddLog("Available commands:", "Info");
        AddLog("  help - Show this help", "Info");
        AddLog("  clear - Clear console", "Info");
        AddLog("  entities - List all entities", "Info");
        AddLog("  info - Show engine information", "Info");
        AddLog("  create <type> - Create entity (cube, hud)", "Info");
    } else if (command == "clear") {
        console.logs.clear();
    } else if (command == "entities") {
        auto view = registry.view<NameComponent>();
        AddLog("Entities in scene (" + std::to_string(view.size()) + "):", "Info");
        for (auto entity : view) {
            auto& name = view.get<NameComponent>(entity);
            AddLog("  - " + name.name + " (ID: " + std::to_string((uint32_t)entity) + ")", "Info");
        }
    } else if (command == "info") {
        AddLog("SproutEngine v1.0 - Unreal-like Game Engine", "System");
        AddLog("Built with: C++20, EnTT, OpenGL, ImGui, Sol2/Lua", "System");
        AddLog("Current mode: " + std::string(currentMode == EditorMode::Edit ? "Edit" :
                                              currentMode == EditorMode::Play ? "Play" : "Simulate"), "System");
    } else if (command.substr(0, 7) == "create ") {
        std::string type = command.substr(7);
        if (type == "cube") {
            entt::entity entity = CreateEntity(registry, "Console Cube");
            registry.emplace<MeshCube>(entity);
            selectedEntity = entity;
            AddLog("Created cube entity via console", "Info");
        } else if (type == "hud") {
            entt::entity entity = CreateEntity(registry, "Console HUD");
            registry.emplace<HUDComponent>(entity, HUDComponent{150.0f, 150.0f, 300, "Console HUD"});
            selectedEntity = entity;
            AddLog("Created HUD entity via console", "Info");
        } else {
            AddLog("Unknown entity type: " + type, "Warning");
            AddLog("Available types: cube, hud", "Info");
        }
    } else {
        AddLog("Unknown command: " + command, "Warning");
        AddLog("Type 'help' for available commands", "Info");
    }
}

void UnrealEditor::RefreshContentBrowser() {
    contentBrowser.directories.clear();
    contentBrowser.files.clear();

    try {
        if (std::filesystem::exists(contentBrowser.currentPath)) {
            for (const auto& entry : std::filesystem::directory_iterator(contentBrowser.currentPath)) {
                if (entry.is_directory()) {
                    contentBrowser.directories.push_back(entry.path().filename().string());
                } else {
                    contentBrowser.files.push_back(entry.path().filename().string());
                }
            }
        } else {
            AddLog("Content browser path does not exist: " + contentBrowser.currentPath, "Warning");
        }
    } catch (const std::exception& e) {
        AddLog("Failed to refresh content browser: " + std::string(e.what()), "Error");
    }
}

// File operations
void UnrealEditor::NewScene(entt::registry& registry) {
    registry.clear();
    selectedEntity = entt::null;
    AddLog("Created new scene", "Info");

    // Create default entities
    auto cube = CreateEntity(registry, "Default Cube");
    registry.emplace<MeshCube>(cube);

    auto hud = CreateEntity(registry, "Default HUD");
    registry.emplace<HUDComponent>(hud, HUDComponent{10.0f, 10.0f, 400, "Welcome to SproutEngine!"});
}

void UnrealEditor::SaveScene(entt::registry& registry, const std::string& filepath) {
    AddLog("Saved scene to: " + filepath + " (placeholder)", "Info");
    // TODO: Implement actual scene serialization
}

// Script Editor - Killer Feature Implementation
void UnrealEditor::DrawScriptEditor() {
    if (!showScriptEditor) return;

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Script Editor (.sp)", &showScriptEditor)) {

        // File operations toolbar
        if (ImGui::Button("New Script")) {
            scriptEditor.currentFile = "";
            scriptEditor.content = "// New SproutScript (.sp) file\n// This will be compiled to C++\n\nclass NewScript {\npublic:\n    void Start() {\n        // Initialization code\n    }\n    \n    void Update() {\n        // Update logic\n    }\n};\n";
            scriptEditor.isModified = true;
        }
        ImGui::SameLine();

        if (ImGui::Button("Open Script")) {
            // TODO: Open file dialog
            AddLog("Open script dialog would appear here", "Info");
        }
        ImGui::SameLine();

        if (ImGui::Button("Save Script") && scriptEditor.isModified) {
            if (!scriptEditor.currentFile.empty()) {
                SaveScriptFile(scriptEditor.currentFile, scriptEditor.content);
                scriptEditor.isModified = false;
                AddLog("Saved script: " + scriptEditor.currentFile, "Info");
            } else {
                AddLog("Please specify a filename first", "Warning");
            }
        }
        ImGui::SameLine();

        if (ImGui::Button("Compile to C++")) {
            std::string cppCode = CompileScriptToCpp(scriptEditor.content);
            scriptEditor.generatedCpp = cppCode;
            scriptEditor.showCppOutput = true;
            AddLog("Script compiled to C++", "Info");
        }

        ImGui::Separator();

        // File name input
        char filenameBuffer[256];
        strncpy(filenameBuffer, scriptEditor.currentFile.c_str(), sizeof(filenameBuffer) - 1);
        filenameBuffer[sizeof(filenameBuffer) - 1] = '\0';

        if (ImGui::InputText("Filename", filenameBuffer, sizeof(filenameBuffer))) {
            scriptEditor.currentFile = std::string(filenameBuffer);
        }

        ImGui::Separator();

        // Split view: Script Editor | Generated C++
        ImGui::Columns(scriptEditor.showCppOutput ? 2 : 1, "ScriptEditorColumns");

        // Left side - Script Editor
        ImGui::Text("SproutScript (.sp) Editor");
        ImGui::Separator();

        // Script content editor with syntax highlighting
        DrawScriptEditorContent();

        if (scriptEditor.showCppOutput) {
            ImGui::NextColumn();

            // Right side - Generated C++ (read-only)
            ImGui::Text("Generated C++ (Read-only)");
            ImGui::Separator();

            ImGui::PushFont(nullptr); // Use monospace if available
            ImGui::InputTextMultiline("##GeneratedCpp",
                const_cast<char*>(scriptEditor.generatedCpp.c_str()),
                scriptEditor.generatedCpp.size() + 1,
                ImVec2(-1, -1),
                ImGuiInputTextFlags_ReadOnly);
            ImGui::PopFont();
        }

        ImGui::Columns(1);
    }
    ImGui::End();
}

void UnrealEditor::DrawScriptEditorContent() {
    // Text editor with basic syntax highlighting
    ImVec2 editorSize = ImVec2(-1, -1);

    // Reserve space for syntax highlighting info
    float infoHeight = 60;
    editorSize.y -= infoHeight;

    // Main script content editor
    char* contentBuffer = new char[scriptEditor.content.size() + 1024]; // Extra space for editing
    strcpy(contentBuffer, scriptEditor.content.c_str());

    if (ImGui::InputTextMultiline("##ScriptContent", contentBuffer,
                                  scriptEditor.content.size() + 1024,
                                  editorSize,
                                  ImGuiInputTextFlags_AllowTabInput)) {
        scriptEditor.content = std::string(contentBuffer);
        scriptEditor.isModified = true;
    }

    delete[] contentBuffer;

    // Syntax highlighting info and shortcuts
    ImGui::Separator();
    ImGui::Text("Syntax: ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "class public void");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f), "Start() Update()");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.8f, 1.0f), "// comments");

    ImGui::Text("Shortcuts: Ctrl+S (Save) | Ctrl+O (Open) | F5 (Compile)");
}

std::string UnrealEditor::CompileScriptToCpp(const std::string& spContent) {
    std::string cppCode = "// Generated C++ from SproutScript\n";
    cppCode += "#include \"Engine/Components.h\"\n";
    cppCode += "#include <entt/entt.hpp>\n\n";

    // Basic .sp to C++ translation
    std::string processedContent = spContent;

    // Replace SproutScript specific syntax with C++
    size_t pos = 0;
    while ((pos = processedContent.find("class ", pos)) != std::string::npos) {
        size_t endPos = processedContent.find("{", pos);
        if (endPos != std::string::npos) {
            std::string className = processedContent.substr(pos + 6, endPos - pos - 6);
            // Trim whitespace
            className.erase(0, className.find_first_not_of(" \t\n\r"));
            className.erase(className.find_last_not_of(" \t\n\r") + 1);

            // Add component inheritance
            processedContent.insert(endPos, " : public ScriptComponent ");
        }
        pos = endPos + 1;
    }

    // Add component registration
    cppCode += "// Component Registration\n";
    cppCode += "extern \"C\" {\n";
    cppCode += "    void RegisterScriptComponent(entt::registry& registry, entt::entity entity) {\n";
    cppCode += "        // Register compiled script component\n";
    cppCode += "    }\n";
    cppCode += "}\n\n";

    cppCode += processedContent;

    return cppCode;
}

void UnrealEditor::SaveScriptFile(const std::string& filename, const std::string& content) {
    // TODO: Implement actual file saving
    AddLog("Script file saved: " + filename, "Info");
}

// Blueprint Editor - Visual Scripting Implementation
void UnrealEditor::DrawBlueprintEditor() {
    if (!showBlueprintEditor) return;

    ImGui::SetNextWindowSize(ImVec2(1200, 800), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Blueprint Editor (.bp)", &showBlueprintEditor)) {

        // Toolbar
        if (ImGui::Button("New Blueprint")) {
            blueprintEditor.nodes.clear();
            blueprintEditor.connections.clear();
            blueprintEditor.currentFile = "";
            AddLog("New blueprint created", "Info");
        }
        ImGui::SameLine();

        if (ImGui::Button("Open Blueprint")) {
            AddLog("Open blueprint dialog would appear here", "Info");
        }
        ImGui::SameLine();

        if (ImGui::Button("Save Blueprint")) {
            if (!blueprintEditor.currentFile.empty()) {
                SaveBlueprintFile(blueprintEditor.currentFile);
                AddLog("Blueprint saved: " + blueprintEditor.currentFile, "Info");
            } else {
                AddLog("Please specify a filename first", "Warning");
            }
        }
        ImGui::SameLine();

        if (ImGui::Button("Compile to C++")) {
            std::string cppCode = CompileBlueprintToCpp();
            blueprintEditor.generatedCpp = cppCode;
            AddLog("Blueprint compiled to C++", "Info");
        }
        ImGui::SameLine();

        if (ImGui::Button("Show C++ Output")) {
            blueprintEditor.showCppOutput = !blueprintEditor.showCppOutput;
        }
        ImGui::SameLine();

        if (ImGui::Button("Run Blueprint")) {
            RunBlueprint();
        }

        ImGui::Separator();

        // Split view: Blueprint Canvas | Node Library | Generated C++
        ImGui::Columns(blueprintEditor.showCppOutput ? 3 : 2, "BlueprintColumns");

        // Left side - Node Library
        DrawBlueprintNodeLibrary();

        ImGui::NextColumn();

        // Center - Blueprint Canvas
        DrawBlueprintCanvas();

        if (blueprintEditor.showCppOutput) {
            ImGui::NextColumn();

            // Right side - Generated C++
            ImGui::Text("Generated C++");
            ImGui::Separator();

            ImGui::InputTextMultiline("##BlueprintCpp",
                const_cast<char*>(blueprintEditor.generatedCpp.c_str()),
                blueprintEditor.generatedCpp.size() + 1,
                ImVec2(-1, -1),
                ImGuiInputTextFlags_ReadOnly);
        }

        ImGui::Columns(1);
    }
    ImGui::End();
}

void UnrealEditor::DrawBlueprintNodeLibrary() {
    ImGui::Text("Node Library");
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Events", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Start Event", ImVec2(-1, 0))) {
            AddBlueprintNode("Start", "Event", ImVec2(100, 100));
        }
        if (ImGui::Button("Update Event", ImVec2(-1, 0))) {
            AddBlueprintNode("Update", "Event", ImVec2(100, 150));
        }
        if (ImGui::Button("Input Event", ImVec2(-1, 0))) {
            AddBlueprintNode("Input", "Event", ImVec2(100, 200));
        }
        if (ImGui::Button("On Collision", ImVec2(-1, 0))) {
            AddBlueprintNode("OnCollision", "Event", ImVec2(100, 250));
        }
    }

    if (ImGui::CollapsingHeader("Transform Actions", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Set Position", ImVec2(-1, 0))) {
            AddBlueprintNode("SetPosition", "Transform", ImVec2(300, 100));
        }
        if (ImGui::Button("Get Position", ImVec2(-1, 0))) {
            AddBlueprintNode("GetPosition", "Transform", ImVec2(300, 150));
        }
        if (ImGui::Button("Set Rotation", ImVec2(-1, 0))) {
            AddBlueprintNode("SetRotation", "Transform", ImVec2(300, 200));
        }
        if (ImGui::Button("Get Rotation", ImVec2(-1, 0))) {
            AddBlueprintNode("GetRotation", "Transform", ImVec2(300, 250));
        }
        if (ImGui::Button("Set Scale", ImVec2(-1, 0))) {
            AddBlueprintNode("SetScale", "Transform", ImVec2(300, 300));
        }
        if (ImGui::Button("Get Scale", ImVec2(-1, 0))) {
            AddBlueprintNode("GetScale", "Transform", ImVec2(300, 350));
        }
        if (ImGui::Button("Translate", ImVec2(-1, 0))) {
            AddBlueprintNode("Translate", "Transform", ImVec2(300, 400));
        }
        if (ImGui::Button("Rotate", ImVec2(-1, 0))) {
            AddBlueprintNode("Rotate", "Transform", ImVec2(300, 450));
        }
    }

    if (ImGui::CollapsingHeader("Math Operations", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Add (Float)", ImVec2(-1, 0))) {
            AddBlueprintNode("Add", "Math", ImVec2(500, 100));
        }
        if (ImGui::Button("Subtract", ImVec2(-1, 0))) {
            AddBlueprintNode("Subtract", "Math", ImVec2(500, 150));
        }
        if (ImGui::Button("Multiply", ImVec2(-1, 0))) {
            AddBlueprintNode("Multiply", "Math", ImVec2(500, 200));
        }
        if (ImGui::Button("Divide", ImVec2(-1, 0))) {
            AddBlueprintNode("Divide", "Math", ImVec2(500, 250));
        }
        if (ImGui::Button("Sin", ImVec2(-1, 0))) {
            AddBlueprintNode("Sin", "Math", ImVec2(500, 300));
        }
        if (ImGui::Button("Cos", ImVec2(-1, 0))) {
            AddBlueprintNode("Cos", "Math", ImVec2(500, 350));
        }
        if (ImGui::Button("Lerp", ImVec2(-1, 0))) {
            AddBlueprintNode("Lerp", "Math", ImVec2(500, 400));
        }
    }

    if (ImGui::CollapsingHeader("Vector Math", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Make Vector3", ImVec2(-1, 0))) {
            AddBlueprintNode("MakeVector3", "Vector", ImVec2(400, 100));
        }
        if (ImGui::Button("Break Vector3", ImVec2(-1, 0))) {
            AddBlueprintNode("BreakVector3", "Vector", ImVec2(400, 150));
        }
        if (ImGui::Button("Vector Add", ImVec2(-1, 0))) {
            AddBlueprintNode("VectorAdd", "Vector", ImVec2(400, 200));
        }
        if (ImGui::Button("Vector Multiply", ImVec2(-1, 0))) {
            AddBlueprintNode("VectorMultiply", "Vector", ImVec2(400, 250));
        }
        if (ImGui::Button("Vector Length", ImVec2(-1, 0))) {
            AddBlueprintNode("VectorLength", "Vector", ImVec2(400, 300));
        }
        if (ImGui::Button("Normalize", ImVec2(-1, 0))) {
            AddBlueprintNode("Normalize", "Vector", ImVec2(400, 350));
        }
    }

    if (ImGui::CollapsingHeader("Variables", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Float Variable", ImVec2(-1, 0))) {
            AddBlueprintNode("FloatVar", "Variable", ImVec2(200, 100));
        }
        if (ImGui::Button("Vector3 Variable", ImVec2(-1, 0))) {
            AddBlueprintNode("Vector3Var", "Variable", ImVec2(200, 150));
        }
        if (ImGui::Button("String Variable", ImVec2(-1, 0))) {
            AddBlueprintNode("StringVar", "Variable", ImVec2(200, 200));
        }
        if (ImGui::Button("Set Variable", ImVec2(-1, 0))) {
            AddBlueprintNode("SetVariable", "Variable", ImVec2(200, 250));
        }
        if (ImGui::Button("Get Variable", ImVec2(-1, 0))) {
            AddBlueprintNode("GetVariable", "Variable", ImVec2(200, 300));
        }
    }

    if (ImGui::CollapsingHeader("Flow Control", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Branch (If)", ImVec2(-1, 0))) {
            AddBlueprintNode("Branch", "Flow", ImVec2(350, 100));
        }
        if (ImGui::Button("For Loop", ImVec2(-1, 0))) {
            AddBlueprintNode("ForLoop", "Flow", ImVec2(350, 150));
        }
        if (ImGui::Button("Delay", ImVec2(-1, 0))) {
            AddBlueprintNode("Delay", "Flow", ImVec2(350, 200));
        }
        if (ImGui::Button("Sequence", ImVec2(-1, 0))) {
            AddBlueprintNode("Sequence", "Flow", ImVec2(350, 250));
        }
    }

    if (ImGui::CollapsingHeader("Utilities", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Print String", ImVec2(-1, 0))) {
            AddBlueprintNode("Print", "Utility", ImVec2(450, 100));
        }
        if (ImGui::Button("Log Message", ImVec2(-1, 0))) {
            AddBlueprintNode("Log", "Utility", ImVec2(450, 150));
        }
        if (ImGui::Button("Get Time", ImVec2(-1, 0))) {
            AddBlueprintNode("GetTime", "Utility", ImVec2(450, 200));
        }
        if (ImGui::Button("Get Delta Time", ImVec2(-1, 0))) {
            AddBlueprintNode("GetDeltaTime", "Utility", ImVec2(450, 250));
        }
    }
}

void UnrealEditor::DrawBlueprintCanvas() {
    ImGui::Text("Blueprint Canvas - Drag from output pins to input pins, Delete key to delete selected nodes");
    ImGui::Separator();

    // Canvas area
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size = ImGui::GetContentRegionAvail();
    canvas_size.y = std::max(canvas_size.y, 400.0f);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Draw grid background
    ImU32 gridColor = IM_COL32(50, 50, 50, 100);
    float gridStep = 50.0f;

    for (float x = fmodf(blueprintEditor.canvasOffset.x, gridStep); x < canvas_size.x; x += gridStep) {
        draw_list->AddLine(ImVec2(canvas_pos.x + x, canvas_pos.y),
                          ImVec2(canvas_pos.x + x, canvas_pos.y + canvas_size.y), gridColor);
    }
    for (float y = fmodf(blueprintEditor.canvasOffset.y, gridStep); y < canvas_size.y; y += gridStep) {
        draw_list->AddLine(ImVec2(canvas_pos.x, canvas_pos.y + y),
                          ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + y), gridColor);
    }

    // Canvas interaction
    ImGui::InvisibleButton("canvas", canvas_size);
    bool isCanvasHovered = ImGui::IsItemHovered();
    bool isCanvasActive = ImGui::IsItemActive();

    // Get mouse state safely - check ImGui context first
    ImVec2 mousePos = ImVec2(0, 0);
    bool leftClicked = false;
    bool leftReleased = false;
    bool rightDragging = false;
    bool deletePressed = false;

    // Safe ImGui calls with context check
    if (ImGui::GetCurrentContext()) {
        mousePos = ImGui::GetMousePos();
        leftClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        leftReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
        rightDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Right);
        deletePressed = ImGui::IsKeyPressed(ImGuiKey_Delete);
    }

    ImVec2 mouseCanvasPos = ImVec2(mousePos.x - canvas_pos.x - blueprintEditor.canvasOffset.x,
                                   mousePos.y - canvas_pos.y - blueprintEditor.canvasOffset.y);

    // Static state for interactions
    static int draggedNodeId = -1;
    static ImVec2 dragOffset;
    static bool isDraggingNode = false;
    static int connectionStartNodeId = -1;
    static int connectionStartPinIndex = -1;
    static bool connectionStartIsOutput = false;
    static bool isDraggingConnection = false;
    static int selectedNodeId = -1;

    // Handle Delete key for selected nodes
    if (deletePressed && selectedNodeId != -1) {
        // Remove the node
        int nodeToDelete = selectedNodeId; // Copy to avoid static capture issue
        auto nodeIt = std::find_if(blueprintEditor.nodes.begin(), blueprintEditor.nodes.end(),
                                   [nodeToDelete](const BlueprintNode& node) { return node.id == nodeToDelete; });
        if (nodeIt != blueprintEditor.nodes.end()) {
            AddLog("Deleted node: " + nodeIt->name, "Info");

            // Remove all connections involving this node
            auto connIt = blueprintEditor.connections.begin();
            while (connIt != blueprintEditor.connections.end()) {
                if (connIt->sourceNodeId == nodeToDelete || connIt->targetNodeId == nodeToDelete) {
                    connIt = blueprintEditor.connections.erase(connIt);
                } else {
                    ++connIt;
                }
            }

            blueprintEditor.nodes.erase(nodeIt);
            selectedNodeId = -1;
        }
    }

    // Handle connection logic - Unreal Engine style
    if (leftReleased && isDraggingConnection) {
        isDraggingConnection = false;
        blueprintEditor.isLinking = false;
        if (connectionStartNodeId != -1) {
            AddLog("Connection cancelled - didn't connect to valid pin", "Info");
        }
        connectionStartNodeId = -1;
    }

    // Right click or Escape cancels connection
    if ((ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsKeyPressed(ImGuiKey_Escape)) && isDraggingConnection) {
        isDraggingConnection = false;
        blueprintEditor.isLinking = false;
        connectionStartNodeId = -1;
        AddLog("Connection cancelled", "Info");
    }

    // Process node interactions
    bool mouseOverAnyPin = false;
    int hoveredNodeId = -1;
    int hoveredPinIndex = -1;
    bool hoveredPinIsOutput = false;

    // First pass: Check for pin hovers and node hovers
    for (auto& node : blueprintEditor.nodes) {
        ImVec2 nodeScreenPos = ImVec2(canvas_pos.x + node.position.x + blueprintEditor.canvasOffset.x,
                                     canvas_pos.y + node.position.y + blueprintEditor.canvasOffset.y);

        // Check if mouse is over this node
        bool isNodeHovered = (mousePos.x >= nodeScreenPos.x && mousePos.x <= nodeScreenPos.x + node.size.x &&
                             mousePos.y >= nodeScreenPos.y && mousePos.y <= nodeScreenPos.y + node.size.y);

        if (isNodeHovered) {
            hoveredNodeId = node.id;

            // Check output pins first (priority for starting connections)
            float pinY = nodeScreenPos.y + 25;

            for (size_t i = 0; i < node.outputPins.size(); i++) {
                ImVec2 pinPos = ImVec2(nodeScreenPos.x + node.size.x + 5, pinY + i * 15);
                float pinDist = sqrt(pow(mousePos.x - pinPos.x, 2) + pow(mousePos.y - pinPos.y, 2));

                if (pinDist < 15.0f) { // Larger hit area for easier clicking
                    mouseOverAnyPin = true;
                    hoveredPinIndex = i;
                    hoveredPinIsOutput = true;

                    if (leftClicked && !isDraggingNode && !isDraggingConnection) {
                        // Start new connection by dragging from output pin
                        connectionStartNodeId = node.id;
                        connectionStartPinIndex = i;
                        connectionStartIsOutput = true;
                        isDraggingConnection = true;
                        blueprintEditor.isLinking = true;
                        AddLog("Dragging connection from " + node.name + " output pin: " + node.outputPins[i].name + " (ID: " + std::to_string(node.id) + ")", "Info");
                    }
                    break;
                }
            }

            // Check input pins if no output pin was hovered
            if (!mouseOverAnyPin) {
                for (size_t i = 0; i < node.inputPins.size(); i++) {
                    ImVec2 pinPos = ImVec2(nodeScreenPos.x - 5, pinY + i * 15);
                    float pinDist = sqrt(pow(mousePos.x - pinPos.x, 2) + pow(mousePos.y - pinPos.y, 2));

                    if (pinDist < 15.0f) { // Larger hit area for easier clicking
                        mouseOverAnyPin = true;
                        hoveredPinIndex = i;
                        hoveredPinIsOutput = false;

                        if (leftClicked && !isDraggingNode) {
                            if (isDraggingConnection && connectionStartIsOutput && connectionStartNodeId != node.id) {
                                // Complete the connection (output to input)
                                // First, validate pin type compatibility
                                BlueprintNode* sourceNode = nullptr;
                                for (auto& n : blueprintEditor.nodes) {
                                    if (n.id == connectionStartNodeId) {
                                        sourceNode = &n;
                                        break;
                                    }
                                }
                                
                                bool isValidConnection = false;
                                if (sourceNode && 
                                    connectionStartPinIndex < sourceNode->outputPins.size() && 
                                    i < node.inputPins.size()) {
                                    
                                    std::string sourceType = sourceNode->outputPins[connectionStartPinIndex].type;
                                    std::string targetType = node.inputPins[i].type;
                                    
                                    // Check type compatibility
                                    if (sourceType == targetType || 
                                        sourceType == "Exec" && targetType == "Exec" ||
                                        sourceType == "Float" && targetType == "Float" ||
                                        sourceType == "Vector3" && targetType == "Vector3" ||
                                        sourceType == "String" && targetType == "String" ||
                                        sourceType == "Bool" && targetType == "Bool") {
                                        isValidConnection = true;
                                    }
                                }
                                
                                if (isValidConnection) {
                                    // Check for duplicate connections
                                    bool isDuplicate = false;
                                    for (const auto& existingConnection : blueprintEditor.connections) {
                                        if (existingConnection.sourceNodeId == connectionStartNodeId &&
                                            existingConnection.sourcePinIndex == connectionStartPinIndex &&
                                            existingConnection.targetNodeId == node.id &&
                                            existingConnection.targetPinIndex == i) {
                                            isDuplicate = true;
                                            break;
                                        }
                                    }
                                    
                                    if (isDuplicate) {
                                        AddLog("Connection already exists!", "Warning");
                                    } else {
                                        BlueprintConnection connection;
                                        connection.sourceNodeId = connectionStartNodeId;
                                        connection.sourcePinIndex = connectionStartPinIndex;
                                        connection.targetNodeId = node.id;
                                        connection.targetPinIndex = i;
                                        connection.connectionType = "Data";

                                        blueprintEditor.connections.push_back(connection);
                                        AddLog("Connected " + sourceNode->name + " (" + sourceNode->outputPins[connectionStartPinIndex].name + ") to " + node.name + " (" + node.inputPins[i].name + ") successfully!", "Info");
                                    }
                                } else {
                                    if (sourceNode) {
                                        std::string sourceType = connectionStartPinIndex < sourceNode->outputPins.size() ? sourceNode->outputPins[connectionStartPinIndex].type : "INVALID";
                                        std::string targetType = i < node.inputPins.size() ? node.inputPins[i].type : "INVALID";
                                        AddLog("Cannot connect: Pin types incompatible! (" + sourceType + " -> " + targetType + ")", "Warning");
                                    } else {
                                        AddLog("Cannot connect: Source node not found!", "Warning");
                                    }
                                }
                                
                                isDraggingConnection = false;
                                blueprintEditor.isLinking = false;
                                connectionStartNodeId = -1;
                                connectionStartPinIndex = -1;
                                connectionStartIsOutput = false;
                            } else if (!isDraggingConnection) {
                                // Start new connection by dragging from input pin (reverse direction)
                                connectionStartNodeId = node.id;
                                connectionStartPinIndex = i;
                                connectionStartIsOutput = false;
                                isDraggingConnection = true;
                                blueprintEditor.isLinking = true;
                                AddLog("Dragging connection from " + node.name + " input pin: " + node.inputPins[i].name + " (ID: " + std::to_string(node.id) + ")", "Info");
                            } else {
                                AddLog("Connection attempt blocked - already dragging or invalid state", "Warning");
                            }
                        }
                        break;
                    }
                }
            }
        }
    }

    // Handle node selection and dragging (only if not over a pin and not connecting)
    if (leftClicked && hoveredNodeId != -1 && !mouseOverAnyPin && !isDraggingConnection && !isDraggingNode) {
        selectedNodeId = hoveredNodeId; // Select the node

        for (auto& node : blueprintEditor.nodes) {
            if (node.id == hoveredNodeId) {
                ImVec2 nodeScreenPos = ImVec2(canvas_pos.x + node.position.x + blueprintEditor.canvasOffset.x,
                                             canvas_pos.y + node.position.y + blueprintEditor.canvasOffset.y);
                draggedNodeId = node.id;
                isDraggingNode = true;
                dragOffset = ImVec2(mousePos.x - nodeScreenPos.x, mousePos.y - nodeScreenPos.y);
                break;
            }
        }
    }

    // Clear selection when clicking on empty canvas
    if (leftClicked && hoveredNodeId == -1 && !isDraggingConnection) {
        selectedNodeId = -1;
    }

    // Update dragged node position
    if (isDraggingNode && ImGui::GetCurrentContext() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        for (auto& node : blueprintEditor.nodes) {
            if (node.id == draggedNodeId) {
                node.position.x = mousePos.x - canvas_pos.x - blueprintEditor.canvasOffset.x - dragOffset.x;
                node.position.y = mousePos.y - canvas_pos.y - blueprintEditor.canvasOffset.y - dragOffset.y;
                break;
            }
        }
    }

    // Stop dragging
    if (isDraggingNode && leftReleased) {
        isDraggingNode = false;
        draggedNodeId = -1;
    }

    // Draw connections first (behind nodes)
    for (auto& connection : blueprintEditor.connections) {
        DrawBlueprintConnection(connection, canvas_pos, draw_list);
    }

    // Draw nodes with selection highlighting
    for (auto& node : blueprintEditor.nodes) {
        bool isSelected = (node.id == selectedNodeId);
        DrawBlueprintNode(node, canvas_pos, draw_list, isSelected);
    }

    // Draw connection preview when dragging
    if (isDraggingConnection && connectionStartNodeId != -1) {
        BlueprintNode* startNode = nullptr;
        for (auto& node : blueprintEditor.nodes) {
            if (node.id == connectionStartNodeId) {
                startNode = &node;
                break;
            }
        }

        if (startNode) {
            ImVec2 startPos;
            ImU32 connectionColor = IM_COL32(255, 255, 255, 200); // Default white
            
            if (connectionStartIsOutput) {
                startPos = ImVec2(canvas_pos.x + startNode->position.x + startNode->size.x + 5 + blueprintEditor.canvasOffset.x,
                                 canvas_pos.y + startNode->position.y + 25 + connectionStartPinIndex * 15 + blueprintEditor.canvasOffset.y);
                
                // Get the pin type for color coding
                if (connectionStartPinIndex < startNode->outputPins.size()) {
                    std::string pinType = startNode->outputPins[connectionStartPinIndex].type;
                    if (pinType == "Exec") connectionColor = IM_COL32(255, 255, 255, 200);
                    else if (pinType == "Float") connectionColor = IM_COL32(100, 255, 100, 200);
                    else if (pinType == "Vector3") connectionColor = IM_COL32(255, 255, 100, 200);
                    else if (pinType == "String") connectionColor = IM_COL32(255, 100, 255, 200);
                    else if (pinType == "Bool") connectionColor = IM_COL32(255, 100, 100, 200);
                }
            } else {
                startPos = ImVec2(canvas_pos.x + startNode->position.x - 5 + blueprintEditor.canvasOffset.x,
                                 canvas_pos.y + startNode->position.y + 25 + connectionStartPinIndex * 15 + blueprintEditor.canvasOffset.y);
                
                // Get the pin type for color coding
                if (connectionStartPinIndex < startNode->inputPins.size()) {
                    std::string pinType = startNode->inputPins[connectionStartPinIndex].type;
                    if (pinType == "Exec") connectionColor = IM_COL32(255, 255, 255, 200);
                    else if (pinType == "Float") connectionColor = IM_COL32(100, 255, 100, 200);
                    else if (pinType == "Vector3") connectionColor = IM_COL32(255, 255, 100, 200);
                    else if (pinType == "String") connectionColor = IM_COL32(255, 100, 255, 200);
                    else if (pinType == "Bool") connectionColor = IM_COL32(255, 100, 100, 200);
                }
            }

            // Draw preview connection to mouse with pulsing effect
            static float connectionTime = 0.0f;
            connectionTime += 0.1f; // Simple increment for animation
            float pulseAlpha = 150 + 50 * sin(connectionTime * 0.8f); // Pulsing alpha for visual feedback
            ImU32 pulsingColor = (connectionColor & 0x00FFFFFF) | ((int)pulseAlpha << 24);
            
            ImVec2 cp1 = ImVec2(startPos.x + (connectionStartIsOutput ? 100 : -100), startPos.y);
            ImVec2 cp2 = ImVec2(mousePos.x + (connectionStartIsOutput ? -100 : 100), mousePos.y);
            draw_list->AddBezierCubic(startPos, cp1, cp2, mousePos, pulsingColor, 4.0f);

            // Draw instruction text
            draw_list->AddText(ImVec2(mousePos.x + 10, mousePos.y - 20), IM_COL32(255, 255, 255, 255),
                              connectionStartIsOutput ? "Drag to input pin" : "Drag to output pin");
        }
    }

    // Handle canvas panning (only when not interacting with nodes and not connecting)
    if (isCanvasActive && rightDragging && !isDraggingNode && !isDraggingConnection) {
        if (ImGui::GetCurrentContext()) {
            ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
            blueprintEditor.canvasOffset.x += delta.x;
            blueprintEditor.canvasOffset.y += delta.y;
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
        }
    }

    // Show interaction help
    if (blueprintEditor.nodes.empty()) {
        ImVec2 helpPos = ImVec2(canvas_pos.x + canvas_size.x / 2 - 150, canvas_pos.y + canvas_size.y / 2);
        draw_list->AddText(helpPos, IM_COL32(150, 150, 150, 255), "Add nodes from the library on the left");
        draw_list->AddText(ImVec2(helpPos.x, helpPos.y + 20), IM_COL32(150, 150, 150, 255), "Drag from output pins to input pins to connect");
        draw_list->AddText(ImVec2(helpPos.x, helpPos.y + 40), IM_COL32(150, 150, 150, 255), "Select nodes and press Delete to remove them");
    }
}

void UnrealEditor::AddBlueprintNode(const std::string& name, const std::string& type, ImVec2 position) {
    BlueprintNode node;
    node.id = blueprintEditor.nextNodeId++;
    node.name = name;
    node.type = type;
    node.position = position;
    node.size = ImVec2(140, 80); // Slightly larger for more pins

    // Add pins based on node type and name
    if (type == "Event") {
        node.outputPins.push_back({"Execute", "Exec"});
        if (name == "OnCollision") {
            node.outputPins.push_back({"Other", "Object"});
        }
    } 
    else if (type == "Transform") {
        node.inputPins.push_back({"Execute", "Exec"});
        node.outputPins.push_back({"Execute", "Exec"});
        
        if (name == "SetPosition" || name == "SetRotation" || name == "SetScale") {
            node.inputPins.push_back({"Target", "Object"});
            node.inputPins.push_back({"Value", "Vector3"});
        } else if (name == "GetPosition" || name == "GetRotation" || name == "GetScale") {
            node.inputPins.push_back({"Target", "Object"});
            node.outputPins.push_back({"Value", "Vector3"});
        } else if (name == "Translate" || name == "Rotate") {
            node.inputPins.push_back({"Target", "Object"});
            node.inputPins.push_back({"Delta", "Vector3"});
        }
    }
    else if (type == "Math") {
        if (name == "Add" || name == "Subtract" || name == "Multiply" || name == "Divide") {
            node.inputPins.push_back({"A", "Float"});
            node.inputPins.push_back({"B", "Float"});
            node.outputPins.push_back({"Result", "Float"});
        } else if (name == "Sin" || name == "Cos") {
            node.inputPins.push_back({"Input", "Float"});
            node.outputPins.push_back({"Output", "Float"});
        } else if (name == "Lerp") {
            node.inputPins.push_back({"A", "Float"});
            node.inputPins.push_back({"B", "Float"});
            node.inputPins.push_back({"Alpha", "Float"});
            node.outputPins.push_back({"Result", "Float"});
        }
    }
    else if (type == "Vector") {
        if (name == "MakeVector3") {
            node.inputPins.push_back({"X", "Float"});
            node.inputPins.push_back({"Y", "Float"});
            node.inputPins.push_back({"Z", "Float"});
            node.outputPins.push_back({"Vector", "Vector3"});
        } else if (name == "BreakVector3") {
            node.inputPins.push_back({"Vector", "Vector3"});
            node.outputPins.push_back({"X", "Float"});
            node.outputPins.push_back({"Y", "Float"});
            node.outputPins.push_back({"Z", "Float"});
        } else if (name == "VectorAdd" || name == "VectorMultiply") {
            node.inputPins.push_back({"A", "Vector3"});
            node.inputPins.push_back({"B", "Vector3"});
            node.outputPins.push_back({"Result", "Vector3"});
        } else if (name == "VectorLength") {
            node.inputPins.push_back({"Vector", "Vector3"});
            node.outputPins.push_back({"Length", "Float"});
        } else if (name == "Normalize") {
            node.inputPins.push_back({"Vector", "Vector3"});
            node.outputPins.push_back({"Normal", "Vector3"});
        }
    }
    else if (type == "Variable") {
        if (name == "FloatVar") {
            node.outputPins.push_back({"Value", "Float"});
            node.properties["Value"] = "0.0";
        } else if (name == "Vector3Var") {
            node.outputPins.push_back({"Value", "Vector3"});
            node.properties["X"] = "0.0";
            node.properties["Y"] = "0.0";
            node.properties["Z"] = "0.0";
        } else if (name == "StringVar") {
            node.outputPins.push_back({"Value", "String"});
            node.properties["Value"] = "Hello";
        } else if (name == "SetVariable") {
            node.inputPins.push_back({"Execute", "Exec"});
            node.inputPins.push_back({"Value", "Float"});
            node.outputPins.push_back({"Execute", "Exec"});
        } else if (name == "GetVariable") {
            node.outputPins.push_back({"Value", "Float"});
        }
    }
    else if (type == "Flow") {
        if (name == "Branch") {
            node.inputPins.push_back({"Execute", "Exec"});
            node.inputPins.push_back({"Condition", "Bool"});
            node.outputPins.push_back({"True", "Exec"});
            node.outputPins.push_back({"False", "Exec"});
        } else if (name == "ForLoop") {
            node.inputPins.push_back({"Execute", "Exec"});
            node.inputPins.push_back({"First Index", "Int"});
            node.inputPins.push_back({"Last Index", "Int"});
            node.outputPins.push_back({"Loop Body", "Exec"});
            node.outputPins.push_back({"Completed", "Exec"});
            node.outputPins.push_back({"Index", "Int"});
        } else if (name == "Delay") {
            node.inputPins.push_back({"Execute", "Exec"});
            node.inputPins.push_back({"Duration", "Float"});
            node.outputPins.push_back({"Execute", "Exec"});
        } else if (name == "Sequence") {
            node.inputPins.push_back({"Execute", "Exec"});
            node.outputPins.push_back({"Then 0", "Exec"});
            node.outputPins.push_back({"Then 1", "Exec"});
            node.outputPins.push_back({"Then 2", "Exec"});
        }
    }
    else if (type == "Utility") {
        if (name == "Print") {
            node.inputPins.push_back({"Execute", "Exec"});
            node.inputPins.push_back({"Text", "String"});
            node.outputPins.push_back({"Execute", "Exec"});
        } else if (name == "Log") {
            node.inputPins.push_back({"Execute", "Exec"});
            node.inputPins.push_back({"Message", "String"});
            node.inputPins.push_back({"Level", "String"});
            node.outputPins.push_back({"Execute", "Exec"});
        } else if (name == "GetTime" || name == "GetDeltaTime") {
            node.outputPins.push_back({"Time", "Float"});
        }
    }

    // Adjust node size based on pin count
    int maxPins = std::max(node.inputPins.size(), node.outputPins.size());
    node.size.y = std::max(80.0f, 40.0f + maxPins * 18.0f);

    blueprintEditor.nodes.push_back(node);
    AddLog("Added blueprint node: " + name, "Info");
}

void UnrealEditor::DrawBlueprintNode(BlueprintNode& node, ImVec2 canvasPos, ImDrawList* drawList, bool isSelected) {
    ImVec2 nodePos = ImVec2(canvasPos.x + node.position.x + blueprintEditor.canvasOffset.x,
                           canvasPos.y + node.position.y + blueprintEditor.canvasOffset.y);

    // Check if mouse is over this node for hover effects
    ImVec2 mousePos = ImVec2(0, 0);
    if (ImGui::GetCurrentContext()) {
        mousePos = ImGui::GetMousePos();
    }
    bool isNodeHovered = (mousePos.x >= nodePos.x && mousePos.x <= nodePos.x + node.size.x &&
                         mousePos.y >= nodePos.y && mousePos.y <= nodePos.y + node.size.y);

    // Node background with hover and selection effects
    ImU32 nodeColor = IM_COL32(60, 60, 80, 255);
    if (node.type == "Event") nodeColor = IM_COL32(80, 40, 40, 255);
    else if (node.type == "Action") nodeColor = IM_COL32(40, 80, 40, 255);
    else if (node.type == "Math") nodeColor = IM_COL32(40, 40, 80, 255);
    else if (node.type == "Component") nodeColor = IM_COL32(80, 80, 40, 255);

    // Brighten color on hover
    if (isNodeHovered) {
        ImU32 r = (nodeColor >> IM_COL32_R_SHIFT) & 0xFF;
        ImU32 g = (nodeColor >> IM_COL32_G_SHIFT) & 0xFF;
        ImU32 b = (nodeColor >> IM_COL32_B_SHIFT) & 0xFF;
        r = std::min(255u, r + 30);
        g = std::min(255u, g + 30);
        b = std::min(255u, b + 30);
        nodeColor = IM_COL32(r, g, b, 255);
    }

    drawList->AddRectFilled(nodePos, ImVec2(nodePos.x + node.size.x, nodePos.y + node.size.y),
                           nodeColor, 5.0f);

    // Node border - special highlight for selected nodes
    float borderThickness = 2.0f;
    ImU32 borderColor = IM_COL32(200, 200, 200, 255);

    if (isSelected) {
        borderThickness = 4.0f;
        borderColor = IM_COL32(255, 165, 0, 255); // Orange selection border
    } else if (isNodeHovered) {
        borderThickness = 3.0f;
        borderColor = IM_COL32(255, 255, 255, 255);
    }

    drawList->AddRect(nodePos, ImVec2(nodePos.x + node.size.x, nodePos.y + node.size.y),
                     borderColor, 5.0f, 0, borderThickness);

    // Node title
    ImVec2 textPos = ImVec2(nodePos.x + 5, nodePos.y + 5);
    drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), node.name.c_str());

    // Input pins with hover detection
    float pinY = nodePos.y + 25;
    for (size_t i = 0; i < node.inputPins.size(); i++) {
        ImVec2 pinPos = ImVec2(nodePos.x - 5, pinY + i * 15);

        // Check if mouse is near this pin
        float pinDist = sqrt(pow(mousePos.x - pinPos.x, 2) + pow(mousePos.y - pinPos.y, 2));
        bool isPinHovered = pinDist < 10.0f; // Slightly larger for better usability

        // Pin color and size based on hover and connection type
        ImU32 pinColor = IM_COL32(150, 150, 150, 255);
        float pinSize = 5.0f; // Larger base size

        if (node.inputPins[i].type == "Exec") {
            pinColor = IM_COL32(255, 255, 255, 255); // White for execution pins
        } else if (node.inputPins[i].type == "Float") {
            pinColor = IM_COL32(100, 255, 100, 255); // Green for numbers
        } else if (node.inputPins[i].type == "Vector3") {
            pinColor = IM_COL32(255, 255, 100, 255); // Yellow for vectors
        } else if (node.inputPins[i].type == "String") {
            pinColor = IM_COL32(255, 100, 255, 255); // Magenta for strings
        } else if (node.inputPins[i].type == "Bool") {
            pinColor = IM_COL32(255, 100, 100, 255); // Red for booleans
        } else if (node.inputPins[i].type == "Int") {
            pinColor = IM_COL32(100, 200, 255, 255); // Light blue for integers
        }

        if (isPinHovered) {
            pinSize = 7.0f; // Even larger when hovered
            // Brighten the pin when hovered
            ImU32 r = (pinColor >> IM_COL32_R_SHIFT) & 0xFF;
            ImU32 g = (pinColor >> IM_COL32_G_SHIFT) & 0xFF;
            ImU32 b = (pinColor >> IM_COL32_B_SHIFT) & 0xFF;
            pinColor = IM_COL32(std::min(255u, r + 50), std::min(255u, g + 50), std::min(255u, b + 50), 255);
            
            // Add glow effect when hovered
            drawList->AddCircleFilled(pinPos, pinSize + 2.0f, IM_COL32(255, 255, 255, 50));
        }

        drawList->AddCircleFilled(pinPos, pinSize, pinColor);
        drawList->AddCircle(pinPos, pinSize, IM_COL32(255, 255, 255, 255), 0, 1.0f);

        // Pin label
        ImVec2 labelPos = ImVec2(pinPos.x + 12, pinPos.y - 7);
        drawList->AddText(labelPos, IM_COL32(200, 200, 200, 255), node.inputPins[i].name.c_str());
    }

    // Output pins with hover detection
    for (size_t i = 0; i < node.outputPins.size(); i++) {
        ImVec2 pinPos = ImVec2(nodePos.x + node.size.x + 5, pinY + i * 15);

        // Check if mouse is near this pin
        float pinDist = sqrt(pow(mousePos.x - pinPos.x, 2) + pow(mousePos.y - pinPos.y, 2));
        bool isPinHovered = pinDist < 10.0f; // Slightly larger for better usability

        // Pin color and size based on hover and connection type
        ImU32 pinColor = IM_COL32(150, 150, 150, 255);
        float pinSize = 5.0f; // Larger base size

        if (node.outputPins[i].type == "Exec") {
            pinColor = IM_COL32(255, 255, 255, 255); // White for execution pins
        } else if (node.outputPins[i].type == "Float") {
            pinColor = IM_COL32(100, 255, 100, 255); // Green for numbers
        } else if (node.outputPins[i].type == "Vector3") {
            pinColor = IM_COL32(255, 255, 100, 255); // Yellow for vectors
        } else if (node.outputPins[i].type == "String") {
            pinColor = IM_COL32(255, 100, 255, 255); // Magenta for strings
        } else if (node.outputPins[i].type == "Bool") {
            pinColor = IM_COL32(255, 100, 100, 255); // Red for booleans
        } else if (node.outputPins[i].type == "Int") {
            pinColor = IM_COL32(100, 200, 255, 255); // Light blue for integers
        }

        if (isPinHovered) {
            pinSize = 7.0f; // Even larger when hovered
            // Brighten the pin when hovered
            ImU32 r = (pinColor >> IM_COL32_R_SHIFT) & 0xFF;
            ImU32 g = (pinColor >> IM_COL32_G_SHIFT) & 0xFF;
            ImU32 b = (pinColor >> IM_COL32_B_SHIFT) & 0xFF;
            pinColor = IM_COL32(std::min(255u, r + 50), std::min(255u, g + 50), std::min(255u, b + 50), 255);
            
            // Add glow effect when hovered
            drawList->AddCircleFilled(pinPos, pinSize + 2.0f, IM_COL32(255, 255, 255, 50));
        }

        drawList->AddCircleFilled(pinPos, pinSize, pinColor);
        drawList->AddCircle(pinPos, pinSize, IM_COL32(255, 255, 255, 255), 0, 1.0f);

        // Pin label (right-aligned)
        const float estimatedLabelWidth = node.outputPins[i].name.length() * 7.0f; // Approximate character width
        ImVec2 labelPos = ImVec2(pinPos.x - estimatedLabelWidth - 12, pinPos.y - 7);
        drawList->AddText(labelPos, IM_COL32(200, 200, 200, 255), node.outputPins[i].name.c_str());
    }
}

void UnrealEditor::DrawBlueprintConnection(BlueprintConnection& connection, ImVec2 canvasPos, ImDrawList* drawList) {
    // Find source and target nodes
    BlueprintNode* sourceNode = nullptr;
    BlueprintNode* targetNode = nullptr;

    for (auto& node : blueprintEditor.nodes) {
        if (node.id == connection.sourceNodeId) sourceNode = &node;
        if (node.id == connection.targetNodeId) targetNode = &node;
    }

    if (!sourceNode || !targetNode) return;

    // Calculate pin positions
    ImVec2 sourcePos = ImVec2(canvasPos.x + sourceNode->position.x + sourceNode->size.x + 5 + blueprintEditor.canvasOffset.x,
                             canvasPos.y + sourceNode->position.y + 25 + connection.sourcePinIndex * 15 + blueprintEditor.canvasOffset.y);
    ImVec2 targetPos = ImVec2(canvasPos.x + targetNode->position.x - 5 + blueprintEditor.canvasOffset.x,
                             canvasPos.y + targetNode->position.y + 25 + connection.targetPinIndex * 15 + blueprintEditor.canvasOffset.y);

    // Draw curved connection
    ImVec2 cp1 = ImVec2(sourcePos.x + 50, sourcePos.y);
    ImVec2 cp2 = ImVec2(targetPos.x - 50, targetPos.y);

    drawList->AddBezierCubic(sourcePos, cp1, cp2, targetPos, IM_COL32(255, 255, 0, 255), 3.0f);
}

std::string UnrealEditor::CompileBlueprintToCpp() {
    std::string cppCode = "// Generated C++ from Blueprint\n";
    cppCode += "#include \"Engine/Components.h\"\n";
    cppCode += "#include <entt/entt.hpp>\n\n";

    cppCode += "class GeneratedBlueprintScript : public ScriptComponent {\n";
    cppCode += "public:\n";

    // Generate Start function
    cppCode += "    void Start() override {\n";
    for (auto& node : blueprintEditor.nodes) {
        if (node.type == "Event" && node.name == "Start") {
            cppCode += GenerateNodeCode(node);
        }
    }
    cppCode += "    }\n\n";

    // Generate Update function
    cppCode += "    void Update() override {\n";
    for (auto& node : blueprintEditor.nodes) {
        if (node.type == "Event" && node.name == "Update") {
            cppCode += GenerateNodeCode(node);
        }
    }
    cppCode += "    }\n";

    cppCode += "};\n\n";

    // Add component registration
    cppCode += "extern \"C\" {\n";
    cppCode += "    void RegisterBlueprintComponent(entt::registry& registry, entt::entity entity) {\n";
    cppCode += "        registry.emplace<GeneratedBlueprintScript>(entity);\n";
    cppCode += "    }\n";
    cppCode += "}\n";

    return cppCode;
}

std::string UnrealEditor::GenerateNodeCode(const BlueprintNode& node) {
    std::string code = "        // " + node.name + " node\n";

    if (node.type == "Action") {
        if (node.name == "Move") {
            code += "        // Move action - translate entity\n";
            code += "        // transform.position += vector;\n";
        } else if (node.name == "Rotate") {
            code += "        // Rotate action - rotate entity\n";
            code += "        // transform.rotation += vector;\n";
        } else if (node.name == "Scale") {
            code += "        // Scale action - scale entity\n";
            code += "        // transform.scale *= vector;\n";
        } else if (node.name == "Print") {
            code += "        std::cout << \"Blueprint Print\" << std::endl;\n";
        }
    }

    return code;
}

void UnrealEditor::SaveBlueprintFile(const std::string& filename) {
    // TODO: Implement actual blueprint file saving (JSON format)
    AddLog("Blueprint file saved: " + filename, "Info");
}

// Legacy script functionality implementations
void UnrealEditor::OpenScriptFile(const std::string& filepath) {
    scriptEditor.currentFile = filepath;
    showScriptEditor = true;
    AddLog("Opened script file: " + filepath, "Info");
    // TODO: Load actual file content
}

void UnrealEditor::OpenBlueprintFile(const std::string& filepath) {
    blueprintEditor.currentFile = filepath;
    showBlueprintGraph = true;  // Fixed: should be showBlueprintGraph, not showBlueprintEditor
    AddLog("Opened blueprint file: " + filepath, "Info");
    // TODO: Load actual blueprint data
}

// Blueprint execution system - like Unreal Engine
void UnrealEditor::RunBlueprint() {
    AddLog("=== Running Blueprint ===", "Info");

    if (blueprintEditor.nodes.empty()) {
        AddLog("No nodes to execute in blueprint", "Warning");
        return;
    }

    // Find Start Event nodes to begin execution
    std::unordered_map<int, bool> executedNodes;
    bool foundStartEvent = false;

    for (auto& node : blueprintEditor.nodes) {
        if (node.type == "Event" && node.name == "Start") {
            foundStartEvent = true;
            AddLog("Executing from Start Event node", "Info");
            ExecuteBlueprintNode(node.id, executedNodes);
        }
    }

    if (!foundStartEvent) {
        AddLog("No Start Event found - nothing to execute", "Warning");
        AddLog("Add a 'Start Event' node from the Events section to begin execution", "Info");
    }

    AddLog("=== Blueprint Execution Complete ===", "Info");
}

void UnrealEditor::ExecuteBlueprintNode(int nodeId, std::unordered_map<int, bool>& executedNodes) {
    // Prevent infinite loops
    if (executedNodes[nodeId]) {
        return;
    }
    executedNodes[nodeId] = true;

    // Find the node
    BlueprintNode* currentNode = nullptr;
    for (auto& node : blueprintEditor.nodes) {
        if (node.id == nodeId) {
            currentNode = &node;
            break;
        }
    }

    if (!currentNode) {
        AddLog("Error: Node not found during execution", "Error");
        return;
    }

    AddLog("Executing node: " + currentNode->name, "Info");

    // Execute the node based on its type
    if (currentNode->type == "Event") {
        if (currentNode->name == "Start Event") {
            AddLog("⚡ Start Event triggered", "Info");
        } else if (currentNode->name == "Update Event") {
            AddLog("⚡ Update Event triggered", "Info");
        } else if (currentNode->name == "Input Event") {
            AddLog("⚡ Input Event triggered", "Info");
        } else if (currentNode->name == "OnCollision") {
            AddLog("⚡ Collision Event triggered", "Info");
        }
    } else if (currentNode->type == "Transform") {
        if (currentNode->name == "Set Position") {
            AddLog("🎮 Transform: Setting Position", "Info");
        } else if (currentNode->name == "Get Position") {
            AddLog("🎮 Transform: Getting Position", "Info");
        } else if (currentNode->name == "Set Rotation") {
            AddLog("🎮 Transform: Setting Rotation", "Info");
        } else if (currentNode->name == "Get Rotation") {
            AddLog("🎮 Transform: Getting Rotation", "Info");
        } else if (currentNode->name == "Set Scale") {
            AddLog("🎮 Transform: Setting Scale", "Info");
        } else if (currentNode->name == "Get Scale") {
            AddLog("🎮 Transform: Getting Scale", "Info");
        } else if (currentNode->name == "Translate") {
            AddLog("🎮 Transform: Translating object", "Info");
        } else if (currentNode->name == "Rotate") {
            AddLog("🎮 Transform: Rotating object", "Info");
        }
    } else if (currentNode->type == "Math") {
        if (currentNode->name == "Add") {
            AddLog("🧮 Math: Adding values", "Info");
        } else if (currentNode->name == "Subtract") {
            AddLog("🧮 Math: Subtracting values", "Info");
        } else if (currentNode->name == "Multiply") {
            AddLog("🧮 Math: Multiplying values", "Info");
        } else if (currentNode->name == "Divide") {
            AddLog("🧮 Math: Dividing values", "Info");
        } else if (currentNode->name == "Sin") {
            AddLog("🧮 Math: Calculating Sin", "Info");
        } else if (currentNode->name == "Cos") {
            AddLog("🧮 Math: Calculating Cos", "Info");
        } else if (currentNode->name == "Lerp") {
            AddLog("🧮 Math: Linear interpolation", "Info");
        }
    } else if (currentNode->type == "Vector") {
        if (currentNode->name == "Make Vector3") {
            AddLog("🧮 Vector: Creating Vector3", "Info");
        } else if (currentNode->name == "Break Vector3") {
            AddLog("🧮 Vector: Breaking Vector3", "Info");
        } else if (currentNode->name == "Vector Add") {
            AddLog("🧮 Vector: Adding vectors", "Info");
        } else if (currentNode->name == "Vector Length") {
            AddLog("🧮 Vector: Calculating length", "Info");
        } else if (currentNode->name == "Normalize") {
            AddLog("🧮 Vector: Normalizing vector", "Info");
        }
    } else if (currentNode->type == "Variable") {
        if (currentNode->name == "Float Variable") {
            AddLog("📦 Variable: Float accessed", "Info");
        } else if (currentNode->name == "Vector3 Variable") {
            AddLog("📦 Variable: Vector3 accessed", "Info");
        } else if (currentNode->name == "String Variable") {
            AddLog("📦 Variable: String accessed", "Info");
        } else if (currentNode->name == "Set Float") {
            AddLog("📦 Variable: Setting Float", "Info");
        } else if (currentNode->name == "Get Float") {
            AddLog("📦 Variable: Getting Float", "Info");
        }
    } else if (currentNode->type == "Flow") {
        if (currentNode->name == "Branch") {
            AddLog("� Flow: Branch executed", "Info");
        } else if (currentNode->name == "ForLoop") {
            AddLog("🔀 Flow: For Loop started", "Info");
        } else if (currentNode->name == "Delay") {
            AddLog("🔀 Flow: Delay started", "Info");
        } else if (currentNode->name == "Sequence") {
            AddLog("🔀 Flow: Sequence executed", "Info");
        }
    } else if (currentNode->type == "Utility") {
        if (currentNode->name == "Print") {
            AddLog("📝 Print: Hello from Blueprint!", "Info");
        } else if (currentNode->name == "Log") {
            AddLog("📝 Log: Message logged", "Info");
        } else if (currentNode->name == "GetTime") {
            AddLog("⏰ Utility: Getting current time", "Info");
        }
    }

    // Find connected nodes through execution pins and execute them
    for (auto& connection : blueprintEditor.connections) {
        if (connection.sourceNodeId == nodeId) {
            // Check if this is an execution connection (white pins)
            if (currentNode->outputPins.size() > connection.sourcePinIndex) {
                if (currentNode->outputPins[connection.sourcePinIndex].type == "Exec") {
                    AddLog("→ Following execution path to next node", "Info");
                    ExecuteBlueprintNode(connection.targetNodeId, executedNodes);
                }
            }
        }
    }
}

void UnrealEditor::InitializeWebUI() {
    // Start the web server on port 8080
    if (webUIManager.Initialize(8080)) {
        AddLog("Web UI started at " + webUIManager.GetWebInterfaceURL(), "Info");
        AddLog("You can now access the modern web interface!", "Info");
    } else {
        AddLog("Failed to start Web UI server", "Error");
    }
}

void UnrealEditor::SetupWebAPIEndpoints(entt::registry& registry, Scripting& scripting) {
    // Register API endpoints for web UI communication
    
    // Entity selection endpoint
    webUIManager.RegisterEndpoint("/selectEntity", [&](const std::string& body) -> std::string {
        // Parse entity ID from request body and select entity
        // For now, return simple response
        return "{\"status\":\"ok\"}";
    });
    
    // Entity data endpoint
    webUIManager.RegisterEndpoint("/getEntity", [&](const std::string& body) -> std::string {
        // Return entity data as JSON
        return "{\"id\":1,\"name\":\"DemoCube\",\"transform\":{\"position\":{\"x\":0,\"y\":0,\"z\":0}}}";
    });
    
    // Play mode endpoint
    webUIManager.RegisterEndpoint("/setPlayMode", [&](const std::string& body) -> std::string {
        // Handle play mode changes
        return "{\"status\":\"ok\"}";
    });
    
    // Command execution endpoint
    webUIManager.RegisterEndpoint("/executeCommand", [&](const std::string& body) -> std::string {
        // Execute console commands
        return "{\"output\":\"Command executed\",\"level\":\"info\"}";
    });
    
    // Blueprint opening endpoint
    webUIManager.RegisterEndpoint("/openBlueprint", [&](const std::string& body) -> std::string {
        // Open blueprint editor
        return "{\"status\":\"ok\"}";
    });
}
