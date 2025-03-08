#pragma once

#include "Window.h"
#include "Renderer.h"

/// <summary>
/// Top level class and event manager.
/// </summary>
class Application
{
	public:
		Application(const char* name, int width, int height);
		~Application();

		void Run();

	private:
		Window* window;
		Renderer* renderer;
};

