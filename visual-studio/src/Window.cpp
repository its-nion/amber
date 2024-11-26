
#include "Window.h"
#include "../embedded-resources/amber.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

void Window::create(const char* name, const int width, const int height)
{
    glfwInit();

    // Window Flags
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Create Window
    _windowHandle = glfwCreateWindow(width, height, name, nullptr, nullptr);

    // Window Properties
    glfwSetWindowSizeLimits(_windowHandle, 640, 360, GLFW_DONT_CARE, GLFW_DONT_CARE);

    // Window Position
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    // Get the window size
    int windowWidth, windowHeight;
    glfwGetWindowSize(_windowHandle, &windowWidth, &windowHeight);

    int posX = (mode->width - windowWidth) / 2;
    int posY = (mode->height - windowHeight) / 2;

    glfwSetWindowPos(_windowHandle, posX, posY);

    // Make the window's context current
    glfwMakeContextCurrent(_windowHandle);

    // Window Icon
    const bin2cpp::File& pngFile = bin2cpp::getAmberPngFile();
    size_t pngSize = pngFile.getSize();
    const unsigned char* pngData = reinterpret_cast<const unsigned char*>(pngFile.getBuffer());

    int pngWidth, pngHeight, pngChannels;
    unsigned char* data = stbi_load_from_memory(pngData, static_cast<int>(pngSize), &pngWidth, &pngHeight, &pngChannels, 4);

    GLFWimage icon = { pngWidth, pngHeight, data };
    glfwSetWindowIcon(_windowHandle, 1, &icon);
}

void Window::destroy()
{
    glfwDestroyWindow(_windowHandle);
    glfwTerminate();
}

bool Window::shouldClose()
{
    return (glfwWindowShouldClose(_windowHandle));
}

void Window::update()
{
    glfwPollEvents();
}

GLFWwindow* Window::getWindowHandle()
{
    return _windowHandle;
}
