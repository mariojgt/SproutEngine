#pragma once
#include <imgui.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "WebUIManager.h"

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
    bool showScriptEditor = false;
    bool showBlueprintEditor = false;

    // Editor state
    enum class EditorMode {
        Edit,
        Play,
        Simulate
    };
    EditorMode currentMode = EditorMode::Edit;

    // Script Editor state
    struct ScriptEditorState {
        std::string currentFile;
        std::string content;
        std::string textBuffer;
        std::string generatedCpp;
        bool isModified = false;
        bool showSyntaxHighlighting = true;
        bool showCppOutput = false;
        std::vector<std::string> keywords = {
            "class", "public", "private", "void", "int", "float", "bool", "string",
            "if", "else", "for", "while", "return", "true", "false",
            "EntityID", "Vector3", "Transform", "OnStart", "OnTick", "OnDestroy",
            "GetPosition", "SetPosition", "GetRotation", "SetRotation", "Print"
        };
    } scriptEditor;

    // Blueprint Editor state
    struct BlueprintPin {
        std::string name;
        std::string type;
    };

    struct BlueprintNode {
        int id;
        std::string type;
        std::string name;
        ImVec2 position;
        ImVec2 size;
        std::vector<BlueprintPin> inputPins;
        std::vector<BlueprintPin> outputPins;
        std::unordered_map<std::string, std::string> properties;
    };

    struct BlueprintConnection {
        int sourceNodeId;
        int sourcePinIndex;
        int targetNodeId;
        int targetPinIndex;
        std::string connectionType;
    };

    struct BlueprintEditorState {
        std::string currentFile;
        std::vector<BlueprintNode> nodes;
        std::vector<BlueprintConnection> connections;
        int nextNodeId = 1;
        int nextConnectionId = 1;
        bool isModified = false;
        int selectedNodeId = -1;
        bool isLinking = false;
        int linkStartNode = -1;
        std::string linkStartPin;
        std::string generatedCpp;
        bool showCppOutput = false;
        ImVec2 canvasOffset{0, 0};
    } blueprintEditor;

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
    void DrawBlueprintGraph();
    void DrawConsole(entt::registry& registry, Scripting& scripting);
    void DrawMaterialEditor();
    void DrawToolbar(bool& playMode);
    void DrawRoadmap();

    // New editor panels
    void DrawScriptEditor();
    void DrawBlueprintEditor();

    // Script Editor functionality
    void DrawScriptEditorContent();
    std::string CompileScriptToCpp(const std::string& spContent);
    void SaveScriptFile(const std::string& filename, const std::string& content);

    // Blueprint Editor functionality
    void DrawBlueprintNodeLibrary();
    void DrawBlueprintCanvas();
    void AddBlueprintNode(const std::string& name, const std::string& type, ImVec2 position);
    void DrawBlueprintNode(BlueprintNode& node, ImVec2 canvasPos, ImDrawList* drawList, bool isSelected = false);
    void DrawBlueprintConnection(BlueprintConnection& connection, ImVec2 canvasPos, ImDrawList* drawList);
    std::string CompileBlueprintToCpp();
    std::string GenerateNodeCode(const BlueprintNode& node);
    void SaveBlueprintFile(const std::string& filename);
    void RunBlueprint();
    void ExecuteBlueprintNode(int nodeId, std::unordered_map<int, bool>& executedNodes);

    // Legacy script functionality (keeping for compatibility)
    void OpenScriptFile(const std::string& filepath);
    void SaveScriptFile();
    void CompileScriptToLua(const std::string& spCode, std::string& luaOutput);
    void OpenBlueprintFile(const std::string& filepath);
    void SaveBlueprintFile();
    void CompileBlueprintToLua(std::string& luaOutput);

    // Blueprint node operations
    BlueprintNode* FindNode(int nodeId);
    void AddNode(const std::string& type, const glm::vec2& position);
    void DeleteNode(int nodeId);
    void ConnectNodes(int fromNode, const std::string& fromPin, int toNode, const std::string& toPin);
    void RenderNode(const BlueprintNode& node);
    void HandleNodeInteractions();

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

    // Content browser functionality
    void RefreshContentBrowser();
    void DrawDirectoryTree();
    void DrawFileGrid();

    // File operations
    void NewScene(entt::registry& registry);
    void SaveScene(entt::registry& registry, const std::string& filepath);

    // Web UI integration
    SproutEngine::WebUIManager webUIManager;
    void InitializeWebUI();
    void SetupWebAPIEndpoints(entt::registry& registry, Scripting& scripting);
};
