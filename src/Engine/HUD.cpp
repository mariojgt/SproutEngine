#include "HUD.h"
#include <imgui.h>

namespace UI {

static void ProgressBarFancy(const char* label, float v01, ImVec4 colFill){
    ImGui::TextUnformatted(label);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f,0.16f,0.20f,1.0f));
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, colFill);
    ImGui::ProgressBar(v01, ImVec2(-1, 14));
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
}

void HUDRenderer::draw(const HUDState& s){
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::Begin("HUDOverlay", nullptr,
        ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoInputs|ImGuiWindowFlags_NoBackground);
    // Position at top
    ImGui::SetWindowPos(ImVec2(20, 20), ImGuiCond_Always);
    ImGui::SetWindowSize(ImVec2(420, 120), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12,12));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f,0.09f,0.12f,0.7f));
    ImGui::BeginChild("HUDCard", ImVec2(400, 100), true, ImGuiWindowFlags_NoScrollbar);

    ImGui::Text("%s", s.title.c_str());
    ImGui::Separator();
    ProgressBarFancy("Health", s.health/100.0f, ImVec4(0.92f,0.20f,0.24f,1.0f));
    ProgressBarFancy("Mana",   s.mana  /100.0f, ImVec4(0.24f,0.52f,0.96f,1.0f));
    ImGui::Text("Score: %d", s.score);

    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::End();
}

} // namespace UI
