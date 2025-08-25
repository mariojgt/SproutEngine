#pragma once
#include <memory>
#include <string>

struct SDL_Window;

namespace Sprout
{
    class Window;
    class Renderer;

    struct AppConfig {
        std::string title = "Sprout Engine";
        int width  = 1280;
        int height = 720;
        bool vsync = true;
    };

    class Application {
    public:
        explicit Application(const AppConfig& cfg = {});
        ~Application();

        int run();

        // Non-copyable
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

    private:
        AppConfig m_cfg;
        std::unique_ptr<Window>   m_window;
        std::unique_ptr<Renderer> m_renderer;
        bool m_running = true;

        // input state
        bool m_mouseCaptured = false;
        int  m_mouseDx = 0, m_mouseDy = 0;
        float m_moveSpeed = 3.0f; // m/s
        float m_lookSensitivity = 0.1f; // deg per pixel

        void handleEvents();
        void handleInput(float dt);
    };
}
