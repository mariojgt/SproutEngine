#pragma once
#include <string>
#include <glm/glm.hpp>

struct Transform {
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f}; // for gizmo compatibility
    glm::vec3 rotationEuler{0.0f}; // degrees
    glm::vec3 scale{1.0f};
};

struct NameComponent {
    std::string name{"Entity"};
};

struct Tag { std::string name{"Entity"}; };

struct MeshCube {
    bool enabled = true; // Dummy member to avoid zero-size struct issues
};

struct MeshSphere {
    bool enabled = true; // Dummy member to avoid zero-size struct issues
};

struct Script {
    std::string filePath;        // e.g. assets/scripts/Rotate.lua
    double      lastUpdateTime{0.0}; // hot-reload tracking
    bool needsUpdate{false};
};


struct HUDComponent {
    float x{100.0f};
    float y{100.0f};
    int width{200};
    std::string text{"HUD Text"};
};
