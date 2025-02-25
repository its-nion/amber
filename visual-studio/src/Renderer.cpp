//#include "Renderer.h"
//
//void Renderer::initialize(char* appName, GLFWwindow* windowHandle) {
//    _vulkanContext.initialize(appName, windowHandle);
//    createSwapchain(windowHandle);
//    _descriptorManager.initialize(_vulkanContext);
//    _computePipeline.initialize(_vulkanContext, _descriptorManager);
//    _userInterface.initialize(_vulkanContext, windowHandle);
//
//    initValues();
//    _isInitialized = true;
//}
//
//void Renderer::render(GLFWwindow* windowHandle) {
//    if (_resizeSwapchain) resizeSwapchain(windowHandle);
//    if (_resizeDrawImage) resizeDrawImage();
//
//    _userInterface.drawImGui();
//    draw();
//}
//
//void Renderer::cleanup() {
//    if (_isInitialized) {
//        vkDeviceWaitIdle(_vulkanContext.getLogicalDevice());
//        _userInterface.cleanup();
//        _computePipeline.cleanup();
//        _descriptorManager.cleanup();
//        destroySwapchain();
//        _vulkanContext.cleanup();
//    }
//}
//
//void Renderer::initValues() {
//    _initTimePoint = std::chrono::steady_clock::now();
//    _rta.initRendertimes(100);
//    // Initialize other values...
//}
//
//void Renderer::createSwapchain(uint32_t width, uint32_t height) {
//    _vulkanContext.createSwapchain(width, height);
//}
//
//void Renderer::resizeSwapchain(GLFWwindow* windowHandle) {
//    vkDeviceWaitIdle(_vulkanContext.getLogicalDevice());
//    destroySwapchain();
//    int wWidth, wHeight;
//    glfwGetWindowSize(windowHandle, &wWidth, &wHeight);
//    createSwapchain(wWidth, wHeight);
//    _resizeSwapchain = false;
//}
//
//void Renderer::destroySwapchain() {
//    _vulkanContext.destroySwapchain();
//}
//
//void Renderer::resizeDrawImage() {
//    vkDeviceWaitIdle(_vulkanContext.getLogicalDevice());
//    _descriptorManager.cleanupDrawImage();
//    _computePipeline.cleanup();
//    _descriptorManager.initImage(_drawImage.imageExtent);
//    _descriptorManager.initDescriptors();
//    _computePipeline.initComputePipeline();
//    _resizeDrawImage = false;
//}
//
//void Renderer::draw() {
//    // Drawing logic...
//}
//
//void Renderer::drawImGui() {
//    _userInterface.drawImGui();
//}
//
//void Renderer::drawCommandsComputeShader(VkCommandBuffer cmd) {
//    _computePipeline.drawCommandsComputeShader(cmd, _cpc);
//}
//
//void Renderer::drawCommandsImGui(VkCommandBuffer cmd, VkImageView targetImageView) {
//    _userInterface.drawCommandsImGui(cmd, targetImageView);
//}
//
//void Renderer::exportImage(std::string filename) {
//    // Export image logic...
//}
