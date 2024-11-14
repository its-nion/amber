
#pragma once

#include "util/libraries.h"

class Window
{
    public:
        void create(const char* name, const int width, const int height);
        void destroy();

        bool shouldClose();
        void update();

        GLFWwindow* getWindowHandle();

    private:
        GLFWwindow* _windowHandle;
};

