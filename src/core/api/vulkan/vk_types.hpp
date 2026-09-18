#pragma once
#include <vulkan/vulkan.h>
#include <vector>

// Packed into: slPhysicalDevice_t::pNativeDeviceHandle
struct VulkanPhysicalDeviceData {
    VkPhysicalDevice handle{VK_NULL_HANDLE};
    VkPhysicalDeviceProperties properties{};
    VkPhysicalDeviceFeatures features{};
};

// Packed into: slLogicalDevice_t::pDeviceData
struct VulkanLogicalDeviceData {
    VkDevice logicalDevice{VK_NULL_HANDLE};
    VkQueue graphicsQueue{VK_NULL_HANDLE};
    VkQueue presentQueue{VK_NULL_HANDLE};
    uint32_t graphicsQueueFamilyIndex{0xFFFFFFFF};
    uint32_t presentQueueFamilyIndex{0xFFFFFFFF};
};

// Packed into: slSwapchain_t::pSwapchainData
struct VulkanSwapchainData {
    VkSwapchainKHR handle{VK_NULL_HANDLE};
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> framebuffers; // Empty if dynamicRenderingEnabled == true
};

// Packed into: slWindowSurface_t::pSurfaceData
struct VulkanSurfaceData {
    VkSurfaceKHR handle{VK_NULL_HANDLE};
};