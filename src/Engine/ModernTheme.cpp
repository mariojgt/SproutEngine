#include "ModernTheme.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <algorithm>

namespace ModernTheme {

void ApplyDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Main colors
    colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.96f, 0.98f, 1.00f); // Gray100
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // Gray500
    colors[ImGuiCol_WindowBg]               = ImVec4(0.12f, 0.16f, 0.22f, 1.00f); // Gray900
    colors[ImGuiCol_ChildBg]                = ImVec4(0.07f, 0.10f, 0.15f, 1.00f); // Gray950
    colors[ImGuiCol_PopupBg]                = ImVec4(0.19f, 0.25f, 0.32f, 1.00f); // Gray800
    colors[ImGuiCol_Border]                 = ImVec4(0.29f, 0.33f, 0.38f, 1.00f); // Gray700
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.19f, 0.25f, 0.32f, 1.00f); // Gray800
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.29f, 0.33f, 0.38f, 1.00f); // Gray700
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.23f, 0.51f, 0.96f, 1.00f); // Blue600
    colors[ImGuiCol_TitleBg]                = ImVec4(0.07f, 0.10f, 0.15f, 1.00f); // Gray950
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.07f, 0.10f, 0.15f, 1.00f); // Gray950
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.07f, 0.10f, 0.15f, 1.00f); // Gray950
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.07f, 0.10f, 0.15f, 1.00f); // Gray950
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.07f, 0.10f, 0.15f, 1.00f); // Gray950
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.29f, 0.33f, 0.38f, 1.00f); // Gray700
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.38f, 0.43f, 0.49f, 1.00f); // Gray600
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.23f, 0.51f, 0.96f, 1.00f); // Blue600
    colors[ImGuiCol_CheckMark]              = ImVec4(0.23f, 0.51f, 0.96f, 1.00f); // Blue600
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.23f, 0.51f, 0.96f, 1.00f); // Blue600
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.11f, 0.31f, 0.85f, 1.00f); // Blue700
    colors[ImGuiCol_Button]                 = ImVec4(0.23f, 0.51f, 0.96f, 1.00f); // Blue600
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.37f, 0.60f, 0.98f, 1.00f); // Blue500
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.11f, 0.31f, 0.85f, 1.00f); // Blue700
    colors[ImGuiCol_Header]                 = ImVec4(0.23f, 0.51f, 0.96f, 0.31f); // Blue600 alpha
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.23f, 0.51f, 0.96f, 0.80f); // Blue600 alpha
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.23f, 0.51f, 0.96f, 1.00f); // Blue600
    colors[ImGuiCol_Separator]              = ImVec4(0.29f, 0.33f, 0.38f, 1.00f); // Gray700
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.38f, 0.43f, 0.49f, 1.00f); // Gray600
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.23f, 0.51f, 0.96f, 1.00f); // Blue600
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.23f, 0.51f, 0.96f, 0.20f); // Blue600 alpha
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.23f, 0.51f, 0.96f, 0.67f); // Blue600 alpha
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.23f, 0.51f, 0.96f, 0.95f); // Blue600 alpha
    colors[ImGuiCol_Tab]                    = ImVec4(0.19f, 0.25f, 0.32f, 1.00f); // Gray800
    colors[ImGuiCol_TabHovered]             = ImVec4(0.23f, 0.51f, 0.96f, 0.80f); // Blue600 alpha
    colors[ImGuiCol_TabActive]              = ImVec4(0.23f, 0.51f, 0.96f, 1.00f); // Blue600
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.19f, 0.25f, 0.32f, 1.00f); // Gray800
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.29f, 0.33f, 0.38f, 1.00f); // Gray700
    colors[ImGuiCol_PlotLines]              = ImVec4(0.23f, 0.51f, 0.96f, 1.00f); // Blue600
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.37f, 0.60f, 0.98f, 1.00f); // Blue500
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.23f, 0.51f, 0.96f, 1.00f); // Blue600
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(0.37f, 0.60f, 0.98f, 1.00f); // Blue500
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.25f, 0.32f, 1.00f); // Gray800
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.29f, 0.33f, 0.38f, 1.00f); // Gray700
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.26f, 0.31f, 1.00f); // Gray750
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.23f, 0.51f, 0.96f, 0.35f); // Blue600 alpha
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.23f, 0.51f, 0.96f, 1.00f); // Blue600
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.23f, 0.51f, 0.96f, 1.00f); // Blue600
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    // Reasonable scaling for 4K displays
    float scale = 1.3f; // Much more moderate scaling

    // Properly sized UI elements for 4K visibility
    style.WindowPadding     = ImVec2(12 * scale, 12 * scale);
    style.FramePadding      = ImVec2(8 * scale, 6 * scale);
    style.CellPadding       = ImVec2(6 * scale, 4 * scale);
    style.ItemSpacing       = ImVec2(8 * scale, 4 * scale);
    style.ItemInnerSpacing  = ImVec2(4 * scale, 4 * scale);
    style.TouchExtraPadding = ImVec2(0, 0);
    style.IndentSpacing     = 21 * scale;
    style.ScrollbarSize     = 14 * scale;
    style.GrabMinSize       = 10 * scale;

    // Moderate rounded corners
    style.WindowRounding    = 8.0f * scale;
    style.ChildRounding     = 6.0f * scale;
    style.FrameRounding     = 6.0f * scale;
    style.PopupRounding     = 6.0f * scale;
    style.ScrollbarRounding = 9.0f * scale;
    style.GrabRounding      = 6.0f * scale;
    style.LogSliderDeadzone = 4.0f * scale;
    style.TabRounding       = 6.0f * scale;

    // Normal borders
    style.WindowBorderSize  = 1.0f * scale;
    style.ChildBorderSize   = 1.0f * scale;
    style.PopupBorderSize   = 1.0f * scale;
    style.FrameBorderSize   = 0.0f;
    style.TabBorderSize     = 0.0f;

    // Other settings
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
    style.SeparatorTextBorderSize = 3.0f * scale;
    style.SeparatorTextAlign = ImVec2(0.0f, 0.5f);
    style.SeparatorTextPadding = ImVec2(20.0f * scale, 3.0f * scale);
}

bool ModernButton(const char* label, ImVec2 size, ImU32 color) {
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(
        ((color >> IM_COL32_R_SHIFT) & 0xFF) + 20,
        ((color >> IM_COL32_G_SHIFT) & 0xFF) + 20,
        ((color >> IM_COL32_B_SHIFT) & 0xFF) + 20,
        ((color >> IM_COL32_A_SHIFT) & 0xFF)
    ));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(
        std::max(0, (int)((color >> IM_COL32_R_SHIFT) & 0xFF) - 20),
        std::max(0, (int)((color >> IM_COL32_G_SHIFT) & 0xFF) - 20),
        std::max(0, (int)((color >> IM_COL32_B_SHIFT) & 0xFF) - 20),
        ((color >> IM_COL32_A_SHIFT) & 0xFF)
    ));

    bool clicked = ImGui::Button(label, size);

    ImGui::PopStyleColor(3);
    return clicked;
}

bool ModernButtonSuccess(const char* label, ImVec2 size) {
    return ModernButton(label, size, Colors::Green600);
}

bool ModernButtonDanger(const char* label, ImVec2 size) {
    return ModernButton(label, size, Colors::Red600);
}

bool ModernButtonSecondary(const char* label, ImVec2 size) {
    return ModernButton(label, size, Colors::Gray600);
}

bool ModernMenuItem(const char* label, const char* shortcut, bool selected) {
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Colors::Blue600);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6)); // Normal spacing

    bool clicked = ImGui::MenuItem(label, shortcut, selected);

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    return clicked;
}

bool ModernSelectable(const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size) {
    if (selected) {
        ImGui::PushStyleColor(ImGuiCol_Header, Colors::Blue600);
    }

    bool clicked = ImGui::Selectable(label, selected, flags, size);

    if (selected) {
        ImGui::PopStyleColor();
    }

    return clicked;
}

void ModernSeparator() {
    ImGui::PushStyleColor(ImGuiCol_Separator, Colors::Gray700);
    ImGui::Separator();
    ImGui::PopStyleColor();
}

void ModernSpacing(float height) {
    ImGui::Dummy(ImVec2(0, height));
}

bool BeginModernWindow(const char* name, bool* p_open, ImGuiWindowFlags flags) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);

    return ImGui::Begin(name, p_open, flags);
}

void EndModernWindow() {
    ImGui::End();
    ImGui::PopStyleVar(2);
}

bool BeginModernMenuBar() {
    // Reasonable frame padding for 4K displays
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 8)); // Moderate padding
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, Colors::Gray950);
    bool result = ImGui::BeginMainMenuBar();
    if (!result) {
        // If BeginMainMenuBar failed, we need to pop the styles we pushed
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }
    return result;
}

void EndModernMenuBar() {
    ImGui::EndMainMenuBar();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

bool BeginModernMenu(const char* label) {
    ImGui::PushStyleColor(ImGuiCol_PopupBg, Colors::Gray800);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 8.0f); // Normal rounding
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6)); // Normal spacing

    bool result = ImGui::BeginMenu(label);
    if (!result) {
        // If BeginMenu failed, we need to pop the styles we pushed
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }
    return result;
}

void EndModernMenu() {
    ImGui::EndMenu();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

void ModernText(const char* text, ImU32 color) {
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::Text("%s", text);
    ImGui::PopStyleColor();
}

void ModernTextSecondary(const char* text) {
    ModernText(text, Colors::Gray400);
}

void ModernTextMuted(const char* text) {
    ModernText(text, Colors::Gray500);
}

void ModernHeader(const char* text) {
    ImGui::PushFont(nullptr); // You can add a bold font here later
    ModernText(text, Colors::Gray100);
    ImGui::PopFont();
    ModernSpacing(4);
}

} // namespace ModernTheme
