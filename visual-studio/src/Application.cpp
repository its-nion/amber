#include "Application.h"

Application::Application()
    : window("amber", 1280, 720)
{
    renderer = new Renderer();
    renderer->initialize((char*)"amber", window.getWindowHandle());

    // Main loop
    while (!window.shouldClose())
    {
        window.update();
        renderer->render(window.getWindowHandle());
    }
}

Application::~Application()
{
    renderer->cleanup();

    delete renderer;
}
