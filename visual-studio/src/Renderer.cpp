#include "Renderer.h"

Renderer::Renderer(char* appName, GLFWwindow* windowHandle)
{
	_vulkanContext = new VulkanContext(appName, windowHandle);
}

Renderer::~Renderer()
{
	delete _vulkanContext;
}

void Renderer::DrawFrame()
{
	// Code for drawing a frame

	// Code for drawing ui

    // -----------------------------------------------------------------------------------------------------------------------------------------------------------------

    // Wait for the gpu to finish rendering the last frame. After one second continue
    //VK_CHECK(vkWaitForFences(_logicalDevice, 1, &getCurrentFrame()._renderFence, true, 1000000000));
    //VK_CHECK(vkResetFences(_logicalDevice, 1, &getCurrentFrame()._renderFence));

    //// Request new Image from Swapchain
    //uint32_t swapchainImageIndex;
    //VkResult e = vkAcquireNextImageKHR(_logicalDevice, _swapchain, 1000000000, getCurrentFrame()._swapchainSemaphore, nullptr, &swapchainImageIndex);
    //if (e == VK_ERROR_OUT_OF_DATE_KHR) {
    //    _resizeSwapchain = true;
    //    return;
    //}
    //VkCommandBuffer cmd = getCurrentFrame()._mainCommandBuffer;

    //// Reset CommandBuffer to start recording again
    //VK_CHECK(vkResetCommandBuffer(cmd, 0));

    //// Begin recording to Commandbuffer, for one time use
    //VkCommandBufferBeginInfo cmdBeginInfo = vkhelper::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    //_drawExtent.width = _drawImage.imageExtent.width;
    //_drawExtent.height = _drawImage.imageExtent.height;

    //VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    //// Transition image to general layout to write on it
    //if (_drawImage.image != nullptr) vkhelper::transitionImage(cmd, _drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    //vkhelper::transitionImage(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    //// Make Swapchain Image black
    //VkClearColorValue clearValue = { { 0.1f, 0.1f, 0.1f, 1.0f } };
    //VkImageSubresourceRange clearRange = vkhelper::imageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

    ////clear image
    //vkCmdClearColorImage(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

    //// Render compute shader
    //if (_drawImage.image != nullptr) drawCommandsComputeShader(cmd);

    //// Transition Swapchain-Image to attachement optimal, so that UI can be drawn
    //vkhelper::transitionImage(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    //// Draw Ui on SwapchainImage
    //drawCommandsImGui(cmd, _swapchainImageViews[swapchainImageIndex]);

    //// Transition Swapchain-Image to present it
    //vkhelper::transitionImage(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    //// End CommandBuffer (we can no longer add commands, but it can now be executed)
    //VK_CHECK(vkEndCommandBuffer(cmd));

    //// Render
    //VkCommandBufferSubmitInfo cmdinfo = vkhelper::commandBufferSubmitInfo(cmd);
    //VkSemaphoreSubmitInfo waitInfo = vkhelper::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, getCurrentFrame()._swapchainSemaphore); // Waits until SwapchainSemaphore is ready
    //VkSemaphoreSubmitInfo signalInfo = vkhelper::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, getCurrentFrame()._renderSemaphore); // Calls RenderSemaphore when ready
    //VkSubmitInfo2 submit = vkhelper::submitInfo(&cmdinfo, &signalInfo, &waitInfo);

    //// Submits and Executes CommandBuffer on Queue, while Renderfence Blocks until rendering is done
    //VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit, getCurrentFrame()._renderFence));

    //// Present

    //VkPresentInfoKHR presentInfo = {};
    //presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    //presentInfo.pNext = nullptr;
    //presentInfo.pSwapchains = &_swapchain;
    //presentInfo.swapchainCount = 1;
    //presentInfo.pWaitSemaphores = &getCurrentFrame()._renderSemaphore; // Wait until Rendersemaphore finishes to present
    //presentInfo.waitSemaphoreCount = 1;
    //presentInfo.pImageIndices = &swapchainImageIndex;

    //// Send Image to Queue for presentation
    //VkResult presentResult = vkQueuePresentKHR(_graphicsQueue, &presentInfo);
    //if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
    //    _resizeSwapchain = true;
    //}

    //// Increase the number of frames drawn
    //_renderedFrames++;
}
