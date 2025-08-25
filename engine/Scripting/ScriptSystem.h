#pragma once
#include <entt/entt.hpp>
#include <string>

namespace sol { class state; }

namespace Sprout::Scripting
{
    struct Script {
        std::string path;   // path to a Lua file
        bool loaded = false;
        double lastWriteTime = 0.0; // simple hot-reload via polling mtime
    };

    void RunScripts(entt::registry& reg, float dt);
}
