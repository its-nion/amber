#include "Application.h"

Application::Application()
{
    _window = new Window("amber", 1280, 720);
	_renderer = new Renderer((char*)"amber", _window->GetWindowHandle());
}

Application::~Application()
{
	delete _renderer;
	delete _window;
}

void Application::Run()
{
    while (!_window->ShouldClose())
    {
        _window->PollEvents();

		_renderer->DrawFrame();
    }
}
