#include "Scripting.h"
#include "FileUtil.h"
#include "Components.h"
#include <iostream>

static glm::vec3 GetRot(entt::registry& R, entt::entity e){ return R.get<Transform>(e).rotationEuler; }
static void SetRot(entt::registry& R, entt::entity e, const glm::vec3& v){ R.get<Transform>(e).rotationEuler = v; }

bool Scripting::init(){
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string);
    return true;
}

void Scripting::shutdown(){ }

void Scripting::attach(entt::registry& reg){
    lua["Print"] = [](const std::string& s){ std::cout << s << std::endl; };

    // Basic accessors. Note: we pass tuples/arrays to/from Lua.
    lua["GetRotation"] = [&reg](uint32_t id){
        entt::entity e = (entt::entity)id;
        auto r = GetRot(reg, e);
        return std::make_tuple(r.x, r.y, r.z);
    };
    lua["SetRotation"] = [&reg](uint32_t id, sol::table t){
        float x = t.get_or(1, 0.0f);
        float y = t.get_or(2, 0.0f);
        float z = t.get_or(3, 0.0f);
        SetRot(reg, (entt::entity)id, glm::vec3{x,y,z});
    };
}

bool Scripting::loadScript(entt::registry& reg, entt::entity e, const std::string& path){
    auto& sc = reg.get<Script>(e);
    sc.filePath = path;
    sc.lastUpdateTime = GetFileWriteTime(path);
    auto src = ReadTextFile(path);
    if(!src){ std::cerr<<"Failed to read script: "<<path<<"\n"; return false; }

    sol::load_result chunk = lua.load(*src);
    if(!chunk.valid()){
        sol::error err = chunk;
        std::cerr << "Lua load error: " << err.what() << std::endl;
        return false;
    }
    sol::protected_function script = chunk;
    auto res = script();
    if(!res.valid()){
        sol::error err = res;
        std::cerr << "Lua exec error: " << err.what() << std::endl;
        return false;
    }

    sc.needsUpdate = true;

    // Call OnStart if present
    sol::object onStartObj = lua["OnStart"];
    if(onStartObj.is<sol::protected_function>()) {
        sol::protected_function onStart = onStartObj.as<sol::protected_function>();
        if(onStart.valid()) onStart((uint32_t)e);
    }
    return true;
}

void Scripting::update(entt::registry& reg, float dt){
    auto view = reg.view<Script>();
    for(auto e : view){
        auto &sc = view.get<Script>(e);
        // Hot reload
        double wt = GetFileWriteTime(sc.filePath);
        if(!sc.filePath.empty() && wt > sc.lastUpdateTime){
            std::cout << "Hot reload: " << sc.filePath << std::endl;
            loadScript(reg, e, sc.filePath);
        }
        // Tick
        sol::object onTickObj = lua["OnTick"];
        if(onTickObj.is<sol::protected_function>()) {
            sol::protected_function onTick = onTickObj.as<sol::protected_function>();
            if(onTick.valid()) onTick((uint32_t)e, dt);
        }
    }
}
