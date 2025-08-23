#include "Actor.h"
#include "World.h"
#include "Transform.h"
#include <random>
#include <chrono>
#include <algorithm>

static std::mt19937_64 s_rng(std::chrono::steady_clock::now().time_since_epoch().count());

Actor::Actor(World* world, const std::string& name)
    : world(world), name(name), actorId(GenerateActorID()) {
    if (world) {
        entity = world->GetRegistry().create();
        // Ensure every actor has a transform
        world->GetRegistry().emplace<Transform>(entity);
    }
}

Actor::~Actor() {
    // Clean up hierarchy
    DetachFromActor();

    // Detach all children
    auto childrenCopy = children; // Copy to avoid iterator invalidation
    for (Actor* child : childrenCopy) {
        child->DetachFromActor();
    }

    // Clean up components (they'll be destroyed automatically by unique_ptr)
    components.clear();
}

glm::vec3 Actor::GetActorLocation() const {
    if (!IsValid()) return glm::vec3(0.0f);

    auto& transform = world->GetRegistry().get<Transform>(entity);
    return transform.position;
}

void Actor::SetActorLocation(const glm::vec3& location) {
    if (!IsValid()) return;

    auto& transform = world->GetRegistry().get<Transform>(entity);
    transform.position = location;
}

glm::vec3 Actor::GetActorRotation() const {
    if (!IsValid()) return glm::vec3(0.0f);

    auto& transform = world->GetRegistry().get<Transform>(entity);
    return transform.rotationEuler;
}

void Actor::SetActorRotation(const glm::vec3& rotation) {
    if (!IsValid()) return;

    auto& transform = world->GetRegistry().get<Transform>(entity);
    transform.rotationEuler = rotation;
}

glm::vec3 Actor::GetActorScale() const {
    if (!IsValid()) return glm::vec3(1.0f);

    auto& transform = world->GetRegistry().get<Transform>(entity);
    return transform.scale;
}

void Actor::SetActorScale(const glm::vec3& scale) {
    if (!IsValid()) return;

    auto& transform = world->GetRegistry().get<Transform>(entity);
    transform.scale = scale;
}

glm::vec3 Actor::GetForwardVector() const {
    glm::vec3 rotation = GetActorRotation();

    // Convert Euler angles to forward vector
    float yaw = glm::radians(rotation.y);
    float pitch = glm::radians(rotation.x);

    return glm::vec3(
        cos(pitch) * cos(yaw),
        sin(pitch),
        cos(pitch) * sin(yaw)
    );
}

glm::vec3 Actor::GetRightVector() const {
    glm::vec3 forward = GetForwardVector();
    glm::vec3 up(0, 1, 0);
    return glm::normalize(glm::cross(forward, up));
}

glm::vec3 Actor::GetUpVector() const {
    glm::vec3 forward = GetForwardVector();
    glm::vec3 right = GetRightVector();
    return glm::normalize(glm::cross(right, forward));
}

void Actor::AttachToActor(Actor* parent) {
    if (!parent || parent == this) return;

    // Detach from current parent
    DetachFromActor();

    // Set new parent
    this->parent = parent;
    parent->AddChild(this);
}

void Actor::DetachFromActor() {
    if (parent) {
        parent->RemoveChild(this);
        parent = nullptr;
    }
}

void Actor::SetBlueprintClass(const std::string& blueprintPath) {
    blueprintClass = blueprintPath;
}

void Actor::AddChild(Actor* child) {
    if (child && std::find(children.begin(), children.end(), child) == children.end()) {
        children.push_back(child);
    }
}

void Actor::RemoveChild(Actor* child) {
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        children.erase(it);
    }
}

ActorID Actor::GenerateActorID() {
    return s_rng();
}

// SceneComponent Implementation

glm::vec3 SceneComponent::GetWorldLocation() const {
    if (!attachParent) {
        return relativeLocation;
    }

    // TODO: Implement proper transform hierarchy calculation
    glm::vec3 parentWorld = attachParent->GetWorldLocation();
    return parentWorld + relativeLocation;
}

glm::vec3 SceneComponent::GetWorldRotation() const {
    if (!attachParent) {
        return relativeRotation;
    }

    glm::vec3 parentWorld = attachParent->GetWorldRotation();
    return parentWorld + relativeRotation;
}

glm::vec3 SceneComponent::GetWorldScale() const {
    if (!attachParent) {
        return relativeScale;
    }

    glm::vec3 parentWorld = attachParent->GetWorldScale();
    return parentWorld * relativeScale;
}

glm::mat4 SceneComponent::GetWorldTransform() const {
    glm::mat4 transform(1.0f);

    glm::vec3 worldPos = GetWorldLocation();
    glm::vec3 worldRot = GetWorldRotation();
    glm::vec3 worldScale = GetWorldScale();

    // TRS composition
    transform = glm::translate(transform, worldPos);
    transform = glm::rotate(transform, glm::radians(worldRot.x), glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, glm::radians(worldRot.y), glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, glm::radians(worldRot.z), glm::vec3(0, 0, 1));
    transform = glm::scale(transform, worldScale);

    return transform;
}

void SceneComponent::AttachToComponent(SceneComponent* parent) {
    if (!parent || parent == this) return;

    DetachFromComponent();

    attachParent = parent;
    parent->AddAttachChild(this);
}

void SceneComponent::DetachFromComponent() {
    if (attachParent) {
        attachParent->RemoveAttachChild(this);
        attachParent = nullptr;
    }
}

void SceneComponent::AddAttachChild(SceneComponent* child) {
    if (child && std::find(attachChildren.begin(), attachChildren.end(), child) == attachChildren.end()) {
        attachChildren.push_back(child);
    }
}

void SceneComponent::RemoveAttachChild(SceneComponent* child) {
    auto it = std::find(attachChildren.begin(), attachChildren.end(), child);
    if (it != attachChildren.end()) {
        attachChildren.erase(it);
    }
}
