#include "Application.h"

Application::Application(const char* name, int width, int height)
{
    window = new Window("amber", 1280, 720);
	renderer = new Renderer((char*)"amber", window->GetWindowHandle());
}

Application::~Application()
{
	delete renderer;
	delete window;
}

void Application::Run()
{
    while (!window->ShouldClose()) {
		// Update Window Events
        window->PollEvents();

		// Draw Window Content
        renderer->Draw();
    }
}
