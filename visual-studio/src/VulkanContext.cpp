#include "VulkanContext.h"

VulkanContext::VulkanContext(char* appName, GLFWwindow* windowHandle)
{
    InitVulkan(appName, windowHandle);

    InitSwapchain(windowHandle);

    //initCommandStructure();

    //initSyncStructure();
}

VulkanContext::~VulkanContext()
{
}

//void Renderer::RenderToImage(VkCommandBuffer commandBuffer) {
//    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
//    vkCmdDispatch(commandBuffer, workgroupCountX, workgroupCountY, 1);
//}

//VkCommandBuffer Renderer::BeginFrame() {
//    vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
//    vkResetFences(device, 1, &inFlightFence);
//
//    uint32_t imageIndex;
//    VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
//
//    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
//        RecreateSwapchain();
//        return VK_NULL_HANDLE;
//    }
//
//    vkResetCommandBuffer(commandBuffers[imageIndex], 0);
//
//    VkCommandBufferBeginInfo beginInfo{};
//    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//    vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo);
//
//    return commandBuffers[imageIndex];
//}

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
    m_Instance = vkb_inst.instance;
    m_DebugMessenger = vkb_inst.debug_messenger;

    // Create surface or set rendercontext
    VK_CHECK(glfwCreateWindowSurface(m_Instance, windowHandle, NULL, &m_Surface));

    // Vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    features.dynamicRendering = true;
    features.synchronization2 = true;

    // Vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;


    // Use vkbootstrap to select a gpu
    // We want a gpu that can write to the SDL surface and supports vulkan 1.3 with the correct features
    vkb::PhysicalDeviceSelector selector{ vkb_inst };
    vkb::PhysicalDevice physicalDevice = selector
        .set_minimum_version(1, 3)
        .set_required_features_13(features)
        .set_required_features_12(features12)
        .set_surface(m_Surface)
        .select()
        .value();


    // Create the final vulkan device
    vkb::DeviceBuilder deviceBuilder{ physicalDevice };

    vkb::Device vkbDevice = deviceBuilder.build().value();

    // Save device handles
    m_PhysicalDevice = physicalDevice.physical_device;
    m_Device = vkbDevice.device;

    // Get a Graphics queue with vkbootstrap
    m_GraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    m_GraphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    // Initialize the memory allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = m_PhysicalDevice;
    allocatorInfo.device = m_Device;
    allocatorInfo.instance = m_Instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &m_Allocator);
}

void VulkanContext::InitSwapchain(GLFWwindow* windowHandle)
{
    int curentWindowWidth, curentWindowHeight;
    glfwGetWindowSize(windowHandle, &curentWindowWidth, &curentWindowHeight);

    vkb::SwapchainBuilder swapchainBuilder{ m_PhysicalDevice,m_Device,m_Surface };

    m_SwapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapchainBuilder
        .set_desired_format(VkSurfaceFormatKHR{ .format = m_SwapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
        .set_desired_extent(curentWindowWidth, curentWindowHeight)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build()
        .value();

    m_SwapchainExtent = vkbSwapchain.extent;
    m_Swapchain = vkbSwapchain.swapchain;
    m_SwapchainImages = vkbSwapchain.get_images().value();
    m_SwapchainImageViews = vkbSwapchain.get_image_views().value();
}

void VulkanContext::ResizeSwapchain(GLFWwindow* windowHandle)
{
    vkDeviceWaitIdle(m_Device);

    // Destroy Swapchain and resources
    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);

    for (int i = 0; i < m_SwapchainImageViews.size(); i++) {

        vkDestroyImageView(m_Device, m_SwapchainImageViews[i], nullptr);
    }

    // Recreate Swapchain
    int curentWindowWidth, curentWindowHeight;
    glfwGetWindowSize(windowHandle, &curentWindowWidth, &curentWindowHeight);

    vkb::SwapchainBuilder swapchainBuilder{ m_PhysicalDevice,m_Device,m_Surface };

    m_SwapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapchainBuilder
        .set_desired_format(VkSurfaceFormatKHR{ .format = m_SwapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
        .set_desired_extent(curentWindowWidth, curentWindowHeight)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build()
        .value();

    m_SwapchainExtent = vkbSwapchain.extent;
    m_Swapchain = vkbSwapchain.swapchain;
    m_SwapchainImages = vkbSwapchain.get_images().value();
    m_SwapchainImageViews = vkbSwapchain.get_image_views().value();
}

void VulkanContext::InitCommandStructure()
{
    // Create a Command Pool for commands submitted to the graphics queue
    VkCommandPoolCreateInfo commandPoolInfo = vkhelper::commandPoolCreateInfo(m_GraphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    VK_CHECK(vkCreateCommandPool(m_Device, &commandPoolInfo, nullptr, &m_CommandPool));

    for (int i = 0; i < 2; i++) {
        // Allocate Commandbuffers for each frame
        VkCommandBufferAllocateInfo cmdAllocInfo = vkhelper::commandBufferAllocateInfo(m_CommandPool, 1);

        VK_CHECK(vkAllocateCommandBuffers(m_Device, &cmdAllocInfo, &_frames[i]._mainCommandBuffer));
    }
}
