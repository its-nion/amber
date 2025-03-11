#include "Window.h"

#define GLFW_INCLUDE_VULKAN
#include "glfw/glfw3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "../embedded-resources/amber.h"

Window::Window(const char* name, int width, int height)
{
    // Initialize GLFW
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    // Set window Params
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Create window
    m_WindowHandle = glfwCreateWindow(width, height, name, nullptr, nullptr);

    if (!m_WindowHandle) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    // Adjust window
    glfwSetWindowSizeLimits(m_WindowHandle, 640, 360, GLFW_DONT_CARE, GLFW_DONT_CARE);

    CenterWindow();

    SetWindowIcon();

    glfwMakeContextCurrent(m_WindowHandle);
}

Window::~Window()
{
    if (m_WindowHandle) {
        glfwDestroyWindow(m_WindowHandle);
    }

    glfwTerminate();
}

void Window::PollEvents() const {
    glfwPollEvents();
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(m_WindowHandle);
}

GLFWwindow* Window::GetWindowHandle() const {
    return m_WindowHandle;
}

void Window::CenterWindow()
{
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    int windowWidth, windowHeight;
    glfwGetWindowSize(m_WindowHandle, &windowWidth, &windowHeight);

    int posX = (mode->width - windowWidth) / 2;
    int posY = (mode->height - windowHeight) / 2;

    glfwSetWindowPos(m_WindowHandle, posX, posY);
}

void Window::SetWindowIcon()
{
    const bin2cpp::File& pngFile = bin2cpp::getAmberPngFile();
    size_t pngSize = pngFile.getSize();
    const unsigned char* pngData = reinterpret_cast<const unsigned char*>(pngFile.getBuffer());

    int pngWidth, pngHeight, pngChannels;
    unsigned char* data = stbi_load_from_memory(pngData, static_cast<int>(pngSize), &pngWidth, &pngHeight, &pngChannels, 4);

    if (data) {
        GLFWimage icon = { pngWidth, pngHeight, data };
        glfwSetWindowIcon(m_WindowHandle, 1, &icon);
        stbi_image_free(data);
    }
    else {
        throw std::runtime_error("Failed to load window icon");
    }
}
