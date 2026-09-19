#include <starlight/starlight.h>
#include "core/sl_internal.hpp"
#include "platform/win32/sl_win32window.hpp"
#include "core/api/vulkan/vk_types.hpp"
#include <algorithm>
#include <set>

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

SL_API slResult slCreateLogicalDevice(slWindowInstance instance, const slLogicalDeviceDesc* pDeviceDesc, slLogicalDevice* pOutDevice) {
    if (!g_starlightInstance.initialized || !instance || !pDeviceDesc || !pDeviceDesc->physicalDevice) {
        return SL_ERROR_INVALID_PARAMETER;
    }

    auto* vkCtx = static_cast<vkWindowInstanceState*>(instance->pApiContext);
    if (!vkCtx || vkCtx->context.instance == VK_NULL_HANDLE) {
        return SL_ERROR_WINSTANCE_INVALID_CONTEXT;
    }

    auto* vkGpuData = static_cast<VulkanPhysicalDeviceData*>(pDeviceDesc->physicalDevice->pNativeDeviceHandle);
    VkPhysicalDevice physicalDevice = vkGpuData->handle;

    auto* outDevice = new slLogicalDevice_t();
    outDevice->physicalDevice = pDeviceDesc->physicalDevice;
    outDevice->dynamicRenderingEnabled = pDeviceDesc->EnableDynamicRendering;

    auto* vkDeviceData = new VulkanLogicalDeviceData();
    outDevice->pDeviceData = vkDeviceData;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            vkDeviceData->graphicsQueueFamilyIndex = i;
        }
        
        // Check if this queue family supports presentation to our window surface
        // (Note: This requires a valid surface initialized on the window instance!)
        VkBool32 presentSupport = VK_FALSE;
        if (vkCtx->surface.surface != VK_NULL_HANDLE) {
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, vkCtx->surface.surface, &presentSupport);
        }
        if (presentSupport) {
            vkDeviceData->presentQueueFamilyIndex = i;
        }

        if (vkDeviceData->graphicsQueueFamilyIndex != 0xFFFFFFFF && vkDeviceData->presentQueueFamilyIndex != 0xFFFFFFFF) {
            break; // Found matching queues
        }
    }

    // Setup queue creation structures
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { vkDeviceData->graphicsQueueFamilyIndex, vkDeviceData->presentQueueFamilyIndex };
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // 5. Build Logical Device Configuration
    VkDeviceCreateInfo createInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    // Standard device extensions required for swapchain presentation
    std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // Handle Modern Pathways: Vulkan 1.3 Dynamic Rendering Toggle
    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES};
    if (pDeviceDesc->EnableDynamicRendering) {
        dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
        createInfo.pNext = &dynamicRenderingFeatures; // Chain modern feature activation rules
    }

    // 6. Instantiate the Hardware Device
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &vkDeviceData->logicalDevice) != VK_SUCCESS) {
        delete vkDeviceData;
        delete outDevice;
        return SL_ERROR_LOGICAL_DEVICE_CREATION_FAILED;
    }

    // Cache the execution queues directly for your render cycles
    vkGetDeviceQueue(vkDeviceData->logicalDevice, vkDeviceData->graphicsQueueFamilyIndex, 0, &vkDeviceData->graphicsQueue);
    vkGetDeviceQueue(vkDeviceData->logicalDevice, vkDeviceData->presentQueueFamilyIndex, 0, &vkDeviceData->presentQueue);

    *pOutDevice = outDevice;

    return SL_SUCCESS;
}

SL_API void slDestroyLogicalDevice(slLogicalDevice device) {
    if (!device) return;

    if (device->pDeviceData) {
        auto* vkDeviceData = static_cast<VulkanLogicalDeviceData*>(device->pDeviceData);

        if (vkDeviceData->logicalDevice != VK_NULL_HANDLE) {
            // Force the CPU to block until the GPU finishes all active command queues
            vkDeviceWaitIdle(vkDeviceData->logicalDevice);

            // Destroy the logical hardware device connection
            vkDestroyDevice(vkDeviceData->logicalDevice, nullptr);
        }

        delete vkDeviceData;
    }

    delete device;
}

SL_API slResult slCreateSwapchain(slLogicalDevice device, const slSwapchainDesc* pDesc, slSwapchain* pOutSwapchain) {
    if (!g_starlightInstance.initialized || !device || !pDesc || !pDesc->targetWindow || !pOutSwapchain) {
        return SL_ERROR_INVALID_PARAMETER;
    }

    // 1. Unpack our core logical device handles
    auto* vkDeviceData = static_cast<VulkanLogicalDeviceData*>(device->pDeviceData);
    auto* vkGpuData = static_cast<VulkanPhysicalDeviceData*>(device->physicalDevice->pNativeDeviceHandle);
    
    // 2. Safely grab the surface associated with the target window instance
    // (Assuming the parent instance context tracked the live VkSurfaceKHR)
    auto* vkWindowInstance = static_cast<vkWindowInstanceState*>(pDesc->targetWindow->parentInstance->pApiContext);
    VkSurfaceKHR surface = vkWindowInstance->surface.surface;

    if (vkDeviceData->logicalDevice == VK_NULL_HANDLE || surface == VK_NULL_HANDLE) {
        return SL_ERROR_WINSTANCE_INVALID_CONTEXT;
    }

    // 3. Query Surface Capabilities to choose optimal sizing boundaries
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkGpuData->handle, surface, &capabilities);

    VkExtent2D swapchainExtent = capabilities.currentExtent;
    // If the system sets extent to 0xFFFFFFFF, it means match the target window coordinates exactly
    if (swapchainExtent.width == 0xFFFFFFFF) {
        swapchainExtent.width = std::clamp(pDesc->targetWindow->parentInstance->width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        swapchainExtent.height = std::clamp(pDesc->targetWindow->parentInstance->height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

    // Determine target buffering constraints
    uint32_t imageCount = pDesc->BufferCount;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }
    if (imageCount < capabilities.minImageCount) {
        imageCount = capabilities.minImageCount;
    }

    // 4. Map Abstract Formats to Vulkan Layout Configurations
    VkSurfaceFormatKHR surfaceFormat{};
    surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM; // Match SL_SURFACE_FORMAT_BGRA8_UNORM
    surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    // Map Present Mode
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // Standard default VSync
    if (pDesc->PresentMode == SL_PRESENT_MODE_IMMEDIATE) presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    else if (pDesc->PresentMode == SL_PRESENT_MODE_MAILBOX) presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

    // 5. Populate standard Vulkan Swapchain Creation Structures
    VkSwapchainCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = swapchainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Handle concurrent execution if presentation and graphics queues differ
    uint32_t queueFamilyIndices[] = { vkDeviceData->graphicsQueueFamilyIndex, vkDeviceData->presentQueueFamilyIndex };
    if (vkDeviceData->graphicsQueueFamilyIndex != vkDeviceData->presentQueueFamilyIndex) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    // 6. Allocate our wrapper node
    auto* outSwapchain = new slSwapchain_t();
    outSwapchain->targetWindow = pDesc->targetWindow;
    outSwapchain->width = swapchainExtent.width;
    outSwapchain->height = swapchainExtent.height;
    
    auto* vkSwapchainData = new VulkanSwapchainData();
    outSwapchain->pSwapchainData = vkSwapchainData;

    if (vkCreateSwapchainKHR(vkDeviceData->logicalDevice, &createInfo, nullptr, &vkSwapchainData->handle) != VK_SUCCESS) {
        delete vkSwapchainData;
        delete outSwapchain;
        return SL_ERROR_SWAPCHAIN_CREATION_FAILED;
    }

    // 7. Extract the Swapchain Image Views
    uint32_t actualImageCount = 0;
    vkGetSwapchainImagesKHR(vkDeviceData->logicalDevice, vkSwapchainData->handle, &actualImageCount, nullptr);
    vkSwapchainData->images.resize(actualImageCount);
    vkGetSwapchainImagesKHR(vkDeviceData->logicalDevice, vkSwapchainData->handle, &actualImageCount, vkSwapchainData->images.data());

    vkSwapchainData->imageViews.resize(actualImageCount);
    for (uint32_t i = 0; i < actualImageCount; ++i) {
        VkImageViewCreateInfo viewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        viewInfo.image = vkSwapchainData->images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = surfaceFormat.format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vkDeviceData->logicalDevice, &viewInfo, nullptr, &vkSwapchainData->imageViews[i]) != VK_SUCCESS) {
            return SL_ERROR_IMAGE_VIEW_CREATION_FAILED;
        }
    }

    // TODO: Append legacy VkRenderPass generation here if dynamicRenderingEnabled is false.

    *pOutSwapchain = outSwapchain;
    return SL_SUCCESS;
}

SL_API void slDestroySwapchain(slSwapchain swapchain) {
    if (!swapchain) return;

    if (swapchain->pSwapchainData) {
        auto* vkSwapData = static_cast<VulkanSwapchainData*>(swapchain->pSwapchainData);
        
        if (swapchain->targetWindow && swapchain->targetWindow->parentInstance) {
            auto* vkCtx = static_cast<vkWindowInstanceState*>(swapchain->targetWindow->parentInstance->pApiContext);
            VkDevice logicalDevice = vkCtx->context.logicalDevice; 

            if (logicalDevice != VK_NULL_HANDLE) {
                // Destroy the created framebuffers (if legacy route compiled them)
                for (VkFramebuffer framebuffer : vkSwapData->framebuffers) {
                    if (framebuffer != VK_NULL_HANDLE) {
                        vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
                    }
                }
                
                // Destroy the concrete Image Views
                for (VkImageView view : vkSwapData->imageViews) {
                    if (view != VK_NULL_HANDLE) {
                        vkDestroyImageView(logicalDevice, view, nullptr);
                    }
                }

                // Destroy the primary Swapchain handle itself
                if (vkSwapData->handle != VK_NULL_HANDLE) {
                    vkDestroySwapchainKHR(logicalDevice, vkSwapData->handle, nullptr);
                }
            }
        }

        delete vkSwapData;
    }

    delete swapchain;
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