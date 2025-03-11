#include "Renderer.h"

Renderer::Renderer(char* appName, GLFWwindow* windowHandle)
{
	m_VulkanContext = new VulkanContext(appName, windowHandle, m_imageBuffers);
	m_Ui = new Ui(windowHandle, m_VulkanContext->GetVulkanData());
}

Renderer::~Renderer()
{
    delete m_Ui;
	delete m_VulkanContext;
}

void Renderer::Draw(GLFWwindow* windowHandle)
{
    // Process Ui events in vulkan renderer

	// Check if window size has changed and act accordingly
	m_VulkanContext->CheckWindowSize(windowHandle);
    
	// Calculate, to which of our imagebuffers we render next
    int frameIndex = m_renderedImageCount % m_imageBuffers;

	// 1) Wait until the GPU has finished rendering the last frame
	// 2) Request and get next image from the swapchain
	// 3) Start recording draw commands for the gpu
	// 4) Clear the swapchain image with a solid color
	RenderData renderData = m_VulkanContext->BeginFrame(frameIndex);

    // Compute pass: Render shader to offscreen image
    m_VulkanContext->RenderComputeShader(renderData, m_Pushconstants);

	// Update UI Elements
	m_Ui->Update(renderData, m_Pushconstants);

    //// UI pass: Render ImGui onto the image
	m_Ui->Render(renderData);

    // 1) Copy the final image to the swapchain
    // 2) Submit and present the frame
    m_VulkanContext->EndFrame(renderData, frameIndex);

	m_renderedImageCount++;
}