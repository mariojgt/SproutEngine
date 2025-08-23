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

UnrealEditor::UnrealEditor() {
}

UnrealEditor::~UnrealEditor() {
}

bool UnrealEditor::Init(GLFWwindow* window) {
    editorWindow = window;
    
    // Don't create ImGui context - TinyImGui already did that
    // Just store that we don't own the backends
    ownsImGuiBackends = false;

    // Initialize console with welcome message
    AddLog("=== SproutEngine Unreal-like Editor Started ===", "System");
    AddLog("Welcome to SproutEngine - Your lightweight Unreal alternative!", "Info");
    AddLog("Type 'help' for available commands", "Info");

    // Refresh content browser
    RefreshContentBrowser();

    return true;
}

void UnrealEditor::Shutdown(GLFWwindow* window) {
    if (ownsImGuiBackends) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
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
    // Start the Dear ImGui frame using TinyImGui
    TinyImGui::NewFrame();

    // Draw main menu bar
    DrawMainMenuBar(registry, scripting, playMode);

    // Draw all editor panels in windows
    if (showViewport) DrawViewport(registry, renderer);
    if (showContentBrowser) DrawContentBrowser();
    if (showWorldOutliner) DrawWorldOutliner(registry);
    if (showInspector) DrawInspector(registry, scripting);
    if (showBlueprintGraph) DrawBlueprintGraph(registry, scripting);
    if (showConsole) DrawConsole(registry, scripting);
    if (showMaterialEditor) DrawMaterialEditor();
    if (showRoadmap) DrawRoadmap();

    // Draw toolbar as overlay
    DrawToolbar(playMode);

    // Demo window for development
    if (showDemoWindow) ImGui::ShowDemoWindow(&showDemoWindow);
    if (showMetrics) ImGui::ShowMetricsWindow(&showMetrics);

    // Render using TinyImGui
    ImGui::Render();
    TinyImGui::RenderDrawData(ImGui::GetDrawData());
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
            ImGui::MenuItem("Blueprint Graph", nullptr, &showBlueprintGraph);
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

        // Handle entity selection via mouse picking
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) {
            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
            ImVec2 contentPos = ImVec2(windowPos.x + contentMin.x, windowPos.y + contentMin.y);
            ImVec2 relativePos = ImVec2(mousePos.x - contentPos.x, mousePos.y - contentPos.y);
            HandleEntitySelection(registry, relativePos, viewportSize);
        }

        // Entity selection list for now (fallback)
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

            // Script Component
            if (auto* script = registry.try_get<Script>(selectedEntity)) {
                if (ImGui::CollapsingHeader("Script", ImGuiTreeNodeFlags_DefaultOpen)) {
                    char scriptBuffer[256];
                    strncpy(scriptBuffer, script->filePath.c_str(), sizeof(scriptBuffer));
                    if (ImGui::InputText("Script Path", scriptBuffer, sizeof(scriptBuffer))) {
                        script->filePath = std::string(scriptBuffer);
                    }

                    if (ImGui::Button("Reload Script")) {
                        scripting.loadScript(registry, selectedEntity, script->filePath);
                        AddLog("Reloaded script: " + script->filePath, "Info");
                    }

                    ImGui::Text("Last Update: %.2f", script->lastUpdateTime);
                    ImGui::Checkbox("Needs Update", &script->needsUpdate);
                }
            }

            ImGui::Separator();

            // Add Component section
            if (ImGui::Button("Add Component")) {
                ImGui::OpenPopup("AddComponentPopup");
            }

            if (ImGui::BeginPopup("AddComponentPopup")) {
                if (ImGui::MenuItem("Mesh (Cube)")) {
                    if (!registry.any_of<MeshCube>(selectedEntity)) {
                        registry.emplace<MeshCube>(selectedEntity);
                        AddLog("Added Mesh Component", "Info");
                    }
                }
                if (ImGui::MenuItem("Script")) {
                    if (!registry.any_of<Script>(selectedEntity)) {
                        registry.emplace<Script>(selectedEntity, "assets/scripts/default.lua", 0.0f, false);
                        AddLog("Added Script Component", "Info");
                    }
                }
                if (ImGui::MenuItem("Blueprint")) {
                    if (!registry.any_of<BlueprintComponent>(selectedEntity)) {
                        // create generated folder if needed
                        std::filesystem::create_directories("assets/scripts/generated");
                        std::string out = "assets/scripts/generated/blueprint_" + std::to_string((uint32_t)selectedEntity) + ".lua";
                        // write a small template
                        std::ofstream ofs(out);
                        ofs << "function OnStart(id)\n  -- Blueprint start\nend\n\nfunction OnTick(id, dt)\n  -- Blueprint tick\nend\n";
                        ofs.close();
                        registry.emplace<BlueprintComponent>(selectedEntity, BlueprintComponent{out});
                        currentBlueprintPath = out;
                        // load file into editor buffer
                        currentBlueprintCode.clear();
                        std::ifstream ifs(out);
                        if (ifs) {
                            std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                            currentBlueprintCode = content;
                        }
                        showBlueprintEditor = true;
                        AddLog("Added Blueprint Component and opened editor: " + out, "Info");
                    }
                }
                if (ImGui::MenuItem("Code")) {
                    // Create a raw Lua script and open code editor
                    std::filesystem::create_directories("assets/scripts/generated");
                    std::string out = "assets/scripts/generated/code_" + std::to_string((uint32_t)selectedEntity) + ".lua";
                    std::ofstream ofs(out);
                    ofs << "-- New code\nfunction OnStart(id) end\nfunction OnTick(id, dt) end\n";
                    ofs.close();
                    // attach as Script component if not present
                    if (!registry.any_of<Script>(selectedEntity)) {
                        registry.emplace<Script>(selectedEntity, out, 0.0, false);
                    } else {
                        auto& s = registry.get<Script>(selectedEntity);
                        s.filePath = out;
                    }
                    currentBlueprintPath = out;
                    std::ifstream ifs(out);
                    currentBlueprintCode.clear();
                    if (ifs) {
                        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                        currentBlueprintCode = content;
                    }
                    showBlueprintEditor = true;
                    AddLog("Created code file and opened editor: " + out, "Info");
                }
                if (ImGui::MenuItem("HUD")) {
                    if (!registry.any_of<HUDComponent>(selectedEntity)) {
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

// include VSGraph generator
#include "VSGraph.h"

void UnrealEditor::DrawBlueprintGraph(entt::registry& registry, Scripting& scripting) {
    if (ImGui::Begin("Blueprint Graph")) {
        ImGui::Text("🔧 Blueprint Visual Scripting / Code Editor");
        ImGui::Separator();

#ifdef IMNODES_AVAILABLE
        ImGui::Text("ImNodes visual editor available (full UI in other editor)");
#else
        ImGui::Text("Visual node editor not available. Using code editor as fallback.");
#endif

        ImGui::Spacing();
        if (ImGui::Button("Generate Rotate Premade")) {
            std::string out = VSGraph::Generate("assets", VSGraph::Premade::RotateOnTick);
            currentBlueprintPath = out;
            std::ifstream ifs(out);
            currentBlueprintCode.clear();
            if (ifs) currentBlueprintCode.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            showBlueprintEditor = true;
            AddLog("Generated premade blueprint: " + out, "Info");
        }

        ImGui::SameLine();
        if (ImGui::Button("Generate PrintHello Premade")) {
            std::string out = VSGraph::Generate("assets", VSGraph::Premade::PrintHelloOnStart);
            currentBlueprintPath = out;
            std::ifstream ifs(out);
            currentBlueprintCode.clear();
            if (ifs) currentBlueprintCode.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            showBlueprintEditor = true;
            AddLog("Generated premade blueprint: " + out, "Info");
        }

        ImGui::Spacing();
        if (showBlueprintEditor) {
            ImGui::Separator();
            ImGui::Text("Editing: %s", currentBlueprintPath.c_str());

            // Ensure buffer is large enough for ImGui to edit in-place
                // prepare char buffer
                blueprintEditBuffer.assign(currentBlueprintCode.begin(), currentBlueprintCode.end());
                blueprintEditBuffer.push_back('\0');
                if (ImGui::InputTextMultiline("##blueprintcode", blueprintEditBuffer.data(), blueprintEditBuffer.size(), ImVec2(-1,300))) {
                    currentBlueprintCode = std::string(blueprintEditBuffer.data());
                }

            if (ImGui::Button("Save")) {
                if (!currentBlueprintPath.empty()) {
                    std::ofstream ofs(currentBlueprintPath, std::ios::binary);
                    ofs << currentBlueprintCode;
                    ofs.close();
                    AddLog("Saved blueprint: " + currentBlueprintPath, "Info");
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Apply to Selected")) {
                if (selectedEntity != entt::null && IsEntityValid(registry, selectedEntity)) {
                    // ensure Script component
                    if (!registry.any_of<Script>(selectedEntity)) {
                        registry.emplace<Script>(selectedEntity, currentBlueprintPath, 0.0, false);
                    } else {
                        auto& s = registry.get<Script>(selectedEntity);
                        s.filePath = currentBlueprintPath;
                    }
                    scripting.loadScript(registry, selectedEntity, currentBlueprintPath);
                    AddLog("Applied blueprint/script to entity", "Info");
                } else {
                    AddLog("No selected entity to apply to", "Warning");
                }
            }
        }
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

void UnrealEditor::HandleEntitySelection(entt::registry& registry, ImVec2 mousePos, ImVec2 viewportSize) {
    int width = (int)viewportSize.x;
    int height = (int)viewportSize.y;
    if (width <= 0 || height <= 0) return;

    glm::vec3 camPos = viewportCamera.position;
    glm::mat4 V = glm::lookAt(viewportCamera.position, viewportCamera.target, viewportCamera.up);
    float aspect = width > 0 ? (float)width / (float)height : 16.0f/9.0f;
    glm::mat4 P = glm::perspective(glm::radians(viewportCamera.fov), aspect, viewportCamera.nearPlane, viewportCamera.farPlane);

    float ndcX = (mousePos.x / (float)width) * 2.0f - 1.0f;
    float ndcY = 1.0f - (mousePos.y / (float)height) * 2.0f;
    glm::vec4 clip = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
    glm::mat4 invPV = glm::inverse(P * V);
    glm::vec4 worldNear = invPV * clip; worldNear /= worldNear.w;

    clip.z = 1.0f; clip.w = 1.0f;
    glm::vec4 worldFar = invPV * clip; worldFar /= worldFar.w;

    glm::vec3 rayOrig = glm::vec3(worldNear);
    glm::vec3 rayDir = glm::normalize(glm::vec3(worldFar - worldNear));

    float bestT = FLT_MAX;
    entt::entity best = entt::null;

    auto view = registry.view<Transform, MeshCube>();
    for (auto e : view) {
        auto& t = view.get<Transform>(e);
        glm::vec3 aabbMin = t.position - glm::vec3(0.5f) * t.scale;
        glm::vec3 aabbMax = t.position + glm::vec3(0.5f) * t.scale;

        float tmin = 0.0f, tmax = FLT_MAX;
        for (int i = 0; i < 3; ++i) {
            float invD = 1.0f / rayDir[i];
            float t0 = (aabbMin[i] - rayOrig[i]) * invD;
            float t1 = (aabbMax[i] - rayOrig[i]) * invD;
            if (invD < 0.0f) std::swap(t0, t1);
            tmin = t0 > tmin ? t0 : tmin;
            tmax = t1 < tmax ? t1 : tmax;
            if (tmax <= tmin) break;
        }
        if (tmax > tmin) {
            if (tmin < bestT) {
                bestT = tmin;
                best = e;
            }
        }
    }

    if (best != entt::null) {
        selectedEntity = best;
        AddLog("Selected entity via viewport click: " + GetEntityName(registry, best), "Info");
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
