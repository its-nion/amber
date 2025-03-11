#include "Ui.h"

Ui::Ui(GLFWwindow* windowHandle, VulkanData vbdata)
{
	InitImGui(windowHandle, vbdata);

	SetImGuiStyle();
}

Ui::~Ui()
{
}

void Ui::Update(RenderData& renderData, PushConstants& pc)
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
                if (renderData.offscreenImage.imageView == NULL)
                {
                    menu_action = "NoImage";
                }
                else
                {
                    /*const char* filterPatterns[] = { "*.png" };
                    char* path = tinyfd_saveFileDialog("Export", "Example", 1, filterPatterns, nullptr);

                    if (path != NULL) exportImage(path);*/
                }
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Show"))
        {
            ImGui::MenuItem("Stats", NULL, &m_StatsOpened);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Zoom"))
        {
            if (ImGui::MenuItem("50%")) m_ZoomFactor = 0.50;
            if (ImGui::MenuItem("70%")) m_ZoomFactor = 0.70;
            if (ImGui::MenuItem("90%")) m_ZoomFactor = 0.90;
            if (ImGui::MenuItem("100%")) m_ZoomFactor = 1.00;
            if (ImGui::MenuItem("110%")) m_ZoomFactor = 1.10;
            if (ImGui::MenuItem("130%")) m_ZoomFactor = 1.30;
            if (ImGui::MenuItem("150%")) m_ZoomFactor = 1.50;
            ImGui::EndMenu();
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0, 0.0, 0.0, 0.0));
        if (ImGui::Button("Help")) menu_action = "Help";
        ImGui::PopStyleColor();

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

        // UV
        {
            float margin = 10.0f;
            ImVec2 cursor_start = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(cursor_start.x + margin, cursor_start.y + margin / 2.0));
            ImGui::PushItemWidth(300);
            ImGui::BeginGroup();

            drawList->ChannelsSetCurrent(1);

            ImGui::Text("UV");
            ImGui::SameLine();
            ImGui::SetCursorPosX(228.0);
            if (ImGui::SmallButton("Default##UV"))
            {
                pc.uv_scale = 2.0;
                pc.uv_offset = glm::vec2(0.0, 0.0);
            };

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
                ImGui::DragFloat("##Scale", &pc.uv_scale, 0.01f, 0.0f, 100.0f, "%.2f", ImGuiSliderFlags_ClampOnInput);

                // Offset
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Offset");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::DragFloat2("##Offset", (float*)&pc.uv_offset, 0.01f, -1000.0f, 1000.0f, "%.2f", ImGuiSliderFlags_ClampOnInput);

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
            ImGui::SameLine();
            ImGui::SetCursorPosX(228.0);
            if (ImGui::SmallButton("Default##FBM"))
            {
                pc.fbm_octaves = 8;
                pc.fbm_amplitude = 0.5;
                pc.fbm_frequency = 1.0;
                pc.fbm_lacunarity = 1.0;
                pc.fbm_gain = 0.5;
                pc.fbm_shift = 2.0;
            };

            ImGui::Dummy(ImVec2(0.0, 1.0));

            if (ImGui::BeginTable("fbmTable", 2, 0))
            {
                ImGui::TableSetupColumn("fbmOne", ImGuiTableColumnFlags_WidthFixed, 115.0f);
                ImGui::TableSetupColumn("fbmTwo", ImGuiTableColumnFlags_WidthFixed, 145.0f);

                // Octaves
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Octaves");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::SliderInt("##Octaves", &pc.fbm_octaves, 1, 10, "%d", 0);

                // Amplitude
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Amplitude");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::DragFloat("##Amplitude", &pc.fbm_amplitude, 0.001, 0.0, 10.0, "%.3f", 0);

                // Frequenzy
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Frequenzy");
                ImGui::SameLine();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::DragFloat("##Frequenzy", &pc.fbm_frequency, 0.001, 0.0, 10.0, "%.03f", 0);

                // Lacunarity
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Lacunarity");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::DragFloat("##Lacunarity", &pc.fbm_lacunarity, 0.001, 0.0, 10.0, "%.03f", 0);

                // Gain
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Gain");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::DragFloat("##Gain", &pc.fbm_gain, 0.001, 0.0, 10.0, "%.03f", 0);

                // Shift
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Shift");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::DragFloat("##Shift", &pc.fbm_shift, 0.001, 0.0, 10.0, "%.03f", 0);

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
            ImGui::SameLine();
            ImGui::SetCursorPosX(228.0);
            if (ImGui::SmallButton("Default##WARP"))
            {
                pc.warp_iterations = 3;
                pc.warp_strength = 3;
                pc.warp_offset = glm::vec2(1.0, 1.0);
                pc.warp_primaryColor = glm::vec4(0.04, 0.02, 0.0, 0.0);
                pc.warp_secondaryColor = glm::vec4(0.9, 0.85, 0.67, 0.0);
                pc.warp_colorBalance = 2;
                pc.warp_tintColor = glm::vec4(0.0, 0.8, 1.0, 1.0);
                pc.warp_tintShade = 8;
                pc.warp_tintStrength = 0.5;
            };

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
                ImGui::SliderInt("##Iterations", &pc.warp_iterations, 1, 4, "%.d", 0);

                // Warp strength
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Strength");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::SliderInt("##Strength", &pc.warp_strength, 1, 6, "%.d", 0);

                // Warp Offset
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Offset");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::DragFloat2("##Offset", (float*)&pc.warp_offset, 0.001f, -1000.0f, 1000.0f, "%.3f", ImGuiSliderFlags_ClampOnInput);

                ImGui::Dummy(ImVec2(0, 5.0));

                // Primary Color
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Primary Color");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::ColorEdit3("##Primary Color", (float*)&pc.warp_primaryColor, ImGuiColorEditFlags_NoInputs);

                // Secondary Color
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Secondary Color");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::ColorEdit3("##Secondary Color", (float*)&pc.warp_secondaryColor, ImGuiColorEditFlags_NoInputs);

                // Color Balance
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Color Balance");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::SliderInt("##Color Balance", &pc.warp_colorBalance, 1, 3, "%.d", 0);

                ImGui::Dummy(ImVec2(0, 5.0));

                // Tint Color
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Tint Color");
                ImGui::SameLine();
                ImGui::CustomHelpMarker("Can be disabled with Tint Strength = 0");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::ColorEdit3("##Tint Color", (float*)&pc.warp_tintColor, ImGuiColorEditFlags_NoInputs);

                // Tint Shade
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Tint Spread");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::SliderInt("##Tint Shade", &pc.warp_tintShade, 1, 16, "%.d", 0);

                // Tint Strength
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Tint Strength");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::SliderFloat("##Tint Strength", &pc.warp_tintStrength, 0.0, 1.0, "%.2f", 0);

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

            float imageAspectRatio = (float)renderData.offscreenImage.imageExtent.width / (float)renderData.offscreenImage.imageExtent.height;
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

            imageSize.x = imageSize.x * m_ZoomFactor;
            imageSize.y = imageSize.y * m_ZoomFactor;

            if (m_ZoomFactor < 1.0f)
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

            // Show rendered image or text
            if (renderData.offscreenImage.imageView != NULL)
            {
                ImGui::Image(renderData.shaderImageTexture, imageSize);
            }
            else
            {
                ImGui::SetCursorPos(ImVec2(viewportSize.x / 2.0 - 175.0, viewportSize.y / 2.0 - 15.0));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3, 0.3, 0.3, 1.0));
                ImGui::SetWindowFontScale(1.5f);
                ImGui::Text("Create new image under File / New");
                ImGui::SetWindowFontScale(1.0f);
                ImGui::PopStyleColor();
            }

            ImGui::End();

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            // Stats Window
            if (m_StatsOpened) {
                ImGui::Begin("Debug", NULL, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
                {
                    ImGui::SetWindowSize(ImVec2(200, 85), 0);
                    ImGui::SetWindowPos(ImVec2(0, 17.0), 0);

                    /*ImGui::Text("Performance: %.1fms / %.0ffps", _rta.getAvgRendertime(), ImGui::GetIO().Framerate);
                    ImGui::Text("Frames: %i", _renderedFrames);
                    ImGui::Text("Runtime: %.2f s", _cpc.time);*/

                    ImGui::Text("TEST");

                    ImGui::End();
                }
            }
        }

        // No Image Error
        if (menu_action == "NoImage") ImGui::OpenPopup("No Image Error");
        if (ImGui::BeginPopupModal("No Image Error", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
            ImGui::SetWindowSize(ImVec2(300, 100), 0);
            ImGui::SetWindowPos(ImVec2((imGuiViewport->WorkSize.x - 300.0) / 2.0, (imGuiViewport->WorkSize.y - 130.0) / 2.0), 0);

            ImGui::Text(

                "Please create a new Image before\n"
                "trying to export!\n"
            );

            ImGui::SetCursorPos(ImVec2(217.5, 72.5));
            if (ImGui::Button("OK", ImVec2(ImGui::GetWindowSize().x * 0.25f, 0.0f))) ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }

        // Help Popup
        if (menu_action == "Help") ImGui::OpenPopup("Help");
        if (ImGui::BeginPopupModal("Help", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
            ImGui::SetWindowSize(ImVec2(450, 160), 0);
            ImGui::SetWindowPos(ImVec2((imGuiViewport->WorkSize.x - 450.0) / 2.0, (imGuiViewport->WorkSize.y - 160.0) / 2.0), 0);

            ImGui::Text(
                "- Each property can be fine tuned with ctrl + left click\n"
                "- Properties without sliders can be changed by clicking and\n  dragging them to the left/right\n"
                "- Exporting images may freeze the app for a short amount of\n  time\n"
                "- Unrealistic resolutions may crash the app\n"
                "- Color Boxes can be clicked for precise changes"
            );

            ImGui::SetCursorPos(ImVec2(330.0, 132.5));
            if (ImGui::Button("OK", ImVec2(ImGui::GetWindowSize().x * 0.25f, 0.0f))) ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }

        // About Popup
        if (menu_action == "About") ImGui::OpenPopup("About");
        if (ImGui::BeginPopupModal("About", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
            ImGui::SetWindowSize(ImVec2(300, 130), 0);
            ImGui::SetWindowPos(ImVec2((imGuiViewport->WorkSize.x - 300.0) / 2.0, (imGuiViewport->WorkSize.y - 130.0) / 2.0), 0);

            ImGui::Text(
                "Amber is a simple image generator that \n"
                "uses Vulkan to render unique visuals\n"
                "based on Warped Fractal Brownian Motion\n"
            );

            ImGui::Dummy(ImVec2(0.0, 3.0));

            ImGui::Text("Get more information on");
            ImGui::SameLine();
            ImGui::TextLinkOpenURL("Github", "https://github.com/its-nion/Amber");

            ImGui::SetCursorPos(ImVec2(217.5, 102.5));
            if (ImGui::Button("OK", ImVec2(ImGui::GetWindowSize().x * 0.25f, 0.0f))) ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }

        // New Image Popup
        if (menu_action == "New") ImGui::OpenPopup("New Image");
        if (ImGui::BeginPopupModal("New Image", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
            ImGui::SetWindowSize(ImVec2(200, 105), 0);
            ImGui::SetWindowPos(ImVec2((imGuiViewport->WorkSize.x - 200.0) / 2.0, (imGuiViewport->WorkSize.y - 100.0) / 2.0), 0);

            static int inputImageWidth = 1280;
            static int inputImageHeight = 720;

            if (ImGui::BeginTable("imageSizeTable", 2, 0))
            {
                ImGui::TableSetupColumn("sizeOne", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                ImGui::TableSetupColumn("sizeTwo", ImGuiTableColumnFlags_WidthFixed, 125.0f);

                // Width
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Width");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::InputInt("##Width", &inputImageWidth, 0);

                // Height
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Height");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1.0);
                ImGui::InputInt("##Height", &inputImageHeight, 0);

                ImGui::EndTable();
            }

            ImGui::Dummy(ImVec2(0.0, 3.0));

            ImGui::SetCursorPos(ImVec2(82.5, 80));

            if (ImGui::Button("OK", ImVec2(ImGui::GetWindowSize().x * 0.25f, 0.0f)))
            {
                //_resizeDrawImage = true;

                renderData.offscreenImage.imageExtent = VkExtent3D(inputImageWidth, inputImageHeight, 1);

                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(ImGui::GetWindowSize().x * 0.25f, 0.0f)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::Render();
    }
}

void Ui::Render(RenderData& renderData)
{
    VkRenderingAttachmentInfo colorAttachment = vkhelper::attachmentInfo(renderData.swapchainImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = vkhelper::renderingInfo(renderData.swapchainExtent, &colorAttachment, nullptr);

    vkCmdBeginRendering(renderData.commandBuffer, &renderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderData.commandBuffer);

    vkCmdEndRendering(renderData.commandBuffer);
}

void Ui::InitImGui(GLFWwindow* windowHandle, VulkanData vbdata)
{
    //VK_CHECK(vkCreateCommandPool(_logicalDevice, &commandPoolInfo, nullptr, &_immCommandPool));

    //VkCommandBufferAllocateInfo cmdAllocInfo = vkhelper::commandBufferAllocateInfo(_immCommandPool, 1);

    //VK_CHECK(vkAllocateCommandBuffers(_logicalDevice, &cmdAllocInfo, &_immCommandBuffer));

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

    VK_CHECK(vkCreateDescriptorPool(vbdata.device, &pool_info, nullptr, &m_ImGuiPool));

    // Initialize ImGui library
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(windowHandle, true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = vbdata.instance;
    init_info.PhysicalDevice = vbdata.physicalDevice;
    init_info.Device = vbdata.device;
    init_info.Queue = vbdata.graphicsQueue;
    init_info.DescriptorPool = m_ImGuiPool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.UseDynamicRendering = true;
    init_info.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &vbdata.swapchainImageFormat;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info);
    ImGui_ImplVulkan_CreateFontsTexture();
}

void Ui::SetImGuiStyle()
{
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
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2, 0.2, 0.2, 1.0);
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.4, 0.4, 0.4, 1.0);

    // Properties Window
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.15, 0.15, 0.15, 1.0);

    // Widgets
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.25, 0.25, 0.25, 1.0);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.35, 0.35, 0.35, 1.0);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.45, 0.45, 0.45, 1.0);
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.45, 0.45, 0.45, 1.0);

    style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.55, 0.55, 0.55, 1.0);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.65, 0.65, 0.65, 1.0);

    // Popup
    style->Colors[ImGuiCol_TitleBg] = ImVec4(0.15, 0.15, 0.15, 1.0);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15, 0.15, 0.15, 1.0);

    style->Colors[ImGuiCol_PopupBg] = ImVec4(0.2, 0.2, 0.2, 1.0);

    // Button
    style->Colors[ImGuiCol_Button] = ImVec4(0.30, 0.30, 0.30, 1.0);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.35, 0.35, 0.35, 1.0);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.4, 0.4, 0.4, 1.0);
}
