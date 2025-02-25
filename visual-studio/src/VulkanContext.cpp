#include "VulkanContext.h"

VulkanContext::VulkanContext(char* appName, GLFWwindow* windowHandle)
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

VulkanContext::~VulkanContext()
{
    // Wait until GPU finished everything
    vkDeviceWaitIdle(_device);

    vkDestroySurfaceKHR(_instance, _surface, nullptr);

    vmaDestroyAllocator(_allocator);

    vkDestroyDevice(_device, nullptr);

    vkb::destroy_debug_utils_messenger(_instance, _debugMessenger);

    vkDestroyInstance(_instance, nullptr);
}
