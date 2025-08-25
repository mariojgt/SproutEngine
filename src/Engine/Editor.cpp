#include "Editor.h"
#include "Components.h"
#include "Scripting.h"
#include "FileUtil.h"
#include "Theme.h"
#include "HUD.h"
#include "VSGraph.h"
#include <imgui.h>
#include <imnodes.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

bool Editor::init(GLFWwindow* window){
    (void)window;
    IMGUI_CHECKVERSION();
    // Enable docking so panels can be attached to screen corners
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Theme
    Theme::SetupImGuiTheme();
    imnodes::Initialize();
    return true;
}

void Editor::shutdown(GLFWwindow* window){
    (void)window;
    imnodes::Shutdown();
}

void Editor::drawDockspace(){
    // Create a full-screen dock space allowing windows to dock to all sides
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
}

static void Vec3Control(const char* label, glm::vec3& v){
    float arr[3] = {v.x, v.y, v.z};
    if(ImGui::DragFloat3(label, arr, 0.1f)) { v = {arr[0],arr[1],arr[2]}; }
}

void Editor::drawPanels(entt::registry& reg, Renderer& renderer, Scripting& scripting, bool& playMode){
    (void)renderer;
    // Menu bar
    if(ImGui::BeginMainMenuBar()){
        if(ImGui::BeginMenu("File")){
            if(ImGui::MenuItem("New Cube")){
                auto e = reg.create();
                reg.emplace<Transform>(e);
                reg.emplace<Tag>(e, Tag{"Cube"});
                reg.emplace<MeshCube>(e);
                selected = e;
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Play")){
            if(ImGui::MenuItem(playMode ? "Stop" : "Play")) {
                playMode = !playMode;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // Hierarchy
    ImGui::Begin("Hierarchy");
    auto v = reg.view<Tag>();
    for(auto e : v){
        auto &t = v.get<Tag>(e);
        if(ImGui::Selectable(t.name.c_str(), selected == e)) selected = e;
    }
    ImGui::End();

    // Inspector
    ImGui::Begin("Inspector");
    if(selected != entt::null && reg.valid(selected)){
        if(auto* tag = reg.try_get<Tag>(selected)){
            char buf[128]; std::snprintf(buf, sizeof(buf), "%s", tag->name.c_str());
            if(ImGui::InputText("Name", buf, sizeof(buf))) tag->name = buf;
        }
        if(auto* tr = reg.try_get<Transform>(selected)){
            Vec3Control("Position", tr->position);
            Vec3Control("Rotation (deg)", tr->rotationEuler);
            Vec3Control("Scale", tr->scale);
        }
        if(auto* sc = reg.try_get<Script>(selected)){
            char pathBuf[256]; std::snprintf(pathBuf, sizeof(pathBuf), "%s", sc->filePath.c_str());
            if(ImGui::InputText("Script Path", pathBuf, sizeof(pathBuf))) sc->filePath = pathBuf;
            if(ImGui::Button("Load Script")){
                if(!sc->filePath.empty()) scripting.loadScript(reg, selected, sc->filePath);
            }
#ifdef SP_TOOLCHAIN_ENABLED
            if(ImGui::Button("Edit Script")){
                if(!sc->filePath.empty()) spEditor.open(sc->filePath);
            }
#endif
        } else {
            if(ImGui::Button("Add Script Component")){
                reg.emplace<Script>(selected, Script{std::string("assets/scripts/Rotate.lua"), 0.0, false});
            }
        }
        if(!reg.any_of<MeshCube>(selected)){
            if(ImGui::Button("Add Cube Mesh")) reg.emplace<MeshCube>(selected);
        }
        if(!reg.any_of<HUDComponent>(selected)){
            if(ImGui::Button("Add HUD Component")) reg.emplace<HUDComponent>(selected);
        }
    }
    ImGui::End();

    // UI Canvas Preview
    ImGui::Begin("UI Canvas");
    static UI::HUDRenderer hud;
    UI::HUDState state;
    if(selected != entt::null && reg.valid(selected)){
        if(auto* hudc = reg.try_get<HUDComponent>(selected)){
            state.health = hudc->x;
            state.mana = hudc->y;
            state.score = hudc->width;
            state.title = hudc->text;
            ImGui::SliderFloat("X", &hudc->x, 0.0f, 1000.0f);
            ImGui::SliderFloat("Y", &hudc->y, 0.0f, 1000.0f);
            ImGui::InputInt("Width", &hudc->width);
            char titleBuf[128]; snprintf(titleBuf, sizeof(titleBuf), "%s", hudc->text.c_str());
            if(ImGui::InputText("Text", titleBuf, sizeof(titleBuf))) hudc->text = titleBuf;
        }
    }
    hud.draw(state);
    ImGui::End();

    // Node Graph (toy)
    ImGui::Begin("Visual Script (stub)");
    imnodes::BeginNodeEditor();
    imnodes::BeginNode(1);
    ImGui::Text("Event Tick");
    imnodes::BeginOutputAttribute(2);
    ImGui::Text("Exec");
    imnodes::EndOutputAttribute();
    imnodes::EndNode();
    imnodes::BeginNode(3);
    ImGui::Text("Rotate Y");
    imnodes::BeginInputAttribute(4); ImGui::Text("Exec"); imnodes::EndInputAttribute();
    imnodes::BeginInputAttribute(5); ImGui::Text("Speed"); imnodes::EndInputAttribute();
    imnodes::BeginOutputAttribute(6); ImGui::Text("Exec"); imnodes::EndOutputAttribute();
    imnodes::EndNode();
    imnodes::EndNodeEditor();
    ImGui::End();

    // Premade Nodes (Generate Lua)
    ImGui::Begin("Premade Nodes");
    if(ImGui::Button("Rotate On Tick")) {
        auto path = VSGraph::Generate(SE_ASSETS_DIR, VSGraph::Premade::RotateOnTick);
        if(selected != entt::null && reg.valid(selected)){
            auto* sc = reg.try_get<Script>(selected);
            if(!sc) sc = &reg.emplace<Script>(selected);
            sc->filePath = path;
        }
    }
    if(ImGui::Button("Print Hello On Start")) {
        auto path = VSGraph::Generate(SE_ASSETS_DIR, VSGraph::Premade::PrintHelloOnStart);
        if(selected != entt::null && reg.valid(selected)){
            auto* sc = reg.try_get<Script>(selected);
            if(!sc) sc = &reg.emplace<Script>(selected);
            sc->filePath = path;
        }
    }
    ImGui::End();
#ifdef SP_TOOLCHAIN_ENABLED
    spEditor.draw();
#endif
}
