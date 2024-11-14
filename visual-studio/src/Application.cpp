
#include "Application.h"

void Application::create(const char* name, const int width, const int height)
{
    _window = new Window();
    _renderer = new Renderer();

    _window->create(name, width, height);
    _renderer->initialize((char*)name, _window->getWindowHandle());
}

void Application::update()
{
    while (!_window->shouldClose())
    {
        _window->update();
        _renderer->render(_window->getWindowHandle());
    }
}

void Application::destroy()
{
    _renderer->cleanup();
    _window->destroy();

    delete _window;
    delete _renderer;
}