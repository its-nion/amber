#pragma once

#include "util/libraries.h"

#include "Window.h"
#include "VulkanContext.h"
#include "Ui.h"

/// <summary>
/// Top level renderer class
/// </summary>
class Renderer 
{
    public:
        Renderer(char* appName, GLFWwindow* windowHandle);
        ~Renderer();

        void Draw();

    private:
        int m_renderedImageCount = 0;
        int m_imageBuffers = 2;

		VulkanContext* m_VulkanContext;
		Ui* m_Ui;
};
