#include "Application.h"

Application::Application()
{
    _window = new Window("amber", 1280, 720);
    _vulkanContext = new VulkanContext((char*)"amber", _window->GetWindowHandle());
}

Application::~Application()
{
	delete _vulkanContext;
	delete _window;
}

void Application::Run()
{
    while (!_window->ShouldClose())
    {
        _window->PollEvents();
    }
}
