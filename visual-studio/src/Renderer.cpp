#include "Renderer.h"

Renderer::Renderer(char* appName, GLFWwindow* windowHandle)
{
	m_VulkanContext = new VulkanContext(appName, windowHandle, m_imageBuffers);
	m_Ui = new Ui(windowHandle, m_VulkanContext->GetVulkanData());

	m_rta.initRendertimes(100);

	loadPresetParams(1);
}

Renderer::~Renderer()
{
    delete m_Ui;
	delete m_VulkanContext;
}

void Renderer::Draw(GLFWwindow* windowHandle)
{
	UiTriggers triggers = m_Ui->GetUiTriggers();

	// Check if preset was loaded through ui
	if (triggers.changeParameterPreset != 0)
	{
		loadPresetParams(triggers.changeParameterPreset);
	}

	// Update VulkanContext (resize, export image, ...)
	m_VulkanContext->Update(windowHandle, triggers);
    
	// Calculate, to which of our imagebuffers we render next
    int frameIndex = m_renderedImageCount % m_imageBuffers;

	// Start measuring Time, since rendering begins now
	auto startTime = std::chrono::high_resolution_clock::now();

	// 1) Wait until the GPU has finished rendering the last frame
	// 2) Request and get next image from the swapchain
	// 3) Start recording draw commands for the gpu
	// 4) Clear the swapchain image with a solid color
	RenderData renderData = m_VulkanContext->BeginFrame(frameIndex);

    // Compute pass: Render shader to offscreen image
    m_VulkanContext->RenderComputeShader(renderData, m_Pushconstants);

	// Update UI Elements
	m_Ui->Update(renderData, m_Pushconstants, m_renderedImageCount, m_rta.getAvgRendertime());

    // UI pass: Render ImGui with the offscreen shader image
	// onto swapchain image
	m_Ui->Render(renderData);

    // 1) Copy the final image to the swapchain
    // 2) Submit and present the frame
    m_VulkanContext->EndFrame(renderData, frameIndex);

	// Calculate how long rendering took
	auto currentTime = std::chrono::high_resolution_clock::now();
	m_rta.pushRendertime(std::chrono::duration<float>(currentTime - startTime).count() * 1000.0f);

	m_renderedImageCount++;
}

void Renderer::loadPresetParams(int preset)
{
	switch (preset)
	{
		case 0: // Nothing
			break;

		case 1: // Default
			m_Pushconstants.uv_scale = 1.3;
			m_Pushconstants.uv_offset = glm::vec2(1.0, 0.25);

			m_Pushconstants.fbm_octaves = 8;
			m_Pushconstants.fbm_amplitude = 0.5;
			m_Pushconstants.fbm_frequency = 1.0;
			m_Pushconstants.fbm_lacunarity = 1.0;
			m_Pushconstants.fbm_gain = 0.5;
			m_Pushconstants.fbm_shift = 2.0;

			m_Pushconstants.warp_iterations = 4;
			m_Pushconstants.warp_strength = 2;
			m_Pushconstants.warp_offset = glm::vec2(2.0, 2.0);
			m_Pushconstants.warp_primaryColor = glm::vec4(0.0, 0.0, 0.0, 1.0);
			m_Pushconstants.warp_secondaryColor = glm::vec4(1.0, 1.0, 1.0, 1.0);
			m_Pushconstants.warp_colorBalance = 3;
			m_Pushconstants.warp_tintColor = glm::vec4(1.0, 1.0, 1.0, 1.0);
			m_Pushconstants.warp_tintShade = 12;
			m_Pushconstants.warp_tintStrength = 0.75;

			m_Pushconstants.time = 0.0f;
			break;

		case 2: // Beach
			m_Pushconstants.uv_scale = 2.0;
			m_Pushconstants.uv_offset = glm::vec2(0.0, 0.0);

			m_Pushconstants.fbm_octaves = 8;
			m_Pushconstants.fbm_amplitude = 0.5;
			m_Pushconstants.fbm_frequency = 1.0;
			m_Pushconstants.fbm_lacunarity = 1.0;
			m_Pushconstants.fbm_gain = 0.5;
			m_Pushconstants.fbm_shift = 2.0;

			m_Pushconstants.warp_iterations = 3;
			m_Pushconstants.warp_strength = 3;
			m_Pushconstants.warp_offset = glm::vec2(1.0, 1.0);
			m_Pushconstants.warp_primaryColor = glm::vec4(0.04, 0.02, 0.0, 0.0);
			m_Pushconstants.warp_secondaryColor = glm::vec4(0.9, 0.85, 0.67, 0.0);
			m_Pushconstants.warp_colorBalance = 2;
			m_Pushconstants.warp_tintColor = glm::vec4(0.0, 0.8, 1.0, 1.0);
			m_Pushconstants.warp_tintShade = 8;
			m_Pushconstants.warp_tintStrength = 0.5;

			m_Pushconstants.time = 0.0f;
			break;

		case 3: // Water
			m_Pushconstants.uv_scale = 2.0;
			m_Pushconstants.uv_offset = glm::vec2(0.0, 0.0);

			m_Pushconstants.fbm_octaves = 8;
			m_Pushconstants.fbm_amplitude = 0.5;
			m_Pushconstants.fbm_frequency = 1.0;
			m_Pushconstants.fbm_lacunarity = 1.0;
			m_Pushconstants.fbm_gain = 0.5;
			m_Pushconstants.fbm_shift = 2.0;

			m_Pushconstants.warp_iterations = 3;
			m_Pushconstants.warp_strength = 2;
			m_Pushconstants.warp_offset = glm::vec2(1.15, 1.35);
			m_Pushconstants.warp_primaryColor = glm::vec4(0.024, 0.021, 0.123, 1.0);
			m_Pushconstants.warp_secondaryColor = glm::vec4(0.0, 0.559, 1.0, 1.0);
			m_Pushconstants.warp_colorBalance = 3;
			m_Pushconstants.warp_tintColor = glm::vec4(0.0, 0.912, 1.0, 1.0);
			m_Pushconstants.warp_tintShade = 9;
			m_Pushconstants.warp_tintStrength = 0.5;

			m_Pushconstants.time = 0.0f;
			break;

		case 4: // Clouds
			m_Pushconstants.uv_scale = 2.0;
			m_Pushconstants.uv_offset = glm::vec2(10.0, 11.0);

			m_Pushconstants.fbm_octaves = 8;
			m_Pushconstants.fbm_amplitude = 0.5;
			m_Pushconstants.fbm_frequency = 1.0;
			m_Pushconstants.fbm_lacunarity = 1.0;
			m_Pushconstants.fbm_gain = 0.5;
			m_Pushconstants.fbm_shift = 2.0;

			m_Pushconstants.warp_iterations = 3;
			m_Pushconstants.warp_strength = 2;
			m_Pushconstants.warp_offset = glm::vec2(5.0, 5.0);
			m_Pushconstants.warp_primaryColor = glm::vec4(0.0, 0.324, 1.0, 1.0);
			m_Pushconstants.warp_secondaryColor = glm::vec4(1.0, 1.0, 1.0, 1.0);
			m_Pushconstants.warp_colorBalance = 1;
			m_Pushconstants.warp_tintColor = glm::vec4(0.0, 0.5, 1.0, 1.0);
			m_Pushconstants.warp_tintShade = 8;
			m_Pushconstants.warp_tintStrength = 0.0;

			m_Pushconstants.time = 0.0f;
			break;
		default:
			break;
	}
}
