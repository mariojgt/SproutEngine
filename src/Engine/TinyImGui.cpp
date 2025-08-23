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

    // Event-based input callbacks for better responsiveness
    void MouseButtonCallback(GLFWwindow*, int button, int action, int) {
        ImGuiIO& io = ImGui::GetIO();
        if (button >= 0 && button < ImGuiMouseButton_COUNT) {
            io.AddMouseButtonEvent(button, action == GLFW_PRESS);
        }
    }

    void ScrollCallback(GLFWwindow*, double, double yoffset) {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseWheelEvent(0.0f, (float)yoffset);
    }

    void KeyCallback(GLFWwindow*, int key, int, int action, int mods) {
        ImGuiIO& io = ImGui::GetIO();
        
        if (action != GLFW_PRESS && action != GLFW_RELEASE) return;
        
        // Handle modifier keys
        io.AddKeyEvent(ImGuiMod_Ctrl, (mods & GLFW_MOD_CONTROL) != 0);
        io.AddKeyEvent(ImGuiMod_Shift, (mods & GLFW_MOD_SHIFT) != 0);
        io.AddKeyEvent(ImGuiMod_Alt, (mods & GLFW_MOD_ALT) != 0);
        io.AddKeyEvent(ImGuiMod_Super, (mods & GLFW_MOD_SUPER) != 0);
        
        // Convert GLFW key to ImGui key
        ImGuiKey imgui_key = ImGuiKey_None;
        switch (key) {
            case GLFW_KEY_TAB: imgui_key = ImGuiKey_Tab; break;
            case GLFW_KEY_LEFT: imgui_key = ImGuiKey_LeftArrow; break;
            case GLFW_KEY_RIGHT: imgui_key = ImGuiKey_RightArrow; break;
            case GLFW_KEY_UP: imgui_key = ImGuiKey_UpArrow; break;
            case GLFW_KEY_DOWN: imgui_key = ImGuiKey_DownArrow; break;
            case GLFW_KEY_PAGE_UP: imgui_key = ImGuiKey_PageUp; break;
            case GLFW_KEY_PAGE_DOWN: imgui_key = ImGuiKey_PageDown; break;
            case GLFW_KEY_HOME: imgui_key = ImGuiKey_Home; break;
            case GLFW_KEY_END: imgui_key = ImGuiKey_End; break;
            case GLFW_KEY_INSERT: imgui_key = ImGuiKey_Insert; break;
            case GLFW_KEY_DELETE: imgui_key = ImGuiKey_Delete; break;
            case GLFW_KEY_BACKSPACE: imgui_key = ImGuiKey_Backspace; break;
            case GLFW_KEY_SPACE: imgui_key = ImGuiKey_Space; break;
            case GLFW_KEY_ENTER: imgui_key = ImGuiKey_Enter; break;
            case GLFW_KEY_ESCAPE: imgui_key = ImGuiKey_Escape; break;
            case GLFW_KEY_APOSTROPHE: imgui_key = ImGuiKey_Apostrophe; break;
            case GLFW_KEY_COMMA: imgui_key = ImGuiKey_Comma; break;
            case GLFW_KEY_MINUS: imgui_key = ImGuiKey_Minus; break;
            case GLFW_KEY_PERIOD: imgui_key = ImGuiKey_Period; break;
            case GLFW_KEY_SLASH: imgui_key = ImGuiKey_Slash; break;
            case GLFW_KEY_SEMICOLON: imgui_key = ImGuiKey_Semicolon; break;
            case GLFW_KEY_EQUAL: imgui_key = ImGuiKey_Equal; break;
            case GLFW_KEY_LEFT_BRACKET: imgui_key = ImGuiKey_LeftBracket; break;
            case GLFW_KEY_BACKSLASH: imgui_key = ImGuiKey_Backslash; break;
            case GLFW_KEY_RIGHT_BRACKET: imgui_key = ImGuiKey_RightBracket; break;
            case GLFW_KEY_GRAVE_ACCENT: imgui_key = ImGuiKey_GraveAccent; break;
            case GLFW_KEY_CAPS_LOCK: imgui_key = ImGuiKey_CapsLock; break;
            case GLFW_KEY_SCROLL_LOCK: imgui_key = ImGuiKey_ScrollLock; break;
            case GLFW_KEY_NUM_LOCK: imgui_key = ImGuiKey_NumLock; break;
            case GLFW_KEY_PRINT_SCREEN: imgui_key = ImGuiKey_PrintScreen; break;
            case GLFW_KEY_PAUSE: imgui_key = ImGuiKey_Pause; break;
            case GLFW_KEY_KP_0: imgui_key = ImGuiKey_Keypad0; break;
            case GLFW_KEY_KP_1: imgui_key = ImGuiKey_Keypad1; break;
            case GLFW_KEY_KP_2: imgui_key = ImGuiKey_Keypad2; break;
            case GLFW_KEY_KP_3: imgui_key = ImGuiKey_Keypad3; break;
            case GLFW_KEY_KP_4: imgui_key = ImGuiKey_Keypad4; break;
            case GLFW_KEY_KP_5: imgui_key = ImGuiKey_Keypad5; break;
            case GLFW_KEY_KP_6: imgui_key = ImGuiKey_Keypad6; break;
            case GLFW_KEY_KP_7: imgui_key = ImGuiKey_Keypad7; break;
            case GLFW_KEY_KP_8: imgui_key = ImGuiKey_Keypad8; break;
            case GLFW_KEY_KP_9: imgui_key = ImGuiKey_Keypad9; break;
            case GLFW_KEY_KP_DECIMAL: imgui_key = ImGuiKey_KeypadDecimal; break;
            case GLFW_KEY_KP_DIVIDE: imgui_key = ImGuiKey_KeypadDivide; break;
            case GLFW_KEY_KP_MULTIPLY: imgui_key = ImGuiKey_KeypadMultiply; break;
            case GLFW_KEY_KP_SUBTRACT: imgui_key = ImGuiKey_KeypadSubtract; break;
            case GLFW_KEY_KP_ADD: imgui_key = ImGuiKey_KeypadAdd; break;
            case GLFW_KEY_KP_ENTER: imgui_key = ImGuiKey_KeypadEnter; break;
            case GLFW_KEY_KP_EQUAL: imgui_key = ImGuiKey_KeypadEqual; break;
            case GLFW_KEY_LEFT_SHIFT: imgui_key = ImGuiKey_LeftShift; break;
            case GLFW_KEY_LEFT_CONTROL: imgui_key = ImGuiKey_LeftCtrl; break;
            case GLFW_KEY_LEFT_ALT: imgui_key = ImGuiKey_LeftAlt; break;
            case GLFW_KEY_LEFT_SUPER: imgui_key = ImGuiKey_LeftSuper; break;
            case GLFW_KEY_RIGHT_SHIFT: imgui_key = ImGuiKey_RightShift; break;
            case GLFW_KEY_RIGHT_CONTROL: imgui_key = ImGuiKey_RightCtrl; break;
            case GLFW_KEY_RIGHT_ALT: imgui_key = ImGuiKey_RightAlt; break;
            case GLFW_KEY_RIGHT_SUPER: imgui_key = ImGuiKey_RightSuper; break;
            case GLFW_KEY_MENU: imgui_key = ImGuiKey_Menu; break;
            case GLFW_KEY_0: imgui_key = ImGuiKey_0; break;
            case GLFW_KEY_1: imgui_key = ImGuiKey_1; break;
            case GLFW_KEY_2: imgui_key = ImGuiKey_2; break;
            case GLFW_KEY_3: imgui_key = ImGuiKey_3; break;
            case GLFW_KEY_4: imgui_key = ImGuiKey_4; break;
            case GLFW_KEY_5: imgui_key = ImGuiKey_5; break;
            case GLFW_KEY_6: imgui_key = ImGuiKey_6; break;
            case GLFW_KEY_7: imgui_key = ImGuiKey_7; break;
            case GLFW_KEY_8: imgui_key = ImGuiKey_8; break;
            case GLFW_KEY_9: imgui_key = ImGuiKey_9; break;
            case GLFW_KEY_A: imgui_key = ImGuiKey_A; break;
            case GLFW_KEY_B: imgui_key = ImGuiKey_B; break;
            case GLFW_KEY_C: imgui_key = ImGuiKey_C; break;
            case GLFW_KEY_D: imgui_key = ImGuiKey_D; break;
            case GLFW_KEY_E: imgui_key = ImGuiKey_E; break;
            case GLFW_KEY_F: imgui_key = ImGuiKey_F; break;
            case GLFW_KEY_G: imgui_key = ImGuiKey_G; break;
            case GLFW_KEY_H: imgui_key = ImGuiKey_H; break;
            case GLFW_KEY_I: imgui_key = ImGuiKey_I; break;
            case GLFW_KEY_J: imgui_key = ImGuiKey_J; break;
            case GLFW_KEY_K: imgui_key = ImGuiKey_K; break;
            case GLFW_KEY_L: imgui_key = ImGuiKey_L; break;
            case GLFW_KEY_M: imgui_key = ImGuiKey_M; break;
            case GLFW_KEY_N: imgui_key = ImGuiKey_N; break;
            case GLFW_KEY_O: imgui_key = ImGuiKey_O; break;
            case GLFW_KEY_P: imgui_key = ImGuiKey_P; break;
            case GLFW_KEY_Q: imgui_key = ImGuiKey_Q; break;
            case GLFW_KEY_R: imgui_key = ImGuiKey_R; break;
            case GLFW_KEY_S: imgui_key = ImGuiKey_S; break;
            case GLFW_KEY_T: imgui_key = ImGuiKey_T; break;
            case GLFW_KEY_U: imgui_key = ImGuiKey_U; break;
            case GLFW_KEY_V: imgui_key = ImGuiKey_V; break;
            case GLFW_KEY_W: imgui_key = ImGuiKey_W; break;
            case GLFW_KEY_X: imgui_key = ImGuiKey_X; break;
            case GLFW_KEY_Y: imgui_key = ImGuiKey_Y; break;
            case GLFW_KEY_Z: imgui_key = ImGuiKey_Z; break;
            case GLFW_KEY_F1: imgui_key = ImGuiKey_F1; break;
            case GLFW_KEY_F2: imgui_key = ImGuiKey_F2; break;
            case GLFW_KEY_F3: imgui_key = ImGuiKey_F3; break;
            case GLFW_KEY_F4: imgui_key = ImGuiKey_F4; break;
            case GLFW_KEY_F5: imgui_key = ImGuiKey_F5; break;
            case GLFW_KEY_F6: imgui_key = ImGuiKey_F6; break;
            case GLFW_KEY_F7: imgui_key = ImGuiKey_F7; break;
            case GLFW_KEY_F8: imgui_key = ImGuiKey_F8; break;
            case GLFW_KEY_F9: imgui_key = ImGuiKey_F9; break;
            case GLFW_KEY_F10: imgui_key = ImGuiKey_F10; break;
            case GLFW_KEY_F11: imgui_key = ImGuiKey_F11; break;
            case GLFW_KEY_F12: imgui_key = ImGuiKey_F12; break;
            default: break;
        }
        
        if (imgui_key != ImGuiKey_None) {
            io.AddKeyEvent(imgui_key, action == GLFW_PRESS);
        }
    }

    void CharCallback(GLFWwindow*, unsigned int c) {
        ImGuiIO& io = ImGui::GetIO();
        io.AddInputCharacter(c);
    }

    void WindowFocusCallback(GLFWwindow*, int focused) {
        ImGuiIO& io = ImGui::GetIO();
        io.AddFocusEvent(focused != 0);
    }

    void CursorEnterCallback(GLFWwindow*, int entered) {
        ImGuiIO& io = ImGui::GetIO();
        if (!entered) {
            io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
        }
    }

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

    // Register event-based input callbacks for better responsiveness
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCharCallback(window, CharCallback);

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

    // Update window size
    int w, h; glfwGetWindowSize(g_Window, &w, &h);
    int fbw, fbh; glfwGetFramebufferSize(g_Window, &fbw, &fbh);
    io.DisplaySize = ImVec2((float)w, (float)h);
    if (w > 0 && h > 0) io.DisplayFramebufferScale = ImVec2((float)fbw / w, (float)fbh / h);

    // Update mouse position (polling is fine for this)
    double mx, my; glfwGetCursorPos(g_Window, &mx, &my);
    io.AddMousePosEvent((float)mx, (float)my);

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
