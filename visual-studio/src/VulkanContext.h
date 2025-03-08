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
        VulkanContext(char* appName, GLFWwindow* windowHandle);
        ~VulkanContext();

        void Draw(VkCommandBuffer commandBuffer);
        VkCommandBuffer BeginFrame();
        void EndFrame(VkCommandBuffer commandBuffer);
        void RenderToImage(VkCommandBuffer commandBuffer);
        void CopyToSwapchain(VkCommandBuffer commandBuffer);

        VkInstance GetInstance() const { return m_Instance; }
        VkDevice GetDevice() const { return m_Device; }

    private:
        void InitVulkan(char* appName, GLFWwindow* windowHandle);
        void InitSwapchain(GLFWwindow* windowHandle);
        void ResizeSwapchain(GLFWwindow* windowHandle);
        void InitCommandStructure();
        void CleanupVulkan();
        void CreateComputePipeline();

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
};
