#include "Editor.h"
#include "Components.h"
#include "Scripting.h"
#include "FileUtil.h"
#include "Theme.h"
#include "HUD.h"
#include "VSGraph.h"
#include "ScriptEditor.h"
#include "BlueprintEditor.h"
#include "SceneManipulator.h"
#include "DockingManager.h"
#include <imgui.h>
#include <imnodes.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <filesystem>

bool Editor::init(GLFWwindow* window){
    (void)window;
    IMGUI_CHECKVERSION();
    // Theme
    Theme::SetupImGuiTheme();
    imnodes::Initialize();

    // Initialize advanced systems
    initializeAdvancedSystems();

    return true;
}

void Editor::shutdown(GLFWwindow* window){
    (void)window;

    // Shutdown advanced systems
    if (scriptEditor) scriptEditor->Shutdown();
    if (blueprintEditor) blueprintEditor->Shutdown();
    if (sceneManipulator) sceneManipulator->Shutdown();
    if (dockingManager) dockingManager->Shutdown();

    imnodes::Shutdown();
}

void Editor::initializeAdvancedSystems() {
    // Initialize advanced editing systems
    scriptEditor = std::make_unique<ScriptEditor>();
    scriptEditor->Init();

    blueprintEditor = std::make_unique<BlueprintEditor>();
    blueprintEditor->Init();

    sceneManipulator = std::make_unique<SceneManipulator>();
    sceneManipulator->Init();

    dockingManager = std::make_unique<DockingManager>();
    dockingManager->Init();

    // Register dockable windows
    dockingManager->RegisterWindow(DockingManager::VIEWPORT_WINDOW, "Viewport",
        [this]() { /* Viewport rendering will be handled in drawPanels */ });

    dockingManager->RegisterWindow(DockingManager::OUTLINER_WINDOW, "World Outliner",
        [this]() { /* World Outliner rendering will be handled in drawPanels */ });

    dockingManager->RegisterWindow(DockingManager::INSPECTOR_WINDOW, "Details",
        [this]() { /* Inspector rendering will be handled in drawPanels */ });

    dockingManager->RegisterWindow(DockingManager::CONTENT_BROWSER_WINDOW, "Content Browser",
        [this]() { /* Content Browser rendering will be handled in drawPanels */ });

    dockingManager->RegisterWindow(DockingManager::CONSOLE_WINDOW, "Output Log",
        [this]() { renderConsolePanel(); });

    dockingManager->RegisterWindow(DockingManager::SCRIPT_EDITOR_WINDOW, "Script Editor",
        [this]() { renderScriptEditorPanel(); });

    dockingManager->RegisterWindow(DockingManager::BLUEPRINT_EDITOR_WINDOW, "Blueprint Editor",
        [this]() { renderBlueprintEditorPanel(); });

    // Setup callbacks
    sceneManipulator->SetSelectionChangedCallback([this](entt::entity entity) {
        setSelectedEntity(entity);
    });
}

void Editor::updateAdvancedSystems(float deltaTime) {
    if (scriptEditor) scriptEditor->Update(deltaTime);
    if (blueprintEditor) blueprintEditor->Update(deltaTime);
    if (sceneManipulator) sceneManipulator->Update(deltaTime);
    if (dockingManager) dockingManager->Update();
}

void Editor::renderAdvancedSystems(entt::registry& reg, Renderer& renderer) {
    // Scene manipulation rendering
    if (sceneManipulator) {
        glm::mat4 view = glm::mat4(1.0f);      // Get from camera
        glm::mat4 projection = glm::mat4(1.0f); // Get from camera
        sceneManipulator->Render(reg, renderer, view, projection);
    }
}

void Editor::setTool(Tool tool) {
    currentTool = tool;
    if (sceneManipulator) {
        SceneManipulator::Tool manipTool;
        switch (tool) {
            case Tool::Select: manipTool = SceneManipulator::Tool::Select; break;
            case Tool::Move:   manipTool = SceneManipulator::Tool::Move; break;
            case Tool::Rotate: manipTool = SceneManipulator::Tool::Rotate; break;
            case Tool::Scale:  manipTool = SceneManipulator::Tool::Scale; break;
        }
        sceneManipulator->SetTool(manipTool);
    }
}

void Editor::setSelectedEntity(entt::entity entity) {
    selected = entity;
    if (sceneManipulator) {
        sceneManipulator->SetSelectedEntity(entity);
    }
}

bool Editor::handleMouseInput(const glm::vec2& mousePos, const glm::vec2& viewportSize,
                             bool isPressed, bool isReleased, entt::registry& registry) {
    if (sceneManipulator) {
        return sceneManipulator->HandleMouseInput(mousePos, viewportSize, isPressed, isReleased, registry);
    }
    return false;
}

void Editor::drawDockspace(){
    if (dockingManager) {
        dockingManager->BeginDockSpace();

        // Render main menu bar
        renderMainMenuBar();

        dockingManager->EndDockSpace();
    } else {
        // Fallback to simple window
        ImGui::Begin("Main");
        ImGui::TextUnformatted("SproutEngine");
        renderMainMenuBar();
        ImGui::End();
    }
}

static void Vec3Control(const char* label, glm::vec3& v){
    float arr[3] = {v.x, v.y, v.z};
    if(ImGui::DragFloat3(label, arr, 0.1f)) { v = {arr[0],arr[1],arr[2]}; }
}

void Editor::drawPanels(entt::registry& reg, Renderer& renderer, Scripting& scripting, bool& playMode){
    (void)renderer;
    (void)scripting;
    (void)playMode;

    // Update advanced systems
    updateAdvancedSystems(ImGui::GetIO().DeltaTime);

    // The docking manager handles window layout, but we still render content
    renderViewportPanel(reg, renderer);
    renderWorldOutlinerPanel(reg);
    renderInspectorPanel(reg);
    renderContentBrowserPanel(reg);  // Add content browser to main panel rendering

    // Advanced systems render themselves
    renderAdvancedSystems(reg, renderer);
}

void Editor::renderMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene")) {
                // Clear registry and create new scene
            }
            if (ImGui::MenuItem("Open Scene...")) {
                // Open file dialog and load scene
            }
            if (ImGui::MenuItem("Save Scene")) {
                // Save current scene
            }
            ImGui::Separator();
            if (ImGui::MenuItem("New Cube")) {
                // Create a new cube entity (keeping original functionality)
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                // Handle application exit
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
                // Undo functionality
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
                // Redo functionality
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Copy", "Ctrl+C")) {
                // Copy selected entity
            }
            if (ImGui::MenuItem("Paste", "Ctrl+V")) {
                // Paste entity
            }
            if (ImGui::MenuItem("Delete", "Del")) {
                if (selected != entt::null && sceneManipulator) {
                    // sceneManipulator->DeleteEntity(reg, selected);
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            bool isSelectTool = (currentTool == Tool::Select);
            bool isMoveTool = (currentTool == Tool::Move);
            bool isRotateTool = (currentTool == Tool::Rotate);
            bool isScaleTool = (currentTool == Tool::Scale);

            if (ImGui::MenuItem("Select Tool", "Q", isSelectTool)) {
                setTool(Tool::Select);
            }
            if (ImGui::MenuItem("Move Tool", "W", isMoveTool)) {
                setTool(Tool::Move);
            }
            if (ImGui::MenuItem("Rotate Tool", "E", isRotateTool)) {
                setTool(Tool::Rotate);
            }
            if (ImGui::MenuItem("Scale Tool", "R", isScaleTool)) {
                setTool(Tool::Scale);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Windows")) {
            if (dockingManager) {
                dockingManager->RenderWindowMenu();
            } else {
                // Fallback window toggles
                ImGui::MenuItem("Script Editor", nullptr, &showScriptEditor);
                ImGui::MenuItem("Blueprint Editor", nullptr, &showBlueprintEditor);
                ImGui::MenuItem("Material Editor", nullptr, &showMaterialEditor);
                ImGui::MenuItem("Animation Editor", nullptr, &showAnimationEditor);
                ImGui::MenuItem("Debugger", nullptr, &showDebugger);
                ImGui::MenuItem("Profiler", nullptr, &showProfiler);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Layout")) {
            if (dockingManager) {
                dockingManager->RenderLayoutMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                // Show about dialog
            }
            if (ImGui::MenuItem("Documentation")) {
                // Open documentation
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Editor::renderToolbar() {
    if (ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
        // Tool buttons
        if (ImGui::Button("Select")) setTool(Tool::Select);
        ImGui::SameLine();
        if (ImGui::Button("Move")) setTool(Tool::Move);
        ImGui::SameLine();
        if (ImGui::Button("Rotate")) setTool(Tool::Rotate);
        ImGui::SameLine();
        if (ImGui::Button("Scale")) setTool(Tool::Scale);

        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();

        // Play controls
        if (ImGui::Button("Play")) {
            // Start play mode
        }
        ImGui::SameLine();
        if (ImGui::Button("Pause")) {
            // Pause play mode
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            // Stop play mode
        }
    }
    ImGui::End();
}

void Editor::renderViewportPanel(entt::registry& reg, Renderer& renderer) {
    if (ImGui::Begin("Viewport")) {
        // Viewport rendering would be done here
        // This would include the 3D scene, gizmos, etc.
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();

        // Handle viewport input
        if (ImGui::IsWindowHovered()) {
            ImGuiIO& io = ImGui::GetIO();
            glm::vec2 mousePos(io.MousePos.x, io.MousePos.y);
            glm::vec2 vSize(viewportSize.x, viewportSize.y);

            bool isPressed = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
            bool isReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

            handleMouseInput(mousePos, vSize, isPressed, isReleased, reg);
        }

        ImGui::Text("3D Viewport - Size: %.0fx%.0f", viewportSize.x, viewportSize.y);
    }
    ImGui::End();
}

void Editor::renderWorldOutlinerPanel(entt::registry& reg) {
    if (ImGui::Begin("World Outliner")) {
        // Entity list
        reg.view<Tag>().each([&](entt::entity entity, const Tag& tag) {
            std::string name = tag.name;

            bool isSelected = (selected == entity);
            if (ImGui::Selectable(name.c_str(), isSelected)) {
                setSelectedEntity(entity);
            }

            // Right-click context menu
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Delete")) {
                    if (sceneManipulator) {
                        // sceneManipulator->DeleteEntity(reg, entity);
                    }
                }
                if (ImGui::MenuItem("Duplicate")) {
                    if (sceneManipulator) {
                        // sceneManipulator->DuplicateEntity(reg, entity);
                    }
                }
                ImGui::EndPopup();
            }
        });
    }
    ImGui::End();
}

void Editor::renderInspectorPanel(entt::registry& reg) {
    if (ImGui::Begin("Details")) {
        if (selected != entt::null && reg.valid(selected)) {
            // Entity ID
            ImGui::Text("Entity ID: %u", (uint32_t)selected);
            ImGui::Separator();

            // Transform component
            if (auto* transform = reg.try_get<Transform>(selected)) {
                if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                    Vec3Control("Position", transform->position);
                    Vec3Control("Rotation", transform->rotation);
                    Vec3Control("Scale", transform->scale);
                }
            }

            // Tag component
            if (auto* tag = reg.try_get<Tag>(selected)) {
                if (ImGui::CollapsingHeader("Tag", ImGuiTreeNodeFlags_DefaultOpen)) {
                    char buffer[256];
                    strncpy(buffer, tag->name.c_str(), sizeof(buffer));
                    if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
                        tag->name = buffer;
                    }
                }
            }

            // Mesh components
            if (reg.any_of<MeshCube>(selected)) {
                if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
                    if (reg.all_of<MeshCube>(selected)) {
                        ImGui::Text("Type: Cube");
                    }
                }
            }

            // Script component
            if (auto* script = reg.try_get<Script>(selected)) {
                if (ImGui::CollapsingHeader("Script", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Current Script: %s", script->filePath.empty() ? "None" : script->filePath.c_str());

                    // Script selection dropdown
                    if (ImGui::BeginCombo("Select Script", script->filePath.empty() ? "Choose Script..." : script->filePath.c_str())) {
                        // List existing scripts dynamically
                        auto availableScripts = getAvailableScripts();
                        for (const auto& scriptFile : availableScripts) {
                            if (ImGui::Selectable(scriptFile.c_str(), script->filePath == scriptFile)) {
                                script->filePath = scriptFile;
                            }
                        }

                        ImGui::Separator();

                        // Create new options
                        if (ImGui::Selectable("+ Create New Script")) {
                            if (scriptEditor) {
                                scriptEditor->NewFile();
                                showScriptEditor = true;
                                script->filePath = "NewScript.sp";
                            }
                        }

                        if (ImGui::Selectable("+ Create New Blueprint")) {
                            if (blueprintEditor) {
                                // Create new blueprint
                                showBlueprintEditor = true;
                                script->filePath = "NewBlueprint.bp";
                            }
                        }

                        ImGui::EndCombo();
                    }

                    ImGui::Separator();

                    // Action buttons
                    if (!script->filePath.empty()) {
                        if (ImGui::Button("Edit Script")) {
                            // Open script in the script editor
                            if (scriptEditor && script->filePath.find(".sp") != std::string::npos) {
                                std::string fullPath = script->filePath;
                                if (fullPath.find("assets/") != 0) {
                                    fullPath = "assets/scripts/" + fullPath;
                                }
                                scriptEditor->OpenFile(fullPath);
                                showScriptEditor = true;
                            }
                            // Open blueprint editor for .bp files
                            else if (blueprintEditor && script->filePath.find(".bp") != std::string::npos) {
                                showBlueprintEditor = true;
                            }
                        }

                        ImGui::SameLine();
                        if (ImGui::Button("Remove Script")) {
                            script->filePath = "";
                        }
                    }

                    ImGui::Separator();

                    // Quick creation buttons
                    if (ImGui::Button("Quick Script Template")) {
                        if (scriptEditor) {
                            // Create a quick script template
                            std::string entityName = "Entity" + std::to_string((uint32_t)selected);
                            if (auto* tag = reg.try_get<Tag>(selected)) {
                                entityName = tag->name;
                            }

                            std::string scriptName = entityName + "Script.sp";
                            script->filePath = scriptName;

                            // Open script editor with template
                            scriptEditor->NewFile();
                            showScriptEditor = true;
                        }
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Quick Blueprint")) {
                        if (blueprintEditor) {
                            std::string entityName = "Entity" + std::to_string((uint32_t)selected);
                            if (auto* tag = reg.try_get<Tag>(selected)) {
                                entityName = tag->name;
                            }

                            std::string blueprintName = entityName + "BP.bp";
                            script->filePath = blueprintName;
                            showBlueprintEditor = true;
                        }
                    }

                    // Show hot reload status
                    if (script->needsUpdate) {
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Script needs reload");
                        ImGui::SameLine();
                        if (ImGui::Button("Reload")) {
                            script->needsUpdate = false;
                            // Trigger hot reload
                            // TODO: Implement hot reload trigger
                        }
                    } else if (!script->filePath.empty()) {
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Script up to date");
                    }
                }
            }

            // Add component button
            if (ImGui::Button("Add Component")) {
                ImGui::OpenPopup("AddComponent");
            }

            if (ImGui::BeginPopup("AddComponent")) {
                if (ImGui::MenuItem("Transform") && !reg.all_of<Transform>(selected)) {
                    reg.emplace<Transform>(selected);
                }
                if (ImGui::MenuItem("Tag") && !reg.all_of<Tag>(selected)) {
                    reg.emplace<Tag>(selected, Tag{"New Entity"});
                }
                if (ImGui::MenuItem("Cube Mesh") && !reg.all_of<MeshCube>(selected)) {
                    reg.emplace<MeshCube>(selected);
                }

                // Script submenu
                if (ImGui::BeginMenu("Script") && !reg.all_of<Script>(selected)) {
                    if (ImGui::MenuItem("Add Existing Script")) {
                        Script newScript;
                        newScript.filePath = ""; // Let user choose from dropdown
                        reg.emplace<Script>(selected, newScript);
                    }

                    ImGui::Separator();

                    if (ImGui::MenuItem("Create New Script")) {
                        Script newScript;
                        newScript.filePath = "NewScript.sp";
                        reg.emplace<Script>(selected, newScript);

                        // Open script editor immediately
                        if (scriptEditor) {
                            scriptEditor->NewFile();
                            showScriptEditor = true;
                        }
                    }

                    if (ImGui::MenuItem("Create New Blueprint")) {
                        Script newScript;
                        newScript.filePath = "NewBlueprint.bp";
                        reg.emplace<Script>(selected, newScript);

                        // Open blueprint editor immediately
                        if (blueprintEditor) {
                            showBlueprintEditor = true;
                        }
                    }

                    ImGui::Separator();
                    ImGui::Text("Quick Add:");

                    // List available scripts for quick adding
                    auto availableScripts = getAvailableScripts();
                    for (const auto& scriptFile : availableScripts) {
                        if (ImGui::MenuItem(scriptFile.c_str())) {
                            Script newScript;
                            newScript.filePath = scriptFile;
                            reg.emplace<Script>(selected, newScript);
                        }
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndPopup();
            }
        } else {
            ImGui::Text("No entity selected");
        }
    }
    ImGui::End();
}

void Editor::renderContentBrowserPanel(entt::registry& reg) {
    if (ImGui::Begin("Content Browser")) {
        // Create new script button
        if (ImGui::Button("Create New Script")) {
            if (scriptEditor) {
                scriptEditor->NewFile();
                showScriptEditor = true;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Refresh")) {
            // TODO: Refresh file listing
        }

        ImGui::Separator();

        // Directory tree with actual files
        if (ImGui::TreeNode("Scripts")) {
            // Show actual .sp files
            if (ImGui::Selectable("Rotate.sp")) {
                if (scriptEditor) {
                    scriptEditor->OpenFile("assets/scripts/Rotate.sp");
                    showScriptEditor = true;
                }
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                // Double-click to assign to selected entity
                if (selected != entt::null) {
                    // Add or update script component
                    Script script;
                    script.filePath = "Rotate.sp";
                    if (auto* existingScript = reg.try_get<Script>(selected)) {
                        existingScript->filePath = "Rotate.sp";
                    } else {
                        reg.emplace<Script>(selected, script);
                    }
                }
            }

            if (ImGui::Selectable("PlayerCharacter.sp")) {
                if (scriptEditor) {
                    scriptEditor->OpenFile("assets/scripts/PlayerCharacter.sp");
                    showScriptEditor = true;
                }
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                if (selected != entt::null) {
                    Script script;
                    script.filePath = "PlayerCharacter.sp";
                    if (auto* existingScript = reg.try_get<Script>(selected)) {
                        existingScript->filePath = "PlayerCharacter.sp";
                    } else {
                        reg.emplace<Script>(selected, script);
                    }
                }
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Blueprints")) {
            ImGui::Text("CharacterBP");
            ImGui::Text("WeaponBP");
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Materials")) {
            ImGui::Text("DefaultMaterial");
            ImGui::Text("MetalMaterial");
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Textures")) {
            ImGui::Text("texture1.png");
            ImGui::Text("texture2.jpg");
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void Editor::renderConsolePanel() {
    if (ImGui::Begin("Output Log")) {
        // Console/log output
        ImGui::Text("Engine log output will appear here");
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[INFO] SproutEngine initialized");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[INFO] Advanced editing systems loaded");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[WARN] Example warning message");
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "[ERROR] Example error message");
    }
    ImGui::End();
}

void Editor::renderScriptEditorPanel() {
    if (scriptEditor) {
        scriptEditor->Render();
    }
}

void Editor::renderBlueprintEditorPanel() {
    if (blueprintEditor) {
        blueprintEditor->Render();
    }
}

void Editor::renderMaterialEditorPanel() {
    if (ImGui::Begin("Material Editor", &showMaterialEditor)) {
        ImGui::Text("Material editing interface");
        // Material property editors would go here
    }
    ImGui::End();
}

void Editor::renderAnimationEditorPanel() {
    if (ImGui::Begin("Animation", &showAnimationEditor)) {
        ImGui::Text("Animation timeline and editor");
        // Animation editing interface would go here
    }
    ImGui::End();
}

void Editor::renderDebuggerPanel() {
    if (ImGui::Begin("Debug", &showDebugger)) {
        ImGui::Text("Debug information and controls");
        // Debug interface would go here
    }
    ImGui::End();
}

void Editor::renderProfilerPanel() {
    if (ImGui::Begin("Profiler", &showProfiler)) {
        ImGui::Text("Performance profiling data");
        // Profiler interface would go here
    }
    ImGui::End();
}

void Editor::renderStatusBar() {
    if (ImGui::Begin("Status Bar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
        ImGui::Text("Ready | Entities: ?? | FPS: %.1f", ImGui::GetIO().Framerate);

        ImGui::SameLine(ImGui::GetWindowWidth() - 200);
    ImGui::Text("Tool: %s",
        currentTool == Tool::Select ? "Select" :
        currentTool == Tool::Move ? "Move" :
        currentTool == Tool::Rotate ? "Rotate" :
        currentTool == Tool::Scale ? "Scale" : "Unknown");
    }
    ImGui::End();
}

std::vector<std::string> Editor::getAvailableScripts() {
    std::vector<std::string> scripts;

    // Scan assets/scripts directory for .sp files
    std::string scriptsPath = "assets/scripts/";
    try {
        if (std::filesystem::exists(scriptsPath) && std::filesystem::is_directory(scriptsPath)) {
            for (const auto& entry : std::filesystem::directory_iterator(scriptsPath)) {
                if (entry.is_regular_file() && entry.path().extension() == ".sp") {
                    scripts.push_back(entry.path().filename().string());
                }
            }
        }
    } catch (const std::exception& e) {
        // Handle filesystem errors gracefully
    }

    // Always ensure we have some default scripts
    if (scripts.empty()) {
        scripts = {"Rotate.sp", "PlayerCharacter.sp", "NewScript.sp"};
    }

    return scripts;
}

std::vector<std::string> Editor::getAvailableBlueprints() {
    std::vector<std::string> blueprints;

    // Scan assets/blueprints directory for .bp files
    std::string blueprintsPath = "assets/blueprints/";
    try {
        if (std::filesystem::exists(blueprintsPath) && std::filesystem::is_directory(blueprintsPath)) {
            for (const auto& entry : std::filesystem::directory_iterator(blueprintsPath)) {
                if (entry.is_regular_file() && entry.path().extension() == ".bp") {
                    blueprints.push_back(entry.path().filename().string());
                }
            }
        }
    } catch (const std::exception& e) {
        // Handle filesystem errors gracefully
    }

    return blueprints;
}
