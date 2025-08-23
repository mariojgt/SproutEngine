#include "DockingManager.h"
#include <imgui_internal.h>
#include <fstream>
#include <filesystem>
#include <algorithm>

DockingManager::DockingManager() {
}

DockingManager::~DockingManager() {
    Shutdown();
}

void DockingManager::Init() {
    // Enable docking (if available in this ImGui version)
    ImGuiIO& io = ImGui::GetIO();
    // Note: Docking may not be available in all ImGui versions
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Create config directory if it doesn't exist
    if (!std::filesystem::exists(configDirectory)) {
        std::filesystem::create_directories(configDirectory);
    }

    // Setup default workspace
    CreateWorkspace("Default", {
        VIEWPORT_WINDOW,
        OUTLINER_WINDOW,
        INSPECTOR_WINDOW,
        CONTENT_BROWSER_WINDOW,
        CONSOLE_WINDOW
    });

    // Setup development workspace
    CreateWorkspace("Development", {
        VIEWPORT_WINDOW,
        OUTLINER_WINDOW,
        INSPECTOR_WINDOW,
        SCRIPT_EDITOR_WINDOW,
        BLUEPRINT_EDITOR_WINDOW,
        CONSOLE_WINDOW
    });

    // Setup debug workspace
    CreateWorkspace("Debug", {
        VIEWPORT_WINDOW,
        OUTLINER_WINDOW,
        INSPECTOR_WINDOW,
        DEBUG_WINDOW,
        PROFILER_WINDOW,
        CONSOLE_WINDOW
    });
}

void DockingManager::Shutdown() {
    // Save current layout before shutdown
    if (!savedLayouts.empty()) {
        SaveLayout("LastSession");
    }

    windows.clear();
    windowMap.clear();
    savedLayouts.clear();
    workspaces.clear();
}

void DockingManager::Update() {
    BeginDockSpace();

    UpdateWindowStates();
    RenderWindows();
    HandleDockingOperations();

    EndDockSpace();
}

void DockingManager::RegisterWindow(const std::string& id, const std::string& title, std::function<void()> renderFunc) {
    auto window = std::make_shared<DockableWindow>(id, title, renderFunc);
    RegisterWindow(window);
}

void DockingManager::RegisterWindow(std::shared_ptr<DockableWindow> window) {
    if (!window) return;

    // Check if window already exists
    auto it = windowMap.find(window->id);
    if (it != windowMap.end()) {
        // Update existing window
        it->second = window;
        // Find and replace in vector
        auto vecIt = std::find_if(windows.begin(), windows.end(),
            [&](const auto& w) { return w->id == window->id; });
        if (vecIt != windows.end()) {
            *vecIt = window;
        }
    } else {
        // Add new window
        windows.push_back(window);
        windowMap[window->id] = window;
    }
}

void DockingManager::UnregisterWindow(const std::string& id) {
    auto it = windowMap.find(id);
    if (it != windowMap.end()) {
        // Remove from vector
        auto vecIt = std::find_if(windows.begin(), windows.end(),
            [&](const auto& w) { return w->id == id; });
        if (vecIt != windows.end()) {
            windows.erase(vecIt);
        }

        // Remove from map
        windowMap.erase(it);
    }
}

void DockingManager::ShowWindow(const std::string& id, bool show) {
    auto window = GetWindow(id);
    if (window) {
        window->isOpen = show;
        window->isVisible = show;
    }
}

void DockingManager::HideWindow(const std::string& id) {
    ShowWindow(id, false);
}

void DockingManager::ToggleWindow(const std::string& id) {
    auto window = GetWindow(id);
    if (window) {
        ShowWindow(id, !window->isOpen);
    }
}

bool DockingManager::IsWindowOpen(const std::string& id) const {
    auto it = windowMap.find(id);
    return it != windowMap.end() && it->second->isOpen;
}

bool DockingManager::IsWindowVisible(const std::string& id) const {
    auto it = windowMap.find(id);
    return it != windowMap.end() && it->second->isVisible;
}

void DockingManager::DockWindow(const std::string& windowId, const std::string& targetId, ImGuiDir direction, float ratio) {
    auto window = GetWindow(windowId);
    if (window) {
        window->preferredDockTarget = targetId;
        window->preferredDockDir = direction;
        window->splitRatio = ratio;
    }
}

void DockingManager::FloatWindow(const std::string& id) {
    // This would require ImGui internal API to undock a window
    // For now, we'll mark it as preferring no dock target
    auto window = GetWindow(id);
    if (window) {
        window->preferredDockTarget = "";
        window->preferredDockDir = ImGuiDir_None;
    }
}

void DockingManager::TabifyWindow(const std::string& windowId, const std::string& targetId) {
    DockWindow(windowId, targetId, ImGuiDir_None, 0.5f);
}

void DockingManager::SaveLayout(const std::string& name) {
    LayoutData layout;
    layout.name = name;

    // Get current ImGui layout
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    // Note: SettingsWindows may not be available in all ImGui versions
    if (ctx) {
        // This is a simplified version - in practice you'd save the full ini data
        layout.iniData = ""; // Would contain ImGui's SaveIniSettingsToMemory() result
    }

    // Save visible windows
    for (const auto& window : windows) {
        if (window->isVisible) {
            layout.visibleWindows.push_back(window->id);
        }
    }

    savedLayouts[name] = layout;
    SaveLayoutToFile(name, layout.iniData);
}

void DockingManager::LoadLayout(const std::string& name) {
    auto it = savedLayouts.find(name);
    if (it != savedLayouts.end()) {
        const LayoutData& layout = it->second;

        // Hide all windows first
        for (auto& window : windows) {
            window->isVisible = false;
        }

        // Show windows that were visible in this layout
        for (const std::string& windowId : layout.visibleWindows) {
            ShowWindow(windowId, true);
        }

        // Load ImGui layout
        if (!layout.iniData.empty()) {
            // Would use ImGui::LoadIniSettingsFromMemory(layout.iniData.c_str())
        }
    }
}

void DockingManager::ResetToDefaultLayout() {
    SetupDefaultLayout();
}

void DockingManager::DeleteLayout(const std::string& name) {
    auto it = savedLayouts.find(name);
    if (it != savedLayouts.end()) {
        savedLayouts.erase(it);

        // Delete file
        std::string filepath = GetLayoutFilePath(name);
        if (std::filesystem::exists(filepath)) {
            std::filesystem::remove(filepath);
        }
    }
}

std::vector<std::string> DockingManager::GetSavedLayouts() const {
    std::vector<std::string> layouts;
    for (const auto& pair : savedLayouts) {
        layouts.push_back(pair.first);
    }
    return layouts;
}

void DockingManager::RenderWindowMenu() {
    if (ImGui::BeginMenu("Windows")) {
        for (const auto& window : windows) {
            bool isOpen = window->isOpen;
            if (ImGui::MenuItem(window->title.c_str(), nullptr, &isOpen)) {
                ShowWindow(window->id, isOpen);
            }
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("Workspaces")) {
            for (const auto& pair : workspaces) {
                bool isCurrent = (currentWorkspace == pair.first);
                if (ImGui::MenuItem(pair.first.c_str(), nullptr, isCurrent)) {
                    SwitchToWorkspace(pair.first);
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }
}

void DockingManager::RenderLayoutMenu() {
    if (ImGui::BeginMenu("Layout")) {
        if (ImGui::MenuItem("Save Current Layout...")) {
            // Would open a dialog to name the layout
            SaveLayout("Custom_" + std::to_string(savedLayouts.size()));
        }

        if (ImGui::MenuItem("Reset to Default")) {
            ResetToDefaultLayout();
        }

        ImGui::Separator();

        auto layouts = GetSavedLayouts();
        for (const std::string& layoutName : layouts) {
            if (ImGui::MenuItem(layoutName.c_str())) {
                LoadLayout(layoutName);
            }
        }

        ImGui::EndMenu();
    }
}

void DockingManager::CreateWorkspace(const std::string& name, const std::vector<std::string>& windowIds) {
    Workspace workspace;
    workspace.name = name;
    workspace.windowIds = windowIds;
    workspace.layoutName = name + "_Layout";

    workspaces[name] = workspace;
}

void DockingManager::SwitchToWorkspace(const std::string& name) {
    auto it = workspaces.find(name);
    if (it != workspaces.end()) {
        currentWorkspace = name;
        const Workspace& workspace = it->second;

        // Hide all windows
        for (auto& window : windows) {
            window->isVisible = false;
        }

        // Show workspace windows
        for (const std::string& windowId : workspace.windowIds) {
            ShowWindow(windowId, true);
        }

        // Load workspace layout if it exists
        auto layoutIt = savedLayouts.find(workspace.layoutName);
        if (layoutIt != savedLayouts.end()) {
            LoadLayout(workspace.layoutName);
        } else {
            SetupDefaultLayout();
        }
    }
}

void DockingManager::DeleteWorkspace(const std::string& name) {
    auto it = workspaces.find(name);
    if (it != workspaces.end()) {
        // Delete associated layout
        DeleteLayout(it->second.layoutName);

        // Remove workspace
        workspaces.erase(it);

        // Switch to default if this was current
        if (currentWorkspace == name) {
            SwitchToWorkspace("Default");
        }
    }
}

std::vector<std::string> DockingManager::GetWorkspaces() const {
    std::vector<std::string> workspaceNames;
    for (const auto& pair : workspaces) {
        workspaceNames.push_back(pair.first);
    }
    return workspaceNames;
}

std::shared_ptr<DockingManager::DockableWindow> DockingManager::GetWindow(const std::string& id) {
    auto it = windowMap.find(id);
    return (it != windowMap.end()) ? it->second : nullptr;
}

void DockingManager::BeginDockSpace() {
    // Get main viewport
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    // Set up dockspace (simplified version without advanced docking)
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    // Note: SetNextWindowViewport may not be available
    // ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    windowFlags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    // Simplified docking (may not work without docking support)
    ImGuiIO& io = ImGui::GetIO();
    // Note: ConfigFlags_DockingEnable may not be available
    // if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        mainDockSpaceID = ImGui::GetID("MainDockSpace");
        // Note: DockSpace may not be available
        // ImGui::DockSpace(mainDockSpaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        // Setup default layout on first run
        if (!dockSpaceInitialized) {
            // SetupDefaultLayout();
            dockSpaceInitialized = true;
        }
    // }
}

void DockingManager::EndDockSpace() {
    ImGui::End();
}

void DockingManager::SetDockingTheme(const std::string& theme) {
    // Would customize ImGui docking colors and styles based on theme
    ImGuiStyle& style = ImGui::GetStyle();

    // Note: Docking-specific colors may not be available
    if (theme == "Dark") {
        // style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
        // style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.4f, 0.4f, 0.9f, 0.7f);
    } else if (theme == "Light") {
        // style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
        // style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.2f, 0.2f, 0.8f, 0.7f);
    }
}

void DockingManager::CustomizeDockingStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    // Customize docking-related styling
    style.WindowMenuButtonPosition = ImGuiDir_Right;
    style.TabRounding = 4.0f;
    style.TabBorderSize = 0.0f;
    style.SeparatorTextBorderSize = 1.0f;
}

void DockingManager::RenderWindows() {
    for (auto& window : windows) {
        if (!window->isOpen || !window->isVisible) {
            continue;
        }

        // Setup window
        ImGui::SetNextWindowSizeConstraints(window->minSize, window->maxSize);

        bool isOpen = window->isOpen;
        if (ImGui::Begin(window->title.c_str(), &isOpen, window->flags)) {
            // Check if window gained focus
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
                if (windowFocusCallback) {
                    windowFocusCallback(window->id);
                }
            }

            // Render window content
            if (window->renderFunc) {
                window->renderFunc();
            }
        }
        ImGui::End();

        // Handle window close
        if (!isOpen) {
            window->isOpen = false;
            if (windowCloseCallback) {
                windowCloseCallback(window->id);
            }
        }
    }
}

void DockingManager::SetupDefaultLayout() {
    if (mainDockSpaceID == 0) return;

    // Note: DockBuilder functions may not be available in all ImGui versions
    // Clear existing layout
    // ImGui::DockBuilderRemoveNode(mainDockSpaceID);
    // ImGui::DockBuilderAddNode(mainDockSpaceID, ImGuiDockNodeFlags_DockSpace);
    // ImGui::DockBuilderSetNodeSize(mainDockSpaceID, ImGui::GetMainViewport()->Size);

    // Split the dockspace
    // ImGuiID dock_left = ImGui::DockBuilderSplitNode(mainDockSpaceID, ImGuiDir_Left, 0.2f, nullptr, &mainDockSpaceID);
    // ImGuiID dock_right = ImGui::DockBuilderSplitNode(mainDockSpaceID, ImGuiDir_Right, 0.25f, nullptr, &mainDockSpaceID);
    // ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(mainDockSpaceID, ImGuiDir_Down, 0.3f, nullptr, &mainDockSpaceID);

    // Dock windows
    // ImGui::DockBuilderDockWindow(OUTLINER_WINDOW, dock_left);
    // ImGui::DockBuilderDockWindow(INSPECTOR_WINDOW, dock_right);
    // ImGui::DockBuilderDockWindow(CONTENT_BROWSER_WINDOW, dock_bottom);
    // ImGui::DockBuilderDockWindow(CONSOLE_WINDOW, dock_bottom);
    // ImGui::DockBuilderDockWindow(VIEWPORT_WINDOW, mainDockSpaceID);

    // ImGui::DockBuilderFinish(mainDockSpaceID);
}

void DockingManager::HandleDockingOperations() {
    // Handle any pending docking operations
    for (auto& window : windows) {
        if (!window->preferredDockTarget.empty() && window->preferredDockDir != ImGuiDir_None) {
            // Find target window's dock node and dock to it
            // This would require more advanced ImGui docking API usage
            // For now, we'll clear the preference
            window->preferredDockTarget = "";
            window->preferredDockDir = ImGuiDir_None;
        }
    }
}

void DockingManager::UpdateWindowStates() {
    // Update any window state changes
    for (auto& window : windows) {
        // Window states are updated in RenderWindows()
    }
}

std::string DockingManager::GetLayoutFilePath(const std::string& name) const {
    return configDirectory + name + ".layout";
}

void DockingManager::SaveLayoutToFile(const std::string& name, const std::string& iniData) {
    std::string filepath = GetLayoutFilePath(name);
    std::ofstream file(filepath);
    if (file.is_open()) {
        file << iniData;
        file.close();
    }
}

std::string DockingManager::LoadLayoutFromFile(const std::string& name) {
    std::string filepath = GetLayoutFilePath(name);
    std::ifstream file(filepath);
    std::string iniData;

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            iniData += line + "\n";
        }
        file.close();
    }

    return iniData;
}
