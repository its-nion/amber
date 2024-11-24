
#include "Window.h"

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
    GLFWimage images[1];
    images[0].pixels = stbi_load("icon/Amber.png", &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(_windowHandle, 1, images);
    stbi_image_free(images[0].pixels);
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
