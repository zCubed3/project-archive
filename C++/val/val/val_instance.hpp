#ifndef SAPPHIRE_VULKAN_INSTANCE_H
#define SAPPHIRE_VULKAN_INSTANCE_H

#include <string>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <val/data/val_buffer.hpp>
#include <val/data/val_staging_buffer.hpp>
#include <val/render_targets/val_window_render_target.h>
#include <val/val_extension.hpp>
#include <val/val_layer.hpp>
#include <val/val_queue.hpp>

#include <optional>

// TODO: Frames in flight?
namespace VAL {
    struct ValInstancePresentPreferences {
        bool sRGB = false;
        bool depth32Bit = false;
        bool vsync = true;
    };

    struct ValPresentInfo {
        VkFormat vk_color_format;
        VkColorSpaceKHR vk_color_space;
        VkFormat vk_depth_format;
        VkPresentModeKHR vk_present_mode;
    };

    struct ValInstanceCreateInfo {
        enum class VulkanAPIVersion {
            Version1_0,
            Version1_1,
            Version1_2,
            Version1_3
        };

        enum VerbosityFlags {
            VERBOSITY_FLAG_LIST_GPUS = 1,
            VERBOSITY_FLAG_LIST_INSTANCE_EXTENSIONS = 2,
            VERBOSITY_FLAG_LIST_DEVICE_EXTENSIONS = 4,
            VERBOSITY_FLAG_LIST_LAYERS = 8,
        };

        std::vector<ValExtension> instance_extensions;
        std::vector<ValExtension> device_extensions;
        std::vector<ValLayer> validation_layers;

        std::vector<ValQueue::QueueType> requested_queues;

        std::string engine_name = "Unknown Engine";
        std::string app_name = "VAL Application";
        VulkanAPIVersion version = VulkanAPIVersion::Version1_0;

        uint32_t app_major_version = 1;
        uint32_t app_minor_version = 0;
        uint32_t app_patch_version = 0;

        uint32_t engine_major_version = 1;
        uint32_t engine_minor_version = 0;
        uint32_t engine_patch_version = 0;

        uint32_t verbosity_flags = 0;

        std::vector<VkFormat> vk_color_formats;
        std::vector<VkFormat> vk_depth_formats;
        std::vector<VkPresentModeKHR> vk_present_modes;

        VkPhysicalDeviceFeatures vk_enabled_features;

#ifdef SDL_SUPPORT
        // TODO: Multiple window support?
        SDL_Window *p_sdl_window = nullptr;
        std::optional<ValInstancePresentPreferences> present_preferences;
#endif
    };

    class ValInstance {
    protected:
        struct ChosenGPU {
            VkPhysicalDevice vk_device;
            VkPhysicalDeviceFeatures vk_features;
            std::vector<ValQueue> queues;
        };

        enum CreationError {
            ERR_NONE,
            ERR_CREATE_INFO_NULLPTR,
            ERR_VK_INSTANCE_FAILURE,
            ERR_VK_DEVICE_FAILURE,
            ERR_VK_GPU_MISSING,
            ERR_VK_MISSING_INSTANCE_EXTENSION
        };

        static CreationError last_error;

        ValInstance() = default;

        static std::vector<ValExtension> validate_instance_extensions(const ValInstanceCreateInfo& p_create_info);
        static std::vector<ValExtension> validate_device_extensions(VkPhysicalDevice vk_gpu, const ValInstanceCreateInfo& p_create_info);
        static std::vector<ValLayer> validate_layers(const ValInstanceCreateInfo& p_create_info);

        static VkInstance create_vk_instance(const ValInstanceCreateInfo& p_create_info);
        static ChosenGPU pick_gpu(VkInstance vk_instance, VkSurfaceKHR vk_surface, const ValInstanceCreateInfo& p_create_info);
        static VkDevice create_vk_device(ChosenGPU &vk_gpu, std::vector<ValQueue> &val_queues, const ValInstanceCreateInfo& p_create_info);
        static VmaAllocator create_vma_allocator(VkInstance vk_instance, VkDevice vk_device, VkPhysicalDevice vk_gpu);
        static VkDescriptorPool create_vk_descriptor_pool(VkDevice vk_device);

        static VkSemaphore create_vk_semaphore(VkDevice vk_device);
        static VkFence create_vk_fence(VkDevice vk_device);

        // Helper for caching present data
        bool cache_surface_info(VkInstance vk_instance, VkSurfaceKHR vk_surface, VkPhysicalDevice vk_gpu);
        VkFormat find_supported_surface_format(const std::vector<VkFormat> &vk_formats);
        VkFormat find_supported_format(const std::vector<VkFormat> &vk_formats, VkImageTiling vk_tiling, VkFormatFeatureFlags vk_feature_flags);
        VkPresentModeKHR find_supported_present_mode(const std::vector<VkPresentModeKHR> &vk_present_modes);

        bool determine_present_info(const ValInstancePresentPreferences &present_preferences);

    public:
        ValInstance(const ValInstanceCreateInfo& create_info);

        ValQueue get_queue(ValQueue::QueueType type);

        // Waits until the next frame is done rendering
        void await_frame();

        ~ValInstance();

        VkInstance vk_instance = nullptr;
        VkDevice vk_device = nullptr;
        VkPhysicalDevice vk_physical_device = nullptr;
        ValPresentInfo *present_info = nullptr;

        VmaAllocator vma_allocator;

        // TODO: Abstract sync objects?
        VkSemaphore vk_image_available_semaphore;
        VkSemaphore vk_render_finished_semaphore;
        VkFence vk_render_fence;

        // TODO: Abstract pools?
        VkDescriptorPool vk_descriptor_pool;

        std::vector<VkSurfaceFormatKHR> vk_supported_surface_formats;
        std::vector<VkPresentModeKHR> vk_supported_present_modes;

#ifdef SDL_SUPPORT
        ValWindowRenderTarget *val_main_window;
#endif

        std::vector<ValQueue> val_queues;
        bool block_await = false;
    };
}

#endif
