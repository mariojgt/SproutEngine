#include "World.h"
#include "Actor.h"
#include <algorithm>
#include <iostream>

World::World(const std::string& name) : worldName(name) {
    // Initialize the world
}

World::~World() {
    EndPlay();
    actors.clear();
    actorMap.clear();
}

void World::DestroyActor(Actor* actor) {
    if (!actor) return;
    
    actor->MarkForDestroy();
    pendingDestroyActors.push_back(actor);
}

void World::DestroyActor(ActorID actorId) {
    Actor* actor = FindActor(actorId);
    if (actor) {
        DestroyActor(actor);
    }
}

Actor* World::FindActor(ActorID actorId) const {
    auto it = actorMap.find(actorId);
    return (it != actorMap.end()) ? it->second : nullptr;
}

Actor* World::FindActorByName(const std::string& name) const {
    for (const auto& actor : actors) {
        if (actor->GetName() == name) {
            return actor.get();
        }
    }
    return nullptr;
}

void World::Tick(float deltaTime) {
    // Tick all actors
    for (const auto& actor : actors) {
        if (!actor->IsPendingDestroy()) {
            actor->Tick(deltaTime);
            
            // Tick components
            for (const auto& [typeIndex, component] : actor->components) {
                if (component->IsTickEnabled()) {
                    component->TickComponent(deltaTime);
                }
            }
        }
    }
    
    // Clean up destroyed actors
    CleanupDestroyedActors();
}

void World::BeginPlay() {
    if (hasBegunPlay) return;
    
    hasBegunPlay = true;
    
    // Begin play for all existing actors
    for (const auto& actor : actors) {
        if (!actor->hasBegunPlay) {
            actor->BeginPlay();
            actor->hasBegunPlay = true;
            
            // Begin play for components
            for (const auto& [typeIndex, component] : actor->components) {
                component->BeginPlay();
            }
        }
    }
}

void World::EndPlay() {
    if (!hasBegunPlay) return;
    
    // End play for all actors
    for (const auto& actor : actors) {
        if (actor->hasBegunPlay) {
            // End play for components first
            for (const auto& [typeIndex, component] : actor->components) {
                component->EndPlay();
            }
            
            actor->EndPlay();
            actor->hasBegunPlay = false;
        }
    }
    
    hasBegunPlay = false;
}

void World::LoadSubLevel(const std::string& levelPath) {
    // TODO: Implement level streaming
    std::cout << "Loading sub-level: " << levelPath << std::endl;
}

void World::UnloadSubLevel(const std::string& levelPath) {
    // TODO: Implement level streaming
    std::cout << "Unloading sub-level: " << levelPath << std::endl;
}

void World::SaveWorld(const std::string& filePath) const {
    // TODO: Implement world serialization
    std::cout << "Saving world to: " << filePath << std::endl;
}

bool World::LoadWorld(const std::string& filePath) {
    // TODO: Implement world deserialization
    std::cout << "Loading world from: " << filePath << std::endl;
    return true;
}

void World::CleanupDestroyedActors() {
    if (pendingDestroyActors.empty()) return;
    
    for (Actor* actor : pendingDestroyActors) {
        // Call destroyed event
        actor->Destroyed();
        
        // Remove from maps and vectors
        UnregisterActor(actor->GetActorID());
        
        // Remove from actors vector
        auto it = std::find_if(actors.begin(), actors.end(), 
            [actor](const std::unique_ptr<Actor>& ptr) {
                return ptr.get() == actor;
            });
        
        if (it != actors.end()) {
            actors.erase(it);
        }
    }
    
    pendingDestroyActors.clear();
}

void World::RegisterActor(std::unique_ptr<Actor> actor) {
    ActorID id = actor->GetActorID();
    Actor* ptr = actor.get();
    
    actorMap[id] = ptr;
    actors.push_back(std::move(actor));
}

void World::UnregisterActor(ActorID actorId) {
    actorMap.erase(actorId);
}
