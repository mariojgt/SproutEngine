#include "CoreComponents.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// MeshRendererComponent Implementation
MeshRendererComponent::MeshRendererComponent(Actor* owner)
    : SceneComponent(owner) {
    bCanTick = false;
}

void MeshRendererComponent::SetMesh(const std::string& meshPath) {
    this->meshPath = meshPath;
    // TODO: Load mesh resource
}

void MeshRendererComponent::SetMaterial(const std::string& materialPath) {
    this->materialPath = materialPath;
    // TODO: Load material resource
}

void MeshRendererComponent::Serialize(class JsonWriter& writer) const {
    // TODO: Implement serialization
}

void MeshRendererComponent::Deserialize(const class JsonReader& reader) {
    // TODO: Implement deserialization
}

// CameraComponent Implementation
CameraComponent::CameraComponent(Actor* owner)
    : SceneComponent(owner) {
    bCanTick = false;
}

glm::mat4 CameraComponent::GetProjectionMatrix() const {
    if (projectionType == ProjectionType::Perspective) {
        return glm::perspective(glm::radians(fieldOfView), aspectRatio, nearClipPlane, farClipPlane);
    } else {
        float left = -orthographicSize * aspectRatio * 0.5f;
        float right = orthographicSize * aspectRatio * 0.5f;
        float bottom = -orthographicSize * 0.5f;
        float top = orthographicSize * 0.5f;
        return glm::ortho(left, right, bottom, top, nearClipPlane, farClipPlane);
    }
}

glm::mat4 CameraComponent::GetViewMatrix() const {
    glm::vec3 worldPos = GetWorldLocation();
    glm::vec3 worldRot = GetWorldRotation();

    // Calculate forward, right, up vectors
    glm::quat rotation = glm::quat(glm::radians(worldRot));
    glm::vec3 forward = rotation * glm::vec3(0, 0, -1); // Forward is -Z in camera space
    glm::vec3 up = rotation * glm::vec3(0, 1, 0);

    return glm::lookAt(worldPos, worldPos + forward, up);
}

glm::mat4 CameraComponent::GetViewProjectionMatrix() const {
    return GetProjectionMatrix() * GetViewMatrix();
}

void CameraComponent::Serialize(class JsonWriter& writer) const {
    // TODO: Implement serialization
}

void CameraComponent::Deserialize(const class JsonReader& reader) {
    // TODO: Implement deserialization
}

// LightComponent Implementation
LightComponent::LightComponent(Actor* owner, LightType type)
    : SceneComponent(owner), lightType(type) {
    bCanTick = false;
}

void LightComponent::Serialize(class JsonWriter& writer) const {
    // TODO: Implement serialization
}

void LightComponent::Deserialize(const class JsonReader& reader) {
    // TODO: Implement deserialization
}

// AudioComponent Implementation
AudioComponent::AudioComponent(Actor* owner)
    : SceneComponent(owner) {
    bCanTick = false;
}

void AudioComponent::Play() {
    // TODO: Implement audio playback
    bIsPlaying = true;
    std::cout << "Playing audio: " << audioClipPath << std::endl;
}

void AudioComponent::Stop() {
    // TODO: Implement audio stop
    bIsPlaying = false;
    std::cout << "Stopping audio: " << audioClipPath << std::endl;
}

void AudioComponent::Pause() {
    // TODO: Implement audio pause
    bIsPlaying = false;
    std::cout << "Pausing audio: " << audioClipPath << std::endl;
}

void AudioComponent::Serialize(class JsonWriter& writer) const {
    // TODO: Implement serialization
}

void AudioComponent::Deserialize(const class JsonReader& reader) {
    // TODO: Implement deserialization
}

// CollisionComponent Implementation
CollisionComponent::CollisionComponent(Actor* owner, CollisionType type)
    : SceneComponent(owner), collisionType(type) {
    bCanTick = false;
}

void CollisionComponent::Serialize(class JsonWriter& writer) const {
    // TODO: Implement serialization
}

void CollisionComponent::Deserialize(const class JsonReader& reader) {
    // TODO: Implement deserialization
}
