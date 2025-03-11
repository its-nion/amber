#include "Application.h"

Application::Application(const char* name, int width, int height)
{
    m_Window = new Window("amber", 1280, 720);
	m_Renderer = new Renderer((char*)"amber", m_Window->GetWindowHandle());
}

Application::~Application()
{
	delete m_Renderer;
	delete m_Window;
}

void Application::Run()
{
    while (!m_Window->ShouldClose()) {
		// Update Window Events
		m_Window->PollEvents();

		// Draw Window Content
		m_Renderer->Draw(m_Window->GetWindowHandle());
    }
}
