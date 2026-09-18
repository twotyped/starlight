#pragma once

#include <starlight/starlight.h>
#include <string>
#include <vector>
#include <memory>
#include "sl_window.hpp"
#include "api/vulkan/vk_internal.hpp"

struct slInstance_t {
    bool initialized{false};
    slGraphicsApiFlags enabledApis = static_cast<slGraphicsApiFlags>(SL_GRAPHICS_API_ALL);

    bool handleEvents{false};
    slEventCallback eventCallback{nullptr};
    void* eventUserData{nullptr};

    std::vector<slWindowInstance> instances;
};

struct slWindowInstance_t {
    std::string appName;
    uint32_t appVersion{0};
    uint32_t width{800};
    uint32_t height{600};
    bool resizableWindow{true};

    slGraphicsApi selectedApi;

    void* pApiContext{nullptr};
    
    slLogicalDevice activeLogicalDevice{nullptr};
    std::vector<std::unique_ptr<starlight::Window>> windows;
};

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

struct slPhysicalDevice_t {
    std::string deviceName;
    uint32_t vendorID{0};
    uint32_t deviceID{0};
    bool isDiscreteGPU{false};

    void* pNativeDeviceHandle{nullptr}; 
};

struct slLogicalDevice_t {
    slPhysicalDevice physicalDevice{nullptr};
    bool dynamicRenderingEnabled{false};

    void* pDeviceData{nullptr}; 
};

struct slSwapchain_t {
    slWindow targetWindow{nullptr};
    uint32_t width{0};
    uint32_t height{0};
    slSurfaceFormat format{SL_SURFACE_FORMAT_BGRA8_UNORM};
    slColorSpace colorSpace{SL_COLOR_SPACE_SRGB_NONLINEAR};
    uint32_t currentImageIndex{0};

    void* pSwapchainData{nullptr}; 
};

struct slWindowSurface_t {
    slWindow parentWindow{nullptr};
    slStructureType sType{SL_STRUCT_TYPE_NONE};
    const void* pNext{nullptr};
    slSurfaceFormat format{SL_SURFACE_FORMAT_BGRA8_UNORM};
    slColorSpace colorSpace{SL_COLOR_SPACE_SRGB_NONLINEAR};

    void* pSurfaceData{nullptr}; 
};