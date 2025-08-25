#include "SpCodeEditor.h"
#ifdef SP_TOOLCHAIN_ENABLED
#include "FileUtil.h"
#include <imgui.h>
#include <fstream>

static int TextEditCallback(ImGuiInputTextCallbackData* data){
    if(data->EventFlag == ImGuiInputTextFlags_CallbackResize){
        auto* str = static_cast<std::string*>(data->UserData);
        str->resize(data->BufSize - 1);
        data->Buf = str->data();
    }
    return 0;
}

void SpCodeEditor::open(const std::string& path){
    m_path = path;
    if(auto text = ReadTextFile(path))
        m_buffer = *text;
    else
        m_buffer.clear();
    m_open = true;
}

void SpCodeEditor::draw(){
    if(!m_open) return;
    std::string title = std::string("Script: ") + m_path;
    if(ImGui::Begin(title.c_str(), &m_open)){
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackResize;
        ImGui::InputTextMultiline("##spedit", m_buffer.data(), m_buffer.size()+1,
                                  ImVec2(-FLT_MIN, ImGui::GetTextLineHeight()*20),
                                  flags, TextEditCallback, &m_buffer);
        if(ImGui::Button("Save")){
            std::ofstream ofs(m_path);
            if(ofs) ofs << m_buffer;
        }
        ImGui::SameLine();
        if(ImGui::Button("Reload")){
            if(auto text = ReadTextFile(m_path))
                m_buffer = *text;
        }
    }
    ImGui::End();
}
#endif // SP_TOOLCHAIN_ENABLED
