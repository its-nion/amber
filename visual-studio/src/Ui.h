#pragma once

#include "util/libraries.h"

class Ui {
public:
    Ui(GLFWwindow* window, VkInstance instance, VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkRenderPass renderPass, uint32_t imageCount);
    ~Ui();
    void NewFrame();
    void Render(VkCommandBuffer commandBuffer);
    void Cleanup();

private:
    void InitImGui(VkRenderPass renderPass, uint32_t imageCount);
    void CleanupImGui();

    GLFWwindow* window;
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkQueue graphicsQueue;
    VkDescriptorPool descriptorPool;
};
