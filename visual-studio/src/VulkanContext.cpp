#include "VulkanContext.h"

#include "../third-party/include/tinyfiledialogs/tinyfiledialogs.h"

#include "../embedded-resources/warpedFbm.h"

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

VulkanContext::VulkanContext(char* appName, GLFWwindow* windowHandle, int imageBuffers)
{
    InitVulkan(appName, windowHandle);

    CreateSwapchain(windowHandle);

    InitCommandStructure(imageBuffers);

    InitSyncStructure(imageBuffers);
}

VulkanContext::~VulkanContext()
{
    // Wait until GPU finished everything
    vkDeviceWaitIdle(m_Device);

    vkDestroySampler(m_Device, m_ShaderImageSampler, nullptr);

    vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);

    for (int i = 0; i < m_Frames.size(); i++) {
        vkDestroyFence(m_Device, m_Frames[i].renderFence, nullptr);
        vkDestroySemaphore(m_Device, m_Frames[i].renderSemaphore, nullptr);
        vkDestroySemaphore(m_Device, m_Frames[i].swapchainSemaphore, nullptr);
    }

    vkDestroyImageView(m_Device, m_DrawImage.imageView, nullptr);
    vmaDestroyImage(m_Allocator, m_DrawImage.image, m_DrawImage.allocation);

    m_GlobalDescriptorAllocator.destroy_pool(m_Device);
    vkDestroyDescriptorSetLayout(m_Device, m_DrawImageDescriptorLayout, nullptr);

    vkDestroyPipelineLayout(m_Device, m_ComputePipelineLayout, nullptr);
    vkDestroyPipeline(m_Device, m_ComputePipeline, nullptr);

    DestroySwapchain();

    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vmaDestroyAllocator(m_Allocator);
    vkDestroyDevice(m_Device, nullptr);

    vkb::destroy_debug_utils_messenger(m_Instance, m_DebugMessenger);
    vkDestroyInstance(m_Instance, nullptr);
}

void VulkanContext::CheckWindowSize(GLFWwindow* windowHandle)
{
    if (m_ResizeSwapchain) ResizeSwapchain(windowHandle);

    if (m_ResizeShaderImage) ResizeDrawImage();
}

RenderData VulkanContext::BeginFrame(int frameIndex)
{
    // Wait for the gpu to finish rendering the last frame. After one second continue
    VK_CHECK(vkWaitForFences(m_Device, 1, &m_Frames[frameIndex].renderFence, true, 1000000000));

    VK_CHECK(vkResetFences(m_Device, 1, &m_Frames[frameIndex].renderFence));

    // Request new Image from Swapchain
    uint32_t swapchainImageIndex;
    VkResult e = vkAcquireNextImageKHR(m_Device, m_Swapchain, 1000000000, m_Frames[frameIndex].swapchainSemaphore, nullptr, &swapchainImageIndex);

    if (e == VK_ERROR_OUT_OF_DATE_KHR) {
        m_ResizeSwapchain = true;
        // return; TODO  NEED BETTER SOLUTION TO CANCEL AFTER RESIZE!
    }

    VkCommandBuffer commandBuffer = m_Frames[frameIndex].mainCommandBuffer;

    // Reset CommandBuffer to start recording again
    VK_CHECK(vkResetCommandBuffer(commandBuffer, 0));

    // Begin recording to Commandbuffer, for one time use
    VkCommandBufferBeginInfo cmdBeginInfo = vkhelper::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    m_DrawExtent.width = m_DrawImage.imageExtent.width;
    m_DrawExtent.height = m_DrawImage.imageExtent.height;

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo));

    // Transition image to general layout to write on it
    if (m_DrawImage.image != nullptr) vkhelper::transitionImage(commandBuffer, m_DrawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    vkhelper::transitionImage(commandBuffer, m_SwapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    // Make Swapchain Image black
    VkClearColorValue clearValue = { { 0.1f, 0.1f, 0.1f, 1.0f } };
    VkImageSubresourceRange clearRange = vkhelper::imageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

    // Clear Swapchain Image
    vkCmdClearColorImage(commandBuffer, m_SwapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

    // Transition Swapchain-Image to attachement optimal, so that UI can be drawn later
    vkhelper::transitionImage(commandBuffer, m_SwapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	return { 
        commandBuffer, 
        m_DrawImage, 
        swapchainImageIndex, 
        m_SwapchainImageViews[swapchainImageIndex],
        m_SwapchainExtent, 
        m_ShaderImageTexture
    };
}

void VulkanContext::RenderComputeShader(RenderData data, PushConstants& pc)
{
    if (data.offscreenImage.image != nullptr)
    {
        // Bind the drawing compute pipeline
        vkCmdBindPipeline(data.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline);

        // Bind the descriptor set containing the draw image for the compute pipeline
        vkCmdBindDescriptorSets(data.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipelineLayout, 0, 1, &m_DrawImageDescriptorSet, 0, nullptr);

        // Push constants to Shader
        vkCmdPushConstants(data.commandBuffer, m_ComputePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &pc);

        // Execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
        vkCmdDispatch(data.commandBuffer, std::ceil(m_DrawExtent.width / 16.0), std::ceil(m_DrawExtent.height / 16.0), 1);
    }
}

void VulkanContext::EndFrame(RenderData renderData, int frameIndex)
{
    // Transition Swapchain-Image to present it
    vkhelper::transitionImage(renderData.commandBuffer, m_SwapchainImages[renderData.swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // End CommandBuffer (we can no longer add commands, but it can now be executed)
    VK_CHECK(vkEndCommandBuffer(renderData.commandBuffer));

    // Render
    VkCommandBufferSubmitInfo cmdinfo = vkhelper::commandBufferSubmitInfo(renderData.commandBuffer);
    VkSemaphoreSubmitInfo waitInfo = vkhelper::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, m_Frames[frameIndex].swapchainSemaphore); // Waits until SwapchainSemaphore is ready
    VkSemaphoreSubmitInfo signalInfo = vkhelper::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, m_Frames[frameIndex].renderSemaphore); // Calls RenderSemaphore when ready
    VkSubmitInfo2 submit = vkhelper::submitInfo(&cmdinfo, &signalInfo, &waitInfo);

    // Submits and Executes CommandBuffer on Queue, while Renderfence Blocks until rendering is done
    VK_CHECK(vkQueueSubmit2(m_GraphicsQueue, 1, &submit, m_Frames[frameIndex].renderFence));

    // Present

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &m_Swapchain;
    presentInfo.swapchainCount = 1;
    presentInfo.pWaitSemaphores = &m_Frames[frameIndex].renderSemaphore; // Wait until Rendersemaphore finishes to present
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices = &renderData.swapchainImageIndex;

    // Send Image to Queue for presentation
    VkResult presentResult = vkQueuePresentKHR(m_GraphicsQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
        m_ResizeSwapchain = true;
    }
}

VulkanData VulkanContext::GetVulkanData() const
{
    return {
        m_Instance,
        m_Device,
        m_PhysicalDevice,
        m_GraphicsQueue,
		m_SwapchainImageFormat
    };
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

void VulkanContext::CreateSwapchain(GLFWwindow* windowHandle)
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

    DestroySwapchain();

    CreateSwapchain(windowHandle);

    m_ResizeSwapchain = false;
}

void VulkanContext::DestroySwapchain()
{
    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);

    // Destroy swapchain resources
    for (int i = 0; i < m_SwapchainImageViews.size(); i++) {

        vkDestroyImageView(m_Device, m_SwapchainImageViews[i], nullptr);
    }
}

void VulkanContext::InitCommandStructure(int imageBuffers)
{
    // Create a Command Pool for commands submitted to the graphics queue
    VkCommandPoolCreateInfo commandPoolInfo = vkhelper::commandPoolCreateInfo(m_GraphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    VK_CHECK(vkCreateCommandPool(m_Device, &commandPoolInfo, nullptr, &m_CommandPool));

    // Ensure m_Frames has enough elements
    m_Frames.resize(imageBuffers);

    for (int i = 0; i < imageBuffers; i++) {
        // Allocate Commandbuffers for each frame
        VkCommandBufferAllocateInfo cmdAllocInfo = vkhelper::commandBufferAllocateInfo(m_CommandPool, 1);

        VK_CHECK(vkAllocateCommandBuffers(m_Device, &cmdAllocInfo, &m_Frames[i].mainCommandBuffer));
    }
}

void VulkanContext::InitSyncStructure(int imageBuffers)
{
    VkFenceCreateInfo fenceCreateInfo = vkhelper::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = vkhelper::semaphoreCreateInfo();

    for (int i = 0; i < imageBuffers; i++) {
        VK_CHECK(vkCreateFence(m_Device, &fenceCreateInfo, nullptr, &m_Frames[i].renderFence)); // Controls when GPU has finished rendering a frame

        VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_Frames[i].swapchainSemaphore));
        VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_Frames[i].renderSemaphore));
    }
}

void VulkanContext::InitImage(VkExtent3D dimension)
{
    // Set Image Drawformat
    m_DrawImage.imageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
    m_DrawImage.imageExtent = dimension;

    VkImageUsageFlags drawImageUsages{};
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageCreateInfo rimg_info = vkhelper::imageCreateInfo(m_DrawImage.imageFormat, drawImageUsages, dimension);

    VmaAllocationCreateInfo rimg_allocinfo = {};
    rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    // Allocate and Create the Image
    vmaCreateImage(m_Allocator, &rimg_info, &rimg_allocinfo, &m_DrawImage.image, &m_DrawImage.allocation, nullptr);

    // Create Image-View with the Image to use for rendering
    VkImageViewCreateInfo rview_info = vkhelper::imageviewCreateInfo(m_DrawImage.imageFormat, m_DrawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

    VK_CHECK(vkCreateImageView(m_Device, &rview_info, nullptr, &m_DrawImage.imageView));

    // Sampler to write ComputeShaderImage to Texture to show on UI
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    //samplerInfo.anisotropyEnable = VK_FALSE;
    //samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VK_CHECK(vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_ShaderImageSampler));
}

void VulkanContext::InitDescriptors()
{
    // Create a descriptor pool that will hold 1 set with 1 image
    std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
    {
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
    };

    m_GlobalDescriptorAllocator.init_pool(m_Device, 1, sizes);

    // Make the descriptor set layout for our compute draw
    {
        DescriptorLayoutBuilder builder;
        builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        m_DrawImageDescriptorLayout = builder.build(m_Device, VK_SHADER_STAGE_COMPUTE_BIT);
    }

    // Allocate a descriptor set for our draw image
    m_DrawImageDescriptorSet = m_GlobalDescriptorAllocator.allocate(m_Device, m_DrawImageDescriptorLayout);

    VkDescriptorImageInfo imgInfo{};
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imgInfo.imageView = m_DrawImage.imageView;

    VkWriteDescriptorSet drawImageWrite = {};
    drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    drawImageWrite.pNext = nullptr;

    drawImageWrite.dstBinding = 0;
    drawImageWrite.dstSet = m_DrawImageDescriptorSet;
    drawImageWrite.descriptorCount = 1;
    drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    drawImageWrite.pImageInfo = &imgInfo;

    vkUpdateDescriptorSets(m_Device, 1, &drawImageWrite, 0, nullptr);
}

void VulkanContext::InitComputePipeline()
{
    // Push Constants
    VkPushConstantRange pushConstant{};
    pushConstant.offset = 0;
    pushConstant.size = sizeof(PushConstants);
    pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    // Layout
    VkPipelineLayoutCreateInfo computeLayout{};
    computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computeLayout.pNext = nullptr;
    computeLayout.pSetLayouts = &m_DrawImageDescriptorLayout;
    computeLayout.setLayoutCount = 1;

    computeLayout.pPushConstantRanges = &pushConstant;
    computeLayout.pushConstantRangeCount = 1;

    VK_CHECK(vkCreatePipelineLayout(m_Device, &computeLayout, nullptr, &m_ComputePipelineLayout));

    // Get the embedded shader bytecode using bin2cpp's API
    const bin2cpp::File& shaderFile = bin2cpp::getWarpedFbmSpvFile();

    // Retrieve the bytecode buffer and size
    const uint32_t* shaderCode = reinterpret_cast<const uint32_t*>(shaderFile.getBuffer());
    size_t shaderCodeSize = shaderFile.getSize();

    // Set up the create info structure for the shader module
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.codeSize = shaderCodeSize;  // Size of the bytecode
    createInfo.pCode = shaderCode;         // Pointer to the bytecode array

    // Create the shader module
    VkShaderModule computeDrawShader;
    if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &computeDrawShader) != VK_SUCCESS) {
        std::cout << "Error when building the compute shader \n" << std::endl;
    }

    // VkPipeline
    VkPipelineShaderStageCreateInfo stageinfo{};
    stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageinfo.pNext = nullptr;
    stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageinfo.module = computeDrawShader;
    stageinfo.pName = "main";

    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.layout = m_ComputePipelineLayout;
    computePipelineCreateInfo.stage = stageinfo;

    VK_CHECK(vkCreateComputePipelines(m_Device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &m_ComputePipeline));

    vkDestroyShaderModule(m_Device, computeDrawShader, nullptr);
}

void VulkanContext::ResizeDrawImage()
{
    vkDeviceWaitIdle(m_Device);

    vkDestroyImageView(m_Device, m_DrawImage.imageView, nullptr);
    vmaDestroyImage(m_Allocator, m_DrawImage.image, m_DrawImage.allocation);

    m_GlobalDescriptorAllocator.destroy_pool(m_Device);
    vkDestroyDescriptorSetLayout(m_Device, m_DrawImageDescriptorLayout, nullptr);

    vkDestroyPipelineLayout(m_Device, m_ComputePipelineLayout, nullptr);
    vkDestroyPipeline(m_Device, m_ComputePipeline, nullptr);

    InitImage(m_DrawImage.imageExtent);
    InitDescriptors();
    InitComputePipeline();

    if (m_ShaderImageTexture != NULL)
    {
        ImGui_ImplVulkan_RemoveTexture(m_ShaderImageTexture);
    }

    m_ShaderImageTexture = ImGui_ImplVulkan_AddTexture(m_ShaderImageSampler, m_DrawImage.imageView, VK_IMAGE_LAYOUT_GENERAL);

    m_ResizeShaderImage = false;
}
