#pragma once
#define GLFW_INCLUDE_NONE
#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
struct ImDrawData;

namespace TinyImGui {
    bool Init(GLFWwindow* window);
    void Shutdown();
    void NewFrame();
    void RenderDrawData(ImDrawData* draw_data);

    // DPI handling functions
    void RefreshDPIScale();
    float GetContentScale();
}
