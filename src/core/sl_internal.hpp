#pragma once

#include <starlight/starlight.h>
#include <string>
#include <vector>
#include <memory>
#include "sl_window.hpp"
#include "api/vulkan/vk_internal.hpp"

// Internal C++ definition backing public slInstance handle
struct slInstance_t {
    bool initialized{false};
    slGraphicsApiFlags enabledApis = SL_GRAPHICS_API_ALL;

    bool handleEvents{false};
    slEventCallback eventCallback{nullptr};
    void* eventUserData{nullptr};

    std::vector<slWindowInstance> instances;
};

// Internal C++ definition backing public slWindowInstance handle
struct slWindowInstance_t {
    std::string appName;
    uint32_t appVersion{0};
    uint32_t width{800};
    uint32_t height{600};
    bool resizableWindow{true};
    
    slLogicalDevice activeLogicalDevice{nullptr};
    std::vector<slPhysicalDevice> physicalDevices;
    std::vector<std::unique_ptr<starlight::Window>> windows;

    struct vkWindowInstanceState* pVk; // DO NOT ACCESS PUBLICLY! MAY CAUSE CATASTROPHIC ERRORS.
};

// Internal C++ definition backing public slLogicalDevice handle (Deferred State)
struct slLogicalDevice_t {
    std::vector<slPhysicalDevice> physicalDevices;
    uint32_t selectedDeviceIndex{0};
    slSwapchainDesc swapchainConfig{};
};

// Internal C++ definition backing public slWindow handle
struct slWindow_t {
    starlight::Window* internalWindow{nullptr};
    slWindowInstance parentInstance{nullptr};
};

struct slNativeWindowHandles {
#if defined(_WIN32)
    void* hwnd{nullptr};
    void* hinstance{nullptr};
#elif defined(__APPLE__)
    void* nsWindow{nullptr}; // For macOS Cocoa
    void* nsView{nullptr};
#elif defined(__linux__)
    void* display{nullptr};  // For X11 / Wayland
    uint32_t window{0};
#endif
};

// Internal C++ definition backing public slWindowSurfaceBuffer handle
struct slWindowSurface_t {
    slWindow parentWindow{nullptr};
    slStructureType sType{SL_STRUCT_TYPE_NONE};
    const void* pNext{nullptr};
    slSurfaceFormat format{SL_SURFACE_FORMAT_BGRA8_UNORM};
    slColorSpace colorSpace{SL_COLOR_SPACE_SRGB_NONLINEAR};

    VkSurfaceKHR vkSurface{VK_NULL_HANDLE};
};