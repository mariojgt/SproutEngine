#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <glm/glm.hpp>

// Forward declarations
class Actor;
class World;

/**
 * Base class for all events in the engine
 */
class Event {
public:
    virtual ~Event() = default;
    virtual std::string GetEventName() const = 0;
};

/**
 * Common game events
 */
class BeginPlayEvent : public Event {
public:
    std::string GetEventName() const override { return "BeginPlay"; }
};

class EndPlayEvent : public Event {
public:
    std::string GetEventName() const override { return "EndPlay"; }
};

class TickEvent : public Event {
public:
    float deltaTime;
    TickEvent(float dt) : deltaTime(dt) {}
    std::string GetEventName() const override { return "Tick"; }
};

class CollisionEvent : public Event {
public:
    Actor* otherActor;
    glm::vec3 hitLocation;
    glm::vec3 hitNormal;

    CollisionEvent(Actor* other, const glm::vec3& location, const glm::vec3& normal)
        : otherActor(other), hitLocation(location), hitNormal(normal) {}
    std::string GetEventName() const override { return "Collision"; }
};

class OverlapEvent : public Event {
public:
    Actor* otherActor;
    bool bBeginOverlap; // true for begin, false for end

    OverlapEvent(Actor* other, bool begin) : otherActor(other), bBeginOverlap(begin) {}
    std::string GetEventName() const override { return "Overlap"; }
};

/**
 * Input events
 */
class InputEvent : public Event {
public:
    std::string inputName;
    float value;

    InputEvent(const std::string& name, float val) : inputName(name), value(val) {}
    std::string GetEventName() const override { return "Input"; }
};

/**
 * Event Dispatcher - manages event binding and triggering
 */
class EventDispatcher {
public:
    template<typename EventType>
    void Subscribe(std::function<void(const EventType&)> callback);

    template<typename EventType>
    void Trigger(const EventType& event);

    void Clear();

private:
    std::unordered_map<std::type_index, std::vector<std::function<void(const Event&)>>> eventBindings;
};

/**
 * Blueprint node base class
 */
class BlueprintNode {
public:
    virtual ~BlueprintNode() = default;
    virtual void Execute() = 0;
    virtual std::string GetNodeType() const = 0;

    // Node connections
    std::vector<BlueprintNode*> outputNodes;
    std::vector<BlueprintNode*> inputNodes;

    void ConnectToNode(BlueprintNode* targetNode);
    void DisconnectFromNode(BlueprintNode* targetNode);

protected:
    void TriggerOutputNodes();
};

/**
 * Blueprint event node - triggered by events
 */
class BlueprintEventNode : public BlueprintNode {
public:
    BlueprintEventNode(const std::string& eventName);
    void Execute() override;
    std::string GetNodeType() const override { return "Event"; }

    const std::string& GetEventName() const { return eventName; }

private:
    std::string eventName;
};

/**
 * Blueprint function node - calls a function
 */
class BlueprintFunctionNode : public BlueprintNode {
public:
    BlueprintFunctionNode(const std::string& functionName, std::function<void()> func);
    void Execute() override;
    std::string GetNodeType() const override { return "Function"; }

private:
    std::string functionName;
    std::function<void()> function;
};

/**
 * Blueprint variable node - gets/sets variables
 */
class BlueprintVariableNode : public BlueprintNode {
public:
    enum class VariableOperation {
        Get,
        Set
    };

    BlueprintVariableNode(const std::string& varName, VariableOperation op);
    void Execute() override;
    std::string GetNodeType() const override { return "Variable"; }

    void SetValue(const std::string& value) { variableValue = value; }
    const std::string& GetValue() const { return variableValue; }

private:
    std::string variableName;
    std::string variableValue;
    VariableOperation operation;
};

/**
 * Blueprint graph - manages a collection of connected nodes
 */
class BlueprintGraph {
public:
    BlueprintGraph(Actor* owner);
    ~BlueprintGraph();

    // Node management
    BlueprintNode* AddNode(std::unique_ptr<BlueprintNode> node);
    void RemoveNode(BlueprintNode* node);
    void ConnectNodes(BlueprintNode* from, BlueprintNode* to);
    void DisconnectNodes(BlueprintNode* from, BlueprintNode* to);

    // Execution
    void TriggerEvent(const std::string& eventName);
    void Execute();

    // Serialization
    void SaveToFile(const std::string& filePath) const;
    bool LoadFromFile(const std::string& filePath);

    Actor* GetOwner() const { return owner; }

private:
    Actor* owner;
    std::vector<std::unique_ptr<BlueprintNode>> nodes;
    std::unordered_map<std::string, std::vector<BlueprintEventNode*>> eventNodes;
};

/**
 * Blueprint class - represents a blueprint that can be instantiated
 */
class BlueprintClass {
public:
    BlueprintClass(const std::string& className);

    // Create an instance of this blueprint
    Actor* CreateInstance(World* world) const;

    // Blueprint properties
    void AddProperty(const std::string& name, const std::string& type, const std::string& defaultValue);
    void AddFunction(const std::string& name, std::function<void(Actor*)> func);

    // Component defaults
    void AddDefaultComponent(const std::string& componentType);

    const std::string& GetClassName() const { return className; }

private:
    std::string className;
    std::vector<std::string> defaultComponents;
    std::unordered_map<std::string, std::string> properties; // name -> type
    std::unordered_map<std::string, std::string> defaultValues; // name -> value
    std::unordered_map<std::string, std::function<void(Actor*)>> functions;
};

/**
 * Blueprint Manager - manages all blueprints in the engine
 */
class BlueprintManager {
public:
    static BlueprintManager& Get();

    // Blueprint registration
    void RegisterBlueprint(const std::string& name, std::unique_ptr<BlueprintClass> blueprint);
    BlueprintClass* GetBlueprint(const std::string& name) const;

    // Load blueprints from files
    bool LoadBlueprintFromFile(const std::string& filePath);
    void LoadAllBlueprints(const std::string& directory);

    // Create blueprint instances
    Actor* CreateBlueprintInstance(const std::string& blueprintName, World* world) const;

private:
    BlueprintManager() = default;
    std::unordered_map<std::string, std::unique_ptr<BlueprintClass>> blueprints;
};

// Template implementations
template<typename EventType>
void EventDispatcher::Subscribe(std::function<void(const EventType&)> callback) {
    std::type_index typeIndex(typeid(EventType));
    eventBindings[typeIndex].push_back([callback](const Event& event) {
        const EventType& typedEvent = static_cast<const EventType&>(event);
        callback(typedEvent);
    });
}

template<typename EventType>
void EventDispatcher::Trigger(const EventType& event) {
    std::type_index typeIndex(typeid(EventType));
    auto it = eventBindings.find(typeIndex);
    if (it != eventBindings.end()) {
        for (auto& binding : it->second) {
            binding(event);
        }
    }
}
