#include "Blueprint.h"
#include "Actor.h"
#include "World.h"
#include <iostream>
#include <algorithm>

// BlueprintNode Implementation
void BlueprintNode::ConnectToNode(BlueprintNode* targetNode) {
    if (targetNode && std::find(outputNodes.begin(), outputNodes.end(), targetNode) == outputNodes.end()) {
        outputNodes.push_back(targetNode);
        targetNode->inputNodes.push_back(this);
    }
}

void BlueprintNode::DisconnectFromNode(BlueprintNode* targetNode) {
    auto it = std::find(outputNodes.begin(), outputNodes.end(), targetNode);
    if (it != outputNodes.end()) {
        outputNodes.erase(it);

        auto inputIt = std::find(targetNode->inputNodes.begin(), targetNode->inputNodes.end(), this);
        if (inputIt != targetNode->inputNodes.end()) {
            targetNode->inputNodes.erase(inputIt);
        }
    }
}

void BlueprintNode::TriggerOutputNodes() {
    for (BlueprintNode* node : outputNodes) {
        node->Execute();
    }
}

// BlueprintEventNode Implementation
BlueprintEventNode::BlueprintEventNode(const std::string& eventName)
    : eventName(eventName) {
}

void BlueprintEventNode::Execute() {
    std::cout << "Event triggered: " << eventName << std::endl;
    TriggerOutputNodes();
}

// BlueprintFunctionNode Implementation
BlueprintFunctionNode::BlueprintFunctionNode(const std::string& functionName, std::function<void()> func)
    : functionName(functionName), function(func) {
}

void BlueprintFunctionNode::Execute() {
    std::cout << "Executing function: " << functionName << std::endl;
    if (function) {
        function();
    }
    TriggerOutputNodes();
}

// BlueprintVariableNode Implementation
BlueprintVariableNode::BlueprintVariableNode(const std::string& varName, VariableOperation op)
    : variableName(varName), operation(op) {
}

void BlueprintVariableNode::Execute() {
    if (operation == VariableOperation::Get) {
        std::cout << "Getting variable: " << variableName << " = " << variableValue << std::endl;
    } else {
        std::cout << "Setting variable: " << variableName << " = " << variableValue << std::endl;
    }
    TriggerOutputNodes();
}

// BlueprintGraph Implementation
BlueprintGraph::BlueprintGraph(Actor* owner) : owner(owner) {
}

BlueprintGraph::~BlueprintGraph() {
    nodes.clear();
}

BlueprintNode* BlueprintGraph::AddNode(std::unique_ptr<BlueprintNode> node) {
    BlueprintNode* ptr = node.get();

    // If it's an event node, register it
    if (auto* eventNode = dynamic_cast<BlueprintEventNode*>(ptr)) {
        eventNodes[eventNode->GetEventName()].push_back(eventNode);
    }

    nodes.push_back(std::move(node));
    return ptr;
}

void BlueprintGraph::RemoveNode(BlueprintNode* node) {
    // Remove from event nodes if it's an event node
    if (auto* eventNode = dynamic_cast<BlueprintEventNode*>(node)) {
        auto& eventNodeList = eventNodes[eventNode->GetEventName()];
        auto it = std::find(eventNodeList.begin(), eventNodeList.end(), eventNode);
        if (it != eventNodeList.end()) {
            eventNodeList.erase(it);
        }
    }

    // Disconnect all connections
    for (auto* inputNode : node->inputNodes) {
        inputNode->DisconnectFromNode(node);
    }
    for (auto* outputNode : node->outputNodes) {
        node->DisconnectFromNode(outputNode);
    }

    // Remove from nodes list
    auto it = std::find_if(nodes.begin(), nodes.end(),
        [node](const std::unique_ptr<BlueprintNode>& ptr) {
            return ptr.get() == node;
        });

    if (it != nodes.end()) {
        nodes.erase(it);
    }
}

void BlueprintGraph::ConnectNodes(BlueprintNode* from, BlueprintNode* to) {
    if (from && to) {
        from->ConnectToNode(to);
    }
}

void BlueprintGraph::DisconnectNodes(BlueprintNode* from, BlueprintNode* to) {
    if (from && to) {
        from->DisconnectFromNode(to);
    }
}

void BlueprintGraph::TriggerEvent(const std::string& eventName) {
    auto it = eventNodes.find(eventName);
    if (it != eventNodes.end()) {
        for (BlueprintEventNode* eventNode : it->second) {
            eventNode->Execute();
        }
    }
}

void BlueprintGraph::Execute() {
    // Execute all event nodes (this is a simple implementation)
    for (const auto& [eventName, nodeList] : eventNodes) {
        for (BlueprintEventNode* node : nodeList) {
            node->Execute();
        }
    }
}

void BlueprintGraph::SaveToFile(const std::string& filePath) const {
    // TODO: Implement serialization
    std::cout << "Saving blueprint graph to: " << filePath << std::endl;
}

bool BlueprintGraph::LoadFromFile(const std::string& filePath) {
    // TODO: Implement deserialization
    std::cout << "Loading blueprint graph from: " << filePath << std::endl;
    return true;
}

// BlueprintClass Implementation
BlueprintClass::BlueprintClass(const std::string& className) : className(className) {
}

Actor* BlueprintClass::CreateInstance(World* world) const {
    if (!world) return nullptr;

    Actor* actor = world->SpawnActor<Actor>(className);

    // Add default components
    for (const std::string& componentType : defaultComponents) {
        // TODO: Create components based on type string
        std::cout << "Adding component: " << componentType << " to " << className << std::endl;
    }

    // Set default property values
    for (const auto& [name, value] : defaultValues) {
        std::cout << "Setting property " << name << " = " << value << std::endl;
    }

    // Bind functions
    for (const auto& [funcName, func] : functions) {
        func(actor);
    }

    return actor;
}

void BlueprintClass::AddProperty(const std::string& name, const std::string& type, const std::string& defaultValue) {
    properties[name] = type;
    defaultValues[name] = defaultValue;
}

void BlueprintClass::AddFunction(const std::string& name, std::function<void(Actor*)> func) {
    functions[name] = func;
}

void BlueprintClass::AddDefaultComponent(const std::string& componentType) {
    defaultComponents.push_back(componentType);
}

// BlueprintManager Implementation
BlueprintManager& BlueprintManager::Get() {
    static BlueprintManager instance;
    return instance;
}

void BlueprintManager::RegisterBlueprint(const std::string& name, std::unique_ptr<BlueprintClass> blueprint) {
    blueprints[name] = std::move(blueprint);
}

BlueprintClass* BlueprintManager::GetBlueprint(const std::string& name) const {
    auto it = blueprints.find(name);
    return (it != blueprints.end()) ? it->second.get() : nullptr;
}

bool BlueprintManager::LoadBlueprintFromFile(const std::string& filePath) {
    // TODO: Implement file loading
    std::cout << "Loading blueprint from file: " << filePath << std::endl;
    return true;
}

void BlueprintManager::LoadAllBlueprints(const std::string& directory) {
    // TODO: Implement directory scanning and loading
    std::cout << "Loading all blueprints from directory: " << directory << std::endl;
}

Actor* BlueprintManager::CreateBlueprintInstance(const std::string& blueprintName, World* world) const {
    BlueprintClass* blueprint = GetBlueprint(blueprintName);
    if (blueprint) {
        return blueprint->CreateInstance(world);
    }
    return nullptr;
}

// EventDispatcher Implementation
void EventDispatcher::Clear() {
    eventBindings.clear();
}
