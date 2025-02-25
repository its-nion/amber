#pragma once

#include "util/libraries.h"

/// <summary>
/// A simple GLFW wrapper for window creation and management
/// </summary>
class Window {
public:
    Window(const char* name, int width, int height);
	~Window();

    void PollEvents() const;
    bool ShouldClose() const;

    GLFWwindow* GetWindowHandle() const;

private:
    GLFWwindow* windowHandle;

    void centerWindow();
	void setWindowIcon();
};
