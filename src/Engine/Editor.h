#pragma once
#include <entt/entt.hpp>
#include <string>
#ifdef SP_TOOLCHAIN_ENABLED
#include "widgets/SpCodeEditor.h"
#endif

struct GLFWwindow;
class Renderer;
class Scripting;

class Editor {
public:
    bool init(GLFWwindow* window); // sets up ImGui + docking
    void shutdown(GLFWwindow* window);

    void drawDockspace();
    void drawPanels(entt::registry& reg, Renderer& renderer, Scripting& scripting, bool& playMode);

    entt::entity selected{entt::null};
#ifdef SP_TOOLCHAIN_ENABLED
    SpCodeEditor spEditor;
#endif
};
