#include "VulkanContext.h"

// VMA library
#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

VulkanContext::VulkanContext(char* appName, GLFWwindow* windowHandle)
{
	InitVulkan(appName, windowHandle);

	InitSwapchain(windowHandle);

	//InitCommandStructure();
}

VulkanContext::~VulkanContext()
{
    // Wait until GPU finished everything
    vkDeviceWaitIdle(_device);

    // Destroy old swapchain
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);

    // Destroy old swapchain resources
    for (int i = 0; i < _swapchainImageViews.size(); i++) {

        vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
    }

    // Destroy surface
    vkDestroySurfaceKHR(_instance, _surface, nullptr);

	// Destroy memory allocator
    vmaDestroyAllocator(_allocator);

	// Destroy logical device (cant delete physical device)
    vkDestroyDevice(_device, nullptr);

	// Destroy debug messenger
    vkb::destroy_debug_utils_messenger(_instance, _debugMessenger);

	// Destroy vulkan instance
    vkDestroyInstance(_instance, nullptr);
}

void VulkanContext::InitVulkan(char* appName, GLFWwindow* windowHandle)
{
    vkb::InstanceBuilder builder;

    auto inst_ret = builder.set_app_name(appName)
        .request_validation_layers(ISDEBUG)
        .use_default_debug_messenger()
        .require_api_version(1, 3, 0)
        .build();

    if (!inst_ret) {
        std::cout << "Could not create a VkInstance!" << std::endl;
    }

    vkb::Instance vkb_inst = inst_ret.value();

    // Save instance and debugMessenger handle
    _instance = vkb_inst.instance;
    _debugMessenger = vkb_inst.debug_messenger;

    // Create surface or set rendercontext
    VK_CHECK(glfwCreateWindowSurface(_instance, windowHandle, NULL, &_surface));

    // Vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    features.dynamicRendering = true;
    features.synchronization2 = true;

    // Vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;


    // Use vkbootstrap to select a gpu
    // We want a gpu that can write to the GLFW surface and supports vulkan 1.3 with the correct features
    vkb::PhysicalDeviceSelector selector{ vkb_inst };
    vkb::PhysicalDevice physicalDevice = selector
        .set_minimum_version(1, 3)
        .set_required_features_13(features)
        .set_required_features_12(features12)
        .set_surface(_surface)
        .select()
        .value();

    // Create the final vulkan device
    vkb::DeviceBuilder deviceBuilder{ physicalDevice };

    vkb::Device vkbDevice = deviceBuilder.build().value();

    // Save device handles
    _physicalDevice = physicalDevice.physical_device;
    _device = vkbDevice.device;

    // Get a Graphics queue with vkbootstrap
    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    // Initialize the memory allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _physicalDevice;
    allocatorInfo.device = _device;
    allocatorInfo.instance = _instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &_allocator);
}

void VulkanContext::InitSwapchain(GLFWwindow* windowHandle)
{
    int newWidth, newHeight;
    glfwGetWindowSize(windowHandle, &newWidth, &newHeight);

    vkb::SwapchainBuilder swapchainBuilder{ _physicalDevice,_device,_surface };

    _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapchainBuilder
        .set_desired_format(VkSurfaceFormatKHR{ .format = _swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
        .set_desired_extent(newWidth, newHeight)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build()
        .value();

    _swapchainExtent = vkbSwapchain.extent;
    _swapchain = vkbSwapchain.swapchain;
    _swapchainImages = vkbSwapchain.get_images().value();
    _swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void VulkanContext::ResizeSwapchain(GLFWwindow* windowHandle)
{
    vkDeviceWaitIdle(_device);

	// Destroy old swapchain
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);

    // Destroy old swapchain resources
    for (int i = 0; i < _swapchainImageViews.size(); i++) {

        vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
    }

	// Get new window size
    int newWidth, newHeight;
    glfwGetWindowSize(windowHandle, &newWidth, &newHeight);

	// Create new swapchain
    vkb::SwapchainBuilder swapchainBuilder{ _physicalDevice,_device,_surface };

    _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapchainBuilder
        .set_desired_format(VkSurfaceFormatKHR{ .format = _swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
        .set_desired_extent(newWidth, newHeight)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build()
        .value();

    _swapchainExtent = vkbSwapchain.extent;
    _swapchain = vkbSwapchain.swapchain;
    _swapchainImages = vkbSwapchain.get_images().value();
    _swapchainImageViews = vkbSwapchain.get_image_views().value();
}

//void VulkanContext::InitCommandStructure()
//{
//    // Create a Command Pool for commands submitted to the graphics queue
//    VkCommandPoolCreateInfo commandPoolInfo = vkhelper::commandPoolCreateInfo(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
//
//    VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_commandPool));
//
//    for (int i = 0; i < 2; i++) {
//        // Allocate Commandbuffers for each frame
//        VkCommandBufferAllocateInfo cmdAllocInfo = vkhelper::commandBufferAllocateInfo(_commandPool, 1);
//
//        VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_frames[i]._mainCommandBuffer));
//    }
//}
