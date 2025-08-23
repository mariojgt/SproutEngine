#pragma once
#include "Actor.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

// Forward declarations
class Material;
class Mesh;

/**
 * Mesh Renderer Component - renders 3D meshes
 */
class MeshRendererComponent : public SceneComponent {
public:
    MeshRendererComponent(Actor* owner);
    virtual ~MeshRendererComponent() = default;

    // Mesh and Material
    void SetMesh(const std::string& meshPath);
    void SetMaterial(const std::string& materialPath);
    
    const std::string& GetMeshPath() const { return meshPath; }
    const std::string& GetMaterialPath() const { return materialPath; }
    
    // Rendering properties
    bool GetCastShadows() const { return bCastShadows; }
    void SetCastShadows(bool cast) { bCastShadows = cast; }
    
    bool GetReceiveShadows() const { return bReceiveShadows; }
    void SetReceiveShadows(bool receive) { bReceiveShadows = receive; }
    
    // Visibility
    bool IsVisible() const { return bVisible; }
    void SetVisible(bool visible) { bVisible = visible; }

    // Serialization
    void Serialize(class JsonWriter& writer) const override;
    void Deserialize(const class JsonReader& reader) override;

private:
    std::string meshPath;
    std::string materialPath;
    bool bCastShadows = true;
    bool bReceiveShadows = true;
    bool bVisible = true;
};

/**
 * Camera Component - provides camera functionality
 */
class CameraComponent : public SceneComponent {
public:
    enum class ProjectionType {
        Perspective,
        Orthographic
    };

    CameraComponent(Actor* owner);
    virtual ~CameraComponent() = default;

    // Projection settings
    void SetProjectionType(ProjectionType type) { projectionType = type; }
    ProjectionType GetProjectionType() const { return projectionType; }
    
    void SetFieldOfView(float fov) { fieldOfView = fov; }
    float GetFieldOfView() const { return fieldOfView; }
    
    void SetNearClipPlane(float nearPlane) { nearClipPlane = nearPlane; }
    float GetNearClipPlane() const { return nearClipPlane; }
    
    void SetFarClipPlane(float farPlane) { farClipPlane = farPlane; }
    float GetFarClipPlane() const { return farClipPlane; }
    
    void SetAspectRatio(float aspect) { aspectRatio = aspect; }
    float GetAspectRatio() const { return aspectRatio; }

    // Orthographic settings
    void SetOrthographicSize(float size) { orthographicSize = size; }
    float GetOrthographicSize() const { return orthographicSize; }

    // Matrix calculations
    glm::mat4 GetProjectionMatrix() const;
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetViewProjectionMatrix() const;

    // Camera properties
    void SetPrimary(bool primary) { bPrimaryCamera = primary; }
    bool IsPrimary() const { return bPrimaryCamera; }

    // Serialization
    void Serialize(class JsonWriter& writer) const override;
    void Deserialize(const class JsonReader& reader) override;

private:
    ProjectionType projectionType = ProjectionType::Perspective;
    float fieldOfView = 60.0f; // degrees
    float nearClipPlane = 0.1f;
    float farClipPlane = 1000.0f;
    float aspectRatio = 16.0f / 9.0f;
    float orthographicSize = 10.0f;
    bool bPrimaryCamera = false;
};

/**
 * Light Component - base class for lights
 */
class LightComponent : public SceneComponent {
public:
    enum class LightType {
        Directional,
        Point,
        Spot
    };

    LightComponent(Actor* owner, LightType type = LightType::Point);
    virtual ~LightComponent() = default;

    // Light properties
    void SetLightType(LightType type) { lightType = type; }
    LightType GetLightType() const { return lightType; }
    
    void SetColor(const glm::vec3& color) { lightColor = color; }
    const glm::vec3& GetColor() const { return lightColor; }
    
    void SetIntensity(float intensity) { lightIntensity = intensity; }
    float GetIntensity() const { return lightIntensity; }
    
    void SetRange(float range) { lightRange = range; }
    float GetRange() const { return lightRange; }
    
    // Spot light properties
    void SetInnerConeAngle(float angle) { innerConeAngle = angle; }
    float GetInnerConeAngle() const { return innerConeAngle; }
    
    void SetOuterConeAngle(float angle) { outerConeAngle = angle; }
    float GetOuterConeAngle() const { return outerConeAngle; }
    
    // Shadow casting
    void SetCastShadows(bool cast) { bCastShadows = cast; }
    bool GetCastShadows() const { return bCastShadows; }

    // Serialization
    void Serialize(class JsonWriter& writer) const override;
    void Deserialize(const class JsonReader& reader) override;

private:
    LightType lightType;
    glm::vec3 lightColor{1.0f, 1.0f, 1.0f};
    float lightIntensity = 1.0f;
    float lightRange = 10.0f;
    float innerConeAngle = 30.0f; // degrees
    float outerConeAngle = 45.0f; // degrees
    bool bCastShadows = true;
};

/**
 * Audio Component - plays 3D positioned audio
 */
class AudioComponent : public SceneComponent {
public:
    AudioComponent(Actor* owner);
    virtual ~AudioComponent() = default;

    // Audio clip
    void SetAudioClip(const std::string& clipPath) { audioClipPath = clipPath; }
    const std::string& GetAudioClip() const { return audioClipPath; }
    
    // Playback control
    void Play();
    void Stop();
    void Pause();
    bool IsPlaying() const { return bIsPlaying; }
    
    // Audio properties
    void SetVolume(float volume) { audioVolume = glm::clamp(volume, 0.0f, 1.0f); }
    float GetVolume() const { return audioVolume; }
    
    void SetPitch(float pitch) { audioPitch = pitch; }
    float GetPitch() const { return audioPitch; }
    
    void SetLoop(bool loop) { bLoop = loop; }
    bool GetLoop() const { return bLoop; }
    
    // 3D Audio properties
    void SetMinDistance(float distance) { minDistance = distance; }
    float GetMinDistance() const { return minDistance; }
    
    void SetMaxDistance(float distance) { maxDistance = distance; }
    float GetMaxDistance() const { return maxDistance; }

    // Serialization
    void Serialize(class JsonWriter& writer) const override;
    void Deserialize(const class JsonReader& reader) override;

private:
    std::string audioClipPath;
    float audioVolume = 1.0f;
    float audioPitch = 1.0f;
    bool bLoop = false;
    bool bIsPlaying = false;
    float minDistance = 1.0f;
    float maxDistance = 100.0f;
};

/**
 * Collision Component - base class for colliders
 */
class CollisionComponent : public SceneComponent {
public:
    enum class CollisionType {
        Box,
        Sphere,
        Capsule,
        Mesh
    };

    CollisionComponent(Actor* owner, CollisionType type = CollisionType::Box);
    virtual ~CollisionComponent() = default;

    // Collision properties
    void SetCollisionType(CollisionType type) { collisionType = type; }
    CollisionType GetCollisionType() const { return collisionType; }
    
    void SetIsTrigger(bool trigger) { bIsTrigger = trigger; }
    bool IsTrigger() const { return bIsTrigger; }
    
    // Box collider
    void SetBoxExtent(const glm::vec3& extent) { boxExtent = extent; }
    const glm::vec3& GetBoxExtent() const { return boxExtent; }
    
    // Sphere collider
    void SetSphereRadius(float radius) { sphereRadius = radius; }
    float GetSphereRadius() const { return sphereRadius; }
    
    // Capsule collider
    void SetCapsuleRadius(float radius) { capsuleRadius = radius; }
    float GetCapsuleRadius() const { return capsuleRadius; }
    
    void SetCapsuleHeight(float height) { capsuleHeight = height; }
    float GetCapsuleHeight() const { return capsuleHeight; }

    // Collision events (to be connected to blueprint/script system)
    std::function<void(Actor*)> OnBeginOverlap;
    std::function<void(Actor*)> OnEndOverlap;
    std::function<void(Actor*)> OnHit;

    // Serialization
    void Serialize(class JsonWriter& writer) const override;
    void Deserialize(const class JsonReader& reader) override;

private:
    CollisionType collisionType;
    bool bIsTrigger = false;
    
    // Collision shapes
    glm::vec3 boxExtent{1.0f, 1.0f, 1.0f};
    float sphereRadius = 1.0f;
    float capsuleRadius = 0.5f;
    float capsuleHeight = 2.0f;
};
