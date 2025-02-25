#pragma once

#include "VkHelper.h"

class ComputePipeline {
public:
    //void initialize(VulkanContext& vulkanContext, DescriptorManager& descriptorManager);
    void cleanup();
    void initComputePipeline();
    void drawCommandsComputeShader(VkCommandBuffer cmd, ComputePushConstants& cpc);

private:
    VkDevice _logicalDevice;
    VkPipeline _computePipeline;
    VkPipelineLayout _computePipelineLayout;
};
