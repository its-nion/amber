#pragma once

#include "util/libraries.h"

#include "Window.h"
#include "VkHelper.h"

class Ui {
public:
    Ui(GLFWwindow* windowHandle, VulkanData vbdata);
    ~Ui();

	void Update(RenderData& renderData, PushConstants& pc, int renderedFrames, float renderTime);
    void Render(RenderData& renderData);

    void ImageExportFinished();

    UiTriggers GetUiTriggers();

private:
    void InitImGui(GLFWwindow* windowHandle, VulkanData vbdata);
    void SetImGuiStyle();

    int GetRandomUniformInt(int min, int max);
    float GetRandomUniformFloat(float min, float max);
    int GetRandomNormalInt(float mean, float stddev);
    float GetRandomNormalFloat(float mean, float stddev);

    bool m_IsExporting = false;

    bool m_StatsOpened = false;
    float m_ZoomFactor = 0.9;

	UiTriggers m_UiTriggers;

    VkDevice m_Device;
    VkDescriptorPool m_ImGuiPool;

    std::mt19937 m_Gen;
};
