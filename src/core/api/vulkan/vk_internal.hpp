#pragma once

#include <vulkan/vulkan.h>
#include <vector>

struct vkWindowInstanceState {
    VulkanContext context;
    VulkanSurface surface;
    VulkanSwapchain swapchain;
    VulkanPipeline pipeline;
    VulkanCommands commands;
    VulkanSync sync;

    bool dynamicRendering{false};
};

struct VulkanContext {
    VkInstance instance{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkDevice logicalDevice{VK_NULL_HANDLE};
    VkQueue graphicsQueue{VK_NULL_HANDLE};
    VkQueue presentQueue{VK_NULL_HANDLE};
};

struct VulkanSurface {
    VkSurfaceKHR surface{VK_NULL_HANDLE};
    slWindow parentWindow{nullptr};
};

struct VulkanSwapchain {
    VkSwapchainKHR handle{VK_NULL_HANDLE};
    std::vector<VkImage> images;
    VkFormat imageFormat{};
    VkExtent2D extent{};
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> framebuffers;
};

struct VulkanPipeline {
    VkPipeline handle{VK_NULL_HANDLE};
    VkPipelineLayout layout{VK_NULL_HANDLE};

    // If not using Dynamic Rendering
    VkRenderPass renderPass{VK_NULL_HANDLE}; 

    // If using Dynamic Rendering
    VkFormat colorAttachmentFormat{VK_FORMAT_UNDEFINED};
};

struct VulkanCommands {
    VkCommandPool pool{VK_NULL_HANDLE};
    std::vector<VkCommandBuffer> buffers;
};

struct VulkanSync {
    std::vector<VkSemaphore> imageAvailable;
    std::vector<VkSemaphore> renderFinished;
    std::vector<VkFence> inFlight;
};