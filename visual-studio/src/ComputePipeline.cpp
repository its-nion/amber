#include "ComputePipeline.h"

//void ComputePipeline::initialize(VulkanContext& vulkanContext, DescriptorManager& descriptorManager) {
//    _logicalDevice = vulkanContext.getLogicalDevice();
//    // Compute pipeline initialization logic...
//}

void ComputePipeline::cleanup() {
    // Compute pipeline cleanup logic...
}

void ComputePipeline::initComputePipeline() {
    // Compute pipeline creation logic...
}

void ComputePipeline::drawCommandsComputeShader(VkCommandBuffer cmd, ComputePushConstants& cpc) {
    // Compute shader drawing logic...
}
