#pragma once
#include <string>
#include <glm/glm.hpp>

struct Transform {
    glm::vec3 position{0.0f};
    glm::vec3 rotationEuler{0.0f}; // degrees
    glm::vec3 scale{1.0f};
};

struct Tag { std::string name{"Entity"}; };

struct MeshCube { /* marker for a unit cube */ };

struct Script {
    std::string path;            // e.g. assets/scripts/Rotate.lua
    double      lastWriteTime{}; // hot-reload tracking
    bool loaded{false};
};


struct HUDComponent {
    float health{100.0f};
    float mana{100.0f};
    int score{0};
    std::string title{"Game HUD"};
};
