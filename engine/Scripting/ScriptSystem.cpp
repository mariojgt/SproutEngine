#include "ScriptSystem.h"
#include "../ECS/Components.h"
#include <sol/sol.hpp>
#include <sys/stat.h>
#include <unordered_map>
#include <iostream>

using namespace Sprout::Scripting;

static double file_mtime(const std::string& path)
{
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
#ifdef _WIN32
        return (double)st.st_mtime;
#else
        return (double)st.st_mtime;
#endif
    }
    return 0.0;
}

// One global Lua state for now
static sol::state& lua_state()
{
    static sol::state L;
    static bool init = false;
    if (!init) {
        L.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string);
        init = true;
    }
    return L;
}

struct ScriptRuntime {
    sol::environment env;
    bool started = false;
};

static std::unordered_map<entt::entity, ScriptRuntime> g_scripts;

static void bind_api(sol::state& L, sol::environment& env)
{
    // Bind Transform struct
    env.set_function("get_position", [&env]() {
        auto t = env["__transform"].get<Sprout::ECS::Transform*>();
        return std::make_tuple(t->position.x, t->position.y, t->position.z);
    });
    env.set_function("set_position", [&env](float x, float y, float z) {
        auto t = env["__transform"].get<Sprout::ECS::Transform*>();
        t->position = {x,y,z};
    });
    env.set_function("rotate_y_degrees", [&env](float deg) {
        auto t = env["__transform"].get<Sprout::ECS::Transform*>();
        t->rotation = glm::angleAxis(glm::radians(deg), glm::vec3(0,1,0)) * t->rotation;
    });
}

void Sprout::Scripting::RunScripts(entt::registry& reg, float dt)
{
    auto& L = lua_state();

    auto view = reg.view<Script, Sprout::ECS::Transform>();
    for (auto e : view) {
        auto& sc = view.get<Script>(e);
        auto& tr = view.get<Sprout::ECS::Transform>(e);

        double mtime = file_mtime(sc.path);
        bool needLoad = !sc.loaded || (mtime > 0.0 && mtime > sc.lastWriteTime);

        ScriptRuntime* rt = nullptr;
        if (g_scripts.find(e) == g_scripts.end()) {
            g_scripts[e] = ScriptRuntime{ sol::environment(L, sol::create, L.globals()), false };
        }
        rt = &g_scripts[e];

        if (needLoad) {
            sc.lastWriteTime = mtime;
            sc.loaded = false;
            rt->env = sol::environment(L, sol::create, L.globals());
            bind_api(L, rt->env);
            rt->env["__transform"] = &tr;

            sol::load_result chunk = L.load_file(sc.path);
            if (!chunk.valid()) {
                sol::error err = chunk;
                std::cerr << "[Lua] load error: " << err.what() << "\\n";
                continue;
            }
            sol::protected_function pf = chunk;
            sol::protected_function_result res = pf(rt->env, rt->env);
            if (!res.valid()) {
                sol::error err = res;
                std::cerr << "[Lua] runtime error: " << err.what() << "\\n";
                continue;
            }
            sc.loaded = true;
            rt->started = false;
        }

        if (sc.loaded) {
            if (!rt->started) {
                sol::function onStart = rt->env["OnStart"];
                if (onStart.valid()) {
                    auto r = onStart();
                    (void)r;
                }
                rt->started = true;
            }
            sol::function onUpdate = rt->env["OnUpdate"];
            if (onUpdate.valid()) {
                auto r = onUpdate(dt);
                (void)r;
            }
        }
    }
}
