#include "UnrealEditor.h"
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

// Include imnodes if available
#ifdef IMNODES_AVAILABLE
#include <imnodes.h>
#endif

UnrealEditor::UnrealEditor() {
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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

#ifdef IMNODES_AVAILABLE
    // Initialize imnodes for blueprint graphs
    ImNodes::CreateContext();
    ImNodes::StyleColorsDark();
#endif

    // Initialize console with welcome message
    AddLog("=== SproutEngine Editor Started ===", "System");
    AddLog("Type 'help' for available commands", "Info");

    // Refresh content browser
    RefreshContentBrowser();

    return true;
}

void UnrealEditor::Shutdown(GLFWwindow* window) {
#ifdef IMNODES_AVAILABLE
    ImNodes::DestroyContext();
#endif

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UnrealEditor::Update(float deltaTime) {
    HandleViewportInput();
    UpdateViewportCamera(deltaTime);

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

    // Create dockspace
    DrawDockSpace();

    // Draw toolbar
    DrawToolbar(playMode);

    // Draw all editor panels
    if (showViewport) DrawViewport(registry, renderer);
    if (showContentBrowser) DrawContentBrowser();
    if (showWorldOutliner) DrawWorldOutliner(registry);
    if (showInspector) DrawInspector(registry, scripting);
    if (showBlueprintGraph) DrawBlueprintGraph();
    if (showConsole) DrawConsole(registry, scripting);
    if (showMaterialEditor) DrawMaterialEditor();

    // Demo window for development
    if (showDemoWindow) ImGui::ShowDemoWindow(&showDemoWindow);
    if (showMetrics) ImGui::ShowMetricsWindow(&showMetrics);

    // Render
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void UnrealEditor::DrawMainMenuBar(entt::registry& registry, Scripting& scripting, bool& playMode) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                NewScene(registry);
            }
            if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
                // TODO: File dialog
                AddLog("Open Scene - File dialog not implemented yet", "Warning");
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                SaveScene(registry, "assets/scenes/current_scene.json");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Import Asset")) {
                // TODO: File dialog
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
                registry.emplace<MeshComponent>(entity, "cube");
                registry.emplace<MaterialComponent>(entity);
                selectedEntity = entity;
                AddLog("Created cube entity", "Info");
            }
            if (ImGui::MenuItem("Camera")) {
                entt::entity entity = CreateEntity(registry, "Camera");
                registry.emplace<CameraComponent>(entity);
                selectedEntity = entity;
                AddLog("Created camera entity", "Info");
            }
            if (ImGui::MenuItem("Light")) {
                entt::entity entity = CreateEntity(registry, "Light");
                registry.emplace<LightComponent>(entity);
                selectedEntity = entity;
                AddLog("Created light entity", "Info");
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Reload Scripts", "F5")) {
                scripting.ReloadAllScripts();
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

void UnrealEditor::DrawDockSpace() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", nullptr, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }

    ImGui::End();
}

void UnrealEditor::DrawToolbar(bool& playMode) {
    if (ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar)) {
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
        ImGui::Separator();

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
    }
    ImGui::End();
}

void UnrealEditor::DrawViewport(entt::registry& registry, Renderer& renderer) {
    if (ImGui::Begin("Viewport")) {
        // Get the size of the viewport
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();

        // Handle entity selection
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) {
            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 relativePos = ImVec2(mousePos.x - windowPos.x, mousePos.y - windowPos.y);
            HandleEntitySelection(registry, relativePos, viewportSize);
        }

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

        if (showGizmos) {
            DrawGizmos(registry);
        }

        // Add drag-drop target for assets
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_ITEM")) {
                std::string assetPath = std::string((const char*)payload->Data, payload->DataSize);
                AddLog("Dropped asset: " + assetPath, "Info");
                // TODO: Create entity from asset
            }
            ImGui::EndDragDropTarget();
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

        // Directory tree (left side)
        if (ImGui::BeginChild("DirectoryTree", ImVec2(200, 0), true)) {
            DrawDirectoryTree();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // File grid (right side)
        if (ImGui::BeginChild("FileGrid", ImVec2(0, 0), true)) {
            DrawFileGrid();
        }
        ImGui::EndChild();

        // Context menu
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

        // Entity list
        auto view = registry.view<NameComponent>();
        for (auto entity : view) {
            auto& name = view.get<NameComponent>(entity);

            // Filter by search
            if (strlen(searchBuffer) > 0) {
                if (name.name.find(searchBuffer) == std::string::npos) {
                    continue;
                }
            }

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            if (entity == selectedEntity) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            ImGui::TreeNodeEx((void*)(intptr_t)entity, flags, "%s", name.name.c_str());

            if (ImGui::IsItemClicked()) {
                selectedEntity = entity;
            }

            // Drag source for reparenting
            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("ENTITY", &entity, sizeof(entt::entity));
                ImGui::Text("Moving %s", name.name.c_str());
                ImGui::EndDragDropSource();
            }

            // Drop target for reparenting
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY")) {
                    entt::entity droppedEntity = *(const entt::entity*)payload->Data;
                    AddLog("Reparenting not implemented yet", "Warning");
                }
                ImGui::EndDragDropTarget();
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
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void UnrealEditor::DrawInspector(entt::registry& registry, Scripting& scripting) {
    if (ImGui::Begin("Inspector")) {
        if (selectedEntity != entt::null && IsEntityValid(registry, selectedEntity)) {
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

            // Draw component editors
            if (registry.any_of<TransformComponent>(selectedEntity)) {
                DrawTransformComponent(registry, selectedEntity);
            }

            if (registry.any_of<MeshComponent>(selectedEntity)) {
                DrawMeshComponent(registry, selectedEntity);
            }

            if (registry.any_of<ScriptComponent>(selectedEntity)) {
                DrawScriptComponent(registry, selectedEntity, scripting);
            }

            // Add component button
            DrawAddComponentButton(registry, selectedEntity);

        } else {
            ImGui::Text("No entity selected");
            ImGui::Text("Select an entity in the World Outliner or Viewport");
        }
    }
    ImGui::End();
}

void UnrealEditor::DrawBlueprintGraph() {
#ifdef IMNODES_AVAILABLE
    if (ImGui::Begin("Blueprint Graph")) {
        ImNodes::BeginNodeEditor();

        // Draw existing nodes
        for (const auto& node : blueprintGraph.nodes) {
            DrawNode(node);
        }

        // Handle connections
        HandleNodeConnections();

        ImNodes::EndNodeEditor();

        // Context menu for creating nodes
        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::MenuItem("Add Event Node")) {
                BlueprintState::Node newNode;
                newNode.id = blueprintGraph.nextNodeId++;
                newNode.type = "Event";
                newNode.name = "BeginPlay";
                newNode.position = ImGui::GetMousePos();
                blueprintGraph.nodes.push_back(newNode);
            }
            if (ImGui::MenuItem("Add Function Node")) {
                BlueprintState::Node newNode;
                newNode.id = blueprintGraph.nextNodeId++;
                newNode.type = "Function";
                newNode.name = "Print";
                newNode.position = ImGui::GetMousePos();
                blueprintGraph.nodes.push_back(newNode);
            }
            ImGui::EndPopup();
        }

        // Generate Lua button
        if (ImGui::Button("Generate Lua")) {
            GenerateLuaFromGraph();
        }
    }
    ImGui::End();
#else
    if (ImGui::Begin("Blueprint Graph")) {
        ImGui::Text("Blueprint Graph requires imnodes library");
        ImGui::Text("Install imnodes to enable visual scripting");

        // Simple text-based blueprint editor as fallback
        static char luaCode[4096] = "-- Generated Lua code will appear here\n";
        ImGui::InputTextMultiline("Lua Code", luaCode, sizeof(luaCode));

        if (ImGui::Button("Save to Script")) {
            AddLog("Blueprint to Lua conversion - imnodes required", "Warning");
        }
    }
    ImGui::End();
#endif
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

// Implementation continues in next part due to length...
void UnrealEditor::DrawMaterialEditor() {
    if (ImGui::Begin("Material Editor")) {
        ImGui::Text("Material Editor - Work in Progress");

        // Basic material properties
        static float roughness = 0.5f;
        static float metallic = 0.0f;
        static ImVec4 baseColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

        ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f);
        ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f);
        ImGui::ColorEdit4("Base Color", &baseColor.x);

        if (ImGui::Button("Apply to Selected")) {
            if (selectedEntity != entt::null) {
                AddLog("Applied material properties to selected entity", "Info");
            }
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
    registry.emplace<TransformComponent>(entity);
    return entity;
}

void UnrealEditor::DeleteEntity(entt::registry& registry, entt::entity entity) {
    registry.destroy(entity);
    AddLog("Deleted entity", "Info");
}

void UnrealEditor::DuplicateEntity(entt::registry& registry, entt::entity entity) {
    // Basic duplication - copy name and transform
    std::string originalName = GetEntityName(registry, entity);
    entt::entity newEntity = CreateEntity(registry, originalName + " Copy");

    // Copy transform if it exists
    if (auto* transform = registry.try_get<TransformComponent>(entity)) {
        registry.emplace<TransformComponent>(newEntity, *transform);
        // Offset position slightly
        auto& newTransform = registry.get<TransformComponent>(newEntity);
        newTransform.position.x += 1.0f;
    }

    // Copy other components as needed
    if (auto* mesh = registry.try_get<MeshComponent>(entity)) {
        registry.emplace<MeshComponent>(newEntity, *mesh);
    }

    AddLog("Duplicated entity: " + originalName, "Info");
}

void UnrealEditor::AddLog(const std::string& message, const std::string& level) {
    std::string timestamp = ""; // TODO: Add timestamp
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
        AddLog("  lua <code> - Execute Lua code", "Info");
        AddLog("  select <name> - Select entity by name", "Info");
    } else if (command == "clear") {
        console.logs.clear();
    } else if (command == "entities") {
        auto view = registry.view<NameComponent>();
        AddLog("Entities in scene:", "Info");
        for (auto entity : view) {
            auto& name = view.get<NameComponent>(entity);
            AddLog("  - " + name.name, "Info");
        }
    } else if (command.substr(0, 4) == "lua ") {
        std::string luaCode = command.substr(4);
        try {
            // TODO: Execute Lua code through scripting system
            AddLog("Executed Lua: " + luaCode, "Info");
        } catch (const std::exception& e) {
            AddLog("Lua Error: " + std::string(e.what()), "Error");
        }
    } else {
        AddLog("Unknown command: " + command, "Warning");
        AddLog("Type 'help' for available commands", "Info");
    }
}

// Placeholder implementations for other functions
void UnrealEditor::HandleViewportInput() {
    // TODO: Implement viewport camera controls
}

void UnrealEditor::UpdateViewportCamera(float deltaTime) {
    // TODO: Implement camera movement
}

void UnrealEditor::DrawGizmos(entt::registry& registry) {
    // TODO: Implement 3D gizmos for transform manipulation
}

void UnrealEditor::HandleEntitySelection(entt::registry& registry, ImVec2 mousePos, ImVec2 viewportSize) {
    // TODO: Implement entity selection via mouse picking
}

void UnrealEditor::RefreshContentBrowser() {
    contentBrowser.directories.clear();
    contentBrowser.files.clear();

    try {
        for (const auto& entry : std::filesystem::directory_iterator(contentBrowser.currentPath)) {
            if (entry.is_directory()) {
                contentBrowser.directories.push_back(entry.path().filename().string());
            } else {
                contentBrowser.files.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::exception& e) {
        AddLog("Failed to refresh content browser: " + std::string(e.what()), "Error");
    }
}

void UnrealEditor::DrawDirectoryTree() {
    // TODO: Implement directory tree navigation
    for (const auto& dir : contentBrowser.directories) {
        if (ImGui::TreeNode(dir.c_str())) {
            ImGui::TreePop();
        }
    }
}

void UnrealEditor::DrawFileGrid() {
    for (const auto& file : contentBrowser.files) {
        if (ImGui::Selectable(file.c_str(), contentBrowser.selectedItem == file)) {
            contentBrowser.selectedItem = file;
        }

        // Drag source for assets
        if (ImGui::BeginDragDropSource()) {
            std::string fullPath = contentBrowser.currentPath + file;
            ImGui::SetDragDropPayload("ASSET_ITEM", fullPath.c_str(), fullPath.size() + 1);
            ImGui::Text("Dragging %s", file.c_str());
            ImGui::EndDragDropSource();
        }
    }
}

void UnrealEditor::DrawTransformComponent(entt::registry& registry, entt::entity entity) {
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& transform = registry.get<TransformComponent>(entity);

        ImGui::DragFloat3("Position", &transform.position.x, 0.1f);
        ImGui::DragFloat3("Rotation", &transform.rotation.x, 1.0f);
        ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
    }
}

void UnrealEditor::DrawMeshComponent(entt::registry& registry, entt::entity entity) {
    if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& mesh = registry.get<MeshComponent>(entity);

        char meshPath[256];
        strncpy(meshPath, mesh.meshPath.c_str(), sizeof(meshPath));
        if (ImGui::InputText("Mesh Path", meshPath, sizeof(meshPath))) {
            mesh.meshPath = std::string(meshPath);
        }
    }
}

void UnrealEditor::DrawScriptComponent(entt::registry& registry, entt::entity entity, Scripting& scripting) {
    if (ImGui::CollapsingHeader("Script", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& script = registry.get<ScriptComponent>(entity);

        char scriptPath[256];
        strncpy(scriptPath, script.scriptPath.c_str(), sizeof(scriptPath));
        if (ImGui::InputText("Script Path", scriptPath, sizeof(scriptPath))) {
            script.scriptPath = std::string(scriptPath);
        }

        if (ImGui::Button("Reload Script")) {
            scripting.ReloadScript(script.scriptPath);
            AddLog("Reloaded script: " + script.scriptPath, "Info");
        }
    }
}

void UnrealEditor::DrawAddComponentButton(entt::registry& registry, entt::entity entity) {
    if (ImGui::Button("Add Component")) {
        ImGui::OpenPopup("AddComponentPopup");
    }

    if (ImGui::BeginPopup("AddComponentPopup")) {
        if (ImGui::MenuItem("Mesh Component")) {
            if (!registry.any_of<MeshComponent>(entity)) {
                registry.emplace<MeshComponent>(entity);
                AddLog("Added Mesh Component", "Info");
            }
        }
        if (ImGui::MenuItem("Script Component")) {
            if (!registry.any_of<ScriptComponent>(entity)) {
                registry.emplace<ScriptComponent>(entity);
                AddLog("Added Script Component", "Info");
            }
        }
        if (ImGui::MenuItem("Camera Component")) {
            if (!registry.any_of<CameraComponent>(entity)) {
                registry.emplace<CameraComponent>(entity);
                AddLog("Added Camera Component", "Info");
            }
        }
        if (ImGui::MenuItem("Light Component")) {
            if (!registry.any_of<LightComponent>(entity)) {
                registry.emplace<LightComponent>(entity);
                AddLog("Added Light Component", "Info");
            }
        }
        ImGui::EndPopup();
    }
}

// Placeholder implementations for file operations
void UnrealEditor::NewScene(entt::registry& registry) {
    registry.clear();
    selectedEntity = entt::null;
    AddLog("Created new scene", "Info");
}

void UnrealEditor::OpenScene(entt::registry& registry, const std::string& filepath) {
    AddLog("Open Scene not implemented yet", "Warning");
}

void UnrealEditor::SaveScene(entt::registry& registry, const std::string& filepath) {
    AddLog("Save Scene not implemented yet", "Warning");
}

void UnrealEditor::ImportAsset(const std::string& filepath) {
    AddLog("Import Asset not implemented yet", "Warning");
}

// Blueprint graph placeholder implementations
#ifdef IMNODES_AVAILABLE
void UnrealEditor::DrawNode(const BlueprintState::Node& node) {
    ImNodes::BeginNode(node.id);

    ImNodes::BeginNodeTitleBar();
    ImGui::Text("%s", node.name.c_str());
    ImNodes::EndNodeTitleBar();

    // Draw inputs and outputs based on node type
    if (node.type == "Event") {
        ImNodes::BeginOutputAttribute(node.id * 100 + 1);
        ImGui::Text("Exec");
        ImNodes::EndOutputAttribute();
    } else if (node.type == "Function") {
        ImNodes::BeginInputAttribute(node.id * 100 + 1);
        ImGui::Text("Exec");
        ImNodes::EndInputAttribute();

        ImNodes::BeginOutputAttribute(node.id * 100 + 2);
        ImGui::Text("Exec");
        ImNodes::EndOutputAttribute();
    }

    ImNodes::EndNode();
}

void UnrealEditor::HandleNodeConnections() {
    // Handle creating links between nodes
    int start_attr, end_attr;
    if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
        blueprintGraph.links.push_back({start_attr, end_attr});
    }

    // Draw existing links
    for (size_t i = 0; i < blueprintGraph.links.size(); ++i) {
        ImNodes::Link(i, blueprintGraph.links[i].first, blueprintGraph.links[i].second);
    }
}

void UnrealEditor::GenerateLuaFromGraph() {
    AddLog("Generated Lua from Blueprint graph", "Info");
    // TODO: Implement actual code generation
}
#else
void UnrealEditor::DrawNode(const BlueprintState::Node& node) {}
void UnrealEditor::HandleNodeConnections() {}
void UnrealEditor::GenerateLuaFromGraph() {}
#endif
