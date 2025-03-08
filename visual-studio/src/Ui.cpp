#include "Ui.h"

Ui::Ui(GLFWwindow* window, VkInstance instance, VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkRenderPass renderPass, uint32_t imageCount)
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
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.35, 0.35, 0.35, 1.0);;
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.4, 0.4, 0.4, 1.0);;
}

Ui::~Ui()
{
}

void Ui::NewFrame()
{
}

void Ui::Render(VkCommandBuffer commandBuffer)
{
}

void Ui::Cleanup()
{
}

void Ui::InitImGui(VkRenderPass renderPass, uint32_t imageCount)
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
}

void Ui::CleanupImGui()
{
}
