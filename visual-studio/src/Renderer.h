#pragma once

#include "VulkanContext.h"

/// <summary>
/// Manages rendering code, meaning compute shader and ui
/// </summary>
class Renderer 
{
	public:
		Renderer(char* appName, GLFWwindow* windowHandle);
		~Renderer();

		void DrawFrame();

	private:
		VulkanContext* _vulkanContext;
};
