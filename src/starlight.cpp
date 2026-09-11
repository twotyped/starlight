#include <starlight/starlight.h>

#include <algorithm>

#include "core/sl_internal.hpp"
#include "platform/win32/sl_win32window.hpp"

// Global Starlight internal state
static slInstance_t g_starlightInstance{};

static bool DispatchNativeEvent(
    starlight::Window* window,
    uint32_t message,
    uintptr_t wParam,
    intptr_t lParam
) {
    slEvent event{};
    event.Window = window->GetPublicHandle();

    switch (message) {
        case WM_QUIT:
        case WM_CLOSE:
            event.Type = SL_EVENT_CLOSE;
            break;

        case WM_SIZE:
            event.Type = SL_EVENT_RESIZE;
            event.resize.Width = LOWORD(lParam);
            event.resize.Height = HIWORD(lParam);
            break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            event.Type = SL_EVENT_KEY;
            event.key.Key = static_cast<uint32_t>(wParam);
            event.key.Pressed = true;
            break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            event.Type = SL_EVENT_KEY;
            event.key.Key = static_cast<uint32_t>(wParam);
            event.key.Pressed = false;
            break;

        default:
            return false;
    }

    if (g_starlightInstance.handleEvents) {
        return true;
    }

    if (!g_starlightInstance.eventCallback) {
        return false;
    }

    return g_starlightInstance.eventCallback(
        &event,
        g_starlightInstance.eventUserData
    );
}

extern "C" {

SL_API slResult slInit(const slInitializationDesc* pDesc) {
    if (g_starlightInstance.initialized) {
        return SL_SUCCESS; // Already initialized
    }

    if (!pDesc) {
        g_starlightInstance.enabledApis = SL_GRAPHICS_API_ALL;
        g_starlightInstance.resizableWindow = true;
        g_starlightInstance.handleEvents = false;
        g_starlightInstance.eventCallback = nullptr;
        g_starlightInstance.eventUserData = nullptr;
    } else if (pDesc->sType == SL_STRUCT_TYPE_NONE) {
        g_starlightInstance.enabledApis = pDesc->GraphicsApi;
        g_starlightInstance.resizableWindow = pDesc->ResizableWindow;
        g_starlightInstance.handleEvents = pDesc->HandleEvents;
        g_starlightInstance.eventCallback = pDesc->EventCallback;
        g_starlightInstance.eventUserData = pDesc->EventUserData;
    } else {
        if (pDesc->sType != SL_STRUCT_TYPE_INIT_DESC) {
            return SL_ERROR_INVALID_PARAMETER;
        }

        g_starlightInstance.enabledApis = SL_GRAPHICS_API_ALL;
        g_starlightInstance.resizableWindow = true;
        g_starlightInstance.handleEvents = false;
        g_starlightInstance.eventCallback = nullptr;
        g_starlightInstance.eventUserData = nullptr;

#if defined(_WIN32) || defined(_WIN64)
        g_starlightInstance.enabledApis = SL_GRAPHICS_API_WINDOWS;
#endif

        if (pDesc->DefinedFields & SL_INIT_DESC_GRAPHICS_API_BIT) {
            g_starlightInstance.enabledApis = pDesc->GraphicsApi;
        }
        if (pDesc->DefinedFields & SL_INIT_DESC_RESIZABLE_WINDOW_BIT) {
            g_starlightInstance.resizableWindow = pDesc->ResizableWindow;
        }
        if (pDesc->DefinedFields & SL_INIT_DESC_HANDLE_EVENTS_BIT) {
            g_starlightInstance.handleEvents = pDesc->HandleEvents;
        }
        if (pDesc->DefinedFields & SL_INIT_DESC_EVENT_CALLBACK_BIT) {
            g_starlightInstance.eventCallback = pDesc->EventCallback;
            g_starlightInstance.eventUserData = pDesc->EventUserData;
        }
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

    g_starlightInstance.instances.push_back(instance);
    *pOutInstance = instance;
    return SL_SUCCESS;
}

SL_API void slDestroyWindowInstance(slWindowInstance instance) {
    if (instance) {
        auto it = std::find(g_starlightInstance.instances.begin(), g_starlightInstance.instances.end(), instance);
        if (it != g_starlightInstance.instances.end()) {
            g_starlightInstance.instances.erase(it);
        }
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
    nativeWindow->SetPublicHandle(windowHandle);

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
    if (!g_starlightInstance.initialized) {
        return;
    }

    for (const auto& instance : g_starlightInstance.instances) {
        for (const auto& window : instance->windows) {
            window->PollEvents(DispatchNativeEvent);
        }
    }
}

} // extern "C"