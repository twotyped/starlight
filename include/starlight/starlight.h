#ifndef STARLIGHT_H
#define STARLIGHT_H

#include "starlight_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the global Starlight subsystems.
 * @param pDesc Pointer to initialization descriptor options.
 * @return SL_SUCCESS on success, or appropriate slResult error code.
 */
SL_API slResult slInit(const slInitializationDesc* pDesc);

/**
 * @brief Shuts down all global Starlight subsystems and cleans up allocated memory.
 */
SL_API void slShutdown(void);

/**
 * @brief Creates a new Window Instance.
 * @param pDesc Configuration descriptor for the window instance.
 * @param pOutInstance Pointer to receive the allocated slWindowInstance handle.
 * @return SL_SUCCESS on success, or slResult error code.
 */
SL_API slResult slCreateWindowInstance(const slWindowInstanceDesc* pDesc, slWindowInstance* pOutInstance);

/**
 * @brief Destroys an existing Window Instance and frees related surface allocations.
 * @param instance The window instance handle to destroy.
 */
SL_API void slDestroyWindowInstance(slWindowInstance instance);

/**
 * @brief Enumerates physical GPUs available on the system for a given window instance.
 * @param instance The active window instance.
 * @param pCount Pointer to hold/receive the count of physical devices.
 * @param pOutDevices Array buffer to receive slPhysicalDevice handles (pass NULL to query count).
 * @return SL_SUCCESS on success, or slResult error code.
 */
SL_API slResult slEnumeratePhysicalDevices(slWindowInstance instance, uint32_t* pCount, slPhysicalDevice* pOutDevices);

/**
 * @brief Configures a logical device and swapchain configuration handle (deferred creation).
 * @param pDeviceDesc Configuration descriptor for selecting physical devices and logical queues.
 * @param pSwapchainDesc Swapchain configuration descriptor.
 * @param pOutDevice Pointer to receive the allocated slLogicalDevice handle.
 * @return SL_SUCCESS on success, or slResult error code.
 */
SL_API slResult slCreateLogicalDevice(const slLogicalDeviceDesc* pDeviceDesc, const slSwapchainDesc* pSwapchainDesc, slLogicalDevice* pOutDevice);

/**
 * @brief Destroys a logical device configuration handle.
 * @param device The logical device handle to destroy.
 */
SL_API void slDestroyLogicalDevice(slLogicalDevice device);

/**
 * @brief Instantiates the actual native OS window, surface, and graphics pipelines using deferred descriptors.
 * @param instance The parent window instance.
 * @param device The configured deferred logical device handle.
 * @param pOutWindow Pointer to receive the fully instantiated slWindow handle.
 * @return SL_SUCCESS on success, or slResult error code.
 */
SL_API slResult slCreateWindow(slWindowInstance instance, slLogicalDevice device, slWindow* pOutWindow);

/**
 * @brief Destroys an active native window, swapchain, and surface handles.
 * @param window The window handle to destroy.
 */
SL_API void slDestroyWindow(slWindow window);


/**
 * @brief Queries whether the user or OS has requested to close the window.
 * @param window The target window to check.
 * @return True if a close request is pending, false otherwise.
 */
SL_API bool slWindowShouldClose(slWindow window);

/**
 * @brief Non-blocking poll for native OS window events across all managed windows.
 */
SL_API void slPollEvents(void);

/**
 * @brief Creates a CPU-backed surface buffer associated with a window.
 * @param window The window that owns and presents the surface buffer.
 * @param pDesc Configuration descriptor for the surface buffer.
 * @param pOutBuffer Pointer to receive the allocated surface buffer handle.
 * @return SL_SUCCESS on success, or an slResult error code.
 */
SL_API slResult slCreateWindowSurfaceBuffer(slWindow window, const slWindowSurfaceBufferDesc* pDesc, slWindowSurfaceBuffer* pOutBuffer);

/**
 * @brief Maps a surface buffer for CPU access.
 * @param buffer The surface buffer to map.
 * @param pPixels Pointer to receive the writable pixel data address.
 * @param pRowPitch Pointer to receive the number of bytes between adjacent rows.
 * @return SL_SUCCESS on success, or an slResult error code.
 */
SL_API slResult slMapWindowSurfaceBuffer(slWindowSurfaceBuffer buffer, void** pPixels, uint32_t* pRowPitch);

/**
 * @brief Presents the current contents of a surface buffer to its window.
 * @param buffer The surface buffer to present.
 * @return SL_SUCCESS on success, or an slResult error code.
 */
SL_API slResult slPresentWindowSurfaceBuffer(slWindowSurfaceBuffer buffer);

/**
 * @brief Destroys a surface buffer and releases its associated storage.
 * @param buffer The surface buffer handle to destroy.
 */
SL_API void slDestroyWindowSurfaceBuffer(slWindowSurfaceBuffer buffer);

#ifdef __cplusplus
}
#endif

#endif