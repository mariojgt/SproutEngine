#pragma once
#include <imgui.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <memory>
#include <functional>

class Renderer;
class Scripting;

/**
 * Complete Unreal-like Editor System
 * Implements all major editor panels and workflows
 */
class UnrealEditor {
public:
    UnrealEditor();
    ~UnrealEditor();

    bool Init(struct GLFWwindow* window);
    void Shutdown(struct GLFWwindow* window);

    // Expose selected entity for external rendering helpers
    entt::entity GetSelectedEntity() const { return selectedEntity; }

    void Update(float deltaTime);
    void Render(entt::registry& registry, Renderer& renderer, Scripting& scripting, bool& playMode);

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

    // Blueprint graph state
    struct BlueprintState {
        int nextNodeId = 1;
        struct Node {
            int id;
            std::string type;
            std::string name;
            ImVec2 position;
            std::vector<int> inputs;
            std::vector<int> outputs;
            // simple single string parameter for many nodes (e.g. print text)
            std::string param;
        };
        std::vector<Node> nodes;
        std::vector<std::pair<int, int>> links; // input->output pairs
    } blueprintGraph;
    // Currently loaded blueprint graph file path (empty if none)
    std::string currentBlueprintFile;

    // Node editor temporary state (for non-imnodes built-in editor)
    int linkDragStart = 0; // attribute id (node*100 + pin)
    int linkDragEnd = 0;
    entt::entity blueprintEditingEntity = entt::null; // entity blueprint is being edited for (optional)

    // Panel implementations
    void DrawMainMenuBar(entt::registry& registry, Scripting& scripting, bool& playMode);
    void DrawDockSpace();
    void DrawViewport(entt::registry& registry, Renderer& renderer);
    void DrawContentBrowser();
    void DrawWorldOutliner(entt::registry& registry);
    void DrawInspector(entt::registry& registry, Scripting& scripting);
    void DrawBlueprintGraph();
    void DrawConsole(entt::registry& registry, Scripting& scripting);
    void DrawMaterialEditor();
    void DrawToolbar(bool& playMode);

    // Whether this editor initialized ImGui platform/renderer backends
    bool ownsImGuiBackends = false;

    // Viewport functionality
    void HandleViewportInput();
    void UpdateViewportCamera(float deltaTime);
    void DrawGizmos(entt::registry& registry);
    void HandleEntitySelection(entt::registry& registry, ImVec2 mousePos, ImVec2 viewportSize);

    // Content browser functionality
    void RefreshContentBrowser();
    void DrawDirectoryTree();
    void DrawFileGrid();
    void HandleAssetDragDrop(entt::registry& registry);

    // Inspector functionality
    void DrawTransformComponent(entt::registry& registry, entt::entity entity);
    void DrawMeshComponent(entt::registry& registry, entt::entity entity);
    void DrawScriptComponent(entt::registry& registry, entt::entity entity, Scripting& scripting);
    void DrawAddComponentButton(entt::registry& registry, entt::entity entity);

    // Console functionality
    void AddLog(const std::string& message, const std::string& level = "Info");
    void ExecuteCommand(const std::string& command, entt::registry& registry, Scripting& scripting);

    // Blueprint functionality
    void DrawNode(const BlueprintState::Node& node);
    void HandleNodeConnections();
    void GenerateLuaFromGraph();

    // Utility functions
    std::string GetEntityName(entt::registry& registry, entt::entity entity);
    void SetEntityName(entt::registry& registry, entt::entity entity, const std::string& name);
    bool IsEntityValid(entt::registry& registry, entt::entity entity);

    // File operations
    void NewScene(entt::registry& registry);
    void OpenScene(entt::registry& registry, const std::string& filepath);
    void SaveScene(entt::registry& registry, const std::string& filepath);
    void ImportAsset(const std::string& filepath);

    // Entity operations
    entt::entity CreateEntity(entt::registry& registry, const std::string& name = "Entity");
    void DeleteEntity(entt::registry& registry, entt::entity entity);
    void DuplicateEntity(entt::registry& registry, entt::entity entity);
};
