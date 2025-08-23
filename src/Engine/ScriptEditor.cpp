#include "ScriptEditor.h"
#include <imgui.h>
#include <iostream>
#include <sstream>
#include <algorithm>

ScriptEditor::ScriptEditor() {
    BuildCompletionList();
}

ScriptEditor::~ScriptEditor() = default;

void ScriptEditor::Init() {
    // Initialize syntax highlighting colors and keywords
    UpdateSyntaxHighlighting();
}

void ScriptEditor::Shutdown() {
    SaveUnsavedChanges();
}

void ScriptEditor::Update(float deltaTime) {
    // Check for file modifications for hot reload
    if (!currentFilePath.empty() && std::filesystem::exists(currentFilePath)) {
        auto lastWriteTime = std::filesystem::last_write_time(currentFilePath);
        if (lastWriteTime != lastModificationTime) {
            // File was modified externally
            if (isModified) {
                // Show dialog asking user what to do
                showExternalModificationDialog = true;
            } else {
                // Auto-reload if no unsaved changes
                ReloadFile();
            }
        }
    }
}

void ScriptEditor::Render(bool* open) {
    if (!ImGui::Begin("Script Editor", open, ImGuiWindowFlags_MenuBar)) {
        ImGui::End();
        return;
    }

    ShowMenuBar();
    ShowToolbar();

    // Check for file changes for hot reload
    if (enableHotReload && !currentFilePath.empty() && !isNewFile) {
        CheckFileChanges();
    }

    ShowEditorArea();
    ShowStatusBar();

    ImGui::End();
}

bool ScriptEditor::OpenFile(const std::string& filepath) {
    if (!std::filesystem::exists(filepath)) {
        std::cerr << "File does not exist: " << filepath << std::endl;
        return false;
    }

    currentFilePath = filepath;
    isNewFile = false;

    if (LoadFileContent()) {
        UpdateSyntaxHighlighting();
        UpdateWindowTitle();

        // Update last write time for hot reload
        try {
            lastWriteTime = std::chrono::steady_clock::now();
        } catch (const std::exception& e) {
            std::cerr << "Error getting file time: " << e.what() << std::endl;
        }

        return true;
    }

    return false;
}

bool ScriptEditor::SaveFile() {
    if (isNewFile) {
        // Need to save as
        return false;
    }

    return SaveFileContent();
}

bool ScriptEditor::SaveFileAs(const std::string& filepath) {
    currentFilePath = filepath;
    isNewFile = false;
    UpdateWindowTitle();
    return SaveFileContent();
}

void ScriptEditor::NewFile() {
    textBuffer.clear();
    currentFilePath.clear();
    isNewFile = true;
    isModified = false;
    UpdateWindowTitle();

    // Add default template
    textBuffer = R"(// New Sprout Script (.sp) file
actor MyActor extends Actor {
    var health: float = 100.0

    fun beginPlay() {
        print("Hello from MyActor!")
        setLocation(0, 0, 0)
    }

    fun tick(deltaTime: float) {
        // Game logic here
    }
}
)";

    UpdateSyntaxHighlighting();
}

void ScriptEditor::CloseFile() {
    if (isModified) {
        // TODO: Show save dialog
    }

    textBuffer.clear();
    currentFilePath.clear();
    isNewFile = true;
    isModified = false;
    UpdateWindowTitle();
}

void ScriptEditor::SetText(const std::string& text) {
    textBuffer = text;
    isModified = true;
    UpdateSyntaxHighlighting();
}

std::string ScriptEditor::GetText() const {
    return textBuffer;
}

void ScriptEditor::SetHotReloadCallback(std::function<void(const std::string&)> callback) {
    hotReloadCallback = callback;
}

void ScriptEditor::TriggerHotReload() {
    if (hotReloadCallback && !currentFilePath.empty()) {
        hotReloadCallback(currentFilePath);
    }
}

void ScriptEditor::ShowMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                NewFile();
            }
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                // TODO: File dialog
            }
            if (ImGui::MenuItem("Save", "Ctrl+S", false, !isNewFile)) {
                SaveFile();
            }
            if (ImGui::MenuItem("Save As", "Ctrl+Shift+S")) {
                // TODO: File dialog
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Close", "Ctrl+W")) {
                CloseFile();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
                // TODO: Implement undo
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
                // TODO: Implement redo
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Find", "Ctrl+F")) {
                // TODO: Implement find
            }
            if (ImGui::MenuItem("Replace", "Ctrl+H")) {
                // TODO: Implement replace
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Line Numbers", nullptr, &showLineNumbers);
            ImGui::MenuItem("Syntax Highlighting", nullptr, &enableSyntaxHighlighting);
            ImGui::MenuItem("Auto Complete", nullptr, &enableAutoComplete);
            ImGui::Separator();
            ImGui::SliderFloat("Font Size", &fontSize, 8.0f, 24.0f);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            ImGui::MenuItem("Hot Reload", nullptr, &enableHotReload);
            if (ImGui::MenuItem("Compile Script")) {
                // TODO: Compile current script
            }
            if (ImGui::MenuItem("Format Code")) {
                // TODO: Format code
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void ScriptEditor::ShowToolbar() {
    if (ImGui::Button("New")) NewFile();
    ImGui::SameLine();
    if (ImGui::Button("Open")) {
        // TODO: File dialog
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        if (isNewFile) {
            // TODO: Save as dialog
        } else {
            SaveFile();
        }
    }
    ImGui::SameLine();

    ImGui::Text("|");
    ImGui::SameLine();

    if (ImGui::Button("Compile")) {
        // TODO: Compile script
    }
    ImGui::SameLine();
    if (ImGui::Button("Hot Reload") && !currentFilePath.empty()) {
        TriggerHotReload();
    }

    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();

    // Syntax highlighting toggle
    if (ImGui::Checkbox("Syntax", &enableSyntaxHighlighting)) {
        if (enableSyntaxHighlighting) {
            UpdateSyntaxHighlighting();
        }
    }

    ImGui::SameLine();
    ImGui::Checkbox("Auto-Complete", &enableAutoComplete);
}

void ScriptEditor::ShowEditorArea() {
    // Create child window for text editor
    ImGui::BeginChild("EditorContent", ImVec2(0, -30), true, ImGuiWindowFlags_HorizontalScrollbar);

    // Set font size
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Use default font with scaling

    // Text input with multi-line
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput |
                               ImGuiInputTextFlags_CallbackCharFilter |
                               ImGuiInputTextFlags_CallbackEdit;

    char buffer[32768];
    strncpy(buffer, textBuffer.c_str(), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    if (ImGui::InputTextMultiline("##editor", buffer, sizeof(buffer), ImVec2(-1, -1), flags)) {
        textBuffer = std::string(buffer);
        isModified = true;
        UpdateSyntaxHighlighting();
    }

    // Handle auto-completion
    if (enableAutoComplete) {
        HandleAutoComplete();
    }

    ImGui::PopFont();
    ImGui::EndChild();
}

void ScriptEditor::ShowStatusBar() {
    // Status bar
    ImGui::Separator();

    std::string status = isNewFile ? "Untitled" : std::filesystem::path(currentFilePath).filename().string();
    if (isModified) status += " *";

    ImGui::Text("File: %s", status.c_str());
    ImGui::SameLine();

    ImGui::Text("| Lines: %d", (int)std::count(textBuffer.begin(), textBuffer.end(), '\n') + 1);
    ImGui::SameLine();

    ImGui::Text("| Characters: %d", (int)textBuffer.length());

    if (enableHotReload && !isNewFile) {
        ImGui::SameLine();
        ImGui::Text("| Hot Reload: ON");
    }
}

void ScriptEditor::UpdateSyntaxHighlighting() {
    if (!enableSyntaxHighlighting) return;

    syntaxTokens.clear();

    // Simple syntax highlighting
    HighlightKeywords();
    HighlightStrings();
    HighlightComments();
    HighlightNumbers();
}

void ScriptEditor::HighlightKeywords() {
    auto keywords = GetSproutKeywords();

    for (const auto& keyword : keywords) {
        size_t pos = 0;
        while ((pos = textBuffer.find(keyword, pos)) != std::string::npos) {
            // Check if it's a whole word
            bool isWholeWord = true;
            if (pos > 0 && std::isalnum(textBuffer[pos - 1])) isWholeWord = false;
            if (pos + keyword.length() < textBuffer.length() &&
                std::isalnum(textBuffer[pos + keyword.length()])) isWholeWord = false;

            if (isWholeWord) {
                syntaxTokens.push_back({pos, keyword.length(), ImVec4(0.3f, 0.7f, 1.0f, 1.0f)}); // Blue
            }
            pos += keyword.length();
        }
    }
}

void ScriptEditor::HighlightStrings() {
    size_t pos = 0;
    while ((pos = textBuffer.find('"', pos)) != std::string::npos) {
        size_t endPos = textBuffer.find('"', pos + 1);
        if (endPos != std::string::npos) {
            syntaxTokens.push_back({pos, endPos - pos + 1, ImVec4(1.0f, 0.8f, 0.3f, 1.0f)}); // Yellow
            pos = endPos + 1;
        } else {
            break;
        }
    }
}

void ScriptEditor::HighlightComments() {
    size_t pos = 0;
    while ((pos = textBuffer.find("//", pos)) != std::string::npos) {
        size_t endPos = textBuffer.find('\n', pos);
        if (endPos == std::string::npos) endPos = textBuffer.length();

        syntaxTokens.push_back({pos, endPos - pos, ImVec4(0.5f, 0.7f, 0.5f, 1.0f)}); // Green
        pos = endPos;
    }
}

void ScriptEditor::HighlightNumbers() {
    // Simple number highlighting
    for (size_t i = 0; i < textBuffer.length(); ++i) {
        if (std::isdigit(textBuffer[i])) {
            size_t start = i;
            while (i < textBuffer.length() && (std::isdigit(textBuffer[i]) || textBuffer[i] == '.')) {
                i++;
            }
            syntaxTokens.push_back({start, i - start, ImVec4(1.0f, 0.5f, 0.5f, 1.0f)}); // Pink
        }
    }
}

void ScriptEditor::HandleAutoComplete() {
    // TODO: Implement auto-completion logic
}

void ScriptEditor::CheckFileChanges() {
    if (currentFilePath.empty() || isNewFile) return;

    try {
        auto currentTime = std::chrono::steady_clock::now();
        // Simple time-based check - in real implementation, check file modification time
        if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastWriteTime).count() > 1) {
            // File might have changed, could reload here
            lastWriteTime = currentTime;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error checking file changes: " << e.what() << std::endl;
    }
}

bool ScriptEditor::LoadFileContent() {
    try {
        std::ifstream file(currentFilePath);
        if (!file.is_open()) return false;

        std::stringstream buffer;
        buffer << file.rdbuf();
        textBuffer = buffer.str();
        isModified = false;

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading file: " << e.what() << std::endl;
        return false;
    }
}

bool ScriptEditor::SaveFileContent() {
    try {
        std::ofstream file(currentFilePath);
        if (!file.is_open()) return false;

        file << textBuffer;
        isModified = false;

        // Trigger hot reload after saving
        if (enableHotReload) {
            TriggerHotReload();
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving file: " << e.what() << std::endl;
        return false;
    }
}

void ScriptEditor::UpdateWindowTitle() {
    // Window title is handled by the parent window
}

void ScriptEditor::BuildCompletionList() {
    // Sprout Script keywords and functions
    completionItems = {
        {"actor", "Define an actor class", "actor ${1:MyActor} extends Actor {\n\t$0\n}"},
        {"extends", "Inherit from a base class", "extends ${1:Actor}"},
        {"var", "Declare a variable", "var ${1:name}: ${2:type} = ${3:value}"},
        {"fun", "Define a function", "fun ${1:name}(${2:params}) {\n\t$0\n}"},
        {"if", "Conditional statement", "if (${1:condition}) {\n\t$0\n}"},
        {"while", "While loop", "while (${1:condition}) {\n\t$0\n}"},
        {"for", "For loop", "for (${1:var} in ${2:range}) {\n\t$0\n}"},
        {"beginPlay", "Called when actor starts", "fun beginPlay() {\n\t$0\n}"},
        {"tick", "Called every frame", "fun tick(deltaTime: float) {\n\t$0\n}"},
        {"endPlay", "Called when actor ends", "fun endPlay() {\n\t$0\n}"},
        {"print", "Print to console", "print(${1:message})"},
        {"setLocation", "Set actor position", "setLocation(${1:x}, ${2:y}, ${3:z})"},
        {"getLocation", "Get actor position", "getLocation()"},
        {"setRotation", "Set actor rotation", "setRotation(${1:x}, ${2:y}, ${3:z})"},
        {"getRotation", "Get actor rotation", "getRotation()"},
        {"moveForward", "Move actor forward", "moveForward(${1:distance})"},
        {"destroy", "Destroy this actor", "destroy()"},
    };
}

std::vector<std::string> ScriptEditor::GetSproutKeywords() const {
    return {
        "actor", "extends", "var", "fun", "if", "else", "while", "for", "return",
        "true", "false", "null", "this", "super", "new", "delete",
        "int", "float", "string", "bool", "vector3", "array",
        "public", "private", "protected", "blueprint", "override"
    };
}

ImVec4 ScriptEditor::GetTokenColor(const std::string& token) const {
    auto keywords = GetSproutKeywords();
    if (std::find(keywords.begin(), keywords.end(), token) != keywords.end()) {
        return ImVec4(0.3f, 0.7f, 1.0f, 1.0f); // Blue for keywords
    }
    return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White for normal text
}

bool ScriptEditor::IsKeyword(const std::string& word) const {
    auto keywords = GetSproutKeywords();
    return std::find(keywords.begin(), keywords.end(), word) != keywords.end();
}

void ScriptEditor::SaveUnsavedChanges() {
    if (isModified && !currentFilePath.empty()) {
        SaveFile();
    }
}

void ScriptEditor::ReloadFile() {
    if (!currentFilePath.empty() && std::filesystem::exists(currentFilePath)) {
        std::ifstream file(currentFilePath);
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
            textBuffer = content;
            isModified = false;
            lastModificationTime = std::filesystem::last_write_time(currentFilePath);
        }
    }
}
