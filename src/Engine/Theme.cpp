#include "Theme.h"
#include <imgui.h>

namespace Theme {
void SetupImGuiTheme(){
    ImGuiStyle& s = ImGui::GetStyle();
    s.FrameRounding = 8.0f;
    s.GrabRounding = 8.0f;
    s.WindowRounding = 8.0f;
    s.ScrollbarRounding = 8.0f;
    s.TabRounding = 6.0f;
    s.FrameBorderSize = 0.0f;
    s.WindowBorderSize = 0.0f;
    s.GrabMinSize = 10.0f;
    s.ItemSpacing = ImVec2(10,8);
    s.WindowPadding = ImVec2(14,12);
    s.IndentSpacing = 16.0f;

    ImVec4 accent = ImVec4(0.40f, 0.70f, 1.00f, 1.00f);
    ImVec4 bg     = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);
    ImVec4 bg2    = ImVec4(0.14f, 0.15f, 0.18f, 1.00f);
    ImVec4 fg     = ImVec4(0.90f, 0.92f, 0.95f, 1.00f);
    ImVec4 mut    = ImVec4(0.60f, 0.64f, 0.70f, 1.00f);

    ImGui::StyleColorsDark();
    ImVec4* c = s.Colors;
    c[ImGuiCol_Text] = fg;
    c[ImGuiCol_TextDisabled] = mut;
    c[ImGuiCol_WindowBg] = bg;
    c[ImGuiCol_ChildBg] = bg;
    c[ImGuiCol_PopupBg] = bg2;
    c[ImGuiCol_Border] = bg2;
    c[ImGuiCol_FrameBg] = bg2;
    c[ImGuiCol_FrameBgHovered] = ImVec4(accent.x, accent.y, accent.z, 0.35f);
    c[ImGuiCol_FrameBgActive] = ImVec4(accent.x, accent.y, accent.z, 0.50f);
    c[ImGuiCol_TitleBg] = bg2;
    c[ImGuiCol_TitleBgActive] = bg2;
    c[ImGuiCol_MenuBarBg] = bg2;
    c[ImGuiCol_ScrollbarBg] = bg2;
    c[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.28f, 0.34f, 1.0f);
    c[ImGuiCol_CheckMark] = accent;
    c[ImGuiCol_SliderGrab] = accent;
    c[ImGuiCol_SliderGrabActive] = accent;
    c[ImGuiCol_Button] = ImVec4(0.20f, 0.22f, 0.26f, 1.0f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.34f, 0.45f, 1.0f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.34f, 0.45f, 0.60f, 1.0f);
    c[ImGuiCol_Header] = ImVec4(0.20f, 0.22f, 0.26f, 1.0f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.34f, 0.45f, 1.0f);
    c[ImGuiCol_HeaderActive] = ImVec4(0.34f, 0.45f, 0.60f, 1.0f);
    c[ImGuiCol_Separator] = ImVec4(0.25f,0.28f,0.34f,1.0f);
    c[ImGuiCol_ResizeGrip] = ImVec4(0.25f,0.28f,0.34f,1.0f);
    c[ImGuiCol_ResizeGripHovered] = ImVec4(0.28f, 0.34f, 0.45f, 1.0f);
    c[ImGuiCol_ResizeGripActive] = ImVec4(0.34f, 0.45f, 0.60f, 1.0f);
    c[ImGuiCol_Tab] = ImVec4(0.20f, 0.22f, 0.26f, 1.0f);
    c[ImGuiCol_TabHovered] = ImVec4(0.34f, 0.45f, 0.60f, 1.0f);
    c[ImGuiCol_TabActive] = ImVec4(0.28f,0.34f,0.45f,1.0f);
}
} // namespace Theme
