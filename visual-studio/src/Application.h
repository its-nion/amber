#pragma once

#include "Window.h"
#include "Renderer.h"

/// <summary>
/// Top level class and event manager.
/// </summary>
class Application
{
	public:
		Application();
		~Application();

	private:
		Window window;
		Renderer* renderer;
};

