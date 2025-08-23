#pragma once
#include <imgui.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <functional>

class Renderer;

/**
 * Scene Manipulation System
 * Handles 3D gizmos, entity picking, drag & drop, and scene editing
 */
class SceneManipulator {
public:
    SceneManipulator();
    ~SceneManipulator();

    void Init();
    void Shutdown();
    void Update(float deltaTime);
    void Render(entt::registry& registry, Renderer& renderer, const glm::mat4& view, const glm::mat4& projection);

    // Tool selection
    enum class Tool {
        Select,
        Move,
        Rotate,
        Scale
    };

    void SetTool(Tool tool) { currentTool = tool; }
    Tool GetTool() const { return currentTool; }

    // Entity selection
    void SetSelectedEntity(entt::entity entity) { selectedEntity = entity; }
    entt::entity GetSelectedEntity() const { return selectedEntity; }

    // Viewport interaction
    bool HandleMouseInput(const glm::vec2& mousePos, const glm::vec2& viewportSize,
                         bool isPressed, bool isReleased, entt::registry& registry);

    // Gizmo operations
    bool IsGizmoActive() const { return isGizmoActive; }
    bool IsGizmoHovered() const { return isGizmoHovered; }

    // Scene operations
    void DeleteEntity(entt::registry& registry, entt::entity entity);
    entt::entity DuplicateEntity(entt::registry& registry, entt::entity entity);
    void MoveEntity(entt::registry& registry, entt::entity entity, const glm::vec3& newPosition);
    void RotateEntity(entt::registry& registry, entt::entity entity, const glm::vec3& rotation);
    void ScaleEntity(entt::registry& registry, entt::entity entity, const glm::vec3& scale);

    // Multi-selection
    void AddToSelection(entt::entity entity);
    void RemoveFromSelection(entt::entity entity);
    void ClearSelection();
    const std::vector<entt::entity>& GetSelectedEntities() const { return selectedEntities; }

    // Callbacks
    void SetSelectionChangedCallback(std::function<void(entt::entity)> callback) {
        selectionChangedCallback = callback;
    }

private:
    // Current state
    Tool currentTool = Tool::Select;
    entt::entity selectedEntity = entt::null;
    std::vector<entt::entity> selectedEntities;

    // Gizmo state
    bool isGizmoActive = false;
    bool isGizmoHovered = false;
    bool isDragging = false;
    glm::vec3 dragStartPosition;
    glm::vec3 dragCurrentPosition;
    glm::vec2 lastMousePos;

    // Gizmo components (axis selection)
    enum class GizmoAxis {
        None,
        X,
        Y,
        Z,
        XY,
        XZ,
        YZ,
        XYZ
    };

    GizmoAxis hoveredAxis = GizmoAxis::None;
    GizmoAxis selectedAxis = GizmoAxis::None;

    // Visual settings
    float gizmoSize = 80.0f;
    float gizmoLineWidth = 3.0f;
    float gizmoArrowSize = 12.0f;
    bool showGizmo = true;

    // Selection box
    bool isBoxSelecting = false;
    glm::vec2 boxSelectStart;
    glm::vec2 boxSelectEnd;

    // Grid settings
    bool showGrid = true;
    float gridSize = 1.0f;
    int gridLines = 100;

    // Snap settings
    bool enableSnapping = false;
    float snapIncrement = 1.0f;
    bool snapToGrid = false;
    bool snapToVertices = false;

    // Callbacks
    std::function<void(entt::entity)> selectionChangedCallback;

    // Ray casting for entity picking
    struct Ray {
        glm::vec3 origin;
        glm::vec3 direction;
    };

    Ray ScreenToWorldRay(const glm::vec2& screenPos, const glm::vec2& viewportSize,
                        const glm::mat4& view, const glm::mat4& projection);

    entt::entity PickEntity(const Ray& ray, entt::registry& registry);
    bool RayIntersectAABB(const Ray& ray, const glm::vec3& min, const glm::vec3& max, float& distance);
    bool RayIntersectSphere(const Ray& ray, const glm::vec3& center, float radius, float& distance);

    // Gizmo rendering
    void RenderGizmo(const glm::vec3& position, const glm::mat4& view, const glm::mat4& projection);
    void RenderMoveGizmo(const glm::vec3& position, const glm::mat4& view, const glm::mat4& projection);
    void RenderRotateGizmo(const glm::vec3& position, const glm::mat4& view, const glm::mat4& projection);
    void RenderScaleGizmo(const glm::vec3& position, const glm::mat4& view, const glm::mat4& projection);

    // Gizmo interaction
    GizmoAxis GetHoveredGizmoAxis(const glm::vec2& mousePos, const glm::vec3& gizmoPos,
                                 const glm::mat4& view, const glm::mat4& projection);

    void HandleGizmoInteraction(const glm::vec2& mousePos, const glm::vec2& mouseDelta,
                               entt::registry& registry);

    // Grid rendering
    void RenderGrid(const glm::mat4& view, const glm::mat4& projection);

    // Selection visualization
    void RenderSelectionOutline(entt::registry& registry, const glm::mat4& view, const glm::mat4& projection);
    void RenderSelectionBox();

    // Utility functions
    glm::vec3 SnapToGrid(const glm::vec3& position);
    glm::vec3 WorldToScreen(const glm::vec3& worldPos, const glm::mat4& viewProj, const glm::vec2& viewportSize);
    glm::vec3 ScreenToWorld(const glm::vec2& screenPos, float depth, const glm::mat4& invViewProj, const glm::vec2& viewportSize);

    // Colors
    ImU32 GetAxisColor(GizmoAxis axis, bool isHovered = false, bool isSelected = false);

    // Constants
    static constexpr ImU32 COLOR_X_AXIS = IM_COL32(255, 100, 100, 255);    // Red
    static constexpr ImU32 COLOR_Y_AXIS = IM_COL32(100, 255, 100, 255);    // Green
    static constexpr ImU32 COLOR_Z_AXIS = IM_COL32(100, 100, 255, 255);    // Blue
    static constexpr ImU32 COLOR_SELECTED = IM_COL32(255, 255, 100, 255);  // Yellow
    static constexpr ImU32 COLOR_HOVERED = IM_COL32(255, 255, 255, 255);   // White
};
