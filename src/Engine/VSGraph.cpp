#include "VSGraph.h"
#include <fstream>
#include <string>

namespace VSGraph {

static void WriteFile(const std::string& path, const std::string& s){
    std::ofstream ofs(path, std::ios::binary);
    ofs << s;
}

std::string Generate(const std::string& assetsDir, Premade type){
    std::string out = assetsDir + "/scripts/Generated_FromGraph.lua";
    std::string lua;

    switch(type){
        case Premade::RotateOnTick:
            lua = R"(speed = 90.0
function OnStart(id) Print("Rotate premade ready") end
function OnTick(id, dt)
  local x,y,z = GetRotation(id)
  y = y + speed * dt
  SetRotation(id, {x,y,z})
end)";
            break;
        case Premade::PrintHelloOnStart:
            lua = R"(function OnStart(id) Print("Hello from node graph!") end
function OnTick(id, dt) end)";
            break;
        case Premade::PulseHealthBar:
            lua = R"(t = 0.0
function OnStart(id) t = 0.0 end
function OnTick(id, dt)
  t = t + dt
  -- Example: nothing yet; hook to HUD when exposed.
end)";
            break;
    }

    WriteFile(out, lua);
    return out;
}

} // namespace VSGraph
