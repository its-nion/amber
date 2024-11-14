#pragma once

// Check if app is in debug mode
#ifdef NDEBUG
constexpr bool ISDEBUG = false;
#else
constexpr bool ISDEBUG = true;
#endif

// Vulkan command check
#define VK_CHECK(x)                                                             \
    do {                                                                        \
        VkResult err = x;                                                       \
        if (err)                                                                \
        {                                                                       \
            std::cout << "Detected Vulkan error: " << string_VkResult(err);     \
            abort();                                                            \
        }                                                                       \
    } while (0)
