#pragma once
#include <entt/entt.hpp>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <typeindex>

class Actor;
using ActorID = uint64_t;

/**
 * World class - manages the game world, actors, and global systems
 * Similar to Unreal's UWorld
 */
class World {
public:
    World(const std::string& name = "World");
    ~World();

    // World management
    const std::string& GetName() const { return worldName; }
    void SetName(const std::string& name) { worldName = name; }

    // Actor management
    template<typename ActorType = Actor, typename... Args>
    ActorType* SpawnActor(const std::string& name = "", Args&&... args);
    
    void DestroyActor(Actor* actor);
    void DestroyActor(ActorID actorId);
    
    Actor* FindActor(ActorID actorId) const;
    Actor* FindActorByName(const std::string& name) const;
    
    template<typename ActorType>
    std::vector<ActorType*> FindActorsOfClass() const;
    
    const std::vector<std::unique_ptr<Actor>>& GetAllActors() const { return actors; }

    // ECS Registry access
    entt::registry& GetRegistry() { return registry; }
    const entt::registry& GetRegistry() const { return registry; }

    // World tick/update
    void Tick(float deltaTime);
    void BeginPlay();
    void EndPlay();

    // Level streaming (for future implementation)
    void LoadSubLevel(const std::string& levelPath);
    void UnloadSubLevel(const std::string& levelPath);

    // Event system
    template<typename EventType>
    void BroadcastEvent(const EventType& event);
    
    template<typename EventType>
    void RegisterGlobalEventHandler(std::function<void(const EventType&)> handler);

    // Serialization
    void SaveWorld(const std::string& filePath) const;
    bool LoadWorld(const std::string& filePath);

    // Utility
    void CleanupDestroyedActors();
    size_t GetActorCount() const { return actors.size(); }

private:
    std::string worldName;
    entt::registry registry;
    std::vector<std::unique_ptr<Actor>> actors;
    std::unordered_map<ActorID, Actor*> actorMap;
    
    // Global event handlers
    std::unordered_map<std::type_index, std::vector<std::function<void(const void*)>>> globalEventHandlers;
    
    // State
    bool hasBegunPlay = false;
    std::vector<Actor*> pendingDestroyActors;

    // Internal helpers
    void RegisterActor(std::unique_ptr<Actor> actor);
    void UnregisterActor(ActorID actorId);
};

// Template implementations
template<typename ActorType, typename... Args>
ActorType* World::SpawnActor(const std::string& name, Args&&... args) {
    static_assert(std::is_base_of_v<Actor, ActorType>, "ActorType must derive from Actor");
    
    std::string actorName = name.empty() ? ActorType::StaticClass() : name;
    auto actor = std::make_unique<ActorType>(this, actorName, std::forward<Args>(args)...);
    ActorType* ptr = actor.get();
    
    RegisterActor(std::move(actor));
    
    // If world has already begun play, begin play for this actor
    if (hasBegunPlay) {
        ptr->BeginPlay();
    }
    
    return ptr;
}

template<typename ActorType>
std::vector<ActorType*> World::FindActorsOfClass() const {
    static_assert(std::is_base_of_v<Actor, ActorType>, "ActorType must derive from Actor");
    
    std::vector<ActorType*> result;
    for (const auto& actor : actors) {
        if (auto* typedActor = dynamic_cast<ActorType*>(actor.get())) {
            result.push_back(typedActor);
        }
    }
    return result;
}

template<typename EventType>
void World::BroadcastEvent(const EventType& event) {
    // Send to global handlers
    std::type_index typeIndex(typeid(EventType));
    auto it = globalEventHandlers.find(typeIndex);
    if (it != globalEventHandlers.end()) {
        for (auto& handler : it->second) {
            handler(&event);
        }
    }
    
    // Send to all actors
    for (const auto& actor : actors) {
        actor->TriggerEvent(event);
    }
}

template<typename EventType>
void World::RegisterGlobalEventHandler(std::function<void(const EventType&)> handler) {
    std::type_index typeIndex(typeid(EventType));
    globalEventHandlers[typeIndex].push_back([handler](const void* eventPtr) {
        handler(*static_cast<const EventType*>(eventPtr));
    });
}
