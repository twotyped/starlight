#include <starlight/starlight.h>
#include <iostream>

int main() {
    std::cout << "Hello, Starlight!" << std::endl;

    slInitializationDesc initDesc{SL_STRUCT_TYPE_INIT_DESC};
    initDesc.GraphicsApi = SL_GRAPHICS_API_ALL;

    if (slInit(&initDesc) != SL_SUCCESS) {
        std::cerr << "Failed to initialize Starlight!\n";
        return -1;
    }

    slWindowInstanceDesc instanceDesc{SL_STRUCT_TYPE_WINDOW_INSTANCE_DESC};
    instanceDesc.ApplicationName = "01: Clear Window";
    instanceDesc.ApplicationVersion = SL_MAKE_VERSION(1,0,0);
    instanceDesc.Height = 1280;
    instanceDesc.Width = 720;

    slWindowInstance instance;
    slCreateWindowInstance(&instanceDesc, &instance);

    slWindow window = nullptr;
    if (slCreateWindow(instance, nullptr, &window) != SL_SUCCESS) {
        std::cerr << "Failed to create window!\n";
        slShutdown();
        return -1;
    }

    while (!slWindowShouldClose(window)) {
        slPollEvents();
        // Clear screen / Rendering logic goes here
    }

    slDestroyWindow(window);
    slShutdown();

    return 0;
}