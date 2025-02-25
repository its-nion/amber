//#pragma once
//
//#include "VulkanContext.h"
//#include "ShaderManager.h"
//#include "DescriptorManager.h"
//#include "ComputePipeline.h"
//#include "UserInterface.h"
//
//class Renderer {
//public:
//    void initialize(char* appName, GLFWwindow* windowHandle);
//    void render(GLFWwindow* windowHandle);
//    void cleanup();
//
//private:
//    bool _isInitialized{ false };
//    int _renderedFrames{ 0 };
//    bool _resizeSwapchain = false;
//    bool _resizeDrawImage = false;
//
//    VulkanContext _vulkanContext;
//    ShaderManager _shaderManager;
//    DescriptorManager _descriptorManager;
//    ComputePipeline _computePipeline;
//    UserInterface _userInterface;
//
//    // Frames
//    FrameData _frames[2];
//    FrameData& getCurrentFrame() { return _frames[_renderedFrames % 2]; };
//
//    // Own image
//    AllocatedImage _drawImage;
//    VkExtent2D _drawExtent;
//
//    // Vars
//    RenderTimeArray _rta;
//    std::chrono::steady_clock::time_point _initTimePoint;
//    ComputePushConstants _cpc;
//
//    // Methods
//    void initValues();
//    void createSwapchain(uint32_t width, uint32_t height);
//    void resizeSwapchain(GLFWwindow* windowHandle);
//    void destroySwapchain();
//    void resizeDrawImage();
//    void draw();
//    void drawImGui();
//    void drawCommandsComputeShader(VkCommandBuffer cmd);
//    void drawCommandsImGui(VkCommandBuffer cmd, VkImageView targetImageView);
//    void exportImage(std::string filename);
//};
