#pragma once

#include "VkHelper.h"

class DescriptorManager {
public:
    /*void initialize(VulkanContext& vulkanContext);*/
    void cleanup();
    void initImage(VkExtent3D dimension);
    void initDescriptors();
    void cleanupDrawImage();

private:
    VkDevice _logicalDevice;
    VmaAllocator _allocator;
    DescriptorAllocator _globalDescriptorAllocator;
    VkDescriptorSet _drawImageDescriptorSet;
    VkDescriptorSetLayout _drawImageDescriptorLayout;
};
