#pragma once

#include "util/libraries.h"

#include "Window.h"
#include "VkHelper.h"

class Ui {
public:
    Ui(GLFWwindow* windowHandle, VulkanData vbdata);
    ~Ui();

	void Update(RenderData& renderData, PushConstants& pc);
    void Render(RenderData& renderData);

private:
    void InitImGui(GLFWwindow* windowHandle, VulkanData vbdata);
    void SetImGuiStyle();

    bool m_StatsOpened;
    float m_ZoomFactor = 0.9;
    //bool _resizeDrawImage = false;

    VkDescriptorPool m_ImGuiPool;
};
