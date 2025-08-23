#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <string>
#include <memory>

struct GLFWwindow;
class Renderer;
class Scripting;
class ScriptEditor;
class BlueprintEditor;
class SceneManipulator;
class DockingManager;

class Editor {
public:
    bool init(GLFWwindow* window); // sets up ImGui + docking
    void shutdown(GLFWwindow* window);

    void drawDockspace();
    void drawPanels(entt::registry& reg, Renderer& renderer, Scripting& scripting, bool& playMode);

    // Advanced editing systems
    void initializeAdvancedSystems();
    void updateAdvancedSystems(float deltaTime);
    void renderAdvancedSystems(entt::registry& reg, Renderer& renderer);

    // Tool selection
    enum class Tool {
        Select,
        Move,
        Rotate,
        Scale
    };

    void setTool(Tool tool);
    Tool getCurrentTool() const { return currentTool; }

    // Selection management
    void setSelectedEntity(entt::entity entity);
    entt::entity getSelectedEntity() const { return selected; }

    // Event handling
    bool handleMouseInput(const glm::vec2& mousePos, const glm::vec2& viewportSize,
                         bool isPressed, bool isReleased, entt::registry& registry);

    // Script management
    std::vector<std::string> getAvailableScripts();
    std::vector<std::string> getAvailableBlueprints();

    entt::entity selected{entt::null};

private:
    // Advanced editing systems
    std::unique_ptr<ScriptEditor> scriptEditor;
    std::unique_ptr<BlueprintEditor> blueprintEditor;
    std::unique_ptr<SceneManipulator> sceneManipulator;
    std::unique_ptr<DockingManager> dockingManager;

    // Current tool
    Tool currentTool = Tool::Select;

    // Panel visibility
    bool showScriptEditor = false;
    bool showBlueprintEditor = false;
    bool showMaterialEditor = false;
    bool showAnimationEditor = false;
    bool showDebugger = false;
    bool showProfiler = false;

    // Render individual panels
    void renderViewportPanel(entt::registry& reg, Renderer& renderer);
    void renderWorldOutlinerPanel(entt::registry& reg);
    void renderInspectorPanel(entt::registry& reg);
    void renderContentBrowserPanel(entt::registry& reg);
    void renderConsolePanel();
    void renderScriptEditorPanel();
    void renderBlueprintEditorPanel();
    void renderMaterialEditorPanel();
    void renderAnimationEditorPanel();
    void renderDebuggerPanel();
    void renderProfilerPanel();

    // Toolbar and menus
    void renderMainMenuBar();
    void renderToolbar();
    void renderStatusBar();
};
