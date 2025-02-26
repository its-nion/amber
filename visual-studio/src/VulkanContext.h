#pragma once

#include "util/libraries.h"
#include "VkHelper.h"

/// <summary>
/// Manages all basic Vulkan objects
/// </summary>
class VulkanContext 
{
    public:
        VulkanContext(char* appName, GLFWwindow* windowHandle);
        ~VulkanContext();

    private:
        VkInstance _instance;
	    VkDebugUtilsMessengerEXT _debugMessenger;

        VkSurfaceKHR _surface;

        VkDevice _device;
	    VkPhysicalDevice _physicalDevice;

        VkQueue _graphicsQueue;
        uint32_t _graphicsQueueFamily;

        VmaAllocator _allocator;

        VkSwapchainKHR _swapchain;
        VkFormat _swapchainImageFormat;
        std::vector<VkImage> _swapchainImages;
        std::vector<VkImageView> _swapchainImageViews;
        VkExtent2D _swapchainExtent;

        VkCommandPool _commandPool;

        void InitVulkan(char* appName, GLFWwindow* windowHandle);
        void InitSwapchain(GLFWwindow* windowHandle);
        void ResizeSwapchain(GLFWwindow* windowHandle);
        //void InitCommandStructure();
        //void InitSyncStructure();
        //void InitImage(VkExtent3D dimension);
        ///void InitDescriptors();
        //void InitComputePipeline();
};
