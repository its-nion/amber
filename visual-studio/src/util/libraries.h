#pragma once

// STANDARD LIBRARIES

#include <iostream>
#include <stdexcept>
#include <memory>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <functional>
#include <deque>
#include <span>
#include <fstream>
#include <Windows.h>

// THIRD PARTY LIBRARIES

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#define GLFW_INCLUDE_VULKAN
#include "glfw/glfw3.h"

#include "glm/glm.hpp"
#include "glm/vec4.hpp"

#include "vkbootstrap/VkBootstrap.h"

#include "vma/vk_mem_alloc.h"

// LOCAL LIBRARIES

#include "../util/preprocessor-directives.h"
#include "../util/structs.h"
#include "../util/imgui-additional-widgets.h"