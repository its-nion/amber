
#include "Renderer.h"
#include "../shader/warpedFbm.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

// Public methods

void Renderer::initialize(char* appName, GLFWwindow* windowHandle)
{
    initVulkan(appName, windowHandle);

    initSwapchain(windowHandle);

    initCommandStructure();

    initSyncStructure();

    initImGui(windowHandle);

    initValues();

    _isInitialized = true;
}

void Renderer::render(GLFWwindow* windowHandle)
{
    if (_resizeSwapchain) resizeSwapchain(windowHandle);

    if (_resizeDrawImage) resizeDrawImage();

    drawImGui();

    draw();
}

void Renderer::cleanup()
{
    if (_isInitialized) {
        // Wait until GPU finished everything
        vkDeviceWaitIdle(_logicalDevice);

        ImGui_ImplVulkan_Shutdown();
        vkDestroyDescriptorPool(_logicalDevice, _imguiPool, nullptr);
        vkDestroySampler(_logicalDevice, _drawImageSampler, nullptr);

        vkDestroyCommandPool(_logicalDevice, _commandPool, nullptr);
        vkDestroyCommandPool(_logicalDevice, _immCommandPool, nullptr);

        for (int i = 0; i < 2; i++) {
            vkDestroyFence(_logicalDevice, _frames[i]._renderFence, nullptr);
            vkDestroySemaphore(_logicalDevice, _frames[i]._renderSemaphore, nullptr);
            vkDestroySemaphore(_logicalDevice, _frames[i]._swapchainSemaphore, nullptr);
        }

        vkDestroyImageView(_logicalDevice, _drawImage.imageView, nullptr);
        vmaDestroyImage(_allocator, _drawImage.image, _drawImage.allocation);

        globalDescriptorAllocator.destroy_pool(_logicalDevice);
        vkDestroyDescriptorSetLayout(_logicalDevice, _drawImageDescriptorLayout, nullptr);

        vkDestroyPipelineLayout(_logicalDevice, _computePipelineLayout, nullptr);
        vkDestroyPipeline(_logicalDevice, _computePipeline, nullptr);

        destroy_swapchain();

        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vmaDestroyAllocator(_allocator);
        vkDestroyDevice(_logicalDevice, nullptr);

        vkb::destroy_debug_utils_messenger(_instance, _debugMessenger);
        vkDestroyInstance(_instance, nullptr);
    }
}

// Local Methods

void Renderer::draw()
{
    // Measure time
    auto t1 = std::chrono::steady_clock::now();

    // Set new runtime in seconds
    _cpc.time = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - _initTimePoint).count() / 1000.0f;

    // Wait for the gpu to finish rendering the last frame. After one second continue
    VK_CHECK(vkWaitForFences(_logicalDevice, 1, &getCurrentFrame()._renderFence, true, 1000000000));

    VK_CHECK(vkResetFences(_logicalDevice, 1, &getCurrentFrame()._renderFence));

    // Request new Image from Swapchain
    uint32_t swapchainImageIndex;
    VkResult e = vkAcquireNextImageKHR(_logicalDevice, _swapchain, 1000000000, getCurrentFrame()._swapchainSemaphore, nullptr, &swapchainImageIndex);
    if (e == VK_ERROR_OUT_OF_DATE_KHR) {
        _resizeSwapchain = true;
        return;
    }
    VkCommandBuffer cmd = getCurrentFrame()._mainCommandBuffer;

    // Reset CommandBuffer to start recording again
    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    // Begin recording to Commandbuffer, for one time use
    VkCommandBufferBeginInfo cmdBeginInfo = vkhelper::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    _drawExtent.width = _drawImage.imageExtent.width;
    _drawExtent.height = _drawImage.imageExtent.height;

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    // Transition image to general layout to write on it
    if (_drawImage.image != nullptr) vkhelper::transitionImage(cmd, _drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    vkhelper::transitionImage(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    // Make Swapchain Image black
    VkClearColorValue clearValue = { { 0.1f, 0.1f, 0.1f, 1.0f } };
    VkImageSubresourceRange clearRange = vkhelper::imageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

    //clear image
    vkCmdClearColorImage(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

    // Render compute shader
    if (_drawImage.image != nullptr) drawCommandsComputeShader(cmd);

    // Transition Swapchain-Image to attachement optimal, so that UI can be drawn
    vkhelper::transitionImage(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Draw Ui on SwapchainImage
    drawCommandsImGui(cmd, _swapchainImageViews[swapchainImageIndex]);

    // Transition Swapchain-Image to present it
    vkhelper::transitionImage(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // End CommandBuffer (we can no longer add commands, but it can now be executed)
    VK_CHECK(vkEndCommandBuffer(cmd));

    // Render
    VkCommandBufferSubmitInfo cmdinfo = vkhelper::commandBufferSubmitInfo(cmd);
    VkSemaphoreSubmitInfo waitInfo = vkhelper::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, getCurrentFrame()._swapchainSemaphore); // Waits until SwapchainSemaphore is ready
    VkSemaphoreSubmitInfo signalInfo = vkhelper::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, getCurrentFrame()._renderSemaphore); // Calls RenderSemaphore when ready
    VkSubmitInfo2 submit = vkhelper::submitInfo(&cmdinfo, &signalInfo, &waitInfo);

    // Submits and Executes CommandBuffer on Queue, while Renderfence Blocks until rendering is done
    VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit, getCurrentFrame()._renderFence));

    // Present

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &_swapchain;
    presentInfo.swapchainCount = 1;
    presentInfo.pWaitSemaphores = &getCurrentFrame()._renderSemaphore; // Wait until Rendersemaphore finishes to present
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices = &swapchainImageIndex;

    // Send Image to Queue for presentation
    VkResult presentResult = vkQueuePresentKHR(_graphicsQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
        _resizeSwapchain = true;
    }

    // Increase the number of frames drawn
    _renderedFrames++;

    // Set deltatime
    auto t2 = std::chrono::steady_clock::now();
    _rta.pushRendertime(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000.0f);
}

void Renderer::drawImGui()
{
    // Get current global windowsize
    ImGuiIO& io = ImGui::GetIO();
    ImGuiViewport* imGuiViewport = ImGui::GetMainViewport();

    // New Frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Menu Bar
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.35, 0.35, 0.35, 0.0));
    std::string menu_action = "";
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New")) menu_action = "New";

            if (ImGui::MenuItem("Export")) 
            {
                if (_drawImage.imageView != NULL) exportImage("F:/Hobby/Vulkan/Amber/Image.png");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Show"))
        {
            ImGui::MenuItem("Stats", NULL, &_statsOpened);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Zoom"))
        {
            if (ImGui::MenuItem("25%")) _zoomFactor = 0.25;
            if (ImGui::MenuItem("50%")) _zoomFactor = 0.50;
            if (ImGui::MenuItem("75%")) _zoomFactor = 0.75;
            if (ImGui::MenuItem("100%")) _zoomFactor = 1.00;
            if (ImGui::MenuItem("125%")) _zoomFactor = 1.25;
            if (ImGui::MenuItem("150%")) _zoomFactor = 1.50;
            if (ImGui::MenuItem("200%")) _zoomFactor = 2.00;
            ImGui::EndMenu();
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0, 0.0, 0.0, 0.0));
        if (ImGui::Button("About")) menu_action = "About";
        ImGui::PopStyleColor();

        ImGui::EndMainMenuBar();
    }
    ImGui::PopStyleColor();

    // Property Window
    ImGui::Begin("Properties", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
    {
        ImGui::SetWindowSize(ImVec2(300.0, imGuiViewport->WorkSize.y), 0);
        ImGui::SetWindowPos(ImVec2(imGuiViewport->WorkSize.x - 300.0, imGuiViewport->WorkPos.y), 0);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->ChannelsSplit(2);

        //ImGui::GetStyle().FrameBorderSize = 1.0;

        // UV
        {
            float margin = 10.0f;
            ImVec2 cursor_start = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(cursor_start.x + margin, cursor_start.y + margin / 2.0));
            ImGui::PushItemWidth(300);
            ImGui::BeginGroup();

            drawList->ChannelsSetCurrent(1);

            ImGui::Text("UV");

            ImGui::Dummy(ImVec2(0.0, 1.0));

            if (ImGui::BeginTable("uvTable", 2, 0))
            {
                ImGui::TableSetupColumn("uvOne", ImGuiTableColumnFlags_WidthFixed, 115.0f);
                ImGui::TableSetupColumn("uvTwo", ImGuiTableColumnFlags_WidthFixed, 145.0f);

                // Scale
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Scale");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::InputFloat("##Scale", &_cpc.uv_scale, 0.0, 0.0, "%.1f", 0);

                // Offset
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Offset");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::InputFloat2("##Offset", (float*)&_cpc.uv_offset, "%.1f", 0);

                // Evolution
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Evolution");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::InputFloat2("##Evolution", (float*)&_cpc.uv_offset, "%.1f", 0);

                ImGui::EndTable();
            }

            ImGui::EndGroup();
            ImGui::PopItemWidth();

            ImVec2 group_min = ImGui::GetItemRectMin();
            ImVec2 group_max = ImGui::GetItemRectMax();
            group_min.x -= margin;
            group_min.y -= margin / 2.0;
            group_max.y += margin / 2.0;

            drawList->ChannelsSetCurrent(0);

            // Background
            drawList->AddRectFilled(group_min, group_max, IM_COL32(47.5, 47.5, 47.5, 255), 0.0, 0.0);
            // Separator
            drawList->AddLine(ImVec2(group_min.x + 7.5, group_min.y + 22.0), ImVec2(group_max.x - 7.5, group_min.y + 22.0), IM_COL32(70.0, 70.0, 70.0, 255), 1.0);
        }

        ImGui::Dummy(ImVec2(0.0, 5.0));

        // Fractal Brownian Motion
        {
            float margin = 10.0f;
            ImVec2 cursor_start = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(cursor_start.x + margin, cursor_start.y + margin / 2.0));
            ImGui::PushItemWidth(300);
            ImGui::BeginGroup();

            drawList->ChannelsSetCurrent(1);

            ImGui::Text("Fractal Brownian Motion");

            ImGui::Dummy(ImVec2(0.0, 1.0));

            if (ImGui::BeginTable("fbmTable", 2, 0))
            {
                ImGui::TableSetupColumn("fbmOne", ImGuiTableColumnFlags_WidthFixed, 115.0f);
                ImGui::TableSetupColumn("fbmTwo", ImGuiTableColumnFlags_WidthFixed, 145.0f);

                //float fbm_shift;

                // Octaves
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Octaves");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::DragInt("##Octaves", &_cpc.fbm_octaves, 0.1, 1, INT_MAX, "%d", 0);

                // Amplitude
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Amplitude");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::DragFloat("##Amplitude", &_cpc.fbm_amplitude, 0.001, 0.0, 1.0, "%.3f", 0);

                // Frequenzy
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Frequenzy");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::DragFloat("##Frequenzy", &_cpc.fbm_frequency, 0.001, 0.0, 2.0, "%.03f", 0);

                // Lacunarity
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Lacunarity");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::DragFloat("##Lacunarity", &_cpc.fbm_lacunarity, 0.001, 0.0, 2.0, "%.03f", 0);

                // Gain
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Gain");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::DragFloat("##Gain", &_cpc.fbm_gain, 0.001, 0.0, 1.0, "%.03f", 0);

                // Shift
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Shift");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::DragFloat("##Shift", &_cpc.fbm_shift, 0.001, 0.0, 5.0, "%.03f", 0);

                ImGui::EndTable();
            }


            ImGui::EndGroup();
            ImGui::PopItemWidth();

            ImVec2 group_min = ImGui::GetItemRectMin();
            ImVec2 group_max = ImGui::GetItemRectMax();
            group_min.x -= margin;
            group_min.y -= margin / 2.0;
            group_max.y += margin / 2.0;

            drawList->ChannelsSetCurrent(0);

            // Background
            drawList->AddRectFilled(group_min, group_max, IM_COL32(47.5, 47.5, 47.5, 255), 0.0, 0.0);
            // Separator
            drawList->AddLine(ImVec2(group_min.x + 7.5, group_min.y + 22.0), ImVec2(group_max.x - 7.5, group_min.y + 22.0), IM_COL32(70.0, 70.0, 70.0, 255), 1.0);
        }

        ImGui::Dummy(ImVec2(0.0, 5.0));

        // Warp
        {
            float margin = 10.0f;
            ImVec2 cursor_start = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(cursor_start.x + margin, cursor_start.y + margin / 2.0));
            ImGui::PushItemWidth(300);
            ImGui::BeginGroup();

            drawList->ChannelsSetCurrent(1);

            ImGui::Text("Warp");

            ImGui::Dummy(ImVec2(0.0, 1.0));

            if (ImGui::BeginTable("warpTable", 2, 0))
            {
                ImGui::TableSetupColumn("warpOne", ImGuiTableColumnFlags_WidthFixed, 115.0f);
                ImGui::TableSetupColumn("warpTwo", ImGuiTableColumnFlags_WidthFixed, 145.0f);

                // Warp iterations
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Iterations");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::SliderInt("##Iterations", &_cpc.warp_iterations, 1, 4, "%.d", 0);

                // Warp strength
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Strength");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::SliderInt("##Strength", &_cpc.warp_strength, 1, 8, "%.d", 0);

                // Primary Color
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Primary Color");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::ColorEdit3("##Primary Color", (float*)&_cpc.warp_primaryColor, 0);

                // Secondary Color
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Secondary Color");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::ColorEdit3("##Secondary Color", (float*)&_cpc.warp_secondaryColor, 0);

                // Color Shade
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Color Shade");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::SliderInt("##Color Shade", &_cpc.warp_colorShade, 1, 16, "%.d", 0);

                // Color Balance
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Color Balance");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::SliderFloat("##Color Balance", &_cpc.warp_colorBalance, -1.0, 8.0, "%.2f", 0);

                // Tint Color
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Tint Color");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::ColorEdit3("##Tint Color", (float*)&_cpc.warp_tintColor, 0);

                // Tint Shade
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Tint Shade");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::SliderInt("##Tint Shade", &_cpc.warp_tintShade, 1, 16, "%.d", 0);

                // Tint Strength
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Tint Strength");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::SliderFloat("##Tint Strength", &_cpc.warp_tintStrength, 0.0, 16.0, "%.2f", 0);

                ImGui::EndTable();
            }


            ImGui::EndGroup();
            ImGui::PopItemWidth();

            ImVec2 group_min = ImGui::GetItemRectMin();
            ImVec2 group_max = ImGui::GetItemRectMax();
            group_min.x -= margin;
            group_min.y -= margin / 2.0;
            group_max.y += margin / 2.0;

            drawList->ChannelsSetCurrent(0);

            // Background
            drawList->AddRectFilled(group_min, group_max, IM_COL32(47.5, 47.5, 47.5, 255), 0.0, 0.0);
            // Separator
            drawList->AddLine(ImVec2(group_min.x + 7.5, group_min.y + 22.0), ImVec2(group_max.x - 7.5, group_min.y + 22.0), IM_COL32(70.0, 70.0, 70.0, 255), 1.0);
        }

        ImGui::GetStyle().FrameBorderSize = 0.0;

        drawList->ChannelsMerge();

        ImGui::End();

        // Viewport
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

        ImGui::Begin("Viewport", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus);
        {
            // Set Window Size and Position
            ImGui::SetWindowSize(ImVec2(imGuiViewport->WorkSize.x - 300, imGuiViewport->WorkSize.y), 0);
            ImGui::SetWindowPos(ImVec2(imGuiViewport->WorkPos), 0);

            // Calculate image size and position and display vulkan render context
            ImVec2 viewportSize = ImVec2(imGuiViewport->WorkSize.x - 300.0, imGuiViewport->WorkSize.y);

            ImVec2 imageSize;
            ImVec2 imagePos;

            float imageAspectRatio = (float)_drawImage.imageExtent.width / (float)_drawImage.imageExtent.height;
            float viewportAspectRatio = (float)viewportSize.x / (float)viewportSize.y;

            if (imageAspectRatio > viewportAspectRatio)
            {
                imageSize.x = viewportSize.x;
                imageSize.y = viewportSize.x / imageAspectRatio;
            }
            else
            {
                imageSize.x = viewportSize.y * imageAspectRatio;
                imageSize.y = viewportSize.y;
            }

            imageSize.x = imageSize.x * _zoomFactor;
            imageSize.y = imageSize.y * _zoomFactor;

            if (_zoomFactor < 1.0f)
            {
                imagePos.x = (viewportSize.x - imageSize.x) / 2.0f;
                imagePos.y = (viewportSize.y - imageSize.y) / 2.0f;
            }
            else
            {
                imagePos.x = -(imageSize.x - viewportSize.x) / 2.0f;
                imagePos.y = -(imageSize.y - viewportSize.y) / 2.0f;
            }

            ImGui::SetCursorPos(imagePos);

            if (_drawImage.imageView != NULL)
            {
                ImGui::Image(_drawImageTexture, imageSize);
            }

            ImGui::End();

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            // Stats Window
            if (_statsOpened) {
                ImGui::Begin("Debug", NULL, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
                {
                    ImGui::SetWindowSize(ImVec2(200, 85), 0);
                    ImGui::SetWindowPos(ImVec2(0, 17.0), 0);

                    ImGui::Text("Performance: %.1fms / %.0ffps", _rta.getAvgRendertime(), ImGui::GetIO().Framerate);
                    ImGui::Text("Frames: %i", _renderedFrames);
                    ImGui::Text("Runtime: %.2f s", _cpc.time);

                    ImGui::End();
                }
            }
        }

        // About Popup
        if (menu_action == "About") ImGui::OpenPopup("About");
        if (ImGui::BeginPopupModal("About", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
            ImGui::SetWindowSize(ImVec2(300, 125), 0);
            ImGui::SetWindowPos(ImVec2((imGuiViewport->WorkSize.x - 300.0) / 2.0, (imGuiViewport->WorkSize.y - 125.0) / 2.0), 0);

            ImGui::Text(
                "Amber is a simple image generator that \n"
                "uses Vulkan to render unique visuals\n"
                "based on Warped Fractal Brownian Motion\n"
            );

            ImGui::Dummy(ImVec2(0.0, 3.0));
            
            ImGui::Text("Get more information on");
            ImGui::SameLine();
            ImGui::TextLinkOpenURL("Github", "https://github.com/its-nion/Amber");

            ImGui::SetCursorPos(ImVec2(270, 97.5));
            if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }

        // New Image Popup
        if (menu_action == "New") ImGui::OpenPopup("New Image");
        if (ImGui::BeginPopupModal("New Image", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
            ImGui::SetWindowSize(ImVec2(200, 100), 0);
            ImGui::SetWindowPos(ImVec2((imGuiViewport->WorkSize.x - 200.0) / 2.0, (imGuiViewport->WorkSize.y - 100.0) / 2.0), 0);

            static int inputImageWidth = 1280;
            static int inputImageHeight = 720;

            ImGui::InputInt("Width", &inputImageWidth, 0);
            ImGui::InputInt("Height", &inputImageHeight, 0);

            if (ImGui::Button("OK"))
            {
                _resizeDrawImage = true;

                _drawImage.imageExtent = VkExtent3D(inputImageWidth, inputImageHeight, 1);

                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::Render();
    }
}

void Renderer::initVulkan(char* appName, GLFWwindow* windowHandle)
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
    // We want a gpu that can write to the SDL surface and supports vulkan 1.3 with the correct features
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
    _logicalDevice = vkbDevice.device;

    // Get a Graphics queue with vkbootstrap
    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    // Initialize the memory allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _physicalDevice;
    allocatorInfo.device = _logicalDevice;
    allocatorInfo.instance = _instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &_allocator);
}

void Renderer::initSwapchain(GLFWwindow* windowHandle)
{
    int wWidth, wHeight;
    glfwGetWindowSize(windowHandle, &wWidth, &wHeight);

    create_swapchain(wWidth, wHeight);
}

void Renderer::initCommandStructure()
{
    // Create a Command Pool for commands submitted to the graphics queue
    VkCommandPoolCreateInfo commandPoolInfo = vkhelper::commandPoolCreateInfo(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    VK_CHECK(vkCreateCommandPool(_logicalDevice, &commandPoolInfo, nullptr, &_commandPool));

    for (int i = 0; i < 2; i++) {
        // Allocate Commandbuffers for each frame
        VkCommandBufferAllocateInfo cmdAllocInfo = vkhelper::commandBufferAllocateInfo(_commandPool, 1);

        VK_CHECK(vkAllocateCommandBuffers(_logicalDevice, &cmdAllocInfo, &_frames[i]._mainCommandBuffer));
    }

    // UI
    VK_CHECK(vkCreateCommandPool(_logicalDevice, &commandPoolInfo, nullptr, &_immCommandPool));

    VkCommandBufferAllocateInfo cmdAllocInfo = vkhelper::commandBufferAllocateInfo(_immCommandPool, 1);

    VK_CHECK(vkAllocateCommandBuffers(_logicalDevice, &cmdAllocInfo, &_immCommandBuffer));
}

void Renderer::initSyncStructure()
{
    VkFenceCreateInfo fenceCreateInfo = vkhelper::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = vkhelper::semaphoreCreateInfo();

    for (int i = 0; i < 2; i++) {
        VK_CHECK(vkCreateFence(_logicalDevice, &fenceCreateInfo, nullptr, &_frames[i]._renderFence)); // Controls when GPU has finished rendering a frame

        VK_CHECK(vkCreateSemaphore(_logicalDevice, &semaphoreCreateInfo, nullptr, &_frames[i]._swapchainSemaphore));
        VK_CHECK(vkCreateSemaphore(_logicalDevice, &semaphoreCreateInfo, nullptr, &_frames[i]._renderSemaphore));
    }
}

void Renderer::initImage(VkExtent3D dimension)
{
    //// Set Image Drawformat
    _drawImage.imageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
    _drawImage.imageExtent = dimension;

    VkImageUsageFlags drawImageUsages{};
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageCreateInfo rimg_info = vkhelper::imageCreateInfo(_drawImage.imageFormat, drawImageUsages, dimension);

    VmaAllocationCreateInfo rimg_allocinfo = {};
    rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    // Allocate and Create the Image
    vmaCreateImage(_allocator, &rimg_info, &rimg_allocinfo, &_drawImage.image, &_drawImage.allocation, nullptr);

    // Create Image-View with the Image to use for rendering
    VkImageViewCreateInfo rview_info = vkhelper::imageviewCreateInfo(_drawImage.imageFormat, _drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

    VK_CHECK(vkCreateImageView(_logicalDevice, &rview_info, nullptr, &_drawImage.imageView));
}

void Renderer::initDescriptors()
{
    // Create a descriptor pool that will hold 1 set with 1 image
    std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
    {
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
    };

    globalDescriptorAllocator.init_pool(_logicalDevice, 1, sizes);

    // Make the descriptor set layout for our compute draw
    {
        DescriptorLayoutBuilder builder;
        builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        _drawImageDescriptorLayout = builder.build(_logicalDevice, VK_SHADER_STAGE_COMPUTE_BIT);
    }

    // Allocate a descriptor set for our draw image
    _drawImageDescriptorSet = globalDescriptorAllocator.allocate(_logicalDevice, _drawImageDescriptorLayout);

    VkDescriptorImageInfo imgInfo{};
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imgInfo.imageView = _drawImage.imageView;

    VkWriteDescriptorSet drawImageWrite = {};
    drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    drawImageWrite.pNext = nullptr;

    drawImageWrite.dstBinding = 0;
    drawImageWrite.dstSet = _drawImageDescriptorSet;
    drawImageWrite.descriptorCount = 1;
    drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    drawImageWrite.pImageInfo = &imgInfo;

    vkUpdateDescriptorSets(_logicalDevice, 1, &drawImageWrite, 0, nullptr);
}

void Renderer::initComputePipeline()
{
    // Push Constants
    VkPushConstantRange pushConstant{};
    pushConstant.offset = 0;
    pushConstant.size = sizeof(ComputePushConstants);
    pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    // Layout
    VkPipelineLayoutCreateInfo computeLayout{};
    computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computeLayout.pNext = nullptr;
    computeLayout.pSetLayouts = &_drawImageDescriptorLayout;
    computeLayout.setLayoutCount = 1;

    computeLayout.pPushConstantRanges = &pushConstant;
    computeLayout.pushConstantRangeCount = 1;

    VK_CHECK(vkCreatePipelineLayout(_logicalDevice, &computeLayout, nullptr, &_computePipelineLayout));

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
    if (vkCreateShaderModule(_logicalDevice, &createInfo, nullptr, &computeDrawShader) != VK_SUCCESS) {
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
    computePipelineCreateInfo.layout = _computePipelineLayout;
    computePipelineCreateInfo.stage = stageinfo;

    VK_CHECK(vkCreateComputePipelines(_logicalDevice, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &_computePipeline));

    vkDestroyShaderModule(_logicalDevice, computeDrawShader, nullptr);
}

void Renderer::initImGui(GLFWwindow* windowHandle)
{
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VK_CHECK(vkCreateDescriptorPool(_logicalDevice, &pool_info, nullptr, &_imguiPool));

    // Initialize ImGui library
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(windowHandle, true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = _instance;
    init_info.PhysicalDevice = _physicalDevice;
    init_info.Device = _logicalDevice;
    init_info.Queue = _graphicsQueue;
    init_info.DescriptorPool = _imguiPool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.UseDynamicRendering = true;
    init_info.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &_swapchainImageFormat;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info);
    ImGui_ImplVulkan_CreateFontsTexture();

    // Sampler to write DrawImage to Texture on Viewport

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

    VK_CHECK(vkCreateSampler(_logicalDevice, &samplerInfo, nullptr, &_drawImageSampler));

    // Disable imgui ini file
    ImGui::GetIO().IniFilename = NULL;
    ImGui::GetIO().LogFilename = NULL;

    // Set Style
    ImGuiStyle* style = &ImGui::GetStyle();

    // Border
    style->WindowBorderSize = 0.0;

    ImVec4 primaryColor = ImVec4(1.0, 0.5, 0.0, 1.0);
    ImVec4 primaryHighlightColor = ImVec4(1.0, 0.7, 0.0, 1.0);
    ImVec4 darkColor = ImVec4(0.35, 0.25, 0.15, 1.0);
    ImVec4 darkHighlightColor = ImVec4(0.35, 0.25, 0.15, 1.0);

    // Menu Bar (Oben)
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2, 0.2, 0.2, 1.0); // BG
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.4, 0.4, 0.4, 1.0); // HOVERED

    // Properties Window
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.15, 0.15, 0.15, 1.0); // BG

    // Widgets
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.25, 0.25, 0.25, 1.0); // Border around inputs
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.35, 0.35, 0.35, 1.0);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.45, 0.45, 0.45, 1.0);
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.45, 0.45, 0.45, 1.0);

    style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.55, 0.55, 0.55, 1.0);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.65, 0.65, 0.65, 1.0);

    // Properties
    //style->Colors[ImGuiCol_FrameBg] = ImVec4(0.14, 0.14, 0.14, 1.0); // BG


    style->Colors[ImGuiCol_TitleBg] = primaryColor;
    style->Colors[ImGuiCol_TitleBgActive] = primaryColor;

    //style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.0, 0.0, 0.0, 1.0);
    //style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0, 0.0, 0.0, 1.0);

    style->Colors[ImGuiCol_Button] = darkColor;
    style->Colors[ImGuiCol_ButtonActive] = primaryHighlightColor;
    style->Colors[ImGuiCol_ButtonHovered] = primaryColor;

    //style->Colors[ImGuiCol_TabHovered] = highlightColor;
    //style->Colors[ImGuiCol_HeaderActive] = primaryColor;
}

void Renderer::initValues()
{
    _initTimePoint = std::chrono::steady_clock::now();

    _rta.initRendertimes(100);

    // Gui Values
    _cpc.uv_scale = 1.0;
    _cpc.fbm_octaves = 8;
    _cpc.fbm_amplitude = 0.5;
    _cpc.fbm_frequency = 1.0;
    _cpc.fbm_lacunarity = 1.0;
    _cpc.fbm_gain = 0.5;
    _cpc.fbm_shift = 2.0;
    _cpc.time = 0.0f;
    _cpc.warp_iterations = 3;
    _cpc.warp_strength = 3;
    _cpc.uv_offset = glm::vec2(0.0, 0.0);
    _cpc.warp_offset = glm::vec2(0.0, 0.0);
    _cpc.warp_primaryColor = glm::vec4(0.0, 0.0, 0.0, 0.0);
    _cpc.warp_secondaryColor = glm::vec4(0.9, 0.85, 0.67, 0.0);
    _cpc.warp_colorShade = 3;
    _cpc.warp_colorBalance = 0.0;
    _cpc.warp_tintColor = glm::vec4(0.0, 0.8, 1.0, 1.0);
    _cpc.warp_tintShade = 8;
    _cpc.warp_tintStrength = 0.1;
}

void Renderer::create_swapchain(uint32_t width, uint32_t height)
{
    vkb::SwapchainBuilder swapchainBuilder{ _physicalDevice,_logicalDevice,_surface };

    _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapchainBuilder
        .set_desired_format(VkSurfaceFormatKHR{ .format = _swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
        .set_desired_extent(width, height)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build()
        .value();

    _swapchainExtent = vkbSwapchain.extent;
    _swapchain = vkbSwapchain.swapchain;
    _swapchainImages = vkbSwapchain.get_images().value();
    _swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void Renderer::destroy_swapchain()
{
    vkDestroySwapchainKHR(_logicalDevice, _swapchain, nullptr);

    // Destroy swapchain resources
    for (int i = 0; i < _swapchainImageViews.size(); i++) {

        vkDestroyImageView(_logicalDevice, _swapchainImageViews[i], nullptr);
    }
}

void Renderer::resizeDrawImage()
{
    vkDeviceWaitIdle(_logicalDevice);

    vkDestroyImageView(_logicalDevice, _drawImage.imageView, nullptr);
    vmaDestroyImage(_allocator, _drawImage.image, _drawImage.allocation);

    globalDescriptorAllocator.destroy_pool(_logicalDevice);
    vkDestroyDescriptorSetLayout(_logicalDevice, _drawImageDescriptorLayout, nullptr);

    vkDestroyPipelineLayout(_logicalDevice, _computePipelineLayout, nullptr);
    vkDestroyPipeline(_logicalDevice, _computePipeline, nullptr);

    initImage(_drawImage.imageExtent);
    initDescriptors();
    initComputePipeline();

    if (_drawImageTexture != NULL)
    {
        ImGui_ImplVulkan_RemoveTexture(_drawImageTexture);
    }

    _drawImageTexture = ImGui_ImplVulkan_AddTexture(_drawImageSampler, _drawImage.imageView, VK_IMAGE_LAYOUT_GENERAL);

    _resizeDrawImage = false;
}

void Renderer::drawCommandsComputeShader(VkCommandBuffer cmd)
{
    // Bind the drawing compute pipeline
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _computePipeline);

    // Bind the descriptor set containing the draw image for the compute pipeline
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _computePipelineLayout, 0, 1, &_drawImageDescriptorSet, 0, nullptr);

    // Push constants to Shader
    vkCmdPushConstants(cmd, _computePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &_cpc);

    // Execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
    vkCmdDispatch(cmd, std::ceil(_drawExtent.width / 16.0), std::ceil(_drawExtent.height / 16.0), 1);
}

void Renderer::resizeSwapchain(GLFWwindow* windowHandle)
{
    vkDeviceWaitIdle(_logicalDevice);

    destroy_swapchain();

    int wWidth, wHeight;
    glfwGetWindowSize(windowHandle, &wWidth, &wHeight);

    create_swapchain(wWidth, wHeight);

    _resizeSwapchain = false;
}

void Renderer::drawCommandsImGui(VkCommandBuffer cmd, VkImageView targetImageView)
{
    VkRenderingAttachmentInfo colorAttachment = vkhelper::attachmentInfo(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = vkhelper::renderingInfo(_swapchainExtent, &colorAttachment, nullptr);

    vkCmdBeginRendering(cmd, &renderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    vkCmdEndRendering(cmd);
}

void Renderer::exportImage(std::string filename)
{
    vkDeviceWaitIdle(_logicalDevice);

    // Get layout of the image (including row pitch)
    VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
    VkSubresourceLayout subResourceLayout;
    vkGetImageSubresourceLayout(_logicalDevice, _drawImage.image, &subResource, &subResourceLayout);

    // Map image memory so we can start copying from it
    void* data;
    vmaMapMemory(_allocator, _drawImage.allocation, &data); 
    char* mappedData = static_cast<char*>(data) + subResourceLayout.offset;

    // Prepare a buffer to store the converted RGB data
    std::vector<uint8_t> imageData(_drawImage.imageExtent.width * _drawImage.imageExtent.height * 3); // 3 bytes per pixel (RGB)
    // Write pixel data row by row into the imageData buffer
    uint8_t* pixelPtr = imageData.data();
    
    // Loop through the image data row by row
    for (uint32_t y = 0; y < _drawImage.imageExtent.height; y++) {
        // Get a pointer to the current row (interpreted as 16-bit floating-point values)
        float* row = reinterpret_cast<float*>(mappedData);

        for (uint32_t x = 0; x < _drawImage.imageExtent.width; x++) {
            // Each pixel consists of 4 floats (R, G, B, A)
            float r = row[0]; // Red
            float g = row[1]; // Green
            float b = row[2]; // Blue
            // Optional: float a = row[3]; // Alpha if needed

            // Convert the float components to 8-bit values (clamping between 0.0f and 1.0f)
            uint8_t r8 = static_cast<uint8_t>(std::min(std::max(r, 0.0f), 1.0f) * 255.0f);
            uint8_t g8 = static_cast<uint8_t>(std::min(std::max(g, 0.0f), 1.0f) * 255.0f);
            uint8_t b8 = static_cast<uint8_t>(std::min(std::max(b, 0.0f), 1.0f) * 255.0f);

            // Store the RGB values in the image data buffer
            *pixelPtr++ = r8; // Red
            *pixelPtr++ = g8; // Green
            *pixelPtr++ = b8; // Blue

            // Move to the next pixel (4 floats per pixel: R, G, B, A)
            row += 4;
        }

        // Move to the next row, taking into account row pitch (potential padding)
        mappedData += subResourceLayout.rowPitch;
    }

    stbi_write_png(filename.c_str(), _drawImage.imageExtent.width, _drawImage.imageExtent.height, 3, imageData.data(), _drawImage.imageExtent.width * 3);

    // Unmap memory using VMA
    vmaUnmapMemory(_allocator, _drawImage.allocation);
}

