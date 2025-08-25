#pragma once
#include <glm/glm.hpp>
#include <entt/entt.hpp>

namespace Sprout { class Renderer; }

namespace Sprout::ECS
{
    void UpdateCameras(entt::registry& reg, int viewportWidth, int viewportHeight);
    void Render(entt::registry& reg, Sprout::Renderer& renderer);
}
