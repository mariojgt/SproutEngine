#pragma once
#include <imgui.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

/**
 * Advanced Blueprint Editor
 * Visual scripting system that can do everything Sprout Scripts can do
 */
class BlueprintEditor {
public:
    BlueprintEditor();
    ~BlueprintEditor();

    void Init();
    void Shutdown();
    void Render(bool* open = nullptr);

    // Blueprint operations
    void NewBlueprint();
    bool LoadBlueprint(const std::string& filepath);
    bool SaveBlueprint(const std::string& filepath = "");
    void CompileBlueprint();
    
    // Code generation
    std::string GenerateSproutScript() const;
    std::string GenerateCppCode() const;

private:
    // Node types
    enum class NodeType {
        Event,          // BeginPlay, Tick, Input events
        Function,       // Built-in functions
        Variable,       // Get/Set variables
        Operator,       // Math, logic operators
        FlowControl,    // If, While, For
        CustomFunction, // User-defined functions
        Comment
    };

    // Pin types
    enum class PinType {
        Execution,      // White execution pins
        Boolean,        // Red boolean pins
        Integer,        // Green integer pins
        Float,          // Green float pins
        String,         // Magenta string pins
        Vector3,        // Yellow vector3 pins
        Object,         // Blue object pins
        Wildcard        // Gray wildcard pins
    };

    // Pin structure
    struct Pin {
        int id;
        std::string name;
        PinType type;
        bool isInput;
        bool isConnected;
        std::string defaultValue;
        int connectedPin = -1; // ID of connected pin
        
        Pin(int i, const std::string& n, PinType t, bool input) 
            : id(i), name(n), type(t), isInput(input), isConnected(false) {}
    };

    // Node structure
    struct BlueprintNode {
        int id;
        NodeType type;
        std::string name;
        std::string function; // Function name or operation
        ImVec2 position;
        ImVec2 size;
        std::vector<Pin> inputPins;
        std::vector<Pin> outputPins;
        bool isSelected;
        
        // Node-specific data
        std::unordered_map<std::string, std::string> properties;
        
        BlueprintNode(int i, NodeType t, const std::string& n) 
            : id(i), type(t), name(n), position(100, 100), size(120, 80), isSelected(false) {}
    };

    // Connection between pins
    struct Connection {
        int id;
        int fromNode;
        int fromPin;
        int toNode;
        int toPin;
        
        Connection(int i, int fn, int fp, int tn, int tp)
            : id(i), fromNode(fn), fromPin(fp), toNode(tn), toPin(tp) {}
    };

    // Blueprint data
    std::vector<std::unique_ptr<BlueprintNode>> nodes;
    std::vector<Connection> connections;
    std::string blueprintName = "NewBlueprint";
    std::string blueprintPath;
    bool isModified = false;
    
    // Editor state
    int nextNodeId = 1;
    int nextPinId = 1;
    int nextConnectionId = 1;
    ImVec2 scrolling{0, 0};
    ImVec2 canvasSize{1000, 1000};
    bool showGrid = true;
    float gridSize = 64.0f;
    
    // Selection and interaction
    std::vector<int> selectedNodes;
    int hoveredNode = -1;
    int hoveredPin = -1;
    bool isDragging = false;
    bool isConnecting = false;
    int connectionStartPin = -1;
    
    // Node palette
    bool showNodePalette = false;
    ImVec2 nodeCreationPos;
    std::string nodeSearchText;

    // UI methods
    void ShowMenuBar();
    void ShowToolbar();
    void ShowCanvas();
    void ShowNodePalette();
    void ShowPropertiesPanel();
    void ShowVariablesPanel();
    void ShowDetailsPanel();
    
    // Canvas rendering
    void DrawGrid(ImDrawList* drawList, const ImVec2& canvasPos, const ImVec2& canvasSize);
    void DrawNode(ImDrawList* drawList, const BlueprintNode& node, const ImVec2& offset);
    void DrawPin(ImDrawList* drawList, const Pin& pin, const ImVec2& pos, float radius = 6.0f);
    void DrawConnection(ImDrawList* drawList, const Connection& conn, const ImVec2& offset);
    void DrawConnectionInProgress(ImDrawList* drawList, const ImVec2& startPos, const ImVec2& endPos);
    
    // Node management
    std::unique_ptr<BlueprintNode> CreateNode(NodeType type, const std::string& name, const ImVec2& pos);
    void DeleteNode(int nodeId);
    void DeleteSelectedNodes();
    BlueprintNode* FindNode(int id);
    Pin* FindPin(int nodeId, int pinId);
    
    // Connection management
    bool CanConnect(const Pin& fromPin, const Pin& toPin) const;
    void CreateConnection(int fromNode, int fromPin, int toNode, int toPin);
    void DeleteConnection(int connectionId);
    void DeleteConnectionsToPin(int nodeId, int pinId);
    
    // Input handling
    void HandleNodeInteraction();
    void HandleConnectionCreation();
    void HandleSelection();
    void HandleDragAndDrop();
    
    // Node templates
    std::unique_ptr<BlueprintNode> CreateEventNode(const std::string& eventName, const ImVec2& pos);
    std::unique_ptr<BlueprintNode> CreateFunctionNode(const std::string& functionName, const ImVec2& pos);
    std::unique_ptr<BlueprintNode> CreateVariableNode(const std::string& varName, bool isGetter, const ImVec2& pos);
    std::unique_ptr<BlueprintNode> CreateOperatorNode(const std::string& operation, const ImVec2& pos);
    std::unique_ptr<BlueprintNode> CreateFlowControlNode(const std::string& flowType, const ImVec2& pos);
    
    // Utility
    ImVec2 GetPinPos(const BlueprintNode& node, const Pin& pin, const ImVec2& nodePos) const;
    ImU32 GetPinColor(PinType type) const;
    ImU32 GetNodeColor(NodeType type) const;
    std::string GetNodeTitle(const BlueprintNode& node) const;
    
    // Code generation helpers
    std::string GenerateNodeCode(const BlueprintNode& node) const;
    std::vector<int> GetExecutionOrder() const;
    std::string GetPinValue(int nodeId, int pinId) const;
    
    // File operations
    bool LoadBlueprintFromFile(const std::string& filepath);
    bool SaveBlueprintToFile(const std::string& filepath);
    
    // Predefined node types
    struct NodeTemplate {
        std::string name;
        std::string category;
        NodeType type;
        std::string function;
        std::vector<std::pair<std::string, PinType>> inputs;
        std::vector<std::pair<std::string, PinType>> outputs;
        std::string description;
    };
    
    std::vector<NodeTemplate> GetAvailableNodes() const;
    std::vector<NodeTemplate> FilterNodes(const std::string& search) const;
};
