#include "Systems.h"
#include "Components.h"
#include "../Render/Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

using namespace Sprout::ECS;

void Sprout::ECS::UpdateCameras(entt::registry& reg, int viewportWidth, int viewportHeight)
{
    float aspect = (viewportHeight > 0) ? (float)viewportWidth / (float)viewportHeight : 1.0f;
    auto view = reg.view<Camera, Transform>();
    for (auto e : view) {
        auto& cam = view.get<Camera>(e);
        auto& tr  = view.get<Transform>(e);

        glm::vec3 forward = tr.rotation * glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 up      = tr.rotation * glm::vec3(0.0f, 1.0f,  0.0f);
        cam.view = glm::lookAt(tr.position, tr.position + forward, up);
        cam.proj = glm::perspective(glm::radians(cam.fovYDegrees), aspect, cam.nearPlane, cam.farPlane);
    }
}

void Sprout::ECS::Render(entt::registry& reg, Sprout::Renderer& renderer)
{
    // Camera
    glm::mat4 view(1.0f), proj(1.0f);
    bool hasCam = false;
    auto camView = reg.view<Camera, Transform>();
    for (auto e : camView) {
        auto& c = camView.get<Camera>(e);
        if (c.primary) {
            view = c.view; proj = c.proj;
            hasCam = true;
            break;
        }
    }
    if (!hasCam) return;
    renderer.setViewProj(view, proj);

    // Directional light (first one)
    glm::vec3 Ldir(-0.3f, -1.0f, -0.2f);
    glm::vec3 Lcol(1.0f);
    float Lint = 1.0f;

    auto lightView = reg.view<DirectionalLight>();
    for (auto e : lightView) {
        auto& l = lightView.get<DirectionalLight>(e);
        Ldir = l.direction;
        Lcol = l.color;
        Lint = l.intensity;
        break;
    }

    renderer.setDirectionalLight(Ldir, Lcol, Lint);

    // Draw meshes
    auto meshView = reg.view<Mesh, Transform>();
    for (auto e : meshView) {
        auto& m = meshView.get<Mesh>(e);
        auto& t = meshView.get<Transform>(e);

        const uint32_t vcount = (uint32_t)m.positions.size();
        if (vcount == 0 || m.indices.empty()) continue;

        const uint32_t stride = sizeof(float) * 8;
        std::vector<float> interleaved;
        interleaved.resize((size_t)vcount * 8);

        for (uint32_t i = 0; i < vcount; ++i) {
            const auto& p = m.positions[i];
            const auto  n = (i < m.normals.size()) ? m.normals[i] : glm::vec3(0,1,0);
            const auto  uv= (i < m.uvs.size())     ? m.uvs[i]     : glm::vec2(0,0);
            size_t base = (size_t)i * 8;
            interleaved[base+0] = p.x; interleaved[base+1] = p.y; interleaved[base+2] = p.z;
            interleaved[base+3] = n.x; interleaved[base+4] = n.y; interleaved[base+5] = n.z;
            interleaved[base+6] = uv.x; interleaved[base+7] = uv.y;
        }

        glm::mat4 model = t.localMatrix();

        renderer.drawLitMesh(interleaved.data(), vcount, stride,
                             m.indices.data(), (uint32_t)m.indices.size(),
                             model);
    }
}
