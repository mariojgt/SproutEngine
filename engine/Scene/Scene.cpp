#include "Scene.h"
#include "../ECS/Components.h"
#include "../Assets/GltfLoader.h"
#include <glm/gtc/quaternion.hpp>
#include <iostream>

using namespace Sprout::SceneSys;
using namespace Sprout;

entt::entity Scene::createEntity(const std::string& /*name*/)
{
    auto e = m_reg.create();
    // Name component could be added later
    return e;
}

entt::entity Scene::createCameraPrimary(float fovYDeg, float zNear, float zFar)
{
    auto e = createEntity("Camera");
    auto& t = m_reg.emplace<ECS::Transform>(e);
    t.position = {0.0f, 1.0f, 3.0f};
    t.rotation = glm::quat(glm::vec3(0.0f)); // identity
    auto& c = m_reg.emplace<ECS::Camera>(e);
    c.fovYDegrees = fovYDeg;
    c.nearPlane = zNear;
    c.farPlane = zFar;
    c.primary = true;
    return e;
}

entt::entity Scene::createMeshFromFile(const std::string& path)
{
    auto loaded = Assets::LoadFirstMeshFromGLTF(path.c_str());
    if (!loaded.has_value()) {
        std::cerr << "[Sprout] Failed to load mesh: " << path << "\\n";
        return entt::null;
    }

    auto e = createEntity("Mesh");
    auto& t = m_reg.emplace<ECS::Transform>(e);
    t.position = {0.0f, 0.0f, 0.0f};

    auto& m = m_reg.emplace<ECS::Mesh>(e);
    m.positions = std::move(loaded->positions);
    m.normals   = std::move(loaded->normals);
    m.uvs       = std::move(loaded->uvs);
    m.indices   = std::move(loaded->indices);
    m.debugName = path;

    std::cout << "[Sprout] Loaded glTF: " << path
              << " (vertices: " << m.positions.size()
              << ", indices: " << m.indices.size() << ")\\n";
    return e;
}
