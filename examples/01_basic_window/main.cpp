#include <starlight/starlight.h>
#include <iostream>

bool HandleEvent(const slEvent* event, void*) {
    if (event && event->Type == SL_EVENT_CLOSE) {
        std::cout << "Closing window..." << std::endl;
        return false; // Here we must (or technically should) set this to false to allow Starlight to handle the window closing.
    }

    return true;
}

int main() {
    std::cout << "Hello, Starlight!" << std::endl;

    slInitializationDesc initDesc{SL_STRUCT_TYPE_INIT_DESC};
    initDesc.GraphicsApi = SL_GRAPHICS_API_ALL;
    initDesc.DefinedFields = SL_INIT_DESC_GRAPHICS_API_BIT | SL_INIT_DESC_EVENT_CALLBACK_BIT;
    initDesc.HandleEvents = false; // Setting this to false allows us to handle events through an event callback.
    initDesc.EventCallback = HandleEvent;

    if (slInit(&initDesc) != SL_SUCCESS) {
        std::cerr << "Failed to initialize Starlight!\n";
        return -1;
    }

    slWindowInstanceDesc instanceDesc{SL_STRUCT_TYPE_WINDOW_INSTANCE_DESC};
    instanceDesc.ApplicationName = "01: Basic Window";
    instanceDesc.ApplicationVersion = SL_MAKE_VERSION(1,0,0);
    instanceDesc.Width = 800;
    instanceDesc.Height = 600;
    instanceDesc.ResizableWindow = false;

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
    }

    slDestroyWindow(window);
    slShutdown();

    return 0;
}