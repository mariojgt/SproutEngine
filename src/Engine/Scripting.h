#pragma once
#include <sol/sol.hpp>
#include <glm/glm.hpp>
#include <string>
#include <optional>
#include <entt/entt.hpp>

class Scripting {
public:
    bool init();
    void shutdown();

    void attach(entt::registry& reg);
    void update(entt::registry& reg, float dt);

    bool loadScript(entt::registry& reg, entt::entity e, const std::string& path);

private:
    sol::state lua;
};
