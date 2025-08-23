#pragma once
#include <entt/entt.hpp>
#include <string>
#include <memory>
#include <vector>

// Forward declarations
class Actor;
class World;

/**
 * Scene class - represents a game level/scene
 * Now uses the new Actor/World system while maintaining compatibility
 */
class Scene {
public:
    Scene(const std::string& name = "Scene");
    ~Scene();

    // Legacy ECS support (for backward compatibility)
    entt::registry registry;
    entt::entity createEntity(const std::string& name);

    // New Actor-based system
    World* GetWorld() const { return world.get(); }
    
    // Actor management (delegates to World)
    template<typename ActorType = Actor, typename... Args>
    ActorType* SpawnActor(const std::string& name = "", Args&&... args);
    
    void DestroyActor(Actor* actor);
    Actor* FindActorByName(const std::string& name);
    
    // Scene lifecycle
    void BeginPlay();
    void EndPlay();
    void Tick(float deltaTime);
    
    // Scene properties
    const std::string& GetName() const { return sceneName; }
    void SetName(const std::string& name) { sceneName = name; }
    
    // Serialization
    void SaveScene(const std::string& filePath) const;
    bool LoadScene(const std::string& filePath);

private:
    std::string sceneName;
    std::unique_ptr<World> world;
};

// Template implementation
template<typename ActorType, typename... Args>
ActorType* Scene::SpawnActor(const std::string& name, Args&&... args) {
    return world->SpawnActor<ActorType>(name, std::forward<Args>(args)...);
}
