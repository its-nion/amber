
#pragma once

#define VMA_IMPLEMENTATION

#include "VkHelper.h"

class Renderer {
public:
    void initialize(char* appName, GLFWwindow* windowHandle);
    void render(GLFWwindow* windowHandle);
    void cleanup();

private:
    bool _isInitialized{ false };
    int _renderedFrames{ 0 };
    bool _resizeSwapchain = false;

    bool _statsOpened;
    float _zoomFactor = 0.75;
    bool _resizeDrawImage = false;

    // Frames
    FrameData _frames[2];
    FrameData& getCurrentFrame() { return _frames[_renderedFrames % 2]; };

    // Vulkan base
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;
    VkPhysicalDevice _physicalDevice;
    VkDevice _logicalDevice;

    // Vulkan display
    VkSurfaceKHR _surface;
    VkSwapchainKHR _swapchain;
    VkFormat _swapchainImageFormat;
    std::vector<VkImage> _swapchainImages;
    std::vector<VkImageView> _swapchainImageViews;
    VkExtent2D _swapchainExtent;

    // Queues
    VkQueue _graphicsQueue;
    uint32_t _graphicsQueueFamily;

    // Commandpool
    VkCommandPool _commandPool;

    // Vma
    VmaAllocator _allocator;

    // Own image
    AllocatedImage _drawImage;
    VkExtent2D _drawExtent;

    // Descriptor memory management
    DescriptorAllocator globalDescriptorAllocator;

    // Image Descriptor
    VkDescriptorSet _drawImageDescriptorSet;
    VkDescriptorSetLayout _drawImageDescriptorLayout;

    // Pipeline
    VkPipeline _computePipeline;
    VkPipelineLayout _computePipelineLayout;

    // ImGui
    VkDescriptorPool _imguiPool;
    VkSampler _drawImageSampler;
    VkDescriptorSet _drawImageTexture;

    // Gui synchronisation
    VkCommandBuffer _immCommandBuffer;
    VkCommandPool _immCommandPool;

    // Vars
    RenderTimeArray _rta;
    std::chrono::steady_clock::time_point _initTimePoint;
    ComputePushConstants _cpc;

    // Methods
    void initVulkan(char* appName, GLFWwindow* windowHandle);
    void initSwapchain(GLFWwindow* windowHandle);
    void initCommandStructure();
    void initSyncStructure();
    void initImage(VkExtent3D dimension);
    void initDescriptors();
    void initComputePipeline();
    void initImGui(GLFWwindow* windowHandle);

    void initValues();

    void create_swapchain(uint32_t width, uint32_t height);
    void resizeSwapchain(GLFWwindow* windowHandle);
    void destroy_swapchain();

    void resizeDrawImage();

    void draw();
    void drawImGui();
    void drawCommandsComputeShader(VkCommandBuffer cmd);
    void drawCommandsImGui(VkCommandBuffer cmd, VkImageView targetImageView);

    void exportImage(std::string filename);

};

