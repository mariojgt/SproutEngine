#pragma once
#include <imgui.h>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <fstream>
#include <functional>
#include <chrono>

/**
 * In-engine Script Editor for .sp files
 * Features syntax highlighting, auto-completion, and hot reload
 */
class ScriptEditor {
public:
    ScriptEditor();
    ~ScriptEditor();

    void Init();
    void Shutdown();
    void Update(float deltaTime);
    void Render(bool* open = nullptr);

    // File operations
    bool OpenFile(const std::string& filepath);
    bool SaveFile();
    bool SaveFileAs(const std::string& filepath);
    void NewFile();
    void CloseFile();

    // Editor features
    void SetText(const std::string& text);
    std::string GetText() const;
    bool HasUnsavedChanges() const { return isModified; }
    const std::string& GetCurrentFile() const { return currentFilePath; }

    // Hot reload support
    void SetHotReloadCallback(std::function<void(const std::string&)> callback);
    void TriggerHotReload();

private:
    // Editor state
    std::string textBuffer;
    std::string currentFilePath;
    bool isModified = false;
    bool isNewFile = true;

    // Editor settings
    bool showLineNumbers = true;
    bool enableSyntaxHighlighting = true;
    bool enableAutoComplete = true;
    bool enableHotReload = true;
    float fontSize = 14.0f;

    // Hot reload
    std::function<void(const std::string&)> hotReloadCallback;
    std::chrono::time_point<std::chrono::steady_clock> lastWriteTime;
    std::filesystem::file_time_type lastModificationTime;
    bool showExternalModificationDialog = false;

    // Syntax highlighting
    struct SyntaxToken {
        size_t start;
        size_t length;
        ImVec4 color;
    };
    std::vector<SyntaxToken> syntaxTokens;

    // Auto-completion
    struct CompletionItem {
        std::string text;
        std::string description;
        std::string insertText;
    };
    std::vector<CompletionItem> completionItems;
    bool showCompletionPopup = false;
    int selectedCompletion = 0;

    // Editor methods
    void UpdateSyntaxHighlighting();
    void HandleInput();
    void HandleAutoComplete();
    void CheckFileChanges();
    void ShowMenuBar();
    void ShowToolbar();
    void ShowEditorArea();
    void ShowStatusBar();

    // Syntax highlighting helpers
    void HighlightKeywords();
    void HighlightStrings();
    void HighlightComments();
    void HighlightNumbers();

    // Auto-completion helpers
    void BuildCompletionList();
    void ShowCompletionWindow();
    std::string GetWordAtCursor() const;

    // Utility
    ImVec4 GetTokenColor(const std::string& token) const;
    void SaveUnsavedChanges();
    void ReloadFile();
    bool IsKeyword(const std::string& word) const;
    std::vector<std::string> GetSproutKeywords() const;

    // File handling
    bool LoadFileContent();
    bool SaveFileContent();
    void UpdateWindowTitle();
};
