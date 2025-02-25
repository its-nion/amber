#pragma once

#include "Window.h"
#include "VulkanContext.h"
#include "Renderer.h"

/// <summary>
/// Top level class and event manager.
/// </summary>
class Application
{
	public:
		Application();
		~Application();

		void Run();

	private:
		Window* _window;
		VulkanContext* _vulkanContext;
		//Renderer* _renderer;
};

