\
#include "../../engine/Core/Application.h"

int main(int, char**)
{
    Sprout::AppConfig cfg;
    cfg.title = "Sprout Engine - Demo";
    cfg.width = 1280;
    cfg.height = 720;
    cfg.vsync = true;
    Sprout::Application app(cfg);
    return app.run();
}
