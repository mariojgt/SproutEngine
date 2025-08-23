#include "BlueprintEditor.h"
#include <algorithm>
#include <sstream>
#include <fstream>

BlueprintEditor::BlueprintEditor() {
    InitializeDefaultNodes();
}

BlueprintEditor::~BlueprintEditor() {
    Clear();
}

void BlueprintEditor::Init() {
    // Initialize any resources needed for blueprint editing
    nodeIdCounter = 1;
    connectionIdCounter = 1;
    selectedNodeId = 0;
    isLinking = false;
    linkStartPin = nullptr;
}

void BlueprintEditor::Shutdown() {
    Clear();
}

void BlueprintEditor::Update(float deltaTime) {
    // Update any animations or time-based operations
}

void BlueprintEditor::Render() {
    if (!isVisible) return;

    if (ImGui::Begin("Blueprint Editor", &isVisible)) {
        RenderToolbar();

        // Main blueprint canvas
        ImGui::BeginChild("BlueprintCanvas", ImVec2(0, -50), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        RenderCanvas();

        ImGui::EndChild();

        RenderStatusBar();
    }
    ImGui::End();
}

void BlueprintEditor::CreateNode(NodeType type, const glm::vec2& position) {
    auto node = std::make_shared<Node>();
    node->id = nodeIdCounter++;
    node->type = type;
    node->position = position;
    node->size = glm::vec2(150, 60); // Default size

    // Set node properties based on type
    switch (type) {
        case NodeType::Event:
            SetupEventNode(node);
            break;
        case NodeType::Function:
            SetupFunctionNode(node);
            break;
        case NodeType::Variable:
            SetupVariableNode(node);
            break;
        case NodeType::Operator:
            SetupOperatorNode(node);
            break;
        case NodeType::FlowControl:
            SetupFlowControlNode(node);
            break;
        case NodeType::Comment:
            SetupCommentNode(node);
            break;
        case NodeType::Custom:
            // Custom nodes have user-defined setup
            break;
    }

    nodes.push_back(node);
    nodeMap[node->id] = node;
}

void BlueprintEditor::DeleteNode(uint32_t nodeId) {
    auto nodeIt = nodeMap.find(nodeId);
    if (nodeIt == nodeMap.end()) return;

    auto node = nodeIt->second;

    // Remove all connections to/from this node
    std::vector<uint32_t> connectionsToRemove;
    for (const auto& connPair : connectionMap) {
        const auto& connection = connPair.second;
        if (connection->outputPin->nodeId == nodeId || connection->inputPin->nodeId == nodeId) {
            connectionsToRemove.push_back(connection->id);
        }
    }

    for (uint32_t connId : connectionsToRemove) {
        DeleteConnection(connId);
    }

    // Remove from containers
    nodes.erase(std::remove_if(nodes.begin(), nodes.end(),
        [nodeId](const auto& n) { return n->id == nodeId; }), nodes.end());
    nodeMap.erase(nodeIt);

    if (selectedNodeId == nodeId) {
        selectedNodeId = 0;
    }
}

void BlueprintEditor::CreateConnection(uint32_t outputPinId, uint32_t inputPinId) {
    auto outputPin = GetPin(outputPinId);
    auto inputPin = GetPin(inputPinId);

    if (!outputPin || !inputPin) return;
    if (outputPin->type != PinType::Output || inputPin->type != PinType::Input) return;

    // Type compatibility check
    if (!ArePinTypesCompatible(outputPin->dataType, inputPin->dataType)) return;

    // Remove existing connection to input pin
    RemoveConnectionsToPin(inputPinId);

    auto connection = std::make_shared<Connection>();
    connection->id = connectionIdCounter++;
    connection->outputPin = outputPin;
    connection->inputPin = inputPin;

    connections.push_back(connection);
    connectionMap[connection->id] = connection;

    // Update pin references
    outputPin->connections.push_back(connection->id);
    inputPin->connections.push_back(connection->id);
}

void BlueprintEditor::DeleteConnection(uint32_t connectionId) {
    auto connIt = connectionMap.find(connectionId);
    if (connIt == connectionMap.end()) return;

    auto connection = connIt->second;

    // Remove from pin connection lists
    auto& outputConnections = connection->outputPin->connections;
    outputConnections.erase(std::remove(outputConnections.begin(), outputConnections.end(), connectionId), outputConnections.end());

    auto& inputConnections = connection->inputPin->connections;
    inputConnections.erase(std::remove(inputConnections.begin(), inputConnections.end(), connectionId), inputConnections.end());

    // Remove from containers
    connections.erase(std::remove_if(connections.begin(), connections.end(),
        [connectionId](const auto& c) { return c->id == connectionId; }), connections.end());
    connectionMap.erase(connIt);
}

std::string BlueprintEditor::GenerateCode() const {
    std::ostringstream code;

    code << "// Generated from Blueprint\n";
    code << "#include \"Actor.h\"\n\n";

    // Find entry point (Event nodes)
    std::vector<std::shared_ptr<Node>> eventNodes;
    for (const auto& node : nodes) {
        if (node->type == NodeType::Event) {
            eventNodes.push_back(node);
        }
    }

    // Generate code for each event
    for (const auto& eventNode : eventNodes) {
        GenerateEventCode(eventNode, code);
    }

    return code.str();
}

std::string BlueprintEditor::GenerateSproutScript() const {
    std::ostringstream script;

    script << "// Generated Blueprint Script\n";
    script << "class BlueprintActor : Actor {\n";

    // Generate member variables
    for (const auto& node : nodes) {
        if (node->type == NodeType::Variable) {
            script << "    " << GetVariableDeclaration(node) << ";\n";
        }
    }

    script << "\n";

    // Generate functions
    std::vector<std::shared_ptr<Node>> eventNodes;
    for (const auto& node : nodes) {
        if (node->type == NodeType::Event) {
            eventNodes.push_back(node);
        }
    }

    for (const auto& eventNode : eventNodes) {
        GenerateEventSproutScript(eventNode, script);
    }

    script << "}\n";

    return script.str();
}

void BlueprintEditor::SaveBlueprint(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    // Save blueprint data in JSON-like format
    file << "{\n";
    file << "  \"nodes\": [\n";

    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        file << "    {\n";
        file << "      \"id\": " << node->id << ",\n";
        file << "      \"type\": " << static_cast<int>(node->type) << ",\n";
        file << "      \"name\": \"" << node->name << "\",\n";
        file << "      \"position\": [" << node->position.x << ", " << node->position.y << "],\n";
        file << "      \"size\": [" << node->size.x << ", " << node->size.y << "]\n";
        file << "    }";
        if (i < nodes.size() - 1) file << ",";
        file << "\n";
    }

    file << "  ],\n";
    file << "  \"connections\": [\n";

    for (size_t i = 0; i < connections.size(); ++i) {
        const auto& conn = connections[i];
        file << "    {\n";
        file << "      \"id\": " << conn->id << ",\n";
        file << "      \"outputPin\": " << conn->outputPin->id << ",\n";
        file << "      \"inputPin\": " << conn->inputPin->id << "\n";
        file << "    }";
        if (i < connections.size() - 1) file << ",";
        file << "\n";
    }

    file << "  ]\n";
    file << "}\n";

    file.close();
}

void BlueprintEditor::LoadBlueprint(const std::string& filename) {
    // Clear existing data
    Clear();

    std::ifstream file(filename);
    if (!file.is_open()) return;

    // For now, this is a simplified loader
    // In a real implementation, you'd use a JSON parser
    std::string line;
    while (std::getline(file, line)) {
        // Parse blueprint data
        // This would be a full JSON parser in practice
    }

    file.close();
}

void BlueprintEditor::Clear() {
    nodes.clear();
    nodeMap.clear();
    connections.clear();
    connectionMap.clear();
    pins.clear();
    pinMap.clear();
    selectedNodeId = 0;
    isLinking = false;
    linkStartPin = nullptr;
    nodeIdCounter = 1;
    connectionIdCounter = 1;
    pinIdCounter = 1;
}

std::shared_ptr<BlueprintEditor::Node> BlueprintEditor::GetNode(uint32_t nodeId) {
    auto it = nodeMap.find(nodeId);
    return (it != nodeMap.end()) ? it->second : nullptr;
}

std::shared_ptr<BlueprintEditor::Node> BlueprintEditor::GetNode(uint32_t nodeId) const {
    auto it = nodeMap.find(nodeId);
    return (it != nodeMap.end()) ? it->second : nullptr;
}

std::shared_ptr<BlueprintEditor::Pin> BlueprintEditor::GetPin(uint32_t pinId) {
    auto it = pinMap.find(pinId);
    return (it != pinMap.end()) ? it->second : nullptr;
}

std::shared_ptr<BlueprintEditor::Connection> BlueprintEditor::GetConnection(uint32_t connectionId) {
    auto it = connectionMap.find(connectionId);
    return (it != connectionMap.end()) ? it->second : nullptr;
}

void BlueprintEditor::InitializeDefaultNodes() {
    // This would initialize any default node types or templates
}

void BlueprintEditor::RenderToolbar() {
    if (ImGui::Button("Add Event")) {
        CreateNode(NodeType::Event, glm::vec2(100, 100));
    }
    ImGui::SameLine();

    if (ImGui::Button("Add Function")) {
        CreateNode(NodeType::Function, glm::vec2(100, 200));
    }
    ImGui::SameLine();

    if (ImGui::Button("Add Variable")) {
        CreateNode(NodeType::Variable, glm::vec2(100, 300));
    }
    ImGui::SameLine();

    if (ImGui::Button("Generate Code")) {
        generatedCode = GenerateCode();
    }
    ImGui::SameLine();

    if (ImGui::Button("Clear")) {
        Clear();
    }
}

void BlueprintEditor::RenderCanvas() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();

    // Draw grid
    RenderGrid(drawList, canvasPos, canvasSize);

    // Handle canvas interaction
    HandleCanvasInteraction(canvasPos, canvasSize);

    // Render connections
    for (const auto& connection : connections) {
        RenderConnection(drawList, canvasPos, connection);
    }

    // Render nodes
    for (const auto& node : nodes) {
        RenderNode(drawList, canvasPos, node);
    }

    // Render linking line
    if (isLinking && linkStartPin) {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 startPos = GetPinScreenPosition(canvasPos, linkStartPin);
        drawList->AddLine(startPos, mousePos, IM_COL32(255, 255, 100, 255), 2.0f);
    }
}

void BlueprintEditor::RenderStatusBar() {
    ImGui::Text("Nodes: %zu | Connections: %zu", nodes.size(), connections.size());
    if (selectedNodeId > 0) {
        ImGui::SameLine();
        ImGui::Text("| Selected: Node %u", selectedNodeId);
    }
}

void BlueprintEditor::RenderGrid(ImDrawList* drawList, const ImVec2& canvasPos, const ImVec2& canvasSize) {
    const float gridStep = 50.0f;
    const ImU32 gridColor = IM_COL32(100, 100, 100, 40);

    for (float x = fmodf(canvasOffset.x, gridStep); x < canvasSize.x; x += gridStep) {
        drawList->AddLine(
            ImVec2(canvasPos.x + x, canvasPos.y),
            ImVec2(canvasPos.x + x, canvasPos.y + canvasSize.y),
            gridColor
        );
    }

    for (float y = fmodf(canvasOffset.y, gridStep); y < canvasSize.y; y += gridStep) {
        drawList->AddLine(
            ImVec2(canvasPos.x, canvasPos.y + y),
            ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + y),
            gridColor
        );
    }
}

void BlueprintEditor::RenderNode(ImDrawList* drawList, const ImVec2& canvasPos, std::shared_ptr<Node> node) {
    ImVec2 nodePos = ImVec2(
        canvasPos.x + node->position.x + canvasOffset.x,
        canvasPos.y + node->position.y + canvasOffset.y
    );
    ImVec2 nodeSize = ImVec2(node->size.x, node->size.y);

    // Node background
    ImU32 nodeColor = GetNodeColor(node->type);
    if (selectedNodeId == node->id) {
        nodeColor = IM_COL32(255, 200, 100, 255); // Highlight selected
    }

    drawList->AddRectFilled(nodePos, ImVec2(nodePos.x + nodeSize.x, nodePos.y + nodeSize.y), nodeColor, 5.0f);
    drawList->AddRect(nodePos, ImVec2(nodePos.x + nodeSize.x, nodePos.y + nodeSize.y), IM_COL32(255, 255, 255, 255), 5.0f, 0, 2.0f);

    // Node title
    ImVec2 textPos = ImVec2(nodePos.x + 10, nodePos.y + 10);
    drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), node->name.c_str());

    // Render pins
    for (const auto& pin : node->pins) {
        RenderPin(drawList, canvasPos, pin);
    }

    // Handle node interaction
    HandleNodeInteraction(nodePos, nodeSize, node);
}

void BlueprintEditor::RenderPin(ImDrawList* drawList, const ImVec2& canvasPos, std::shared_ptr<Pin> pin) {
    ImVec2 pinPos = GetPinScreenPosition(canvasPos, pin);
    ImU32 pinColor = GetPinColor(pin->dataType);

    float pinRadius = 6.0f;
    drawList->AddCircleFilled(pinPos, pinRadius, pinColor);
    drawList->AddCircle(pinPos, pinRadius, IM_COL32(255, 255, 255, 255), 0, 2.0f);

    // Pin label
    if (pin->type == PinType::Input) {
        ImVec2 textPos = ImVec2(pinPos.x + 15, pinPos.y - 8);
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), pin->name.c_str());
    } else {
        ImVec2 textSize = ImGui::CalcTextSize(pin->name.c_str());
        ImVec2 textPos = ImVec2(pinPos.x - textSize.x - 15, pinPos.y - 8);
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), pin->name.c_str());
    }

    // Handle pin interaction
    HandlePinInteraction(pinPos, pin);
}

void BlueprintEditor::RenderConnection(ImDrawList* drawList, const ImVec2& canvasPos, std::shared_ptr<Connection> connection) {
    ImVec2 startPos = GetPinScreenPosition(canvasPos, connection->outputPin);
    ImVec2 endPos = GetPinScreenPosition(canvasPos, connection->inputPin);

    // Bezier curve for connection
    ImVec2 cp1 = ImVec2(startPos.x + 50, startPos.y);
    ImVec2 cp2 = ImVec2(endPos.x - 50, endPos.y);

    ImU32 connectionColor = GetPinColor(connection->outputPin->dataType);
    drawList->AddBezierCubic(startPos, cp1, cp2, endPos, connectionColor, 3.0f);
}

void BlueprintEditor::HandleCanvasInteraction(const ImVec2& canvasPos, const ImVec2& canvasSize) {
    ImGuiIO& io = ImGui::GetIO();

    // Canvas panning
    if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
        canvasOffset.x += io.MouseDelta.x;
        canvasOffset.y += io.MouseDelta.y;
    }

    // Canvas zooming
    if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f) {
        canvasZoom += io.MouseWheel * 0.1f;
        canvasZoom = std::clamp(canvasZoom, 0.5f, 2.0f);
    }

    // Right-click context menu
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        contextMenuPos = glm::vec2(io.MousePos.x - canvasPos.x - canvasOffset.x, io.MousePos.y - canvasPos.y - canvasOffset.y);
        ImGui::OpenPopup("CanvasContextMenu");
    }

    if (ImGui::BeginPopup("CanvasContextMenu")) {
        if (ImGui::MenuItem("Add Event Node")) {
            CreateNode(NodeType::Event, contextMenuPos);
        }
        if (ImGui::MenuItem("Add Function Node")) {
            CreateNode(NodeType::Function, contextMenuPos);
        }
        if (ImGui::MenuItem("Add Variable Node")) {
            CreateNode(NodeType::Variable, contextMenuPos);
        }
        ImGui::EndPopup();
    }
}

void BlueprintEditor::HandleNodeInteraction(const ImVec2& nodePos, const ImVec2& nodeSize, std::shared_ptr<Node> node) {
    ImGuiIO& io = ImGui::GetIO();

    bool isHovered = (io.MousePos.x >= nodePos.x && io.MousePos.x <= nodePos.x + nodeSize.x &&
                     io.MousePos.y >= nodePos.y && io.MousePos.y <= nodePos.y + nodeSize.y);

    if (isHovered) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            selectedNodeId = node->id;
            draggedNodeId = node->id;
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            selectedNodeId = node->id;
            ImGui::OpenPopup("NodeContextMenu");
        }
    }

    // Node dragging
    if (draggedNodeId == node->id && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        node->position.x += io.MouseDelta.x;
        node->position.y += io.MouseDelta.y;
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        draggedNodeId = 0;
    }

    // Node context menu
    if (ImGui::BeginPopup("NodeContextMenu") && selectedNodeId == node->id) {
        if (ImGui::MenuItem("Delete Node")) {
            DeleteNode(node->id);
        }
        if (ImGui::MenuItem("Duplicate Node")) {
            CreateNode(node->type, node->position + glm::vec2(20, 20));
        }
        ImGui::EndPopup();
    }
}

void BlueprintEditor::HandlePinInteraction(const ImVec2& pinPos, std::shared_ptr<Pin> pin) {
    ImGuiIO& io = ImGui::GetIO();
    float pinRadius = 8.0f;

    bool isHovered = (glm::distance(glm::vec2(io.MousePos.x, io.MousePos.y), glm::vec2(pinPos.x, pinPos.y)) <= pinRadius);

    if (isHovered) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            if (isLinking) {
                // Complete connection
                if (linkStartPin->type != pin->type) {
                    if (linkStartPin->type == PinType::Output) {
                        CreateConnection(linkStartPin->id, pin->id);
                    } else {
                        CreateConnection(pin->id, linkStartPin->id);
                    }
                }
                isLinking = false;
                linkStartPin = nullptr;
            } else {
                // Start linking
                isLinking = true;
                linkStartPin = pin;
            }
        }
    }
}

ImVec2 BlueprintEditor::GetPinScreenPosition(const ImVec2& canvasPos, std::shared_ptr<Pin> pin) {
    auto node = GetNode(pin->nodeId);
    if (!node) return ImVec2(0, 0);

    float pinY = node->position.y + 30 + pin->index * 20; // Approximate pin spacing
    float pinX = (pin->type == PinType::Input) ?
                 node->position.x :
                 node->position.x + node->size.x;

    return ImVec2(
        canvasPos.x + pinX + canvasOffset.x,
        canvasPos.y + pinY + canvasOffset.y
    );
}

ImU32 BlueprintEditor::GetNodeColor(NodeType type) {
    switch (type) {
        case NodeType::Event:      return IM_COL32(200, 100, 100, 255);
        case NodeType::Function:   return IM_COL32(100, 100, 200, 255);
        case NodeType::Variable:   return IM_COL32(100, 200, 100, 255);
        case NodeType::Operator:   return IM_COL32(150, 150, 100, 255);
        case NodeType::FlowControl:return IM_COL32(200, 150, 100, 255);
        case NodeType::Comment:    return IM_COL32(120, 120, 120, 255);
        default:                   return IM_COL32(100, 100, 100, 255);
    }
}

ImU32 BlueprintEditor::GetPinColor(DataType dataType) {
    switch (dataType) {
        case DataType::Exec:    return IM_COL32(255, 255, 255, 255);
        case DataType::Bool:    return IM_COL32(200, 100, 100, 255);
        case DataType::Int:     return IM_COL32(100, 200, 100, 255);
        case DataType::Float:   return IM_COL32(100, 100, 200, 255);
        case DataType::String:  return IM_COL32(200, 100, 200, 255);
        case DataType::Vector:  return IM_COL32(255, 200, 100, 255);
        case DataType::Object:  return IM_COL32(100, 200, 200, 255);
        default:                return IM_COL32(128, 128, 128, 255);
    }
}

bool BlueprintEditor::ArePinTypesCompatible(DataType outputType, DataType inputType) {
    if (outputType == inputType) return true;

    // Allow some implicit conversions
    if (outputType == DataType::Int && inputType == DataType::Float) return true;
    if (outputType == DataType::Float && inputType == DataType::Int) return true;

    return false;
}

void BlueprintEditor::RemoveConnectionsToPin(uint32_t pinId) {
    std::vector<uint32_t> connectionsToRemove;

    for (const auto& connPair : connectionMap) {
        const auto& connection = connPair.second;
        if (connection->inputPin->id == pinId) {
            connectionsToRemove.push_back(connection->id);
        }
    }

    for (uint32_t connId : connectionsToRemove) {
        DeleteConnection(connId);
    }
}

void BlueprintEditor::SetupEventNode(std::shared_ptr<Node> node) {
    node->name = "Event BeginPlay";
    node->size = glm::vec2(150, 60);

    // Output execution pin
    auto execPin = std::make_shared<Pin>();
    execPin->id = pinIdCounter++;
    execPin->nodeId = node->id;
    execPin->type = PinType::Output;
    execPin->dataType = DataType::Exec;
    execPin->name = "";
    execPin->index = 0;

    node->pins.push_back(execPin);
    pins.push_back(execPin);
    pinMap[execPin->id] = execPin;
}

void BlueprintEditor::SetupFunctionNode(std::shared_ptr<Node> node) {
    node->name = "Function Call";
    node->size = glm::vec2(150, 80);

    // Input execution pin
    auto execInPin = std::make_shared<Pin>();
    execInPin->id = pinIdCounter++;
    execInPin->nodeId = node->id;
    execInPin->type = PinType::Input;
    execInPin->dataType = DataType::Exec;
    execInPin->name = "";
    execInPin->index = 0;

    // Output execution pin
    auto execOutPin = std::make_shared<Pin>();
    execOutPin->id = pinIdCounter++;
    execOutPin->nodeId = node->id;
    execOutPin->type = PinType::Output;
    execOutPin->dataType = DataType::Exec;
    execOutPin->name = "";
    execOutPin->index = 0;

    node->pins.push_back(execInPin);
    node->pins.push_back(execOutPin);
    pins.push_back(execInPin);
    pins.push_back(execOutPin);
    pinMap[execInPin->id] = execInPin;
    pinMap[execOutPin->id] = execOutPin;
}

void BlueprintEditor::SetupVariableNode(std::shared_ptr<Node> node) {
    node->name = "Variable";
    node->size = glm::vec2(120, 60);

    // Output data pin
    auto dataPin = std::make_shared<Pin>();
    dataPin->id = pinIdCounter++;
    dataPin->nodeId = node->id;
    dataPin->type = PinType::Output;
    dataPin->dataType = DataType::Float;
    dataPin->name = "Value";
    dataPin->index = 0;

    node->pins.push_back(dataPin);
    pins.push_back(dataPin);
    pinMap[dataPin->id] = dataPin;
}

void BlueprintEditor::SetupOperatorNode(std::shared_ptr<Node> node) {
    node->name = "Add";
    node->size = glm::vec2(100, 80);

    // Input pins
    auto inputA = std::make_shared<Pin>();
    inputA->id = pinIdCounter++;
    inputA->nodeId = node->id;
    inputA->type = PinType::Input;
    inputA->dataType = DataType::Float;
    inputA->name = "A";
    inputA->index = 0;

    auto inputB = std::make_shared<Pin>();
    inputB->id = pinIdCounter++;
    inputB->nodeId = node->id;
    inputB->type = PinType::Input;
    inputB->dataType = DataType::Float;
    inputB->name = "B";
    inputB->index = 1;

    // Output pin
    auto output = std::make_shared<Pin>();
    output->id = pinIdCounter++;
    output->nodeId = node->id;
    output->type = PinType::Output;
    output->dataType = DataType::Float;
    output->name = "Result";
    output->index = 0;

    node->pins.push_back(inputA);
    node->pins.push_back(inputB);
    node->pins.push_back(output);
    pins.push_back(inputA);
    pins.push_back(inputB);
    pins.push_back(output);
    pinMap[inputA->id] = inputA;
    pinMap[inputB->id] = inputB;
    pinMap[output->id] = output;
}

void BlueprintEditor::SetupFlowControlNode(std::shared_ptr<Node> node) {
    node->name = "Branch";
    node->size = glm::vec2(120, 100);

    // Input execution pin
    auto execIn = std::make_shared<Pin>();
    execIn->id = pinIdCounter++;
    execIn->nodeId = node->id;
    execIn->type = PinType::Input;
    execIn->dataType = DataType::Exec;
    execIn->name = "";
    execIn->index = 0;

    // Condition input
    auto condition = std::make_shared<Pin>();
    condition->id = pinIdCounter++;
    condition->nodeId = node->id;
    condition->type = PinType::Input;
    condition->dataType = DataType::Bool;
    condition->name = "Condition";
    condition->index = 1;

    // True output
    auto trueOut = std::make_shared<Pin>();
    trueOut->id = pinIdCounter++;
    trueOut->nodeId = node->id;
    trueOut->type = PinType::Output;
    trueOut->dataType = DataType::Exec;
    trueOut->name = "True";
    trueOut->index = 0;

    // False output
    auto falseOut = std::make_shared<Pin>();
    falseOut->id = pinIdCounter++;
    falseOut->nodeId = node->id;
    falseOut->type = PinType::Output;
    falseOut->dataType = DataType::Exec;
    falseOut->name = "False";
    falseOut->index = 1;

    node->pins.push_back(execIn);
    node->pins.push_back(condition);
    node->pins.push_back(trueOut);
    node->pins.push_back(falseOut);
    pins.push_back(execIn);
    pins.push_back(condition);
    pins.push_back(trueOut);
    pins.push_back(falseOut);
    pinMap[execIn->id] = execIn;
    pinMap[condition->id] = condition;
    pinMap[trueOut->id] = trueOut;
    pinMap[falseOut->id] = falseOut;
}

void BlueprintEditor::SetupCommentNode(std::shared_ptr<Node> node) {
    node->name = "Comment";
    node->size = glm::vec2(200, 100);
    // Comments don't have pins
}

void BlueprintEditor::GenerateEventCode(std::shared_ptr<Node> eventNode, std::ostringstream& code) const {
    code << "void " << eventNode->name << "() {\n";

    // Find connected nodes and generate code
    for (const auto& pin : eventNode->pins) {
        if (pin->type == PinType::Output && pin->dataType == DataType::Exec) {
            GenerateExecutionChain(pin, code, 1);
        }
    }

    code << "}\n\n";
}

void BlueprintEditor::GenerateExecutionChain(std::shared_ptr<Pin> execPin, std::ostringstream& code, int indentLevel) const {
    for (uint32_t connId : execPin->connections) {
        auto connection = GetConnection(connId);
        if (!connection) continue;

        auto targetNode = GetNode(connection->inputPin->nodeId);
        if (!targetNode) continue;

        std::string indent(indentLevel * 2, ' ');

        switch (targetNode->type) {
            case NodeType::Function:
                code << indent << targetNode->name << "();\n";
                break;
            case NodeType::FlowControl:
                code << indent << "if (condition) {\n";
                // Generate true branch
                code << indent << "} else {\n";
                // Generate false branch
                code << indent << "}\n";
                break;
            default:
                break;
        }

        // Continue execution chain
        for (const auto& pin : targetNode->pins) {
            if (pin->type == PinType::Output && pin->dataType == DataType::Exec) {
                GenerateExecutionChain(pin, code, indentLevel);
            }
        }
    }
}

void BlueprintEditor::GenerateEventSproutScript(std::shared_ptr<Node> eventNode, std::ostringstream& script) const {
    script << "    function " << eventNode->name << "() {\n";

    // Generate Sprout Script code
    for (const auto& pin : eventNode->pins) {
        if (pin->type == PinType::Output && pin->dataType == DataType::Exec) {
            GenerateSproutExecutionChain(pin, script, 2);
        }
    }

    script << "    }\n\n";
}

void BlueprintEditor::GenerateSproutExecutionChain(std::shared_ptr<Pin> execPin, std::ostringstream& script, int indentLevel) const {
    for (uint32_t connId : execPin->connections) {
        auto connection = GetConnection(connId);
        if (!connection) continue;

        auto targetNode = GetNode(connection->inputPin->nodeId);
        if (!targetNode) continue;

        std::string indent(indentLevel * 2, ' ');

        switch (targetNode->type) {
            case NodeType::Function:
                script << indent << targetNode->name << "();\n";
                break;
            case NodeType::FlowControl:
                script << indent << "if (condition) {\n";
                // Generate branches
                script << indent << "}\n";
                break;
            default:
                break;
        }

        // Continue chain
        for (const auto& pin : targetNode->pins) {
            if (pin->type == PinType::Output && pin->dataType == DataType::Exec) {
                GenerateSproutExecutionChain(pin, script, indentLevel);
            }
        }
    }
}

std::string BlueprintEditor::GetVariableDeclaration(std::shared_ptr<Node> varNode) const {
    // Return appropriate variable declaration based on node properties
    return "float " + varNode->name; // Simplified
}

std::shared_ptr<BlueprintEditor::Connection> BlueprintEditor::GetConnection(uint32_t connectionId) const {
    auto it = connectionMap.find(connectionId);
    return (it != connectionMap.end()) ? it->second : nullptr;
}
