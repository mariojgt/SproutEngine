#pragma once
#include <entt/entt.hpp>
#include <string>

namespace Sprout {
    class Renderer;
}

namespace Sprout::ECS {
    struct Transform;
    struct Camera;
    struct Mesh;
}

namespace Sprout::Assets {
    struct LoadedMesh;
}

namespace Sprout::SceneSys
{
    class Scene {
    public:
        entt::entity createEntity(const std::string& name = "");

        entt::registry& registry() { return m_reg; }
        const entt::registry& registry() const { return m_reg; }

        // Utilities
        entt::entity createCameraPrimary(float fovYDeg, float zNear, float zFar);
        entt::entity createMeshFromFile(const std::string& path);

    private:
        entt::registry m_reg;
    };
}
