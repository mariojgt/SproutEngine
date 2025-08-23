#include "SceneManipulator.h"
#include "Components.h"
#include "Renderer.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <algorithm>
#include <cmath>

SceneManipulator::SceneManipulator() {
}

SceneManipulator::~SceneManipulator() {
    Shutdown();
}

void SceneManipulator::Init() {
    // Initialize any resources needed for scene manipulation
}

void SceneManipulator::Shutdown() {
    // Cleanup resources
}

void SceneManipulator::Update(float deltaTime) {
    // Update any time-based animations or interpolations
}

void SceneManipulator::Render(entt::registry& registry, Renderer& renderer,
                             const glm::mat4& view, const glm::mat4& projection) {
    // Render grid
    if (showGrid) {
        RenderGrid(view, projection);
    }

    // Render selection outlines
    RenderSelectionOutline(registry, view, projection);

    // Render gizmo for selected entity
    if (selectedEntity != entt::null && showGizmo) {
        auto* transform = registry.try_get<Transform>(selectedEntity);
        if (transform) {
            RenderGizmo(transform->position, view, projection);
        }
    }

    // Render selection box if active
    if (isBoxSelecting) {
        RenderSelectionBox();
    }
}

bool SceneManipulator::HandleMouseInput(const glm::vec2& mousePos, const glm::vec2& viewportSize,
                                       bool isPressed, bool isReleased, entt::registry& registry) {
    bool handled = false;

    if (isPressed) {
        // Check if we're clicking on a gizmo
        if (selectedEntity != entt::null && showGizmo) {
            auto* transform = registry.try_get<Transform>(selectedEntity);
            if (transform) {
                glm::mat4 viewProj = glm::mat4(1.0f); // Should be passed from caller
                hoveredAxis = GetHoveredGizmoAxis(mousePos, transform->position, glm::mat4(1.0f), viewProj);

                if (hoveredAxis != GizmoAxis::None) {
                    selectedAxis = hoveredAxis;
                    isDragging = true;
                    dragStartPosition = transform->position;
                    dragCurrentPosition = transform->position;
                    lastMousePos = mousePos;
                    isGizmoActive = true;
                    handled = true;
                }
            }
        }

        // If not clicking on gizmo, try entity picking
        if (!handled) {
            glm::mat4 view = glm::mat4(1.0f);      // Should be passed from caller
            glm::mat4 projection = glm::mat4(1.0f); // Should be passed from caller
            Ray ray = ScreenToWorldRay(mousePos, viewportSize, view, projection);
            entt::entity pickedEntity = PickEntity(ray, registry);

            if (pickedEntity != entt::null) {
                // Entity selection
                if (ImGui::GetIO().KeyCtrl) {
                    // Multi-selection
                    auto it = std::find(selectedEntities.begin(), selectedEntities.end(), pickedEntity);
                    if (it != selectedEntities.end()) {
                        RemoveFromSelection(pickedEntity);
                    } else {
                        AddToSelection(pickedEntity);
                    }
                } else {
                    // Single selection
                    SetSelectedEntity(pickedEntity);
                    ClearSelection();
                    AddToSelection(pickedEntity);
                }
                handled = true;
            } else {
                // Start box selection
                if (!ImGui::GetIO().KeyCtrl) {
                    ClearSelection();
                    SetSelectedEntity(entt::null);
                }

                isBoxSelecting = true;
                boxSelectStart = mousePos;
                boxSelectEnd = mousePos;
            }
        }
    }

    if (isDragging && selectedEntity != entt::null) {
        // Handle gizmo dragging
        glm::vec2 mouseDelta = mousePos - lastMousePos;
        HandleGizmoInteraction(mousePos, mouseDelta, registry);
        lastMousePos = mousePos;
        handled = true;
    }

    if (isBoxSelecting) {
        boxSelectEnd = mousePos;
        handled = true;
    }

    if (isReleased) {
        if (isDragging) {
            isDragging = false;
            isGizmoActive = false;
            selectedAxis = GizmoAxis::None;
            handled = true;
        }

        if (isBoxSelecting) {
            // Perform box selection
            // TODO: Implement box selection logic
            isBoxSelecting = false;
            handled = true;
        }
    }

    return handled;
}

void SceneManipulator::DeleteEntity(entt::registry& registry, entt::entity entity) {
    if (entity == entt::null || !registry.valid(entity)) {
        return;
    }

    // Remove from selection if selected
    RemoveFromSelection(entity);
    if (selectedEntity == entity) {
        selectedEntity = entt::null;
    }

    // Destroy the entity
    registry.destroy(entity);

    if (selectionChangedCallback) {
        selectionChangedCallback(selectedEntity);
    }
}

entt::entity SceneManipulator::DuplicateEntity(entt::registry& registry, entt::entity entity) {
    if (entity == entt::null || !registry.valid(entity)) {
        return entt::null;
    }

    entt::entity newEntity = registry.create();

    // Copy transform component if it exists
    if (auto* transform = registry.try_get<Transform>(entity)) {
        auto& newTransform = registry.emplace<Transform>(newEntity);
        newTransform = *transform;
        // Offset position slightly so it's visible
        newTransform.position += glm::vec3(1.0f, 0.0f, 0.0f);
    }

    // Copy mesh component if it exists
    if (auto* mesh = registry.try_get<MeshCube>(entity)) {
        auto& newMesh = registry.emplace<MeshCube>(newEntity);
        newMesh = *mesh;
    }

    if (auto* sphere = registry.try_get<MeshSphere>(entity)) {
        auto& newSphere = registry.emplace<MeshSphere>(newEntity);
        newSphere = *sphere;
    }

    // Copy name component if it exists
    if (auto* name = registry.try_get<NameComponent>(entity)) {
        auto& newName = registry.emplace<NameComponent>(newEntity);
        newName.name = name->name + "_Copy";
    }

    return newEntity;
}

void SceneManipulator::MoveEntity(entt::registry& registry, entt::entity entity, const glm::vec3& newPosition) {
    if (entity == entt::null || !registry.valid(entity)) {
        return;
    }

    auto* transform = registry.try_get<Transform>(entity);
    if (transform) {
        transform->position = enableSnapping ? SnapToGrid(newPosition) : newPosition;
    }
}

void SceneManipulator::RotateEntity(entt::registry& registry, entt::entity entity, const glm::vec3& rotation) {
    if (entity == entt::null || !registry.valid(entity)) {
        return;
    }

    auto* transform = registry.try_get<Transform>(entity);
    if (transform) {
        transform->rotation = rotation;
    }
}

void SceneManipulator::ScaleEntity(entt::registry& registry, entt::entity entity, const glm::vec3& scale) {
    if (entity == entt::null || !registry.valid(entity)) {
        return;
    }

    auto* transform = registry.try_get<Transform>(entity);
    if (transform) {
        transform->scale = scale;
    }
}

void SceneManipulator::AddToSelection(entt::entity entity) {
    if (entity == entt::null) return;

    auto it = std::find(selectedEntities.begin(), selectedEntities.end(), entity);
    if (it == selectedEntities.end()) {
        selectedEntities.push_back(entity);
    }
}

void SceneManipulator::RemoveFromSelection(entt::entity entity) {
    auto it = std::find(selectedEntities.begin(), selectedEntities.end(), entity);
    if (it != selectedEntities.end()) {
        selectedEntities.erase(it);
    }
}

void SceneManipulator::ClearSelection() {
    selectedEntities.clear();
}

SceneManipulator::Ray SceneManipulator::ScreenToWorldRay(const glm::vec2& screenPos, const glm::vec2& viewportSize,
                                                        const glm::mat4& view, const glm::mat4& projection) {
    // Convert screen coordinates to NDC
    glm::vec2 ndc = glm::vec2(
        (2.0f * screenPos.x) / viewportSize.x - 1.0f,
        1.0f - (2.0f * screenPos.y) / viewportSize.y
    );

    // Create ray in clip space
    glm::vec4 rayClip = glm::vec4(ndc.x, ndc.y, -1.0f, 1.0f);

    // Transform to eye space
    glm::mat4 invProjection = glm::inverse(projection);
    glm::vec4 rayEye = invProjection * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // Transform to world space
    glm::mat4 invView = glm::inverse(view);
    glm::vec4 rayWorld = invView * rayEye;

    Ray ray;
    ray.direction = glm::normalize(glm::vec3(rayWorld));

    // Extract camera position from view matrix
    glm::vec3 cameraPos = glm::vec3(invView[3]);
    ray.origin = cameraPos;

    return ray;
}

entt::entity SceneManipulator::PickEntity(const Ray& ray, entt::registry& registry) {
    float closestDistance = std::numeric_limits<float>::max();
    entt::entity closestEntity = entt::null;

    auto view = registry.view<Transform>();
    for (auto entity : view) {
        auto& transform = view.get<Transform>(entity);

        // Simple sphere intersection test (can be improved with actual mesh bounds)
        float distance;
        float radius = 1.0f; // Default radius, should be calculated from mesh bounds

        if (RayIntersectSphere(ray, transform.position, radius, distance)) {
            if (distance < closestDistance) {
                closestDistance = distance;
                closestEntity = entity;
            }
        }
    }

    return closestEntity;
}

bool SceneManipulator::RayIntersectAABB(const Ray& ray, const glm::vec3& min, const glm::vec3& max, float& distance) {
    glm::vec3 invDir = 1.0f / ray.direction;
    glm::vec3 t1 = (min - ray.origin) * invDir;
    glm::vec3 t2 = (max - ray.origin) * invDir;

    glm::vec3 tMin = glm::min(t1, t2);
    glm::vec3 tMax = glm::max(t1, t2);

    float tNear = glm::max(glm::max(tMin.x, tMin.y), tMin.z);
    float tFar = glm::min(glm::min(tMax.x, tMax.y), tMax.z);

    if (tNear > tFar || tFar < 0.0f) {
        return false;
    }

    distance = tNear > 0.0f ? tNear : tFar;
    return true;
}

bool SceneManipulator::RayIntersectSphere(const Ray& ray, const glm::vec3& center, float radius, float& distance) {
    glm::vec3 oc = ray.origin - center;
    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0f * glm::dot(oc, ray.direction);
    float c = glm::dot(oc, oc) - radius * radius;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        return false;
    }

    float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
    float t2 = (-b + sqrt(discriminant)) / (2.0f * a);

    distance = (t1 > 0) ? t1 : t2;
    return distance > 0;
}

void SceneManipulator::RenderGizmo(const glm::vec3& position, const glm::mat4& view, const glm::mat4& projection) {
    switch (currentTool) {
        case Tool::Move:
            RenderMoveGizmo(position, view, projection);
            break;
        case Tool::Rotate:
            RenderRotateGizmo(position, view, projection);
            break;
        case Tool::Scale:
            RenderScaleGizmo(position, view, projection);
            break;
        default:
            break;
    }
}

void SceneManipulator::RenderMoveGizmo(const glm::vec3& position, const glm::mat4& view, const glm::mat4& projection) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 viewportSizeImGui = ImGui::GetWindowSize();
    glm::vec2 viewportSize = glm::vec2(viewportSizeImGui.x, viewportSizeImGui.y);

    // Convert world position to screen space
    glm::vec3 screenPos = WorldToScreen(position, view * projection, viewportSize);

    if (screenPos.z > 1.0f) return; // Behind camera

    ImVec2 center = ImVec2(screenPos.x, screenPos.y);

    // Draw X axis (red)
    ImVec2 xEnd = ImVec2(center.x + gizmoSize, center.y);
    drawList->AddLine(center, xEnd, GetAxisColor(GizmoAxis::X, hoveredAxis == GizmoAxis::X), gizmoLineWidth);
    drawList->AddTriangleFilled(
        ImVec2(xEnd.x, xEnd.y - gizmoArrowSize/2),
        ImVec2(xEnd.x, xEnd.y + gizmoArrowSize/2),
        ImVec2(xEnd.x + gizmoArrowSize, xEnd.y),
        GetAxisColor(GizmoAxis::X, hoveredAxis == GizmoAxis::X)
    );

    // Draw Y axis (green)
    ImVec2 yEnd = ImVec2(center.x, center.y - gizmoSize);
    drawList->AddLine(center, yEnd, GetAxisColor(GizmoAxis::Y, hoveredAxis == GizmoAxis::Y), gizmoLineWidth);
    drawList->AddTriangleFilled(
        ImVec2(yEnd.x - gizmoArrowSize/2, yEnd.y),
        ImVec2(yEnd.x + gizmoArrowSize/2, yEnd.y),
        ImVec2(yEnd.x, yEnd.y - gizmoArrowSize),
        GetAxisColor(GizmoAxis::Y, hoveredAxis == GizmoAxis::Y)
    );

    // Draw Z axis (blue) - this would need proper 3D projection
    // For now, just draw at a 45-degree angle
    ImVec2 zEnd = ImVec2(center.x - gizmoSize * 0.7f, center.y + gizmoSize * 0.7f);
    drawList->AddLine(center, zEnd, GetAxisColor(GizmoAxis::Z, hoveredAxis == GizmoAxis::Z), gizmoLineWidth);
    drawList->AddTriangleFilled(
        ImVec2(zEnd.x - gizmoArrowSize/2, zEnd.y),
        ImVec2(zEnd.x + gizmoArrowSize/2, zEnd.y),
        ImVec2(zEnd.x, zEnd.y + gizmoArrowSize),
        GetAxisColor(GizmoAxis::Z, hoveredAxis == GizmoAxis::Z)
    );
}

void SceneManipulator::RenderRotateGizmo(const glm::vec3& position, const glm::mat4& view, const glm::mat4& projection) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 viewportSizeImGui = ImGui::GetWindowSize();
    glm::vec2 viewportSize = glm::vec2(viewportSizeImGui.x, viewportSizeImGui.y);

    // Convert world position to screen space
    glm::vec3 screenPos = WorldToScreen(position, view * projection, viewportSize);

    if (screenPos.z > 1.0f) return; // Behind camera

    ImVec2 center = ImVec2(screenPos.x, screenPos.y);

    // Draw rotation circles for each axis
    drawList->AddCircle(center, gizmoSize, GetAxisColor(GizmoAxis::X, hoveredAxis == GizmoAxis::X), 32, gizmoLineWidth);
    drawList->AddCircle(center, gizmoSize * 0.8f, GetAxisColor(GizmoAxis::Y, hoveredAxis == GizmoAxis::Y), 32, gizmoLineWidth);
    drawList->AddCircle(center, gizmoSize * 0.6f, GetAxisColor(GizmoAxis::Z, hoveredAxis == GizmoAxis::Z), 32, gizmoLineWidth);
}

void SceneManipulator::RenderScaleGizmo(const glm::vec3& position, const glm::mat4& view, const glm::mat4& projection) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 viewportSizeImGui = ImGui::GetWindowSize();
    glm::vec2 viewportSize = glm::vec2(viewportSizeImGui.x, viewportSizeImGui.y);

    // Convert world position to screen space
    glm::vec3 screenPos = WorldToScreen(position, view * projection, viewportSize);

    if (screenPos.z > 1.0f) return; // Behind camera

    ImVec2 center = ImVec2(screenPos.x, screenPos.y);

    // Draw scale handles (similar to move gizmo but with boxes at the ends)
    float boxSize = gizmoArrowSize;

    // X axis
    ImVec2 xEnd = ImVec2(center.x + gizmoSize, center.y);
    drawList->AddLine(center, xEnd, GetAxisColor(GizmoAxis::X, hoveredAxis == GizmoAxis::X), gizmoLineWidth);
    drawList->AddRectFilled(
        ImVec2(xEnd.x - boxSize/2, xEnd.y - boxSize/2),
        ImVec2(xEnd.x + boxSize/2, xEnd.y + boxSize/2),
        GetAxisColor(GizmoAxis::X, hoveredAxis == GizmoAxis::X)
    );

    // Y axis
    ImVec2 yEnd = ImVec2(center.x, center.y - gizmoSize);
    drawList->AddLine(center, yEnd, GetAxisColor(GizmoAxis::Y, hoveredAxis == GizmoAxis::Y), gizmoLineWidth);
    drawList->AddRectFilled(
        ImVec2(yEnd.x - boxSize/2, yEnd.y - boxSize/2),
        ImVec2(yEnd.x + boxSize/2, yEnd.y + boxSize/2),
        GetAxisColor(GizmoAxis::Y, hoveredAxis == GizmoAxis::Y)
    );

    // Z axis
    ImVec2 zEnd = ImVec2(center.x - gizmoSize * 0.7f, center.y + gizmoSize * 0.7f);
    drawList->AddLine(center, zEnd, GetAxisColor(GizmoAxis::Z, hoveredAxis == GizmoAxis::Z), gizmoLineWidth);
    drawList->AddRectFilled(
        ImVec2(zEnd.x - boxSize/2, zEnd.y - boxSize/2),
        ImVec2(zEnd.x + boxSize/2, zEnd.y + boxSize/2),
        GetAxisColor(GizmoAxis::Z, hoveredAxis == GizmoAxis::Z)
    );
}

SceneManipulator::GizmoAxis SceneManipulator::GetHoveredGizmoAxis(const glm::vec2& mousePos, const glm::vec3& gizmoPos,
                                                                 const glm::mat4& view, const glm::mat4& projection) {
    // This is a simplified version - in a real implementation you'd do proper 3D hit testing
    ImVec2 viewportSizeImGui = ImGui::GetWindowSize();
    glm::vec2 viewportSize(viewportSizeImGui.x, viewportSizeImGui.y);
    glm::vec3 screenPos = WorldToScreen(gizmoPos, view * projection, viewportSize);

    if (screenPos.z > 1.0f) return GizmoAxis::None;

    glm::vec2 center = glm::vec2(screenPos.x, screenPos.y);
    glm::vec2 diff = mousePos - center;
    float distance = glm::length(diff);

    if (distance > gizmoSize + 20.0f) return GizmoAxis::None;

    // Simple angle-based detection
    float angle = atan2(diff.y, diff.x);
    angle = angle * 180.0f / M_PI;
    if (angle < 0) angle += 360.0f;

    // X axis (horizontal right)
    if ((angle >= 350.0f || angle <= 10.0f) && distance >= gizmoSize * 0.8f) {
        return GizmoAxis::X;
    }

    // Y axis (vertical up)
    if (angle >= 80.0f && angle <= 100.0f && distance >= gizmoSize * 0.8f) {
        return GizmoAxis::Y;
    }

    // Z axis (diagonal)
    if (angle >= 225.0f && angle <= 235.0f && distance >= gizmoSize * 0.8f) {
        return GizmoAxis::Z;
    }

    return GizmoAxis::None;
}

void SceneManipulator::HandleGizmoInteraction(const glm::vec2& mousePos, const glm::vec2& mouseDelta,
                                             entt::registry& registry) {
    if (selectedEntity == entt::null || selectedAxis == GizmoAxis::None) {
        return;
    }

    auto* transform = registry.try_get<Transform>(selectedEntity);
    if (!transform) return;

    switch (currentTool) {
        case Tool::Move: {
            glm::vec3 delta = glm::vec3(0.0f);
            float sensitivity = 0.01f;

            switch (selectedAxis) {
                case GizmoAxis::X:
                    delta.x = mouseDelta.x * sensitivity;
                    break;
                case GizmoAxis::Y:
                    delta.y = -mouseDelta.y * sensitivity;
                    break;
                case GizmoAxis::Z:
                    delta.z = mouseDelta.y * sensitivity;
                    break;
                default:
                    break;
            }

            transform->position += delta;
            break;
        }

        case Tool::Rotate: {
            glm::vec3 rotation = glm::vec3(0.0f);
            float sensitivity = 0.01f;

            switch (selectedAxis) {
                case GizmoAxis::X:
                    rotation.x = mouseDelta.y * sensitivity;
                    break;
                case GizmoAxis::Y:
                    rotation.y = mouseDelta.x * sensitivity;
                    break;
                case GizmoAxis::Z:
                    rotation.z = mouseDelta.x * sensitivity;
                    break;
                default:
                    break;
            }

            transform->rotation += rotation;
            break;
        }

        case Tool::Scale: {
            glm::vec3 scale = glm::vec3(0.0f);
            float sensitivity = 0.01f;
            float scaleChange = (mouseDelta.x + mouseDelta.y) * sensitivity;

            switch (selectedAxis) {
                case GizmoAxis::X:
                    scale.x = scaleChange;
                    break;
                case GizmoAxis::Y:
                    scale.y = scaleChange;
                    break;
                case GizmoAxis::Z:
                    scale.z = scaleChange;
                    break;
                default:
                    break;
            }

            transform->scale += scale;
            transform->scale = glm::max(transform->scale, glm::vec3(0.01f)); // Prevent negative scale
            break;
        }

        default:
            break;
    }
}

void SceneManipulator::RenderGrid(const glm::mat4& view, const glm::mat4& projection) {
    // Grid rendering would be done through the actual renderer
    // This is a placeholder for the grid rendering logic
}

void SceneManipulator::RenderSelectionOutline(entt::registry& registry, const glm::mat4& view, const glm::mat4& projection) {
    // Selection outline rendering would be done through the actual renderer
    // This could use a post-processing effect or render wireframe overlay
}

void SceneManipulator::RenderSelectionBox() {
    if (!isBoxSelecting) return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    glm::vec2 min = glm::min(boxSelectStart, boxSelectEnd);
    glm::vec2 max = glm::max(boxSelectStart, boxSelectEnd);

    drawList->AddRect(
        ImVec2(min.x, min.y),
        ImVec2(max.x, max.y),
        IM_COL32(100, 150, 255, 128),
        0.0f,
        0,
        2.0f
    );

    drawList->AddRectFilled(
        ImVec2(min.x, min.y),
        ImVec2(max.x, max.y),
        IM_COL32(100, 150, 255, 32)
    );
}

glm::vec3 SceneManipulator::SnapToGrid(const glm::vec3& position) {
    if (!enableSnapping) return position;

    return glm::vec3(
        round(position.x / snapIncrement) * snapIncrement,
        round(position.y / snapIncrement) * snapIncrement,
        round(position.z / snapIncrement) * snapIncrement
    );
}

glm::vec3 SceneManipulator::WorldToScreen(const glm::vec3& worldPos, const glm::mat4& viewProj, const glm::vec2& viewportSize) {
    glm::vec4 clipPos = viewProj * glm::vec4(worldPos, 1.0f);

    if (clipPos.w <= 0.0f) {
        return glm::vec3(0.0f, 0.0f, 2.0f); // Behind camera
    }

    glm::vec3 ndc = glm::vec3(clipPos) / clipPos.w;

    return glm::vec3(
        (ndc.x + 1.0f) * 0.5f * viewportSize.x,
        (1.0f - ndc.y) * 0.5f * viewportSize.y,
        ndc.z
    );
}

glm::vec3 SceneManipulator::ScreenToWorld(const glm::vec2& screenPos, float depth, const glm::mat4& invViewProj, const glm::vec2& viewportSize) {
    glm::vec2 ndc = glm::vec2(
        (2.0f * screenPos.x) / viewportSize.x - 1.0f,
        1.0f - (2.0f * screenPos.y) / viewportSize.y
    );

    glm::vec4 worldPos = invViewProj * glm::vec4(ndc, depth, 1.0f);
    return glm::vec3(worldPos) / worldPos.w;
}

ImU32 SceneManipulator::GetAxisColor(GizmoAxis axis, bool isHovered, bool isSelected) {
    if (isSelected) return COLOR_SELECTED;
    if (isHovered) return COLOR_HOVERED;

    switch (axis) {
        case GizmoAxis::X: return COLOR_X_AXIS;
        case GizmoAxis::Y: return COLOR_Y_AXIS;
        case GizmoAxis::Z: return COLOR_Z_AXIS;
        default: return IM_COL32(128, 128, 128, 255);
    }
}
