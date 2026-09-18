#ifndef STARLIGHT_TYPES_H
#define STARLIGHT_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#if defined(_WIN32) || defined(_WIN64)
    #ifdef STARLIGHT_BUILD_DLL
        #define SL_API __declspec(dllexport)
    #elif defined(STARLIGHT_USE_DLL)
        #define SL_API __declspec(dllimport)
    #else
        #define SL_API
    #endif
#elif defined(__GNUG__) || defined(__clang__)
    #define SL_API __attribute__((visibility("default")))
#else
    #define SL_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// VERSIONING MACRO
#define SL_MAKE_VERSION(major, minor, patch) \
    ((((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))


// OPAQUE HANDLES

typedef struct slInstance_t*            slInstance;         // An internal instance. This is not created publicly.
typedef struct slWindowInstance_t*      slWindowInstance;   // An instance for the window. This *is* created publicly and is effectively the public equivalent of the Starlight instance.
typedef struct slPhysicalDevice_t*      slPhysicalDevice;   // Acts as an abstracted gateway to the physical device (the GPU).
typedef struct slLogicalDevice_t*       slLogicalDevice;    // Handles physical devices, swapchains, and more.
typedef struct slSwapchain_t*           slSwapchain;        // Swap chain.
typedef struct slWindow_t*              slWindow;           // The actual abstracted window class, managed by the window instance.
typedef struct slWindowSurface_t* slWindowSurface;

// CORE ENUMS

typedef enum slResult {
    SL_SUCCESS = 0,
    SL_ERROR_INITIALIZATION_FAILED = -1,
    SL_ERROR_OUT_OF_MEMORY = -2,
    SL_ERROR_DEVICE_LOST = -3,
    SL_ERROR_INVALID_PARAMETER = -4,
    SL_ERROR_WINSTANCE_INITIALIZATION_FAILED = -5,
    SL_ERROR_WINSTANCE_INVALID_CONTEXT = -6,
    SL_ERROR_PHYSICAL_DEVICE_ENUMERATION_FAILED = -7,
} slResult;

typedef enum slStructureType {
    SL_STRUCT_TYPE_NONE                     = 0,        // Essentially nothing as a default—helps if you want to be aware.
    SL_STRUCT_TYPE_INIT_DESC                = 1,        // Default Descriptor for the initialization.
    SL_STRUCT_TYPE_WINDOW_INSTANCE_DESC     = 2,        // Default Descriptor for the creation of a window instance.
    SL_STRUCT_TYPE_LOGICAL_DEVICE_DESC      = 3,        // Default Descriptor for the creation of a logical device.
    SL_STRUCT_TYPE_SWAPCHAIN_DESC           = 4,        // Default Descriptor for the creation of a swapchain.
    SL_STRUCT_TYPE_WINDOW_SURFACE_DESC      = 5,
} slStructureType;

// Bitflags for graphics APIs to allow combining backends (e.g., Vulkan + DX12)
typedef uint32_t slGraphicsApiFlags;

typedef enum slGraphicsApiFlagBits {
    SL_GRAPHICS_API_NONE_BIT       = 0,
    SL_GRAPHICS_API_VULKAN_BIT     = (1 << 0),          // Exclusively Vulkan
    SL_GRAPHICS_API_DIRECTX11_BIT  = (1 << 1),          // Exclusively DirectX 11
    SL_GRAPHICS_API_DIRECTX12_BIT  = (1 << 2),          // Exclusively DirectX 12
    SL_GRAPHICS_API_METAL_BIT      = (1 << 3),          // Exclusively Metal
    SL_GRAPHICS_API_OPENGL_BIT     = (1 << 4),          // Exclusively OpenGL (not recommended, disables many features of Starlight)

    // Convenient combination masks
    SL_GRAPHICS_API_ALL            = 0xFFFFFFFF,        // Ensures all APIs are available to use (recommended).
    SL_GRAPHICS_API_NO_OPENGL      = (0xFFFFFFFF & ~SL_GRAPHICS_API_OPENGL_BIT), // Disables OpenGL (which can be too implicit for a lot of programs—this allows the developer to disable using OpenGL)
    SL_GRAPHICS_API_MACOS          = (SL_GRAPHICS_API_METAL_BIT | SL_GRAPHICS_API_VULKAN_BIT), // Default on macOS.
    SL_GRAPHICS_API_LINUX          = (SL_GRAPHICS_API_VULKAN_BIT | SL_GRAPHICS_API_OPENGL_BIT), // Default on Linux.
    SL_GRAPHICS_API_WINDOWS        = (SL_GRAPHICS_API_VULKAN_BIT | SL_GRAPHICS_API_DIRECTX12_BIT | SL_GRAPHICS_API_DIRECTX11_BIT) // Default on Windows.
} slGraphicsApiFlagBits;

typedef enum slGraphicsApi {
    SL_GRAPHICS_API_NONE        = 0,
    SL_GRAPHICS_API_VULKAN      = 1,
    SL_GRAPHICS_API_DIRECTX11   = 2,
    SL_GRAPHICS_API_DIRECTX12   = 3,
    SL_GRAPHICS_API_METAL       = 4,
    SL_GRAPHICS_API_OPENGL      = 5
} slGraphicsApi;

typedef enum slPresentMode {
    SL_PRESENT_MODE_IMMEDIATE = 0,          // Not recommended! The GPU sends frames to the monitor immediately—can lead to intense screen tearing.
    SL_PRESENT_MODE_VSYNC = 1,              // Enables V-Sync (FIFO). The GPU waits for the monitor to finish its refresh cycle before sending a new frame.
    SL_PRESENT_MODE_TRIPLE_BUFFER = 2,      // The default (Mailbox). The GPU uses three frame buffers to prep frames ahead of time, eliminating V-Sync wait times.
    SL_PRESENT_MODE_MAILBOX = 2             // Alias for triple buffering.
} slPresentMode;

typedef enum slPhysicalDeviceType {
    SL_PHYSICAL_DEVICE_TYPE_OTHER = 0,
    SL_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
    SL_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
    SL_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
    SL_PHYSICAL_DEVICE_TYPE_CPU,
} slPhysicalDeviceType;

// EXTENSION SUB-STRUCTS

/**
 * @brief Vulkan-specific initialization and extension configuration.
 *
 * Supply this structure through the @c pNext chain of a window-instance
 * descriptor when the Vulkan backend requires application-provided names or
 * version information. Counts must match the corresponding name arrays; the
 * arrays and their strings are borrowed and must remain valid as required by
 * the initialization call.
 */
typedef struct slVulkanInfo {
    /** @brief Application or engine name reported to Vulkan. */
    const char* EngineName;

    /** @brief Application or engine version, conventionally made with SL_MAKE_VERSION. */
    uint32_t EngineVersion;

    /** @brief Vulkan API version requested by the application. */
    uint32_t ApiVersion;

    /** @brief Number of validation or other instance layers to enable. */
    uint32_t EnabledLayerCount;

    /** @brief Names of instance layers to enable; may be NULL when the count is zero. */
    const char* const* ppEnabledLayerNames;

    /** @brief Number of Vulkan instance extensions to enable. */
    uint32_t EnabledExtensionCount;

    /** @brief Names of instance extensions to enable; may be NULL when the count is zero. */
    const char* const* ppEnabledExtensionNames;

    /** @brief Number of Vulkan device extensions to enable. */
    uint32_t EnabledDeviceExtensionCount;

    /** @brief Names of device extensions to enable; may be NULL when the count is zero. */
    const char* const* ppEnabledDeviceExtensionNames;

    /**
     * @brief Whether Starlight should add the extensions required to create
     *        a surface for its window system.
     *
     * Set this to @c true when Starlight owns surface setup. Set it to @c false
     * when the application manages the required window extensions itself.
     */
    bool IncludeStarlightWindowExtensions;
} slVulkanInfo;

typedef struct slSwapchainExtensions {
    bool HdrEnabled;
} slSwapchainExtensions;

// STUFF I GUESS

typedef enum slEventType {
    SL_EVENT_NONE = 0,
    SL_EVENT_CLOSE,
    SL_EVENT_RESIZE,
    SL_EVENT_KEY,
    SL_EVENT_MOUSE
} slEventType;

typedef struct slEvent {
    slEventType Type;
    slWindow Window;

    union {
        struct {
            uint32_t Width;
            uint32_t Height;
        } resize;

        struct {
            uint32_t Key;
            bool Pressed;
        } key;
    };
} slEvent;

typedef bool (*slEventCallback)(
    const slEvent* event,
    void* userData
);

typedef enum slInitializationDescFieldBits {
    SL_INIT_DESC_GRAPHICS_API_BIT    = (1 << 0),
    SL_INIT_DESC_RESIZABLE_WINDOW_BIT = (1 << 1),
    SL_INIT_DESC_HANDLE_EVENTS_BIT    = (1 << 2),
    SL_INIT_DESC_EVENT_CALLBACK_BIT   = (1 << 3)
} slInitializationDescFieldBits;


typedef enum slSurfaceFormat {
    SL_SURFACE_FORMAT_BGRA8_UNORM,
    SL_SURFACE_FORMAT_RGBA8_UNORM
} slSurfaceFormat;

typedef enum slColorSpace {
    // Standard Dynamic Range (SDR)
    SL_COLOR_SPACE_SRGB_NONLINEAR = 0, // Most common. standard sRGB gamma 2.2 curve (Rec. 709)
    SL_COLOR_SPACE_SRGB_LINEAR,        // Linearized sRGB primaries (excellent for high-end rendering pipelines)

    // Wide Color Gamut (WCG)
    SL_COLOR_SPACE_DISPLAY_P3,         // Common on modern Apple displays, newer laptops, and mobile devices

    // High Dynamic Range (HDR)
    SL_COLOR_SPACE_HDR10_ST2084,       // Standard Ultra-HD HDR (BT.2020 gamut with SMPTE ST 2084 PQ curve)
    SL_COLOR_SPACE_SCRGB_LINEAR,       // Windows HDR standard (Extended linear sRGB/Rec.709 that allows values > 1.0)
    
    SL_COLOR_SPACE_MAX_ENUM = 0x7FFFFFFF
} slColorSpace;

typedef struct slPhysicalDeviceProperties {
    char DeviceName[256];
    uint32_t VendorID;
    uint32_t DeviceID;
    slPhysicalDeviceType DeviceType;
    uint64_t DedicatedVideoMemory;
} slPhysicalDeviceProperties;

// DESCRIPTOR STRUCTS

typedef struct slInitializationDesc {
    slStructureType sType;      // Should be set to SL_STRUCT_TYPE_INIT_DESC
    const void* pNext;          // Pointer to extension-specific structures, or NULL
    uint32_t DefinedFields;     // Bitmask of fields explicitly supplied by the application.
    slGraphicsApiFlags GraphicsApi; // Defaults depending on platform—refer to slGraphicsApi.

    bool HandleEvents;          // Defaults to false (manual mode, you must handle events manually)
    slEventCallback EventCallback; // The function which handles the events.
    void* EventUserData;
} slInitializationDesc;

typedef struct slWindowSurfaceDesc {
    slStructureType sType;
    const void* pNext;
    
    slSurfaceFormat RequestedFormat;
    slColorSpace RequestedColorSpace;
} slWindowSurfaceDesc;

typedef struct slWindowInstanceDesc {
    slStructureType sType;          // Should be set to SL_STRUCT_TYPE_WINDOW_INSTANCE_DESC
    const void* pNext;              // Pointer to extension-specific structures, or NULL:w

    const char* ApplicationName;    // The application name, equivalent to the window name.
    uint32_t ApplicationVersion;    // The version of the application. Currently useless. Should use SL_MAKE_VERSION(major, minor, patch).
    uint32_t Width;                 // The window's width (note that this is explicitly for the WINDOW surface, not the swapchain—allowing you to stretch, or upscale/downscale).
    uint32_t Height;                // The window's height (note that this is explicitly for the WINDOW surface, not the swapchain—allowing you to stretch, or upscale/downscale).
    bool ResizableWindow;           // Defaults to true
    slGraphicsApi graphicsApi;      // The active graphics API of the window instance.

    slWindowSurfaceDesc* pWindowSurface;

    slVulkanInfo vk{nullptr};       // Vulkan-specific sub-struct
} slWindowInstanceDesc;

// typedef struct slLogicalDeviceDesc {
//     slStructureType sType;              // Should be set to SL_STRUCT_TYPE_LOGICAL_DEVICE_DESC
//     const void* pNext;                  // Pointer to extension-specific structures, or NULL
//     slPhysicalDevice* pPhysicalDevices; // Pointer to array of handles.
//     uint32_t PhysicalDeviceCount;       // The number of physical device enumerated over.
//     uint32_t SelectedDeviceIndex;       // Default is 0 (i.e., the primary device/GPU)
// } slLogicalDeviceDesc;

// typedef struct slSwapchainDesc {
//     slStructureType sType;              // Should be set to SL_STRUCT_TYPE_SWAPCHAIN_DESC
//     const void* pNext;                  // Pointer to extension-specific structures, or NULL
//     uint32_t SurfaceBuffers;            // 1 for immediate, 2 for double buffering, 3 for triple buffering, etc.
//     slPresentMode PresentMode;          // Selects the presentation mode (refer to slPresentMode).
//     bool Clipped;                       // Whether the swapchain can discard rendering operations for pixels that are obscured or completely hidden from view.
//     slSwapchainExtensions ext;          // Swapchain extension sub-struct.
// } slSwapchainDesc;

typedef struct slLogicalDeviceDesc {
    slStructureType sType;
    const void* pNext;

    slPhysicalDevice physicalDevice;
    
    bool EnableDynamicRendering;
    bool EnableAnisotropicFiltering;
} slLogicalDeviceDesc;

typedef struct slSwapchainDesc {
    slStructureType sType;
    const void* pNext;

    slWindow targetWindow;
    uint32_t BufferCount;
    slPresentMode PresentMode;
} slSwapchainDesc;

#ifdef __cplusplus
}
#endif

#endif // STARLIGHT_TYPES_H