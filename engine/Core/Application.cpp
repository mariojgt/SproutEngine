#include "Application.h"
#include "../Platform/Window.h"
#include "../Render/Renderer.h"

#include <SDL.h>
#include <iostream>

using namespace Sprout;

Application::Application(const AppConfig& cfg) : m_cfg(cfg) {}

Application::~Application() = default;

void Application::handleEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            m_running = false;
        }
        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            if (m_renderer) {
                m_renderer->resize(e.window.data1, e.window.data2);
            }
        }
    }
}

int Application::run()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    m_window  = std::make_unique<Window>(m_cfg.title.c_str(), m_cfg.width, m_cfg.height);
    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->init(*m_window, m_cfg.vsync)) {
        std::cerr << "Renderer init failed.\n";
        return 2;
    }

    while (m_running) {
        handleEvents();
        m_renderer->frame();
    }

    m_renderer->shutdown();
    m_renderer.reset();
    m_window.reset();

    SDL_Quit();
    return 0;
}
