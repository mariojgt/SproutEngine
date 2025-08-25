#pragma once
#include <memory>
#include <string>

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

        void handleEvents();
    };
}
