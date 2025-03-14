#pragma once

#include "util/libraries.h"

#include "Window.h"
#include "VulkanContext.h"
#include "Ui.h"

/// <summary>
/// Top level renderer class, manages vulkan and ui rendering
/// </summary>
class Renderer 
{
    public:
        Renderer(char* appName, GLFWwindow* windowHandle);
        ~Renderer();

        void Draw(GLFWwindow* windowHandle);

    private:
        int m_renderedImageCount = 0;
        RenderTimeArray m_rta;
        int m_imageBuffers = 2;


        PushConstants m_Pushconstants;
		VulkanContext* m_VulkanContext;
		Ui* m_Ui;

        std::thread m_exportThread;

		void loadPresetParams(int preset);
};
