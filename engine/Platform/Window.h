#pragma once
#include <string>
struct SDL_Window;

namespace Sprout
{
    class Window {
    public:
        Window(const char* title, int width, int height);
        ~Window();

        SDL_Window* native() const { return m_window; }
        int width()  const { return m_width; }
        int height() const { return m_height; }

    private:
        SDL_Window* m_window = nullptr;
        int m_width = 0;
        int m_height = 0;
    };
}
