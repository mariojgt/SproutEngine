#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <typeindex>
#include <functional>

// Forward declarations
class World;
class Component;
class ActorComponent;

// UUID system for stable references
using ActorID = uint64_t;

/**
 * Core Actor class - similar to Unreal's AActor
 * Actors are the primary objects in the world that can have components attached
 */
class Actor {
public:
    Actor(World* world, const std::string& name = "Actor");
    virtual ~Actor();

    // Core properties
    ActorID GetActorID() const { return actorId; }
    const std::string& GetName() const { return name; }
    void SetName(const std::string& newName) { name = newName; }

    World* GetWorld() const { return world; }
    entt::entity GetEntity() const { return entity; }

    // Transform operations
    glm::vec3 GetActorLocation() const;
    void SetActorLocation(const glm::vec3& location);
    glm::vec3 GetActorRotation() const;
    void SetActorRotation(const glm::vec3& rotation);
    glm::vec3 GetActorScale() const;
    void SetActorScale(const glm::vec3& scale);

    // Transform helpers
    glm::vec3 GetForwardVector() const;
    glm::vec3 GetRightVector() const;
    glm::vec3 GetUpVector() const;

    // Hierarchy
    void AttachToActor(Actor* parent);
    void DetachFromActor();
    Actor* GetAttachParent() const { return parent; }
    const std::vector<Actor*>& GetAttachedActors() const { return children; }

    // Component system
    template<typename T, typename... Args>
    T* CreateComponent(Args&&... args);

    template<typename T>
    T* GetComponent() const;

    template<typename T>
    bool HasComponent() const;

    template<typename T>
    void RemoveComponent();

    // Lifecycle events (virtual for blueprint/script overrides)
    virtual void BeginPlay() {}
    virtual void EndPlay() {}
    virtual void Tick(float deltaTime) {}
    virtual void Destroyed() {}

    // Event system
    template<typename EventType>
    void BindEvent(std::function<void(const EventType&)> callback);

    template<typename EventType>
    void TriggerEvent(const EventType& event);

    // Blueprint/Script integration
    void SetBlueprintClass(const std::string& blueprintPath);
    const std::string& GetBlueprintClass() const { return blueprintClass; }

    // Serialization support
    virtual void Serialize(class JsonWriter& writer) const;
    virtual void Deserialize(const class JsonReader& reader);

    // Utility
    bool IsValid() const { return world != nullptr && entity != entt::null; }
    void MarkForDestroy() { pendingDestroy = true; }
    bool IsPendingDestroy() const { return pendingDestroy; }

    // Static class info (for blueprint system)
    static std::string StaticClass() { return "Actor"; }

protected:
    World* world;
    entt::entity entity;
    ActorID actorId;
    std::string name;
    std::string blueprintClass;

    // Hierarchy
    Actor* parent = nullptr;
    std::vector<Actor*> children;

    // Component management
    std::unordered_map<std::type_index, std::unique_ptr<ActorComponent>> components;

    // Event bindings
    std::unordered_map<std::type_index, std::vector<std::function<void(const void*)>>> eventBindings;

    // State
    bool pendingDestroy = false;
    bool hasBegunPlay = false;

    // Internal methods
    void AddChild(Actor* child);
    void RemoveChild(Actor* child);
    static ActorID GenerateActorID();

    friend class World;
};

/**
 * Base class for all actor components - similar to Unreal's UActorComponent
 */
class ActorComponent {
public:
    ActorComponent(Actor* owner) : owner(owner) {}
    virtual ~ActorComponent() = default;

    Actor* GetOwner() const { return owner; }
    World* GetWorld() const { return owner ? owner->GetWorld() : nullptr; }

    // Lifecycle
    virtual void BeginPlay() {}
    virtual void EndPlay() {}
    virtual void TickComponent(float deltaTime) {}

    // Serialization
    virtual void Serialize(class JsonWriter& writer) const {}
    virtual void Deserialize(const class JsonReader& reader) {}

    // Component properties
    bool IsTickEnabled() const { return bCanTick; }
    void SetTickEnabled(bool enabled) { bCanTick = enabled; }

protected:
    Actor* owner;
    bool bCanTick = false;
};

/**
 * Scene component - for components that have a transform
 */
class SceneComponent : public ActorComponent {
public:
    SceneComponent(Actor* owner) : ActorComponent(owner) {}
    virtual ~SceneComponent() = default;

    // Local transform
    glm::vec3 GetRelativeLocation() const { return relativeLocation; }
    void SetRelativeLocation(const glm::vec3& location) { relativeLocation = location; }

    glm::vec3 GetRelativeRotation() const { return relativeRotation; }
    void SetRelativeRotation(const glm::vec3& rotation) { relativeRotation = rotation; }

    glm::vec3 GetRelativeScale() const { return relativeScale; }
    void SetRelativeScale(const glm::vec3& scale) { relativeScale = scale; }

    // World transform (calculated)
    glm::vec3 GetWorldLocation() const;
    glm::vec3 GetWorldRotation() const;
    glm::vec3 GetWorldScale() const;
    glm::mat4 GetWorldTransform() const;

    // Attachment
    void AttachToComponent(SceneComponent* parent);
    void DetachFromComponent();
    SceneComponent* GetAttachParent() const { return attachParent; }
    const std::vector<SceneComponent*>& GetAttachChildren() const { return attachChildren; }

protected:
    // Local transform
    glm::vec3 relativeLocation{0.0f};
    glm::vec3 relativeRotation{0.0f}; // Euler angles in degrees
    glm::vec3 relativeScale{1.0f};

    // Attachment hierarchy
    SceneComponent* attachParent = nullptr;
    std::vector<SceneComponent*> attachChildren;

    void AddAttachChild(SceneComponent* child);
    void RemoveAttachChild(SceneComponent* child);
};

// Template implementations
template<typename T, typename... Args>
T* Actor::CreateComponent(Args&&... args) {
    static_assert(std::is_base_of_v<ActorComponent, T>, "T must derive from ActorComponent");

    auto component = std::make_unique<T>(this, std::forward<Args>(args)...);
    T* ptr = component.get();

    std::type_index typeIndex(typeid(T));
    components[typeIndex] = std::move(component);

    return ptr;
}

template<typename T>
T* Actor::GetComponent() const {
    static_assert(std::is_base_of_v<ActorComponent, T>, "T must derive from ActorComponent");

    std::type_index typeIndex(typeid(T));
    auto it = components.find(typeIndex);
    if (it != components.end()) {
        return static_cast<T*>(it->second.get());
    }
    return nullptr;
}

template<typename T>
bool Actor::HasComponent() const {
    std::type_index typeIndex(typeid(T));
    return components.find(typeIndex) != components.end();
}

template<typename T>
void Actor::RemoveComponent() {
    std::type_index typeIndex(typeid(T));
    components.erase(typeIndex);
}

template<typename EventType>
void Actor::BindEvent(std::function<void(const EventType&)> callback) {
    std::type_index typeIndex(typeid(EventType));
    eventBindings[typeIndex].push_back([callback](const void* eventPtr) {
        callback(*static_cast<const EventType*>(eventPtr));
    });
}

template<typename EventType>
void Actor::TriggerEvent(const EventType& event) {
    std::type_index typeIndex(typeid(EventType));
    auto it = eventBindings.find(typeIndex);
    if (it != eventBindings.end()) {
        for (auto& binding : it->second) {
            binding(&event);
        }
    }
}
