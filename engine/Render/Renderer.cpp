#include "Renderer.h"
#include "../Platform/Window.h"

#include <SDL.h>
#include <SDL_syswm.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <cstdio>

using namespace Sprout;

static void set_platform_data_from_sdl(SDL_Window* window)
{
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);

    if (!SDL_GetWindowWMInfo(window, &wmi)) {
        printf("SDL_GetWindowWMInfo failed: %s\n", SDL_GetError());
        return;
    }

    bgfx::PlatformData pd{};

#if defined(_WIN32)
    pd.nwh = wmi.info.win.window;
#elif defined(__APPLE__)
    pd.nwh = wmi.info.cocoa.window;
    printf("Setting Cocoa window handle: %p\n", pd.nwh);
    printf("Window subsystem: %d\n", wmi.subsystem);
#else
    // X11
    pd.nwh = (void*)(uintptr_t)wmi.info.x11.window;
    pd.ndt = wmi.info.x11.display;
#endif

    bgfx::setPlatformData(pd);
    printf("Platform data set successfully\n");
}

bool Renderer::init(Window& window, bool vsync)
{
    SDL_Window* sdlWin = window.native();
    if (!sdlWin) {
        printf("SDL Window is null!\n");
        return false;
    }

    int w = 0, h = 0;
    SDL_GetWindowSize(sdlWin, &w, &h);
    m_width = (uint32_t)w;
    m_height = (uint32_t)h;
    printf("Window size: %dx%d\n", w, h);

    // Try headless mode first to test if bgfx works at all
    bgfx::Init init{};
    init.type = bgfx::RendererType::Noop; // Headless renderer
    init.resolution.width  = m_width;
    init.resolution.height = m_height;
    init.resolution.reset  = BGFX_RESET_NONE;

    printf("Attempting bgfx::init with Noop renderer (headless)...\n");
    if (!bgfx::init(init)) {
        printf("Even headless bgfx::init failed\n");
        return false;
    }

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, m_width, m_height);

    m_initialized = true;
    printf("bgfx initialization successful with headless renderer!\n");
    return true;
}

void Renderer::resize(uint32_t w, uint32_t h)
{
    if (!m_initialized) return;
    m_width = w; m_height = h;
    bgfx::reset(w, h, BGFX_RESET_VSYNC);
    bgfx::setViewRect(0, 0, 0, w, h);
}

void Renderer::frame()
{
    if (!m_initialized) return;

    // Keep view 0 alive & cleared
    bgfx::touch(0);

    // Optional: draw debug text in the corner
    bgfx::dbgTextClear();
    bgfx::dbgTextPrintf(2, 1, 0x0f, "Sprout Engine running");

    bgfx::frame();
}

void Renderer::shutdown()
{
    if (m_initialized) {
        bgfx::shutdown();
        m_initialized = false;
    }
}
