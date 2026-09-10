#include <starlight/starlight.h>

#include "core/sl_internal.hpp"
#include "platform/win32/sl_win32window.hpp"

// Global Starlight internal state
static slInstance_t g_starlightInstance{};

extern "C" {

SL_API slResult slInit(const slInitializationDesc* pDesc) {
    if (g_starlightInstance.initialized) {
        return SL_SUCCESS; // Already initialized
    }

    if (pDesc) {
        if (pDesc->sType != SL_STRUCT_TYPE_INIT_DESC) {
            return SL_ERROR_INVALID_PARAMETER;
        }
        g_starlightInstance.enabledApis = pDesc->GraphicsApi;
    } else {
        g_starlightInstance.enabledApis = SL_GRAPHICS_API_ALL;
    }

    g_starlightInstance.initialized = true;
    return SL_SUCCESS;
}

SL_API void slShutdown(void) {
    g_starlightInstance.initialized = false;
}

SL_API slResult slCreateWindowInstance(const slWindowInstanceDesc* pDesc, slWindowInstance* pOutInstance) {
    if (!g_starlightInstance.initialized || !pDesc || !pOutInstance) {
        return SL_ERROR_INVALID_PARAMETER;
    }

    if (pDesc->sType != SL_STRUCT_TYPE_WINDOW_INSTANCE_DESC) {
        return SL_ERROR_INVALID_PARAMETER;
    }

    auto instance = new slWindowInstance_t();
    instance->appName = pDesc->ApplicationName ? pDesc->ApplicationName : "Starlight Application";
    instance->appVersion = pDesc->ApplicationVersion;

    *pOutInstance = instance;
    return SL_SUCCESS;
}

SL_API void slDestroyWindowInstance(slWindowInstance instance) {
    if (instance) {
        delete instance;
    }
}

SL_API slResult slCreateWindow(slWindowInstance instance, slLogicalDevice device, slWindow* pOutWindow) {
    if (!instance || !pOutWindow) {
        return SL_ERROR_INVALID_PARAMETER;
    }

    // 1. Instantiate concrete platform window (Win32 for now)
    auto nativeWindow = std::make_unique<starlight::Win32Window>();

    // Build temporary desc for window creation from stored instance specs
    slWindowInstanceDesc desc{};
    desc.sType = SL_STRUCT_TYPE_WINDOW_INSTANCE_DESC;
    desc.ApplicationName = instance->appName.c_str();
    desc.Width = 800;  // Default fallback if not defined in device/instance
    desc.Height = 600;

    if (!nativeWindow->Initialize(desc)) {
        return SL_ERROR_INITIALIZATION_FAILED;
    }

    // 2. Allocate public slWindow handle
    auto windowHandle = new slWindow_t();
    windowHandle->internalWindow = nativeWindow.get();
    windowHandle->parentInstance = instance;

    // 3. Store ownership in WindowInstance
    instance->windows.push_back(std::move(nativeWindow));

    *pOutWindow = windowHandle;
    return SL_SUCCESS;
}

SL_API void slDestroyWindow(slWindow window) {
    if (window) {
        delete window;
    }
}

SL_API bool slWindowShouldClose(slWindow window) {
    if (!window || !window->internalWindow) {
        return true;
    }
    return window->internalWindow->ShouldClose();
}

SL_API void slPollEvents(void) {
    // For now, poll directly across active instances/windows
    // In full implementation, iterates through active g_starlightInstance windows
}

} // extern "C"