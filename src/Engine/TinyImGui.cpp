#include "TinyImGui.h"
#include <imgui.h>
#include <cstdio>

namespace {
    GLFWwindow* g_Window = nullptr;
    double g_Time = 0.0;
    unsigned int g_FontTexture = 0;
    unsigned int g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
    int g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
    int g_AttribLocationVtxPos = 0, g_AttribLocationVtxUV = 0, g_AttribLocationVtxColor = 0;
    unsigned int g_VboHandle = 0, g_ElementsHandle = 0, g_VaoHandle = 0;

    const char* GetClipboardText(void*) { return glfwGetClipboardString(g_Window); }
    void SetClipboardText(void*, const char* text) { glfwSetClipboardString(g_Window, text); }

    void CreateFontsTexture() {
        ImGuiIO& io = ImGui::GetIO();
        unsigned char* pixels; int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        glGenTextures(1, &g_FontTexture);
        glBindTexture(GL_TEXTURE_2D, g_FontTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        io.Fonts->SetTexID((ImTextureID)(intptr_t)g_FontTexture);
    }

    void DestroyFontsTexture() {
        if (g_FontTexture) {
            ImGui::GetIO().Fonts->SetTexID(0);
            glDeleteTextures(1, &g_FontTexture);
            g_FontTexture = 0;
        }
    }

    void CreateDeviceObjects() {
        const char* vertex_shader =
            "#version 330 core\n"
            "uniform mat4 ProjMtx;\n"
            "layout (location = 0) in vec2 Position;\n"
            "layout (location = 1) in vec2 UV;\n"
            "layout (location = 2) in vec4 Color;\n"
            "out vec2 Frag_UV;\n"
            "out vec4 Frag_Color;\n"
            "void main(){ Frag_UV = UV; Frag_Color = Color; gl_Position = ProjMtx * vec4(Position,0,1);}";
        const char* fragment_shader =
            "#version 330 core\n"
            "in vec2 Frag_UV;\n"
            "in vec4 Frag_Color;\n"
            "uniform sampler2D Texture;\n"
            "out vec4 Out_Color;\n"
            "void main(){ Out_Color = Frag_Color * texture(Texture, Frag_UV.st); }";

        g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(g_VertHandle, 1, &vertex_shader, 0);
        glCompileShader(g_VertHandle);
        g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(g_FragHandle, 1, &fragment_shader, 0);
        glCompileShader(g_FragHandle);
        g_ShaderHandle = glCreateProgram();
        glAttachShader(g_ShaderHandle, g_VertHandle);
        glAttachShader(g_ShaderHandle, g_FragHandle);
        glLinkProgram(g_ShaderHandle);

        g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
        g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
        g_AttribLocationVtxPos = 0;
        g_AttribLocationVtxUV = 1;
        g_AttribLocationVtxColor = 2;

        glGenBuffers(1, &g_VboHandle);
        glGenBuffers(1, &g_ElementsHandle);

        glGenVertexArrays(1, &g_VaoHandle);
        glBindVertexArray(g_VaoHandle);
        glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
        glEnableVertexAttribArray(g_AttribLocationVtxPos);
        glEnableVertexAttribArray(g_AttribLocationVtxUV);
        glEnableVertexAttribArray(g_AttribLocationVtxColor);
        glVertexAttribPointer(g_AttribLocationVtxPos,   2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
        glVertexAttribPointer(g_AttribLocationVtxUV,    2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
        glVertexAttribPointer(g_AttribLocationVtxColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));
        glBindVertexArray(0);

        CreateFontsTexture();
    }

    void DestroyDeviceObjects() {
        DestroyFontsTexture();
        if (g_VaoHandle) glDeleteVertexArrays(1, &g_VaoHandle);
        if (g_VboHandle) glDeleteBuffers(1, &g_VboHandle);
        if (g_ElementsHandle) glDeleteBuffers(1, &g_ElementsHandle);
        if (g_VertHandle) glDeleteShader(g_VertHandle);
        if (g_FragHandle) glDeleteShader(g_FragHandle);
        if (g_ShaderHandle) glDeleteProgram(g_ShaderHandle);
        g_VaoHandle = g_VboHandle = g_ElementsHandle = 0;
        g_VertHandle = g_FragHandle = g_ShaderHandle = 0;
    }
}

namespace TinyImGui {

bool Init(GLFWwindow* window){
    g_Window = window;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = "tiny_glfw";
    io.BackendRendererName = "tiny_opengl3";
    io.SetClipboardTextFn = SetClipboardText;
    io.GetClipboardTextFn = GetClipboardText;
    io.MouseDrawCursor = false;

    int w, h; glfwGetWindowSize(window, &w, &h);
    int fbw, fbh; glfwGetFramebufferSize(window, &fbw, &fbh);
    io.DisplaySize = ImVec2((float)w, (float)h);
    if (w > 0 && h > 0) io.DisplayFramebufferScale = ImVec2((float)fbw / w, (float)fbh / h);

    CreateDeviceObjects();
    return true;
}

void Shutdown(){
    DestroyDeviceObjects();
    ImGui::DestroyContext();
    g_Window = nullptr;
}

void NewFrame(){
    ImGuiIO& io = ImGui::GetIO();
    double current_time = glfwGetTime();
    io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (1.0f / 60.0f);
    g_Time = current_time;

    // Mouse
    double mx, my; glfwGetCursorPos(g_Window, &mx, &my);
    io.AddMousePosEvent((float)mx, (float)my);
    io.AddMouseButtonEvent(0, glfwGetMouseButton(g_Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    io.AddMouseButtonEvent(1, glfwGetMouseButton(g_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    io.AddMouseButtonEvent(2, glfwGetMouseButton(g_Window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);

    // Modifiers (approx)
    io.AddKeyEvent(ImGuiMod_Ctrl,  glfwGetKey(g_Window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(g_Window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
    io.AddKeyEvent(ImGuiMod_Shift, glfwGetKey(g_Window, GLFW_KEY_LEFT_SHIFT)   == GLFW_PRESS || glfwGetKey(g_Window, GLFW_KEY_RIGHT_SHIFT)   == GLFW_PRESS);
    io.AddKeyEvent(ImGuiMod_Alt,   glfwGetKey(g_Window, GLFW_KEY_LEFT_ALT)     == GLFW_PRESS || glfwGetKey(g_Window, GLFW_KEY_RIGHT_ALT)     == GLFW_PRESS);
    io.AddKeyEvent(ImGuiMod_Super, glfwGetKey(g_Window, GLFW_KEY_LEFT_SUPER)   == GLFW_PRESS || glfwGetKey(g_Window, GLFW_KEY_RIGHT_SUPER)   == GLFW_PRESS);

    ImGui::NewFrame();
}

void RenderDrawData(ImDrawData* draw_data){
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    const float L = draw_data->DisplayPos.x;
    const float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    const float T = draw_data->DisplayPos.y;
    const float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    const float ortho_projection[4][4] = {
        { 2.0f/(R-L), 0.0f,         0.0f, 0.0f },
        { 0.0f,       2.0f/(T-B),   0.0f, 0.0f },
        { 0.0f,       0.0f,        -1.0f, 0.0f },
        { (R+L)/(L-R), (T+B)/(B-T), 0.0f, 1.0f },
    };

    glUseProgram(g_ShaderHandle);
    glUniform1i(g_AttribLocationTex, 0);
    glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
    glBindVertexArray(g_VaoHandle);
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * (int)sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * (int)sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

        int idx_offset = 0;
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback) {
                pcmd->UserCallback(cmd_list, pcmd);
            } else {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->GetTexID());
                glScissor((int)(pcmd->ClipRect.x), (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx)==2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(idx_offset * sizeof(ImDrawIdx)));
            }
            idx_offset += pcmd->ElemCount;
        }
    }
    glBindVertexArray(0);
    glUseProgram(0);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
}

} // namespace TinyImGui
