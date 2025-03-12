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

    UiTriggers GetUiTriggers();

private:
    void InitImGui(GLFWwindow* windowHandle, VulkanData vbdata);
    void SetImGuiStyle();

    bool m_StatsOpened;
    float m_ZoomFactor = 0.9;

	UiTriggers m_UiTriggers;

    VkDevice m_Device;
    VkDescriptorPool m_ImGuiPool;
};
