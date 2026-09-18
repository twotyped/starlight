#include <starlight/starlight.h>
#include <algorithm>
#include "core/sl_internal.hpp"
#include "platform/win32/sl_win32window.hpp"
#include "core/api/vulkan/vk_types.hpp"


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
        g_starlightInstance.handleEvents = false;
        g_starlightInstance.eventCallback = nullptr;
        g_starlightInstance.eventUserData = nullptr;
    } else if (pDesc->sType == SL_STRUCT_TYPE_NONE) {
        g_starlightInstance.enabledApis = pDesc->GraphicsApi;
        g_starlightInstance.handleEvents = pDesc->HandleEvents;
        g_starlightInstance.eventCallback = pDesc->EventCallback;
        g_starlightInstance.eventUserData = pDesc->EventUserData;
    } else {
        if (pDesc->sType != SL_STRUCT_TYPE_INIT_DESC) {
            return SL_ERROR_INVALID_PARAMETER;
        }

        g_starlightInstance.enabledApis = SL_GRAPHICS_API_ALL;
        g_starlightInstance.handleEvents = false;
        g_starlightInstance.eventCallback = nullptr;
        g_starlightInstance.eventUserData = nullptr;

#if defined(_WIN32) || defined(_WIN64)
        g_starlightInstance.enabledApis = SL_GRAPHICS_API_WINDOWS;
#endif

        if (pDesc->DefinedFields & SL_INIT_DESC_GRAPHICS_API_BIT) {
            g_starlightInstance.enabledApis = pDesc->GraphicsApi;
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
    instance->width = pDesc->Width;
    instance->height = pDesc->Height;
    instance->resizableWindow = pDesc->ResizableWindow;
    instance->pApiContext = nullptr;

    if (pDesc->graphicsApi == SL_GRAPHICS_API_VULKAN) { // Vulkan enabled, initialize it.
        auto* vkCtx = new vkWindowInstanceState();
        instance->pApiContext = vkCtx;

        VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        appInfo.pApplicationName = instance->appName.c_str();
        appInfo.applicationVersion = instance->appVersion; // same versioning structure
        appInfo.pEngineName = pDesc->vk.EngineName ? pDesc->vk.EngineName : "No Engine";
        appInfo.engineVersion = pDesc->vk.EngineVersion ? pDesc->vk.EngineVersion : VK_MAKE_VERSION(0, 0, 0);
        appInfo.apiVersion = pDesc->vk.ApiVersion ? pDesc->vk.ApiVersion : VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        createInfo.pApplicationInfo = &appInfo;
        
        if (pDesc->vk.IncludeStarlightWindowExtensions) {
            // Do starlight default extensions here.
        }

        createInfo.enabledExtensionCount = pDesc->vk.EnabledExtensionCount;
        createInfo.enabledLayerCount = pDesc->vk.EnabledLayerCount;
        createInfo.ppEnabledExtensionNames = pDesc->vk.ppEnabledExtensionNames;
        createInfo.ppEnabledLayerNames = pDesc->vk.ppEnabledLayerNames;

        if (vkCreateInstance(&createInfo, nullptr, &vkCtx->context.instance) != VK_SUCCESS) {
            delete vkCtx;
            delete instance;
            return SL_ERROR_WINSTANCE_INITIALIZATION_FAILED;
        }

    }

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

SL_API slResult slCreateWindow(slWindowInstance instance, slLogicalDevice device, slWindow* pOutWindow, const slWindowSurfaceDesc* pSurfaceDesc) {
    if (!instance || !pOutWindow) {
        return SL_ERROR_INVALID_PARAMETER;
    }

    if (pSurfaceDesc && pSurfaceDesc->sType != SL_STRUCT_TYPE_WINDOW_SURFACE_DESC) {
        return SL_ERROR_INVALID_PARAMETER;
    }

    auto nativeWindow = std::make_unique<starlight::Win32Window>();

    slWindowInstanceDesc desc{};
    desc.sType = SL_STRUCT_TYPE_WINDOW_INSTANCE_DESC;
    desc.ApplicationName = instance->appName.c_str();
    desc.Width = instance->width;
    desc.Height = instance->height;
    desc.ResizableWindow = instance->resizableWindow;

    slWindowSurface surface = nullptr;
    if (pSurfaceDesc) {
        surface = new slWindowSurface_t();
        surface->sType = pSurfaceDesc->sType;
        surface->pNext = pSurfaceDesc->pNext;
        surface->format = pSurfaceDesc->RequestedFormat;
        surface->colorSpace = pSurfaceDesc->RequestedColorSpace;
    }

    if (!nativeWindow->Initialize(desc, surface)) {
        delete surface;
        return SL_ERROR_INITIALIZATION_FAILED;
    }

    auto windowHandle = new slWindow_t();
    windowHandle->internalWindow = nativeWindow.get();
    windowHandle->parentInstance = instance;
    nativeWindow->SetPublicHandle(windowHandle);

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


SL_API slResult slEnumeratePhysicalDevices(slWindowInstance instance, uint32_t* pCount, slPhysicalDevice* pOutDevices) {
    if (!g_starlightInstance.initialized || !instance || !pCount) {
        return SL_ERROR_INVALID_PARAMETER;
    }

    auto* vkCtx = static_cast<vkWindowInstanceState*>(instance->pApiContext);
    if (!vkCtx || vkCtx->context.instance == VK_NULL_HANDLE) {
        return SL_ERROR_WINSTANCE_INVALID_CONTEXT;
    }

    uint32_t vkDeviceCount = 0;
    if (vkEnumeratePhysicalDevices(vkCtx->context.instance, &vkDeviceCount, nullptr) != VK_SUCCESS) {
        return SL_ERROR_PHYSICAL_DEVICE_ENUMERATION_FAILED;
    }

    if (pOutDevices == nullptr) {
        *pCount = vkDeviceCount;
        return SL_SUCCESS;
    }

    std::vector<VkPhysicalDevice> vkDevices(vkDeviceCount);
    if (vkEnumeratePhysicalDevices(vkCtx->context.instance, &vkDeviceCount, vkDevices.data()) != VK_SUCCESS) {
        return SL_ERROR_PHYSICAL_DEVICE_ENUMERATION_FAILED;
    }

    uint32_t devicesToCopy = (*pCount < vkDeviceCount) ? *pCount : vkDeviceCount;
    for (uint32_t i = 0; i < devicesToCopy; ++i) {
        // Query hardware metrics natively
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(vkDevices[i], &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(vkDevices[i], &features);

        // 2. Allocate the public-facing C wrapper node
        auto* outGpu = new slPhysicalDevice_t();
        outGpu->deviceName = properties.deviceName;
        outGpu->vendorID = properties.vendorID;
        outGpu->deviceID = properties.deviceID;
        outGpu->isDiscreteGPU = (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);

        // 3. Allocate our private backend data struct and map it to the void* slot!
        auto* vkGpuData = new VulkanPhysicalDeviceData();
        vkGpuData->handle = vkDevices[i];
        vkGpuData->properties = properties;
        vkGpuData->features = features;

        outGpu->pNativeDeviceHandle = vkGpuData; 

        pOutDevices[i] = outGpu;
    }

    *pCount = devicesToCopy;
    return SL_SUCCESS;
}

SL_API slResult slGetPhysicalDeviceProperties(slPhysicalDevice device, slPhysicalDeviceProperties* pProperties) {
    if (!device || !pProperties) {
        return SL_ERROR_INVALID_PARAMETER;
    }

    memset(pProperties, 0, sizeof(slPhysicalDeviceProperties));

    strncpy(pProperties->DeviceName, device->deviceName.c_str(), sizeof(pProperties->DeviceName) - 1);
    
    pProperties->VendorID = device->vendorID;
    pProperties->DeviceID = device->deviceID;
    
    if (device->isDiscreteGPU) {
        pProperties->DeviceType = SL_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    } else {
        // TODO: Expand internal slPhysicalDevice_t to track details precisely
        pProperties->DeviceType = SL_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
    }

    // TODO: Map over memory tracking metrics
    pProperties->DedicatedVideoMemory = 0; 

    return SL_SUCCESS;
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