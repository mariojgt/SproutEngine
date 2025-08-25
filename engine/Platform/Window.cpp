#include "Window.h"
#include <SDL.h>
#include <SDL_video.h>
#include <iostream>

using namespace Sprout;

Window::Window(const char* title, int width, int height)
    : m_width(width), m_height(height)
{
    Uint32 flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
#if defined(__APPLE__)
    // Allow high-DPI on macOS
    flags |= SDL_WINDOW_METAL;
#endif
    m_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
    if (!m_window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\\n";
    }
}

Window::~Window()
{
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
}
