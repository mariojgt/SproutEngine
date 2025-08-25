#include "Renderer.h"
#include "../Platform/Window.h"

#include <SDL.h>
#include <iostream>

using namespace Sprout;

bool Renderer::init(Window& window, bool vsync)
{
    SDL_Window* sdlWin = window.native();
    if (!sdlWin) {
        std::cerr << "SDL Window is null!\n";
        return false;
    }

    int w = 0, h = 0;
    SDL_GetWindowSize(sdlWin, &w, &h);
    m_width = (uint32_t)w;
    m_height = (uint32_t)h;

    // For now, just create a simple software renderer to get things working
    m_renderer = SDL_CreateRenderer(sdlWin, -1, SDL_RENDERER_ACCELERATED | (vsync ? SDL_RENDERER_PRESENTVSYNC : 0));
    if (!m_renderer) {
        std::cerr << "Failed to create SDL renderer: " << SDL_GetError() << "\n";
        return false;
    }

    m_initialized = true;
    std::cout << "Basic SDL renderer initialized successfully!\n";
    return true;
}

void Renderer::resize(uint32_t w, uint32_t h)
{
    m_width = w;
    m_height = h;
    // SDL renderer handles resize automatically
}

void Renderer::frame()
{
    if (!m_initialized || !m_renderer) return;

    // Clear the screen with a dark grey color
    SDL_SetRenderDrawColor(m_renderer, 0x30, 0x30, 0x30, 0xFF);
    SDL_RenderClear(m_renderer);

    // Draw a simple colored rectangle as a placeholder
    SDL_SetRenderDrawColor(m_renderer, 0x60, 0x80, 0xFF, 0xFF);
    SDL_Rect rect = {100, 100, 200, 150};
    SDL_RenderFillRect(m_renderer, &rect);

    // Present the frame
    SDL_RenderPresent(m_renderer);
}

void Renderer::shutdown()
{
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    m_initialized = false;
}
