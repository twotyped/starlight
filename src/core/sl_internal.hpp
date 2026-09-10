#pragma once

#include <starlight/starlight.h>
#include <string>
#include <vector>
#include <memory>
#include "sl_window.hpp"

// Internal C++ definition backing public slInstance handle
struct slInstance_t {
    bool initialized{false};
    slGraphicsApiFlags enabledApis = SL_GRAPHICS_API_ALL;
};

// Internal C++ definition backing public slWindowInstance handle
struct slWindowInstance_t {
    std::string appName;
    uint32_t appVersion{0};
    
    slLogicalDevice activeLogicalDevice{nullptr};
    std::vector<slPhysicalDevice> physicalDevices;
    std::vector<std::unique_ptr<starlight::Window>> windows;
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