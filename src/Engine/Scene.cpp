#include "Scene.h"
#include "Components.h"

entt::entity Scene::createEntity(const std::string& name) {
    #include "Scene.h"
#include "Components.h"
#include "World.h"
#include "Actor.h"

Scene::Scene(const std::string& name) : sceneName(name) {
    world = std::make_unique<World>(name);
}

Scene::~Scene() = default;

entt::entity Scene::createEntity(const std::string& name) {
    auto entity = registry.create();
    registry.emplace<Tag>(entity, Tag{name});
    registry.emplace<Transform>(entity);
    return entity;
}

void Scene::DestroyActor(Actor* actor) {
    if (world) {
        world->DestroyActor(actor);
    }
}

Actor* Scene::FindActorByName(const std::string& name) {
    if (world) {
        return world->FindActorByName(name);
    }
    return nullptr;
}

void Scene::BeginPlay() {
    if (world) {
        world->BeginPlay();
    }
}

void Scene::EndPlay() {
    if (world) {
        world->EndPlay();
    }
}

void Scene::Tick(float deltaTime) {
    if (world) {
        world->Tick(deltaTime);
    }
}

void Scene::SaveScene(const std::string& filePath) const {
    if (world) {
        world->SaveWorld(filePath);
    }
}

bool Scene::LoadScene(const std::string& filePath) {
    if (world) {
        return world->LoadWorld(filePath);
    }
    return false;
}
}
