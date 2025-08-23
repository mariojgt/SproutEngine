#pragma once
#include <imgui.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "TinyImGui.h"
#include "ModernTheme.h"

class Renderer;
class Scripting;

/**
 * Simplified Unreal-like Editor System
 * Compatible with current ImGui and component setup
 */
class UnrealEditor {
public:
    UnrealEditor();
    ~UnrealEditor();

    bool Init(struct GLFWwindow* window);
    void Shutdown(struct GLFWwindow* window);

    void Update(float deltaTime);
    void Render(entt::registry& registry, Renderer& renderer, Scripting& scripting, bool& playMode);

    // Expose selected entity for external rendering/helpers
    entt::entity GetSelectedEntity() const { return selectedEntity; }

    // ...existing code...

    // Simple blueprint/code editor state
    bool showBlueprintEditor = false;
    std::string currentBlueprintPath;
    std::string currentBlueprintCode;
    // Editable buffer for ImGui InputTextMultiline
    std::vector<char> blueprintEditBuffer;
    // Whether this editor initialized ImGui backends
    bool ownsImGuiBackends = false;

    // Blueprint node system
    struct BlueprintNode {
        int id;
        std::string type; // "Event", "Function", "Variable", "Math"
        std::string name;
        ImVec2 position;
        std::string param1, param2, param3; // node parameters
        std::vector<int> inputPins, outputPins;
    };
    std::vector<BlueprintNode> blueprintNodes;
    std::vector<std::pair<int, int>> blueprintLinks; // pin connections
    int nextNodeId = 1;

private:
    // Core editor state
    struct GLFWwindow* editorWindow = nullptr;
    entt::entity selectedEntity = entt::null;
    bool showDemoWindow = false;
    bool showMetrics = false;

    // Panel visibility flags
    bool showViewport = true;
    bool showContentBrowser = true;
    bool showWorldOutliner = true;
    bool showInspector = true;
    bool showBlueprintGraph = false;
    bool showConsole = true;
    bool showMaterialEditor = false;
    bool showRoadmap = true;

    // Editor state
    enum class EditorMode {
        Edit,
        Play,
        Simulate
    };
    EditorMode currentMode = EditorMode::Edit;

    // Camera controls for viewport
    struct ViewportCamera {
        glm::vec3 position{5, 3, 8};
        glm::vec3 target{0, 0, 0};
        glm::vec3 up{0, 1, 0};
        float fov = 60.0f;
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;
        bool isOrbiting = false;
        glm::vec2 lastMousePos{0, 0};
    } viewportCamera;

    // Content browser state
    struct ContentBrowserState {
        std::string currentPath = "assets/";
        std::vector<std::string> directories;
        std::vector<std::string> files;
        std::string selectedItem;
        bool needsRefresh = true;
    } contentBrowser;

    // Console state
    struct ConsoleState {
        std::vector<std::string> logs;
        std::string inputBuffer;
        bool autoScroll = true;
        int maxLogs = 1000;
    } console;

    // Panel implementations
    void DrawMainMenuBar(entt::registry& registry, Scripting& scripting, bool& playMode);
    void DrawViewport(entt::registry& registry, Renderer& renderer);
    void DrawContentBrowser();
    void DrawWorldOutliner(entt::registry& registry);
    void DrawInspector(entt::registry& registry, Scripting& scripting);
    void DrawBlueprintGraph(entt::registry& registry, Scripting& scripting);
    void DrawConsole(entt::registry& registry, Scripting& scripting);
    void DrawMaterialEditor();
    void DrawToolbar(bool& playMode);
    void DrawRoadmap();

    // Viewport selection via mouse
    void HandleEntitySelection(entt::registry& registry, ImVec2 mousePos, ImVec2 viewportSize);

    // Console functionality
    void AddLog(const std::string& message, const std::string& level = "Info");
    void ExecuteCommand(const std::string& command, entt::registry& registry, Scripting& scripting);

    // Utility functions
    std::string GetEntityName(entt::registry& registry, entt::entity entity);
    void SetEntityName(entt::registry& registry, entt::entity entity, const std::string& name);
    bool IsEntityValid(entt::registry& registry, entt::entity entity);

    // Entity operations
    entt::entity CreateEntity(entt::registry& registry, const std::string& name = "Entity");
    void DeleteEntity(entt::registry& registry, entt::entity entity);
    void DuplicateEntity(entt::registry& registry, entt::entity entity);

    // ...existing code...

    // Content browser functionality
    void RefreshContentBrowser();
    void DrawDirectoryTree();
    void DrawFileGrid();

    // File operations
    void NewScene(entt::registry& registry);
    void SaveScene(entt::registry& registry, const std::string& filepath);

    // Blueprint system methods
    void GenerateBlueprintSP();
    void GenerateLuaFromSP();
    void SaveCodeToFile();
    void ApplyScriptToSelected(entt::registry& registry, Scripting& scripting);
};
