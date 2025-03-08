#include "Renderer.h"

Renderer::Renderer(char* appName, GLFWwindow* windowHandle)
{
	m_VulkanContext = new VulkanContext(appName, windowHandle);
	//m_Ui = new Ui(windowHandle);
}

Renderer::~Renderer()
{
    delete m_Ui;
	delete m_VulkanContext;
}

void Renderer::Draw()
{
    //// Start recording frame commands
    //VkCommandBuffer commandBuffer = m_VulkanContext->BeginFrame();

    //if (commandBuffer == VK_NULL_HANDLE) return; // Skip if frame not acquired

    //// Compute pass: Render to offscreen image
    //m_VulkanContext->RenderToImage(commandBuffer);

    //// UI pass: Render ImGui onto the image
    //m_Ui->NewFrame();
    //m_Ui->Render(commandBuffer);

    //// Copy the final image to the swapchain
    //m_VulkanContext->CopyToSwapchain(commandBuffer);

    //// Submit and present the frame
    //m_VulkanContext->EndFrame(commandBuffer);
}