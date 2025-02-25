#pragma once

#define VMA_IMPLEMENTATION

#include "VkHelper.h"
#include "util/libraries.h"

/// <summary>
/// Manages all basic Vulkan objects
/// </summary>
class VulkanContext {
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
};
