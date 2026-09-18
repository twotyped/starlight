#include <starlight/starlight.h>
#include <iostream>
#include <vector>

bool HandleEvent(const slEvent* event, void*) {
    if (event && event->Type == SL_EVENT_CLOSE) {
        std::cout << "Closing window..." << std::endl;
        return false; // Here we must (or, technically, should) set this to false to allow Starlight to handle the window closing.
    }

    return true;
}

int main() {
    std::cout << "Hello, Starlight!" << std::endl;

    slInitializationDesc initDesc{SL_STRUCT_TYPE_INIT_DESC};
    initDesc.GraphicsApi = SL_GRAPHICS_API_ALL;
    initDesc.DefinedFields = SL_INIT_DESC_GRAPHICS_API_BIT | SL_INIT_DESC_EVENT_CALLBACK_BIT;
    initDesc.HandleEvents = false;
    initDesc.EventCallback = HandleEvent;

    if (slInit(&initDesc) != SL_SUCCESS) {
        std::cerr << "Failed to initialize Starlight!\n";
        return -1;
    }

    slWindowInstanceDesc instanceDesc{SL_STRUCT_TYPE_WINDOW_INSTANCE_DESC};
    instanceDesc.ApplicationName = "02: Green Window";
    instanceDesc.ApplicationVersion = SL_MAKE_VERSION(1,0,0);

    instanceDesc.Width = 800;
    instanceDesc.Height = 600;
    instanceDesc.ResizableWindow = false;

    instanceDesc.graphicsApi = SL_GRAPHICS_API_VULKAN;

    slWindowInstance instance;
    slCreateWindowInstance(&instanceDesc, &instance);

    // Get the number of physical devices
    uint32_t deviceCount = 0;
    slEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    std::vector<slPhysicalDevice> devices(deviceCount);
    if (deviceCount > 0) {
        slEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    }

    std::cout << "Found " << deviceCount << " physical device(s):\n";

    if (deviceCount < 0) {
        for (slPhysicalDevice device : devices) {
            slPhysicalDeviceProperties props;
            if (slGetPhysicalDeviceProperties(device, &props) == SL_SUCCESS) {
                std::cout << "- " << props.DeviceName << "\n";
            }
        }

        std::cout << "Selecting 0 (default)" << std::endl;
    } else {
        std::cerr << "We need a physical device to select.\n";
        return -1;
    }

    slWindowSurfaceDesc surfaceDesc{SL_STRUCT_TYPE_WINDOW_SURFACE_DESC};
    surfaceDesc.RequestedColorSpace = SL_COLOR_SPACE_SRGB_LINEAR;
    surfaceDesc.RequestedFormat = SL_SURFACE_FORMAT_BGRA8_UNORM;

    slWindow window{nullptr};
    if (slCreateWindow(instance, nullptr, &window, &surfaceDesc) != SL_SUCCESS) {
        std::cerr << "Failed to create window!\n";
        slShutdown();
        return -1;
    }

    while (!slWindowShouldClose(window)) {
        slPollEvents();
    }

    slDestroyWindow(window);
    slShutdown();

    return 0;
}