#pragma once
#include <imgui.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

/**
 * Visual Blueprint Editor
 * Node-based visual scripting system equivalent to script capabilities
 */
class BlueprintEditor {
public:
    BlueprintEditor();
    ~BlueprintEditor();

    void Init();
    void Shutdown();
    void Update(float deltaTime);
    void Render();

    // Node types
    enum class NodeType {
        Event,
        Function,
        Variable,
        Operator,
        FlowControl,
        Comment,
        Custom
    };

    // Pin types
    enum class PinType {
        Input,
        Output
    };

    // Data types
    enum class DataType {
        Exec,
        Bool,
        Int,
        Float,
        String,
        Vector,
        Object,
        Custom
    };

    // Core structures
    struct Pin {
        uint32_t id;
        uint32_t nodeId;
        PinType type;
        DataType dataType;
        std::string name;
        int index;
        std::vector<uint32_t> connections;
    };

    struct Node {
        uint32_t id;
        NodeType type;
        std::string name;
        glm::vec2 position;
        glm::vec2 size;
        std::vector<std::shared_ptr<Pin>> pins;
    };

    struct Connection {
        uint32_t id;
        std::shared_ptr<Pin> outputPin;
        std::shared_ptr<Pin> inputPin;
    };

    // Node management
    void CreateNode(NodeType type, const glm::vec2& position);
    void DeleteNode(uint32_t nodeId);
    void CreateConnection(uint32_t outputPinId, uint32_t inputPinId);
    void DeleteConnection(uint32_t connectionId);

    // Code generation
    std::string GenerateCode() const;
    std::string GenerateSproutScript() const;

    // Persistence
    void SaveBlueprint(const std::string& filename);
    void LoadBlueprint(const std::string& filename);
    void Clear();

    // Getters
    std::shared_ptr<Node> GetNode(uint32_t nodeId);
    std::shared_ptr<Node> GetNode(uint32_t nodeId) const;
    std::shared_ptr<Pin> GetPin(uint32_t pinId);
    std::shared_ptr<Connection> GetConnection(uint32_t connectionId);
    std::shared_ptr<Connection> GetConnection(uint32_t connectionId) const;

private:
    // Data storage
    std::vector<std::shared_ptr<Node>> nodes;
    std::unordered_map<uint32_t, std::shared_ptr<Node>> nodeMap;
    std::vector<std::shared_ptr<Connection>> connections;
    std::unordered_map<uint32_t, std::shared_ptr<Connection>> connectionMap;
    std::vector<std::shared_ptr<Pin>> pins;
    std::unordered_map<uint32_t, std::shared_ptr<Pin>> pinMap;

    // State management
    uint32_t nodeIdCounter = 1;
    uint32_t connectionIdCounter = 1;
    uint32_t pinIdCounter = 1;
    uint32_t selectedNodeId = 0;
    uint32_t draggedNodeId = 0;

    // Linking state
    bool isLinking = false;
    std::shared_ptr<Pin> linkStartPin = nullptr;

    // Canvas state
    glm::vec2 canvasOffset{0.0f, 0.0f};
    float canvasZoom = 1.0f;
    glm::vec2 contextMenuPos{0.0f, 0.0f};

    // Visibility
    bool isVisible = true;

    // Generated code
    std::string generatedCode;

    // Initialization
    void InitializeDefaultNodes();

    // Rendering
    void RenderToolbar();
    void RenderCanvas();
    void RenderStatusBar();
    void RenderGrid(ImDrawList* drawList, const ImVec2& canvasPos, const ImVec2& canvasSize);
    void RenderNode(ImDrawList* drawList, const ImVec2& canvasPos, std::shared_ptr<Node> node);
    void RenderPin(ImDrawList* drawList, const ImVec2& canvasPos, std::shared_ptr<Pin> pin);
    void RenderConnection(ImDrawList* drawList, const ImVec2& canvasPos, std::shared_ptr<Connection> connection);

    // Interaction
    void HandleCanvasInteraction(const ImVec2& canvasPos, const ImVec2& canvasSize);
    void HandleNodeInteraction(const ImVec2& nodePos, const ImVec2& nodeSize, std::shared_ptr<Node> node);
    void HandlePinInteraction(const ImVec2& pinPos, std::shared_ptr<Pin> pin);

    // Utilities
    ImVec2 GetPinScreenPosition(const ImVec2& canvasPos, std::shared_ptr<Pin> pin);
    ImU32 GetNodeColor(NodeType type);
    ImU32 GetPinColor(DataType dataType);
    bool ArePinTypesCompatible(DataType outputType, DataType inputType);
    void RemoveConnectionsToPin(uint32_t pinId);

    // Node setup helpers
    void SetupEventNode(std::shared_ptr<Node> node);
    void SetupFunctionNode(std::shared_ptr<Node> node);
    void SetupVariableNode(std::shared_ptr<Node> node);
    void SetupOperatorNode(std::shared_ptr<Node> node);
    void SetupFlowControlNode(std::shared_ptr<Node> node);
    void SetupCommentNode(std::shared_ptr<Node> node);

    // Code generation helpers
    void GenerateEventCode(std::shared_ptr<Node> eventNode, std::ostringstream& code) const;
    void GenerateExecutionChain(std::shared_ptr<Pin> execPin, std::ostringstream& code, int indentLevel) const;
    void GenerateEventSproutScript(std::shared_ptr<Node> eventNode, std::ostringstream& script) const;
    void GenerateSproutExecutionChain(std::shared_ptr<Pin> execPin, std::ostringstream& script, int indentLevel) const;
    std::string GetVariableDeclaration(std::shared_ptr<Node> varNode) const;
};
