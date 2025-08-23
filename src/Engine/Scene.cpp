#include "Scene.h"
#include "Components.h"
// Temporarily comment out World system until compilation issues are resolved
// #include "World.h"
// #include "Actor.h"

Scene::Scene(const std::string& name) : sceneName(name) {
    // world = std::make_unique<World>(name);
}

Scene::~Scene() = default;

entt::entity Scene::createEntity(const std::string& name) {
    auto entity = registry.create();
    registry.emplace<NameComponent>(entity, NameComponent{name});
    registry.emplace<Transform>(entity);
    return entity;
}
