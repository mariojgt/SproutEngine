#pragma once
#include <cstdint>

struct SDL_Window;

namespace Sprout
{
    class Window;

    class Renderer {
    public:
        bool init(Window& window, bool vsync);
        void resize(uint32_t w, uint32_t h);
        void frame();
        void shutdown();
    private:
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        bool m_initialized = false;
    };
}
