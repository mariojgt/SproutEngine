#pragma once
#include <imgui.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

/**
 * Dockable Window System
 * Manages layout, persistence, and dynamic window creation
 */
class DockingManager {
public:
    DockingManager();
    ~DockingManager();

    void Init();
    void Shutdown();
    void Update();

    // Window management
    struct DockableWindow {
        std::string id;
        std::string title;
        std::function<void()> renderFunc;
        bool isOpen = true;
        bool isVisible = true;
        ImGuiWindowFlags flags = 0;

        // Docking preferences
        ImGuiDir preferredDockDir = ImGuiDir_None;
        std::string preferredDockTarget = "";
        float splitRatio = 0.5f;

        // Size constraints
        ImVec2 minSize = ImVec2(100, 100);
        ImVec2 maxSize = ImVec2(FLT_MAX, FLT_MAX);
        ImVec2 defaultSize = ImVec2(400, 300);

        DockableWindow(const std::string& id, const std::string& title, std::function<void()> render)
            : id(id), title(title), renderFunc(render) {}
    };

    // Register windows
    void RegisterWindow(const std::string& id, const std::string& title, std::function<void()> renderFunc);
    void RegisterWindow(std::shared_ptr<DockableWindow> window);
    void UnregisterWindow(const std::string& id);

    // Window operations
    void ShowWindow(const std::string& id, bool show = true);
    void HideWindow(const std::string& id);
    void ToggleWindow(const std::string& id);
    bool IsWindowOpen(const std::string& id) const;
    bool IsWindowVisible(const std::string& id) const;

    // Docking operations
    void DockWindow(const std::string& windowId, const std::string& targetId, ImGuiDir direction, float ratio = 0.5f);
    void FloatWindow(const std::string& id);
    void TabifyWindow(const std::string& windowId, const std::string& targetId);

    // Layout management
    void SaveLayout(const std::string& name);
    void LoadLayout(const std::string& name);
    void ResetToDefaultLayout();
    void DeleteLayout(const std::string& name);
    std::vector<std::string> GetSavedLayouts() const;

    // Menu integration
    void RenderWindowMenu();
    void RenderLayoutMenu();

    // Workspace management
    void CreateWorkspace(const std::string& name, const std::vector<std::string>& windowIds);
    void SwitchToWorkspace(const std::string& name);
    void DeleteWorkspace(const std::string& name);
    std::vector<std::string> GetWorkspaces() const;

    // Event callbacks
    void SetWindowFocusCallback(std::function<void(const std::string&)> callback) {
        windowFocusCallback = callback;
    }
    void SetWindowCloseCallback(std::function<void(const std::string&)> callback) {
        windowCloseCallback = callback;
    }

    // Utility
    std::shared_ptr<DockableWindow> GetWindow(const std::string& id);
    const std::vector<std::shared_ptr<DockableWindow>>& GetAllWindows() const { return windows; }

    // Docking space management
    void BeginDockSpace();
    void EndDockSpace();
    ImGuiID GetMainDockSpaceID() const { return mainDockSpaceID; }

    // Theme and customization
    void SetDockingTheme(const std::string& theme);
    void CustomizeDockingStyle();

private:
    std::vector<std::shared_ptr<DockableWindow>> windows;
    std::unordered_map<std::string, std::shared_ptr<DockableWindow>> windowMap;

    // Layout data
    struct LayoutData {
        std::string name;
        std::string iniData;
        std::vector<std::string> visibleWindows;
    };
    std::unordered_map<std::string, LayoutData> savedLayouts;

    // Workspace data
    struct Workspace {
        std::string name;
        std::vector<std::string> windowIds;
        std::string layoutName;
    };
    std::unordered_map<std::string, Workspace> workspaces;
    std::string currentWorkspace = "Default";

    // Docking state
    ImGuiID mainDockSpaceID = 0;
    bool dockSpaceInitialized = false;
    bool isDockingEnabled = true;

    // Callbacks
    std::function<void(const std::string&)> windowFocusCallback;
    std::function<void(const std::string&)> windowCloseCallback;

    // Internal methods
    void RenderWindows();
    void SetupDefaultLayout();
    void HandleDockingOperations();
    void UpdateWindowStates();

    // Layout persistence
    std::string GetLayoutFilePath(const std::string& name) const;
    void SaveLayoutToFile(const std::string& name, const std::string& iniData);
    std::string LoadLayoutFromFile(const std::string& name);

    // Configuration
    std::string configDirectory = "config/layouts/";
    std::string defaultLayoutName = "Default";

    // Built-in window IDs (for easy reference)
public:
    static constexpr const char* VIEWPORT_WINDOW = "Viewport";
    static constexpr const char* OUTLINER_WINDOW = "World Outliner";
    static constexpr const char* INSPECTOR_WINDOW = "Details";
    static constexpr const char* CONTENT_BROWSER_WINDOW = "Content Browser";
    static constexpr const char* CONSOLE_WINDOW = "Output Log";
    static constexpr const char* SCRIPT_EDITOR_WINDOW = "Script Editor";
    static constexpr const char* BLUEPRINT_EDITOR_WINDOW = "Blueprint Editor";
    static constexpr const char* ANIMATION_WINDOW = "Animation";
    static constexpr const char* MATERIALS_WINDOW = "Material Editor";
    static constexpr const char* PROFILER_WINDOW = "Profiler";
    static constexpr const char* DEBUG_WINDOW = "Debug";
};
