#pragma once
#include <cstdint>

struct SDL_Window;
struct SDL_Renderer;

namespace Sprout
{
    class Window;

    class Renderer {
    public:
        bool init(Window& window, bool vsync);
        void resize(uint32_t w, uint32_t h);
        void frame();
        void shutdown();

        uint32_t width() const { return m_width; }
        uint32_t height() const { return m_height; }

    private:
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        bool m_initialized = false;
        SDL_Renderer* m_renderer = nullptr;
    };
}
