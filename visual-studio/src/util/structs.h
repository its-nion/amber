#pragma once

enum ParameterPresets {
    NOTHING,
    OCEAN,
    CELESTE,
    HARMONY,
    WETLANDS,
    CRIMSON
};

struct UiTriggers {
	bool resizeDrawImage = false;
    VkExtent3D drawImageDimension = {0,0,0};
    bool exportImage = false;
    char* exportImagePath;
    int changeParameterPreset = 0;
};

struct PushConstants {
    float uv_scale;
    int fbm_octaves;
    float fbm_amplitude;
    float fbm_frequency;
    float fbm_lacunarity;
    float fbm_gain;
    float fbm_shift;
    float time;
    int warp_iterations;
    int warp_strength;
    int warp_colorShade;
    int warp_tintShade;
    int warp_colorBalance;
    float warp_tintStrength;
    float th;
    float fo;
    glm::vec2 uv_offset;
    glm::vec2 warp_offset;
    glm::vec4 warp_primaryColor;
    glm::vec4 warp_secondaryColor;
    glm::vec4 warp_tintColor;
};

struct RenderTimeArray {
    std::vector<float> rendertimeVec;
    std::vector<float>::iterator currentIt;

    void initRendertimes(int span) 
    {
        for (int i = 0; i < span; i++)
            rendertimeVec.push_back(0.0f);

        currentIt = rendertimeVec.begin();
    }

    void pushRendertime(float rendertime)
    {
        if (currentIt >= rendertimeVec.end() - 1)
            currentIt = rendertimeVec.begin();
        else
            currentIt++;

        *currentIt = rendertime;
    }

    float getAvgRendertime()
    {
        float sum = 0.0f;

        for (int i = 0; i < rendertimeVec.size(); i++)
        {
            sum += rendertimeVec[i];
        }

        return sum / rendertimeVec.size();
    }
};

struct AllocatedImage
{
    VkImage image;
    VkImageView imageView;
    VmaAllocation allocation;
    VkExtent3D imageExtent;
    VkFormat imageFormat;
};

struct FrameData {
    // Record commands
    VkCommandBuffer mainCommandBuffer;

    // Synchronisation
    VkSemaphore swapchainSemaphore, renderSemaphore;
    VkFence renderFence;
};

struct RenderData {
    VkCommandBuffer commandBuffer;
	AllocatedImage offscreenImage;
	unsigned int swapchainImageIndex;
	VkImageView swapchainImageView;
    VkExtent2D swapchainExtent;
    VkDescriptorSet shaderImageTexture;
};

struct VulkanData {
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkQueue graphicsQueue;
    VkFormat swapchainImageFormat;
};