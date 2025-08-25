#include "Application.h"
#include "../Platform/Window.h"
#include "../Render/Renderer.h"
#include "../Scene/Scene.h"
#include "../ECS/Systems.h"
#include "../ECS/Components.h"
#include "../Scripting/ScriptSystem.h"

#include <SDL.h>
#include <iostream>
#include <cmath>

using namespace Sprout;

Application::Application(const AppConfig& cfg) : m_cfg(cfg) {}

Application::~Application() = default;

void Application::handleEvents()
{
    SDL_Event e;
    m_mouseDx = m_mouseDy = 0;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            m_running = false;
        }
        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            if (m_renderer) {
                m_renderer->resize(e.window.data1, e.window.data2);
            }
        }
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {
            m_mouseCaptured = true;
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_RIGHT) {
            m_mouseCaptured = false;
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
        if (e.type == SDL_MOUSEMOTION && m_mouseCaptured) {
            m_mouseDx += e.motion.xrel;
            m_mouseDy += e.motion.yrel;
        }
        if (e.type == SDL_MOUSEWHEEL) {
            m_moveSpeed = std::max(0.5f, m_moveSpeed + (e.wheel.y > 0 ? 0.5f : -0.5f));
        }
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            if (m_mouseCaptured) {
                m_mouseCaptured = false;
                SDL_SetRelativeMouseMode(SDL_FALSE);
            } else {
                m_running = false;
            }
        }
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F2) {
            // Open default editor for a known script for demo (can be expanded to current selection)
#if defined(_WIN32)
            system("start scripts\\rotator.lua");
#elif defined(__APPLE__)
            system("open scripts/rotator.lua");
#else
            system("xdg-open scripts/rotator.lua");
#endif
        }
    }
}

void Application::handleInput(float dt)
{
    // Move the primary camera with WASD + QE
    const Uint8* ks = SDL_GetKeyboardState(nullptr);
    float forward = (ks[SDL_SCANCODE_W] ? 1.f : 0.f) - (ks[SDL_SCANCODE_S] ? 1.f : 0.f);
    float right   = (ks[SDL_SCANCODE_D] ? 1.f : 0.f) - (ks[SDL_SCANCODE_A] ? 1.f : 0.f);
    float up      = (ks[SDL_SCANCODE_E] ? 1.f : 0.f) - (ks[SDL_SCANCODE_Q] ? 1.f : 0.f);

    // Access scene through a static (demo); better would be app-level member. We'll keep local in run().
    (void)dt;
}

int Application::run()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\\n";
        return 1;
    }

    m_window  = std::make_unique<Window>(m_cfg.title.c_str(), m_cfg.width, m_cfg.height);
    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->init(*m_window, m_cfg.vsync)) {
        std::cerr << "Renderer init failed.\\n";
        return 2;
    }

    // Create a scene with a primary camera and load the sample glTF
    Sprout::SceneSys::Scene scene;
    auto camEntity = scene.createCameraPrimary(60.0f, 0.1f, 200.0f);
    // Light
    scene.registry().emplace<ECS::DirectionalLight>(camEntity, ECS::DirectionalLight{ {-0.3f, -1.0f, -0.2f}, {1.0f, 1.0f, 1.0f}, 1.0f });

    // Load sample mesh
    auto meshEnt = scene.createMeshFromFile("assets/Box.gltf");
    (void)meshEnt;

    // Attach a demo script to the mesh
    if (meshEnt != entt::null) {
        scene.registry().emplace<Scripting::Script>(meshEnt, Scripting::Script{ "scripts/rotator.lua" });
    }

    // Timing
    uint64_t prev = SDL_GetPerformanceCounter();
    double freq = (double)SDL_GetPerformanceFrequency();

    while (m_running) {
        uint64_t now = SDL_GetPerformanceCounter();
        float dt = float((now - prev) / freq);
        prev = now;

        handleEvents();

        // Fly camera input: adjust primary camera transform
        // Find primary camera
        auto view = scene.registry().view<ECS::Camera, ECS::Transform>();
        for (auto e : view) {
            auto& cam = view.get<ECS::Camera>(e);
            auto& tr  = view.get<ECS::Transform>(e);
            if (!cam.primary) continue;

            const Uint8* ks = SDL_GetKeyboardState(nullptr);
            float f = (ks[SDL_SCANCODE_W] ? 1.f : 0.f) - (ks[SDL_SCANCODE_S] ? 1.f : 0.f);
            float r = (ks[SDL_SCANCODE_D] ? 1.f : 0.f) - (ks[SDL_SCANCODE_A] ? 1.f : 0.f);
            float u = (ks[SDL_SCANCODE_E] ? 1.f : 0.f) - (ks[SDL_SCANCODE_Q] ? 1.f : 0.f);

            // Mouse look if captured
            if (m_mouseCaptured) {
                float yawDeg   = -m_mouseDx * m_lookSensitivity;
                float pitchDeg = -m_mouseDy * m_lookSensitivity;

                // Apply yaw around world up, pitch around camera right
                glm::vec3 worldUp(0,1,0);
                glm::vec3 right = tr.rotation * glm::vec3(1,0,0);

                tr.rotation = glm::angleAxis(glm::radians(yawDeg), worldUp) * tr.rotation;
                tr.rotation = glm::angleAxis(glm::radians(pitchDeg), right) * tr.rotation;
            }

            // Move in local space
            glm::vec3 forward = tr.rotation * glm::vec3(0,0,-1);
            glm::vec3 rightv  = tr.rotation * glm::vec3(1,0,0);
            glm::vec3 upv     = glm::vec3(0,1,0);

            tr.position += (forward * f + rightv * r + upv * u) * (m_moveSpeed * dt);
            break;
        }

        // Scripting
        Scripting::RunScripts(scene.registry(), dt);

        // Update cameras (compute view/proj)
        ECS::UpdateCameras(scene.registry(), m_renderer->width(), m_renderer->height());

        // Render
        ECS::Render(scene.registry(), *m_renderer);

        m_renderer->frame();
    }

    m_renderer->shutdown();
    m_renderer.reset();
    m_window.reset();

    SDL_Quit();
    return 0;
}
