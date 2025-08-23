#pragma once
#include <imgui.h>

namespace ModernTheme {
    
    // Modern Color Palette (Tailwind-inspired)
    namespace Colors {
        // Grays (Dark theme base)
        constexpr ImU32 Gray50  = IM_COL32(249, 250, 251, 255);
        constexpr ImU32 Gray100 = IM_COL32(243, 244, 246, 255);
        constexpr ImU32 Gray200 = IM_COL32(229, 231, 235, 255);
        constexpr ImU32 Gray300 = IM_COL32(209, 213, 219, 255);
        constexpr ImU32 Gray400 = IM_COL32(156, 163, 175, 255);
        constexpr ImU32 Gray500 = IM_COL32(107, 114, 128, 255);
        constexpr ImU32 Gray600 = IM_COL32(75, 85, 99, 255);
        constexpr ImU32 Gray700 = IM_COL32(55, 65, 81, 255);
        constexpr ImU32 Gray800 = IM_COL32(31, 41, 55, 255);
        constexpr ImU32 Gray900 = IM_COL32(17, 24, 39, 255);
        constexpr ImU32 Gray950 = IM_COL32(3, 7, 18, 255);
        
        // Accent Colors
        constexpr ImU32 Blue500  = IM_COL32(59, 130, 246, 255);
        constexpr ImU32 Blue600  = IM_COL32(37, 99, 235, 255);
        constexpr ImU32 Blue700  = IM_COL32(29, 78, 216, 255);
        
        constexpr ImU32 Green500 = IM_COL32(34, 197, 94, 255);
        constexpr ImU32 Green600 = IM_COL32(22, 163, 74, 255);
        
        constexpr ImU32 Red500   = IM_COL32(239, 68, 68, 255);
        constexpr ImU32 Red600   = IM_COL32(220, 38, 38, 255);
        
        constexpr ImU32 Orange500 = IM_COL32(249, 115, 22, 255);
        constexpr ImU32 Orange600 = IM_COL32(234, 88, 12, 255);
        
        constexpr ImU32 Purple500 = IM_COL32(168, 85, 247, 255);
        constexpr ImU32 Purple600 = IM_COL32(147, 51, 234, 255);
    }
    
    // Apply modern dark theme
    void ApplyDarkTheme();
    
    // Modern UI Components
    bool ModernButton(const char* label, ImVec2 size = ImVec2(0, 0), ImU32 color = Colors::Blue600);
    bool ModernButtonSuccess(const char* label, ImVec2 size = ImVec2(0, 0));
    bool ModernButtonDanger(const char* label, ImVec2 size = ImVec2(0, 0));
    bool ModernButtonSecondary(const char* label, ImVec2 size = ImVec2(0, 0));
    
    bool ModernMenuItem(const char* label, const char* shortcut = nullptr, bool selected = false);
    bool ModernSelectable(const char* label, bool selected = false, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));
    
    void ModernSeparator();
    void ModernSpacing(float height = 8.0f);
    
    bool BeginModernWindow(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
    void EndModernWindow();
    
    bool BeginModernMenuBar();
    void EndModernMenuBar();
    
    bool BeginModernMenu(const char* label);
    void EndModernMenu();
    
    void ModernText(const char* text, ImU32 color = Colors::Gray100);
    void ModernTextSecondary(const char* text);
    void ModernTextMuted(const char* text);
    void ModernHeader(const char* text);
    
    // Icons (using Unicode symbols for now)
    namespace Icons {
        constexpr const char* File = "📁";
        constexpr const char* Save = "💾";
        constexpr const char* Open = "📂";
        constexpr const char* Delete = "🗑️";
        constexpr const char* Edit = "✏️";
        constexpr const char* Play = "▶️";
        constexpr const char* Pause = "⏸️";
        constexpr const char* Stop = "⏹️";
        constexpr const char* Settings = "⚙️";
        constexpr const char* Blueprint = "🌱";
        constexpr const char* Code = "📝";
        constexpr const char* World = "🌍";
        constexpr const char* Console = "💻";
        constexpr const char* Search = "🔍";
        constexpr const char* Add = "➕";
        constexpr const char* Close = "❌";
        constexpr const char* Check = "✅";
        constexpr const char* Warning = "⚠️";
        constexpr const char* Info = "ℹ️";
        constexpr const char* Error = "❌";
    }
}
