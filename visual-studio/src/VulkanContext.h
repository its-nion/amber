#pragma once

#include "util/libraries.h"

#include "Window.h"
#include "VkHelper.h"

/// <summary>
/// Manages Vulkan related rendering code
/// </summary>
class VulkanContext
{
    public:
        VulkanContext(char* appName, GLFWwindow* windowHandle, int imageBuffers);
        ~VulkanContext();

		void Update(GLFWwindow* windowHandle, UiTriggers triggers);
        RenderData BeginFrame(int frameIndex);
        void RenderComputeShader(RenderData data, PushConstants& pc);
		void EndFrame(RenderData renderData, int frameIndex);

        VulkanData GetVulkanData() const;

    private:
        void InitVulkan(char* appName, GLFWwindow* windowHandle);

        void CreateSwapchain(GLFWwindow* windowHandle);
        void ResizeSwapchain(GLFWwindow* windowHandle);
        void DestroySwapchain();

        void InitCommandStructure(int imageBuffers);
		void InitSyncStructure(int imageBuffers);

        void InitImage(VkExtent3D dimension);
        void InitDescriptors();
        void InitComputePipeline();

		void ResizeDrawImage();

		// Changes if window has to be resized
        bool m_ResizeSwapchain = false;

        // Vulkan base
        VkInstance m_Instance;
        VkDebugUtilsMessengerEXT m_DebugMessenger;

        // Devices
        VkPhysicalDevice m_PhysicalDevice;
        VkDevice m_Device;

        // Surface
        VkSurfaceKHR m_Surface;

        // Swapchain
        VkSwapchainKHR m_Swapchain;
        VkFormat m_SwapchainImageFormat;
        std::vector<VkImage> m_SwapchainImages;
        std::vector<VkImageView> m_SwapchainImageViews;
        VkExtent2D m_SwapchainExtent;

        // GPU queue properties
        VkQueue m_GraphicsQueue;
        uint32_t m_GraphicsQueueFamily;

        // Vma
        VmaAllocator m_Allocator;

        // Commandpool
        VkCommandPool m_CommandPool;

        // Frame Info
        std::vector<FrameData> m_Frames;

        // Own image
        AllocatedImage m_DrawImage;
        VkExtent2D m_DrawExtent;

        // Descriptor memory management
        DescriptorAllocator m_GlobalDescriptorAllocator;

        // Image Descriptor
        VkDescriptorSet m_DrawImageDescriptorSet;
        VkDescriptorSetLayout m_DrawImageDescriptorLayout;

        // Pipeline
        VkPipeline m_ComputePipeline;
        VkPipelineLayout m_ComputePipelineLayout;

        // Final image from compute Shader
        VkSampler m_ShaderImageSampler;
        VkDescriptorSet m_ShaderImageTexture;
};
